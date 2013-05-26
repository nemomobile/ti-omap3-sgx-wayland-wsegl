TEMPLATE = subdirs
SUBDIRS = wsegl libegl libwayland-egl

wsegl.depends = libwayland-egl
