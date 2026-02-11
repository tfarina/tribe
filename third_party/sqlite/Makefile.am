## Process this file with automake to generate a Makefile.in.

NULL =

noinst_LIBRARIES = libsqlite.a

libsqlite_a_CPPFLAGS = \
	$(PTHREAD_FLAGS) \
	$(NULL)

libsqlite_a_SOURCES = \
	sqlite3.c \
	sqlite3.h \
	$(NULL)

-include $(top_srcdir)/git.mk
