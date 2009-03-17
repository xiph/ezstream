dnl $Id$

dnl # Check for a working installation of libshout.
dnl # Provides appropriate --with configuration options, fills and substitutes
dnl # the LIBSHOUT_CFLAGS, LIBSHOUT_CPPFLAGS, LIBSHOUT_LDFLAGS and
dnl # LIBSHOUT_LIBS variables accordingly.


dnl # Copyright (c) 2008 Moritz Grimm <mgrimm@mrsserver.net>
dnl #
dnl # Permission to use, copy, modify, and distribute this software for any
dnl # purpose with or without fee is hereby granted, provided that the above
dnl # copyright notice and this permission notice appear in all copies.
dnl
dnl # THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
dnl # WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
dnl # MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
dnl # ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
dnl # WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
dnl # ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
dnl # OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


dnl # For LIBSHOUT_CFLAGS, LIBSHOUT_CPPFLAGS, LIBSHOUT_LDFLAGS and
dnl # LIBSHOUT_LIBS:
dnl # AX_CHECK_LIBSHOUT([LIBSHOUT-VERSION], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])


AC_DEFUN([AX_CHECK_LIBSHOUT],
[
AC_REQUIRE([PKG_PROG_PKG_CONFIG])
AC_REQUIRE([AC_PROG_FGREP])
AC_ARG_VAR([LIBSHOUT_CFLAGS],
	[C compiler flags for libshout])
AC_ARG_VAR([LIBSHOUT_CPPFLAGS],
	[C preprocessor flags for libshout])
AC_ARG_VAR([LIBSHOUT_LDFLAGS],
	[linker flags for libshout])
AC_ARG_VAR([LIBSHOUT_LIBS],
	[libraries to use for libshout])
if test x"${prefix}" = "xNONE"; then
	ax_check_libshout_prefix="/usr/local"
else
	ax_check_libshout_prefix="${prefix}"
fi
AC_CACHE_VAL([local_cv_have_lib_libshout],
[
local_cv_have_lib_libshout=no

PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${ax_check_libshout_prefix}/lib/pkgconfig"
if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi

dnl ####### BEGIN CHECK ######
PKG_CHECK_EXISTS([shout $1], [
dnl ##########################

libshout_libs_autodetect=no
if test -z "${LIBSHOUT_CFLAGS}"; then
	LIBSHOUT_CFLAGS="`${PKG_CONFIG} --cflags-only-other shout`"
fi
if test -z "${LIBSHOUT_CPPFLAGS}"; then
	LIBSHOUT_CPPFLAGS="`${PKG_CONFIG} --cflags-only-I shout`"
fi
if test -z "${LIBSHOUT_LDFLAGS}"; then
	LIBSHOUT_LDFLAGS="\
		`${PKG_CONFIG} --libs-only-L shout` \
		`${PKG_CONFIG} --libs-only-other shout` \
	"
fi
if test -z "${LIBSHOUT_LIBS}"; then
	LIBSHOUT_LIBS="`${PKG_CONFIG} --libs-only-l shout`"
	libshout_libs_autodetect=yes
fi

# If libshout required libm, make sure that it is listed at the end:
if test "${libshout_libs_autodetect}" = "yes"; then
	if test -n "`echo ${LIBSHOUT_LIBS} | ${FGREP} -e ' -lm'`"; then
		xt_shout_TEMP="`echo ${LIBSHOUT_LIBS} | sed -e 's, -lm,,g'`"
		LIBSHOUT_LIBS="${xt_shout_TEMP} -lm"
	fi
fi

ax_check_libshout_save_CFLAGS="${CFLAGS}"
ax_check_libshout_save_CPPFLAGS="${CPPFLAGS}"
ax_check_libshout_save_LDFLAGS="${LDFLAGS}"
ax_check_libshout_save_LIBS="${LIBS}"
AC_LANG_PUSH([C])
CFLAGS="${CFLAGS} ${LIBSHOUT_CFLAGS}"
CPPFLAGS="${CPPFLAGS} ${LIBSHOUT_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${LIBSHOUT_LDFLAGS}"
LIBS="${LIBSHOUT_LIBS} ${LIBS}"
AC_CHECK_HEADER([shout/shout.h],
[
	AC_MSG_CHECKING([if libshout works])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
			[[
			  #include <shout/shout.h>
			]],
			[[
			  shout_new();
			]]
		)],
		[
		  AC_MSG_RESULT([yes])
		  local_cv_have_lib_libshout=yes
		],
		[
		  AC_MSG_RESULT([no])
		]
	)
])
CFLAGS="${ax_check_libshout_save_CFLAGS}"
CPPFLAGS="${ax_check_libshout_save_CPPFLAGS}"
LDFLAGS="${ax_check_libshout_save_LDFLAGS}"
LIBS="${ax_check_libshout_save_LIBS}"
AC_LANG_POP([C])

dnl ####### END CHECK ########
], [])
dnl ##########################

])

AC_MSG_CHECKING([for libshout $1])
if test x"${local_cv_have_lib_libshout}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi

])
