libdir           = @plugindir@
lib_LTLIBRARIES  = smsgcm.la

smsgcm_la_CFLAGS  = -Wall $(BITLBEE_CFLAGS) $(GLIB_CFLAGS)
smsgcm_la_LDFLAGS = -module -avoid-version $(LIBCURL_LIBS) $(JANSSON_LIBS)
smsgcm_la_SOURCES = smsgcm.c smsgcm-http.c smsgcm-lib.c
