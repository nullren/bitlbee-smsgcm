libdir           = @plugindir@
lib_LTLIBRARIES  = smsgcm.la

smsgcm_la_CFLAGS  = -Wall $(BITLBEE_CFLAGS) $(GLIB_CFLAGS) -ljansson
smsgcm_la_LDFLAGS = -module -avoid-version
smsgcm_la_SOURCES = smsgcm.c smsgcm-gnutls-creds.c smsgcm-lib.c
