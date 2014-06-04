/*
 * Copyright (c) 2012 Carsten Munk <carsten.munk@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* EGL function pointers */
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

/* TI headers don't define these */
#ifndef EGL_WAYLAND_BUFFER_WL
#define EGL_WAYLAND_BUFFER_WL    0x31D5 /* eglCreateImageKHR target */
#endif

#ifndef EGL_WAYLAND_PLANE_WL
#define EGL_WAYLAND_PLANE_WL     0x31D6 /* eglCreateImageKHR attribute */
#endif

#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#define WANT_WAYLAND

#ifdef WANT_WAYLAND
#include "server_wlegl.h"
#include "server_wlegl_buffer.h"
#endif

#include "log.h"

static void *_libegl = NULL;

static EGLint  (*_eglGetError)(void) = NULL;

static EGLDisplay  (*_eglGetDisplay)(EGLNativeDisplayType display_id) = NULL;
static EGLBoolean  (*_eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor) = NULL;
static EGLBoolean  (*_eglTerminate)(EGLDisplay dpy) = NULL;

static const char *  (*_eglQueryString)(EGLDisplay dpy, EGLint name) = NULL;

static EGLBoolean  (*_eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs,
		EGLint config_size, EGLint *num_config) = NULL;
static EGLBoolean  (*_eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list,
		EGLConfig *configs, EGLint config_size,
		EGLint *num_config) = NULL;
static EGLBoolean  (*_eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config,
		EGLint attribute, EGLint *value) = NULL;

static EGLSurface  (*_eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config,
		EGLNativeWindowType win,
		const EGLint *attrib_list) = NULL;
static EGLSurface  (*_eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config,
		const EGLint *attrib_list) = NULL;
static EGLSurface  (*_eglCreatePixmapSurface)(EGLDisplay dpy, EGLConfig config,
		EGLNativePixmapType pixmap,
		const EGLint *attrib_list) = NULL;
static EGLBoolean  (*_eglDestroySurface)(EGLDisplay dpy, EGLSurface surface) = NULL;
static EGLBoolean  (*_eglQuerySurface)(EGLDisplay dpy, EGLSurface surface,
		EGLint attribute, EGLint *value) = NULL;

static EGLBoolean  (*_eglBindAPI)(EGLenum api) = NULL;
static EGLenum  (*_eglQueryAPI)(void) = NULL;

static EGLBoolean  (*_eglWaitClient)(void) = NULL;

static EGLBoolean  (*_eglReleaseThread)(void) = NULL;

static EGLSurface  (*_eglCreatePbufferFromClientBuffer)(
		EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
		EGLConfig config, const EGLint *attrib_list) = NULL;

static EGLBoolean  (*_eglSurfaceAttrib)(EGLDisplay dpy, EGLSurface surface,
		EGLint attribute, EGLint value) = NULL;
static EGLBoolean  (*_eglBindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer) = NULL;
static EGLBoolean  (*_eglReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer) = NULL;


static EGLBoolean  (*_eglSwapInterval)(EGLDisplay dpy, EGLint interval) = NULL;


static EGLContext  (*_eglCreateContext)(EGLDisplay dpy, EGLConfig config,
		EGLContext share_context,
		const EGLint *attrib_list) = NULL;
static EGLBoolean  (*_eglDestroyContext)(EGLDisplay dpy, EGLContext ctx) = NULL;
static EGLBoolean  (*_eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw,
		EGLSurface read, EGLContext ctx) = NULL;

static EGLContext  (*_eglGetCurrentContext)(void) = NULL;
static EGLSurface  (*_eglGetCurrentSurface)(EGLint readdraw) = NULL;
static EGLDisplay  (*_eglGetCurrentDisplay)(void) = NULL;
static EGLBoolean  (*_eglQueryContext)(EGLDisplay dpy, EGLContext ctx,
		EGLint attribute, EGLint *value) = NULL;

