#include "../ref_gl/gl_local.h"
#include "dll.h"

void ( APIENTRY * qglColorTableEXT)( int, int, int, int, int, const void * );
void ( APIENTRY * qglMTexCoord2fSGIS)( GLenum, GLfloat, GLfloat );
void ( APIENTRY * qglLockArraysEXT) (int , int);
void ( APIENTRY * qglUnlockArraysEXT) (void);
void ( APIENTRY * qglPointParameterfEXT)( GLenum param, GLfloat value );
void ( APIENTRY * qglPointParameterfvEXT)( GLenum param, const GLfloat *value );
void ( APIENTRY * qglSelectTextureSGIS)( GLenum );
void ( APIENTRY * qglActiveTextureARB)( GLenum );
void ( APIENTRY * qglClientActiveTextureARB)( GLenum );

void *qwglGetProcAddress(char *x)
{
    return 0;
}

void QGL_Shutdown(void)
{
}

qboolean QGL_Init(const char *dllname)
{
    qglColorTableEXT        = 0; //glColorTable;

    qglMTexCoord2fSGIS      = 0;
    qglLockArraysEXT        = 0;
    qglUnlockArraysEXT      = 0;
    qglPointParameterfEXT   = 0;
    qglPointParameterfvEXT  = 0;
    qglSelectTextureSGIS    = 0;

    return true;
}

void GLimp_EnableLogging( qboolean enable )
{
}

void GLimp_LogNewFrame( void )
{
}


void* __saveds dllFindResource(int id, char *pType)
{
	return NULL;
}

void* __saveds dllLoadResource(void *pHandle)
{
	return NULL;
}

void __saveds dllFreeResource(void *pHandle)
{
	return;
}

extern struct Window *GetWindowHandle(void);
refexport_t GetRefAPI (refimport_t rimp );

dll_tExportSymbol DLL_ExportSymbols[]=
{
	{dllFindResource,"dllFindResource"},
	{dllLoadResource,"dllLoadResource"},
	{dllFreeResource,"dllFreeResource"},
	{GetRefAPI,"GetRefAPI"},
	{GetWindowHandle, "GetWindowHandle"},
	{0,0}
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
    {0, 0, 0, 0}
};

int DLL_Init()
{
    return 1;
}

void DLL_DeInit()
{
}
