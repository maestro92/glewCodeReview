So these are some notes to understand what the fxxk glew.h is doing.
having watched handmade hero and seeing How Casey handles OpenGL extension functions, I am very curious as to how does glew.h does,
and how is it different from what Casey does

a bit of background, quoting glew_s github page 

        "The OpenGL Extension Wrangler Library (GLEW) is a cross-platform open-source C/C++ extension loading library. 
        GLEW provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform. 
        OpenGL core and extension functionality is exposed in a single header file. 
        GLEW has been tested on a variety of operating systems, including Windows, Linux, Mac OS X, FreeBSD, Irix, and Solaris."


simply put, you use glew to load extension functions. 




1.  so first thing is that we can look at glew website and see how is to use glew 

http://glew.sourceforge.net/basic.html



2.  what is the relationship between glcorearb.h and glew?

Khronos is the official group for OpenGL matters. They publish the headers.
People at GLFW, GLEW, VS, or whatever, download the headers and incorporate in their code. 
Sometimes they do some small changes, mainly for 32/64 bit types or compiler adjustments.

If you compare glext.h and glcorearb.h you will see that the later doesnt include any stuff 
for OGL before 3.2, while the glext.h contains all. So, general libs (like GLEW) will provide and use glext.h instead of glcorearb.h.

If your code requires glcorearb.h then just fetch it from Khronos and put it in the same folder where glext.h is.


https://stackoverflow.com/questions/54913829/where-is-glcorearb-h-supposed-to-come-from-and-where-is-it-supposed-to-live-in



3.  my overal impression is that this is very similar to what Casey does in handmade hero. 




########################################################
################ Directory Structure ###################
########################################################

1.  most of the time when you have to use glew, the way to use it is 


    ------->    #include <GL/glew.h>
                #include <GL/glut.h>
                ...
                glutInit(&argc, argv);
                glutCreateWindow("GLEW Test");
    ------->    GLenum err = glewInit();
                if (GLEW_OK != err)
                {
                  /* Problem: glewInit failed, something is seriously wrong. */
                  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
                  ...
                }
                fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
                so lets take a look at what does glew.h does 


as you can see, the two most important thing is glew.h and glewInit();



2.  lets first look at glew.h. glew.h resides in the folder 

                include/GL/glew.h 

you can see that along with glew.h, you also have 

                include/
                    eglew.h 
                    glew.h 
                    glxew.h 
                    wglew.h


what is interesting is that if you go to glfw_s website, the usage is similar 
https://www.glfw.org/docs/3.3/build_guide.html

GLFW does 

                #include <GLFW/glfw3.h>

where as glew does 

                #include <GL/glew.h>



3.  platform specific extensions 
http://glew.sourceforge.net/basic.html
so on the basic.html page, there is a section called "Platform Specific Extensions"
it says that 
                
                "Platform specific extensions are separated into two header files: wglew.h and glxew.h, 
                which define the available WGL and GLX extensions."

so glew.h contains all the common extensions,

wglew.h and glxew.h contains all the platform specific 

        wglew.h is for windows OS
        glxew.h is for "X Window System", which is for linux/unix systems OS 


4.  so assuming that you are on windows, you will most likely be dealing with functions with "wgl" prefix
wgl is windows openGL




##########################################################
############## MACRO Definitions #########################
##########################################################


5.  so if you look at glew.h, first thing you see is 

                #if defined(__gl_h_) || defined(__GL_H__) || defined(_GL_H) || defined(__X_GL_H)
                #error gl.h included before glew.h
                #endif
                #if defined(__gl2_h_)
                #error gl2.h included before glew.h
                #endif
                #if defined(__gltypes_h_)
                #error gltypes.h included before glew.h
                #endif
                #if defined(__REGAL_H__)
                #error Regal.h included before glew.h
                #endif
                #if defined(__glext_h_) || defined(__GLEXT_H_)
                #error glext.h included before glew.h
                #endif
                #if defined(__gl_ATI_h_)
                #error glATI.h included before glew.h
                #endif

