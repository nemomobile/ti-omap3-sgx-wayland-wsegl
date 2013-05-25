system(wayland-scanner code < wayland-sgx.xml > wayland-sgx-protocol.c)
system(wayland-scanner server-header < wayland-sgx.xml > wayland-sgx-server-protocol.h)
system(wayland-scanner client-header < wayland-sgx.xml > wayland-sgx-client-protocol.h)

SOURCES += $$PWD/wayland-sgx-protocol.c
HEADERS += $$PWD/wayland-sgx-server-protocol.h \
           $$PWD/wayland-sgx-client-protocol.h

           INCLUDEPATH += $$PWD


