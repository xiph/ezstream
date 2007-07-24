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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#ifndef SIZE_T_MAX
# define SIZE_T_MAX	((size_t)-1)
#endif

extern char	*__progname;

void *
xmalloc(size_t size)
{
	void	*ret;

	if (size == 0) {
		printf("%s: xmalloc(): Internal error: zero size\n",
		       __progname);
		abort();
	}

	if ((ret = malloc(size)) == NULL) {
		printf("%s: xmalloc(): Allocating %lu bytes: %s\n",
		       __progname,  (unsigned long)(size), strerror(errno));
		exit(1);
	}

	return (ret);
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void	*ret;

	if (nmemb == 0 || size == 0) {
		printf("%s: xcalloc(): Internal error: zero size\n",
		       __progname);
		abort();
	}

	if (SIZE_T_MAX / nmemb < size) {
		printf("%s: xcalloc(): Integer overflow: nmemb * size > SIZE_T_MAX\n",
		       __progname);
		exit(1);
	}
 
	if ((ret = calloc(nmemb, size)) == NULL) {
		printf("%s: xcalloc(): Allocating %lu bytes: %s\n",
		       __progname,  (unsigned long)(nmemb * size), strerror(errno));
		exit(1);
	}

	return (ret);
}

void *
xrealloc(void *ptr, size_t nmemb, size_t size)
{
	void	*ret;
	size_t	 nsiz = nmemb * size;

	if (nmemb == 0 || size == 0) {
		printf("%s: xrealloc(): Internal error: zero size\n",
		       __progname);
		abort();
	}

	if (SIZE_T_MAX / nmemb < size) {
		printf("%s: xrealloc(): Integer overflow: nmemb * size > SIZE_T_MAX\n",
		       __progname);
		exit(1);
	}

	if (ptr == NULL)
		ret = malloc(nsiz);
	else
		ret = realloc(ptr, nsiz);

	if (ret == NULL) {
		printf("%s: xrealloc(): (Re)allocating %lu bytes: %s\n",
		       __progname,  (unsigned long)(nmemb * size), strerror(errno));
		exit(1);
	}

	return (ret);
}

char *
xstrdup(const char *str)
{
	size_t	 len;
	char	*nstr;

	len = strlen(str) + 1;
	nstr = xcalloc(len, sizeof(char));
	memcpy(nstr, str, len);
	return (nstr);
}

int
strrcmp(const char *s, const char *sub)
{
	size_t	slen = strlen(s);
	size_t	sublen = strlen(sub);

	if (sublen > slen)
		return (1);

	return (memcmp(s + slen - sublen, sub, sublen));
}
