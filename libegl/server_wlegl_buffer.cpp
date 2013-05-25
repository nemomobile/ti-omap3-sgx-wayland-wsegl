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
#include <assert.h>

#include "server_wlegl_buffer.h"
#include "server_wlegl_private.h"

static void
destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static const struct wl_buffer_interface server_wlegl_buffer_impl = {
	destroy
};

 #include <stddef.h>

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:>---the pointer to the member.
 * @type:>--the type of the container struct this is embedded in.
 * @member:>the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({ \
   const typeof( ((type *)0)->member ) *__mptr = (ptr); \
   (type *)( (char *)__mptr - offsetof(type,member) );})

server_wlegl_buffer *
server_wlegl_buffer_from(struct wl_buffer *buffer)
{
	if (buffer->resource.object.implementation !=
	    (void (**)(void)) &server_wlegl_buffer_impl)
		return NULL;

	return container_of(buffer, server_wlegl_buffer, base);
}

static void
server_wlegl_buffer_dtor(struct wl_resource *resource)
{
	struct wl_buffer *base =
		reinterpret_cast<struct wl_buffer*>(resource->data);
	server_wlegl_buffer *buffer = server_wlegl_buffer_from(base);
	delete buffer;
}

server_wlegl_buffer *
server_wlegl_buffer_create(uint32_t id,
			   int32_t width,
			   int32_t height,
			   int32_t stride,
			   int32_t format,
               int32_t handle,
               server_wlegl *wlegl)
{
	server_wlegl_buffer *buffer = new server_wlegl_buffer;
	int ret;

	memset(buffer, 0, sizeof(*buffer));

	buffer->wlegl = wlegl;

	buffer->base.resource.object.id = id;
	buffer->base.resource.object.interface = &wl_buffer_interface;
	buffer->base.resource.object.implementation =
		(void (**)(void)) &server_wlegl_buffer_impl;

	buffer->base.resource.data = &buffer->base;
	buffer->base.resource.destroy = server_wlegl_buffer_dtor;

	buffer->base.width = width;
	buffer->base.height = height;

	buffer->buf = remote_window_buffer_create(
		width, height, stride, format, handle);
	return buffer;
}

struct remote_window_buffer *remote_window_buffer_create(unsigned int width,
            unsigned int height,
            unsigned int stride,
            unsigned int format,
            int32_t handle)
{
    remote_window_buffer *t = new remote_window_buffer;
    t->width = width;
    t->height = height;
    t->stride = stride;
    t->format = format;
    t->handle = handle;
}

