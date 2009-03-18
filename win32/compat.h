#ifndef __WIN32_COMPAT_H__
#define __WIN32_COMPAT_H__

/* #define WIN32_LEAN_AND_MEAN */
#include <io.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#define inline			__inline

#define va_copy(dst, src)	memcpy(&(dst), &(src), sizeof(va_list))

#define PATH_SEPARATORS 	"\\/"

#ifndef ssize_t
# define ssize_t		long
#endif /* !ssize_t */

#endif /* __WIN32_COMPAT_H__ */