SUBDIRS=common libmemo src doc
if WITH_CHECK
    SUBDIRS+=test
    DISTCHECK_CONFIGURE_FLAGS=--with-check
endif
bin_SCRIPTS=memo
memo_libdir=$(libdir)/memo
dist_memo_lib_DATA=general.sh globals.sh sending.sh receiving.sh
ACLOCAL_AMFLAGS=-I m4

## vim:ts=4:sw=4:et
