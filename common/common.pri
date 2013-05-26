system(wayland-scanner code < wayland-sgx.xml > wayland-sgx-protocol.c)
system(wayland-scanner server-header < wayland-sgx.xml > wayland-sgx-server-protocol.h)
system(wayland-scanner client-header < wayland-sgx.xml > wayland-sgx-client-protocol.h)

SOURCES += $$PWD/wayland-sgx-protocol.c \
           $$PWD/server_wlegl_buffer.cpp \
           $$PWD/server_wlegl.cpp \
           $$PWD/log.c

HEADERS += $$PWD/wayland-sgx-server-protocol.h \
           $$PWD/wayland-sgx-client-protocol.h \
	   $$PWD/server_wlegl_buffer.h \
	   $$PWD/server_wlegl.h \
       $$PWD/server_wlegl_private.h \
       $$PWD/log.h

           INCLUDEPATH += $$PWD


