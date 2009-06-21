dnl # $Id$

dnl # Append or prepend items to a variable, but keep the items in the
dnl # variable unique.
dnl # Re-implemented from scratch after seeing XIPH_VAR_[PRE,AP]PEND, written
dnl # by Karl Heyes of the Icecast project, in action. The end result turned
dnl # out to be quite similar ...


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


dnl # AX_UNIQVAR_APPEND([VARIABLE], [ITEM-LIST])
dnl # AX_UNIQVAR_PREPEND([VARIABLE], [ITEM-LIST])


AC_DEFUN([AX_UNIQVAR_APPEND],
[
AC_REQUIRE([AC_PROG_FGREP])
for arg in $2
do
	if `echo " ${$1} x" | ${FGREP} -v -e " ${arg} " > /dev/null`; then
		if test -z "${$1}"; then
			$1="${arg}"
		else
			$1="${$1} ${arg}"
		fi
	fi
done
])


AC_DEFUN([AX_UNIQVAR_PREPEND],
[
AC_REQUIRE([AC_PROG_FGREP])
_prepend_args=""
for arg in $2
do
	_prepend_args="${arg} ${_prepend_args}"
done
for arg in ${_prepend_args}
do
	if `echo " ${$1} x" | ${FGREP} -v -e " ${arg} " > /dev/null`; then
		if test -z "${$1}"; then
			$1="${arg}"
		else
			$1="${arg} ${$1}"
		fi
	fi
done
])
