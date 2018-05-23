/*********************************************************

custom optimized inline functions for Quake2 in-application
vertexbuffer construction.
Written by Christian Michael

- Should be included after qgl.h and gl.h.

**********************************************************/

#ifndef MGL_MACROS_H
#define MGL_MACROS_H

// The interface:

#ifdef STORM
#define QGLAPI static __inline
#else
#define QGLAPI static inline
#endif

extern GLcontext mini_CurrentContext;

#define CC mini_CurrentContext
#define MGLVERT CC->VertexBuffer[CC->VertexBufferPointer]

#undef qglTexCoord2f
#undef qglTexCoord2fv
#undef qglMultiTexCoord2fARB
#undef qglMultiTexCoord2fvARB
#undef qglActiveTextureARB

#undef qglVertex4f
#undef qglVertex3f
#undef qglVertex2f
#undef qglVertex2i
#undef qglVertex4fv
#undef qglVertex3fv
#undef qglVertex2fv

#undef qglArrayElement

#undef qglColor3ubv

#ifdef COLORBUFFER

QGLAPI void qglColor3ubv(GLubyte *v)
{
	const W3D_Float f = 1.f/255.f;

	MGLVERT.v.color.r = (W3D_Float)v[0]*f;
	MGLVERT.v.color.g = (W3D_Float)v[1]*f;
	MGLVERT.v.color.b = (W3D_Float)v[2]*f;
	MGLVERT.v.color.a = CC->CurrentColor.a;

	CC->ColorIndex = CC->VertexBufferPointer;
}

#else

QGLAPI void qglColor3ubv(GLubyte *v)
{
	const W3D_Float f = 1.f/255.f;

	CC->CurrentColor.r = (W3D_Float)v[0]*f;
	CC->CurrentColor.g = (W3D_Float)v[1]*f;
	CC->CurrentColor.b = (W3D_Float)v[2]*f;

	CC->UpdateCurrentColor = GL_TRUE;
}

#endif

//Texcoords

QGLAPI void qglTexCoord2f(GLfloat s, GLfloat t)
{
	MGLVERT.v.u = (W3D_Float)s;
	MGLVERT.v.v = (W3D_Float)t;
}


//pointer version

QGLAPI void qglTexCoord2fv(GLfloat *v)
{
	MGLVERT.v.u = (W3D_Float)v[0];
	MGLVERT.v.v = (W3D_Float)v[1];
}

//unit selection

#define qglActiveTextureARB(u) {\
CC->ActiveTexture = (GLenum)u - GL_TEXTURE0_ARB;}

//multitexture coords


QGLAPI void qglMultiTexCoord2fARB(GLenum unit, GLfloat s, GLfloat t)
{
	if(unit - GL_TEXTURE0_ARB)
	{
		MGLVERT.s1 = (GLfloat)s;
		MGLVERT.t1 = (GLfloat)t;
	}
	else
	{
		MGLVERT.s0 = (GLfloat)s;
		MGLVERT.t0 = (GLfloat)t;
	}
}


//pointer version

QGLAPI void qglMultiTexCoord2fvARB(GLenum unit, GLfloat *v)
{
	if(unit - GL_TEXTURE0_ARB)
	{
		MGLVERT.s1 = (GLfloat)v[0];
		MGLVERT.t1 = (GLfloat)v[1];
	}
	else
	{
		MGLVERT.s0 = (GLfloat)v[0];
		MGLVERT.t0 = (GLfloat)v[1];
	}
}


//Verts

#ifdef COLORBUFFER