static EGLBoolean  (*_eglWaitGL)(void) = NULL;
static EGLBoolean  (*_eglWaitNative)(EGLint engine) = NULL;
static EGLBoolean  (*_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = NULL;
static EGLBoolean  (*_eglCopyBuffers)(EGLDisplay dpy, EGLSurface surface,
		EGLNativePixmapType target) = NULL;


static EGLImageKHR (*_eglCreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list) = NULL;
static EGLBoolean (*_eglDestroyImageKHR) (EGLDisplay dpy, EGLImageKHR image) = NULL;

static __eglMustCastToProperFunctionPointerType (*_eglGetProcAddress)(const char *procname) = NULL;

static void _init_egl()
{
	_libegl = (void *) dlopen(getenv("LIBEGL") ? getenv("LIBEGL") : "/usr/lib/sgx/libEGL-sgx.so", RTLD_LAZY);
}

#define EGL_DLSYM(fptr, sym) do { if (_libegl == NULL) { _init_egl(); }; if (*(fptr) == NULL) { *(fptr) = (void *) dlsym(_libegl, sym); } } while (0) 

EGLint eglGetError(void)
{
	EGL_DLSYM(&_eglGetError, "eglGetError");
	return (*_eglGetError)();
}

EGLDisplay eglGetDisplay(EGLNativeDisplayType display_id)
{
	EGL_DLSYM(&_eglGetDisplay, "eglGetDisplay");
	return (*_eglGetDisplay)(display_id);
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
	wsegl_info("wayland-wsegl: eglInitialize");
	EGL_DLSYM(&_eglInitialize, "eglInitialize");
	return (*_eglInitialize)(dpy, major, minor);
}

EGLBoolean eglTerminate(EGLDisplay dpy)
{
	EGL_DLSYM(&_eglTerminate, "eglTerminate");
	return (*_eglTerminate)(dpy);
}

const char * eglQueryString(EGLDisplay dpy, EGLint name)
{
	EGL_DLSYM(&_eglQueryString, "eglQueryString");
#ifdef WANT_WAYLAND
	if (name == EGL_EXTENSIONS)
	{
		const char *ret = (*_eglQueryString)(dpy, name);
		static char eglextensionsbuf[512];
		assert(ret != NULL);
		snprintf(eglextensionsbuf, 510, "%sEGL_WL_bind_wayland_display ", ret);
		ret = eglextensionsbuf;
		return ret;
	}
#endif
	return (*_eglQueryString)(dpy, name);
}

EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig *configs,
		EGLint config_size, EGLint *num_config)
{
	EGL_DLSYM(&_eglGetConfigs, "eglGetConfigs");
	return (*_eglGetConfigs)(dpy, configs, config_size, num_config);
}

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list,
		EGLConfig *configs, EGLint config_size,
		EGLint *num_config)
{
	EGL_DLSYM(&_eglChooseConfig, "eglChooseConfig");
	return (*_eglChooseConfig)(dpy, attrib_list,
			configs, config_size,
			num_config);
}

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config,
		EGLint attribute, EGLint *value)
{
	EGL_DLSYM(&_eglGetConfigAttrib, "eglGetConfigAttrib");
	return (*_eglGetConfigAttrib)(dpy, config,
			attribute, value);
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
		EGLNativeWindowType win,
		const EGLint *attrib_list)
{
	EGL_DLSYM(&_eglCreateWindowSurface, "eglCreateWindowSurface");
	return (*_eglCreateWindowSurface)(dpy, config, win, attrib_list);
}

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
		const EGLint *attrib_list)
{
	EGL_DLSYM(&_eglCreatePbufferSurface, "eglCreatePbufferSurface");
	return (*_eglCreatePbufferSurface)(dpy, config, attrib_list);
}

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
		EGLNativePixmapType pixmap,
		const EGLint *attrib_list)
{
	EGL_DLSYM(&_eglCreatePixmapSurface, "eglCreatePixmapSurface");
	return (*_eglCreatePixmapSurface)(dpy, config, pixmap, attrib_list);
}

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
	EGL_DLSYM(&_eglDestroySurface, "eglDestroySurface");
	return (*_eglDestroySurface)(dpy, surface);
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
		EGLint attribute, EGLint *value)
{
	EGL_DLSYM(&_eglQuerySurface, "eglQuerySurface");
	return (*_eglQuerySurface)(dpy, surface, attribute, value);
}


EGLBoolean eglBindAPI(EGLenum api)
{
	EGL_DLSYM(&_eglBindAPI, "eglBindAPI");
	return (*_eglBindAPI)(api);
}

