/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007, 2009, 2017        Moritz Grimm <mgrimm@mrsserver.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "compat.h"

#include <sys/types.h>
#include <sys/file.h>

#include <ctype.h>
#include <errno.h>
#include <langinfo.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_ICONV
# include <iconv.h>
#endif

#include "log.h"
#include "util.h"
#include "xalloc.h"

#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

static char		*pidfile_path;
static FILE		*pidfile_file;
static pid_t		 pidfile_pid;
static unsigned int	 pidfile_numlocks;

static char *	_util_iconvert(const char *, const char *, const char *);
static void	_util_cleanup_pidfile(void);

static char *
_util_iconvert(const char *in_str, const char *from, const char *to)
{
#ifdef HAVE_ICONV
	iconv_t 		 cd;
	ICONV_CONST char	*input, *ip;
	size_t			 input_len;
	char			*output;
	size_t			 output_size;
	char			 buf[BUFSIZ], *bp;
	size_t			 bufavail;
	size_t			 out_pos;
	char			*tocode;

	if (NULL == in_str)
		return (xstrdup(""));

	tocode = xstrdup(to);
	if ((cd = iconv_open(tocode, from)) == (iconv_t)-1 &&
	    (cd = iconv_open("", from)) == (iconv_t)-1 &&
	    (cd = iconv_open(tocode, "")) == (iconv_t)-1) {
		xfree(tocode);
		log_syserr(ERROR, errno, "iconv_open");
		return (xstrdup(in_str));
	}

	ip = input = (ICONV_CONST char *)in_str;
	input_len = strlen(input);
	output_size = 1;
	output = xcalloc(output_size, sizeof(char));
	out_pos = 0;
	output[out_pos] = '\0';
	while (input_len > 0) {
		char	*op;
		size_t	 count;

		buf[0] = '\0';
		bp = buf;
		bufavail = sizeof(buf) - 1;

		if (iconv(cd, &ip, &input_len, &bp, &bufavail) == (size_t)-1 &&
		    errno != E2BIG) {
			*bp++ = '?';
			ip++;
			input_len--;
			bufavail--;
		}
		*bp = '\0';

		count = sizeof(buf) - bufavail - 1;

		output_size += count;
		op = output = xreallocarray(output, output_size, sizeof(char));
		op += out_pos;
		memcpy(op, buf, count);
		out_pos += count;
		op += count;
		*op = '\0';
	}

	if (iconv_close(cd) == -1) {
		log_syserr(ERROR, errno, "iconv_close");
		xfree(output);
		xfree(tocode);
		return (xstrdup(in_str));
	}

	xfree(tocode);
	return (output);
#else
	(void)from;
	(void)to;

	if (NULL == in_str)
		return (xstrdup(""));

	return (xstrdup(in_str));
#endif /* HAVE_ICONV */
}

static void
_util_cleanup_pidfile(void)
{
	if (NULL != pidfile_path && getpid() == pidfile_pid) {
		(void)unlink(pidfile_path);
		(void)fclose(pidfile_file);
	}
}

const char *
util_get_progname(const char *argv0)
{
#ifdef HAVE___PROGNAME
	extern char	*__progname;

	(void)argv0;

	return (__progname);
#else
	if (argv0 == NULL) {
		return (UTIL_DEFAULT_PROGNAME);
	} else {
		const char	*p = strrchr(argv0, '/');

		if (p == NULL)
			p = argv0;
		else
			p++;

		return (p);
	}
#endif /* HAVE___PROGNAME */
}

int
util_write_pid_file(const char *path)
{
	int	 save_errno = 0;
	pid_t	 pid;

	if (NULL == path)
		return (0);

	xfree(pidfile_path);
	pidfile_path = xstrdup(path);

	if (NULL != pidfile_file)
		fclose(pidfile_file);
	if (NULL == (pidfile_file = fopen(pidfile_path, "w"))) {
		xfree(pidfile_path);
		pidfile_path = NULL;
		return (-1);
	}

	pid = getpid();
	if (0 >= fprintf(pidfile_file, "%ld\n", (long)pid) ||
	    0 > fflush(pidfile_file) ||
	    0 > flock(fileno(pidfile_file), LOCK_EX | LOCK_NB))
		goto error;

	if (0 == pidfile_numlocks) {
		pidfile_pid = pid;
		if (0 != atexit(_util_cleanup_pidfile))
			goto error;
		pidfile_numlocks++;
	}

	return (0);

error:
	save_errno = errno;
	(void)unlink(pidfile_path);
	xfree(pidfile_path);
	pidfile_path = NULL;
	(void)fclose(pidfile_file);
	pidfile_file = NULL;
	pidfile_pid = 0;
	errno = save_errno;

	return (-1);
}

