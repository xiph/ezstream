#!/bin/sh

PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"

if [ ! -f "./`basename $0`" ]; then
	echo "Please chdir into `basename $0`'s directory first."
	exit 1
fi

AUTOCONF_VERSION=2.61 AUTOMAKE_VERSION=1.9 aclocal -I m4
AUTOCONF_VERSION=2.61 AUTOMAKE_VERSION=1.9 autoconf
AUTOCONF_VERSION=2.61 AUTOMAKE_VERSION=1.9 autoheader
# AUTOCONF_VERSION=2.61 AUTOMAKE_VERSION=1.9 libtoolize --automake -c -f
AUTOCONF_VERSION=2.61 AUTOMAKE_VERSION=1.9 automake -a -c
rm -r autom4te.cache
