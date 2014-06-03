lib_LTLIBRARIES = libwayland-egl.la

include $(top_srcdir)/common/common.mk

libwayland_egl_la_SOURCES =	\
	wayland-egl.c		\
	wayland-egl-priv.h	\
	$(COMMON_SOURCES)

libwayland_egl_la_CFLAGS = \
	-I$(top_srcdir)/common

libwayland_egl_la_LDFLAGS = \
	-module \
	-no-undefined \
	-version-number @MESA_VERSION_MAJOR@:@MESA_VERSION_MINOR@:@MESA_VERSION_PATCH@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = wayland-egl.pc
