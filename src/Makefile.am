# It should be bin_PROGRAMS when this C implementation will be ready to
# replace the version written in bash.
noinst_PROGRAMS=memo
memo_SOURCES=main.c
AM_CFLAGS=-I$(top_srcdir)/libmemo -I$(top_srcdir)/common/
AM_LDFLAGS=-L$(top_srcdir)/libmemo -lmemo
memo_LDADD = $(top_srcdir)/common/libxmalloc.la
