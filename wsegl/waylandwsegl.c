/**
** Primary part of this is:
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception   
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.

** Copyright (C) 2011 Carsten Munk. All rights reserved
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#include "wayland-egl-priv.h"
#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <time.h>
#include "linux/omapfb.h"

#include "log.h"
#include "wayland-sgx-server-protocol.h"
#include "wayland-sgx-client-protocol.h"
#include "server_wlegl_buffer.h"

static WSEGLCaps const wseglDisplayCaps[] = {
    {WSEGL_CAP_WINDOWS_USE_HW_SYNC, 1},
    {WSEGL_CAP_PIXMAPS_USE_HW_SYNC, 1},
    {WSEGL_NO_CAPS, 0}
};


struct wl_egl_display*
wl_egl_display_create(struct wl_display *display)
{
	struct wl_egl_display *egl_display;

	egl_display = malloc(sizeof *egl_display);
	if (!egl_display)
		return NULL;
	egl_display->display = display;

	egl_display->fd = -1;
	egl_display->device_name = NULL;
	egl_display->authenticated = false;
	egl_display->context_refcnt = 0;
	egl_display->context = 0;

	egl_display->wseglDisplayConfigs[0].ui32DrawableType = WSEGL_DRAWABLE_WINDOW;
	egl_display->wseglDisplayConfigs[0].ePixelFormat = WSEGL_PIXELFORMAT_8888;
	egl_display->wseglDisplayConfigs[0].ulNativeRenderable = WSEGL_FALSE;
	egl_display->wseglDisplayConfigs[0].ulFrameBufferLevel = 0;
	egl_display->wseglDisplayConfigs[0].ulNativeVisualID = 0;
	egl_display->wseglDisplayConfigs[0].ulNativeVisualType = 0;
	egl_display->wseglDisplayConfigs[0].eTransparentType = WSEGL_OPAQUE;
        egl_display->wseglDisplayConfigs[0].ulTransparentColor = 0;
        	
	egl_display->wseglDisplayConfigs[1].ui32DrawableType = WSEGL_DRAWABLE_PIXMAP;
	egl_display->wseglDisplayConfigs[1].ePixelFormat = WSEGL_PIXELFORMAT_8888;
	egl_display->wseglDisplayConfigs[1].ulNativeRenderable = WSEGL_FALSE;
	egl_display->wseglDisplayConfigs[1].ulFrameBufferLevel = 0;
	egl_display->wseglDisplayConfigs[1].ulNativeVisualID = 0;
	egl_display->wseglDisplayConfigs[1].ulNativeVisualType = 0;
	egl_display->wseglDisplayConfigs[1].eTransparentType = WSEGL_OPAQUE;
        egl_display->wseglDisplayConfigs[1].ulTransparentColor = 0;

	egl_display->wseglDisplayConfigs[2].ui32DrawableType = WSEGL_NO_DRAWABLE;
	egl_display->wseglDisplayConfigs[2].ePixelFormat = 0;
	egl_display->wseglDisplayConfigs[2].ulNativeRenderable = 0;
	egl_display->wseglDisplayConfigs[2].ulFrameBufferLevel = 0;
	egl_display->wseglDisplayConfigs[2].ulNativeVisualID = 0;
	egl_display->wseglDisplayConfigs[2].ulNativeVisualType = 0;
	egl_display->wseglDisplayConfigs[2].eTransparentType = 0;
        egl_display->wseglDisplayConfigs[2].ulTransparentColor = 0;

	
        return egl_display;
}

WL_EGL_EXPORT void
wl_egl_display_destroy(struct wl_egl_display *egl_display)
{
	free(egl_display->device_name);
	close(egl_display->fd);

	free(egl_display);
}

/* PVR2D Context handling */
static int wseglFetchContext(struct wl_egl_display *nativeDisplay)
{
   int numDevs;
   PVR2DDEVICEINFO *devs;
   unsigned long devId;
   
   if (nativeDisplay->context_refcnt > 0)
     return 1;
   
   numDevs = PVR2DEnumerateDevices(0);
   if (numDevs <= 0)
     return 0;
     
   devs = (PVR2DDEVICEINFO *)malloc(sizeof(PVR2DDEVICEINFO) * numDevs);
   
   if (!devs)
     return 0;
     
   if (PVR2DEnumerateDevices(devs) != PVR2D_OK)
   {
     free(devs);
     return 0;
   }   
   
   devId = devs[0].ulDevID;
   free(devs);
   if (PVR2DCreateDeviceContext(devId, &nativeDisplay->context, 0) != PVR2D_OK)
     return 0;
   
   nativeDisplay->context_refcnt++;
   
   return 1;         
}

