/*
 * Copyright (C) 2007  Moritz Grimm <mdgrimm@gmx.net>
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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xalloc.h"

#ifndef SIZE_T_MAX
# define SIZE_T_MAX		((size_t)-1)
#endif
#ifndef va_copy
# define va_copy(d, s)		(d) = (s)
#endif
#define XALLOC_DBGLVL_MAX	2

#if defined(XALLOC_DEBUG) && defined(XALLOC_SILENT)
# undef XALLOC_SILENT
#endif /* XALLOC_DEBUG && XALLOC_SILENT */

#ifdef THREAD_SAFE
# include <pthread.h>
static pthread_mutex_t	 xalloc_mutex;
static pthread_mutex_t	 strerror_mutex;
# define XALLOC_LOCK(mtx) do {						\
	int error;							\
	if ((error = pthread_mutex_lock(&mtx)) != 0)			\
		_xalloc_error(error, "XALLOC: Internal error in %s:%u: pthread_mutex_lock()", \
			      __FILE__, __LINE__);			\
} while (0)
# define XALLOC_UNLOCK(mtx) do {					\
	int error;							\
	if ((error = pthread_mutex_unlock(&mtx)) != 0)			\
		_xalloc_error(error, "XALLOC: Internal error in %s:%u: pthread_mutex_unlock()", \
			      __FILE__, __LINE__);			\
} while (0)
#else
# define XALLOC_LOCK(mtx)	((void)0)
# define XALLOC_UNLOCK(mtx)	((void)0)
#endif /* THREAD_SAFE */

#ifdef XALLOC_DEBUG
# include <sys/tree.h>

int	_memory_cmp(void *, void *);

struct memory {
	RB_ENTRY(memory) entry;
	void		*ptr;
	unsigned int	 id;
	size_t		 size;
	const char	*allocated_by;
	unsigned int	 allocated_in_line;
	const char	*reallocated_by;
	unsigned int	 reallocated_in_line;
	const char	*freed_by;
	unsigned int	 freed_in_line;
};
RB_HEAD(memory_tree, memory) memory_tree_head = RB_INITIALIZER(&memory_tree_head);
RB_PROTOTYPE(memory_tree, memory, entry, _memory_cmp)

void	_memory_free(struct memory **);
#endif /* XALLOC_DEBUG */

void	_xalloc_warn(const char *, ...);
void	_xalloc_error(int, const char *, ...);
void	_xalloc_fatal(const char *, ...);
void	_xalloc_debug_printf(unsigned int, const char *, ...);
#ifdef XALLOC_WITH_XASPRINTF
int	_xalloc_vasprintf(char **, const char *, va_list, size_t *);
#endif /* XALLOC_WITH_XASPRINTF */

static unsigned int	  debug_level = 0;
static FILE		 *debug_output = NULL;
#ifdef XALLOC_DEBUG
static unsigned int	  xalloc_next_id = 0;
#endif
static int		  xalloc_initialized = 0;
static size_t		  xalloc_allocated;
static size_t		  xalloc_total;
static size_t		  xalloc_peak;
static size_t		  xalloc_freed;
static void *		(*real_malloc)(size_t) = NULL;
static void *		(*real_calloc)(size_t, size_t) = NULL;
static void *		(*real_realloc)(void *, size_t) = NULL;
static void		(*real_free)(void *) = NULL;
static const char	 *unknown_file = "<unknown>";

#ifdef XALLOC_DEBUG
RB_GENERATE(memory_tree, memory, entry, _memory_cmp)

int
_memory_cmp(void *arg_a, void *arg_b)
{
	struct memory	*a = (struct memory *)arg_a;
	struct memory	*b = (struct memory *)arg_b;

	if (a->ptr < b->ptr)
		return (-1);
	else if (a->ptr > b->ptr)
		return (1);
	return (0);
}

void
_memory_free(struct memory **mem_p)
{
	struct memory	*mem = *mem_p;

	if (mem->allocated_by != NULL)
		mem->allocated_by = NULL;
	if (mem->reallocated_by != NULL)
		mem->reallocated_by = NULL;
	if (mem->freed_by != NULL)
		mem->freed_by = NULL;
	real_free(mem);
	*mem_p = NULL;
}
#endif /* XALLOC_DEBUG */

