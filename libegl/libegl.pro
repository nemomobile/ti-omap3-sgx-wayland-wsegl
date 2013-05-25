TEMPLATE = lib
TARGET = EGL-proxy

QT = 
SOURCES = egl.c \
          server_wlegl_buffer.cpp \
          server_wlegl.cpp

HEADERS = \
    server_wlegl_buffer.h \
    server_wlegl.h \
    server_wlegl_private.h

include(../common/common.pri)

CONFIG += link_pkgconfig
PKGCONFIG += wayland-client wayland-server

LIBS += -ldl

target.path = /usr/lib
INSTALLS += target
