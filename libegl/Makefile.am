lib_LTLIBRARIES = libEGL.la

libEGL_la_SOURCES =	\
	egl.c

libEGL_la_CFLAGS = \
	-I$(top_srcdir)/common

libEGL_la_LDFLAGS = \
	-module \
	-no-undefined \
	-version-number @MESA_VERSION_MAJOR@:@MESA_VERSION_MINOR@:@MESA_VERSION_PATCH@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = egl.pc
