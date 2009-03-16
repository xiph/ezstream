dnl $Id: libogg.m4 729 2008-11-01 17:38:43Z mgrimm $

dnl # Check for a working installation of libogg.
dnl # Provides appropriate --with configuration options, fills and substitutes
dnl # the LIBOGG_CFLAGS, LIBOGG_CPPFLAGS, LIBOGG_LDFLAGS and LIBOGG_LIBS
dnl # variables accordingly.


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


dnl # For LIBOGG_CFLAGS, LIBOGG_CPPFLAGS, LIBOGG_LDFLAGS and LIBOGG_LIBS:
dnl # AX_CHECK_LIBOGG([LIBOGG-VERSION], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])


AC_DEFUN([AX_CHECK_LIBOGG],
[
AC_REQUIRE([PKG_PROG_PKG_CONFIG])
AC_ARG_VAR([LIBOGG_CFLAGS],
	[C compiler flags for libogg])
AC_ARG_VAR([LIBOGG_CPPFLAGS],
	[C preprocessor flags for libogg])
AC_ARG_VAR([LIBOGG_LDFLAGS],
	[linker flags for libogg])
AC_ARG_VAR([LIBOGG_LIBS],
	[libraries to use for libogg])
if test x"${prefix}" = "xNONE"; then
	ax_check_libogg_prefix="/usr/local"
else
	ax_check_libogg_prefix="${prefix}"
fi
AC_CACHE_VAL([local_cv_have_lib_libogg],
[
local_cv_have_lib_libogg=no

PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:${ax_check_libogg_prefix}/lib/pkgconfig"
if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi

dnl ####### BEGIN CHECK ######
PKG_CHECK_EXISTS([ogg $1], [
dnl ##########################

libogg_libs_autodetect=no
if test -z "${LIBOGG_CFLAGS}"; then
	LIBOGG_CFLAGS="`${PKG_CONFIG} --cflags-only-other ogg`"
fi
if test -z "${LIBOGG_CPPFLAGS}"; then
	LIBOGG_CPPFLAGS="`${PKG_CONFIG} --cflags-only-I ogg`"
fi
if test -z "${LIBOGG_LDFLAGS}"; then
	LIBOGG_LDFLAGS="\
		`${PKG_CONFIG} --libs-only-L ogg` \
		`${PKG_CONFIG} --libs-only-other ogg` \
	"
fi
if test -z "${LIBOGG_LIBS}"; then
	LIBOGG_LIBS="`${PKG_CONFIG} --libs-only-l ogg`"
	libogg_libs_autodetect=yes
fi

ax_check_libogg_save_CFLAGS="${CFLAGS}"
ax_check_libogg_save_CPPFLAGS="${CPPFLAGS}"
ax_check_libogg_save_LDFLAGS="${LDFLAGS}"
ax_check_libogg_save_LIBS="${LIBS}"
AC_LANG_PUSH([C])
CFLAGS="${CFLAGS} ${LIBOGG_CFLAGS}"
CPPFLAGS="${CPPFLAGS} ${LIBOGG_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${LIBOGG_LDFLAGS}"
LIBS="${LIBOGG_LIBS} ${LIBS}"
AC_CHECK_HEADER([ogg/ogg.h],
[
	AC_MSG_CHECKING([if libogg works])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
			[[
			  #include <stdlib.h>
			  #include <ogg/ogg.h>
			]],
			[[
			  ogg_sync_init(NULL);
			]]
		)],
		[
		  AC_MSG_RESULT([yes])
		  local_cv_have_lib_libogg=yes
		],
		[
		  AC_MSG_RESULT([no])
		]
	)
])
CFLAGS="${ax_check_libogg_save_CFLAGS}"
CPPFLAGS="${ax_check_libogg_save_CPPFLAGS}"
LDFLAGS="${ax_check_libogg_save_LDFLAGS}"
LIBS="${ax_check_libogg_save_LIBS}"
AC_LANG_POP([C])

dnl ####### END CHECK ########
], [])
dnl ##########################

])

AC_MSG_CHECKING([for libogg $1])
if test x"${local_cv_have_lib_libogg}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi

])
