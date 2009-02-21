SUBDIRS=common libmemo src doc
if WITH_CHECK
    SUBDIRS+=test
    DISTCHECK_CONFIGURE_FLAGS=--with-check
endif
if ENABLE_WEBSITE
    SUBDIRS+=website
endif
ACLOCAL_AMFLAGS=-I m4

## vim:ts=4:sw=4:et
