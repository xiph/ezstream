dnl # $Id$

dnl # Check for working installations of TagLib and its C-wrapper library,
dnl # libtag_c.
dnl # Provides appropriate --with configuration options, fills the
dnl # TAGLIB_CFLAGS, TAGLIB_CPPFLAGS, TAGLIB_LDFLAGS, TAGLIB_LIBS and
dnl # TAGLIB_C_LIBS variables accordingly.


dnl # Copyright (c) 2008, 2009 Moritz Grimm <mgrimm@mrsserver.net>
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


dnl # Both check functions provide TAGLIB_CPPFLAGS and TAGLIB_LDFLAGS.

dnl # For TAGLIB_LIBS:
dnl # AX_CHECK_TAGLIB([TAGLIB-VERSION], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])

dnl # For TAGLIB_C_LIBS:
dnl # AX_CHECK_TAGLIB_C([TAGLIB-VERSION], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])


AC_DEFUN([_AX_CHECK_TAGLIB_OPTS],
[
AC_REQUIRE([PKG_PROG_PKG_CONFIG])
if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi
AC_ARG_VAR([TAGLIB_CFLAGS],
	[C compiler flags for TagLib])
AC_ARG_VAR([TAGLIB_CPPFLAGS],
	[C preprocessor flags for TagLib])
AC_ARG_VAR([TAGLIB_LDFLAGS],
	[linker flags for TagLib])
have_taglib_includes=""
have_taglib_libs=""
want_taglib="auto"
AC_ARG_WITH([taglib],
	[AS_HELP_STRING([--with-taglib=PFX],
		[prefix where the TagLib header files and library are installed (default: autodetect)])],
	[
case "${withval}" in
	yes)
		want_taglib="yes"
		;;
	no)
		want_taglib="no"
		;;
	*)
		have_taglib_prefix="${withval}"
		want_taglib="yes"
		;;
esac
	]
)
AC_ARG_WITH([taglib-includes],
	[AS_HELP_STRING([--with-taglib-includes=DIR],
		[directory where TagLib header files are installed (optional)]) ],
	[
case "${withval}" in
	yes|no) ;;
	*)
		have_taglib_includes="${withval}"
		;;
esac
	]
)
AC_ARG_WITH([taglib-libs],
	[AS_HELP_STRING([--with-taglib-libs=DIR],
		[directory where TagLib is installed (optional)]) ],
	[
case "${withval}" in
	yes|no) ;;
	*)
		have_taglib_libs="${withval}"
		;;
esac
	]
)
AC_CACHE_VAL([local_cv_have_lib_taglib_opts],
[
ax_check_taglib_taglib_pc="no"
PKG_CHECK_EXISTS([taglib], [ax_check_taglib_taglib_pc=yes])
if test -z "${TAGLIB_CFLAGS}" \
    -a x"${ax_check_taglib_taglib_pc}" = "xyes"; then
	TAGLIB_CFLAGS="`${PKG_CONFIG} --cflags-only-other taglib`"
fi
if test -n "${TAGLIB_CPPFLAGS}"; then
	if test -n "${have_taglib_includes}"; then
		TAGLIB_CPPFLAGS="${TAGLIB_CPPFLAGS} -I${have_taglib_includes}"
	fi
else
	if test -n "${have_taglib_includes}"; then
		TAGLIB_CPPFLAGS="-I${have_taglib_includes}"
	else
		if test x"${want_taglib}" = "xauto" \
		    -a x"${ax_check_taglib_taglib_pc}" = "xyes"; then
			TAGLIB_CPPFLAGS="`${PKG_CONFIG} --cflags-only-I taglib`"
		elif test -n "${have_taglib_prefix}"; then
			TAGLIB_CPPFLAGS="-I${have_taglib_prefix}/include/taglib"
		fi
	fi
fi
if test -n "${TAGLIB_LDFLAGS}"; then
	if test -n "${have_taglib_libs}"; then
		TAGLIB_LDFLAGS="-L${have_taglib_libs} ${TAGLIB_LDFLAGS}"
	fi
else
	if test -n "${have_taglib_libs}"; then
		TAGLIB_LDFLAGS="-L${have_taglib_libs}"
	else
		if test x"${want_taglib}" = "xauto" \
		    -a x"${ax_check_taglib_taglib_pc}" = "xyes"; then
			TAGLIB_LDFLAGS=" \
				`${PKG_CONFIG} --libs-only-L taglib` \
				`${PKG_CONFIG} --libs-only-other taglib` \
			"
		elif test -n "${have_taglib_prefix}"; then
			TAGLIB_LDFLAGS="-L${have_taglib_prefix}/lib"
		fi
	fi
fi
local_cv_have_lib_taglib_opts=yes
])
])


