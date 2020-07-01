
so as mentioned in glew.h file. 

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


2.	now lets look at what glewInit(); does 

				GLenum GLEWAPIENTRY glewInit (void)
				{
				  GLenum r;
				#if defined(GLEW_EGL)
				  PFNEGLGETCURRENTDISPLAYPROC getCurrentDisplay = NULL;
				#endif
				  r = glewContextInit();
				  if ( r != 0 ) return r;
				#if defined(GLEW_EGL)
				  getCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC) glewGetProcAddress("eglGetCurrentDisplay");
				  return eglewInit(getCurrentDisplay());
				#elif defined(GLEW_OSMESA) || defined(__ANDROID__) || defined(__native_client__) || defined(__HAIKU__)
				  return r;
				#elif defined(_WIN32)
	--------->    return wglewInit();
				#elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX) /* _UNIX */
				  return glxewInit();
				#else
				  return r;
				#endif /* _WIN32 */
				}

since we are in windows, we are only interested in the _WIN32 logic path, so it looks like 

so the code that we are really interested is:

				GLenum GLEWAPIENTRY glewInit (void)
				{
				  GLenum r;
				  r = glewContextInit();
				  if ( r != 0 ) return r;
	--------->    return wglewInit();
				}


3.	as mentioned in the specs, we need to create an OpenGL context before anything else
so that is what glewContextInit(); does 




###############################################################################
####################### OpenGL Context ########################################
###############################################################################

4.	so now lets look at what glewContextInit(); does 


before we look any further, we need to introduce one function.
which is 
			
				glewGetProcAddress();


the definition for is: 				
				
				glew.c

				/*
				 * Define glewGetProcAddress.
				 */
				#if defined(GLEW_REGAL)
				#  define glewGetProcAddress(name) regalGetProcAddress((const GLchar *)name)
				#elif defined(GLEW_OSMESA)
				#  define glewGetProcAddress(name) OSMesaGetProcAddress((const char *)name)
				#elif defined(GLEW_EGL)
				#  define glewGetProcAddress(name) eglGetProcAddress((const char *)name)
				#elif defined(_WIN32)
				#  define glewGetProcAddress(name) wglGetProcAddress((LPCSTR)name)
				#elif defined(__APPLE__) && !defined(GLEW_APPLE_GLX)
				#  define glewGetProcAddress(name) NSGLGetProcAddress(name)
				#elif defined(__sgi) || defined(__sun) || defined(__HAIKU__)
				#  define glewGetProcAddress(name) dlGetProcAddress(name)
				#elif defined(__ANDROID__)
				#  define glewGetProcAddress(name) NULL /* TODO */
				#elif defined(__native_client__)
				#  define glewGetProcAddress(name) NULL /* TODO */
				#else /* __linux */
				#  define glewGetProcAddress(name) (*glXGetProcAddressARB)(name)
				#endif

as usuall, we are only interested in the _WIN32 logic path, so we see 


				#  define glewGetProcAddress(name) wglGetProcAddress((LPCSTR)name)


and we all know that wglGetProcAddress(); is the function you use to query OpenGL extension functions 
this is also used in Handmade hero
https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglgetprocaddress



