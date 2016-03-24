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

#ifndef __STREAM_H__
#define __STREAM_H__

#include <shout/shout.h>

#include "metadata.h"

#define STREAM_DEFAULT	"default"

typedef struct stream *	stream_t;

int	stream_init(void);
void	stream_exit(void);

stream_t
	stream_get(const char *);
int	stream_setup(stream_t);
int	stream_set_metadata(stream_t, metadata_t, char **);
char *	stream_get_metadata_str(const char *, metadata_t);

shout_t *
	stream_get_shout(stream_t);

#endif /* __STREAM_H__ */