static void wseglReleaseContext(struct wl_egl_display *nativeDisplay)
{
   if (nativeDisplay->context_refcnt > 0)
   {
      nativeDisplay->context_refcnt--;
      if (nativeDisplay->context_refcnt == 0)
      {
         PVR2DDestroyDeviceContext(nativeDisplay->context);
         nativeDisplay->context = 0;
      }
   }
}

static PVR2DFORMAT wsegl2pvr2dformat(WSEGLPixelFormat format)
{
    switch (format)
    {
       case WSEGL_PIXELFORMAT_565:
          return PVR2D_RGB565;
       case WSEGL_PIXELFORMAT_4444:
          return PVR2D_ARGB4444;
       case WSEGL_PIXELFORMAT_8888:
          return PVR2D_ARGB8888;
       default:
          assert(0);
    }
}


/* Determine if nativeDisplay is a valid display handle */
static WSEGLError wseglIsDisplayValid(NativeDisplayType nativeDisplay)
{
  return WSEGL_SUCCESS;
}

/* Helper routines for pixel formats */
static WSEGLPixelFormat getwseglPixelFormat(struct wl_egl_display *egldisplay)
{
       if (egldisplay->var.bits_per_pixel == 16) {
          if (egldisplay->var.red.length == 5 && egldisplay->var.green.length == 6 &&
              egldisplay->var.blue.length == 5 && egldisplay->var.red.offset == 11 &&
              egldisplay->var.green.offset == 5 && egldisplay->var.blue.offset == 0) {
              return WSEGL_PIXELFORMAT_565;
          }
          if (egldisplay->var.red.length == 4 && egldisplay->var.green.length == 4 &&
              egldisplay->var.blue.length == 4 && egldisplay->var.transp.length == 4 &&
              egldisplay->var.red.offset == 8 && egldisplay->var.green.offset == 4 &&
              egldisplay->var.blue.offset == 0 && egldisplay->var.transp.offset == 12) {
              return WSEGL_PIXELFORMAT_4444;
          }
       } else if (egldisplay->var.bits_per_pixel == 32) {
          if (egldisplay->var.red.length == 8 && egldisplay->var.green.length == 8 &&
              egldisplay->var.blue.length == 8 && egldisplay->var.transp.length == 8 &&
              egldisplay->var.red.offset == 16 && egldisplay->var.green.offset == 8 &&
              egldisplay->var.blue.offset == 0 && egldisplay->var.transp.offset == 24) {
              return WSEGL_PIXELFORMAT_8888;
          }
       }
       else
        assert(0);  
    return WSEGL_SUCCESS;
}

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
    const char *interface, uint32_t version)
{
    struct wl_egl_display *egldisplay = (struct wl_egl_display *)data;

    if (strcmp(interface, "sgx_wlegl") == 0) {
        egldisplay->sgx_wlegl = (struct sgx_wlegl *)wl_registry_bind(registry, name, &sgx_wlegl_interface, 1);
    }
}

static const struct wl_registry_listener registry_listener = {
    registry_handle_global
};

static void roundtrip_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
   int *done = (int *)data;

   *done = 1;
   wl_callback_destroy(callback);
}

static const struct wl_callback_listener roundtrip_listener = {
   roundtrip_callback
};

