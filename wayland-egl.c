#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <dlfcn.h>
#include <assert.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include "wayland-egl-priv.h"

WL_EGL_EXPORT void
wl_egl_window_resize(struct wl_egl_window *egl_window,
		     int width, int height,
		     int dx, int dy)
{
        if (width == 0 && height == 0)
        {
           width=400;
           height=400;
        }
	egl_window->width  = width;
	egl_window->height = height;
	egl_window->dx     = dx;
	egl_window->dy     = dy;
	egl_window->backBuffersValid = 0;
}

WL_EGL_EXPORT struct wl_egl_window *
wl_egl_window_create(struct wl_surface *surface,
		     int width, int height,
		     struct wl_visual *visual)
{
	struct wl_egl_window *egl_window;
 
	egl_window = malloc(sizeof *egl_window);
	if (!egl_window)
	return NULL;

        egl_window->header.type = WWSEGL_DRAWABLE_TYPE_WINDOW;
	egl_window->surface = surface;
	egl_window->visual  = visual;
	egl_window->display = NULL;
	egl_window->block_swap_buffers = 0;
	memset(egl_window->backBuffers, 0, sizeof(egl_window->backBuffers));
	egl_window->currentBackBuffer = -1;
	egl_window->backBuffersValid = 0;
	wl_egl_window_resize(egl_window, width, height, 0, 0);
	egl_window->attached_width  = 0;
	egl_window->attached_height = 0;
	egl_window->numFlipBuffers = 0;
        egl_window->usingFlipBuffers = 0;
	return egl_window;
}

WL_EGL_EXPORT void
wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
	free(egl_window);
}

WL_EGL_EXPORT void
wl_egl_window_get_attached_size(struct wl_egl_window *egl_window,
				int *width, int *height)
{
	if (width)
		*width = egl_window->attached_width;
	if (height)
		*height = egl_window->attached_height;
}

WL_EGL_EXPORT struct wl_egl_pixmap *
wl_egl_pixmap_create(int width, int height,
		     struct wl_visual *visual, uint32_t flags)
{
	struct wl_egl_pixmap *egl_pixmap;

	egl_pixmap = malloc(sizeof *egl_pixmap);
	if (egl_pixmap == NULL)
		return NULL;
        
        egl_pixmap->header.type  = WWSEGL_DRAWABLE_TYPE_PIXMAP;
	egl_pixmap->display = NULL;
	egl_pixmap->width   = width;
	egl_pixmap->height  = height;
	egl_pixmap->visual  = visual;
	egl_pixmap->name    = 0;
	egl_pixmap->stride  = 0;
	egl_pixmap->flags   = flags;

	return egl_pixmap;
}

WL_EGL_EXPORT void
wl_egl_pixmap_destroy(struct wl_egl_pixmap *egl_pixmap)
{
        assert(0);
	free(egl_pixmap);
}

WL_EGL_EXPORT struct wl_buffer *
wl_egl_pixmap_create_buffer(struct wl_egl_pixmap *egl_pixmap)
{
	if (egl_pixmap->name == 0)
		return NULL;

        assert(0);
        return NULL;
/*	return wl_drm_create_buffer(egl_display->drm, egl_pixmap->name,
				    egl_pixmap->width, egl_pixmap->height,
				    egl_pixmap->stride, egl_pixmap->visual); */
}
