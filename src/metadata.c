/*
 * Copyright (c) 2007, 2009 Moritz Grimm <mgrimm@mrsserver.net>
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
#endif

#include "compat.h"

#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#if defined(HAVE_LIBGEN_H) && !defined(__linux__)
# include <libgen.h>
#endif /* HAVE_LIBGEN_H && !__linux__ */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <taglib/tag_c.h>

#include "cfg.h"
#include "log.h"
#include "metadata.h"
#include "util.h"
#include "xalloc.h"

/* Usually defined in <sys/stat.h>. */
#ifndef S_IEXEC
# define S_IEXEC	S_IXUSR
#endif /* !S_IEXEC */

static const char	*blankString = "";

struct metadata {
	char	*filename;
	char	*string;
	char	*artist;
	char	*title;
	char	*album;
	int	 songLen;
	int	 normalize;
	int	 program;
};

static struct metadata *
		metadata_create(const char *);
static void	metadata_get(struct metadata *, FILE **);
static void	metadata_clean_md(struct metadata *);
static char *	metadata_get_name(const char *);
static void	metadata_process_md(struct metadata *);
static void	metadata_normalize_string(char **);

static struct metadata *
metadata_create(const char *filename)
{
	metadata_t	md;

	md = xcalloc(1UL, sizeof(*md));
	md->filename = xstrdup(filename);
	md->songLen = -1;

	return (md);
}

static void
metadata_get(struct metadata *md, FILE **filep)
{
	TagLib_File			*tf;
	TagLib_Tag			*tt;
	const TagLib_AudioProperties	*ta;
	char				*str;

	if (filep != NULL)
		fclose(*filep);

	metadata_clean_md(md);
	taglib_set_string_management_enabled(0);
#ifdef HAVE_ICONV
	taglib_set_strings_unicode(1);
#else
	taglib_set_strings_unicode(0);
#endif /* HAVE_ICONV */

	if (md->string != NULL) {
		xfree(md->string);
		md->string = NULL;
	}

	if ((tf = taglib_file_new(md->filename)) == NULL) {
		md->string = metadata_get_name(md->filename);
		return;
	}

	tt = taglib_file_tag(tf);
	ta = taglib_file_audioproperties(tf);

	str = taglib_tag_artist(tt);
	if (str != NULL) {
		if (strlen(str) > 0)
			md->artist = xstrdup(str);
		free(str);
	}

	str = taglib_tag_title(tt);
	if (str != NULL) {
		if (strlen(str) > 0)
			md->title = xstrdup(str);
		free(str);
	}

	str = taglib_tag_album(tt);
	if (str != NULL) {
		if (strlen(str) > 0)
			md->album = xstrdup(str);
		free(str);
	}

	md->songLen = taglib_audioproperties_length(ta);

	taglib_file_free(tf);
}

static void
metadata_clean_md(struct metadata *md)
{
	if (md->string != NULL) {
		xfree(md->string);
		md->string = NULL;
	}
	if (md->artist != NULL) {
		xfree(md->artist);
		md->artist = NULL;
	}
	if (md->title != NULL) {
		xfree(md->title);
		md->title = NULL;
	}
	if (md->album != NULL) {
		xfree(md->title);
		md->album = NULL;
	}
}

static char *
metadata_get_name(const char *file)
{
	char	*filename = xstrdup(file);
	char	*p1, *p2, *name;

	if ((p1 = basename(filename)) == NULL) {
		log_alert("basename: unexpected failure with input: %s",
		    filename);
		exit(1);
	}

	if ((p2 = strrchr(p1, '.')) != NULL)
		*p2 = '\0';

	if (strlen(p1) == 0)
		name = xstrdup("[unknown]");
	else
		name = util_char2utf8(p1);

	xfree(filename);
	return (name);
}

static void
metadata_process_md(struct metadata *md)
{
	if (md->string == NULL)
		md->string = metadata_assemble_string(md);

	if (md->normalize) {
		metadata_normalize_string(&md->string);
		metadata_normalize_string(&md->artist);
		metadata_normalize_string(&md->title);
		metadata_normalize_string(&md->album);
	}
}

