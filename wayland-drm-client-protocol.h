/*
 * Copyright © 2010 Kristian Høgsberg
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */


#ifndef DRM_CLIENT_PROTOCOL_H
#define DRM_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-util.h"

struct wl_client;

struct wl_drm;

extern const struct wl_interface wl_drm_interface;

struct wl_drm_listener {
	void (*device)(void *data,
		       struct wl_drm *wl_drm,
		       const char *name);
	void (*authenticated)(void *data,
			      struct wl_drm *wl_drm);
};

static inline int
wl_drm_add_listener(struct wl_drm *wl_drm,
		       const struct wl_drm_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) wl_drm,
				     (void (**)(void)) listener, data);
}

#define WL_DRM_AUTHENTICATE	0
#define WL_DRM_CREATE_BUFFER	1

static inline void
wl_drm_set_user_data(struct wl_drm *wl_drm, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) wl_drm, user_data);
}

static inline void *
wl_drm_get_user_data(struct wl_drm *wl_drm)
{
	return wl_proxy_get_user_data((struct wl_proxy *) wl_drm);
}

static inline void
wl_drm_destroy(struct wl_drm *wl_drm)
{
	wl_proxy_destroy((struct wl_proxy *) wl_drm);
}

static inline void
wl_drm_authenticate(struct wl_drm *wl_drm, uint32_t id)
{
	wl_proxy_marshal((struct wl_proxy *) wl_drm,
			 WL_DRM_AUTHENTICATE, id);
}

static inline struct wl_buffer *
wl_drm_create_buffer(struct wl_drm *wl_drm, uint32_t name, int width, int height, uint32_t stride, uint32_t format)
{
	struct wl_proxy *id;

	id = wl_proxy_create((struct wl_proxy *) wl_drm,
			     &wl_buffer_interface);
	if (!id)
		return NULL;

	wl_proxy_marshal((struct wl_proxy *) wl_drm,
			 WL_DRM_CREATE_BUFFER, id, name, width, height, stride, format);

	return (struct wl_buffer *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
