dnl # $Id$

dnl # Check if the compiler understands a certain __attribute__.


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


dnl # AX_FUNC_ATTRIBUTE([FUNC-SIGNATURE], [ATTRIBUTE], [ARGS],
dnl #     [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])

dnl # AX_FUNC_ATTRIBUTE_UNUSED([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])


AC_DEFUN([AX_FUNC_ATTRIBUTE],
[
AC_REQUIRE([AX_CHECK_ERROR_FLAG])
if test x"${local_cv_prog_cc_error_flag}" != "xno"; then
	_errflag="${local_cv_prog_cc_error_flag}"
else
	_errflag=""
fi
AC_MSG_CHECKING([if __attribute__((__$2__$3)) is supported])
_cv_sufx_=$(echo "$3" | sed 'y|-+,=() |_p_____|')
AC_CACHE_VAL([local_cv_prog_cc_attr_$2_${_cv_sufx}],
[
	AC_LANG_PUSH([C])
	save_CFLAGS="${CFLAGS}"
	CFLAGS="${CFLAGS} ${_errflag}"
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
		[[
		  #ifdef HAVE_SYS_TYPES_H
		  # include <sys/types.h>
		  #endif
		  #include <stdlib.h>
		  void * attrtest($1)
			__attribute__((__$2__$3));
		  void *
		  attrtest($1) { return(NULL); }
		]],
		[[]])],
		[eval "local_cv_prog_cc_attr_$2_${_cv_sufx_}=yes"],
		[eval "local_cv_prog_cc_attr_$2_${_cv_sufx_}=no"])
	CFLAGS="${save_CFLAGS}"
	AC_LANG_POP([C])
])
eval "_cache_val_=\${local_cv_prog_cc_attr_$2_${_cv_sufx_}}"
if test x"${_cache_val_}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$4
else
	AC_MSG_RESULT([no])
	:
	$5
fi
])


AC_DEFUN([AX_FUNC_ATTRIBUTE_UNUSED],
[
AC_REQUIRE([AX_CHECK_ERROR_FLAG])
if test x"${local_cv_prog_cc_error_flag}" != "xno"; then
	_errflag="${local_cv_prog_cc_error_flag}"
else
	_errflag=""
fi
AC_MSG_CHECKING([if __attribute__((unused)) is supported])
AC_CACHE_VAL([local_cv_proc_cc_attr_unused],
[
	AC_LANG_PUSH([C])
	save_CFLAGS="${CFLAGS}"
	CFLAGS="${CFLAGS} ${_errflag}"
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
		[[
		  void	attrtest(void)
			__attribute__((unused));
		  void
		  attrtest(void) {}
		]],
		[[
		  int attrtest_var
			__attribute__((unused));
		]])],
		[local_cv_proc_cc_attr_unused=yes],
		[local_cv_proc_cc_attr_unused=no])
	CFLAGS="${save_CFLAGS}"
	AC_LANG_POP([C])
])
if test x"${local_cv_proc_cc_attr_unused}" = "xyes"; then
	AC_MSG_RESULT([yes])
	:
	$1
else
	AC_MSG_RESULT([no])
	:
	$2
fi
])
