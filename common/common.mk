BUILT_SOURCES = \
	wayland-sgx-protocol.c				\
	wayland-sgx-client-protocol.h			\
	wayland-sgx-server-protocol.h

AUTOMAKE_OPTIONS = subdir-objects

COMMON_SOURCES =					\
	$(top_srcdir)/common/server_wlegl_buffer.cpp	\
	$(top_srcdir)/common/server_wlegl_buffer.h	\
	$(top_srcdir)/common/server_wlegl.cpp		\
	$(top_srcdir)/common/server_wlegl.h		\
	$(top_srcdir)/common/log.c			\
	$(top_srcdir)/common/log.h			\
       	$(top_srcdir)/common/server_wlegl_private.h	\
	$(BUILT_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

wayland_protocoldir = $(top_srcdir)/protocol
include $(wayland_protocoldir)/wayland-scanner.mk


