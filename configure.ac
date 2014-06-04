# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([ti-omap3-sgx-wayland-wsegl], [0.2.0])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

# Initialize libtool
AC_PROG_LIBTOOL

# check for compilers
AC_PROG_CC
AC_PROG_CXX

# weston requires mesa version for wayland-egl
MESA_VERSION_MAJOR=10
MESA_VERSION_MINOR=1
MESA_VERSION_PATCH=2
AC_SUBST([MESA_VERSION_MAJOR])
AC_SUBST([MESA_VERSION_MINOR])
AC_SUBST([MESA_VERSION_PATCH])

# Checks for all headers/libraries/packages we need
AC_CHECK_HEADERS([EGL/egl.h EGL/eglext.h pvr2d.h wsegl.h])
AC_CHECK_LIB([pvr2d], [PVR2DEnumerateDevices], [PVR2D_LIB=-lpvr2d])
PKG_CHECK_MODULES(UDEV, libudev)
PKG_CHECK_MODULES(DRM, libdrm)
PKG_CHECK_MODULES(WAYLAND_CLIENT, wayland-client)
PKG_CHECK_MODULES(WAYLAND_SERVER, wayland-server)
AC_PATH_PROG([wayland_scanner], [wayland-scanner])
if test x$wayland_scanner = x; then
	AC_MSG_ERROR([wayland-scanner is needed to compile wayland-egl and wsegl])
fi

# variable substitutions for templates
AC_CONFIG_FILES([Makefile
		libegl/Makefile
		libegl/egl.pc
		libwayland-egl/Makefile
		libwayland-egl/wayland-egl.pc
		wsegl/Makefile])
AC_OUTPUT