void
_xalloc_warn(const char *fmt, ...)
{
	va_list ap;

	if (debug_output == NULL)
		debug_output = XALLOC_DEFAULT_OUTPUT;

	va_start(ap, fmt);
#ifndef XALLOC_SILENT
	vfprintf(debug_output, fmt, ap);
	fflush(debug_output);
#endif /* !XALLOC_SILENT */
	va_end(ap);
}

void
_xalloc_error(int errnum, const char *fmt, ...)
{
	va_list ap;

	if (debug_output == NULL)
		debug_output = XALLOC_DEFAULT_OUTPUT;

	va_start(ap, fmt);
#ifndef XALLOC_SILENT
	vfprintf(debug_output, fmt, ap);
	if (errnum > 0) {
		if (xalloc_initialized)
			XALLOC_LOCK(strerror_mutex);
		vfprintf(debug_output, ": %s\n", strerror(errnum));
		if (xalloc_initialized)
			XALLOC_UNLOCK(strerror_mutex);
	}
	fflush(debug_output);
#endif /* !XALLOC_SILENT */
	va_end(ap);

	exit(1);
}

void
_xalloc_fatal(const char *fmt, ...)
{
	va_list ap;

	if (debug_output == NULL)
		debug_output = XALLOC_DEFAULT_OUTPUT;

	va_start(ap, fmt);
#ifndef XALLOC_SILENT
	vfprintf(debug_output, fmt, ap);
	fflush(debug_output);
#endif /* !XALLOC_SILENT */
	va_end(ap);

	abort();
}

void
_xalloc_debug_printf(unsigned int level, const char *fmt, ...)
{
	va_list ap;

	if (level > debug_level)
		return;

	va_start(ap, fmt);
#ifdef XALLOC_DEBUG
	vfprintf(debug_output, fmt, ap);
	fflush(debug_output);
#endif /* XALLOC_DEBUG */
	va_end(ap);
}

#ifdef XALLOC_WITH_XASPRINTF
int
_xalloc_vasprintf(char **str_p, const char *fmt, va_list ap, size_t *strsiz)
{
	int	ret = -1;
	va_list ap_local;

	*str_p = NULL;
#ifndef WIN32
# ifndef HAVE_BROKEN_VSNPRINTF

	/* MODERN UNIX */

	va_copy(ap_local, ap);
	*strsiz = vsnprintf(NULL, (size_t)0, fmt, ap_local) + 1;
	va_end(ap_local);
#  ifdef HAVE_ASPRINTF
	if ((ret = vasprintf(str_p, fmt, ap)) == -1)
		*str_p = NULL;
#  else
	if ((*str_p = real_calloc(*strsiz, sizeof(char))) == NULL)
		return (-1);
	ret = vsnprintf(*str_p, *strsiz, fmt, ap);
#  endif /* HAVE_ASPRINTF */
# else

	/* ANCIENT UNIX */

	{
		char	*buf = NULL;

		*strsiz = 4;
		for (;;) {
			char	*tbuf;
			int	 pret;

			if ((tbuf = real_realloc(buf, *strsiz)) == NULL) {
				real_free(buf);
				return (-1);
			}
			buf = tbuf;
			va_copy(ap_local, ap);
			pret = vsnprintf(buf, *strsiz, fmt, ap_local);
			va_end(ap_local);
			if (pret > 0 && pret < (int)*strsiz)
				break;
			if ((int)(*strsiz *= 2) < 0) {
				real_free(buf);
				return (-1);
			}
		}
		ret = vsnprintf(buf, *strsiz, fmt, ap);
		*str_p = buf;
	}
# endif /* !HAVE_BROKEN_VSNPRINTF */
#else

	/* WINDOWS */

	va_copy(ap_local, ap);
	*strsiz = _vscprintf(fmt, ap_local) + 1;
	va_end(ap_local);
	if ((*str_p = real_calloc(*strsiz, sizeof(char))) == NULL)
		return (-1);
	ret = _vsnprintf(*str_p, *strsiz, fmt, ap);
#endif /* !WIN32 */

	return (ret);
}
#endif /* XALLOC_WITH_XASPRINTF */

void
xalloc_initialize_debug(unsigned int level, FILE *output)
{
#ifdef THREAD_SAFE
	int	err;
#endif /* THREAD_SAFE */

	if (xalloc_initialized)
		_xalloc_fatal("XALLOC: xalloc_initialize(): Xalloc library already initialized\n");

	if ((debug_level = level) > XALLOC_DBGLVL_MAX)
		debug_level = XALLOC_DBGLVL_MAX;
	if (output == NULL)
		debug_output = XALLOC_DEFAULT_OUTPUT;
	else
		debug_output = output;

	real_malloc = malloc;
	real_calloc = calloc;
	real_realloc = realloc;
	real_free = free;
	xalloc_allocated = 0;
	xalloc_total = 0;
	xalloc_peak = 0;
	xalloc_freed = 0;

#ifdef THREAD_SAFE
	if ((err = pthread_mutex_init(&strerror_mutex, NULL)) != 0)
		_xalloc_error(err, "XALLOC: xalloc_initialize(): Initializing xalloc_mutex");
	if ((err = pthread_mutex_init(&xalloc_mutex, NULL)) != 0)
		_xalloc_error(err, "XALLOC: xalloc_initialize(): Initializing strerror_mutex");
#endif /* THREAD_SAFE */

	xalloc_initialized = 1;
}

void
xalloc_set_functions(void *(*malloc_func)(size_t),
		     void *(*calloc_func)(size_t, size_t),
		     void *(*realloc_func)(void *, size_t),
		     void (*free_func)(void *))
{
	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xalloc_set_functions(): Xalloc library not initialized\n");

	if (malloc_func == NULL ||
	    calloc_func == NULL ||
	    realloc_func == NULL)
		_xalloc_fatal("XALLOC: xalloc_set_functions(): Bad argument(s)\n");

	XALLOC_LOCK(xalloc_mutex);
	real_malloc = malloc_func;
	real_calloc = calloc_func;
	real_realloc = realloc_func;
	real_free = free_func;
	XALLOC_UNLOCK(xalloc_mutex);
}

void
xalloc_shutdown(void)
{
	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xalloc_shutdown(): Xalloc library not initialized\n");

#ifdef XALLOC_DEBUG
	if (debug_level > 0) {
		struct memory	*mem, *mem_next;
		size_t		 leaked_bytes = 0;

		XALLOC_LOCK(xalloc_mutex);

		for (mem = RB_MIN(memory_tree, &memory_tree_head);
		     mem != NULL;
		     mem = mem_next) {
			mem_next = RB_NEXT(memory_tree, &memory_tree_head, mem);
			RB_REMOVE(memory_tree, &memory_tree_head, mem);

			if (mem->freed_by == NULL) {
				_xalloc_debug_printf(1, "XALLOC: MEMORY LEAK (%p:%u): allocated in %s:%u, ",
						     mem->ptr,
						     mem->id,
						     mem->allocated_by,
						     mem->allocated_in_line);
				if (mem->reallocated_by != NULL)
					_xalloc_debug_printf(1, "last reallocated in %s:%u, ",
							     mem->reallocated_by,
							     mem->reallocated_in_line);
				_xalloc_debug_printf(1, "leaks %lu bytes\n",
						     (unsigned long)mem->size);
				leaked_bytes += mem->size;
				real_free(mem->ptr);
			}

			_memory_free(&mem);
		}

		if (leaked_bytes != xalloc_allocated)
			_xalloc_fatal("XALLOC: Internal error: xalloc_shutdown(): leaked_bytes(%lu) != xalloc_allocated(%lu)\n",
				      (unsigned long)leaked_bytes,
				      (unsigned long)xalloc_allocated);

		_xalloc_debug_printf(1, "XALLOC: STATS: leaked: %lu bytes, peak allocation: %lu bytes (freed/total: %lu/%lu bytes)\n",
				     (unsigned long)xalloc_allocated,
				     (unsigned long)xalloc_peak,
				     (unsigned long)xalloc_freed,
				     (unsigned long)xalloc_total);

		XALLOC_UNLOCK(xalloc_mutex);
	}
#endif /* XALLOC_DEBUG */

#ifdef THREAD_SAFE
	if (pthread_mutex_destroy(&xalloc_mutex) != 0)
		_xalloc_fatal("XALLOC: Internal error: xalloc_shutdown(): xalloc_mutex %p cannot be destroyed\n",
			      xalloc_mutex);
	if (pthread_mutex_destroy(&strerror_mutex) != 0)
		_xalloc_fatal("XALLOC: Internal error: xalloc_shutdown(): strerror_mutex %p cannot be destroyed\n",
			      strerror_mutex);
#endif /* THREAD_SAFE */

	xalloc_initialized = 0;
}

void *
xmalloc_c(size_t size, const char *file, unsigned int line)
{
	void		*ret;

	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xmalloc(): Xalloc library not initialized\n");

	if (size == 0)
		_xalloc_fatal("XALLOC: xmalloc(): %s:%u: Zero size\n",
			      file ? file : unknown_file, line);

	if ((ret = real_malloc(size)) == NULL)
		_xalloc_error(errno, "XALLOC: xmalloc(): %s:%u: Allocating %lu bytes",
			      file ? file : unknown_file, line,
			      (unsigned long)(size));

#ifdef XALLOC_DEBUG
	if (debug_level > 0) {
		struct memory	*mem, *mem_exists;

		if ((mem = real_calloc(1, sizeof(struct memory))) == NULL)
			_xalloc_error(errno, "XALLOC: Internal error");
		mem->ptr = ret;
		mem->size = size;
		if (file)
			mem->allocated_by = file;
		else
			mem->allocated_by = unknown_file;
		mem->allocated_in_line = line;
		XALLOC_LOCK(xalloc_mutex);
		mem->id = ++xalloc_next_id;
		if ((mem_exists = RB_INSERT(memory_tree, &memory_tree_head, mem)) != NULL) {
			/* Freed pointer is being reused: */
			if (mem_exists->id != 0)
				_xalloc_fatal("XALLOC: Internal error: Assertion (mem_exists->id == 0) in %s:%u failed!\n",
					      __FILE__, __LINE__);
			RB_REMOVE(memory_tree, &memory_tree_head, mem_exists);
			_memory_free(&mem_exists);
			RB_INSERT(memory_tree, &memory_tree_head, mem);
		}
		xalloc_allocated += size;
		xalloc_total += size;
		if (xalloc_allocated > xalloc_peak)
			xalloc_peak = xalloc_allocated;
		XALLOC_UNLOCK(xalloc_mutex);
	}
#endif /* XALLOC_DEBUG */

	return (ret);
}

void *
xcalloc_c(size_t nmemb, size_t size, int may_fail,
	  const char *file, unsigned int line)
{
	void		*ret;

	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xcalloc(): Xalloc library not initialized\n");

	if (nmemb == 0 || size == 0)
		_xalloc_fatal("XALLOC: xcalloc(): %s:%u: Zero size\n",
			      file ? file : unknown_file, line);

	if (SIZE_T_MAX / nmemb < size)
		_xalloc_fatal("XALLOC: xcalloc(): %s:%u: Integer overflow (nmemb * size > SIZE_T_MAX)\n",
			      file ? file : unknown_file, line);
 
	if ((ret = real_calloc(nmemb, size)) == NULL && may_fail == 0)
		_xalloc_error(errno, "XALLOC: xcalloc(): %s:%u: Allocating %lu bytes",
			      file ? file : unknown_file, line,
			      (unsigned long)(nmemb * size));

#ifdef XALLOC_DEBUG
	if (ret != NULL && debug_level > 0) {
		struct memory	*mem, *mem_exists;

		if ((mem = real_calloc(1, sizeof(struct memory))) == NULL)
			_xalloc_error(errno, "XALLOC: Internal error");
		mem->ptr = ret;
		mem->size = nmemb * size;
		if (file)
			mem->allocated_by = file;
		else
			mem->allocated_by = unknown_file;
		mem->allocated_in_line = line;
		XALLOC_LOCK(xalloc_mutex);
		mem->id = ++xalloc_next_id;
		if ((mem_exists = RB_INSERT(memory_tree, &memory_tree_head, mem)) != NULL) {
			/* Freed pointer is being reused: */
			if (mem_exists->id != 0)
				_xalloc_fatal("XALLOC: Internal error: Assertion (mem_exists->id == 0) in %s:%u failed!\n",
					      __FILE__, __LINE__);
			RB_REMOVE(memory_tree, &memory_tree_head, mem_exists);
			_memory_free(&mem_exists);
			RB_INSERT(memory_tree, &memory_tree_head, mem);
		}
		xalloc_allocated += nmemb * size;
		xalloc_total += nmemb * size;
		if (xalloc_allocated > xalloc_peak)
			xalloc_peak = xalloc_allocated;
		XALLOC_UNLOCK(xalloc_mutex);
	}
#endif /* XALLOC_DEBUG */

	return (ret);
}

