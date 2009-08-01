dnl # $Id$

dnl # * Check if the compiler understands a certain flag or not.
dnl #   Inspiration for this came from watching TagLib configure.
dnl # * Check for the CFLAG required to turn warnings into errors.


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


dnl # AX_CHECK_CFLAG(CFLAG, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl # AX_CHECK_CXXFLAG(CXXFLAG, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])


AC_DEFUN([AX_CHECK_CFLAG],
[
AC_MSG_CHECKING([if ${CC} understands -$1])
_cv_sufx_=$(echo $1 | sed 'y|-+,= |_p___|')
AC_CACHE_VAL([local_cv_prog_cc_${_cv_sufx_}],
[
	AC_LANG_PUSH([C])
	save_CFLAGS="${CFLAGS}"
	CFLAGS="${CFLAGS} -$1"
	AC_TRY_LINK([], [],
		[eval "local_cv_prog_cc_${_cv_sufx_}=yes"],
		[eval "local_cv_prog_cc_${_cv_sufx_}=no"])
	CFLAGS="${save_CFLAGS}"
	AC_LANG_POP([C])
])
eval "_cache_val_=\${local_cv_prog_cc_${_cv_sufx_}}"
if test x"${_cache_val_}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi
])

AC_DEFUN([AX_CHECK_CXXFLAG],
[
AC_MSG_CHECKING([if ${CXX} understands -$1])
_cv_sufx_=$(echo $1 | sed 'y|-+,= |_p___|')
AC_CACHE_VAL([local_cv_prog_cxx_${_cv_sufx_}],
[
	AC_LANG_PUSH([C++])
	save_CXXFLAGS="${CXXFLAGS}"
	CXXFLAGS="${CXXFLAGS} -$1"
	AC_TRY_LINK([], [],
		[eval "local_cv_prog_cxx_${_cv_sufx_}=yes"],
		[eval "local_cv_prog_cxx_${_cv_sufx_}=no"])
	CXXFLAGS="${save_CXXFLAGS}"
	AC_LANG_POP([C++])
])
eval "_cache_val_=\${local_cv_prog_cxx_${_cv_sufx_}}"
if test x"${_cache_val_}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi
])

AC_DEFUN([AX_CHECK_ERROR_FLAG],
[
AC_CACHE_VAL([local_cv_prog_cc_error_flag],
[
	AC_MSG_CHECKING([if ${CC} supports an error flag])
	AC_LANG_PUSH([C])
	save_CFLAGS="${CFLAGS}"
	local_cv_prog_cc_error_flag="-Werror" # GCC
	CFLAGS="${save_CFLAGS} ${local_cv_prog_cc_error_flag}"
	AC_TRY_LINK([], [], [],
	[
	  local_cv_prog_cc_error_flag="-errwarn" # Sun C
	  CFLAGS="${save_CFLAGS} ${local_cv_prog_cc_error_flag}"
	  AC_TRY_LINK([], [], [],
	  [
	    local_cv_prog_cc_error_flag=no]
	  )]
	)
	CFLAGS="${save_CFLAGS}"
	AC_LANG_POP([C])
	if test x"${local_cv_prog_cc_error_flag}" != "xno"; then
		AC_MSG_RESULT([yes (${local_cv_prog_cc_error_flag})])
		:
		$2
	else
		AC_MSG_RESULT([no])
		:
		$3
	fi
])
])