EGLenum eglQueryAPI(void)
{
	EGL_DLSYM(&_eglQueryAPI, "eglQueryAPI");
	return (*_eglQueryAPI)();
}

EGLBoolean eglWaitClient(void)
{
	EGL_DLSYM(&_eglWaitClient, "eglWaitClient");
	return (*_eglWaitClient)();
}

EGLBoolean eglReleaseThread(void)
{
	EGL_DLSYM(&_eglReleaseThread, "eglReleaseThread");
	return (*_eglReleaseThread)();
}

EGLSurface eglCreatePbufferFromClientBuffer(
		EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
		EGLConfig config, const EGLint *attrib_list)
{
	EGL_DLSYM(&_eglCreatePbufferFromClientBuffer, "eglCreatePbufferFromClientBuffer");
	return (*_eglCreatePbufferFromClientBuffer)(dpy, buftype, buffer, config, attrib_list);
}

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
		EGLint attribute, EGLint value)
{
	EGL_DLSYM(&_eglSurfaceAttrib, "eglSurfaceAttrib");
	return (*_eglSurfaceAttrib)(dpy, surface, attribute, value);
}

EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
	EGL_DLSYM(&_eglBindTexImage, "eglBindTexImage");
	return (*_eglBindTexImage)(dpy, surface, buffer);
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
	EGL_DLSYM(&_eglReleaseTexImage, "eglReleaseTexImage");
	return (*_eglReleaseTexImage)(dpy, surface, buffer);
}

EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
	EGL_DLSYM(&_eglSwapInterval, "eglSwapInterval");
	return (*_eglSwapInterval)(dpy, interval);
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config,
		EGLContext share_context,
		const EGLint *attrib_list)
{
	EGL_DLSYM(&_eglCreateContext, "eglCreateContext");
	return (*_eglCreateContext)(dpy, config, share_context, attrib_list);
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
	EGL_DLSYM(&_eglDestroyContext, "eglDestroyContext");
	return (*_eglDestroyContext)(dpy, ctx);
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw,
		EGLSurface read, EGLContext ctx)
{
	EGL_DLSYM(&_eglMakeCurrent, "eglMakeCurrent");
	return (*_eglMakeCurrent)(dpy, draw, read, ctx);
}

EGLContext eglGetCurrentContext(void)
{
	EGL_DLSYM(&_eglGetCurrentContext, "eglGetCurrentContext");
	return (*_eglGetCurrentContext)();
}

EGLSurface eglGetCurrentSurface(EGLint readdraw)
{
	EGL_DLSYM(&_eglGetCurrentSurface, "eglGetCurrentSurface");
	return (*_eglGetCurrentSurface)(readdraw);
}

EGLDisplay eglGetCurrentDisplay(void)
{
	EGL_DLSYM(&_eglGetCurrentDisplay, "eglGetCurrentDisplay");
	return (*_eglGetCurrentDisplay)();
}

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx,
		EGLint attribute, EGLint *value)
{
	EGL_DLSYM(&_eglQueryContext, "eglQueryContext");
	return (*_eglQueryContext)(dpy, ctx, attribute, value);
}

EGLBoolean eglWaitGL(void)
{
	EGL_DLSYM(&_eglWaitGL, "eglWaitGL");
	return (*_eglWaitGL)();
}

EGLBoolean eglWaitNative(EGLint engine)
{
	EGL_DLSYM(&_eglWaitNative, "eglWaitNative");
	return (*_eglWaitNative)(engine); 
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
	EGL_DLSYM(&_eglSwapBuffers, "eglSwapBuffers");
	return (*_eglSwapBuffers)(dpy, surface);
}

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface,
		EGLNativePixmapType target)
{
	EGL_DLSYM(&_eglCopyBuffers, "eglCopyBuffers");
	return (*_eglCopyBuffers)(dpy, surface, target);
}

