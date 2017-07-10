/*
 * Copyright (c) 2015, 2017 Moritz Grimm <mgrimm@mrsserver.net>
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

#ifndef __CFG_PRIVATE_H__
#define __CFG_PRIVATE_H__

#include "cfg.h"

#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#define EXTENSIONS_MAX		16
#define UCREDS_SIZE		256
#define CSUITE_SIZE		2048

#define DEFAULT_PORT		8000
#define DEFAULT_USER		"source"

struct cfg {
	int				 _master_cfg;
	struct program {
		char			 name[PATH_MAX];
		enum cfg_config_type	 config_type;
		char			 config_file[PATH_MAX];
		char			 pid_file[PATH_MAX];
		int			 quiet_stderr;
		int			 rtstatus_output;
		unsigned int		 verbosity;
	} program;
	struct server {
		enum cfg_server_protocol protocol;
		char			 hostname[NI_MAXHOST];
		unsigned int		 port;
		char			 user[UCREDS_SIZE];
		char			 password[UCREDS_SIZE];
		enum cfg_server_tls	 tls;
		char			 tls_cipher_suite[CSUITE_SIZE];
		char			 ca_dir[PATH_MAX];
		char			 ca_file[PATH_MAX];
		char			 client_cert[PATH_MAX];
		unsigned int		 reconnect_attempts;
	} server;
	struct stream {
		char			*mountpoint;
		char			*name;
		char			*url;
		char			*genre;
		char			*description;
		char			*quality;
		char			*bitrate;
		char			*samplerate;
		char			*channels;
		int			 server_public;
		enum cfg_stream_format	 format;
		char			*encoder;
	} stream;
	struct media {
		enum cfg_media_type	 type;
		char			 filename[PATH_MAX];
		int			 shuffle;
		int			 stream_once;
	} media;
	struct metadata {
		char			 program[PATH_MAX];
		char			*format_str;
		int			 refresh_interval;
		int			 normalize_strings;
		int			 no_updates;
	} metadata;
};

#define SET_STRLCPY(t, s, e)	do {		\
	if (!(s) || !(s)[0]) {			\
		if ((e))			\
			*(e) = "empty"; 	\
		return (-1);			\
	}					\
	if (sizeof((t)) <=			\
	    strlcpy((t), (s), sizeof((t)))) {	\
		if ((e))			\
			*(e) = "too long";	\
		return (-1);			\
	}					\
} while (0)

#define SET_XSTRDUP(t, s, e)	do {		\
	if (!(s) || !(s)[0]) {			\
		if ((e))			\
			*(e) = "empty"; 	\
		return (-1);			\
	}					\
	if ((t))				\
		xfree((t));			\
	(t) = xstrdup((s));			\
} while (0)

#define SET_BOOLEAN(t, s, e)	do {			\
	int	val;					\
	if (!(s) || !(s)[0]) {				\
		if ((e))				\
			*(e) = "empty"; 		\
		return (-1);				\
	}						\
	if (0 == strcasecmp((s), "true") ||		\
	    0 == strcasecmp((s), "yes") ||		\
	    0 == strcasecmp((s), "1")) {		\
		val = 1;				\
	} else if (0 == strcasecmp((s), "false") ||	\
	    0 == strcasecmp((s), "no") ||		\
	    0 == strcasecmp((s), "0")) {		\
		val = 0;				\
	} else {					\
		if ((e))				\
			*(e) = "invalid";		\
		return (-1);				\
	}						\
	(t) = val;					\
} while (0)

#define SET_UINTNUM(t, s, e)	do {			\
	const char	*errstr;			\
	unsigned int	 num;				\
							\
	if (!(s) || !(s)[0]) {				\
		if ((e))				\
			*(e) = "empty"; 		\
		return (-1);				\
	}						\
							\
	num = (unsigned int)strtonum((s), 0, UINT_MAX, &errstr); \
	if (errstr) {					\
		if ((e))				\
			*(e) = errstr;			\
		return (-1);				\
	}						\
	(t) = num;					\
} while (0)

#define SET_INTNUM(t, s, e)	do {			\
	const char	*errstr;			\
	int		 num;				\
							\
	if (!(s) || !(s)[0]) {				\
		if ((e))				\
			*(e) = "empty"; 		\
		return (-1);				\
	}						\
							\
	num = (int)strtonum((s), INT_MIN, INT_MAX, &errstr); \
	if (errstr) {					\
		if ((e))				\
			*(e) = errstr;			\
		return (-1);				\
	}						\
	(t) = num;					\
} while (0)

#define CHECKPH_PROHIBITED(s, p)	do {				\
	if (NULL != strstr((s), (p))) { 				\
		if (errstrp)						\
			*errstrp = "prohibited placeholder " p; 	\
		return (-1);						\
	}								\
} while (0)

#define CHECKPH_DUPLICATE(s, p)	do {				\
	char	*c;							\
	if (NULL != (c = strstr((s), (p)))) {				\
		c += strlen((p));					\
		if (NULL != strstr(c, (p))) {				\
			if (errstrp)					\
				*errstrp = "duplicate placeholder " p;	\
			return (-1);					\
		}							\
	}								\
} while (0)

#define CHECKPH_REQUIRED(s, p)	do {				\
	if (NULL == strstr((s), (p))) { 				\
		if (errstrp)						\
			*errstrp = "missing placeholder " p;		\
		return (-1);						\
	}								\
} while (0)

#endif /* __CFG_PRIVATE_H__ */
