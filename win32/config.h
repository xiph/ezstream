#include <sys/types.h>
#include <windows.h>
#include <io.h>

#define HAVE_INTTYPES_H 	1
#define HAVE_LOCALE_H		1
#define HAVE_STAT		1
#define HAVE_STDINT_H		1
#define HAVE_STRUCT_TIMEVAL	1
#define HAVE_SYS_STAT_H 	1

#define ICONV_CONST		const

/* Name of package */
#define PACKAGE "ezstream"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://trac.xiph.org/newticket?component=ezstream"

/* Define to the full name of this package. */
#define PACKAGE_NAME "ezstream"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "ezstream 0.5.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "ezstream"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.5.1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.5.1"

#ifdef XALLOC_DEBUG
typedef long ssize_t;
#endif
