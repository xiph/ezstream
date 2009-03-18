#ifndef __WIN32_COMPAT_H__
#define __WIN32_COMPAT_H__

/* #define WIN32_LEAN_AND_MEAN */
#include <io.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#define inline			__inline

#define popen			_popen
#define pclose			_pclose
#define snprintf		_snprintf
#define stat			_stat
#define strncasecmp		strnicmp
#ifndef __GNUC__
# define strtoll		_strtoi64
#endif /* !__GNUC__ */

#define sleep(a)		Sleep((a) * 1000)

#define va_copy(dst, src)	memcpy(&(dst), &(src), sizeof(va_list))

#define S_IRGRP 		0
#define S_IROTH 		0
#define S_IWGRP 		0
#define S_IWOTH 		0
#define S_IXGRP 		0
#define S_IXOTH 		0
#define PATH_SEPARATORS 	"\\/"

#ifndef ssize_t
# define ssize_t		long
#endif /* !ssize_t */

#endif /* __WIN32_COMPAT_H__ */