int wayland_roundtrip(struct wl_egl_display *display)
{
    struct wl_callback *callback;
    int done = 0, ret = 0;
    wl_display_dispatch_queue_pending(display->display, display->queue);

    callback = wl_display_sync(display->display);
    wl_callback_add_listener(callback, &roundtrip_listener, &done);
    wl_proxy_set_queue((struct wl_proxy *)callback, display->queue);
    while (ret == 0 && !done)
        ret = wl_display_dispatch_queue(display->display, display->queue);

    return ret;
}


/* Initialize a native display for use with WSEGL */
static WSEGLError wseglInitializeDisplay
    (NativeDisplayType nativeDisplay, WSEGLDisplayHandle *display,
     const WSEGLCaps **caps, WSEGLConfig **configs)
{
    struct wl_egl_display *egldisplay = wl_egl_display_create((struct wl_display *) nativeDisplay);

    if (wseglFetchContext(egldisplay) != 1)
    {
       wl_egl_display_destroy(egldisplay);
       return WSEGL_OUT_OF_MEMORY;
    }

    /* If it is a framebuffer */
    if (egldisplay->display == NULL)
    {
        wsegl_info("wayland-wsegl: Initializing framebuffer");
       int fd;
       WSEGLPixelFormat format;
       
       /* Open the framebuffer and fetch its properties */
       fd = open("/dev/fb0", O_RDWR, 0);
       if (fd < 0) {
          perror("/dev/fb0");
          wseglReleaseContext(egldisplay);
          wl_egl_display_destroy(egldisplay);
          return WSEGL_CANNOT_INITIALISE;
       }
       if (ioctl(fd, FBIOGET_VSCREENINFO, &egldisplay->var) < 0) {
          perror("FBIOGET_VSCREENINFO");
          wseglReleaseContext(egldisplay);
          wl_egl_display_destroy(egldisplay);
          close(fd);
          return WSEGL_CANNOT_INITIALISE; 
       }
       if (ioctl(fd, FBIOGET_FSCREENINFO, &egldisplay->fix) < 0) {
          perror("FBIOGET_FSCREENINFO");
          wseglReleaseContext(egldisplay);
          wl_egl_display_destroy(egldisplay);
          close(fd);
          return WSEGL_CANNOT_INITIALISE; 
       }
       egldisplay->fd = fd;
       format = getwseglPixelFormat(egldisplay);

       egldisplay->wseglDisplayConfigs[0].ePixelFormat = format;
       egldisplay->wseglDisplayConfigs[1].ePixelFormat = format;
    }
    else
    {
        egldisplay->queue = wl_display_create_queue(nativeDisplay);
        egldisplay->frame_callback = NULL;
        egldisplay->registry = wl_display_get_registry(nativeDisplay);
        wl_proxy_set_queue(egldisplay->registry, egldisplay->queue);
        wl_registry_add_listener(egldisplay->registry, &registry_listener, egldisplay);

        assert(wayland_roundtrip(egldisplay) >= 0);
        assert(egldisplay->sgx_wlegl);
    }

    *display = (WSEGLDisplayHandle)egldisplay;
    *caps = wseglDisplayCaps;
    *configs = egldisplay->wseglDisplayConfigs;
    return WSEGL_SUCCESS;
}

/* Close the WSEGL display */
static WSEGLError wseglCloseDisplay(WSEGLDisplayHandle display)
{
   struct wl_egl_display *egldisplay = (struct wl_egl_display *) display;
   wseglReleaseContext(egldisplay);
   assert(egldisplay->context == 0);
   wl_egl_display_destroy(egldisplay);
   
   return WSEGL_SUCCESS;
}

