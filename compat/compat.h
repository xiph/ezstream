#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>

#ifndef HAVE_GETOPT
extern int	 opterr;
extern int	 optind;
extern int	 optopt;
extern int	 optreset;
extern char	*optarg;

int	getopt(int, char * const *, const char *);
#endif /* !HAVE_GETOPT */

#ifndef HAVE_STRLCAT
size_t strlcat(char *, const char *, size_t);
#endif /* !HAVE_STRLCAT */

#ifndef HAVE_STRLCPY
size_t strlcpy(char *, const char *, size_t);
#endif /* !HAVE_STRLCPY */

#ifndef HAVE_STRTONUM
long long strtonum(const char *, long long, long long, const char **);
#endif /* !HAVE_STROTONUM */

#ifndef HAVE_REALLOCARRAY
void *	reallocarray(void *, size_t, size_t);
#endif /* !HAVE_REALLOCARRAY */

#endif /* __COMPAT_H__ */
