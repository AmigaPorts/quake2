#ifndef _AMIGA_ENDIAN_H
#define _AMIGA_ENDIAN_H

#ifdef __PPC__

volatile static inline long LittleLong(long l)
{
    volatile long r;
    volatile long d = l;
    __asm volatile(
	  "lwbrx %0,0,%1"
	  : "=r" (r)
	  : "r" (&d)
    );

    return r;
}

volatile static inline short LittleShort(short l)
{
    volatile short r;
    volatile short d = l;
    __asm volatile (
	  "lhbrx %0,0,%1"
	  : "=r" (r)
	  : "r" (&d)
    );

    return r;
}

static inline float LittleFloat(float l)
{
    volatile float r;
    volatile float d = l;
    __asm volatile(
	  "lwbrx %0,0,%1"
	  : "=r" (r)
	  : "r" (&d)
    );

    return r;
}

#define BigShort(x) (x)
#define BigLong(x) (x)
#define BigFloat(x) (x)


#endif

#endif