you probably often see the compiler error

        "error C1189: #error :  gl.h included before glew.h"

this is where it comes from. 




6.  next we see the APIENTRY definition


                #ifdef APIENTRY
                #  ifndef GLAPIENTRY
                #    define GLAPIENTRY APIENTRY
                #  endif
                #  ifndef GLEWAPIENTRY
                #    define GLEWAPIENTRY APIENTRY
                #  endif
                #else
                #define GLEW_APIENTRY_DEFINED
                #  if defined(__MINGW32__) || defined(__CYGWIN__) || (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
                #    define APIENTRY __stdcall
                #    ifndef GLAPIENTRY
                #      define GLAPIENTRY __stdcall
                #    endif
                #    ifndef GLEWAPIENTRY
                #      define GLEWAPIENTRY __stdcall
                #    endif
                #  else
                #    define APIENTRY
                #  endif
                #endif


so you can see that we are trying to define a constant of APIENTRY

so APIENTRY is an alias for WINAPI. WINAPI itself is a definition for the type of calling convention 
used for windows API calls. 
https://stackoverflow.com/questions/2081409/what-does-apientry-do


you can also see that if you are on windows, but you are using MINGW32 compiler, 
we define APIENTRY as equivalent of __stdcall. 

the specs for __stdcall is below:
https://docs.microsoft.com/en-us/cpp/cpp/stdcall?view=vs-2019

        The __stdcall calling convention is used to call Win32 API functions. The callee cleans the stack, 
        so the compiler makes vararg functions __cdecl. Functions that use this calling 
        convention require a function prototype. The __stdcall modifier is Microsoft-specific.


essentially you are telling the compiler the calling convention for this routine. 
Basically this is explaining to the compiler how to handle the stack and arguments when calling this function. 





7.  then you can see the definition for GLEW_STATI and GLEW_BUILD 


                /*
                 * GLEW_STATIC is defined for static library.
                 * GLEW_BUILD  is defined for building the DLL library.
                 */

                #ifdef GLEW_STATIC
                #  define GLEWAPI extern
                #else
                #  ifdef GLEW_BUILD
                #    define GLEWAPI extern __declspec(dllexport)
                #  else
                #    define GLEWAPI extern __declspec(dllimport)
                #  endif
                #endif




8.  so on line 253 ~ 267 we see 


                /* <glu.h> */
                #ifndef GLAPI
                #define GLAPI extern
                #endif

                #endif /* _WIN32 */

                #ifndef GLAPIENTRY
                #define GLAPIENTRY
                #endif

                #ifndef GLEWAPIENTRY
                #define GLEWAPIENTRY
                #endif

                #define GLEW_VAR_EXPORT GLEWAPI
                #define GLEW_FUN_EXPORT GLEWAPI

Regarding lots of the empty macro definitions 
this is the purpose of it:
https://stackoverflow.com/questions/13892191/are-empty-macro-definitions-allowed-in-c-how-do-they-behave



