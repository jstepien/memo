lib_LTLIBRARIES=libmemo.la
libmemo_la_SOURCES=libmemo.h database.c database.h word.c messaging.c
libmemo_la_CFLAGS = -I$(srcdir)/../common/
libmemo_la_LIBADD = $(builddir)/../common/libcommon.la
pkginclude_HEADERS=libmemo.h
memo_libdir=$(libdir)/memo
dist_memo_lib_SCRIPTS=parse-reply.pl
