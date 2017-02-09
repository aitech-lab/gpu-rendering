
#include <stdio.h>
#include <unistd.h>

#include <GL/gl.h>
#include <EGL/egl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "egl.h"

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE , 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE  , 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
};

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH , BUFFER_WIDTH ,
    EGL_HEIGHT, BUFFER_HEIGHT,
    EGL_NONE,
};

static const EGLint contextAttribs[] = {
  EGL_CONTEXT_CLIENT_VERSION, 2,
  EGL_NONE,
};

static EGLDisplay display;
static EGLint major, minor;
static EGLint numConfigs;
static EGLConfig config;
static EGLSurface surface;
static EGLContext context;

static void check_err();

#define ATTR(attr)                                      \
   eglGetConfigAttrib(display, configs[i], attr, &val); \
   fprintf(stderr, "%*s: %d\n", 24, #attr, val);

// https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglChooseConfig.xhtml
<<<<<<< HEAD
// https://www.khronos.org/files/egl-1-4-quick-reference-card.pdf
=======
>>>>>>> 37282358b2d8090afc78c42a0d118692f34eae8c
static void egl_print_available_configs() {
    EGLint ccnt, n;
    eglGetConfigs(display, NULL, 0, &ccnt);
    fprintf(stdout, "EGL has %d configs total\n", ccnt);
    EGLConfig* configs = calloc(ccnt, sizeof *configs);
    eglChooseConfig(display, configAttribs, configs, ccnt, &n);
    for (int i = 0; i < n; i++) {
        fprintf(stderr, "Config id: %d\n", configs[i]);
        EGLint val;
        ATTR(EGL_BUFFER_SIZE);
        ATTR(EGL_RED_SIZE);
        ATTR(EGL_GREEN_SIZE);
        ATTR(EGL_BLUE_SIZE);
        ATTR(EGL_ALPHA_SIZE);
        ATTR(EGL_RENDERABLE_TYPE);
        ATTR(EGL_SURFACE_TYPE);
        // just choose the first one
        // egl_conf = configs[i];
        // break;
    }
}

#define DEBUG

#ifdef DEBUG
#define ERR(res, cmd, ...) {                          \
    res = cmd(__VA_ARGS__);                           \
    EGLint err = eglGetError();                       \
    fprintf(stderr, #cmd"\t");                        \
    if(err!=0x3000) fprintf(stderr, "ERR %h\n", err); \
    else printf("OK\n");                              \
}
#else
#define ERR(res, cmd, ...) res = cmd(__VA_ARGS__);
#endif

/*
  https://jan.newmarch.name/Wayland/EGL/

  EGL has a display that it writes on. The display is built on a native display,
  and is obtained by the call eglGetDisplay. The EGL platform is then
  initialised using eglInitialize.

  Typically an EGL display will support a number of configurations. For example,
  a pixel may be 16 bits (5 red, 5 blue and 6 green), 24 bits (8 red, 8 green
  and 8 blue) or 32 bits (8 extra bits for alpha transparency). An application
  will specify certain parameters such as the minimum size of a red pixel, and
  can then access the array of matching configurations using eglChooseConfig.
  The attributes of a configuration can be queried using eglGetConfigAttrib. One
  configuration should be chosen before proceeding.

  Each configuration will support one or more client APIs such as OpenGL. The
  API is usually requested through the configuration attribute
  EGL_RENDERABLE_TYPE which should have a value such as EGL_OPENGL_ES2_BIT.

  In addition to a configuration, each application needs one or more contexts.
  Each context defines a level of the API that will be used for rendering.
  Examples typically use a level of 2, and a context is created using
  eglCreateContext.
*/

void egl_init() {

    EGLBoolean res;

    // 1. Initialize EGL
    ERR(display, eglGetDisplay, EGL_DEFAULT_DISPLAY);
    ERR(res, eglInitialize, display, &major, &minor);
    printf("version %d.%d\n", major, minor);
    egl_print_available_configs();

    // 2. Select an appropriate configuration
    ERR(res, eglChooseConfig, display, configAttribs, &config, 1, &numConfigs);

    // 3. Create a surface
    ERR(surface, eglCreatePbufferSurface, display, config, pbufferAttribs);

    // 4. Bind the API
    ERR(res, eglBindAPI, EGL_OPENGL_API);

    // 5. Create a context and make it current
    ERR(context, eglCreateContext, display, config, EGL_NO_CONTEXT, contextAttribs);
    ERR(res, eglMakeCurrent, display, surface, surface, context);
}

void egl_swap() {
    eglSwapBuffers(display, surface);
}

unsigned char pixels[BUFFER_WIDTH*BUFFER_HEIGHT*4];
void egl_save(char const* filename) {
    glReadPixels(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT,
                 GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
    stbi_write_png(filename, BUFFER_WIDTH, BUFFER_HEIGHT, 4, pixels, BUFFER_WIDTH*4);
}

static FILE* out;
void egl_init_stream() {
   out = fdopen(dup(fileno(stdout)), "wb");
}
void egl_write_stream() {
    static int pos = 0;
    glReadPixels(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT,
                 GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);

    pixels[pos%BUFFER_WIDTH*4+0] = 255;
    pixels[pos%BUFFER_WIDTH*4+1] = 255;
    pixels[pos%BUFFER_WIDTH*4+2] = 255;
    pixels[pos%BUFFER_WIDTH*4+3] = 255;
    pos++;
    fwrite(pixels, BUFFER_WIDTH*BUFFER_HEIGHT, 4, out);
}
void egl_close_stream() {
    fflush(out);
    fclose(out);
}

void egl_close() {
    // 6. Terminate EGL when finished
    eglTerminate(display);
}

static void check_err() {
   EGLint err = eglGetError();
   if(err!=0x3000) {
       fprintf(stderr, "Error %h\n", err);
   } else {
       printf("OK\n");
   }
}
