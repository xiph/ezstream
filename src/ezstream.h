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

#ifndef __EZSTREAM_H__
#define __EZSTREAM_H__

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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#if defined(HAVE_LIBGEN_H) && !defined(__linux__)
# include <libgen.h>
#endif /* HAVE_LIBGEN_H && !__linux__ */
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

#include "cfg.h"
#include "log.h"

#ifndef _PATH_DEVNULL
# define _PATH_DEVNULL		"/dev/null"
#endif /* !_PATH_DEVNULL */

#ifndef PATH_MAX
# define PATH_MAX		256
#endif /* !PATH_MAX */

#endif /* __EZSTREAM_H__ */