void *
xrealloc_c(void *ptr, size_t nmemb, size_t size,
	   const char *file, unsigned int line)
{
	void		*ret;
	size_t		 nsiz = nmemb * size;
#ifdef XALLOC_DEBUG
	struct memory	*mem = NULL;
#endif /* XALLOC_DEBUG */

	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xrealloc(): Xalloc library not initialized\n");

	if (nmemb == 0 || size == 0)
		_xalloc_fatal("XALLOC: xrealloc(): %s:%u: Zero size\n",
			      file ? file : unknown_file, line);

	if (SIZE_T_MAX / nmemb < size)
		_xalloc_fatal("XALLOC: xrealloc(): %s:%u: Integer overflow (nmemb * size > SIZE_T_MAX)\n",
			      file ? file : unknown_file, line);

	if (ptr == NULL) {
		ret = real_malloc(nsiz);
#ifdef XALLOC_DEBUG
		if (debug_level > 0) {
			if ((mem = real_calloc(1, sizeof(struct memory))) == NULL)
				_xalloc_error(errno, "XALLOC: Internal error");
			mem->ptr = ret;
			XALLOC_LOCK(xalloc_mutex);
			mem->id = ++xalloc_next_id;
			XALLOC_UNLOCK(xalloc_mutex);
			if (file)
				mem->allocated_by = file;
			else
				mem->allocated_by = unknown_file;
			mem->allocated_in_line = line;
		}
#endif /* XALLOC_DEBUG */
	} else {
#ifdef XALLOC_DEBUG
		struct memory	find_mem;

		XALLOC_LOCK(xalloc_mutex);
		if (debug_level > 0) {
			find_mem.ptr = ptr;
			if ((mem = RB_FIND(memory_tree, &memory_tree_head, &find_mem)) == NULL)
				_xalloc_fatal("XALLOC: xrealloc(): %s:%u: Junk pointer %p not accounted for\n",
					      file ? file : unknown_file,
					      line, ptr);
			RB_REMOVE(memory_tree, &memory_tree_head, mem);
		}
		XALLOC_UNLOCK(xalloc_mutex);
#endif /* XALLOC_DEBUG */
		ret = real_realloc(ptr, nsiz);
#ifdef XALLOC_DEBUG
		if (debug_level > 0) {
			mem->ptr = ret;
			if (file)
				mem->reallocated_by = file;
			else
				mem->reallocated_by = unknown_file;
			mem->reallocated_in_line = line;
		}
#endif /* XALLOC_DEBUG */
	}

	if (ret == NULL)
		_xalloc_error(errno, "XALLOC: xrealloc(): %s:%u: (Re)allocating %lu bytes",
			      file ? file : unknown_file, line,
			      (unsigned long)(nmemb * size));

#ifdef XALLOC_DEBUG
	if (debug_level > 0) {
		struct memory	*mem_exists;
		ssize_t 	 diff = nsiz - mem->size;

		XALLOC_LOCK(xalloc_mutex);
		xalloc_allocated += diff;
		if (diff < 0)
			xalloc_freed += -diff;
		else
			xalloc_total += diff;
		if (xalloc_allocated > xalloc_peak)
			xalloc_peak = xalloc_allocated;
		mem->size = nsiz;
		if ((mem_exists = RB_INSERT(memory_tree, &memory_tree_head, mem)) != NULL) {
			/* Freed pointer is being reused: */
			if (mem_exists->id != 0)
				_xalloc_fatal("XALLOC: Internal error: Assertion (mem_exists->id == 0) in %s:%u failed!\n",
					      __FILE__, __LINE__);
			RB_REMOVE(memory_tree, &memory_tree_head, mem_exists);
			_memory_free(&mem_exists);
			RB_INSERT(memory_tree, &memory_tree_head, mem);
		}
		XALLOC_UNLOCK(xalloc_mutex);
	}
#endif /* XALLOC_DEBUG */

	return (ret);
}

char *
xstrdup_c(const char *str, const char *file, unsigned int line)
{
	size_t	 len;
	char	*nstr;

	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xstrdup(): Xalloc library not initialized\n");

	len = strlen(str) + 1;
	if ((nstr = xcalloc_c(len, sizeof(char), 0, file, line)) == NULL)
		_xalloc_error(errno, "XALLOC: xstrdup(): %s:%u: Allocating %lu bytes: %s\n",
			      file ? file : unknown_file, line,
			      (unsigned long)(len));
	memcpy(nstr, str, len);
	return (nstr);
}

