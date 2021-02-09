/*
 * Copyright (c) 2017, 2020 Moritz Grimm <mgrimm@mrsserver.net>
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

#include <sys/queue.h>

#include <string.h>

#include "cfg_private.h"
#include "cfg_stream.h"
#include "xalloc.h"

struct cfg_stream {
	TAILQ_ENTRY(cfg_stream)  entry;
	char			*name;
	char			*mountpoint;
	char			*intake;
	char			*server;
	int			 public;
	enum cfg_stream_format	 format;
	char			*encoder;
	char			*stream_name;
	char			*stream_url;
	char			*stream_genre;
	char			*stream_description;
	char			*stream_quality;
	char			*stream_bitrate;
	char			*stream_samplerate;
	char			*stream_channels;
	char			*language_tag;
};

TAILQ_HEAD(cfg_stream_list, cfg_stream);

struct cfg_stream_list *
cfg_stream_list_create(void)
{
	struct cfg_stream_list *sl;

	sl = xcalloc(1UL, sizeof(*sl));
	TAILQ_INIT(sl);

	return (sl);
}

void
cfg_stream_list_destroy(cfg_stream_list_t *sl_p)
{
	struct cfg_stream_list	*sl = *sl_p;
	struct cfg_stream	*s;

	if (!sl)
		return;

	while (NULL != (s = TAILQ_FIRST(sl))) {
		TAILQ_REMOVE(sl, s, entry);
		cfg_stream_destroy(&s);
	}

	xfree(sl);
	*sl_p = NULL;
}

unsigned int
cfg_stream_list_nentries(struct cfg_stream_list *sl)
{
	struct cfg_stream	*s;
	unsigned int		 n = 0;

	TAILQ_FOREACH(s, sl, entry) {
		n++;
	}

	return (n);
}

struct cfg_stream *
cfg_stream_list_find(struct cfg_stream_list *sl, const char *name)
{
	struct cfg_stream	*s;

	TAILQ_FOREACH(s, sl, entry) {
		if (0 == strcasecmp(s->name, name))
			return (s);
	}

	return (NULL);
}

struct cfg_stream *
cfg_stream_list_get(struct cfg_stream_list *sl, const char *name)
{
	struct cfg_stream	*s;

	s = cfg_stream_list_find(sl, name);
	if (s)
		return (s);
	s = cfg_stream_create(name);
	if (!s)
		return (NULL);

	TAILQ_INSERT_TAIL(sl, s, entry);

	return (s);
}

void
cfg_stream_list_foreach(struct cfg_stream_list *sl,
    void (*cb)(cfg_stream_t, void *), void *cb_arg)
{
	struct cfg_stream	*s;

	TAILQ_FOREACH(s, sl, entry) {
		cb(s, cb_arg);
	}
}

struct cfg_stream *
cfg_stream_create(const char *name)
{
	struct cfg_stream	*s;

	if (!name || !name[0])
		return (NULL);

	s = xcalloc(1UL, sizeof(*s));
	s->name = xstrdup(name);

	return (s);
}

void
cfg_stream_destroy(struct cfg_stream **s_p)
{
	struct cfg_stream	*s = *s_p;

	xfree(s->name);
	xfree(s->mountpoint);
	xfree(s->intake);
	xfree(s->server);
	xfree(s->encoder);
	xfree(s->stream_name);
	xfree(s->stream_url);
	xfree(s->stream_genre);
	xfree(s->stream_description);
	xfree(s->stream_quality);
	xfree(s->stream_bitrate);
	xfree(s->stream_samplerate);
	xfree(s->stream_channels);
	xfree(s->language_tag);
	xfree(s);
	*s_p = NULL;
}

int
cfg_stream_str2fmt(const char *str, enum cfg_stream_format *fmt_p)
{
	if (0 == strcasecmp(str, CFG_SFMT_OGG)) {
		*fmt_p = CFG_STREAM_OGG;
	} else if (0 == strcasecmp(str, CFG_SFMT_MP3)) {
		*fmt_p = CFG_STREAM_MP3;
	} else if (0 == strcasecmp(str, CFG_SFMT_WEBM)) {
		*fmt_p = CFG_STREAM_WEBM;
	} else if (0 == strcasecmp(str, CFG_SFMT_MATROSKA)) {
		*fmt_p = CFG_STREAM_MATROSKA;
	} else
		return (-1);
	return (0);
}

const char *
cfg_stream_fmt2str(enum cfg_stream_format fmt)
{
	switch (fmt) {
	case CFG_STREAM_OGG:
		return (CFG_SFMT_OGG);
	case CFG_STREAM_MP3:
		return (CFG_SFMT_MP3);
	case CFG_STREAM_WEBM:
		return (CFG_SFMT_WEBM);
	case CFG_STREAM_MATROSKA:
		return (CFG_SFMT_MATROSKA);
	default:
		return (NULL);
	}
}


int
cfg_stream_set_name(struct cfg_stream *s, struct cfg_stream_list *sl,
    const char *name, const char **errstrp)
{
	struct cfg_stream	*s2;

	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	s2 = cfg_stream_list_find(sl, name);
	if (s2 && s2 != s) {
		if (errstrp)
			*errstrp = "already exists";
		return (-1);
	}

	SET_XSTRDUP(s->name, name, errstrp);

	return (0);
}

int
cfg_stream_set_mountpoint(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *mountpoint,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->mountpoint, mountpoint, errstrp);
	return (0);
}

int
cfg_stream_set_intake(struct cfg_stream *s, struct cfg_stream_list *not_used,
    const char *intake, const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->intake, intake, errstrp);
	return (0);
}

int
cfg_stream_set_server(struct cfg_stream *s, struct cfg_stream_list *not_used,
    const char *server, const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->server, server, errstrp);
	return (0);
}

int
cfg_stream_set_public(struct cfg_stream *s, struct cfg_stream_list *not_used,
    const char *public, const char **errstrp)
{
	(void)not_used;
	SET_BOOLEAN(s->public, public, errstrp);
	return (0);
}

int
cfg_stream_set_format(struct cfg_stream *s, struct cfg_stream_list *not_used,
    const char *fmt_str, const char **errstrp)
{
	enum cfg_stream_format	fmt;

	(void)not_used;

	if (!fmt_str || !fmt_str[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 > cfg_stream_str2fmt(fmt_str, &fmt)) {
		if (errstrp)
			*errstrp = "unsupported stream format";
		return (-1);
	}

	s->format = fmt;

	return (0);
}

int
cfg_stream_set_encoder(struct cfg_stream *s, struct cfg_stream_list *not_used,
    const char *encoder, const char **errstrp)
{
	(void)not_used;

	if (NULL == encoder) {
		if (s->encoder)
			xfree(s->encoder);
		s->encoder = NULL;
		return (0);
	}

	SET_XSTRDUP(s->encoder, encoder, errstrp);
	return (0);
}

int
cfg_stream_set_stream_name(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_name,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_name, stream_name, errstrp);
	return (0);
}

int
cfg_stream_set_stream_url(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_url,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_url, stream_url, errstrp);
	return (0);
}

int
cfg_stream_set_stream_genre(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_genre,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_genre, stream_genre, errstrp);
	return (0);
}

int
cfg_stream_set_stream_description(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_description,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_description, stream_description, errstrp);
	return (0);
}

int
cfg_stream_set_stream_quality(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_quality,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_quality, stream_quality, errstrp);
	return (0);
}

int
cfg_stream_set_stream_bitrate(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_bitrate,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_bitrate, stream_bitrate, errstrp);
	return (0);
}

int
cfg_stream_set_stream_samplerate(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_samplerate,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_samplerate, stream_samplerate, errstrp);
	return (0);
}

int
cfg_stream_set_stream_channels(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *stream_channels,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->stream_channels, stream_channels, errstrp);
	return (0);
}

int
cfg_stream_set_language_tag(struct cfg_stream *s,
    struct cfg_stream_list *not_used, const char *language_tag,
    const char **errstrp)
{
	(void)not_used;
	SET_XSTRDUP(s->language_tag, language_tag, errstrp);
	return (0);
}

int
cfg_stream_validate(struct cfg_stream *s, const char **errstrp)
{
	if (CFG_STREAM_INVALID == cfg_stream_get_format(s)) {
		if (NULL != errstrp)
			*errstrp = "format missing or unsupported";
		return (-1);
	}

	return (0);
}

const char *
cfg_stream_get_name(struct cfg_stream *s)
{
	return (s->name);
}

const char *
cfg_stream_get_mountpoint(struct cfg_stream *s)
{
	return (s->mountpoint);
}

const char *
cfg_stream_get_intake(struct cfg_stream *s)
{
	return (s->intake);
}

const char *
cfg_stream_get_server(struct cfg_stream *s)
{
	return (s->server);
}

int
cfg_stream_get_public(struct cfg_stream *s)
{
	return (s->public);
}

enum cfg_stream_format
cfg_stream_get_format(struct cfg_stream *s)
{
	return (s->format);
}

const char *
cfg_stream_get_format_str(struct cfg_stream *s)
{
	return (cfg_stream_fmt2str(s->format));
}

const char *
cfg_stream_get_encoder(struct cfg_stream *s)
{
	return (s->encoder);
}

const char *
cfg_stream_get_stream_name(struct cfg_stream *s)
{
	return (s->stream_name);
}

const char *
cfg_stream_get_stream_url(struct cfg_stream *s)
{
	return (s->stream_url);
}

const char *
cfg_stream_get_stream_genre(struct cfg_stream *s)
{
	return (s->stream_genre);
}

const char *
cfg_stream_get_stream_description(struct cfg_stream *s)
{
	return (s->stream_description);
}

const char *
cfg_stream_get_stream_quality(struct cfg_stream *s)
{
	return (s->stream_quality);
}

const char *
cfg_stream_get_stream_bitrate(struct cfg_stream *s)
{
	return (s->stream_bitrate);
}

const char *
cfg_stream_get_stream_samplerate(struct cfg_stream *s)
{
	return (s->stream_samplerate);
}

const char *
cfg_stream_get_stream_channels(struct cfg_stream *s)
{
	return (s->stream_channels);
}

const char *
cfg_stream_get_language_tag(struct cfg_stream *s)
{
	return (s->language_tag);
}