QGLAPI void qglVertex4f(GLfloat x,GLfloat y, GLfloat z, GLfloat w)
{
	MGLVERT.bx=(GLfloat)x;
	MGLVERT.by=(GLfloat)y;
	MGLVERT.bz=(GLfloat)z;
	MGLVERT.bw=(GLfloat)w;
	MGLVERT.reserved = CC->ColorIndex;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex3f(GLfloat x,GLfloat y, GLfloat z)
{
	MGLVERT.bx=(GLfloat)x;
	MGLVERT.by=(GLfloat)y;
	MGLVERT.bz=(GLfloat)z;
	MGLVERT.bw=(GLfloat)1.f;
	MGLVERT.reserved = CC->ColorIndex;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex2f(GLfloat x, GLfloat y)
{
	MGLVERT.bx=(GLfloat)x;
	MGLVERT.by=(GLfloat)y;
	MGLVERT.bz=(GLfloat)0.f;
	MGLVERT.bw=(GLfloat)1.f;
	MGLVERT.reserved = CC->ColorIndex;

CC->VertexBufferPointer++;
}

//pointer versions

QGLAPI void qglVertex4fv(GLfloat *v)
{
	MGLVERT.bx=(GLfloat)v[0];
	MGLVERT.by=(GLfloat)v[1];
	MGLVERT.bz=(GLfloat)v[2];
	MGLVERT.bw=(GLfloat)v[3];
	MGLVERT.reserved = CC->ColorIndex;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex3fv(GLfloat *v)
{
	MGLVERT.bx=(GLfloat)v[0];
	MGLVERT.by=(GLfloat)v[1];
	MGLVERT.bz=(GLfloat)v[2];
	MGLVERT.bw=(GLfloat)1.f;
	MGLVERT.reserved = CC->ColorIndex;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex2fv(GLfloat *v)
{
	MGLVERT.bx=(GLfloat)v[0];
	MGLVERT.by=(GLfloat)v[1];
	MGLVERT.bz=(GLfloat)0.f;
	MGLVERT.bw=(GLfloat)1.f;
	MGLVERT.reserved = CC->ColorIndex;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex2i(GLint ax, GLint ay) 
{
	qglVertex2f((GLfloat)ax, (GLfloat)ay);
}

QGLAPI void qglArrayElement(GLint i) 
{
	MGLVERT.reserved = CC->ColorIndex;	
	CC->ElementIndex[CC->VertexBufferPointer++] = (UWORD)i;
}

#else

QGLAPI void qglVertex4f(GLfloat x,GLfloat y, GLfloat z, GLfloat w)
{
	if(CC->ShadeModel == GL_SMOOTH)
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	MGLVERT.bx=(GLfloat)x;
	MGLVERT.by=(GLfloat)y;
	MGLVERT.bz=(GLfloat)z;
	MGLVERT.bw=(GLfloat)w;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex3f(GLfloat x,GLfloat y, GLfloat z)
{
	if(CC->ShadeModel == GL_SMOOTH)
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	MGLVERT.bx=(GLfloat)x;
	MGLVERT.by=(GLfloat)y;
	MGLVERT.bz=(GLfloat)z;
	MGLVERT.bw=(GLfloat)1.f;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex2f(GLfloat x, GLfloat y)
{
	if(CC->ShadeModel == GL_SMOOTH)
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	MGLVERT.bx=(GLfloat)x;
	MGLVERT.by=(GLfloat)y;
	MGLVERT.bz=(GLfloat)0.f;
	MGLVERT.bw=(GLfloat)1.f;

CC->VertexBufferPointer++;
}

//pointer versions

QGLAPI void qglVertex4fv(GLfloat *v)
{
	if(CC->ShadeModel == GL_SMOOTH)
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	MGLVERT.bx=(GLfloat)v[0];
	MGLVERT.by=(GLfloat)v[1];
	MGLVERT.bz=(GLfloat)v[2];
	MGLVERT.bw=(GLfloat)v[3];

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex3fv(GLfloat *v)
{
	if(CC->ShadeModel == GL_SMOOTH)
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	MGLVERT.bx=(GLfloat)v[0];
	MGLVERT.by=(GLfloat)v[1];
	MGLVERT.bz=(GLfloat)v[2];
	MGLVERT.bw=(GLfloat)1.f;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex2fv(GLfloat *v)
{
	if(CC->ShadeModel == GL_SMOOTH)
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	MGLVERT.bx=(GLfloat)v[0];
	MGLVERT.by=(GLfloat)v[1];
	MGLVERT.bz=(GLfloat)0.f;
	MGLVERT.bw=(GLfloat)1.f;

CC->VertexBufferPointer++;
}

QGLAPI void qglVertex2i(GLint ax, GLint ay) 
{
	qglVertex2f((GLfloat)ax, (GLfloat)ay);
}

QGLAPI void qglArrayElement(GLint i) 
{
	if(CC->ShadeModel == GL_SMOOTH && !(CC->ClientState & GLCS_COLOR))
	{
		MGLVERT.v.color.r = CC->CurrentColor.r;
		MGLVERT.v.color.g = CC->CurrentColor.g;
		MGLVERT.v.color.b = CC->CurrentColor.b;
		MGLVERT.v.color.a = CC->CurrentColor.a;
	}

	CC->ElementIndex[CC->VertexBufferPointer++] = (UWORD)i;
}

#endif

#endif
