#!/bin/sh

# Minimalist example metadata script that has the behavior required by
# ezstream.
#
# From the current working directory, return the contents of metadata.txt,
# artist.txt, and title.txt.

test -z "${1}" && cat metadata.txt 2> /dev/null
test x"${1}" = "xartist" && cat artist.txt 2> /dev/null
test x"${1}" = "xtitle" && cat title.txt 2> /dev/null
test x"${1}" = "xalbum" && cat album.txt 2> /dev/null

exit 0
