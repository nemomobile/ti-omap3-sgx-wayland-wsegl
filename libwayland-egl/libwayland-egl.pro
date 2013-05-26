TARGET = wayland-egl
TEMPLATE = lib
CONFIG += link_pkgconfig
CONFIG -= qt

PKGCONFIG += wayland-client

SOURCES = wayland-egl.c
HEADERS = wayland-egl-priv.h wsegl.h

include(../common/common.pri) | error("can't include common.pri")

target.path = /usr/lib
INSTALLS += target
