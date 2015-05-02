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

#include "ezstream.h"

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

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

shout_t *
stream_setup(const char *host, unsigned short port, const char *mount)
{
	shout_t *shout = NULL;

	if ((shout = shout_new()) == NULL) {
		log_syserr(ERROR, ENOMEM, "shout_new");
		return (NULL);
	}

	if (shout_set_host(shout, host) != SHOUTERR_SUCCESS) {
		log_error("shout_set_host: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
		log_error("shout_set_protocol: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_port(shout, port) != SHOUTERR_SUCCESS) {
		log_error("shout_set_port: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_password(shout, cfg_get_server_password()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_password: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_mount(shout, mount) != SHOUTERR_SUCCESS) {
		log_error("shout_set_mount: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS) {
		log_error("shout_set_user: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	if (CFG_STREAM_MP3 == cfg_get_stream_format() &&
	    shout_set_format(shout, SHOUT_FORMAT_MP3) != SHOUTERR_SUCCESS) {
		log_error("shout_set_format(MP3): %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if ((CFG_STREAM_VORBIS == cfg_get_stream_format() ||
	    CFG_STREAM_THEORA == cfg_get_stream_format()) &&
	    shout_set_format(shout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
		log_error("shout_set_format(OGG): %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	if (shout_set_user(shout, cfg_get_server_user()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_user: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_name() &&
	    shout_set_name(shout, cfg_get_stream_name()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_name: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_url() &&
	    shout_set_url(shout, cfg_get_stream_url()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_url: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_genre() &&
	    shout_set_genre(shout, cfg_get_stream_genre()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_genre: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_description() &&
	    shout_set_description(shout, cfg_get_stream_description()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_description: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_bitrate() &&
	    shout_set_audio_info(shout, SHOUT_AI_BITRATE, cfg_get_stream_bitrate()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_audio_info(AI_BITRATE): %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_channels() &&
	    shout_set_audio_info(shout, SHOUT_AI_CHANNELS, cfg_get_stream_channels()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_audio_info(AI_CHANNELS): %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_samplerate() &&
	    shout_set_audio_info(shout, SHOUT_AI_SAMPLERATE, cfg_get_stream_samplerate()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_audio_info(AI_SAMPLERATE): %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (cfg_get_stream_quality() &&
	    shout_set_audio_info(shout, SHOUT_AI_QUALITY, cfg_get_stream_quality()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_audio_info(AI_QUALITY): %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	if (shout_set_public(shout, (unsigned int)cfg_get_stream_server_public()) != SHOUTERR_SUCCESS) {
		log_error("shout_set_public: %s",
		    shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	return (shout);
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

int
ez_gettimeofday(void *tp_arg)
{
	struct timeval	*tp = (struct timeval *)tp_arg;
	int		 ret = -1;

#ifdef HAVE_GETTIMEOFDAY
	ret = gettimeofday(tp, NULL);
#else /* HAVE_GETTIMEOFDAY */
	/* Fallback to time(): */
	tp->tv_sec = (long)time(NULL);
	tp->tv_usec = 0;
	ret = 0;
#endif /* HAVE_GETTIMEOFDAY */

	return (ret);
}
