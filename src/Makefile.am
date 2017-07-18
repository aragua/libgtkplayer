lib_LTLIBRARIES = liblibgtkplayer-@API_VERSION@.la

liblibgtkplayer_@API_VERSION@_la_SOURCES = \
	libgtkplayer-version.h \
	libgtkplayer.h \
	player.c \
	resources.c \
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

resources_file = $(srcdir)/resources/player.gresource.xml
BUILT_SOURCES = resources.c

resources.c: $(resources_file) $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir=$(srcdir)/resources $(resources_file) )
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir)/resources --generate-source $<

#-include $(top_srcdir)/git.mk
