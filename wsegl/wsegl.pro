TARGET = waylandwsegl
TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix link_pkgconfig
CONFIG -= qt

LIBS += -lpvr2d ../libwayland-egl/libwayland-egl.so
PKGCONFIG += wayland-client wayland-server

SOURCES = waylandwsegl.c

include(../common/common.pri) | error("can't include common.pri")

INCLUDEPATH += ../libwayland-egl

target.path = /usr/lib
INSTALLS += target