static WSEGLError allocateBackBuffers(struct wl_egl_display *egldisplay, NativeWindowType nativeWindow)
{
    int index;
    
    if (nativeWindow->numFlipBuffers)
    {
        long stride = 0;
        
        unsigned long flipId = 0;
        unsigned long numBuffers;
        PVR2DERROR ret = PVR2DCreateFlipChain(egldisplay->context, 0,
                                 //PVR2D_CREATE_FLIPCHAIN_SHARED |
                                 //PVR2D_CREATE_FLIPCHAIN_QUERY,
                                 nativeWindow->numFlipBuffers,
                                 nativeWindow->width,
                                 nativeWindow->height,
                                 wsegl2pvr2dformat(nativeWindow->format),
                                 &stride, &flipId, &(nativeWindow->flipChain));
        assert(ret == PVR2D_OK); 
        PVR2DGetFlipChainBuffers(egldisplay->context,
                                     nativeWindow->flipChain,
                                     &numBuffers,
                                     nativeWindow->flipBuffers);
        for (index = 0; index < numBuffers; ++index)
        {
             nativeWindow->backBuffers[index] = nativeWindow->flipBuffers[index];
        }
    }
    else {
	    for (index = 0; index < WAYLANDWSEGL_MAX_BACK_BUFFERS; ++index)
	    {
		if (PVR2DMemAlloc(egldisplay->context,
			      nativeWindow->strideBytes * nativeWindow->height,
	 		      128, 0,
			      &(nativeWindow->backBuffers[index])) != PVR2D_OK)
	        {
		       assert(0);
					       
		       while (--index >= 0)
				PVR2DMemFree(egldisplay->context, nativeWindow->backBuffers[index]);
			       
	               memset(nativeWindow->backBuffers, 0, sizeof(nativeWindow->backBuffers));
		       
		       nativeWindow->backBuffersValid = 0;
   		       
			       
		       return WSEGL_OUT_OF_MEMORY;
	        }
	    }
    }
    nativeWindow->backBuffersValid = 1;
    nativeWindow->currentBackBuffer = 0;
    return WSEGL_SUCCESS;
}


int wseglPixelFormatBytesPP(WSEGLPixelFormat format)
{
    if (format == WSEGL_PIXELFORMAT_565)
       return 2;
    else
    if (format == WSEGL_PIXELFORMAT_8888)
       return 4;
    else
       assert(0);
   
}