static void
metadata_normalize_string(char **s)
{
	char	*str, *cp, *tmpstr, *tp;
	int	 is_space;

	if (s == NULL || (str = *s) == NULL || strlen(str) == 0)
		return;

	tmpstr = xcalloc(strlen(str) + 1, sizeof(char));

	tp = tmpstr;
	is_space = 1;
	for (cp = str; *cp != '\0'; cp++) {
		if (*cp == ' ') {
			if (!is_space && strlen(tmpstr) > 0 &&
			    tmpstr[strlen(tmpstr) - 1] != ' ')
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
	*s = xreallocarray(tmpstr, strlen(tmpstr) + 1, sizeof(char));
}

struct metadata *
metadata_file(const char *filename, int normalize)
{
	struct metadata *md;

	md = metadata_create(filename);
	if (!metadata_file_update(md)) {
		metadata_free(&md);
		return (NULL);
	}

	md->normalize = normalize;

	return (md);
}

struct metadata *
metadata_program(const char *program, int normalize)
{
	struct metadata *md;
	struct stat	 st;

	if (stat(program, &st) == -1) {
		log_error("%s: %s", program, strerror(errno));
		return (NULL);
	}
	if (st.st_mode & S_IWOTH) {
		log_error("%s: world writeable", program);
		return (NULL);
	}
	if (!(st.st_mode & (S_IEXEC | S_IXGRP | S_IXOTH))) {
		log_error("%s: not an executable program", program);
		return (NULL);
	}

	md = metadata_create(program);
	md->program = 1;
	md->string = xstrdup("");
	md->normalize = normalize;

	return (md);
}

void
metadata_free(struct metadata **md_p)
{
	struct metadata *md;

	if (md_p == NULL || (md = *md_p) == NULL)
		return;

	if (md->filename != NULL) {
		xfree(md->filename);
		md->filename = NULL;
	}
	metadata_clean_md(md);
	xfree(*md_p);
	*md_p = NULL;
}


int
metadata_file_update(struct metadata *md)
{
	FILE	*filep;

	assert(!md->program);

	if ((filep = fopen(md->filename, "rb")) == NULL) {
		log_error("%s: %s", md->filename, strerror(errno));
		return (0);
	}

	metadata_get(md, &filep);
	metadata_process_md(md);

	return (1);
}

int
metadata_program_update(struct metadata *md, enum metadata_request md_req)
{
	FILE	*filep;
	char	 buf[METADATA_MAX + 1];
	char	 command[PATH_MAX + sizeof(" artist")];

	assert(md->program);

	switch (md_req) {
	case METADATA_ALL:
		metadata_clean_md(md);
		if (!metadata_program_update(md, METADATA_STRING) ||
		    !metadata_program_update(md, METADATA_ARTIST) ||
		    !metadata_program_update(md, METADATA_TITLE)  ||
		    !metadata_program_update(md, METADATA_ALBUM))
			return (0);
		else
			return (1);
	case METADATA_STRING:
		strlcpy(command, md->filename, sizeof(command));
		if (md->string != NULL) {
			xfree(md->string);
			md->string = NULL;
		}
		break;
	case METADATA_ARTIST:
		snprintf(command, sizeof(command), "%s artist", md->filename);
		if (md->artist != NULL) {
			xfree(md->artist);
			md->artist = NULL;
		}
		break;
	case METADATA_TITLE:
		snprintf(command, sizeof(command), "%s title", md->filename);
		if (md->title != NULL) {
			xfree(md->title);
			md->title = NULL;
		}
		break;
	case METADATA_ALBUM:
		snprintf(command, sizeof(command), "%s album", md->filename);
		if (md->album != NULL) {
			xfree(md->album);
			md->album = NULL;
		}
		break;
	default:
		log_alert("metadata_program_update: unknown md_req");
		abort();
	}

	fflush(NULL);
	errno = 0;
	log_debug("running command: %s", command);
	if ((filep = popen(command, "r")) == NULL) {
		/* popen() does not set errno reliably ... */
		if (errno)
			log_error("execution error: %s: %s", command,
			    strerror(errno));
		else
			log_error("execution error: %s", command);
		return (0);
	}

	memset(buf, 0, sizeof(buf));
	if (fgets(buf, (int)sizeof(buf), filep) == NULL &&
	    ferror(filep)) {
		log_alert("%s: output read error: %s", md->filename,
		    strerror(errno));
		pclose(filep);
		exit(1);
	}
	pclose(filep);

	if (strlen(buf) == sizeof(buf) - 1)
		log_warning("metadata output truncated: %s", command);

	buf[strcspn(buf, "\n")] = '\0';
	buf[strcspn(buf, "\r")] = '\0';

	switch (md_req) {
	case METADATA_STRING:
		if (strlen(buf) == 0) {
			log_warning("metadata output empty: %s",
			    md->filename);
			md->string = xstrdup("");
		} else
			md->string = xstrdup(buf);
		break;
	case METADATA_ARTIST:
		if (strlen(buf) > 0)
			md->artist = xstrdup(buf);
		break;
	case METADATA_TITLE:
		if (strlen(buf) > 0)
			md->title = xstrdup(buf);
		break;
	case METADATA_ALBUM:
		if (strlen(buf) > 0)
			md->album = xstrdup(buf);
		break;
	case METADATA_ALL:
	default:
		log_alert("metadata_program_update: METADATA_ALL in code unreachable by METADATA_ALL");
		abort();
	}

	if (md->normalize) {
		metadata_normalize_string(&md->string);
		metadata_normalize_string(&md->artist);
		metadata_normalize_string(&md->title);
		metadata_normalize_string(&md->album);
	}

	return (1);
}

const char *
metadata_get_string(struct metadata *md)
{
	assert(md->string);
	return (md->string);
}

const char *
metadata_get_artist(struct metadata *md)
{
	if (md->artist == NULL)
		return (blankString);
	else
		return (md->artist);
}

const char *
metadata_get_album(struct metadata *md)
{
	if (md->album == NULL)
		return (blankString);
	else
		return (md->album);
}

const char *
metadata_get_title(struct metadata *md)
{
	if (md->title == NULL)
		return (blankString);
	else
		return (md->title);
}

const char *
metadata_get_filename(struct metadata *md)
{
	if (md->filename == NULL)
		/* Should never happen: */
		return (blankString);
	else
		return (md->filename);
}

int
metadata_get_length(struct metadata *md)
{
	return (md->songLen);
}

char *
metadata_assemble_string(struct metadata *md)
{
	size_t	  len;
	char	 *str;

	if (md->artist == NULL && md->title == NULL && md->album && md->program == 0)
		return (metadata_get_name(md->filename));

	len = 0;
	if (md->artist != NULL)
		len += strlen(md->artist);
	if (md->title != NULL) {
		if (len > 0)
			len += strlen(" - ");
		len += strlen(md->title);
	}
	if (md->album != NULL) {
		if (len > 0)
			len += strlen(" - ");
		len += strlen(md->album);
	}
	len++;
	str = xcalloc(len, sizeof(char));

	if (md->artist != NULL)
		strlcpy(str, md->artist, len);
	if (md->title != NULL) {
		if (md->artist != NULL)
			strlcat(str, " - ", len);
		strlcat(str, md->title, len);
	}
	if (md->album != NULL) {
		if (md->artist != NULL || md->title != NULL)
			strlcat(str, " - ", len);
		strlcat(str, md->album, len);
	}

	return (str);
}

char *
metadata_format_string(struct metadata *md, const char *format)
{
	struct util_dict	 dicts[6];

	if (format == NULL)
		return (NULL);

	memset(dicts, 0, sizeof(dicts));
	dicts[0].from = PLACEHOLDER_ARTIST;
	dicts[0].to = metadata_get_artist(md);
	dicts[1].from = PLACEHOLDER_ALBUM;
	dicts[1].to = metadata_get_album(md);
	dicts[2].from = PLACEHOLDER_TITLE;
	dicts[2].to = metadata_get_title(md);
	dicts[3].from = PLACEHOLDER_TRACK;
	dicts[3].to = metadata_get_filename(md);
	dicts[4].from = PLACEHOLDER_STRING;
	dicts[4].to = metadata_get_string(md);

	return (util_expand_words(format, dicts));
}
