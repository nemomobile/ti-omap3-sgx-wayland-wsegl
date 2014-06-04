lib_LTLIBRARIES = libwayland-egl.la

include $(top_srcdir)/common/common.mk

libwayland_egl_la_SOURCES =	\
	wayland-egl.c		\
	wayland-egl-priv.h	\
	$(COMMON_SOURCES)

libwayland_egl_la_CFLAGS = \
	-I$(top_srcdir)/common

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = wayland-egl.pc
