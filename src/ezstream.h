/*	$Id$	*/
/*
 * Copyright (c) 2007, 2009 Moritz Grimm <mgrimm@mrsserver.net>
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

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#include <time.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifdef HAVE_PATHS_H
# include <paths.h>
#endif /* HAVE_PATHS_H */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef STDIN_FILENO
# define STDIN_FILENO		0
#endif /* !STDIN_FILENO */

#ifndef _PATH_DEVNULL
# ifdef WIN32
#  define _PATH_DEVNULL 	"nul"
# else /* WIN32 */
#  define _PATH_DEVNULL 	"/dev/null"
# endif /* WIN32 */
#endif /* !_PATH_DEVNULL */

#ifndef PATH_MAX
# define PATH_MAX		256
#endif /* !PATH_MAX */

#if !defined(HAVE_PCLOSE) && defined(HAVE__PCLOSE)
# define pclose 	_pclose
#endif /* !HAVE_PCLOSE && HAVE__PCLOSE */
#if !defined(HAVE_POPEN) && defined(HAVE__POPEN)
# define popen		_popen
#endif /* !HAVE_POPEN && HAVE__POPEN */
#if !defined(HAVE_SNPRINTF) && defined(HAVE__SNPRINTF)
# define snprintf	_snprintf
#endif /* !HAVE_SNPRINTF && HAVE__SNPRINTF */
#if !defined(HAVE_STAT) && defined(HAVE__STAT)
# define stat		_stat
#endif /* !HAVE_STAT && HAVE__STAT */
#if !defined(HAVE_STRNCASECMP) && defined(HAVE_STRNICMP)
# define strncasecmp	strnicmp
#endif /* !HAVE_STRNCASECMP && HAVE_STRNICMP */
#if !defined(HAVE_STRTOLL) && defined(HAVE__STRTOI64)
# define strtoll	_strtoi64
#endif /* !HAVE_STRTOLL && HAVE__STRTOI64 */

#ifndef S_IRGRP
# define S_IRGRP	0
# define S_IWGRP	0
# define S_IXGRP	0
#endif /* !S_IRGRP */
#ifndef S_IROTH
# define S_IROTH	0
# define S_IWOTH	0
# define S_IXOTH	0
#endif /* !S_IROTH */

#ifdef WIN32
# include <windows.h>
# define sleep(a)	Sleep((a) * 1000)
#endif /* WIN32 */

/*
 * For compat.c and getopt.c:
 */

extern int	 opterr;
extern int	 optind;
extern int	 optopt;
extern int	 optreset;
extern char	*optarg;

extern int
	local_getopt(int, char * const *, const char *);

extern const char *path_separators;

extern char *
	local_basename(const char *);

#endif /* __COMPAT_H__ */
