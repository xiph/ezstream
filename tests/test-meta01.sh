#!/bin/sh

test -z "${1}" && echo "  songinfo  "
test x"${1}" = "xartist" && echo "  artist  "
test x"${1}" = "xtitle" && echo "  title  "
test x"${1}" = "xalbum" && echo "  album  "

exit 0
