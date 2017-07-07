lib_LTLIBRARIES = liblibgtkplayer-@API_VERSION@.la

liblibgtkplayer_@API_VERSION@_la_SOURCES = \
	libgtkplayer-version.h \
	libgtkplayer.h \
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

-include $(top_srcdir)/git.mk
