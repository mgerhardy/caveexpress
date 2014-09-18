/*
 *  Simple DirectMedia Layer
 *  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>
 * 
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 * 
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 */
#include "../SDL_internal.h"

#if SDL_VIDEO_OPENGL_EGL

#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
#include "../core/windows/SDL_windows.h"
#endif

#include "SDL_sysvideo.h"
#include "SDL_egl_c.h"
#include "SDL_loadso.h"
#include "SDL_hints.h"

#if SDL_VIDEO_DRIVER_RPI
/* Raspbian places the OpenGL ES/EGL binaries in a non standard path */
#define DEFAULT_EGL "/opt/vc/lib/libEGL.so"
#define DEFAULT_OGL_ES2 "/opt/vc/lib/libGLESv2.so"
#define DEFAULT_OGL_ES_PVR "/opt/vc/lib/libGLES_CM.so"
#define DEFAULT_OGL_ES "/opt/vc/lib/libGLESv1_CM.so"

#elif SDL_VIDEO_DRIVER_ANDROID
/* Android */
#define DEFAULT_EGL "libEGL.so"
#define DEFAULT_OGL_ES2 "libGLESv2.so"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.so"
#define DEFAULT_OGL_ES "libGLESv1_CM.so"

#elif SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
/* EGL AND OpenGL ES support via ANGLE */
#define DEFAULT_EGL "libEGL.dll"
#define DEFAULT_OGL_ES2 "libGLESv2.dll"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.dll"
#define DEFAULT_OGL_ES "libGLESv1_CM.dll"

#else
/* Desktop Linux */
#define DEFAULT_OGL "libGL.so.1"
#define DEFAULT_EGL "libEGL.so.1"
#define DEFAULT_OGL_ES2 "libGLESv2.so.2"
#define DEFAULT_OGL_ES_PVR "libGLES_CM.so.1"
#define DEFAULT_OGL_ES "libGLESv1_CM.so.1"
#endif /* SDL_VIDEO_DRIVER_RPI */