void
xfree_c(void **ptr_p, const char *file, unsigned int line)
{
	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xfree(): Xalloc library not initialized\n");

	if (ptr_p == NULL)
		_xalloc_fatal("XALLOC: xfree(): Bad argument(s)\n");

	if (*ptr_p == NULL) {
		_xalloc_warn("XALLOC: xfree(): Warning: %s:%u: Freeing NULL pointer\n",
			     file ? file : unknown_file, line);
		return;
	}

#ifdef XALLOC_DEBUG
	if (debug_level > 0) {
		struct memory	*mem = NULL, find_mem;

		XALLOC_LOCK(xalloc_mutex);
		find_mem.ptr = *ptr_p;
		if ((mem = RB_FIND(memory_tree, &memory_tree_head, &find_mem)) == NULL)
			_xalloc_fatal("XALLOC: xfree(): %s:%u: Junk pointer %p not accounted for\n",
				      file ? file : unknown_file, line,
				      *ptr_p);

		if (mem->freed_by != NULL && mem->id == 0) {
			_xalloc_debug_printf(2, "XALLOC: DOUBLE FREE of pointer %p in %s:%u: allocated in %s:%u, ",
					     mem->ptr,
					     file ? file : unknown_file, line,
					     mem->allocated_by,
					     mem->allocated_in_line);
			if (mem->reallocated_by != NULL)
				_xalloc_debug_printf(2, "last reallocated in %s:%u, ",
						     mem->reallocated_by,
						     mem->reallocated_in_line);
			_xalloc_debug_printf(2, "already freed in %s:%u\n",
					     mem->freed_by,
					     mem->freed_in_line);
			abort();
		}

		xalloc_freed += mem->size;
		xalloc_allocated -= mem->size;
		mem->id = 0;
		mem->size = 0;
		if (debug_level > 1) {
			if (file)
				mem->freed_by = file;
			else
				mem->freed_by = unknown_file;
			mem->freed_in_line = line;
		} else {
			RB_REMOVE(memory_tree, &memory_tree_head, mem);
			_memory_free(&mem);
		}
		XALLOC_UNLOCK(xalloc_mutex);
	}
#endif /* XALLOC_DEBUG */

	real_free(*ptr_p);
#ifdef XALLOC_DEBUG
	if (debug_level <= 1)
#endif /* XALLOC_DEBUG */
	{
		*ptr_p = NULL;
	}
}

#ifdef XALLOC_WITH_XASPRINTF
int
xasprintf_c(const char *file, unsigned int line,
	    char **str_p, const char *fmt, ...)
{
	int	ret;
	va_list ap;
	size_t	strsiz = 0;

	if (!xalloc_initialized)
		_xalloc_fatal("XALLOC: xasprintf(): Xalloc library not initialized\n");

	if (str_p == NULL || fmt == NULL)
		_xalloc_fatal("XALLOC: xasprintf(): Bad argument(s)\n");

	va_start(ap, fmt);
	ret = _xalloc_vasprintf(str_p, fmt, ap, &strsiz);
	va_end(ap);
	if (ret == -1)
		_xalloc_error(errno, "XALLOC: xasprintf(): %s:%u: Allocating %lu bytes",
			      file ? file : unknown_file, line, strsiz);

# ifdef XALLOC_DEBUG
	if (debug_level > 0) {
		struct memory	*mem, *mem_exists;

		if ((mem = real_calloc(1, sizeof(struct memory))) == NULL)
			_xalloc_error(errno, "XALLOC: Internal error");
		mem->ptr = *str_p;
		mem->size = strsiz;
		if (file)
			mem->allocated_by = file;
		else
			mem->allocated_by = unknown_file;
		mem->allocated_in_line = line;
		XALLOC_LOCK(xalloc_mutex);
		mem->id = ++xalloc_next_id;
		if ((mem_exists = RB_INSERT(memory_tree, &memory_tree_head, mem)) != NULL) {
			/* Freed pointer is being reused: */
			if (mem_exists->id != 0)
				_xalloc_fatal("XALLOC: Internal error: Assertion (mem_exists->id == 0) in %s:%u failed!\n",
					      __FILE__, __LINE__);
			RB_REMOVE(memory_tree, &memory_tree_head, mem_exists);
			_memory_free(&mem_exists);
			RB_INSERT(memory_tree, &memory_tree_head, mem);
		}
		xalloc_allocated += strsiz;
		xalloc_total += strsiz;
		if (xalloc_allocated > xalloc_peak)
			xalloc_peak = xalloc_allocated;
		XALLOC_UNLOCK(xalloc_mutex);
	}
# endif /* XALLOC_DEBUG */

	return (ret);
}
#endif /* XALLOC_WITH_XASPRINTF */
