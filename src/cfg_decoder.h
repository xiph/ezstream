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

#ifndef __CFG_INPUT_H__
#define __CFG_INPUT_H__

typedef struct cfg_decoder *		cfg_decoder_t;
typedef struct cfg_decoder_list *	cfg_decoder_list_t;

cfg_decoder_list_t
	cfg_decoder_list_create(void);
void	cfg_decoder_list_destroy(cfg_decoder_list_t *);

cfg_decoder_t
	cfg_decoder_list_find(cfg_decoder_list_t, const char *);
cfg_decoder_t
	cfg_decoder_list_findext(cfg_decoder_list_t, const char *);
cfg_decoder_t
	cfg_decoder_list_get(cfg_decoder_list_t, const char *);

cfg_decoder_t
	cfg_decoder_create(const char *);
void	cfg_decoder_destroy(cfg_decoder_t *);

int	cfg_decoder_set_name(cfg_decoder_t, cfg_decoder_list_t, const char *,
	    const char **);
int	cfg_decoder_set_program(cfg_decoder_t, cfg_decoder_list_t,
	    const char *, const char **);
int	cfg_decoder_add_match(cfg_decoder_t, cfg_decoder_list_t, const char *,
	    const char **);

int	cfg_decoder_validate(cfg_decoder_t, const char **);

int	cfg_decoder_extsupport(cfg_decoder_t, const char *);

const char *
	cfg_decoder_get_name(cfg_decoder_t);
const char *
	cfg_decoder_get_program(cfg_decoder_t);

#endif /* __CFG_INPUT_H__ */