5.	so to actually create the OpenGL context, we need all the OpenGL setup information. 
to do that we first need to query the glGetString(); function



				typedef const GLubyte* (GLAPIENTRY * PFNGLGETSTRINGPROC) (GLenum name);
				typedef void (GLAPIENTRY * PFNGLGETINTEGERVPROC) (GLenum pname, GLint *params);

				static GLenum GLEWAPIENTRY glewContextInit ()
				{
				  PFNGLGETSTRINGPROC getString;
				  const GLubyte* s;
				  GLuint dot;
				  GLint major, minor;
				  size_t n;

				  #ifdef _WIN32
				  getString = glGetString;
				  #else
				  getString = (PFNGLGETSTRINGPROC) glewGetProcAddress((const GLubyte*)"glGetString");
				  if (!getString)
				    return GLEW_ERROR_NO_GL_VERSION;
				  #endif






6.	after we get the glGetString(); function, we query, the major, minor, 
with the major and minor, we can know what OpnGL versions we support.
and the we can set the boolean flags for GLEW_VERSION_X_X properly 

so if we have major 3, minor 2, then any versions <= OpenGL3.2 is supported,
and we just set the GLEW_VERSION accordingly.

				static GLenum GLEWAPIENTRY glewContextInit ()
				{

					...
					...

				  /* query opengl version */
				  s = getString(GL_VERSION);
				  dot = _glewStrCLen(s, '.');
				  if (dot == 0)
				    return GLEW_ERROR_NO_GL_VERSION;

				  major = s[dot-1]-'0';
				  minor = s[dot+1]-'0';

				  if (minor < 0 || minor > 9)
				    minor = 0;
				  if (major<0 || major>9)
				    return GLEW_ERROR_NO_GL_VERSION;

				  if (major == 1 && minor == 0)
				  {
				    return GLEW_ERROR_GL_VERSION_10_ONLY;
				  }
				  else
				  {
				    GLEW_VERSION_4_6   = ( major > 4 )                 || ( major == 4 && minor >= 6 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_4_5   = GLEW_VERSION_4_4   == GL_TRUE || ( major == 4 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_4_4   = GLEW_VERSION_4_5   == GL_TRUE || ( major == 4 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_4_3   = GLEW_VERSION_4_4   == GL_TRUE || ( major == 4 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_4_2   = GLEW_VERSION_4_3   == GL_TRUE || ( major == 4 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_4_1   = GLEW_VERSION_4_2   == GL_TRUE || ( major == 4 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_4_0   = GLEW_VERSION_4_1   == GL_TRUE || ( major == 4               ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_3_3   = GLEW_VERSION_4_0   == GL_TRUE || ( major == 3 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_3_2   = GLEW_VERSION_3_3   == GL_TRUE || ( major == 3 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_3_1   = GLEW_VERSION_3_2   == GL_TRUE || ( major == 3 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_3_0   = GLEW_VERSION_3_1   == GL_TRUE || ( major == 3               ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_2_1   = GLEW_VERSION_3_0   == GL_TRUE || ( major == 2 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_2_0   = GLEW_VERSION_2_1   == GL_TRUE || ( major == 2               ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_1_5   = GLEW_VERSION_2_0   == GL_TRUE || ( major == 1 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_1_4   = GLEW_VERSION_1_5   == GL_TRUE || ( major == 1 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_1_3   = GLEW_VERSION_1_4   == GL_TRUE || ( major == 1 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_1_2_1 = GLEW_VERSION_1_3   == GL_TRUE                                 ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_1_2   = GLEW_VERSION_1_2_1 == GL_TRUE || ( major == 1 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
				    GLEW_VERSION_1_1   = GLEW_VERSION_1_2   == GL_TRUE || ( major == 1 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
				  }

				  ...
				  ...

				}




4.	once we query the versions, we query the extensions 
-	so notice that we have three crucial arrays defined in glew.c 

notice that both _glewExtensionEnabled and _glewExtensionLookup lists inside are sorted in alphabetical order 

the _glewExtensionLookup() contains a list of all the string names of the functions 

				/* Detected in the extension string or strings */
				static GLboolean  _glewExtensionString[801];
				/* Detected via extension string or experimental mode */
				static GLboolean* _glewExtensionEnabled[] = {

					...
					...

					#ifdef GL_ATI_separate_stencil
					  &__GLEW_ATI_separate_stencil,
					#endif

					...
					...

					#ifdef GL_EXT_fog_coord
					  &__GLEW_EXT_fog_coord,
					#endif

					...
					...

				}


				static const char * _glewExtensionLookup[] = {

					#ifdef GL_VERSION_1_2
					  "GL_VERSION_1_2",
					#endif
					#ifdef GL_VERSION_1_2_1
					  "GL_VERSION_1_2_1",

					...
					...
					#ifdef GL_AMD_vertex_shader_tessellator
					  "GL_AMD_vertex_shader_tessellator",
					#endif

					...
				}

then _glewExtensionEnabled(); is a list of booleans flags indicating whether the extensions exists or not. 
we first check take a guess with the boolean value with (GL_ATI_separate_stencil &__GLEW_ATI_separate_stencil);
then if they dont exist, we correct it later (this will be explained later);



whats funny is that we will doing searches in this list later on, which is why sorting this array 
in alphabetical order is useful.



this array is just a list of booleans for all the functions that are available 


-	so you can see that we first initaialize everyone inside _glewExtensionString
	to false 


-	if we have "MODERN opengl", which is OpenGL 3.0 or above,
	we are lucky in that we can just query GL_EXTENSIONS to get a space-separated list of supported extensions 	

	https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetString.xhtml

-	so we get the name of the extension 

				ext = (const char *) getStringi(GL_EXTENSIONS, i);

	then we set the flags in our arrays to true in our _glewGetExtensionString(ext) and _glewGetExtensionEnable(ext) function



-	full code below:

				  for (n = 0; n < sizeof(_glewExtensionString) / sizeof(_glewExtensionString[0]); ++n)
				    _glewExtensionString[n] = GL_FALSE;

				  if (GLEW_VERSION_3_0)
				  {
				    GLint n = 0;
				    GLint i;
				    PFNGLGETINTEGERVPROC getIntegerv;
				    PFNGLGETSTRINGIPROC getStringi;
				    const char *ext;
				    GLboolean *enable;

				    #ifdef _WIN32
				    getIntegerv = glGetIntegerv;
				    #else
				    getIntegerv = (PFNGLGETINTEGERVPROC) glewGetProcAddress((const GLubyte*)"glGetIntegerv");
				    #endif

				    if (getIntegerv)
				      getIntegerv(GL_NUM_EXTENSIONS, &n);

				    /* glGetStringi is OpenGL 3.0 */
				    getStringi = (PFNGLGETSTRINGIPROC) glewGetProcAddress((const GLubyte*)"glGetStringi");
				    if (getStringi)
				      for (i = 0; i<n; ++i)
				      {
				        ext = (const char *) getStringi(GL_EXTENSIONS, i);

				        /* Based on extension string(s), glewGetExtension purposes */
				        enable = _glewGetExtensionString(ext);
				        if (enable)
				          *enable = GL_TRUE;

				        /* Based on extension string(s), experimental mode, glewIsSupported purposes */
				        enable = _glewGetExtensionEnable(ext);
				        if (enable)
				          *enable = GL_TRUE;
				      }
				  }

				  ...
				  ...

here we will skip the non GLEW_VERSION_3_0 case. since we are lazy.


5.	once we got our list of extensions, then we do an initalization check by versions 




				#ifdef GL_VERSION_1_2
				  if (glewExperimental || GLEW_VERSION_1_2) GLEW_VERSION_1_2 = !_glewInit_GL_VERSION_1_2();
				#endif /* GL_VERSION_1_2 */
				#ifdef GL_VERSION_1_3
				  if (glewExperimental || GLEW_VERSION_1_3) GLEW_VERSION_1_3 = !_glewInit_GL_VERSION_1_3();
				#endif /* GL_VERSION_1_3 */
				#ifdef GL_VERSION_1_4
				  if (glewExperimental || GLEW_VERSION_1_4) GLEW_VERSION_1_4 = !_glewInit_GL_VERSION_1_4();
				#endif /* GL_VERSION_1_4 */
				#ifdef GL_VERSION_1_5
				  if (glewExperimental || GLEW_VERSION_1_5) GLEW_VERSION_1_5 = !_glewInit_GL_VERSION_1_5();
				#endif /* GL_VERSION_1_5 */
				#ifdef GL_VERSION_2_0
				  if (glewExperimental || GLEW_VERSION_2_0) GLEW_VERSION_2_0 = !_glewInit_GL_VERSION_2_0();
				#endif /* GL_VERSION_2_0 */
				#ifdef GL_VERSION_2_1
				  if (glewExperimental || GLEW_VERSION_2_1) GLEW_VERSION_2_1 = !_glewInit_GL_VERSION_2_1();
				#endif /* GL_VERSION_2_1 */
				#ifdef GL_VERSION_3_0
				  if (glewExperimental || GLEW_VERSION_3_0) GLEW_VERSION_3_0 = !_glewInit_GL_VERSION_3_0();
				#endif /* GL_VERSION_3_0 */
				#ifdef GL_VERSION_3_1
				  if (glewExperimental || GLEW_VERSION_3_1) GLEW_VERSION_3_1 = !_glewInit_GL_VERSION_3_1();
				#endif /* GL_VERSION_3_1 */
				#ifdef GL_VERSION_3_2
				  if (glewExperimental || GLEW_VERSION_3_2) GLEW_VERSION_3_2 = !_glewInit_GL_VERSION_3_2();
				#endif /* GL_VERSION_3_2 */
				#ifdef GL_VERSION_3_3
				  if (glewExperimental || GLEW_VERSION_3_3) GLEW_VERSION_3_3 = !_glewInit_GL_VERSION_3_3();
				#endif /* GL_VERSION_3_3 */
				#ifdef GL_VERSION_4_0
				  if (glewExperimental || GLEW_VERSION_4_0) GLEW_VERSION_4_0 = !_glewInit_GL_VERSION_4_0();
				#endif /* GL_VERSION_4_0 */
				#ifdef GL_VERSION_4_5
				  if (glewExperimental || GLEW_VERSION_4_5) GLEW_VERSION_4_5 = !_glewInit_GL_VERSION_4_5();
				#endif /* GL_VERSION_4_5 */
				#ifdef GL_VERSION_4_6
				  if (glewExperimental || GLEW_VERSION_4_6) GLEW_VERSION_4_6 = !_glewInit_GL_VERSION_4_6();
				#endif /* GL_VERSION_4_6 */



6.	for example, lets take a look at GL_VERSION_1_2. notice that we check if the functions that is released in 1.2 are actually 
supported. Meaning, that if glewGetProcAddress(); returns a valid function pointer. 

we also assign the function address to glCopyTexSubImage3D(), glDrawRangeElements(), glTexImage3D(), and glTexSubImage3D();

so now all of our extern functions are properly assigned, and people can now use it. 


				static GLboolean _glewInit_GL_VERSION_1_2 ()
				{
				  GLboolean r = GL_FALSE;

				  r = ((glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage3D")) == NULL) || r;
				  r = ((glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElements")) == NULL) || r;
				  r = ((glTexImage3D = (PFNGLTEXIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexImage3D")) == NULL) || r;
				  r = ((glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage3D")) == NULL) || r;

				  return r;
				}



#####################################################################
######################## wglewInit(); ###############################
#####################################################################				

7.	so glewContextInit(); does all the initalization that is common to all platform 
then we go into wglewInit(); which we do initalizations that is windows specific. Notice that we are only calling initalizations
on function wgl only function.s 


				Additions to the WGL Specification

				    Advertising WGL Extensions

				    Applications should call wglGetProcAddress to see whether or not
				    wglGetExtensionsStringEXT is supported.  If it is supported then it
				    can be used to determine which WGL extensions are supported by the device.

				        const char *wglGetExtensionsString(void);

				    If the function succeeds, it returns a list of supported
				    extensions to WGL.  Although the contents of the string is
				    implementation specific, the string will be NULL terminated and
				    will contain a space-separated list of extension names. (The
				    extension names themselves do not contain spaces.) If there are no
				    extensions then the empty string is returned.

				    If the function fails, the return value is NULL. To get extended
				    error information, call GetLastError.


the thing with wgl functions is that, you want to use wglGetExtensionsStringEXT(); to search for it. 

				WGL #defines a preprocessor token corresponding to the extension name in the wglext.h header provided 
				in the registry (the wgl.h supplied with Microsoft Windows does not #include wglext.h, 
					or define any extensions itself). When this token is defined, it indicates that the 
				function prototypes and enumerant definitions required to use the extension are available at compile time.


				If a WGL extension is supported at runtime, the extension name must also be 
				included in the string returned by wglGetExtensionsStringEXT.




8.	see full code below:

				GLenum GLEWAPIENTRY wglewInit ()
				{
				  GLboolean crippled;
				  const GLubyte* extStart;
				  const GLubyte* extEnd;
				  /* find wgl extension string query functions */
				  _wglewGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB");
				  _wglewGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT");
				  /* query wgl extension string */
				  if (_wglewGetExtensionsStringARB == NULL)
				    if (_wglewGetExtensionsStringEXT == NULL)
				      extStart = (const GLubyte*)"";
				    else
				      extStart = (const GLubyte*)_wglewGetExtensionsStringEXT();
				  else
				    extStart = (const GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
				  extEnd = extStart + _glewStrLen(extStart);
				  /* initialize extensions */
				  crippled = _wglewGetExtensionsStringARB == NULL && _wglewGetExtensionsStringEXT == NULL;
				#ifdef WGL_3DFX_multisample
				  WGLEW_3DFX_multisample = _glewSearchExtension("WGL_3DFX_multisample", extStart, extEnd);
				#endif /* WGL_3DFX_multisample */
				#ifdef WGL_3DL_stereo_control
				  WGLEW_3DL_stereo_control = _glewSearchExtension("WGL_3DL_stereo_control", extStart, extEnd);
				  if (glewExperimental || WGLEW_3DL_stereo_control|| crippled) WGLEW_3DL_stereo_control= !_glewInit_WGL_3DL_stereo_control();
				#endif /* WGL_3DL_stereo_control */

				 ...
				 ...

				#ifdef WGL_NV_video_output
				  WGLEW_NV_video_output = _glewSearchExtension("WGL_NV_video_output", extStart, extEnd);
				  if (glewExperimental || WGLEW_NV_video_output|| crippled) WGLEW_NV_video_output= !_glewInit_WGL_NV_video_output();
				#endif /* WGL_NV_video_output */
				#ifdef WGL_OML_sync_control
				  WGLEW_OML_sync_control = _glewSearchExtension("WGL_OML_sync_control", extStart, extEnd);
				  if (glewExperimental || WGLEW_OML_sync_control|| crippled) WGLEW_OML_sync_control= !_glewInit_WGL_OML_sync_control();
				#endif /* WGL_OML_sync_control */

				  return GLEW_OK;
				}








#####################################################################
####################### OpenGL Versions #############################
#####################################################################



so if you actually look at win32_handmade.cpp from handmade hero, you will see that Casey also uses a bunch of typedef 
to load the extension functions 


                win32_handmade.cpp

                typedef void WINAPI gl_tex_image_2d_multisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
                typedef void WINAPI gl_bind_framebuffer(GLenum target, GLuint framebuffer);
                typedef void WINAPI gl_gen_framebuffers(GLsizei n, GLuint *framebuffers);

                ...
                ...

                global_variable gl_tex_image_2d_multisample *glTexImage2DMultisample;
                global_variable gl_bind_framebuffer *glBindFramebuffer;
                global_variable gl_gen_framebuffers *glGenFramebuffers;

                ...
                ...

                glTexImage2DMultisample = (gl_tex_image_2d_multisample *)wglGetProcAddress("glTexImage2DMultisample");
                glBlitFramebuffer = (gl_blit_framebuffer *)wglGetProcAddress("glBlitFramebuffer");
                




















3.  the glewInit(); is here in glew.c 

                glew.c

                GLenum GLEWAPIENTRY glewInit (void)
                {
                  GLenum r;
                #if defined(GLEW_EGL)
                  PFNEGLGETCURRENTDISPLAYPROC getCurrentDisplay = NULL;
                #endif
                  r = glewContextInit();
                  if ( r != 0 ) return r;
                #if defined(GLEW_EGL)
                  getCurrentDisplay = (PFNEGLGETCURRENTDISPLAYPROC) glewGetProcAddress("eglGetCurrentDisplay");
                  return eglewInit(getCurrentDisplay());
                #elif defined(GLEW_OSMESA) || defined(__ANDROID__) || defined(__native_client__) || defined(__HAIKU__)
                  return r;
                #elif defined(_WIN32)
                  return wglewInit();
                #elif !defined(__APPLE__) || defined(GLEW_APPLE_GLX) /* _UNIX */
                  return glxewInit();
                #else
                  return r;
                #endif /* _WIN32 */
                }


4.  assuming we are on windows, the code that we are interested is 


                glew.c

                GLenum GLEWAPIENTRY glewInit (void)
                {
                  GLenum r;

                  ...
                  ...

                  r = glewContextInit();
                  if ( r != 0 ) return r;

                ...
                ...  

                #elif defined(_WIN32)
                  return wglewInit();
                
                ...
                ...
                }





5.  glewContextInit();

so we first need to create an OpenGL Context
https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_information_queries
https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)



only with the creation of an OpenGL context then you can move on to do other OpenGL things. 
Because OpenGL doesnt exist until you create an OpenGL Context, OpenGL context creation is not governed by the OpenGL Specification. It is instead governed by platform-specific APIs. 

as you can see, the first thing we do is to query the version 


as to why are we calling getString(); see the link:
https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_information_queries

it says: The string returned starts with "<major version>.<minor version>".

you can then see that we are determining which versions are supported.
for every boolean, the more important part is the latter part. 

as you can see, if 

                GLEW_VERSION_4_2 = ( major == 4 && minor >= 2 ) ? GL_TRUE : GL_FALSE;

-   full code below:                

                glew.c

                static GLenum GLEWAPIENTRY glewContextInit ()
                {
                    ...
                    GLint major, minor;

                    ...
                    s = getString(GL_VERSION);
                    dot = _glewStrCLen(s, '.');
                    if (dot == 0)
                        return GLEW_ERROR_NO_GL_VERSION;

                    major = s[dot-1]-'0';
                    minor = s[dot+1]-'0';

                    ...
                    ...

                    if (major == 1 && minor == 0)
                    {
                        return GLEW_ERROR_GL_VERSION_10_ONLY;
                    }
                    else
                    {
                        GLEW_VERSION_4_6   = ( major > 4 )                 || ( major == 4 && minor >= 6 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_4_5   = GLEW_VERSION_4_4   == GL_TRUE || ( major == 4 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_4_4   = GLEW_VERSION_4_5   == GL_TRUE || ( major == 4 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_4_3   = GLEW_VERSION_4_4   == GL_TRUE || ( major == 4 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_4_2   = GLEW_VERSION_4_3   == GL_TRUE || ( major == 4 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_4_1   = GLEW_VERSION_4_2   == GL_TRUE || ( major == 4 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_4_0   = GLEW_VERSION_4_1   == GL_TRUE || ( major == 4               ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_3_3   = GLEW_VERSION_4_0   == GL_TRUE || ( major == 3 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_3_2   = GLEW_VERSION_3_3   == GL_TRUE || ( major == 3 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_3_1   = GLEW_VERSION_3_2   == GL_TRUE || ( major == 3 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_3_0   = GLEW_VERSION_3_1   == GL_TRUE || ( major == 3               ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_2_1   = GLEW_VERSION_3_0   == GL_TRUE || ( major == 2 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_2_0   = GLEW_VERSION_2_1   == GL_TRUE || ( major == 2               ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_1_5   = GLEW_VERSION_2_0   == GL_TRUE || ( major == 1 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_1_4   = GLEW_VERSION_1_5   == GL_TRUE || ( major == 1 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_1_3   = GLEW_VERSION_1_4   == GL_TRUE || ( major == 1 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_1_2_1 = GLEW_VERSION_1_3   == GL_TRUE                                 ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_1_2   = GLEW_VERSION_1_2_1 == GL_TRUE || ( major == 1 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
                        GLEW_VERSION_1_1   = GLEW_VERSION_1_2   == GL_TRUE || ( major == 1 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
                    }
                }



6. so after identifying the versions, you can see we do the initalization depending on the versions 

                static GLenum GLEWAPIENTRY glewContextInit ()
                {
                    ...
                    ...

                    #ifdef GL_VERSION_1_2
                      if (glewExperimental || GLEW_VERSION_1_2) GLEW_VERSION_1_2 = !_glewInit_GL_VERSION_1_2();
                    #endif /* GL_VERSION_1_2 */
                    #ifdef GL_VERSION_1_3
                      if (glewExperimental || GLEW_VERSION_1_3) GLEW_VERSION_1_3 = !_glewInit_GL_VERSION_1_3();
                    #endif /* GL_VERSION_1_3 */
                    #ifdef GL_VERSION_1_4
                      if (glewExperimental || GLEW_VERSION_1_4) GLEW_VERSION_1_4 = !_glewInit_GL_VERSION_1_4();
                    #endif /* GL_VERSION_1_4 */
                    #ifdef GL_VERSION_1_5
                      if (glewExperimental || GLEW_VERSION_1_5) GLEW_VERSION_1_5 = !_glewInit_GL_VERSION_1_5();
                    #endif /* GL_VERSION_1_5 */
                    #ifdef GL_VERSION_2_0
                      if (glewExperimental || GLEW_VERSION_2_0) GLEW_VERSION_2_0 = !_glewInit_GL_VERSION_2_0();
                    #endif /* GL_VERSION_2_0 */
                    #ifdef GL_VERSION_2_1
                      if (glewExperimental || GLEW_VERSION_2_1) GLEW_VERSION_2_1 = !_glewInit_GL_VERSION_2_1();
                    #endif /* GL_VERSION_2_1 */
                    #ifdef GL_VERSION_3_0
                      if (glewExperimental || GLEW_VERSION_3_0) GLEW_VERSION_3_0 = !_glewInit_GL_VERSION_3_0();
                    #endif /* GL_VERSION_3_0 */
                    #ifdef GL_VERSION_3_1
                      if (glewExperimental || GLEW_VERSION_3_1) GLEW_VERSION_3_1 = !_glewInit_GL_VERSION_3_1();
                    #endif /* GL_VERSION_3_1 */
                    #ifdef GL_VERSION_3_2
                      if (glewExperimental || GLEW_VERSION_3_2) GLEW_VERSION_3_2 = !_glewInit_GL_VERSION_3_2();
                    #endif /* GL_VERSION_3_2 */
                    #ifdef GL_VERSION_3_3
                      if (glewExperimental || GLEW_VERSION_3_3) GLEW_VERSION_3_3 = !_glewInit_GL_VERSION_3_3();
                    #endif /* GL_VERSION_3_3 */
                    #ifdef GL_VERSION_4_0
                      if (glewExperimental || GLEW_VERSION_4_0) GLEW_VERSION_4_0 = !_glewInit_GL_VERSION_4_0();
                    #endif /* GL_VERSION_4_0 */
                    #ifdef GL_VERSION_4_5
                      if (glewExperimental || GLEW_VERSION_4_5) GLEW_VERSION_4_5 = !_glewInit_GL_VERSION_4_5();
                    #endif /* GL_VERSION_4_5 */
                    #ifdef GL_VERSION_4_6
                      if (glewExperimental || GLEW_VERSION_4_6) GLEW_VERSION_4_6 = !_glewInit_GL_VERSION_4_6();
                    #endif /* GL_VERSION_4_6 */

                    ...
                    ...
                }



7.  if you look at what happens in here is that
it is calling on the newly released extensions functions for OpenGL 1.2 

                glew.c 

                static GLboolean _glewInit_GL_VERSION_1_2 ()
                {
                    GLboolean r = GL_FALSE;

                    r = ((glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage3D")) == NULL) || r;
                    r = ((glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElements")) == NULL) || r;
                    r = ((glTexImage3D = (PFNGLTEXIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexImage3D")) == NULL) || r;
                    r = ((glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage3D")) == NULL) || r;

                    return r;
                }

you can see that these four functions all have the word 
https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glDrawRangeElements.xml
                glDrawRangeElements is available only if the GL version is 1.2 or greater.


https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glCopyTexSubImage3D.xml                
                glCopyTexSubImage3D is available only if the GL version is 1.2 or greater.


                ...
                ...

you get the idea 


8.  

#######################################################################################
############################### wglGetProcAddress(); ##################################
#######################################################################################

5.  the wglewInit(); is defined in glew.c 
    lets see what goes on here.

    the first thing is that we first 

                glew.c

                GLenum GLEWAPIENTRY wglewInit ()
                {
                    GLboolean crippled;
                    const GLubyte* extStart;
                    const GLubyte* extEnd;
                    /* find wgl extension string query functions */
                    _wglewGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB");
                    _wglewGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT");
                    /* query wgl extension string */
                    if (_wglewGetExtensionsStringARB == NULL)
                    {
                        if (_wglewGetExtensionsStringEXT == NULL)
                        {
                            extStart = (const GLubyte*)"";
                        }
                        else
                        {
                            extStart = (const GLubyte*)_wglewGetExtensionsStringEXT();
                        }
                    }
                    else
                    {
                        extStart = (const GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
                    }
                    extEnd = extStart + _glewStrLen(extStart);

                    ...
                    ...

                    ...
                    ...
                }



6.  first we look at the glewGetProcAddress(); function 

obviously, on windows we will only be interested in the _WIN32 path 

                #if defined(GLEW_REGAL)
                #  define glewGetProcAddress(name) regalGetProcAddress((const GLchar *)name)
                #elif defined(GLEW_OSMESA)
                #  define glewGetProcAddress(name) OSMesaGetProcAddress((const char *)name)
                #elif defined(GLEW_EGL)
                #  define glewGetProcAddress(name) eglGetProcAddress((const char *)name)
                #elif defined(_WIN32)
    ------->    #  define glewGetProcAddress(name) wglGetProcAddress((LPCSTR)name)
                #elif defined(__APPLE__) && !defined(GLEW_APPLE_GLX)
                #  define glewGetProcAddress(name) NSGLGetProcAddress(name)
                #elif defined(__sgi) || defined(__sun) || defined(__HAIKU__)
                #  define glewGetProcAddress(name) dlGetProcAddress(name)
                #elif defined(__ANDROID__)
                #  define glewGetProcAddress(name) NULL /* TODO */
                #elif defined(__native_client__)
                #  define glewGetProcAddress(name) NULL /* TODO */
                #else /* __linux */
                #  define glewGetProcAddress(name) (*glXGetProcAddressARB)(name)
                #endif


7.  so regarding wglGetProcAddress(); The specs for this function is here 

https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglgetprocaddress
        
        "The wglGetProcAddress function returns the address of an OpenGL extension function for use with the current OpenGL rendering context."


this is actually the same function that Casey uses in handmade hero. 

See "Win32LoadWGLExtensions()" in win32_handmade.cpp you can see Casey uses the wglGetProcAddress(); to query 

                win32_handmade.cpp

                internal void Win32LoadWGLExtensions(void)
                {
                    ...
                    ...

                    wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
                    wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
                    wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
                    wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");

                    ...
                    ...
                }









8.  if you look at the WGL_EXT_extensions_string.txt specification 
                
        https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_extensions_string.txt


        "Applications should call wglGetProcAddress to see whether or not
        wglGetExtensionsStringEXT is supported.  If it is supported then it
        can be used to determine which WGL extensions are supported by the device.

            const char *wglGetExtensionsString(void);

        If the function succeeds, it returns a list of supported
        extensions to WGL.  Although the contents of the string is
        implementation specific, the string will be NULL terminated and
        will contain a space-separated list of extension names. (The
        extension names themselves do not contain spaces.) If there are no
        extensions then the empty string is returned."

as you can see this will return extensions in the format of 







9.  back to the wglewInit(); function 


                GLenum GLEWAPIENTRY wglewInit ()
                {
                    ...........................................
                    ...... Querying extensions string .........
                    ...........................................

                    ...
                    ...

                    #ifdef WGL_EXT_framebuffer_sRGB
                        WGLEW_EXT_framebuffer_sRGB = _glewSearchExtension("WGL_EXT_framebuffer_sRGB", extStart, extEnd);
                    #endif /* WGL_EXT_framebuffer_sRGB */
                
                    ...
                    ...
                }



10.  The _glewSearchExtension(); is just searching the name in the space-separated list of extension names.
    here as you can see, we are just doing a brute force string comparison

                static GLboolean _glewSearchExtension (const char* name, const GLubyte *start, const GLubyte *end)
                {
                    const GLubyte* p;
                    GLuint len = _glewStrLen((const GLubyte*)name);
                    p = start;
                    while (p < end)
                    {
                        GLuint n = _glewStrCLen(p, ' ');
                        if (len == n && _glewStrSame((const GLubyte*)name, p, n)) 
                            return GL_TRUE;
                        p += n+1;
                    }
                    return GL_FALSE;
                }


11. _glewStrSame is just doing a string comparision, character by character

                static GLboolean _glewStrSame (const GLubyte* a, const GLubyte* b, GLuint n)
                {
                  GLuint i=0;
                  if(a == NULL || b == NULL)
                    return (a == NULL && b == NULL && n == 0) ? GL_TRUE : GL_FALSE;
                  while (i < n && a[i] != '\0' && b[i] != '\0' && a[i] == b[i]) i++;
                  return i == n ? GL_TRUE : GL_FALSE;
                }


