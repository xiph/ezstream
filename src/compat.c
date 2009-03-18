/*	$Id$	*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ezstream.h"

#if defined(HAVE_LIBGEN_H) && !defined(__linux__)
# include <libgen.h>
#endif /* HAVE_LIBGEN_H && !__linux__ */

#ifndef PATH_SEPARATORS
# define PATH_SEPARATORS	"/"
#endif /* !PATH_SEPARATORS */

const char	*path_separators = PATH_SEPARATORS;

char *	local_basename(const char *);

static inline int
	is_separator(int);

static inline int
is_separator(int c)
{
	const char	*cp;

	for (cp = path_separators; '\0' != *cp; cp++) {
		if (*cp == c)
			return (1);
	}

	return (0);
}

/*
 * Modified basename() implementation from OpenBSD, based on:
 * $OpenBSD: src/lib/libc/gen/basename.c,v 1.14 2005/08/08 08:05:33 espie Exp $
 */
/*
 * Copyright (c) 1997, 2004 Todd C. Miller <Todd.Miller@courtesan.com>
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
char *
local_basename(const char *path)
{
#ifdef HAVE_BASENAME
	return (basename(path));
#else /* HAVE_BASENAME */
	static char	 bname[PATH_MAX];
	size_t		 len;
	const char	*startp, *endp;

	if (path == NULL || *path == '\0') {
		bname[0] = '.';
		bname[1] = '\0';
		return (bname);
	}

	/* Strip any trailing path separators */
	endp = path + strlen(path) - 1;
	while (endp > path && is_separator(*endp))
		endp--;

	/*
	 * All path separators become a single one; pick the first in the
	 * list as the default.
	 */
	if (endp == path && is_separator(*endp)) {
		bname[0] = path_separators[0];
		bname[1] = '\0';
		return (bname);
	}

	/* Find the start of the base */
	startp = endp;
	while (startp > path && !is_separator(*(startp - 1)))
		startp--;

	len = endp - startp + 1;
	if (len >= sizeof(bname)) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	memcpy(bname, startp, len);
	bname[len] = '\0';

	return (bname);
#endif /* HAVE_BASENAME */
}
