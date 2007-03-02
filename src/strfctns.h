#ifndef __STRLFCTNS_H__
#define __STRLFCTNS_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef HAVE_STRLCAT
# define strlcat	local_strlcat
#endif
size_t		local_strlcat(char *, const char *, size_t);

#ifndef HAVE_STRLCPY
# define strlcpy	local_strlcpy
#endif
size_t		local_strlcpy(char *, const char *, size_t);

#ifndef HAVE_STRTONUM
# define strtonum	local_strtonum
#endif
long long	local_strtonum(const char *, long long, long long, const char **);

#endif /* __STRLFCTNS_H__ */