#define LOAD_FUNC(NAME) \
*((void**)&_this->egl_data->NAME) = SDL_LoadFunction(_this->egl_data->dll_handle, #NAME); \
if (!_this->egl_data->NAME) \
{ \
    return SDL_SetError("Could not retrieve EGL function " #NAME); \
}
    
/* EGL implementation of SDL OpenGL ES support */
#ifdef EGL_KHR_create_context        
static int SDL_EGL_HasExtension(_THIS, const char *ext)
{
    int i;
    int len = 0;
    int ext_len;
    const char *exts;
    const char *ext_word;

    ext_len = SDL_strlen(ext);
    exts = _this->egl_data->eglQueryString(_this->egl_data->egl_display, EGL_EXTENSIONS);

    if(exts) {
        ext_word = exts;

        for(i = 0; exts[i] != 0; i++) {
            if(exts[i] == ' ') {
                if(ext_len == len && !SDL_strncmp(ext_word, ext, len)) {
                    return 1;
                }

                len = 0;
                ext_word = &exts[i + 1];
            }
            else {
                len++;
            }
        }
    }

    return 0;
}
#endif /* EGL_KHR_create_context */

void *
SDL_EGL_GetProcAddress(_THIS, const char *proc)
{
    static char procname[1024];
    void *retval;
    
    /* eglGetProcAddress is busted on Android http://code.google.com/p/android/issues/detail?id=7681 */
#if !defined(SDL_VIDEO_DRIVER_ANDROID) && !defined(SDL_VIDEO_DRIVER_MIR) 
    if (_this->egl_data->eglGetProcAddress) {
        retval = _this->egl_data->eglGetProcAddress(proc);
        if (retval) {
            return retval;
        }
    }
#endif
    
    retval = SDL_LoadFunction(_this->egl_data->egl_dll_handle, proc);
    if (!retval && SDL_strlen(proc) <= 1022) {
        procname[0] = '_';
        SDL_strlcpy(procname + 1, proc, 1022);
        retval = SDL_LoadFunction(_this->egl_data->egl_dll_handle, procname);
    }
    return retval;
}

void
SDL_EGL_UnloadLibrary(_THIS)
{
    if (_this->egl_data) {
        if (_this->egl_data->egl_display) {
            _this->egl_data->eglTerminate(_this->egl_data->egl_display);
            _this->egl_data->egl_display = NULL;
        }

        if (_this->egl_data->dll_handle) {
            SDL_UnloadObject(_this->egl_data->dll_handle);
            _this->egl_data->dll_handle = NULL;
        }
        if (_this->egl_data->egl_dll_handle) {
            SDL_UnloadObject(_this->egl_data->egl_dll_handle);
            _this->egl_data->egl_dll_handle = NULL;
        }
        
        SDL_free(_this->egl_data);
        _this->egl_data = NULL;
    }
}

int
SDL_EGL_LoadLibrary(_THIS, const char *egl_path, NativeDisplayType native_display)
{
    void *dll_handle = NULL, *egl_dll_handle = NULL; /* The naming is counter intuitive, but hey, I just work here -- Gabriel */
    char *path = NULL;
#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
    const char *d3dcompiler;
#endif

    if (_this->egl_data) {
        return SDL_SetError("OpenGL ES context already created");
    }

    _this->egl_data = (struct SDL_EGL_VideoData *) SDL_calloc(1, sizeof(SDL_EGL_VideoData));
    if (!_this->egl_data) {
        return SDL_OutOfMemory();
    }

#if SDL_VIDEO_DRIVER_WINDOWS || SDL_VIDEO_DRIVER_WINRT
    d3dcompiler = SDL_GetHint(SDL_HINT_VIDEO_WIN_D3DCOMPILER);
    if (!d3dcompiler) {
        if (WIN_IsWindowsVistaOrGreater()) {
            d3dcompiler = "d3dcompiler_46.dll";
        } else {
            d3dcompiler = "d3dcompiler_43.dll";
        }
    }
    if (SDL_strcasecmp(d3dcompiler, "none") != 0) {
        SDL_LoadObject(d3dcompiler);
    }
#endif

    /* A funny thing, loading EGL.so first does not work on the Raspberry, so we load libGL* first */
    path = SDL_getenv("SDL_VIDEO_GL_DRIVER");
    if (path != NULL) {
        egl_dll_handle = SDL_LoadObject(path);
    }

    if (egl_dll_handle == NULL) {
        if(_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
            if (_this->gl_config.major_version > 1) {
                path = DEFAULT_OGL_ES2;
                egl_dll_handle = SDL_LoadObject(path);
            }
            else {
                path = DEFAULT_OGL_ES;
                egl_dll_handle = SDL_LoadObject(path);
                if (egl_dll_handle == NULL) {
                    path = DEFAULT_OGL_ES_PVR;
                    egl_dll_handle = SDL_LoadObject(path);
                }
            }
        }
#ifdef DEFAULT_OGL         
        else {
            path = DEFAULT_OGL;
            egl_dll_handle = SDL_LoadObject(path);
        }
#endif        
    }
    _this->egl_data->egl_dll_handle = egl_dll_handle;

    if (egl_dll_handle == NULL) {
        return SDL_SetError("Could not initialize OpenGL / GLES library");
    }

    /* Loading libGL* in the previous step took care of loading libEGL.so, but we future proof by double checking */
    if (egl_path != NULL) {
        dll_handle = SDL_LoadObject(egl_path);
    }   
    /* Try loading a EGL symbol, if it does not work try the default library paths */
    if (dll_handle == NULL || SDL_LoadFunction(dll_handle, "eglChooseConfig") == NULL) {
        if (dll_handle != NULL) {
            SDL_UnloadObject(dll_handle);
        }
        path = SDL_getenv("SDL_VIDEO_EGL_DRIVER");
        if (path == NULL) {
            path = DEFAULT_EGL;
        }
        dll_handle = SDL_LoadObject(path);
        if (dll_handle == NULL || SDL_LoadFunction(dll_handle, "eglChooseConfig") == NULL) {
            if (dll_handle != NULL) {
                SDL_UnloadObject(dll_handle);
            }
            return SDL_SetError("Could not load EGL library");
        }
        SDL_ClearError();
    }

    _this->egl_data->dll_handle = dll_handle;

    /* Load new function pointers */
    LOAD_FUNC(eglGetDisplay);
    LOAD_FUNC(eglInitialize);
    LOAD_FUNC(eglTerminate);
    LOAD_FUNC(eglGetProcAddress);
    LOAD_FUNC(eglChooseConfig);
    LOAD_FUNC(eglGetConfigAttrib);
    LOAD_FUNC(eglCreateContext);
    LOAD_FUNC(eglDestroyContext);
    LOAD_FUNC(eglCreateWindowSurface);
    LOAD_FUNC(eglDestroySurface);
    LOAD_FUNC(eglMakeCurrent);
    LOAD_FUNC(eglSwapBuffers);
    LOAD_FUNC(eglSwapInterval);
    LOAD_FUNC(eglWaitNative);
    LOAD_FUNC(eglWaitGL);
    LOAD_FUNC(eglBindAPI);
    LOAD_FUNC(eglQueryString);
    
#if !defined(__WINRT__)
    _this->egl_data->egl_display = _this->egl_data->eglGetDisplay(native_display);
    if (!_this->egl_data->egl_display) {
        return SDL_SetError("Could not get EGL display");
    }
    
    if (_this->egl_data->eglInitialize(_this->egl_data->egl_display, NULL, NULL) != EGL_TRUE) {
        return SDL_SetError("Could not initialize EGL");
    }
#endif

    _this->gl_config.driver_loaded = 1;

    if (path) {
        SDL_strlcpy(_this->gl_config.driver_path, path, sizeof(_this->gl_config.driver_path) - 1);
    } else {
        *_this->gl_config.driver_path = '\0';
    }
    
    return 0;
}

/*
 Activate this define to dump the contents of EGL Configs
 (you can also turn on/off dump_all, in "dump_attribs_response()"
*/
#define DEBUG_EGL_CONFIG




#ifdef DEBUG_EGL_CONFIG
#include "SDL_log.h"

static int tab_key[32] = 
{
    EGL_ALPHA_SIZE,
    EGL_ALPHA_MASK_SIZE,
    EGL_BIND_TO_TEXTURE_RGB,
    EGL_BIND_TO_TEXTURE_RGBA,
    EGL_BLUE_SIZE,
    EGL_BUFFER_SIZE,
    EGL_COLOR_BUFFER_TYPE,
    EGL_CONFIG_CAVEAT,
    EGL_CONFIG_ID,
    EGL_CONFORMANT,
    EGL_DEPTH_SIZE,
    EGL_GREEN_SIZE,
    EGL_LEVEL,
    EGL_LUMINANCE_SIZE,
    EGL_MAX_PBUFFER_WIDTH,
    EGL_MAX_PBUFFER_HEIGHT,
    EGL_MAX_PBUFFER_PIXELS,
    EGL_MAX_SWAP_INTERVAL,
    EGL_MIN_SWAP_INTERVAL,
    EGL_NATIVE_RENDERABLE,
    EGL_NATIVE_VISUAL_ID,
    EGL_NATIVE_VISUAL_TYPE,
    EGL_RED_SIZE,
    EGL_RENDERABLE_TYPE,
    EGL_SAMPLE_BUFFERS,
    EGL_SAMPLES,
    EGL_STENCIL_SIZE,
    EGL_SURFACE_TYPE,
    EGL_TRANSPARENT_TYPE,
    EGL_TRANSPARENT_RED_VALUE,
    EGL_TRANSPARENT_GREEN_VALUE,
    EGL_TRANSPARENT_BLUE_VALUE
};

static const char *tab_str[32] = 
{
    "EGL_ALPHA_SIZE",
    "EGL_ALPHA_MASK_SIZE",
    "EGL_BIND_TO_TEXTURE_RGB",
    "EGL_BIND_TO_TEXTURE_RGBA",
    "EGL_BLUE_SIZE",
    "EGL_BUFFER_SIZE",
    "EGL_COLOR_BUFFER_TYPE",
    "EGL_CONFIG_CAVEAT",
    "EGL_CONFIG_ID",
    "EGL_CONFORMANT",
    "EGL_DEPTH_SIZE",
    "EGL_GREEN_SIZE",
    "EGL_LEVEL",
    "EGL_LUMINANCE_SIZE",
    "EGL_MAX_PBUFFER_WIDTH",
    "EGL_MAX_PBUFFER_HEIGHT",
    "EGL_MAX_PBUFFER_PIXELS",
    "EGL_MAX_SWAP_INTERVAL",
    "EGL_MIN_SWAP_INTERVAL",
    "EGL_NATIVE_RENDERABLE",
    "EGL_NATIVE_VISUAL_ID",
    "EGL_NATIVE_VISUAL_TYPE",
    "EGL_RED_SIZE",
    "EGL_RENDERABLE_TYPE",
    "EGL_SAMPLE_BUFFERS",
    "EGL_SAMPLES",
    "EGL_STENCIL_SIZE",
    "EGL_SURFACE_TYPE",
    "EGL_TRANSPARENT_TYPE",
    "EGL_TRANSPARENT_RED_VALUE",
    "EGL_TRANSPARENT_GREEN_VALUE",
    "EGL_TRANSPARENT_BLUE_VALUE"
};


/* Convert EGL_* key to its string */
static const char *
key2str(int key)
{
    const int    nb      = sizeof (tab_key) / sizeof (tab_key[0]);
    const char  *str     = NULL;
    static char  buf[64];
    int          i;

    for (i = 0; i < nb; i++)
    {
        if (key == tab_key[i])
        {
            str = tab_str[i];
            break;
        }
    }

    if (str == NULL)
    {
        str = (const char *)buf;
        sprintf(buf, "unknown (%d)", key);
    }

    return str;
}


/* Convert EGL_* value to its string */
static const char *
value2str(int key, int value)
{
    const char  *str             = NULL;
    static char  buf[64];
    char         buf_bitmask[64];

    if (
               key == EGL_BIND_TO_TEXTURE_RGB 
            || key == EGL_BIND_TO_TEXTURE_RGBA
            || key == EGL_NATIVE_RENDERABLE
       )
    {
        if (value == EGL_TRUE)
            str = "EGL_TRUE";
        else if (value == EGL_FALSE)
            str = "EGL_FALSE";
    }
    else if (key == EGL_COLOR_BUFFER_TYPE)
    {
        if (value == EGL_RGB_BUFFER)
            str = "EGL_RGB_BUFFER";
        else if (value == EGL_LUMINANCE_BUFFER)
            str = "EGL_LUMINANCE_BUFFER";
    }
    else if (key == EGL_CONFIG_CAVEAT)
    {
        /* If the EGL version is 1.3 or later, caveat EGL_NON_CONFORMANT_CONFIG is obsolete, 
           since the same information can be specified via the EGL_CONFORMANT attribute
           on a per-client-API basis, not just for OpenGL ES.  */

        if (value == EGL_NONE)
            str = "EGL_NONE";
        else if (value == EGL_SLOW_CONFIG)
            str = "EGL_SLOW_CONFIG";
        else if (value == EGL_NON_CONFORMANT_CONFIG)
            str = "EGL_NON_CONFORMANT_CONFIG";
    }
    else if (
               key == EGL_CONFORMANT
            || key == EGL_RENDERABLE_TYPE
            || key == EGL_SURFACE_TYPE
            )
    {
        str = (const char *) buf_bitmask;
        sprintf(buf_bitmask, "0x%08x", value);
    }
    else if (key == EGL_TRANSPARENT_TYPE)
    {
        if (value == EGL_NONE)
            str = "EGL_NONE";      
        else if (value == EGL_TRANSPARENT_RGB)
            str = "EGL_TRANSPARENT_RGB";
    }

    if (str == NULL)
    {
        sprintf(buf, "%d", value);
    }
    else
    {
        sprintf(buf, "%s (%d == 0x%08x)", str, value, value);
    }

    return buf;
}


/* Dump the attribs request */
static void
dump_attribs_request(EGLint *attribs)
{
    int i;
    SDL_Log("{");
    for (i = 0; i < 64; i++)
    {
        if (attribs[i] == EGL_NONE)
        {
            break;
        }

        {
            const int   key   = attribs[i++];
            const int   value = attribs[i];
            SDL_Log("  %s => %s", key2str(key), value2str(key, value));
        }
    }
    SDL_Log("}");
    return;
}

/* Dump the config returned as a response from the request */
static void
dump_attribs_response(EGLint *attribs, EGLConfig config, _THIS)
{
    const int nb = sizeof (tab_key) / sizeof (tab_key[0]);
    int i;
    SDL_Log("{");

    /* Set to 1 or 0 to dump all params of config, or only the one requested */
    const int dump_all = 1;

    for (i = 0; i < nb; i++)
    {
        const int key = tab_key[i];
        EGLint value = 2014;
        int j;
        int found = 0;

        /* Get value of the config */
        _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, config, key, &value);

        /* Only dump when it matches the attribs request */
        for (j = 0; j < 64; j += 2)
        {
            /* End of attribs table */
            if (attribs[j] == EGL_NONE)
            {
                break;
            }

            /* Match the attribs request */
            if (attribs[j] == key)
            {
                SDL_Log("  %s => %s  ******* (requested=%d, diff=%d)",
                        key2str(key), value2str(key, value),
                        attribs[j+1], value-attribs[j+1]);
                found = 1;
                break;
            }
        }

        /* Dump all */
        if (dump_all && found == 0)
        {
            SDL_Log("  %s => %s", key2str(key), value2str(key, value));
        }
    }

    SDL_Log("}");
    return;
}
#endif

int
SDL_EGL_ChooseConfig(_THIS) 
{
    /* 64 seems nice. */
    EGLint attribs[64];
    EGLint found_configs = 0, value;
    /* 128 seems even nicer here */
    EGLConfig configs[128];
    int i, j, best_bitdiff = -1, bitdiff;
    
    if (!_this->egl_data) {
        /* The EGL library wasn't loaded, SDL_GetError() should have info */
        return -1;
    }
  
    /* Get a valid EGL configuration */
    i = 0;
    attribs[i++] = EGL_RED_SIZE;
    attribs[i++] = _this->gl_config.red_size;
    attribs[i++] = EGL_GREEN_SIZE;
    attribs[i++] = _this->gl_config.green_size;
    attribs[i++] = EGL_BLUE_SIZE;
    attribs[i++] = _this->gl_config.blue_size;
    
    if (_this->gl_config.alpha_size) {
        attribs[i++] = EGL_ALPHA_SIZE;
        attribs[i++] = _this->gl_config.alpha_size;
    }
    
    if (_this->gl_config.buffer_size) {
        attribs[i++] = EGL_BUFFER_SIZE;
        attribs[i++] = _this->gl_config.buffer_size;
    }
    
    attribs[i++] = EGL_DEPTH_SIZE;
    attribs[i++] = _this->gl_config.depth_size;
    
    if (_this->gl_config.stencil_size) {
        attribs[i++] = EGL_STENCIL_SIZE;
        attribs[i++] = _this->gl_config.stencil_size;
    }
    
    if (_this->gl_config.multisamplebuffers) {
        attribs[i++] = EGL_SAMPLE_BUFFERS;
        attribs[i++] = _this->gl_config.multisamplebuffers;
    }
    
    if (_this->gl_config.multisamplesamples) {
        attribs[i++] = EGL_SAMPLES;
        attribs[i++] = _this->gl_config.multisamplesamples;
    }
    
    attribs[i++] = EGL_RENDERABLE_TYPE;
    if(_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
        if (_this->gl_config.major_version == 2) {
            attribs[i++] = EGL_OPENGL_ES2_BIT;
        } else {
            attribs[i++] = EGL_OPENGL_ES_BIT;
        }
        _this->egl_data->eglBindAPI(EGL_OPENGL_ES_API);
    }
    else {
        attribs[i++] = EGL_OPENGL_BIT;
        _this->egl_data->eglBindAPI(EGL_OPENGL_API);
    }
    
    attribs[i++] = EGL_NONE;

#ifdef DEBUG_EGL_CONFIG
    /* Dump the request */
    SDL_Log("Request configs (i == %d)", i);
    dump_attribs_request(attribs);
#endif
   
    if (_this->egl_data->eglChooseConfig(_this->egl_data->egl_display,
        attribs,
        configs, SDL_arraysize(configs),
        &found_configs) == EGL_FALSE ||
        found_configs == 0) {
        return SDL_SetError("Couldn't find matching EGL config");
    }
    
#ifdef DEBUG_EGL_CONFIG
    SDL_Log("==> Found configs: %d", found_configs);
#endif

    /* eglChooseConfig returns a number of configurations that match or exceed the requested attribs. */
    /* From those, we select the one that matches our requirements more closely via a makeshift algorithm */

    for ( i=0; i<found_configs; i++ ) {

#ifdef DEBUG_EGL_CONFIG
        SDL_Log("Dump config %d", i);
        dump_attribs_response(attribs, configs[i], _this);
#endif

        bitdiff = 0;
        for (j = 0; j < SDL_arraysize(attribs) - 1; j += 2) {
            if (attribs[j] == EGL_NONE) {
               break;
            }
            
            if ( attribs[j+1] != EGL_DONT_CARE && (
                attribs[j] == EGL_RED_SIZE ||
                attribs[j] == EGL_GREEN_SIZE ||
                attribs[j] == EGL_BLUE_SIZE ||
                attribs[j] == EGL_ALPHA_SIZE ||
                attribs[j] == EGL_DEPTH_SIZE ||
                attribs[j] == EGL_STENCIL_SIZE)) {
                _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display, configs[i], attribs[j], &value);
                bitdiff += value - attribs[j + 1]; /* value is always >= attrib */
            }
        }
        
#ifdef DEBUG_EGL_CONFIG
        SDL_Log("bitdiff = %d", bitdiff);
#endif

        if (bitdiff < best_bitdiff || best_bitdiff == -1) {
            _this->egl_data->egl_config = configs[i];
            
            best_bitdiff = bitdiff;
#ifdef DEBUG_EGL_CONFIG
            SDL_Log("New best bitdiff = %d !", bitdiff);
#endif
        }
           
        if (bitdiff == 0) {
#ifdef DEBUG_EGL_CONFIG
            SDL_Log("Found an exact match (bitdiff == %d)", bitdiff);
#endif
            break; /* we found an exact match! */
        }
    }
    
    return 0;
}

