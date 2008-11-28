#!/bin/sh
set -x
autoheader || exit 1
aclocal -I m4 || exit 1
autoconf || exit 1
libtoolize || exit 1
automake -a -c || exit 1
