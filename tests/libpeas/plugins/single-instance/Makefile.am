include $(top_srcdir)/tests/Makefile.plugin

INCLUDES = \
	-I$(top_srcdir)		\
	$(PEAS_CFLAGS)		\
	$(WARN_CFLAGS)		\
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libsingle-instance.la

libsingle_instance_la_SOURCES = \
	single-instance-plugin.c	\
	single-instance-plugin.h

libsingle_instance_la_LDFLAGS = $(TEST_PLUGIN_LIBTOOL_FLAGS)
libsingle_instance_la_LIBADD  = $(PEAS_LIBS)

noinst_PLUGIN = single-instance.plugin

EXTRA_DIST = $(noinst_PLUGIN)
