## Process this file with automake to generate a Makefile.in.

NULL =

SUBDIRS = . cli

noinst_LIBRARIES = libab.a

libab_a_CPPFLAGS = \
        -I$(top_srcdir) \
        -I$(top_srcdir)/src \
	$(AM_CPPFLAGS) \
	$(NULL)

libab_a_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(NULL)

libab_a_SOURCES = \
	ab.c \
	ab.h \
	ab_contact.c \
	ab_contact.h \
	dirs.c \
	dirs.h \
	util.c \
	util.h \
	$(NULL)

libab_a_LIBADD = \
	$(top_builddir)/third_party/sqlite/libsqlite.a \
	$(DL_LIBS) \
	$(NULL)

bin_PROGRAMS = tribe

tribe_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(NULL)

tribe_SOURCES = \
	about_dialog.c \
	about_dialog.h \
	contact_editor.c \
	contact_editor.h \
	main.c \
	window.c \
	window.h \
	$(NULL)

tribe_LDADD = \
	libab.a \
	$(top_builddir)/third_party/sqlite/libsqlite.a \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