/* Create the WSEGL drawable version of a native window */
static WSEGLError wseglCreateWindowDrawable
    (WSEGLDisplayHandle display, WSEGLConfig *config,
     WSEGLDrawableHandle *drawable, NativeWindowType nativeWindow,
     WSEGLRotationAngle *rotationAngle)
{
    struct wl_egl_display *egldisplay = (struct wl_egl_display *) display;
    int index;
    /* Framebuffer */
    if (nativeWindow == NULL)
    {
       PVR2DDISPLAYINFO displayInfo;

       assert(egldisplay->display == NULL);

       /* Let's create a fake wl_egl_window to simplify code */

       nativeWindow = wl_egl_window_create(NULL, egldisplay->var.xres, egldisplay->var.yres);
       nativeWindow->format = getwseglPixelFormat(egldisplay);
       nativeWindow->display = egldisplay;

       assert(PVR2DGetDeviceInfo(egldisplay->context, &displayInfo) == PVR2D_OK);

       wsegl_debug("ulMaxFlipChains: %lu", displayInfo.ulMaxFlipChains);
       wsegl_debug("ulMaxBuffersInChain: %lu", displayInfo.ulMaxBuffersInChain);
       wsegl_debug("eFormat: %d", displayInfo.eFormat);
       wsegl_debug("ulWidth: %lu", displayInfo.ulWidth);
       wsegl_debug("ulHeight: %lu", displayInfo.ulHeight);
       wsegl_debug("lStride: %lu", displayInfo.lStride);
       wsegl_debug("ulMinFlipInterval: %lu", displayInfo.ulMinFlipInterval);
       wsegl_debug("ulMaxFlipInterval: %lu", displayInfo.ulMaxFlipInterval);

       if (displayInfo.ulMaxFlipChains > 0 && displayInfo.ulMaxBuffersInChain > 0)
              nativeWindow->numFlipBuffers = displayInfo.ulMaxBuffersInChain;
       if (nativeWindow->numFlipBuffers > WAYLANDWSEGL_MAX_FLIP_BUFFERS)
              nativeWindow->numFlipBuffers = WAYLANDWSEGL_MAX_FLIP_BUFFERS;

       /* Workaround for broken devices, seen in debugging */
//       if (nativeWindow->numFlipBuffers < 2)
              nativeWindow->numFlipBuffers = 0;
    }
    else
    {
       nativeWindow->display = egldisplay;
       nativeWindow->format = config->ePixelFormat;
    }

    /* We can't do empty buffers, so let's make a 8x8 one. */
    if (nativeWindow->width == 0 || nativeWindow->height == 0)
    {
        nativeWindow->width = nativeWindow->height = 8;
    }


    /* If we don't have back buffers allocated already */
    if (!(nativeWindow->backBuffers[0] && nativeWindow->backBuffersValid))
    {
       nativeWindow->stridePixels = (nativeWindow->width + 7) & ~7; 
       nativeWindow->strideBytes = nativeWindow->stridePixels * wseglPixelFormatBytesPP(nativeWindow->format);

       if (allocateBackBuffers(egldisplay, nativeWindow) == WSEGL_OUT_OF_MEMORY)
          return WSEGL_OUT_OF_MEMORY;

       /* Wayland window */  
       if (nativeWindow->display->display != NULL)
       {
            for (index = 0; index < WAYLANDWSEGL_MAX_BACK_BUFFERS; index++)
            {
              PVR2D_HANDLE name;

              assert(PVR2DMemExport(egldisplay->context, 0, nativeWindow->backBuffers[index], &name) == PVR2D_OK);
              nativeWindow->exporthandles[index] = name;

              // TODO: clear exporthandles up
            }
       }
       /* Framebuffer */
       else
       {
           /* XXX should assert something about stride etc.. */
           assert(PVR2DGetFrameBuffer(egldisplay->context, PVR2D_FB_PRIMARY_SURFACE, &nativeWindow->frontBufferPVRMEM) == PVR2D_OK);
           // nativeWindow->frontBuffer = (void *) nativeWindow->frontBufferPVRMEM->pBase;
       }
    }      
  
    *drawable = (WSEGLDrawableHandle) nativeWindow; /* Reuse the egldisplay */
    *rotationAngle = WSEGL_ROTATE_0;
    return WSEGL_SUCCESS;
}

/* Create the WSEGL drawable version of a native pixmap */
static WSEGLError wseglCreatePixmapDrawable
    (WSEGLDisplayHandle display, WSEGLConfig *config,
     WSEGLDrawableHandle *drawable, NativePixmapType nativePixmap,
     WSEGLRotationAngle *rotationAngle)
{
    struct wl_egl_display *egldisplay = (struct wl_egl_display *) display;
    struct server_wlegl_buffer *buffer = (struct server_wlegl_buffer *)nativePixmap;

    struct wl_egl_pixmap *pixmap = wl_egl_pixmap_create(buffer->buf->width, buffer->buf->height, 0);
    pixmap->display = egldisplay;
    pixmap->stride = buffer->buf->stride;
    pixmap->handle = buffer->buf->handle;
    pixmap->format = buffer->buf->format;
    assert(PVR2DMemMap(egldisplay->context, 0, (void *)pixmap->handle, &pixmap->pvrmem) == PVR2D_OK);
    *drawable = (WSEGLDrawableHandle) pixmap;
    *rotationAngle = WSEGL_ROTATE_0;
    return WSEGL_SUCCESS;
}

