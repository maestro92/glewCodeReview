http://glew.sourceforge.net/basic.html

GLEW provides two command-line utilities: one for creating a list of available extensions and visuals; 
and another for verifying extension entry points.


visualinfo: extensions and visuals
visualinfo is an extended version of glxinfo. 
The Windows version creates a file called visualinfo.txt, which contains a list of available OpenGL, 
WGL, and GLU extensions as well as a table of visuals aka. pixel formats. 
Pbuffer and MRT capable visuals are also included. For additional usage information, type visualinfo -h.

glewinfo: extension verification utility
glewinfo allows you to verify the entry points for the extensions supported on your platform. 
The Windows version reports the results to a text file called glewinfo.txt. The Unix version prints the results to stdout.


###############################################################################
############################ visualinfo.c #####################################
###############################################################################

1.	first you see a bunch usage prompts. We will skip that

				visualinfo.c

				int main (int argc, char** argv)
				{
				  GLenum err;
				  GLContext ctx;

				  /* ---------------------------------------------------------------------- */
				  /* parse arguments */
				  if (GL_TRUE == ParseArgs(argc-1, argv+1))
				  {
				#if defined(_WIN32)
				    fprintf(stderr, "Usage: visualinfo [-a] [-s] [-h] [-pf <id>]\n");
				    fprintf(stderr, "        -a: show all visuals\n");
				    fprintf(stderr, "        -s: display to stdout instead of visualinfo.txt\n");
				    fprintf(stderr, "        -pf <id>: use given pixelformat\n");
				    fprintf(stderr, "        -h: this screen\n");
				#else
				    fprintf(stderr, "Usage: visualinfo [-h] [-display <display>] [-visual <id>]\n");
				    fprintf(stderr, "        -h: this screen\n");
				    fprintf(stderr, "        -display <display>: use given display\n");
				    fprintf(stderr, "        -visual <id>: use given visual\n");
				#endif
				    return 1;
				  }


2.	then we call context creation.
	(not really sure what goes on in InitContext(); and CreateContext());
	Doesnt seem anything important. So we will skip it.

				  /* ---------------------------------------------------------------------- */
				  /* create OpenGL rendering context */
				  InitContext(&ctx);
				  if (GL_TRUE == CreateContext(&ctx))
				  {
				    fprintf(stderr, "Error: CreateContext failed\n");
				    DestroyContext(&ctx);
				    return 1;
				  }


3.	then we call glewInit(); which we covered previously

				  /* ---------------------------------------------------------------------- */
				  /* initialize GLEW */
				  glewExperimental = GL_TRUE;
				#ifdef GLEW_MX
				  err = glewContextInit(glewGetContext());
				#  ifdef _WIN32
				  err = err || wglewContextInit(wglewGetContext());
				#  elif !defined(__APPLE__) && !defined(__HAIKU__) || defined(GLEW_APPLE_GLX)
				  err = err || glxewContextInit(glxewGetContext());
				#  endif
				#else
				  err = glewInit();
				#endif
				  if (GLEW_OK != err)
				  {
				    fprintf(stderr, "Error [main]: glewInit failed: %s\n", glewGetErrorString(err));
				    DestroyContext(&ctx);
				    return 1;
				  }

				  /* ---------------------------------------------------------------------- */
				  /* open file */
				#if defined(_WIN32)
				  if (!displaystdout)
				  {
				#if defined(_MSC_VER) && (_MSC_VER >= 1400)
				    if (fopen_s(&file, "visualinfo.txt", "w") != 0)
				      file = stdout;
				#else
				    file = fopen("visualinfo.txt", "w");
				#endif
				  }
				  if (file == NULL)
				    file = stdout;
				#else
				  file = stdout;
				#endif


4.	once we do the initalization, we just print all of them out. The hard work is done in the glewInit(); funciton
so this file is pretty straightforward. 

				  /* ---------------------------------------------------------------------- */
				  /* output header information */
				  /* OpenGL extensions */
				  fprintf(file, "OpenGL vendor string: %s\n", glGetString(GL_VENDOR));
				  fprintf(file, "OpenGL renderer string: %s\n", glGetString(GL_RENDERER));
				  fprintf(file, "OpenGL version string: %s\n", glGetString(GL_VERSION));
				  fprintf(file, "OpenGL extensions (GL_): \n");
				  PrintExtensions((const char*)glGetString(GL_EXTENSIONS));

				#ifndef GLEW_NO_GLU
				  /* GLU extensions */
				  fprintf(file, "GLU version string: %s\n", gluGetString(GLU_VERSION));
				  fprintf(file, "GLU extensions (GLU_): \n");
				  PrintExtensions((const char*)gluGetString(GLU_EXTENSIONS));
				#endif

				  /* ---------------------------------------------------------------------- */
				  /* extensions string */
				#if defined(GLEW_OSMESA)
				#elif defined(GLEW_EGL)
				#elif defined(_WIN32)
				  /* WGL extensions */
				  if (WGLEW_ARB_extensions_string || WGLEW_EXT_extensions_string)
				  {
				    fprintf(file, "WGL extensions (WGL_): \n");
				    PrintExtensions(wglGetExtensionsStringARB ? 
				                    (const char*)wglGetExtensionsStringARB(ctx.dc) :
						    (const char*)wglGetExtensionsStringEXT());
				  }
				#elif defined(__APPLE__) && !defined(GLEW_APPLE_GLX)
				  
				#elif defined(__HAIKU__)

				  /* TODO */

				#else
				  /* GLX extensions */
				  fprintf(file, "GLX extensions (GLX_): \n");
				  PrintExtensions(glXQueryExtensionsString(glXGetCurrentDisplay(), 
				                                           DefaultScreen(glXGetCurrentDisplay())));
				#endif

				  /* ---------------------------------------------------------------------- */
				  /* enumerate all the formats */
				  VisualInfo(&ctx);

				  /* ---------------------------------------------------------------------- */
				  /* release resources */
				  DestroyContext(&ctx);
				  if (file != stdout)
				    fclose(file);
				  return 0;
				}




###############################################################################
############################ glewinfo.cpp #####################################
###############################################################################

glewinfo.cpp is more or less the same thing. all the hardwork is done on glewInit();
so not much to look at here.
