/*
 * Copyright (c) 2017 Moritz Grimm <mgrimm@mrsserver.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "compat.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#if defined(HAVE_LIBGEN_H) && !defined(__linux__)
# include <libgen.h>
#endif /* HAVE_LIBGEN_H && !__linux__ */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <taglib/tag_c.h>

#include "cfg.h"
#include "log.h"
#include "mdata.h"
#include "util.h"
#include "xalloc.h"

struct mdata {
	char	*filename;
	char	*name;
	char	*artist;
	char	*album;
	char	*title;
	char	*songinfo;
	int	 length;
	int	 normalize_strings;
	int	 run_program;
};

enum mdata_request {
	MDATA_ARTIST,
	MDATA_ALBUM,
	MDATA_TITLE,
	MDATA_SONGINFO
};

static void	_mdata_clear(struct mdata *);
static char *	_mdata_get_name_from_filename(const char *);
static void	_mdata_generate_songinfo(struct mdata *);
static void	_mdata_normalize_string(char **);
static void	_mdata_normalize_strings(struct mdata *);
static char *	_mdata_run(const char *, enum mdata_request);

static void
_mdata_clear(struct mdata *md)
{
	int	normalize_strings;

	normalize_strings = md->normalize_strings;
	xfree(md->filename);
	xfree(md->name);
	xfree(md->artist);
	xfree(md->album);
	xfree(md->title);
	xfree(md->songinfo);
	memset(md, 0, sizeof(*md));
	md->length = -1;
	md->normalize_strings = normalize_strings;
}

static char *
_mdata_get_name_from_filename(const char *filename)
{
	char	*tmp;
	char	*p1, *p2, *name;

	/*
	 * Make a copy of filename in case basename() is broken and attempts
	 * to modify its argument.
	 */
	tmp = xstrdup(filename);
	if ((p1 = basename(tmp)) == NULL) {
		/*
		 * Some implementations limit the input to PATH_MAX; bail out
		 * if that is exceeded and an error is returned.
		 */
		log_alert("%s: %s", tmp, strerror(errno));
		exit(1);
	}

	if ((p2 = strrchr(p1, '.')) != NULL)
		*p2 = '\0';

	if (strlen(p1) == 0)
		name = xstrdup("[unknown]");
	else
		name = util_char2utf8(p1);

	xfree(tmp);

	return (name);
}

void
_mdata_generate_songinfo(struct mdata *md)
{
	char	 *str;
	size_t	  str_size;

	str_size = 0;
	if (md->artist)
		str_size += strlen(md->artist);
	if (md->title) {
		if (str_size)
			str_size += strlen(" - ");
		str_size += strlen(md->title);
	}
	if (md->album) {
		if (str_size)
			str_size += strlen(" - ");
		str_size += strlen(md->album);
	}
	if (!str_size)
		return;
	str_size++;
	str = xcalloc(str_size, sizeof(*str));

	if (md->artist)
		strlcpy(str, md->artist, str_size);
	if (md->title) {
		if (strlen(str))
			strlcat(str, " - ", str_size);
		strlcat(str, md->title, str_size);
	}
	if (md->album) {
		if (strlen(str))
			strlcat(str, " - ", str_size);
		strlcat(str, md->album, str_size);
	}

	md->songinfo = str;
}

static void
_mdata_normalize_string(char **s)
{
	char	*str, *cp, *tmpstr, *tp;
	int	 is_space;

	str = *s;
	if (NULL == str)
		return;

	tmpstr = xcalloc(strlen(str) + 1, sizeof(char));

	tp = tmpstr;
	is_space = 1;
	for (cp = str; *cp != '\0'; cp++) {
		if (*cp == ' ') {
			if (!is_space)
				*tp++ = ' ';
			is_space = 1;
		} else {
			*tp++ = *cp;
			is_space = 0;
		}
	}
	if (strlen(tmpstr) > 0 && tmpstr[strlen(tmpstr) - 1] == ' ')
		tmpstr[strlen(tmpstr) - 1] = '\0';

	xfree(str);
	*s = xreallocarray(tmpstr, strlen(tmpstr) + 1, sizeof(*tmpstr));
}

