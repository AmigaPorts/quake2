/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
** GLA_IMP.C
**
** This file contains ALL Amiga specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/
#include <exec/exec.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <mgl/gl.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>

#ifndef __amigaos4__
#include <cybergraphx/cybergraphics.h>
#include <clib/chunkyppc_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#endif

#include <proto/intuition.h>
#include <proto/exec.h>
//#include <proto/cybergraphics.h>

//#ifndef __PPC__
#include <proto/timer.h>
//#endif


#include "../ref_gl/gl_local.h"

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
//struct Library *CyberGfxBase;
struct TimerBase * TimerBase;
extern struct SysBase * SysBase;

hard_cache *TextureCache = NULL;

cvar_t *gl_lockmode;
cvar_t *gl_texturecache;
cvar_t *gl_drawparticles;
cvar_t *gl_point_size;
cvar_t *gl_skipparticles; //surgeon
cvar_t *gl_mintriarea; //surgeon
cvar_t *gl_subdivide_size; //surgeon
cvar_t *gl_affinemodels; //surgeon
cvar_t *gl_mtexbuffersize; //surgeon
cvar_t *gl_framestat;

cvar_t *gl_guardband;

#ifndef __amigaos4__
GLboolean old_context = GL_FALSE;
#endif

int gl_lmaps_drawn = 0;
int gl_bpolies_drawn = 0;


extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

struct MiniGLIFace *IMiniGL = 0;
struct Library *MiniGLBase = 0;

struct GLContextIFace *context = 0;

GLboolean MiniGL_Init(void)
{
	// Open MiniGL.library and get the interface
	MiniGLBase = OpenLibrary("minigl.library", 0);
	if (!MiniGLBase)
		Com_Printf("GL_StartOpenGL() - could not load OpenGL subsystem\n");

	IMiniGL = (struct MiniGLIFace *)GetInterface(MiniGLBase, "main", 1, NULL);
	if (!IMiniGL)
	{
		CloseLibrary(MiniGLBase);
		Com_Printf("GL_StartOpenGL() - could not load OpenGL subsystem\n");
	
		return GL_FALSE;
	}

	return GL_TRUE;
}

void MiniGL_Term(void)
{
	if (IMiniGL)
	{
		DropInterface((struct Interface *)IMiniGL);
		CloseLibrary(MiniGLBase);

		IMiniGL = 0;
		MiniGLBase = 0;
	}
}


void OpenLibs(void)
{
	if (MiniGL_Init() == GL_FALSE)
	    Sys_Error("Unable to initialize MiniGL");
}

void CloseLibs(void)
{
	MiniGL_Term();
}


void GL_SetLockMode(char *mode)
{
#ifdef AUTOMATIC_LOCKING_ENABLE // surgeon: see mgl/config.h

	if (0 == stricmp(mode, "SMART"))
	{
		Com_Printf("MiniGL Lock Mode: Smart\n");
		mglLockMode(MGL_LOCK_SMART);
	}
	else if (0 == stricmp(mode, "AUTO"))
	{
		Com_Printf("MiniGL Lock Mode: Automatic\n");
		mglLockMode(MGL_LOCK_AUTOMATIC);
	}
	else
	{
		Com_Printf("MiniGL Lock Mode: Manual (per Frame)\n");
		mglLockMode(MGL_LOCK_MANUAL);
	}
#else
		Com_Printf("MiniGL Lock Mode: Manual (per Frame)\n");
#endif
}


/*
** VID_CreateWindow
*/
#define WINDOW_CLASS_NAME       "Quake 2"

qboolean VID_CreateWindow( int width, int height, qboolean fullscreen )
{
	cvar_t                  *vid_xpos, *vid_ypos;
	int                     x, y, w, h;
	int                     exstyle;
	cvar_t                  *depth, *buffers, *cv;

#ifndef __amigaos4__
	if (old_context)
	{
	    mglDeleteContext();
	    old_context = GL_FALSE;
	}
#else
	if (context)
	{
		context->DeleteContext();
	}
#endif

	depth =     ri.Cvar_Get("gl_forcedepth", "16", CVAR_ARCHIVE);
	buffers =   ri.Cvar_Get("gl_buffers", "3", CVAR_ARCHIVE);
	cv =        ri.Cvar_Get("gl_closeworkbench", "1", CVAR_ARCHIVE);
#ifndef __amigaos4__
	if (cv->value) mglProposeCloseDesktop(GL_TRUE);
	else           mglProposeCloseDesktop(GL_FALSE);

	mglChoosePixelDepth((int)depth->value);
	mglChooseNumberOfBuffers((int)buffers->value);


	gl_guardband = ri.Cvar_Get("gl_guardband", "0", CVAR_ARCHIVE); //default off for PM2 compatibility

	if(gl_guardband->value)
		mglChooseGuardBand(GL_TRUE);
	else
		mglChooseGuardBand(GL_FALSE);
#endif

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		vid_xpos = ri.Cvar_Get ("vid_xpos", "0", 0);
		vid_ypos = ri.Cvar_Get ("vid_ypos", "0", 0);
		x = vid_xpos->value;
		y = vid_ypos->value;
#ifndef __amigaos4__
		mglChooseWindowMode(GL_TRUE);
#endif
	}


