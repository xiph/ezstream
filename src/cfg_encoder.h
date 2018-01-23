/*
 * Copyright (c) 2015 Moritz Grimm <mgrimm@mrsserver.net>
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

#ifndef __CFG_ENCODER_H__
#define __CFG_ENCODER_H__

#include "cfg_stream.h"

typedef struct cfg_encoder *		cfg_encoder_t;
typedef struct cfg_encoder_list *	cfg_encoder_list_t;

cfg_encoder_list_t
	cfg_encoder_list_create(void);
void	cfg_encoder_list_destroy(cfg_encoder_list_t *);
unsigned int
	cfg_encoder_list_nentries(cfg_encoder_list_t);

cfg_encoder_t
	cfg_encoder_list_find(cfg_encoder_list_t, const char *);
cfg_encoder_t
	cfg_encoder_list_get(cfg_encoder_list_t, const char *);
void	cfg_encoder_list_remove(cfg_encoder_list_t, cfg_encoder_t *o);
void	cfg_encoder_list_foreach(cfg_encoder_list_t, void (*)(cfg_encoder_t,
	    void *), void *);

cfg_encoder_t
	cfg_encoder_create(const char *);
void	cfg_encoder_destroy(cfg_encoder_t *);

int	cfg_encoder_set_format(cfg_encoder_t, enum cfg_stream_format);

int	cfg_encoder_set_name(cfg_encoder_t, cfg_encoder_list_t, const char *,
	    const char **);
int	cfg_encoder_set_format_str(cfg_encoder_t, cfg_encoder_list_t,
	    const char *, const char **);
int	cfg_encoder_set_program(cfg_encoder_t, cfg_encoder_list_t,
	    const char *, const char **);

int	cfg_encoder_validate(cfg_encoder_t, const char **);

const char *
	cfg_encoder_get_name(cfg_encoder_t);
enum cfg_stream_format
	cfg_encoder_get_format(cfg_encoder_t);
const char *
	cfg_encoder_get_program(cfg_encoder_t);

#endif /* __CFG_ENCODER_H__ */
