/*
 * cfb8bit.h
 *
 * Defines which are only useful to 8 bit color frame buffers
 */

/*
Copyright 1989 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.  M.I.T. makes no
representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
*/

/* $XConsortium: Exp $ */

#if (BITMAP_BIT_ORDER == MSBFirst)
#define GetFourBits(x)		(((x) >> 28) & 0xf)
#define BitRight(x,xoff)	((x) >> (xoff))
#define NextFourBits(x)		((x) <<= 4)
#define BitLeft(x,y)		((x) << (y))
#else
#define GetFourBits(x)		((x) & 0xf)
#define BitRight(x,xoff)	((x) << (xoff))
#define NextFourBits(x)		((x) >>= 4)
#define BitLeft(x,y)		((x) >> (y))
#endif

#define GetFourPixels(x)	(cfb8Pixels[GetFourBits(x)])

extern unsigned long		cfb8Pixels[16];
extern unsigned long		cfb8Pixelsfg;
extern unsigned long		cfb8Pixelsbg;

extern void			cfb8SetPixels ();

#define cfb8CheckPixels(fg, bg) \
    (((fg) & 0xff) == cfb8Pixelsfg && ((bg) & 0xff) == cfb8Pixelsbg)

#ifdef mips
#define AVOID_SCREEN_READ
#endif

/*
 * WriteFourBits takes the destination address, a pixel
 * value (which must be 8 bits duplicated 4 time with PFILL)
 * and the four bits to write, which must be in the low order
 * bits of the register (probably from GetFourBits) and writes
 * the appropriate locations in memory with the pixel value.  This
 * is a copy-mode only operation.
 */

#ifndef AVOID_SCREEN_READ

extern unsigned long	cfb8PixelMasks[16];

#define WriteFourBits(dst,pixel,bits)				\
    {								\
    register unsigned long _maskTmp = cfb8PixelMasks[(bits)];   \
    *(dst) = (*(dst) & ~_maskTmp) | ((pixel) & _maskTmp);	\
    }

#else

#if (BITMAP_BIT_ORDER == MSBFirst)
#define WriteFourBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 2:				\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 3:				\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 4:				\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 5:				\
	    ((char *) (dst))[3] = (pixel);	\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 6:				\
	    ((char *) (dst))[2] = (pixel);	\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 7:				\
	    ((short *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 8:				\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((char *) (dst))[3] = (pixel);	\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 10:			\
	    ((char *) (dst))[2] = (pixel);	\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 11:			\
	    ((short *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 12:			\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 13:			\
	    ((char *) (dst))[3] = (pixel);	\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 14:			\
	    ((char *) (dst))[2] = (pixel);	\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[0] = (pixel);	\
	    break;			\
	}
#else
#define WriteFourBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((char *) (dst))[0] = (pixel);	\
	    break;			\
	case 2:				\
	    ((char *) (dst))[1] = (pixel);	\
	    break;			\
	case 3:				\
	    ((short *) (dst))[0] = (pixel);	\
	    break;			\
	case 4:				\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 5:				\
	    ((char *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 6:				\
	    ((char *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 7:				\
	    ((short *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[2] = (pixel);	\
	    break;			\
	case 8:				\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((char *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 10:			\
	    ((char *) (dst))[1] = (pixel);	\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 11:			\
	    ((short *) (dst))[0] = (pixel);	\
	    ((char *) (dst))[3] = (pixel);	\
	    break;			\
	case 12:			\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 13:			\
	    ((char *) (dst))[0] = (pixel);	\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 14:			\
	    ((char *) (dst))[1] = (pixel);	\
	    ((short *) (dst))[1] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[0] = (pixel);	\
	    break;			\
	}
#endif
#endif

extern unsigned long	cfb8BitLenMasks[32];
extern int		cfb8ComputeClipMasks32 ();