int
util_strrcmp(const char *s, const char *sub)
{
	size_t	slen = strlen(s);
	size_t	sublen = strlen(sub);

	if (sublen > slen)
		return (1);

	return (!!memcmp(s + slen - sublen, sub, sublen));
}

int
util_strrcasecmp(const char *s, const char *sub)
{
	char	*s_cpy = xstrdup(s);
	char	*sub_cpy = xstrdup(sub);
	char	*p;
	int	 ret;

	for (p = s_cpy; *p != '\0'; p++)
		*p = (char)tolower((int)*p);

	for (p = sub_cpy; *p != '\0'; p++)
		*p = (char)tolower((int)*p);

	ret = util_strrcmp(s_cpy, sub_cpy);

	xfree(s_cpy);
	xfree(sub_cpy);

	return (ret);
}

char *
util_char2utf8(const char *in_str)
{
	char	*codeset;

	setlocale(LC_CTYPE, "");
	codeset = nl_langinfo((nl_item)CODESET);
	setlocale(LC_CTYPE, "C");

	return (_util_iconvert(in_str, codeset, "UTF-8"));
}

char *
util_utf82char(const char *in_str)
{
	char	*codeset;

	setlocale(LC_CTYPE, "");
	codeset = nl_langinfo((nl_item)CODESET);
	setlocale(LC_CTYPE, "C");

	return (_util_iconvert(in_str, "UTF-8", codeset));
}

char *
util_expand_words(const char *in, struct util_dict dicts[])
{
	size_t	 i;
	char	*out;
	size_t	 out_size = strlen(in) + 1;

	/* empty input string? */
	if (1 == out_size)
		return (NULL);

	out = xstrdup(in);
	i = out_size - 1;
	while (i--) {
		struct util_dict *d = dicts;

		while (d && d->from) {
			if (0 == strncmp(&out[i], d->from, strlen(d->from))) {
				char	*buf, *tmp;
				size_t	 buf_len;

				buf_len = strlen(&out[i]) + strlen(d->to)
				    - strlen(d->from);
				buf = xcalloc(buf_len + 1, sizeof(*buf));
				snprintf(buf, buf_len + 1, "%s%s",
				    d->to, &out[i + strlen(d->from)]);

				out_size += buf_len;
				tmp = xcalloc(out_size, sizeof(*tmp));
				snprintf(tmp, i + 1, "%s", out);
				snprintf(tmp + i, out_size - i, "%s", buf);
				free(buf);
				free(out);
				out = tmp;

				break;
			}
			d++;
		}
	}

	return (out);
}

#define SHELLQUOTE_OUTLEN_MAX	8191UL

char *
util_shellquote(const char *in, size_t outlen_max)
{
	char		*out, *out_p;
	size_t		 out_len;
	const char	*in_p;

	if (!outlen_max || outlen_max > SHELLQUOTE_OUTLEN_MAX)
		outlen_max = SHELLQUOTE_OUTLEN_MAX;
	out_len = outlen_max;

	out = xcalloc(out_len + 1, sizeof(char));

	out_p = out;
	in_p = in;

	*out_p++ = '\'';
	out_len--;
	while (*in_p && 1 < out_len) {
		int	stop = 0;

		switch (*in_p) {
		case '\'':
			if (4 < out_len) {
				*out_p++ = '\'';
				*out_p++ = '\\';
				*out_p++ = '\'';
				out_len -= 3;
			} else
				stop = 1;
			break;
		default:
			break;
		}
		if (stop)
			break;
		*out_p++ = *in_p++;
		out_len--;
	}
	*out_p++ = '\'';
	out_len--;

	return (out);
}
