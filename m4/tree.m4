dnl # $Id$

dnl # Check for sys/tree.h and if it is recent enough by looking at a certain
dnl # macro. Defines HAVE_SYS_TREE_H if all conditions are met.

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


dnl # AX_HEADER_TREE([REQUIRED-MACRO], [ACTION-IF-FOUND],
dnl #     [ACTION-IF-NOT-FOUND])


AC_DEFUN([AX_HEADER_TREE],
[
if test x"$1" = "x"; then
	_tree_macro="RB_INIT"
else
	_tree_macro=$1
fi
AC_MSG_CHECKING([for ${_tree_macro} in sys/tree.h])
AC_CACHE_VAL([local_cv_header_sys_tree_h],
[
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM(
		[[
		  #include <sys/tree.h>
		  #if !defined(${_tree_macro})
		  # error Required tree macro missing
		  #endif /* !${_tree_macro} */
		]],
		[[]])],
		[local_cv_header_sys_tree_h=yes],
		[local_cv_header_sys_tree_h=no])
])
if test x"${local_cv_header_sys_tree_h}" = "xyes"; then
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE_SYS_TREE_H], [1],
		[Define to 1 if you have a sufficiently recent <sys/tree.h> header.])
	:
	$2
else
	AC_MSG_RESULT([no])
	:
	$3
fi
])
