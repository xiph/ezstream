dnl # $Id$

dnl # Check for a working installation of libxml (version 2.)
dnl # Provides appropriate --with configuration options, fills and substitutes
dnl # the LIBXML2_CFLAGS, LIBXML2_CPPFLAGS, LIBXML2_LDFLAGS and LIBXML2_LIBS
dnl # variables accordingly.

dnl # Copyright (c) 2008 Moritz Grimm <mgrimm@mrsserver.net>
dnl #
dnl # Permission to use, copy, modify, and distribute this software for any
dnl # purpose with or without fee is hereby granted, provided that the above
dnl # copyright notice and this permission notice appear in all copies.
dnl #
dnl # THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
dnl # WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
dnl # MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
dnl # ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
dnl # WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
dnl # ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
dnl # OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

dnl # AX_CHECK_LIBXML2([LIBXML2-VERSION], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])

AC_DEFUN([AX_CHECK_LIBXML2],
[
AC_REQUIRE([PKG_PROG_PKG_CONFIG])
AC_ARG_VAR([LIBXML2_CFLAGS],
	[C compiler flags for libxml2])
AC_ARG_VAR([LIBXML2_CPPFLAGS],
	[C preprocessor flags for libxml2])
AC_ARG_VAR([LIBXML2_LDFLAGS],
	[linker flags for libxml2])
AC_ARG_VAR([LIBXML2_LIBS],
	[libraries to use for libxml2])
if test x"${prefix}" = "xNONE"; then
	ax_check_libxml2_prefix="/usr/local"
else
	ax_check_libxml2_prefix="${prefix}"
fi
AC_CACHE_VAL([local_cv_have_lib_libxml2],
[
local_cv_have_lib_libxml2=no

PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${ax_check_libxml2_prefix}/lib/pkgconfig"
if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi

dnl ####### BEGIN CHECK ######
PKG_CHECK_EXISTS([libxml-2.0 $1], [
dnl ##########################

libxml2_libs_autodetect=no
if test -z "${LIBXML2_CFLAGS}"; then
	LIBXML2_CFLAGS="`${PKG_CONFIG} --cflags-only-other libxml-2.0`"
fi
if test -z "${LIBXML2_CPPFLAGS}"; then
	LIBXML2_CPPFLAGS="`${PKG_CONFIG} --cflags-only-I libxml-2.0`"
fi
if test -z "${LIBXML2_LDFLAGS}"; then
	LIBXML2_LDFLAGS="\
		`${PKG_CONFIG} --libs-only-L libxml-2.0` \
		`${PKG_CONFIG} --libs-only-other libxml-2.0` \
	"
fi
if test -z "${LIBXML2_LIBS}"; then
	LIBXML2_LIBS="`${PKG_CONFIG} --libs-only-l libxml-2.0`"
	libxml2_libs_autodetect=yes
fi

ax_check_libxml2_save_CFLAGS="${CFLAGS}"
ax_check_libxml2_save_CPPFLAGS="${CPPFLAGS}"
ax_check_libxml2_save_LDFLAGS="${LDFLAGS}"
ax_check_libxml2_save_LIBS="${LIBS}"
AC_LANG_PUSH([C])
CFLAGS="${CFLAGS} ${LIBXML2_CFLAGS}"
CPPFLAGS="${CPPFLAGS} ${LIBXML2_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${LIBXML2_LDFLAGS}"
LIBS="${LIBXML2_LIBS} ${LIBS}"
AC_CHECK_HEADERS([libxml/parser.h], [
	AC_MSG_CHECKING([if libxml2 works])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
		[[
		  #include <stdlib.h>
		  #include <libxml/parser.h>
		]],
		[[
		  xmlParseFile(NULL);
		]])],
		[
		  AC_MSG_RESULT([yes])
		  local_cv_have_lib_libxml2=yes
		],
		[
		  AC_MSG_RESULT([no])
		  if test x"${libxml2_libs_autodetect}" = "xyes"; then
			LIBXML2_LIBS="`${PKG_CONFIG} --static --libs-only-l libxml-2.0`"
			LIBS="${LIBXML2_LIBS} ${ax_check_libxml2_save_LIBS}"
			AC_MSG_CHECKING([if libxml2 works with explicit dependencies])
			AC_RUN_IFELSE(
				[AC_LANG_PROGRAM(
				[[
                                  #include <stdlib.h>
				  #include <libxml/parser.h>
				]],
				[[
				  xmlParseFile(NULL);
				]])],
				[
				  AC_MSG_RESULT([yes])
				  local_cv_have_lib_libxml2=yes
				],
				[
				  AC_MSG_RESULT([no])
				]
			)
		  fi
		]
	)
])
CFLAGS="${ax_check_libxml2_save_CFLAGS}"
CPPFLAGS="${ax_check_libxml2_save_CPPFLAGS}"
LDFLAGS="${ax_check_libxml2_save_LDFLAGS}"
LIBS="${ax_check_libxml2_save_LIBS}"
AC_LANG_POP([C])

dnl ####### END CHECK ########
], [])
dnl ##########################

])

AC_MSG_CHECKING([for libxml2 $1])
if test x"${local_cv_have_lib_libxml2}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi

])