static void
_mdata_normalize_strings(struct mdata *md)
{
	_mdata_normalize_string(&md->artist);
	_mdata_normalize_string(&md->album);
	_mdata_normalize_string(&md->title);
	_mdata_normalize_string(&md->songinfo);
}

static char *
_mdata_run(const char *program, enum mdata_request md_req)
{
	char	 cmd[PATH_MAX + sizeof(" artist")];
	char	 buf[BUFSIZ];
	FILE	*fp;
	int	 ret;

	switch (md_req) {
	case MDATA_ARTIST:
		snprintf(cmd, sizeof(cmd), "%s artist", program);
		break;
	case MDATA_ALBUM:
		snprintf(cmd, sizeof(cmd), "%s album", program);
		break;
	case MDATA_TITLE:
		snprintf(cmd, sizeof(cmd), "%s title", program);
		break;
	case MDATA_SONGINFO:
	default:
		snprintf(cmd, sizeof(cmd), "%s", program);
		break;
	}

	fflush(NULL);
	log_debug("running metadata command: %s", cmd);
	errno = ENOMEM;
	if ((fp = popen(cmd, "r")) == NULL) {
		log_error("%s: execution error: %s", cmd,
		    strerror(errno));
		return (NULL);
	}

	memset(buf, 0, sizeof(buf));
	if (NULL == fgets(buf, (int)sizeof(buf), fp) &&
	    ferror(fp)) {
		log_alert("%s: output read error: %s", program,
		    strerror(errno));
		pclose(fp);
		exit(1);
	}
	ret = pclose(fp);
	if (0 > ret) {
		log_error("%s: %s", program, strerror(errno));
		return (NULL);
	} else if (WIFSIGNALED(ret)) {
		log_error("%s: exited with signal %d", program,
		    WTERMSIG(ret));
		return (NULL);
	} else if (0 != WEXITSTATUS(ret)) {
		log_error("%s: exited with error code %d", program,
		    WEXITSTATUS(ret));
		return (NULL);
	}

	if (strlen(buf) == sizeof(buf) - 1)
		log_warning("metadata output truncated: %s", cmd);

	buf[strcspn(buf, "\n")] = '\0';
	buf[strcspn(buf, "\r")] = '\0';

	return (xstrdup(buf));
}

struct mdata *
mdata_create(void)
{
	struct mdata	*md;

	md = xcalloc(1UL, sizeof(*md));
	md->length = -1;

	return (md);
}

void
mdata_destroy(struct mdata **md_p)
{
	struct mdata	*md = *md_p;

	_mdata_clear(md);
	xfree(md);
	*md_p = NULL;
}

void
mdata_set_normalize_strings(struct mdata *md, int normalize_strings)
{
	md->normalize_strings = normalize_strings ? 1 : 0;
}

int
mdata_parse_file(struct mdata *md, const char *filename)
{
	TagLib_File			*tf;
	TagLib_Tag			*tt;
	const TagLib_AudioProperties	*ta;
	char				*str;

	if (0 > access(filename, R_OK)) {
		log_error("%s: %s", filename, strerror(errno));
		return (-1);
	}

	//taglib_set_string_management_enabled(0);
#ifdef HAVE_ICONV
	taglib_set_strings_unicode(1);
#else
	taglib_set_strings_unicode(0);
#endif /* HAVE_ICONV */

	_mdata_clear(md);
	md->filename = xstrdup(filename);
	md->name = _mdata_get_name_from_filename(filename);

	if ((tf = taglib_file_new(md->filename)) == NULL) {
		log_info("%s: unable to extract metadata",
		    md->filename);
		md->songinfo = xstrdup(md->name);
		return (0);
	}

	tt = taglib_file_tag(tf);

	str = taglib_tag_artist(tt);
	if (0 < strlen(str))
		md->artist = xstrdup(str);
	str = taglib_tag_album(tt);
	if (0 < strlen(str))
		md->album = xstrdup(str);
	str = taglib_tag_title(tt);
	if (0 < strlen(str))
		md->title = xstrdup(str);

	taglib_tag_free_strings();

	ta = taglib_file_audioproperties(tf);
	md->length = taglib_audioproperties_length(ta);

	taglib_file_free(tf);

	if (md->normalize_strings)
		_mdata_normalize_strings(md);
	_mdata_generate_songinfo(md);

	md->run_program = 0;

	return (0);
}