9.  at line 269, we see 

                                
                #ifdef __cplusplus
                extern "C" {
                #endif

these three lines are actually quite common.

in handmade_platform.h, we actually see this as well 

                handmade_platform.h 

                #ifdef __cplusplus
                extern "C" {
                #endif

#ifdef __cplusplus, lets you knkow if a c++ compiler is compiling your code.
if a c++ compiler is indeed compiling your code, it will enter inside the __cplusplus logic path 

the idea is that by wrapping the C code with extern "C" the C++ compiler will not mangle the C codes names

https://www.codeproject.com/Questions/51411/What-s-the-theory-behind-this-ifdef-cplusplus-ext





################################################################
###################### Opengl Versions #########################
################################################################

10. so if you look at the history of OpenGL 
https://www.khronos.org/opengl/wiki/History_of_OpenGL#OpenGL_1.0_.281992.29

you can see that OpenGL 1.0 is released in 1992 and i ahd a bunch of basic functions 


then for every new version of OpenGL that comes out (whenever the spec for the new OpenGL version comes out);
the new added functionality which are called "extensions" are introduced in these new versions. 
 
glew organizes all the extensions by versions.

you can see that we have:

                /* ----------------------------- GL_VERSION_1_1 ---------------------------- */

                #ifndef GL_VERSION_1_1
                #define GL_VERSION_1_1 1

                ...........................................
                ... all the extensions in OpenGL 1.1 ......
                ...........................................

                /* ----------------------------- GL_VERSION_1_2 ---------------------------- */

                #ifndef GL_VERSION_1_2
                #define GL_VERSION_1_2 1

                ...........................................
                ... all the extensions in OpenGL 1.2 ......
                ...........................................

                ...
                ...


lets take an example: 

In OpenGL 1.2, we have the following newly released extensions:

                typedef void (GLAPIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
                typedef void (GLAPIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
                typedef void (GLAPIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
                typedef void (GLAPIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

                #define glCopyTexSubImage3D GLEW_GET_FUN(__glewCopyTexSubImage3D)
                #define glDrawRangeElements GLEW_GET_FUN(__glewDrawRangeElements)
                #define glTexImage3D GLEW_GET_FUN(__glewTexImage3D)
                #define glTexSubImage3D GLEW_GET_FUN(__glewTexSubImage3D)


you can see that these four functions all have the word 
https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glDrawRangeElements.xml
                glDrawRangeElements is available only if the GL version is 1.2 or greater.


https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glCopyTexSubImage3D.xml                
                glCopyTexSubImage3D is available only if the GL version is 1.2 or greater.


                ...
                ...



regarding what typedef void (GLAPIENTRY* PFNGLCOPYTEXSUBIMAGE3DPROC) this is 

https://docs.microsoft.com/en-us/cpp/cpp/stdcall?view=vs-2019

this defines PFNGLCOPYTEXSUBIMAGE3DPROC as a pointer to a stdcall, that has the signature 
void function(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);



another thing is 

                #define glCopyTexSubImage3D GLEW_GET_FUN(__glewCopyTexSubImage3D)



the definition for GLEW_GET_FUN() is 

                #ifndef GLEW_GET_FUN
                #define GLEW_GET_FUN(x) x
                #endif


so the definition above 
                
                #define glCopyTexSubImage3D GLEW_GET_FUN(__glewCopyTexSubImage3D)

becomes 

                __glewCopyTexSubImage3D




11. definitions for __glewCopyTexSubImage3D are all below in glew.h


                GLEW_FUN_EXPORT PFNGLCOPYTEXSUBIMAGE3DPROC __glewCopyTexSubImage3D;
                GLEW_FUN_EXPORT PFNGLDRAWRANGEELEMENTSPROC __glewDrawRangeElements;
                GLEW_FUN_EXPORT PFNGLTEXIMAGE3DPROC __glewTexImage3D;
                GLEW_FUN_EXPORT PFNGLTEXSUBIMAGE3DPROC __glewTexSubImage3D;

                GLEW_FUN_EXPORT PFNGLACTIVETEXTUREPROC __glewActiveTexture;
                GLEW_FUN_EXPORT PFNGLCLIENTACTIVETEXTUREPROC __glewClientActiveTexture;
                GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXIMAGE1DPROC __glewCompressedTexImage1D;
                GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXIMAGE2DPROC __glewCompressedTexImage2D;
                GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXIMAGE3DPROC __glewCompressedTexImage3D;
                GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC __glewCompressedTexSubImage1D;
                GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC __glewCompressedTexSubImage2D;
                GLEW_FUN_EXPORT PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC __glewCompressedTexSubImage3D;
                GLEW_FUN_EXPORT PFNGLGETCOMPRESSEDTEXIMAGEPROC __glewGetCompressedTexImage;
                GLEW_FUN_EXPORT PFNGLLOADTRANSPOSEMATRIXDPROC __glewLoadTransposeMatrixd;
                GLEW_FUN_EXPORT PFNGLLOADTRANSPOSEMATRIXFPROC __glewLoadTransposeMatrixf;
                GLEW_FUN_EXPORT PFNGLMULTTRANSPOSEMATRIXDPROC __glewMultTransposeMatrixd;
                GLEW_FUN_EXPORT PFNGLMULTTRANSPOSEMATRIXFPROC __glewMultTransposeMatrixf;


recall that GLEW_FUN_EXPORT is defined as extern.

so this will extend the funciton_s visibility to the whole program. This function can be used anywhere 
in any of the files of the whole program. 





11. so if we look under GL_VERSION_1_2, we see a lot of constants being defined 


                #ifndef GL_VERSION_1_2
                #define GL_VERSION_1_2 1

                #define GL_SMOOTH_POINT_SIZE_RANGE 0x0B12
                #define GL_SMOOTH_POINT_SIZE_GRANULARITY 0x0B13
                #define GL_SMOOTH_LINE_WIDTH_RANGE 0x0B22
                #define GL_SMOOTH_LINE_WIDTH_GRANULARITY 0x0B23
                #define GL_UNSIGNED_BYTE_3_3_2 0x8032
                #define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
                #define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
                #define GL_UNSIGNED_INT_8_8_8_8 0x8035
                #define GL_UNSIGNED_INT_10_10_10_2 0x8036
                #define GL_RESCALE_NORMAL 0x803A
                #define GL_TEXTURE_BINDING_3D 0x806A
                #define GL_PACK_SKIP_IMAGES 0x806B
                #define GL_PACK_IMAGE_HEIGHT 0x806C
                #define GL_UNPACK_SKIP_IMAGES 0x806D
                #define GL_UNPACK_IMAGE_HEIGHT 0x806E
                #define GL_TEXTURE_3D 0x806F
                #define GL_PROXY_TEXTURE_3D 0x8070
                #define GL_TEXTURE_DEPTH 0x8071
                #define GL_TEXTURE_WRAP_R 0x8072
                #define GL_MAX_3D_TEXTURE_SIZE 0x8073
                #define GL_BGR 0x80E0
                #define GL_BGRA 0x80E1
                #define GL_MAX_ELEMENTS_VERTICES 0x80E8
                #define GL_MAX_ELEMENTS_INDICES 0x80E9
                #define GL_CLAMP_TO_EDGE 0x812F
                #define GL_TEXTURE_MIN_LOD 0x813A
                #define GL_TEXTURE_MAX_LOD 0x813B
                #define GL_TEXTURE_BASE_LEVEL 0x813C
                #define GL_TEXTURE_MAX_LEVEL 0x813D
                #define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
                #define GL_SINGLE_COLOR 0x81F9
                #define GL_SEPARATE_SPECULAR_COLOR 0x81FA
                #define GL_UNSIGNED_BYTE_2_3_3_REV 0x8362
                #define GL_UNSIGNED_SHORT_5_6_5 0x8363
                #define GL_UNSIGNED_SHORT_5_6_5_REV 0x8364
                #define GL_UNSIGNED_SHORT_4_4_4_4_REV 0x8365
                #define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
                #define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
                #define GL_ALIASED_POINT_SIZE_RANGE 0x846D
                #define GL_ALIASED_LINE_WIDTH_RANGE 0x846E



these are all functions you can find in glcorearb.h

so if you look at in glcorearb.h, even in there, all the constants are defined based on version. 
so essentially we are organizing our code based on versions. 


as you can see this proves that 

        "Khronos is the official group for OpenGL matters. They publish the headers.
        People at GLFW, GLEW, VS, or whatever, download the headers and incorporate in their code. "

and sure glew incorporates glcorearb.h into glew.h




so overal, glew.h contains all the function declarations and all the constants defined in glcorearb.h
just a bunch of declarations.