/* Delete a specific drawable */
static WSEGLError wseglDeleteDrawable(WSEGLDrawableHandle _drawable)
{
    struct wl_egl_window *drawable = (struct wl_egl_window *) _drawable;

    int index;
    int numBuffers = WAYLANDWSEGL_MAX_BACK_BUFFERS;

    if (drawable->header.type == WWSEGL_DRAWABLE_TYPE_WINDOW) {
        for (index = 0; index < numBuffers; ++index) {
            if (drawable->drmbuffers[index])
                wl_buffer_destroy(drawable->drmbuffers[index]);
            if (drawable->backBuffers[index])
                PVR2DMemFree(drawable->display->context, drawable->backBuffers[index]);
        }

        memset(drawable->drmbuffers, 0, sizeof(drawable->drmbuffers));
        memset(drawable->backBuffers, 0, sizeof(drawable->backBuffers));
        drawable->backBuffersValid = 0;
        return WSEGL_SUCCESS;
    } else if (drawable->header.type == WWSEGL_DRAWABLE_TYPE_PIXMAP) {
        struct wl_egl_pixmap *pixmap = (struct wl_egl_pixmap *)drawable;
        PVR2DMemFree(pixmap->display->context, pixmap->pvrmem);
    } else {
        assert(0);
    }

    return WSEGL_SUCCESS;
}