static EGLImageKHR _my_eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
	if (_eglCreateImageKHR == NULL) {
		/* we can't EGL_DLSYM this, because it doesn't exist in
		 * SGX's libEGL. we also can't ask ourselves for the location of
		 * eglGetProcAddress, otherwise we'll end up calling ourselves again, so
		 * we must look up eglGetProcAddress first and ask SGX
		 */
		EGL_DLSYM(&_eglGetProcAddress, "eglGetProcAddress");
		_eglCreateImageKHR = (*_eglGetProcAddress)("eglCreateImageKHR");
	}

	if (target == EGL_WAYLAND_BUFFER_WL) {
		/* TI implementation does not know EGL_WAYLAND_PLANE_WL so remove it for now.
		 * In my test environment there was only plane 0 required (EGL_TEXTURE_RGB/EGL_TEXTURE_RGBA)
		 * so removing plane information is not mandatory */
		EGLint *attrib_source, *attrib_dest;
		int attribs_count = 0;
		int found_EGL_WAYLAND_PLANE_WL = 0;

		/* first check number of attributes */
		for(attrib_source = attrib_list; attrib_source != 0 && *attrib_source != EGL_NONE; ++attrib_source)
		{
			++attribs_count;
			if (*attrib_source == EGL_WAYLAND_PLANE_WL)
			{
				found_EGL_WAYLAND_PLANE_WL = 1;
				wsegl_debug("eglCreateImageKHR: removing attribute EGL_WAYLAND_PLANE_WL(%d)", *(attrib_source+1));
			}
		}
		/* rebuild without EGL_WAYLAND_PLANE_WL */
		if (found_EGL_WAYLAND_PLANE_WL && attribs_count >= 2)
			attribs_count -= 2;
		EGLint *attrib_list_new = malloc(sizeof(EGLint *) * (attribs_count+1)); /* terminate with EGL_NONE */
		assert(attrib_list_new != NULL);
		for(attrib_source = attrib_list, attrib_dest = attrib_list_new;
			attrib_source != 0 && *attrib_source != EGL_NONE;
			++attrib_source)
		{
			/* ignore EGL_WAYLAND_PLANE_WL + plane */
			if (*attrib_source == EGL_WAYLAND_PLANE_WL)
			{
				++attrib_source;
			}
			else
			{
				*attrib_dest = *attrib_source;
				++attrib_dest;
			}
		}
		*attrib_dest = EGL_NONE;

		EGLImageKHR ret = (*_eglCreateImageKHR)(dpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, buffer, attrib_list_new);
		free(attrib_list_new);
		return ret;
	}

	EGLImageKHR ret = (*_eglCreateImageKHR)(dpy, ctx, target, buffer, attrib_list);
	return ret;
}

#ifdef WANT_WAYLAND
static EGLBoolean _my_eglBindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display)
{
	server_wlegl_create(display);
}

static EGLBoolean _my_eglUnbindWaylandDisplayWL(EGLDisplay dpy, struct wl_display *display)
{
}

static EGLBoolean _my_eglQueryWaylandBufferWL(EGLDisplay dpy, struct wl_buffer *buffer, EGLint attribute, EGLint *value)
{
	struct server_wlegl_buffer *buf = server_wlegl_buffer_from(buffer);

	struct remote_window_buffer *awb = (struct RemoteWindowBuffer *)buf->buf;

	if (attribute == EGL_TEXTURE_FORMAT) {
		*value = awb->format;
		return EGL_TRUE;
	} else if (attribute == EGL_WIDTH) {
		*value = awb->width;
		return EGL_TRUE;
	} else if (attribute == EGL_HEIGHT) {
		*value = awb->height;
		return EGL_TRUE;
	}

	return EGL_FALSE;
}
#endif

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname)
{
	EGL_DLSYM(&_eglGetProcAddress, "eglGetProcAddress");
	if (strcmp(procname, "eglCreateImageKHR") == 0)
	{
		return _my_eglCreateImageKHR;
	} 
#ifdef WANT_WAYLAND
	else if (strcmp(procname, "eglBindWaylandDisplayWL") == 0)
	{
		return _my_eglBindWaylandDisplayWL;
	}
	else if (strcmp(procname, "eglUnbindWaylandDisplayWL") == 0)
	{
		return _my_eglUnbindWaylandDisplayWL;
	}
	else if (strcmp(procname, "eglQueryWaylandBufferWL") == 0)
	{
		return _my_eglQueryWaylandBufferWL;
	}
#endif
	return (*_eglGetProcAddress)(procname);
}

EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
	EGL_DLSYM(&_eglDestroyImageKHR, "eglDestroyImageKHR");
	return (*_eglDestroyImageKHR)(dpy, image);
}


// vim:ts=4:sw=4:noexpandtab
