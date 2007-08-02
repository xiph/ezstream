/*
 * libxalloc - Portable memory allocation wrapper library, with extensive
 *             error checking, debugging facilities and hooks for 3rd party
 *             memory allocation functions.
 *             This library also detects and prevents double-free errors,
 *             and ensures that out-of-memory issues always cause the
 *             application to exit.
 *
 * Copyright (C) 2007  Moritz Grimm <mgrimm@gmx.net>
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

#ifndef __XALLOC_H__
#define __XALLOC_H__

/*
 * Define XALLOC_DEBUG to compile the debugging features. Doing so will make
 * this library more expensive in every case, but not change its (visible)
 * behavior unless the debugging level is set > 0. The debugging levels are:
 *   0: disable debugging
 *   1: enable most debugging features
 *   2: additionally enable double-free checking
 *      (Warning: This requires libxalloc to keep track of all allocations
 *                and frees, which means that memory usage may increase a lot
 *                over time!)
 *
 * Define XALLOC_SILENT to suppress all messages, which makes libxalloc
 * abort() and exit() silently. This has no effect when THREAD_DEBUG is
 * defined.
 *
 * Define XALLOC_WITH_XASPRINTF to expose the xasprintf() interface. Doing
 * so will require libxalloc to be compiled with a compiler that supports C99
 * variadic macros, and work only on systems with vasprintf() or vsnprintf(),
 * and MS Windows. Note that doing so constitutes an incompatible ABI change!
 *
 * Note that none of the x*_c() functions should be used directly, unless it
 * is ensured that /file/ is a const char * to a real string constant.
 */
/* #define XALLOC_DEBUG 1 */
/* #define XALLOC_SILENT 1 */
/* #define XALLOC_WITH_XASPRINTF 1 */

/* The default output stream for messages: */
#define XALLOC_DEFAULT_OUTPUT	stderr

#if (defined(_REENTRANT) || defined(_POSIX_THREADS)) && !defined(THREAD_SAFE)
# define THREAD_SAFE		1
#endif


/*
 * Library initialization and shutdown.
 */

#define xalloc_initialize()						\
	xalloc_initialize_debug(0, NULL)
void	xalloc_initialize_debug(unsigned int /* level */,
				FILE * /* output stream */);

void	xalloc_set_functions(void *(*)(size_t) /* malloc function */,
			     void *(*)(size_t, size_t) /* calloc function */,
			     void *(*)(void *, size_t) /* realloc function */);

/* Memory leak checks happen during shutdown! */
void	xalloc_shutdown(void);


/*
 * Memory management functions.
 * Note that xrealloc() has calloc() semantics, to detect and prevent integer
 * overflows.
 */

#define xmalloc(s)							\
	xmalloc_c(s, __FILE__, __LINE__)
void *	xmalloc_c(size_t /* size */,
		  const char * /* file */, unsigned int /* line */);

#define xcalloc(n, s)							\
	xcalloc_c(n, s, 0, __FILE__, __LINE__)
void *	xcalloc_c(size_t /* nmemb */, size_t /* size */, int /* may fail */,
		  const char * /* file */, unsigned int /* line */);

#define xrealloc(p, n, s)						\
	xrealloc_c(p, n, s, __FILE__, __LINE__)
void *	xrealloc_c(void *, size_t /* nmemb */, size_t /* size */,
		   const char * /* file */, unsigned int /* line */);

#define xstrdup(s)							\
	xstrdup_c(s, __FILE__, __LINE__)
char *	xstrdup_c(const char *,
		  const char * /* file */, unsigned int /* line */);

#define xfree(p)							\
	xfree_c((void *)&(p), __FILE__, __LINE__)
void	xfree_c(void **,
		const char * /* file */, unsigned int /* line */);

#ifdef XALLOC_WITH_XASPRINTF
# define xasprintf(s, f, ...)						\
	xasprintf_c(__FILE__, __LINE__, s, f, __VA_ARGS__)
int	xasprintf_c(const char * /* file */, unsigned int /* line */,
		    char ** /* string pointer */, const char * /* format */,
		    ...);
#endif /* XALLOC_WITH_XASPRINTF */

#endif /* __XALLOC_H__ */
