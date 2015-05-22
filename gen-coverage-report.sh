#!/bin/sh

PROGRAM="ezstream"

set -e

trap '' 1
trap 'exit 1' 2 15

BUILD_DATE=$(date '+%Y%m%d')
DESTDIR=$PROGRAM-coverage-$BUILD_DATE

if [ -z "$(which lcov)" ]; then
	echo "lcov is required" >&2
	exit 1
fi
if [ -z "$(which genhtml)" ]; then
	echo "genhtml is required" >&2
	exit 1
fi

rm -rf "$DESTDIR"

make distclean || :
./configure CFLAGS='-O0 -fprofile-arcs -ftest-coverage -fstack-protector-all'
make check

mkdir -p $DESTDIR
lcov --capture --output-file $DESTDIR/coverage.tmp \
	--rc lcov_branch_coverage=1 \
	--directory src \
    --test-name "Ezstream $VERSION"
genhtml --prefix . --output-directory $DESTDIR \
	--branch-coverage --function-coverage \
	--rc lcov_branch_coverage=1 \
    --title "Ezstream $VERSION" --legend --show-detail $DESTDIR/coverage.tmp

make distclean

echo "Coverage report available under $DESTDIR"
