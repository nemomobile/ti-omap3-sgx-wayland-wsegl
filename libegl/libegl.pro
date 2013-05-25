TEMPLATE = lib
TARGET = EGL-proxy

system(wayland-scanner code < wayland-sgx.xml > wayland-sgx-protocol.c)
system(wayland-scanner server-header < wayland-sgx.xml > wayland-sgx-server-protocol.h)
system(wayland-scanner client-header < wayland-sgx.xml > wayland-sgx-client-protocol.h)

QT = 
SOURCES = egl.c \
          server_wlegl_buffer.cpp \
          server_wlegl.cpp \
          wayland-sgx-protocol.c

HEADERS = \
    server_wlegl_buffer.h \
    server_wlegl.h \
    server_wlegl_private.h \
    wayland-sgx-server-protocol.h \
    wayland-sgx-client-protocol.h

CONFIG += link_pkgconfig
PKGCONFIG += wayland-client wayland-server

LIBS += -ldl

target.path = /usr/lib
INSTALLS += target
