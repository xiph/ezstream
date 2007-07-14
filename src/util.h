/*
 * Copyright (c) 2007 Moritz Grimm <mdgrimm@gmx.net>
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

#ifndef __UTIL_H__
#define __UTIL_H__

void *	xmalloc(size_t /* size */);
void *	xcalloc(size_t /* nmemb */, size_t /* size */);
void *	xrealloc(void *, size_t /* nmemb */, size_t /* size */);
char *	xstrdup(const char *);
int	strrcmp(const char *, const char *);

#define xfree(ptr)	do {						\
	if ((ptr) == NULL) {						\
		printf("%s: xfree(): Internal error: NULL argument\n",	\
		       __progname);					\
		abort();						\
	}								\
	free(ptr);							\
	(ptr) = NULL;							\
} while (0)

#endif /* __UTIL_H__ */
