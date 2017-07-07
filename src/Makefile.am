lib_LTLIBRARIES = liblibgtkplayer-@API_VERSION@.la

liblibgtkplayer_@API_VERSION@_la_SOURCES = \
	libgtkplayer-version.h \
	libgtkplayer.h \
	player.c \
	$(NULL)

liblibgtkplayer_@API_VERSION@_la_CFLAGS = \
	$(LIBGTKPLAYER_CFLAGS) \
	$(WARN_CFLAGS) \
	$(NULL)

liblibgtkplayer_@API_VERSION@_la_LIBADD = \
	$(LIBGTKPLAYER_LIBS) \
	$(NULL)

liblibgtkplayer_@API_VERSION@_la_LDFLAGS = \
	$(WARN_LDFLAGS) \
	$(NULL)

bin_PROGRAMS = gtkplayer

gtkplayer_SOURCES = gtkplayer.c
gtkplayer_CFLAGS = $(GTKPLAYER_CFLAGS) $(LIBGTKPLAYER_CFLAGS) -g -O0
gtkplayer_LDFLAGS = $(GTKPLAYER_LDFLAGS) $(LIBGTKPLAYER_LIBS) -g -O0
gtkplayer_LDADD = $(GTKPLAYER_LIBS) liblibgtkplayer-@API_VERSION@.la

-include $(top_srcdir)/git.mk

-include $(top_srcdir)/git.mk
