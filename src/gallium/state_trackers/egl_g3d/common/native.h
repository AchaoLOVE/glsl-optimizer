/*
 * Mesa 3-D graphics library
 * Version:  7.8
 *
 * Copyright (C) 2009-2010 Chia-I Wu <olv@0xlab.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _NATIVE_H_
#define _NATIVE_H_

#include "EGL/egl.h"  /* for EGL native types */
#include "GL/gl.h" /* for GL types needed by __GLcontextModes */
#include "GL/internal/glcore.h"  /* for __GLcontextModes */

#include "pipe/p_compiler.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"

/**
 * Only color buffers are listed.  The others are allocated privately through,
 * for example, st_renderbuffer_alloc_storage().
 */
enum native_attachment {
   NATIVE_ATTACHMENT_FRONT_LEFT,
   NATIVE_ATTACHMENT_BACK_LEFT,
   NATIVE_ATTACHMENT_FRONT_RIGHT,
   NATIVE_ATTACHMENT_BACK_RIGHT,

   NUM_NATIVE_ATTACHMENTS
};

struct native_surface {
   void (*destroy)(struct native_surface *nsurf);

   /**
    * Swap the front and back buffers so that the back buffer is visible.  It
    * is no-op if the surface is single-buffered.  The contents of the back
    * buffer after swapping may or may not be preserved.
    */
   boolean (*swap_buffers)(struct native_surface *nsurf);

   /**
    * Make the front buffer visible.  In some native displays, changes to the
    * front buffer might not be visible immediately and require manual flush.
    */
   boolean (*flush_frontbuffer)(struct native_surface *nsurf);

   /**
    * Validate the buffers of the surface.  Those not listed in the attachments
    * will be destroyed.  The returned textures are owned by the caller.
    */
   boolean (*validate)(struct native_surface *nsurf,
                       const enum native_attachment *natts,
                       unsigned num_natts,
                       struct pipe_texture **textures,
                       int *width, int *height);

   /**
    * Wait until all native commands affecting the surface has been executed.
    */
   void (*wait)(struct native_surface *nsurf);
};

struct native_config {
   __GLcontextModes mode;
   enum pipe_format color_format;
   enum pipe_format depth_format;
   enum pipe_format stencil_format;
};

/**
 * A pipe winsys abstracts the OS.  A pipe screen abstracts the graphcis
 * hardware.  A native display consists of a pipe winsys, a pipe screen, and
 * the native display server.
 */
struct native_display {
   struct pipe_screen *screen;
   void (*destroy)(struct native_display *ndpy);

   /**
    * Get the supported configs.  The configs are owned by the display, but
    * the returned array should be free()ed.
    *
    * The configs will be converted to EGL config by
    * _eglConfigFromContextModesRec and validated by _eglValidateConfig.
    * Those failing to pass the test will be skipped.
    */
   const struct native_config **(*get_configs)(struct native_display *ndpy,
                                               int *num_configs);

   /**
    * Create a pipe context.
    */
   struct pipe_context *(*create_context)(struct native_display *ndpy,
                                          void *context_private);

   /**
    * Create a window surface.  Required unless no config has GLX_WINDOW_BIT
    * set.
    */
   struct native_surface *(*create_window_surface)(struct native_display *ndpy,
                                                   EGLNativeWindowType win,
                                                   const struct native_config *nconf);

   /**
    * Create a pixmap surface.  Required unless no config has GLX_PIXMAP_BIT
    * set.
    */
   struct native_surface *(*create_pixmap_surface)(struct native_display *ndpy,
                                                   EGLNativePixmapType pix,
                                                   const struct native_config *nconf);

   /**
    * Create a pbuffer surface.  Required unless no config has GLX_PBUFFER_BIT
    * set.
    */
   struct native_surface *(*create_pbuffer_surface)(struct native_display *ndpy,
                                                    const struct native_config *nconf,
                                                    uint width, uint height);

#if 0
   struct native_mode *(*get_modes)(struct native_display *ndpy,
                                    int *num_modes);

   /**
    * Create a screen surface.
    */
   struct native_surface *(*create_screen_surface)(struct native_display *ndpy,
                                                   const struct native_config *nconf,
                                                   uint width, uint height);

   boolean (*set_mode)(struct native_display *ndpy,
                       const struct native_mode *nmode,
                       struct native_surface *nsurf);
#endif
};


typedef void (*native_flush_frontbuffer)(void *dummy,
                                         struct pipe_surface *surf,
                                         void *context_private);

const char *
native_get_name(void);

struct native_display *
native_create_display(EGLNativeDisplayType dpy,
                      native_flush_frontbuffer flush_frontbuffer);

#endif /* _NATIVE_H_ */