static void
wayland_frame_callback(void *data, struct wl_callback *callback, uint32_t time)
{
    //wsegl_info("wayland-wsegl: wayland_frame_callback");
    struct wl_egl_window *drawable = (struct wl_egl_window *)data;
    drawable->display->frame_callback = NULL;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener frame_listener = {
    wayland_frame_callback
};

/* Swap the contents of a drawable to the screen */
static WSEGLError wseglSwapDrawable
    (WSEGLDrawableHandle _drawable, unsigned long data)
{
    struct wl_egl_window *drawable = (struct wl_egl_window *) _drawable;
    struct wl_callback *callback;

    if (drawable->numFlipBuffers)
    {
//        wsegl_info("PRESENT FLIP");
        PVR2DPresentFlip(drawable->display->context, drawable->flipChain, drawable->backBuffers[drawable->currentBackBuffer], 0);
    }
    else if (drawable->display->display)
    { 
        //wsegl_info("wseglSwapDrawable for wayland, %d %p", drawable->currentBackBuffer, drawable->drmbuffers[drawable->currentBackBuffer]);

        int ret = 0;
        while (drawable->display->frame_callback && ret != -1)
            ret = wl_display_dispatch_queue(drawable->display->display, drawable->display->queue);

        drawable->display->frame_callback = wl_surface_frame(drawable->surface);
        wl_callback_add_listener(drawable->display->frame_callback, &frame_listener, drawable);
        wl_proxy_set_queue((struct wl_proxy *)drawable->display->frame_callback, drawable->display->queue);

        if (!drawable->drmbuffers[drawable->currentBackBuffer])
        {
            int32_t handle;
            struct wl_buffer *wlbuf;

            handle = drawable->exporthandles[drawable->currentBackBuffer];
            wlbuf = sgx_wlegl_create_buffer(drawable->display->sgx_wlegl,
                        drawable->width, drawable->height, drawable->strideBytes,
                        drawable->format, handle);
            drawable->drmbuffers[drawable->currentBackBuffer] = wlbuf;
            wsegl_info("sgx_wlegl_create_buffer for %d", drawable->currentBackBuffer);

            wsegl_info("Add listener for %p with %p (buf %d) inside", drawable, wlbuf, drawable->currentBackBuffer);

            // TODO: listen for release

            wl_proxy_set_queue((struct wl_proxy *)wlbuf, drawable->display->queue);
        }

        struct wl_buffer *wlbuf = drawable->drmbuffers[drawable->currentBackBuffer];
        wl_surface_attach(drawable->surface, wlbuf, 0, 0); 
        wl_surface_damage(drawable->surface, 0, 0, drawable->width, drawable->height);
        wl_surface_commit(drawable->surface);
    }
    else
    {
       PVR2DBLTINFO blit;

       memset(&blit, 0, sizeof(blit));
    
       blit.CopyCode = PVR2DROPcopy;
       blit.BlitFlags = PVR2D_BLIT_DISABLE_ALL;
       blit.pSrcMemInfo = drawable->backBuffers[drawable->currentBackBuffer];
       blit.SrcStride = drawable->strideBytes;
       blit.SrcX = 0;
       blit.SrcY = 0;
       blit.SizeX = drawable->width;
       blit.SizeY = drawable->height;
       blit.SrcFormat = wsegl2pvr2dformat(drawable->format);

       blit.pDstMemInfo = drawable->frontBufferPVRMEM;
       blit.DstStride = drawable->strideBytes; 
       blit.DstX = 0;
       blit.DstY = 0;
       blit.DSizeX = drawable->width;
       blit.DSizeY = drawable->height;
       blit.DstFormat = wsegl2pvr2dformat(drawable->format);
       PVR2DBlt(drawable->display->context, &blit); 
       PVR2DQueryBlitsComplete
          (drawable->display->context, drawable->frontBufferPVRMEM, 1);                      
       assert (drawable->display->fd >= 0);


       struct omapfb_update_window update_window;
         
       update_window.x = update_window.out_x = 0;
       update_window.y = update_window.out_y = 0;
       update_window.width = update_window.out_width = drawable->width;
       update_window.height = update_window.out_height = drawable->height;
       update_window.format = 0;

       assert(ioctl(drawable->display->fd, OMAPFB_UPDATE_WINDOW, &update_window) == 0);
    }
    
    drawable->currentBackBuffer   
      = (drawable->currentBackBuffer + 1) % WAYLANDWSEGL_MAX_BACK_BUFFERS;

    return WSEGL_SUCCESS;
}

/* Set the swap interval of a window drawable */
static WSEGLError wseglSwapControlInterval
    (WSEGLDrawableHandle drawable, unsigned long interval)
{
    return WSEGL_SUCCESS;
}

 /* Flush native rendering requests on a drawable */
static WSEGLError wseglWaitNative
    (WSEGLDrawableHandle drawable, unsigned long engine)
{
    return WSEGL_SUCCESS;
}

/* Copy color data from a drawable to a native pixmap */
static WSEGLError wseglCopyFromDrawable
    (WSEGLDrawableHandle _drawable, NativePixmapType nativePixmap)
{
    return WSEGL_SUCCESS;
}

/* Copy color data from a PBuffer to a native pixmap */
static WSEGLError wseglCopyFromPBuffer
    (void *address, unsigned long width, unsigned long height,
     unsigned long stride, WSEGLPixelFormat format,
     NativePixmapType nativePixmap)
{
    return WSEGL_SUCCESS;
}

static int wseglGetBuffers(struct wl_egl_window *drawable, PVR2DMEMINFO **source, PVR2DMEMINFO **render)
{
  if (!drawable->backBuffersValid)
      return 0;
  *render = drawable->backBuffers[drawable->currentBackBuffer];
  *source = drawable->backBuffers
  [(drawable->currentBackBuffer + WAYLANDWSEGL_MAX_BACK_BUFFERS - 1) %
                 WAYLANDWSEGL_MAX_BACK_BUFFERS];
  return 1;
}                                                   


/* Return the parameters of a drawable that are needed by the EGL layer */
static WSEGLError wseglGetDrawableParameters
    (WSEGLDrawableHandle _drawable, WSEGLDrawableParams *sourceParams,
     WSEGLDrawableParams *renderParams,unsigned long ulPlaneOffset)
     {
/*
 * [22:26:17] <Stskeeps> note: you'll need this in future:
 * [22:26:19] <Stskeeps> if static WSEGLError wseglGetDrawableParameters
 * [22:26:19] <Stskeeps> 651
 * [22:26:33] <Stskeeps> returns return WSEGL_BAD_DRAWABLE;
 * [22:26:38] <Stskeeps> it'll re-create the drawable
 * [22:26:50] <Stskeeps> do this when window format,  height,  width has changed
 */
    struct wl_egl_window *eglwindow = (struct wl_egl_window *) _drawable;
    PVR2DMEMINFO *source, *render;

    WSEGL_UNREFERENCED_PARAMETER(ulPlaneOffset);

    memset(renderParams, 0, sizeof(*renderParams));
    memset(sourceParams, 0, sizeof(*sourceParams));

    if (eglwindow->header.type == WWSEGL_DRAWABLE_TYPE_PIXMAP)
    {
        struct wl_egl_pixmap *pixmap = (struct wl_egl_pixmap *) _drawable;

        int strideDiv = 0;

        switch (pixmap->format) {
            case WSEGL_PIXELFORMAT_565:
                strideDiv = 2;
                break;
            case WSEGL_PIXELFORMAT_8888:
                strideDiv = 4;
                break;
        }

        assert(strideDiv != 0);

        sourceParams->ui32Width = pixmap->width;
        sourceParams->ui32Height = pixmap->height;
        sourceParams->ui32Stride = pixmap->stride / strideDiv;
        sourceParams->ePixelFormat = pixmap->format;
        sourceParams->pvLinearAddress = pixmap->pvrmem->pBase;
        sourceParams->ui32HWAddress = pixmap->pvrmem->ui32DevAddr;
        sourceParams->hMemInfo = pixmap->pvrmem->hPrivateData;

        renderParams->ui32Width = pixmap->width;
        renderParams->ui32Height = pixmap->height;
        renderParams->ui32Stride = pixmap->stride / strideDiv;
        renderParams->ePixelFormat = pixmap->format;
        renderParams->pvLinearAddress = pixmap->pvrmem->pBase;
        renderParams->ui32HWAddress = pixmap->pvrmem->ui32DevAddr;
        renderParams->hMemInfo = pixmap->pvrmem->hPrivateData;

        return WSEGL_SUCCESS;
    }

    if (!wseglGetBuffers(eglwindow, &source, &render))
    {
       return WSEGL_BAD_DRAWABLE;
    }
    
    sourceParams->ui32Width = eglwindow->width;
    sourceParams->ui32Height = eglwindow->height;
    sourceParams->ui32Stride = eglwindow->stridePixels;
    sourceParams->ePixelFormat = eglwindow->format;   
    sourceParams->pvLinearAddress = source->pBase;
    sourceParams->ui32HWAddress = source->ui32DevAddr;
    sourceParams->hMemInfo = source->hPrivateData;

    renderParams->ui32Width = eglwindow->width;
    renderParams->ui32Height = eglwindow->height;
    renderParams->ui32Stride = eglwindow->stridePixels;
    renderParams->ePixelFormat = eglwindow->format;
    renderParams->pvLinearAddress = render->pBase;
    renderParams->ui32HWAddress = render->ui32DevAddr;
    renderParams->hMemInfo = render->hPrivateData;

    return WSEGL_SUCCESS;

}


/* Function stub for ConnectDrawable() */
static WSEGLError wseglConnectDrawable(WSEGLDrawableHandle hDrawable)
{
    WSEGL_UNREFERENCED_PARAMETER(hDrawable);
    return WSEGL_SUCCESS;
}

/* Function stub for DisconnectDrawable() */
static WSEGLError wseglDisconnectDrawable(WSEGLDrawableHandle hDrawable)
{
    WSEGL_UNREFERENCED_PARAMETER(hDrawable);
    return WSEGL_SUCCESS;
}

/* Function stub for FlagStartFrame() */
static WSEGLError wseglFlagStartFrame(void)
{
    return WSEGL_SUCCESS;
}

static WSEGL_FunctionTable const wseglFunctions = {
    WSEGL_VERSION,
    wseglIsDisplayValid,
    wseglInitializeDisplay,
    wseglCloseDisplay,
    wseglCreateWindowDrawable,
    wseglCreatePixmapDrawable,
    wseglDeleteDrawable,
    wseglSwapDrawable,
    wseglSwapControlInterval,
    wseglWaitNative,
    wseglCopyFromDrawable,
    wseglCopyFromPBuffer,
    wseglGetDrawableParameters,
    wseglConnectDrawable,
    wseglDisconnectDrawable,
    wseglFlagStartFrame
};

/* Return the table of WSEGL functions to the EGL implementation */
const WSEGL_FunctionTable *WSEGL_GetFunctionTablePointer(void)
{
    return &wseglFunctions;
}