int
mdata_run_program(struct mdata *md, const char *program)
{
	struct stat	 st;
	char		*artist, *album, *title, *songinfo;

	if (stat(program, &st) == -1) {
		log_error("%s: %s", program, strerror(errno));
		return (-1);
	}
	if (st.st_mode & S_IWOTH) {
		log_error("%s: world writeable", program);
		return (-1);
	}
	if (!(st.st_mode & (S_IEXEC | S_IXGRP | S_IXOTH))) {
		log_error("%s: not an executable program", program);
		return (-1);
	}

	artist = album = title = songinfo = NULL;
	if (NULL == (artist   = _mdata_run(program, MDATA_ARTIST)) ||
	    NULL == (album    = _mdata_run(program, MDATA_ALBUM)) ||
	    NULL == (title    = _mdata_run(program, MDATA_TITLE)) ||
	    NULL == (songinfo = _mdata_run(program, MDATA_SONGINFO)))
		goto error;

	_mdata_clear(md);
	md->filename = xstrdup(program);
	md->name = xstrdup("[unknown]");

	if (0 == strlen(artist))
		xfree(artist);
	else
		md->artist = artist;

	if (0 == strlen(album))
		xfree(album);
	else
		md->album = album;

	if (0 == strlen(title))
		xfree(title);
	else
		md->title = title;

	if (0 == strlen(songinfo))
		xfree(songinfo);
	else
		md->songinfo = songinfo;

	if (md->normalize_strings)
		_mdata_normalize_strings(md);

	md->run_program = 1;

	return (0);

error:
	xfree(artist);
	xfree(album);
	xfree(title);
	xfree(songinfo);

	return (-1);
}

int
mdata_refresh(struct mdata *md)
{
	char	*filename = xstrdup(md->filename);
	int	 ret;

	if (md->run_program)
		ret = mdata_run_program(md, filename);
	else
		ret = mdata_parse_file(md, filename);
	xfree(filename);

	return (ret);
}

const char *
mdata_get_filename(struct mdata *md)
{
	return (md->filename);
}

const char *
mdata_get_name(struct mdata *md)
{
	return (md->name);
}

const char *
mdata_get_artist(struct mdata *md)
{
	return (md->artist);
}

const char *
mdata_get_album(struct mdata *md)
{
	return (md->album);
}

const char *
mdata_get_title(struct mdata *md)
{
	return (md->title);
}

const char *
mdata_get_songinfo(struct mdata *md)
{
	return (md->songinfo);
}

int
mdata_get_length(struct mdata *md)
{
	return (md->length);
}

int
mdata_strformat(struct mdata *md, char *buf, size_t bufsize, const char *format)
{
	struct util_dict	 dicts[6];
	char			*str;
	int			 ret;

	if (format == NULL)
		return (-1);

	memset(dicts, 0, sizeof(dicts));
	dicts[0].from = PLACEHOLDER_ARTIST;
	dicts[0].to = mdata_get_artist(md);
	dicts[1].from = PLACEHOLDER_ALBUM;
	dicts[1].to = mdata_get_album(md);
	dicts[2].from = PLACEHOLDER_TITLE;
	dicts[2].to = mdata_get_title(md);
	dicts[3].from = PLACEHOLDER_TRACK;
	dicts[3].to = mdata_get_filename(md);
	dicts[4].from = PLACEHOLDER_STRING;
	dicts[4].to = mdata_get_songinfo(md);

	str = util_expand_words(format, dicts);
	ret = (int)strlen(str);

	strlcpy(buf, str, bufsize);
	xfree(str);

	return (ret);
}
