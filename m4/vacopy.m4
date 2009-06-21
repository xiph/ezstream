dnl # $Id$

dnl # Check for means to copy variable argument lists. After this check,
dnl # va_copy will at least be defined to something working.
dnl # This requires stdarg.h, which is assumed to exist.


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


dnl # AX_FUNC_VA_COPY


AC_DEFUN([AX_FUNC_VA_COPY],
[
AC_MSG_CHECKING([for means to copy variable argument lists])
AC_CACHE_VAL([local_ac_cv_func_va_copy],
[
	AC_LINK_IFELSE(
		[AC_LANG_PROGRAM(
		[[
		  #include <stdarg.h>
		]],
		[[
		  va_list ap, ap2;
		  va_copy(ap2, ap);
		]])],
		[local_ac_cv_func_va_copy=va_copy],
		[AC_LINK_IFELSE(
			[AC_LANG_PROGRAM(
			[[
			  #include <stdarg.h>
			]],
			[[
			  va_list ap, ap2;
			  __va_copy(ap2, ap);
			]])],
			[local_ac_cv_func_va_copy=__va_copy],
			[local_ac_cv_func_va_copy=memcpy]
		)]
	)
])
AH_TEMPLATE([va_copy],
	[Define vararg lists copy function if necessary.])
case "${local_ac_cv_func_va_copy}" in
	va_copy)
		AC_MSG_RESULT([va_copy])
		;;
	__va_copy)
		AC_MSG_RESULT([__va_copy])
		AC_DEFINE([va_copy],
			[__va_copy])
		;;
	memcpy)
		AC_MSG_RESULT([memcpy])
		AC_DEFINE([va_copy(dst, src)],
			[memcpy(&(dst), &(src), sizeof(va_list))])
		;;
	*)
		dnl Cannot happen unless the cache value is corrupt.
		;;
esac
])
