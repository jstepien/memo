# It should be bin_PROGRAMS when this C implementation will be ready to
# replace the version written in bash.
noinst_PROGRAMS=memo
memo_SOURCES=main.c
AM_CFLAGS=-I$(srcdir)/../libmemo -I$(srcdir)/../common/
AM_LDFLAGS=-L$(builddir)/../libmemo -lmemo
memo_LDADD = $(builddir)/../common/libcommon.la