/*
** surgeon: The highest number of verts is 4096
** (max point-particles) and in that case no clipping-space
** is needed. However, plenty of clippingspace is needed for
** locked vertexarrays which offsets the transformation to
** buffersize/4
**/
	/*
	** MAX_VERTS is 2048 (for alias models) 
	** MiniGL offsets clip/transform to either 
	** buffersize/2 or buffersize/4 for glDrawElements 
	** Therefore 4096 is enough for the vertexbuffer 
	*/

	/*
	** The multitexture buffer is able to store
	** buffersize/4 polygons after backface-culling, which
	** means that 4096 is able to store 1024 polys with
	** max 4096 verts in total (tightly packed)
	*/
#ifndef __amigaos4__
	mglChooseVertexBufferSize( 4096 );
#endif

//	mglChooseMtexBufferSize( 8192 );

	//base the size on #of polygons to store
	gl_mtexbuffersize = ri.Cvar_Get("gl_mtexbuffersize", "1024", CVAR_ARCHIVE);

#ifndef __amigaos4__
	if((int)gl_mtexbuffersize <= 1024)
		mglChooseMtexBufferSize( 4096 );
	else
		mglChooseMtexBufferSize( (int)gl_mtexbuffersize->value * 4);
#endif

#if 0
	//enable texture-sorted drawing for the virtual TMU1

	mglChooseTMU1Sort( TEXNUM_LIGHTMAPS, TEXNUM_LIGHTMAPS+128 ); //this is the range of lightmap-texture numbers, all inclusive.

#endif

#ifndef __amigaos4__
	old_context = (NULL == mglCreateContext(0, 0, width, height) ? GL_FALSE : GL_TRUE);
#else

	if (depth != 16)
		depth = 24;

	context = IMiniGL->CreateContextTags(
			MGLCC_Width,                    width,
			MGLCC_Height,                   height,
			MGLCC_TextureBufferSize,		65536,
			MGLCC_NoMipMapping,				FALSE,
			MGLCC_Windowed,                 !fullscreen,
			MGLCC_Buffers,                  3,
			MGLCC_PixelDepth,               depth,
			MGLCC_CloseGadget,              FALSE /*TRUE*/,
			MGLCC_SizeGadget,               FALSE /*TRUE*/,
			MGLCC_StencilBuffer,			GL_FALSE /*useStencil TODO */,
		TAG_DONE);

#endif

#ifndef __amigaos4__
	if (!old_context)
#else
	if (!context)
#endif
	{
		ri.Sys_Error (ERR_FATAL, "Couldn't create window");
		return false;
	}

#ifdef __amigaos4__
	mglMakeCurrent(context);
#endif

	if (!fullscreen)
	    MoveWindow(mglGetWindowHandle(), x, y);

	if (buffers->value == 3)
	    mglEnableSync(GL_FALSE);
	else
	    mglEnableSync(GL_TRUE);

	gl_lockmode = ri.Cvar_Get("gl_lockmode", "MANUAL", CVAR_ARCHIVE);
	GL_SetLockMode(gl_lockmode->string);

	gl_drawparticles = ri.Cvar_Get("gl_drawparticles", "1", CVAR_ARCHIVE); //surgeon: added CVAR_ARCHIVE

	gl_point_size = ri.Cvar_Get("gl_point_size", "2.25", CVAR_ARCHIVE);

	//surgeon: these two cvars are clamped every frame
	gl_skipparticles = ri.Cvar_Get("gl_skipparticles", "0", CVAR_ARCHIVE); //surgeon
	gl_mintriarea = ri.Cvar_Get("gl_mintriarea", "0.5", CVAR_ARCHIVE); //surgeon
	gl_subdivide_size = ri.Cvar_Get("gl_subdivide_size", "64", CVAR_ARCHIVE); //surgeon


	gl_framestat = ri.Cvar_Get("gl_framestat", "0", 0);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return true;
}


/*
** GLimp_SetMode
*/
rserr_t GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width, height;
	const char *win_fs[] = { "W", "FS" };

	ri.Con_Printf( PRINT_ALL, "Initializing OpenGL display\n");
	ri.Con_Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	// do a CDS if needed
	if (fullscreen)
	{
		ri.Con_Printf( PRINT_ALL, "fullscreen %d * %d\n", width, height );
		*pwidth = width;
		*pheight = height;
		if ( !VID_CreateWindow (width, height, true) )
			return rserr_invalid_mode;
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "windowed\n" );
		*pwidth = width;
		*pheight = height;
		if ( !VID_CreateWindow (width, height, false) )
			return rserr_invalid_mode;
	}

	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void )
{
#ifndef __amigaos4__
	if (old_context == GL_TRUE)
	    mglDeleteContext();

	old_context = GL_FALSE;
#else
	if (context)
	{
		context->DeleteContext();
		context = NULL;
	}
#endif

	if (TextureCache)
	{
		HARD_DeleteCache(TextureCache);
		TextureCache = NULL;
	}

	CloseLibs();
}