SDL_GLContext
SDL_EGL_CreateContext(_THIS, EGLSurface egl_surface)
{
    EGLint context_attrib_list[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        1,
        EGL_NONE,
        EGL_NONE,
        EGL_NONE,
        EGL_NONE,
        EGL_NONE
    };
    
    EGLContext egl_context, share_context = EGL_NO_CONTEXT;
    
    if (!_this->egl_data) {
        /* The EGL library wasn't loaded, SDL_GetError() should have info */
        return NULL;
    }
    
    if (_this->gl_config.share_with_current_context) {
        share_context = (EGLContext)SDL_GL_GetCurrentContext();
    }
    
    /* Bind the API */
    if(_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
        _this->egl_data->eglBindAPI(EGL_OPENGL_ES_API);
        if (_this->gl_config.major_version) {
            context_attrib_list[1] = _this->gl_config.major_version;
        }

        egl_context = _this->egl_data->eglCreateContext(_this->egl_data->egl_display,
                                          _this->egl_data->egl_config,
                                          share_context, context_attrib_list);
    }
    else {
        _this->egl_data->eglBindAPI(EGL_OPENGL_API);
#ifdef EGL_KHR_create_context        
        if(SDL_EGL_HasExtension(_this, "EGL_KHR_create_context")) {
            context_attrib_list[0] = EGL_CONTEXT_MAJOR_VERSION_KHR;
            context_attrib_list[1] = _this->gl_config.major_version;
            context_attrib_list[2] = EGL_CONTEXT_MINOR_VERSION_KHR;
            context_attrib_list[3] = _this->gl_config.minor_version;
            context_attrib_list[4] = EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR;
            switch(_this->gl_config.profile_mask) {
            case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
                context_attrib_list[5] = EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;
                break;

            case SDL_GL_CONTEXT_PROFILE_CORE:
            default:
                context_attrib_list[5] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
                break;
            }
        }
        else {
            context_attrib_list[0] = EGL_NONE;
        }
#else /* EGL_KHR_create_context */
        context_attrib_list[0] = EGL_NONE;
#endif /* EGL_KHR_create_context */
        egl_context = _this->egl_data->eglCreateContext(_this->egl_data->egl_display,
                                          _this->egl_data->egl_config,
                                          share_context, context_attrib_list);
    }
    
    if (egl_context == EGL_NO_CONTEXT) {
        SDL_SetError("Could not create EGL context");
        return NULL;
    }
    
    _this->egl_data->egl_swapinterval = 0;
    
    if (SDL_EGL_MakeCurrent(_this, egl_surface, egl_context) < 0) {
        SDL_EGL_DeleteContext(_this, egl_context);
        SDL_SetError("Could not make EGL context current");
        return NULL;
    }
  
    return (SDL_GLContext) egl_context;
}

