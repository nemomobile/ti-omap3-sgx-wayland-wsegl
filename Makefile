all: waylandwsegl.so libwayland-egl.so.1.0

install:
	mkdir -p $(INSTALL_ROOT)/usr/lib
	cp waylandwsegl.so $(INSTALL_ROOT)/usr/lib
	cp libwayland-egl.so.1.0 $(INSTALL_ROOT)/usr/lib
	ln -s libwayland-egl.so.1.0 $(INSTALL_ROOT)/usr/lib/libwayland-egl.so.1
	
libwayland-egl.so.1.0:
	gcc -g -shared -Wl,-soname,libwayland-egl.so.1 -DPIC -fPIC -o libwayland-egl.so.1.0 wayland-egl.c -lwayland-client

waylandwsegl.so: libwayland-egl.so.1.0
	gcc -g -shared -DPIC -fPIC -o waylandwsegl.so waylandwsegl.c wayland-drm-protocol.c -lpvr2d -lwayland-client -lwayland-server libwayland-egl.so.1.0

clean:
