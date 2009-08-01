dnl # $Id$

dnl # Check for a working installation of libshout.
dnl # Provides appropriate --with configuration options, fills the
dnl # LIBSHOUT_CFLAGS, LIBSHOUT_CPPFLAGS, LIBSHOUT_LDFLAGS and LIBSHOUT_LIBS
dnl # variables accordingly.


dnl # Copyright (c) 2009 Moritz Grimm <mgrimm@mrsserver.net>
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


dnl # For LIBSHOUT_CFLAGS, LIBSHOUT_CPPFLAGS, LIBSHOUT_LDFLAGS and
dnl # LIBSHOUT_LIBS:
dnl # AX_CHECK_LIBSHOUT([LIBSHOUT-VERSION], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])


AC_DEFUN([_AX_CHECK_LIBSHOUT_OPTS],
[
AC_REQUIRE([PKG_PROG_PKG_CONFIG])
if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi
AC_ARG_VAR([LIBSHOUT_CFLAGS],
	[C compiler flags for libshout])
AC_ARG_VAR([LIBSHOUT_CPPFLAGS],
	[C preprocessor flags for libshout])
AC_ARG_VAR([LIBSHOUT_LDFLAGS],
	[linker flags for libshout])
have_libshout_includes=""
have_libshout_libs=""
want_libshout="auto"
AC_ARG_WITH([libshout],
	[AS_HELP_STRING([--with-libshout=PFX],
		[prefix where the libshout header files and library are installed (default: autodetect)])],
	[
case "${withval}" in
	yes)
		want_libshout="yes"
		;;
	no)
		want_libshout="no"
		;;
	*)
		have_libshout_prefix="${withval}"
		want_libshout="yes"
		;;
esac
	]
)
AC_ARG_WITH([libshout-includes],
	[AS_HELP_STRING([--with-libshout-includes=DIR],
		[directory where libshout header files are installed (optional)]) ],
	[
case "${withval}" in
	yes|no) ;;
	*)
		have_libshout_includes="${withval}"
		;;
esac
	]
)
AC_ARG_WITH([libshout-libs],
	[AS_HELP_STRING([--with-libshout-libs=DIR],
		[directory where libshout is installed (optional)]) ],
	[
case "${withval}" in
	yes|no) ;;
	*)
		have_libshout_libs="${withval}"
		;;
esac
	]
)
AC_CACHE_VAL([local_cv_have_lib_libshout_opts],
[
ax_check_libshout_shout_pc="no"
PKG_CHECK_EXISTS([shout], [ax_check_libshout_shout_pc=yes])
if test -z "${LIBSHOUT_CFLAGS}" \
    -a x"${ax_check_libshout_shout_pc}" = "xyes"; then
	LIBSHOUT_CFLAGS="`${PKG_CONFIG} --cflags-only-other shout`"
fi
if test -n "${LIBSHOUT_CPPFLAGS}"; then
	if test -n "${have_libshout_includes}"; then
		LIBSHOUT_CPPFLAGS="${LIBSHOUT_CPPFLAGS} -I${have_libshout_includes}"
	fi
else
	if test -n "${have_libshout_includes}"; then
		LIBSHOUT_CPPFLAGS="-I${have_libshout_includes}"
	else
		if test x"${want_libshout}" = "xauto" \
		    -a x"${ax_check_libshout_shout_pc}" = "xyes"; then
			LIBSHOUT_CPPFLAGS="`${PKG_CONFIG} --cflags-only-I shout`"
		elif test -n "${have_libshout_prefix}"; then
			LIBSHOUT_CPPFLAGS="-I${have_libshout_prefix}/include"
		fi
	fi
fi
if test -n "${LIBSHOUT_LDFLAGS}"; then
	if test -n "${have_libshout_libs}"; then
		LIBSHOUT_LDFLAGS="-L${have_libshout_libs} ${LIBSHOUT_LDFLAGS}"
	fi
else
	if test -n "${have_libshout_libs}"; then
		LIBSHOUT_LDFLAGS="-L${have_libshout_libs}"
	else
		if test x"${want_libshout}" = "xauto" \
		    -a x"${ax_check_libshout_shout_pc}" = "xyes"; then
			LIBSHOUT_LDFLAGS=" \
				`${PKG_CONFIG} --libs-only-L shout` \
				`${PKG_CONFIG} --libs-only-other shout` \
			"
		elif test -n "${have_libshout_prefix}"; then
			LIBSHOUT_LDFLAGS="-L${have_libshout_prefix}/lib"
		fi
	fi
fi
local_cv_have_lib_libshout_opts=yes
])
])


AC_DEFUN([AX_CHECK_LIBSHOUT],
[
AC_REQUIRE([_AX_CHECK_LIBSHOUT_OPTS])
AC_REQUIRE([AC_PROG_FGREP])
AC_ARG_VAR([LIBSHOUT_LIBS],
	[libraries to use for libshout])
AC_CACHE_VAL([local_cv_have_lib_libshout],
[
local_cv_have_lib_libshout=no

if test x"${want_libshout}" != "xno"; then	# want_libshout != no

if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi

if test -n "$1" -a x"${ax_check_libshout_shout_pc}" = "xyes"; then
	PKG_CHECK_EXISTS([shout $1], [],
		[AC_MSG_ERROR([libshout version $1 is required.], [1])]
	)
fi

libshout_libs_autodetect=no
if test -z "${LIBSHOUT_LIBS}" \
    -a x"${ax_check_libshout_shout_pc}" = "xyes"; then
	LIBSHOUT_LIBS="`${PKG_CONFIG} --libs-only-l shout`"
	libshout_libs_autodetect=yes
fi

# If libshout required libm, make sure that it is listed at the end:
if test x"${libshout_libs_autodetect}" = x"yes"; then
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
		  if test x"${libshout_libs_autodetect}" = "xyes"; then
			LIBSHOUT_LIBS="`${PKG_CONFIG} --static --libs-only-l shout`"
			LIBS="${LIBSHOUT_LIBS} ${ax_check_libshout_save_LIBS}"
			AC_MSG_CHECKING([if libshout works with explicit dependencies])
			AC_LINK_IFELSE(
				[AC_LANG_PROGRAM(
				[[
				  #include <shout/shout.h>
				]],
				[[
				  shout_new();
				]])],
				[
				  AC_MSG_RESULT([yes])
				  local_cv_have_lib_libshout=yes
				],
				[
				  AC_MSG_RESULT([no])
				]
			)
		  fi
		]
	)
])
CFLAGS="${ax_check_libshout_save_CFLAGS}"
CPPFLAGS="${ax_check_libshout_save_CPPFLAGS}"
LDFLAGS="${ax_check_libshout_save_LDFLAGS}"
LIBS="${ax_check_libshout_save_LIBS}"
AC_LANG_POP([C])

fi						# want_libshout != no

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
