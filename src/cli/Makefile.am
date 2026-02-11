## Process this file with automake to generate a Makefile.in.

NULL =

bin_PROGRAMS = tribecli

tribecli_CPPFLAGS = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/src \
	$(AM_CPPFLAGS) \
	$(NULL)

tribecli_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(NULL)

tribecli_SOURCES = \
	cmd_add.c \
	cmd_delete.c \
	cmd_list.c \
	cmd_modify.c \
	commands.h \
	common.c \
	common.h \
	main.c \
	$(NULL)

tribecli_LDADD = \
	$(top_builddir)/src/libab.a \
	$(top_builddir)/third_party/libfort/libfort.a \
	$(top_builddir)/third_party/sqlite/libsqlite.a \
	$(PTHREAD_LIBS) \
	$(GLIB_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
