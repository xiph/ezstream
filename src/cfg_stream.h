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

#ifndef __CFG_STREAM_H__
#define __CFG_STREAM_H__

typedef struct cfg_stream *		cfg_stream_t;
typedef struct cfg_stream_list *	cfg_stream_list_t;

cfg_stream_list_t
	cfg_stream_list_create(void);
void	cfg_stream_list_destroy(cfg_stream_list_t *);

cfg_stream_t
	cfg_stream_list_find(cfg_stream_list_t, const char *);
cfg_stream_t
	cfg_stream_list_get(cfg_stream_list_t, const char *);

cfg_stream_t
	cfg_stream_create(const char *);
void	cfg_stream_destroy(cfg_stream_t *);

int	cfg_stream_set_name(cfg_stream_t, cfg_stream_list_t, const char *,
	    const char **);
int	cfg_stream_set_mountpoint(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_server(cfg_stream_t, cfg_stream_list_t, const char *,
	    const char **);
int	cfg_stream_set_public(cfg_stream_t, cfg_stream_list_t, const char *,
	    const char **);
int	cfg_stream_set_format(cfg_stream_t, cfg_stream_list_t, const char *,
	    const char **);
int	cfg_stream_set_encoder(cfg_stream_t, cfg_stream_list_t, const char *,
	    const char **);
int	cfg_stream_set_stream_name(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_url(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_genre(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_description(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_quality(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_bitrate(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_samplerate(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);
int	cfg_stream_set_stream_channels(cfg_stream_t, cfg_stream_list_t,
	    const char *, const char **);

int	cfg_stream_validate(cfg_stream_t, const char **);

const char *
	cfg_stream_get_name(cfg_stream_t);
const char *
	cfg_stream_get_mountpoint(cfg_stream_t);
const char *
	cfg_stream_get_server(cfg_stream_t);
int	cfg_stream_get_public(cfg_stream_t);
enum cfg_stream_format
	cfg_stream_get_format(cfg_stream_t);
const char *
	cfg_stream_get_format_str(cfg_stream_t);
const char *
	cfg_stream_get_encoder(cfg_stream_t);
const char *
	cfg_stream_get_stream_name(cfg_stream_t);
const char *
	cfg_stream_get_stream_url(cfg_stream_t);
const char *
	cfg_stream_get_stream_genre(cfg_stream_t);
const char *
	cfg_stream_get_stream_description(cfg_stream_t);
const char *
	cfg_stream_get_stream_quality(cfg_stream_t);
const char *
	cfg_stream_get_stream_bitrate(cfg_stream_t);
const char *
	cfg_stream_get_stream_samplerate(cfg_stream_t);
const char *
	cfg_stream_get_stream_channels(cfg_stream_t);

#endif /* __CFG_STREAM_H__ */
