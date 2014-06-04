lib_LTLIBRARIES = libwaylandwsegl.la

include $(top_srcdir)/common/common.mk

libwaylandwsegl_la_SOURCES =	\
	waylandwsegl.c		\
	$(COMMON_SOURCES)

libwaylandwsegl_la_CFLAGS =	\
	-I$(top_srcdir)/common	\
	-I$(top_srcdir)/libwayland-egl

libwaylandwsegl_la_LIBADD =		\
	$(top_builddir)/libwayland-egl/libwayland-egl.la \
	$(PVR2D_LIB)

