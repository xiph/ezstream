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

#ifdef WIN32

# define _PATH_DEVNULL	"nul"

# define strncasecmp	strnicmp
# define popen		_popen
# define pclose 	_pclose
# define snprintf	_snprintf
# define stat		_stat

# define S_IXGRP	0
# define S_IXOTH	0
# define S_IWGRP	0
# define S_IWOTH	0
# define S_IRGRP	0
# define S_IROTH	0

#else

# ifndef S_IEXEC
#  define S_IEXEC	S_IXUSR
# endif

#endif /* WIN32 */

#endif /* __COMPAT_H__ */
