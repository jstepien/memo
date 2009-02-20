# Copyright (C) 2008, 2009 Jan Stępień
#
# This file is part of Memo.
#
# Memo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Memo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Memo.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.61)
AC_INIT(memo, alpha, jstepien@users.sourceforge.net)
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

AC_PROG_CC
AC_PROG_AWK
AC_PROG_SED

AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2], , [AC_MSG_ERROR([cannot find dup2 function])])

AC_TYPE_SIZE_T

AX_LIB_SQLITE3
if test x$have_sqlite3 != xtrue; then
    AC_MSG_ERROR([cannot find SQLite3])
fi
LDFLAGS="$LDFLAGS $SQLITE3_LDFLAGS"
CFLAGS="$CFLAGS $SQLITE3_CFLAGS"

AC_ARG_WITH([check],
            AC_HELP_STRING([--with-check],
                           [enable Check tests run with 'make check' (default=no)]),
                           [case "${withval}" in
                              yes) with_check=true ;;
                              no)  with_check=false ;;
                              *) AC_MSG_ERROR([bad value '${withval}' for --with-check]) ;;
                            esac], [with_check=false])
AM_CONDITIONAL([WITH_CHECK], [test x$with_check = xtrue])

if test x$with_check = xtrue; then
    PKG_CHECK_MODULES([Check], [check >= 0.9.5],,
                      AC_MSG_ERROR([cannot find Check (required by --with-check)]))
fi

AC_PROG_LIBTOOL

DEF_CFLAGS='-DLIBDIR=\""${libdir}/memo\"" -DTOP_SRCDIR=\""${top_srcdir}\""'
CFLAGS="$CFLAGS $DEF_CFLAGS"

AC_CONFIG_FILES([Makefile
                 common/Makefile
                 libmemo/Makefile
                 src/Makefile
                 test/Makefile
                 doc/Makefile
                 Doxyfile])
AC_OUTPUT

# vim:ts=4:sw=4:et
