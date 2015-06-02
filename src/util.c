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
stream_setup(void)
{
	shout_t *shout;

	if ((shout = shout_new()) == NULL) {
		log_syserr(ERROR, ENOMEM, "shout_new");
		return (NULL);
	}

	switch (cfg_get_server_protocol()) {
	case CFG_PROTO_HTTP:
		if (SHOUTERR_SUCCESS !=
		    shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP)) {
			log_error("shout_set_protocol: %s",
			    shout_get_error(shout));
			goto error;
		}
		break;
	default:
		log_error("unsupported protocol: %s",
		    cfg_get_server_protocol_str());
		goto error;
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_host(shout, cfg_get_server_hostname())) {
		log_error("shout_set_host: %s", shout_get_error(shout));
		goto error;
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_port(shout, (unsigned short)cfg_get_server_port())) {
		log_error("shout_set_port: %s", shout_get_error(shout));
		goto error;
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_user(shout, cfg_get_server_user())) {
		log_error("shout_set_user: %s", shout_get_error(shout));
		goto error;
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_password(shout, cfg_get_server_password())) {
		log_error("shout_set_password: %s", shout_get_error(shout));
		goto error;
	}

	if (SHOUTERR_SUCCESS !=
	    shout_set_mount(shout, cfg_get_stream_mountpoint())) {
		log_error("shout_set_mount: %s", shout_get_error(shout));
		goto error;
	}

	switch (cfg_get_stream_format()) {
	case CFG_STREAM_VORBIS:
	case CFG_STREAM_THEORA:
		if (SHOUTERR_SUCCESS !=
		    shout_set_format(shout, SHOUT_FORMAT_OGG)) {
			log_error("shout_set_format: OGG: %s",
			    shout_get_error(shout));
			goto error;
		}
		break;
	case CFG_STREAM_MP3:
		if (SHOUTERR_SUCCESS !=
		    shout_set_format(shout, SHOUT_FORMAT_MP3)) {
			log_error("shout_set_format: MP3: %s",
			    shout_get_error(shout));
			goto error;
		}
		break;
	default:
		log_error("unsupported stream format: %s",
		    cfg_get_stream_format_str());
		goto error;
	}

	if (cfg_get_stream_name() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_name(shout, cfg_get_stream_name())) {
		log_error("shout_set_name: %s", shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_url() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_url(shout, cfg_get_stream_url())) {
		log_error("shout_set_url: %s", shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_genre() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_genre(shout, cfg_get_stream_genre())) {
		log_error("shout_set_genre: %s", shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_description() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_description(shout, cfg_get_stream_description())) {
		log_error("shout_set_description: %s",
		    shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_quality() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(shout, SHOUT_AI_QUALITY,
				 cfg_get_stream_quality())) {
		log_error("shout_set_audio_info: QUALITY: %s",
		    shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_bitrate() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(shout, SHOUT_AI_BITRATE,
				 cfg_get_stream_bitrate())) {
		log_error("shout_set_audio_info: BITRATE: %s",
		    shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_samplerate() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(shout, SHOUT_AI_SAMPLERATE,
				 cfg_get_stream_samplerate())) {
		log_error("shout_set_audio_info: SAMPLERATE: %s",
		    shout_get_error(shout));
		goto error;
	}
	if (cfg_get_stream_channels() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(shout, SHOUT_AI_CHANNELS,
				 cfg_get_stream_channels())) {
		log_error("shout_set_audio_info: CHANNELS: %s",
		    shout_get_error(shout));
		goto error;
	}

	if (SHOUTERR_SUCCESS !=
	    shout_set_public(shout, (unsigned int)cfg_get_stream_server_public())) {
		log_error("shout_set_public: %s", shout_get_error(shout));
		goto error;
	}

	return (shout);

error:
	shout_free(shout);
	return (NULL);
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
