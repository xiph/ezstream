#!/bin/sh

# Minimalist example metadata script that has the behavior required by
# ezstream.

test -z "${1}" && echo "Ezstream presents"
test x"${1}" = "xartist" && echo "Great Artist"
test x"${1}" = "xtitle" && echo "Great Song"
