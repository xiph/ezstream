/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007, 2009              Moritz Grimm <mgrimm@mrsserver.net>
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

/*
 * This file contains utility functions, as well as a few other unexciting
 * but verbose functions outsourced from ezstream.c to make it more readable.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "compat.h"

#include <ctype.h>
#include <errno.h>
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif
#include <limits.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#include <stdio.h>
#include <string.h>

#ifdef HAVE_ICONV
# include <iconv.h>
#endif
#include <shout/shout.h>

#include "cfg.h"
#include "log.h"
#include "util.h"
#include "xalloc.h"

#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

char *	iconvert(const char *, const char *, const char *, int);

int
strrcmp(const char *s, const char *sub)
{
	size_t	slen = strlen(s);
	size_t	sublen = strlen(sub);

	if (sublen > slen)
		return (1);

	return (memcmp(s + slen - sublen, sub, sublen));
}

int
strrcasecmp(const char *s, const char *sub)
{
	char	*s_cpy = xstrdup(s);
	char	*sub_cpy = xstrdup(sub);
	char	*p;
	int	 ret;

	for (p = s_cpy; *p != '\0'; p++)
		*p = tolower((int)*p);

	for (p = sub_cpy; *p != '\0'; p++)
		*p = tolower((int)*p);

	ret = strrcmp(s_cpy, sub_cpy);

	xfree(s_cpy);
	xfree(sub_cpy);

	return (ret);
}

char *
CHARtoUTF8(const char *in_str, int mode)
{
	char	*codeset;

#if defined(HAVE_NL_LANGINFO) && defined(HAVE_SETLOCALE) && defined(CODESET)
	setlocale(LC_CTYPE, "");
	codeset = nl_langinfo((nl_item)CODESET);
	setlocale(LC_CTYPE, "C");
#else
	codeset = (char *)"";
#endif /* HAVE_NL_LANGINFO && HAVE_SETLOCALE && CODESET */

	return (iconvert(in_str, codeset, "UTF-8", mode));
}

char *
UTF8toCHAR(const char *in_str, int mode)
{
	char	*codeset;

#if defined(HAVE_NL_LANGINFO) && defined(HAVE_SETLOCALE) && defined(CODESET)
	setlocale(LC_CTYPE, "");
	codeset = nl_langinfo((nl_item)CODESET);
	setlocale(LC_CTYPE, "C");
#else
	codeset = (char *)"";
#endif /* HAVE_NL_LANGINFO && HAVE_SETLOCALE && CODESET */

	return (iconvert(in_str, "UTF-8", codeset, mode));
}

char *
iconvert(const char *in_str, const char *from, const char *to, int mode)
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

	switch (mode) {
		size_t	siz;

	case ICONV_TRANSLIT:
		siz = strlen(to) + strlen("//TRANSLIT") + 1;
		tocode = xcalloc(siz, sizeof(char));
		snprintf(tocode, siz, "%s//TRANSLIT", to);
		break;
	case ICONV_IGNORE:
		siz = strlen(to) + strlen("//IGNORE") + 1;
		tocode = xcalloc(siz, sizeof(char));
		snprintf(tocode, siz, "%s//IGNORE", to);
		break;
	case ICONV_REPLACE:
		/* FALLTHROUGH */
	default:
		tocode = xstrdup(to);
		break;
	}

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
	(void)mode;

	if (NULL == in_str)
		return (xstrdup(""));

	return (xstrdup(in_str));
#endif /* HAVE_ICONV */
}

char *
replaceString(const char *source, const char *from, const char *to)
{
	char		*to_quoted, *dest;
	size_t		 dest_size;
	const char	*p1, *p2;

	to_quoted = shellQuote(to);
	dest_size = strlen(source) + strlen(to_quoted) + 1;
	dest = xcalloc(dest_size, sizeof(char));

	p1 = source;
	p2 = strstr(p1, from);
	if (p2 != NULL) {
		strncat(dest, p1, (size_t)(p2 - p1));
		strlcat(dest, to_quoted, dest_size);
		p1 = p2 + strlen(from);
	}
	strlcat(dest, p1, dest_size);

	xfree(to_quoted);

	return (dest);
}

#define SHELLQUOTE_INLEN_MAX	8191UL

char *
shellQuote(const char *in)
{
	char		*out, *out_p;
	size_t		 out_len;
	const char	*in_p;

	out_len = (strlen(in) > SHELLQUOTE_INLEN_MAX)
	    ? SHELLQUOTE_INLEN_MAX
	    : strlen(in);
	out_len = out_len * 2 + 2;
	out = xcalloc(out_len + 1, sizeof(char));

	out_p = out;
	in_p = in;

	*out_p++ = '\'';
	out_len--;
	while (*in_p && out_len > 2) {
		switch (*in_p) {
		case '\'':
		case '\\':
			*out_p++ = '\\';
			out_len--;
			break;
		default:
			break;
		}
		*out_p++ = *in_p++;
		out_len--;
	}
	*out_p++ = '\'';

	return (out);
}

int
urlParse(const char *url, char **hostname, unsigned short *port,
	 char **mountname)
{
	const char	*p1, *p2, *p3;
	char		 tmpPort[6] = "";
	size_t		 hostsiz, mountsiz;
	const char	*errstr;

	if (strncmp(url, "http://", strlen("http://")) != 0) {
		log_error("invalid <url>: not an HTTP address");
		return (0);
	}

	p1 = url + strlen("http://");
	p2 = strchr(p1, ':');
	if (p2 == NULL) {
		log_error("invalid <url>: missing port");
		return (0);
	}
	hostsiz = (p2 - p1) + 1;
	if (hostsiz <= 1) {
		log_error("invalid <url>: missing host");
		return (0);
	}
	*hostname = xmalloc(hostsiz);
	strlcpy(*hostname, p1, hostsiz);

	p2++;
	p3 = strchr(p2, '/');
	if (p3 == NULL || p3 - p2 >= (int)sizeof(tmpPort)) {
		log_error("invalid <url>: mountpoint missing, or port number too long");
		xfree(*hostname);
		return (0);
	}

	strlcpy(tmpPort, p2, (p3 - p2) + 1UL);
	*port = (unsigned short)strtonum(tmpPort, 1LL, (long long)USHRT_MAX, &errstr);
	if (errstr) {
		log_error("invalid <url>: port: %s is %s", tmpPort, errstr);
		xfree(*hostname);
		return (0);
	}

	mountsiz = strlen(p3) + 1;
	*mountname = xmalloc(mountsiz);
	strlcpy(*mountname, p3, mountsiz);

	return (1);
}