void GLimp_TextureSwap(void *data, int ordinal)
{
	if (data)
	{
		image_t *img = (image_t *)data;
		img->is_swapped = true;
		glDeleteTextures(1, &img->texnum);
	}
}

void GLimp_TextureReload(void *data, int ordinal)
{
	if (data)
	{
		extern void GL_RestoreImage(image_t *image);

		image_t *img = (image_t *)data;
		GL_RestoreImage(img);
		img->is_swapped = false;
	}
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/

int GLimp_Init( void *hinstance, void *wndproc )
{
	unsigned int cachesize;

	OpenLibs();

	gl_texturecache = ri.Cvar_Get("gl_texturecache", 
"4000000", CVAR_ARCHIVE);

	cachesize = (unsigned int)(gl_texturecache->value);

	if (cachesize >= 1000000)
	{
		 TextureCache = HARD_CreateCache(cachesize, (MAX_GLTEXTURES - (TEXNUM_SCRAPS - TEXNUM_LIGHTMAPS)), GLimp_TextureSwap, GLimp_TextureReload);
	}

	return 1;
}


/*
** GLimp_BeginFrame
*/
void GLimp_BeginFrame( float camera_separation )
{
	extern cvar_t *gl_debug;

	static int timed = 0;
	static struct timeval tv;
	ULONG ms;

  int current,peak;

#ifndef __amigaos4__
  mglTexMemStat(&current,&peak);
#endif

  //kprintf("Stat: %i %i\n",current,peak);

#if 0

	if (timed == 0)
	{
#if defined (__PPC__) && !defined(__amigaos4__)
	GetSysTimePPC(&tv);
#else
	  if (!TimerBase)
	TimerBase = (struct TimerBase*) FindName(&SysBase->DeviceList,"timer.device");

	   GetSysTime(&tv);
#endif
	    timed = 1;
	}
	else
	{
	    struct timeval end;
	    struct timeval start;
	    
#ifndef __PPC__
	    extern struct ExecBase *SysBase;
#endif	    

	    start = tv;

#if defined (__PPC__) && !defined(__amigaos4__)
	    GetSysTimePPC(&end);

	    tv = end;

	    SubTimePPC(&end, &start);
#else
		  if (!TimerBase)
	    TimerBase=FindName(&SysBase->DeviceList,"timer.device");      

	    GetSysTime(&end);
	    tv = end;
		  
	    SubTime(&end,&start);  
#endif

#ifdef __PPC__ 
	    ms = end.tv_micro;
#endif
	}
#endif

	if ( gl_lockmode->modified)
	{
		GL_SetLockMode(gl_lockmode->string);
		gl_lockmode->modified = false;
	}


	if (TextureCache && gl_texturecache->modified)
	{
		if(gl_texturecache->value < 1000000)
		{
			ri.Cvar_SetValue(gl_texturecache, 1000000);
			Com_Printf("Cannot accept values below 1000000\nSetting to 1000000\n");
		 }

		 HARD_ResizeCache(TextureCache, gl_texturecache->value);

		 gl_texturecache->modified = false;
	}

	if ( gl_lockmode->modified)
	{
		GL_SetLockMode(gl_lockmode->string);
		gl_lockmode->modified = false;
	}


	glHint(MGL_W_ONE_HINT, GL_FASTEST);

/*        if ( gl_bitdepth->modified )
	{
		if ( gl_bitdepth->value != 0)
		{
			ri.Cvar_SetValue( "gl_bitdepth", 0 );
			ri.Con_Printf( PRINT_ALL, "gl_bitdepth not supported (yet)\n" );
		}
		gl_bitdepth->modified = false;
	}*/
	mglLockDisplay();
#if 0
	if (gl_framestat->value == 1)
	{
	    ri.Con_Printf(PRINT_ALL, "Frame time: %d microseconds\n", ms);
	    ri.Con_Printf(PRINT_ALL, "FPS: %3.3f fps\n", 1000000.0/(float)ms);
	    ri.Con_Printf(PRINT_ALL, "LM: %d   BP: %d\n", gl_lmaps_drawn, gl_bpolies_drawn);
	}
#endif
	gl_lmaps_drawn = 0;
	gl_bpolies_drawn = 0;

}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	mglUnlockDisplay();
	mglSwitchDisplay();
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
}


struct Window *GetWindowHandle(void)
{
	return (context) ? mglGetWindowHandle() : NULL;
}

