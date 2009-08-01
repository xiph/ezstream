#!/bin/sh

PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/usr/pkg/bin:/usr/pkg/sbin"

_ac_version="2.61"
_am_version="1.10"

if [ ! -f "./`basename $0`" ]; then
	echo "Please chdir into `basename $0`'s directory first."
	exit 1
fi

USE_LIBTOOL="$(grep AC_PROG_LIBTOOL ./configure.* 2> /dev/null)"

EXTRA=
if [ -d m4 ]; then
	EXTRA="-I m4"
fi

if [ -d /usr/local/share/aclocal ]; then
	EXTRA="${EXTRA} -I /usr/local/share/aclocal"
fi

AUTOCONF_VERSION="${_ac_version}" AUTOMAKE_VERSION="${_am_version}" \
	aclocal ${EXTRA} || exit 1
AUTOCONF_VERSION="${_ac_version}" AUTOMAKE_VERSION="${_am_version}" \
	autoconf || exit 1
AUTOCONF_VERSION="${_ac_version}" AUTOMAKE_VERSION="${_am_version}" \
	autoheader || exit 1
if [ -n "${USE_LIBTOOL}" ]; then
	AUTOCONF_VERSION="${_ac_version}" AUTOMAKE_VERSION="${_am_version}" \
		libtoolize --automake -c -f || exit 1
fi
AUTOCONF_VERSION="${_ac_version}" AUTOMAKE_VERSION="${_am_version}" \
	automake -a -c || exit 1

rm -r autom4te.cache

exit 0
