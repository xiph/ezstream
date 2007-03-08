/*
 * Copyright (c) 2007 Moritz Grimm <gtgbr@gmx.net>
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

#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PATH_SEPARATOR
# ifdef WIN32
#  define PATH_SEPARATOR	'\\'
# else
#  define PATH_SEPARATOR	'/'
# endif /* WIN32 */
#endif /* !PATH_SEPARATOR */

#ifndef PATH_MAX
# define PATH_MAX	256
#endif /* !PATH_MAX */

/* Sometimes defined through <limits.h>. */
#ifndef SIZE_T_MAX
# define SIZE_T_MAX	((size_t)-1)
#endif /* !SIZE_T_MAX */

#ifdef WIN32
# define _PATH_DEVNULL	"nul"

# define pclose 	_pclose
# define popen		_popen
# define snprintf	_snprintf
# define stat		_stat
# define strncasecmp	strnicmp
# define strtoll	_strtoi64

# define S_IRGRP	0
# define S_IROTH	0
# define S_IWGRP	0
# define S_IWOTH	0
# define S_IXGRP	0
# define S_IXOTH	0

# define basename	local_basename
# define sleep(a)	Sleep((a) * 1000)
#endif /* WIN32 */

/* Usually defined in <sys/stat.h>. */
#ifndef S_IEXEC
# define S_IEXEC	S_IXUSR
#endif /* !S_IEXEC */

/* For Solaris, possibly others (usually defined in <paths.h>.) */
#ifndef _PATH_DEVNULL
# define _PATH_DEVNULL	"/dev/null"
#endif /* !_PATH_DEVNULL */

char *	local_basename(const char *);

#endif /* __COMPAT_H__ */
