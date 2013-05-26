TEMPLATE = lib
TARGET = EGL-proxy

QT = 
SOURCES = egl.c


include(../common/common.pri)

CONFIG += link_pkgconfig
PKGCONFIG += wayland-client wayland-server

LIBS += -ldl

target.path = /usr/lib
INSTALLS += target
