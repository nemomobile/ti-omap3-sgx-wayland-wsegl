TARGET = waylandwsegl
TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix link_pkgconfig
CONFIG -= qt

LIBS += -lpvr2d
PKGCONFIG += wayland-client wayland-server

SOURCES = waylandwsegl.c wayland-egl.c
HEADERS = wayland-egl-priv.h wsegl.h

target.path = /usr/lib
INSTALLS += target
