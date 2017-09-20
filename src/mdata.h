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

#ifndef __MDATA_H__
#define __MDATA_H__

typedef struct mdata * mdata_t;

mdata_t mdata_create(void);
void	mdata_destroy(mdata_t *);

void	mdata_set_normalize_strings(mdata_t, int);

int	mdata_parse_file(mdata_t, const char *);
int	mdata_run_program(mdata_t, const char *);

int	mdata_refresh(mdata_t);

const char *
	mdata_get_filename(mdata_t);
const char *
	mdata_get_name(mdata_t);
const char *
	mdata_get_artist(mdata_t);
const char *
	mdata_get_album(mdata_t);
const char *
	mdata_get_title(mdata_t);
const char *
	mdata_get_songinfo(mdata_t);

int	mdata_get_length(mdata_t);

int	mdata_strformat(mdata_t, char *, size_t, const char *);

#endif /* __MDATA_H__ */
