/*
 * Copyright © 2012 Collabora, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <wayland-server.h>
#include "wayland-sgx-server-protocol.h"
#include "server_wlegl_private.h"
#include "server_wlegl_buffer.h"

static inline server_wlegl *
server_wlegl_from(struct wl_resource *resource)
{
	return reinterpret_cast<server_wlegl *>(resource->data);
}

static void
server_wlegl_create_buffer(struct wl_client *client,
			   struct wl_resource *resource,
			   uint32_t id,
			   int32_t width,
			   int32_t height,
			   int32_t stride,
			   int32_t format,
			   int32_t handle)
{
	server_wlegl *wlegl = server_wlegl_from(resource);
	server_wlegl_buffer *buffer;

	if (width < 1 || height < 1) {
		wl_resource_post_error(resource,
				       SGX_WLEGL_ERROR_BAD_VALUE,
				       "bad width (%d) or height (%d)",
				       width, height);
		return;
	}

	buffer = server_wlegl_buffer_create(id, width, height, stride,
					    format, handle, wlegl);
	if (!buffer) {
		wl_resource_post_error(resource,
				       SGX_WLEGL_ERROR_BAD_HANDLE,
				       "invalid native handle");
		return;
	}

	wl_client_add_resource(client, &buffer->base.resource);
}

static const struct sgx_wlegl_interface server_wlegl_impl = {
	server_wlegl_create_buffer,
};

static void
server_wlegl_bind(struct wl_client *client, void *data,
		  uint32_t version, uint32_t id)
{
	server_wlegl *wlegl = reinterpret_cast<server_wlegl *>(data);
	struct wl_resource *resource;

	resource = wl_client_add_object(client, &sgx_wlegl_interface,
					&server_wlegl_impl, id, wlegl);
}

server_wlegl *
server_wlegl_create(struct wl_display *display)
{
	struct server_wlegl *wlegl;
	int ret;

	wlegl = new server_wlegl;

	wlegl->display = display;
	wlegl->global = wl_display_add_global(display,
					      &sgx_wlegl_interface,
					      wlegl, server_wlegl_bind);

	return wlegl;
}

void
server_wlegl_destroy(server_wlegl *wlegl)
{
	/* FIXME: server_wlegl_buffer objects may exist */

	/* FIXME: remove global_ */

	/* Better to leak than expose dtor segfaults, the server
	 * supposedly exits soon after. */
	//LOGW("server_wlegl object leaked on UnbindWaylandDisplayWL");
	/* delete wlegl; */
}

