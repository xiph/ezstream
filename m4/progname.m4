dnl # $Id$

dnl # Check whether libc defines __progname. Defines HAVE___PROGNAME, if
dnl # applicable. This check is heavily inspired by the one in OpenNTPd
dnl # (portable.)

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


dnl # AX_CHECK___PROGNAME([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])


AC_DEFUN([AX_CHECK___PROGNAME],
[
AC_MSG_CHECKING([whether libc defines __progname])
AC_CACHE_VAL([local_cv_libc_defines___progname],
[
	AC_LANG_PUSH([C])
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
		[[
		  #include <stdio.h>
		]],
		[[
		  extern char *__progname;
		  printf("%s", __progname);
		]])],
		[local_cv_libc_defines___progname=yes],
		[local_cv_libc_defines___progname=no]
	)
	AC_LANG_POP([C])
])
if test x"${local_cv_libc_defines___progname}" = "xyes"; then
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE___PROGNAME], [1],
		[Define whether libc defines __progname.])
	:
	$1
else
	AC_MSG_RESULT([no])
	:
	$2
fi
])