int
SDL_EGL_MakeCurrent(_THIS, EGLSurface egl_surface, SDL_GLContext context)
{
    EGLContext egl_context = (EGLContext) context;

    if (!_this->egl_data) {
        return SDL_SetError("OpenGL not initialized");
    }
    
    /* The android emulator crashes badly if you try to eglMakeCurrent 
     * with a valid context and invalid surface, so we have to check for both here.
     */
    if (!egl_context || !egl_surface) {
         _this->egl_data->eglMakeCurrent(_this->egl_data->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    else {
        if (!_this->egl_data->eglMakeCurrent(_this->egl_data->egl_display,
            egl_surface, egl_surface, egl_context)) {
            return SDL_SetError("Unable to make EGL context current");
        }
    }
      
    return 0;
}

int
SDL_EGL_SetSwapInterval(_THIS, int interval)
{
    EGLBoolean status;
    
    if (!_this->egl_data) {
        return SDL_SetError("EGL not initialized");
    }
    
    status = _this->egl_data->eglSwapInterval(_this->egl_data->egl_display, interval);
    if (status == EGL_TRUE) {
        _this->egl_data->egl_swapinterval = interval;
        return 0;
    }
    
    return SDL_SetError("Unable to set the EGL swap interval");
}

int
SDL_EGL_GetSwapInterval(_THIS)
{
    if (!_this->egl_data) {
        return SDL_SetError("EGL not initialized");
    }
    
    return _this->egl_data->egl_swapinterval;
}

void
SDL_EGL_SwapBuffers(_THIS, EGLSurface egl_surface)
{
    _this->egl_data->eglSwapBuffers(_this->egl_data->egl_display, egl_surface);
}

void
SDL_EGL_DeleteContext(_THIS, SDL_GLContext context)
{
    EGLContext egl_context = (EGLContext) context;

    /* Clean up GLES and EGL */
    if (!_this->egl_data) {
        return;
    }
    
    if (egl_context != NULL && egl_context != EGL_NO_CONTEXT) {
        SDL_EGL_MakeCurrent(_this, NULL, NULL);
        _this->egl_data->eglDestroyContext(_this->egl_data->egl_display, egl_context);
    }
        
}

EGLSurface *
SDL_EGL_CreateSurface(_THIS, NativeWindowType nw) 
{
    if (SDL_EGL_ChooseConfig(_this) != 0) {
        return EGL_NO_SURFACE;
    }
    
#if __ANDROID__
    {
        /* Android docs recommend doing this!
         * Ref: http://developer.android.com/reference/android/app/NativeActivity.html 
         */
        EGLint format;
        _this->egl_data->eglGetConfigAttrib(_this->egl_data->egl_display,
                                            _this->egl_data->egl_config, 
                                            EGL_NATIVE_VISUAL_ID, &format);

        ANativeWindow_setBuffersGeometry(nw, 0, 0, format);
    }
#endif    
    
    return _this->egl_data->eglCreateWindowSurface(
            _this->egl_data->egl_display,
            _this->egl_data->egl_config,
            nw, NULL);
}

void
SDL_EGL_DestroySurface(_THIS, EGLSurface egl_surface) 
{
    if (!_this->egl_data) {
        return;
    }
    
    if (egl_surface != EGL_NO_SURFACE) {
        _this->egl_data->eglDestroySurface(_this->egl_data->egl_display, egl_surface);
    }
}

#endif /* SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
    