AC_DEFUN([AX_CHECK_TAGLIB],
[
AC_REQUIRE([_AX_CHECK_TAGLIB_OPTS])
AC_ARG_VAR([TAGLIB_LIBS],
	[libraries to use for TagLib])
AC_CACHE_VAL([local_cv_have_lib_taglib],
[
local_cv_have_lib_taglib=no

if test x"${want_taglib}" != "xno"; then	# want_taglib != no

if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi

dnl ####### BEGIN CHECK ######
PKG_CHECK_EXISTS([taglib $1], [
dnl ##########################

libtag_libs_autodetected=no
if test -z "${TAGLIB_LIBS}"; then
	TAGLIB_LIBS="`${PKG_CONFIG} --libs-only-l taglib`"
	libtag_libs_autodetected=yes
fi

ax_check_taglib_save_CXXFLAGS="${CXXFLAGS}"
ax_check_taglib_save_CPPFLAGS="${CPPFLAGS}"
ax_check_taglib_save_LDFLAGS="${LDFLAGS}"
ax_check_taglib_save_LIBS="${LIBS}"
AC_LANG_PUSH([C++])
CXXFLAGS="${CXXFLAGS} ${TAGLIB_CFLAGS}"
CPPFLAGS="${CPPFLAGS} ${TAGLIB_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${TAGLIB_LDFLAGS}"
LIBS="${TAGLIB_LIBS} ${LIBS}"
AC_CHECK_HEADER([tag.h],
[
	AC_MSG_CHECKING([if TagLib works])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
			[[
			  #include <tag.h>
			  #include <fileref.h>
			]],
			[[
			  TagLib::FileRef f(static_cast<TagLib::File *>(NULL));
			]]
		)],
		[
		  AC_MSG_RESULT([yes])
		  local_cv_have_lib_taglib=yes
		],
		[
		  AC_MSG_RESULT([no])
		  LIBS="${LIBS} -lstdc++ -lz"
		  TAGLIB_LIBS="${TAGLIB_LIBS} -lstdc++ -lz"
		  AC_MSG_CHECKING([if TagLib works with -lstdc++ -lz])
		  AC_LINK_IFELSE(
			[AC_LANG_PROGRAM(
				[[
				  #include <tag.h>
				  #include <fileref.h>
				]],
				[[
				  TagLib::FileRef f(static_cast<TagLib::File *>(NULL));
				]]
			)],
			[
			  AC_MSG_RESULT([yes])
			  local_cv_have_lib_taglib=yes
			],
			[
			  AC_MSG_RESULT([no])
			]
		  )
		]
	)
])
CXXFLAGS="${ax_check_taglib_save_CXXFLAGS}"
CPPFLAGS="${ax_check_taglib_save_CPPFLAGS}"
LDFLAGS="${ax_check_taglib_save_LDFLAGS}"
LIBS="${ax_check_taglib_save_LIBS}"
AC_LANG_POP([C++])

dnl ####### END CHECK ########
], [])
dnl ##########################

fi						# want_taglib != no

])

AC_MSG_CHECKING([for TagLib])
if test x"${local_cv_have_lib_taglib}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi
])


AC_DEFUN([AX_CHECK_TAGLIB_C],
[
AC_REQUIRE([_AX_CHECK_TAGLIB_OPTS])
AC_ARG_VAR([TAGLIB_C_LIBS],
	[libraries to use for the TagLib C wrapper])
AC_CACHE_VAL([local_cv_have_lib_taglib_c],
[
local_cv_have_lib_taglib_c=no

if test x"${want_taglib}" != "xno"; then	# want_taglib != no

if test -z "${PKG_CONFIG}"; then
	AC_MSG_ERROR([The pkg-config utility is required.], [1])
fi

dnl ####### BEGIN CHECK ######
PKG_CHECK_EXISTS([taglib_c $1], [
dnl ##########################

if test -z "${TAGLIB_C_LIBS}"; then
	TAGLIB_C_LIBS="-ltag_c"
fi

ax_check_taglib_c_save_CFLAGS="${CFLAGS}"
ax_check_taglib_c_save_CPPFLAGS="${CPPFLAGS}"
ax_check_taglib_c_save_LDFLAGS="${LDFLAGS}"
ax_check_taglib_c_save_LIBS="${LIBS}"
AC_LANG_PUSH([C])
CFLAGS="${CFLAGS} ${TAGLIB_CFLAGS}"
CPPFLAGS="${CPPFLAGS} ${TAGLIB_CPPFLAGS}"
LDFLAGS="${LDFLAGS} ${TAGLIB_LDFLAGS}"
LIBS="${TAGLIB_C_LIBS} ${LIBS}"
AC_CHECK_HEADER([tag_c.h],
[
	AC_MSG_CHECKING([if libtag_c works])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
			[[
			  #include <tag_c.h>
			]],
			[[
			  taglib_set_string_management_enabled(0);
			]]
		)],
		[
		  AC_MSG_RESULT([yes])
		  local_cv_have_lib_taglib_c=yes
		],
		[
		  AC_MSG_RESULT([no])
		  LIBS="${LIBS} -ltag -lstdc++ -lz"
		  TAGLIB_C_LIBS="${TAGLIB_C_LIBS} -ltag -lstdc++ -lz"
		  AC_MSG_CHECKING([if libtag_c works with -ltag -lstdc++ -lz])
		  AC_LINK_IFELSE(
			[AC_LANG_PROGRAM(
				[[
				  #include <tag_c.h>
				]],
				[[
				  taglib_set_string_management_enabled(0);
				]]
			)],
			[
			  AC_MSG_RESULT([yes])
			  local_cv_have_lib_taglib_c=yes
			],
			[
			  AC_MSG_RESULT([no])
			]
		  )
		]
	)
])
CFLAGS="${ax_check_taglib_c_save_CFLAGS}"
CPPFLAGS="${ax_check_taglib_c_save_CPPFLAGS}"
LDFLAGS="${ax_check_taglib_c_save_LDFLAGS}"
LIBS="${ax_check_taglib_c_save_LIBS}"
AC_LANG_POP([C])

dnl ####### END CHECK ########
], [])
dnl ##########################

fi						# want_taglib != no

])

AC_MSG_CHECKING([for libtag_c])
if test x"${local_cv_have_lib_taglib_c}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi
])
