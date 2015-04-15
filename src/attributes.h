#ifndef __ATTRIBUTES_H__
#define __ATTRIBUTES_H__

#include "attr_config.h"

#if defined(CFG_HAVE_ATTRIBUTES) && 1 == CFG_HAVE_ATTRIBUTES
	/* nothing */
#else
# define __attribute__(x)		\
	/* nothing, e.g. for sys/tree.h */
#endif /* CFG_HAVE_ATTRIBUTES */

#if defined(CFG_ATTRIBUTE_UNUSED) && 1 == CFG_ATTRIBUTE_UNUSED
# define ATTRIBUTE_UNUSED()	       \
	__attribute__((unused))
#else
# define ATTRIBUTE_UNUSED()	       \
	/* nothing */
#endif /* CFG_ATTRIBUTE_UNUSED */

#if defined(CFG_ATTRIBUTE_FORMAT) && 1 == CFG_ATTRIBUTE_FORMAT
# define ATTRIBUTE_FORMAT(t, n, m)     \
	__attribute__((__format__(t, (n), (m))))
#else
# define ATTRIBUTE_FORMAT(t, n, m)     \
	/* nothing */
#endif /* CFG_ATTRIBUTE_FORMAT */

#if defined(CFG_ATTRIBUTE_NONNULL) && 1 == CFG_ATTRIBUTE_NONNULL
# define ATTRIBUTE_NONNULL(n)	       \
	__attribute__((__nonnull__(n)))
# define ATTRIBUTE_NONNULL_2(n, m)     \
	__attribute__((__nonnull__(n, m)))
# define ATTRIBUTE_NONNULL_3(n, m, o)  \
	__attribute__((__nonnull__(n, m, o)))
#else
# define ATTRIBUTE_NONNULL(n)	       \
	/* nothing */
# define ATTRIBUTE_NONNULL_2(n, m)     \
	/* nothing */
# define ATTRIBUTE_NONNULL_3(n, m, o)  \
	/* nothing */
#endif /* CFG_ATTRIBUTE_NONNULL */

#endif /* __ATTRIBUTES_H__ */
