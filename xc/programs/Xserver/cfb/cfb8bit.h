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

/* $XConsortium: cfb8bit.h,v 1.12 91/12/19 14:15:30 keith Exp $ */

#include "servermd.h"

#if (BITMAP_BIT_ORDER == MSBFirst)
#define GetFourBits(x)		(((unsigned long) (x)) >> 28)
#define NextFourBits(x)		((x) <<= 4)
#define NextSomeBits(x,n)	((x) <<= (n))
#else
#define GetFourBits(x)		((x) & 0xf)
#define NextFourBits(x)		((x) >>= 4)
#define NextSomeBits(x,n)	((x) >>= (n))
#endif

#define RotBitsLeft(x,k)    ((x) = BitLeft (x,k) | \
				   BitRight (x, 32-(k)))

#if defined(__GNUC__) && defined(mc68020)
#undef RotBitsLeft
#define RotBitsLeft(x,k)	asm("rol%.l %2,%0" \
				: "=d" (x) \
 				: "0" (x), "dI" (k))
#endif

#if PSZ == 8

#define GetFourPixels(x)	(cfb8StippleXor[GetFourBits(x)])
#define RRopPixels(dst,x)	(DoRRop(dst,cfb8StippleAnd[x], cfb8StippleXor[x]))
#define RRopFourPixels(dst,x)	(RRopPixels(dst,GetFourBits(x)))
#define MaskRRopPixels(dst,x,mask)  (DoMaskRRop(dst,cfb8StippleAnd[x], cfb8StippleXor[x], mask))


extern int			cfb8StippleMode, cfb8StippleAlu;
extern unsigned long		cfb8StippleFg, cfb8StippleBg, cfb8StipplePm;
extern unsigned long		cfb8StippleMasks[16];
extern unsigned long		cfb8StippleAnd[16], cfb8StippleXor[16];
extern int			cfb8StippleRRop;

#define cfb8PixelMasks	    cfb8StippleMasks
#define cfb8Pixels	    cfb8StippleXor

extern int			cfb8SetStipple (), cfb8SetOpaqueStipple();

#define cfb8CheckPixels(fg, bg) \
    (FillOpaqueStippled == cfb8StippleMode && \
     GXcopy == cfb8StippleAlu && \
     ((fg) & PMSK) == cfb8StippleFg && \
     ((bg) & PMSK) == cfb8StippleBg && \
     PMSK == cfb8StipplePm)

#define cfb8CheckOpaqueStipple(alu,fg,bg,pm) \
    ((FillOpaqueStippled == cfb8StippleMode && \
      (alu) == cfb8StippleAlu && \
      ((fg) & PMSK) == cfb8StippleFg && \
      ((bg) & PMSK) == cfb8StippleBg && \
      ((pm) & PMSK) == cfb8StipplePm) ? 0 : cfb8SetOpaqueStipple(alu,fg,bg,pm))

#define cfb8CheckStipple(alu,fg,pm) \
    ((FillStippled == cfb8StippleMode && \
      (alu) == cfb8StippleAlu && \
      ((fg) & PMSK) == cfb8StippleFg && \
      ((pm) & PMSK) == cfb8StipplePm) ? 0 : cfb8SetStipple(alu,fg,pm))

#define cfb8SetPixels(fg,bg) cfb8SetOpaqueStipple(GXcopy,fg,bg,PMSK)

/*
 * These macros are shared between the unnatural spans code
 * and the unnatural rectangle code.  No reasonable person
 * would attempt to use them anyplace else.
 */

#define NextUnnaturalStippleWord \
	if (bitsLeft >= 32) \
	{ \
	    inputBits = *srcTemp++; \
	    bitsLeft -= 32; \
	    partBitsLeft = 32; \
	} \
	else \
	{ \
	    inputBits = 0; \
	    if (bitsLeft) \
		inputBits = *srcTemp & ~cfb8BitLenMasks[bitsLeft]; \
	    srcTemp = srcStart; \
	    partBitsLeft = bitsLeft; \
	    bitsLeft = bitsWhole; \
	}

#define NextUnnaturalStippleBits \
    if (partBitsLeft >= 4) { \
	bits = GetFourBits (inputBits); \
	NextFourBits (inputBits); \
	partBitsLeft -= 4; \
    } else { \
	bits = GetFourBits (inputBits); \
	nextPartBits = 4 - partBitsLeft; \
	NextUnnaturalStippleWord \
	if (partBitsLeft < nextPartBits) { \
	    if (partBitsLeft) {\
	    	bits |= BitRight (GetFourBits (inputBits), \
				  4 - nextPartBits) & 0xf;\
	    	nextPartBits -= partBitsLeft; \
	    } \
	    NextUnnaturalStippleWord \
	} \
	bits |= BitRight (GetFourBits (inputBits), \
			  4 - nextPartBits) & 0xf; \
	NextSomeBits (inputBits, nextPartBits); \
	partBitsLeft -= nextPartBits; \
    }

#define NextUnnaturalStippleBitsFast \
    if (partBitsLeft >= 4) { \
	bits = GetFourBits(inputBits); \
	NextFourBits(inputBits); \
	partBitsLeft -= 4; \
    } else { \
	bits = GetFourBits (inputBits); \
	nextPartBits = 4 - partBitsLeft; \
	inputBits = *srcTemp++; \
	bits |= BitRight (GetFourBits (inputBits), \
		          partBitsLeft) & 0xf; \
	NextSomeBits (inputBits, nextPartBits); \
	partBitsLeft = 32 - nextPartBits; \
    }

/*
 * WriteFourBits takes the destination address, a pixel
 * value (which must be 8 bits duplicated 4 time with PFILL)
 * and the four bits to write, which must be in the low order
 * bits of the register (probably from GetFourBits) and writes
 * the appropriate locations in memory with the pixel value.  This
 * is a copy-mode only operation.
 */

#define RRopFourBits(dst,bits)					\
    {								\
    register int    _bitsTmp = (bits);				\
    *(dst) = RRopPixels(*(dst),bits);				\
    }

#define MaskRRopFourBits(dst,bits,mask)				\
    {								\
    register int    _bitsTmp = (bits);				\
    *(dst) = MaskRRopPixels(*(dst),bits,mask);			\
    }
#endif /* PSZ == 8 */

#if !defined(AVOID_MEMORY_READ) && PSZ == 8

#define WriteFourBits(dst,pixel,bits)				\
    {								\
    register unsigned long _maskTmp = cfb8PixelMasks[(bits)];   \
    *(dst) = (*(dst) & ~_maskTmp) | ((pixel) & _maskTmp);	\
    }

#define SwitchFourBits(dst,pixel,bits)				\
    {								\
    register unsigned long _maskTmp = cfb8PixelMasks[(bits)];   \
    register unsigned long _pixTmp = ((pixel) & _maskTmp);	\
    _maskTmp = ~_maskTmp;					\
    SwitchBitsLoop (*(dst) = (*(dst) & _maskTmp) | _pixTmp;)	\
    }
    
#else /* AVOID_MEMORY_READ */

#if (BITMAP_BIT_ORDER == MSBFirst)
#define Byte0	3
#define Byte1	2
#define Byte2	1
#define Byte3	0
#define Short0	1
#define Short1	0
#define Short2	3
#define Short3	2
#else
#define Byte0	0
#define Byte1	1
#define Byte2	2
#define Byte3	3
#define Short0	0
#define Short1	1
#define Short2	2
#define Short3	3
#endif
#define Long0	0
#define Long1	1
#define Long2	2
#define Long3	3

#if PSZ == 8

#define WriteFourBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((char *) (dst))[Byte0] = (pixel);	\
	    break;			\
	case 2:				\
	    ((char *) (dst))[Byte1] = (pixel);	\
	    break;			\
	case 3:				\
	    ((short *) (dst))[Short0] = (pixel);	\
	    break;			\
	case 4:				\
	    ((char *) (dst))[Byte2] = (pixel);	\
	    break;			\
	case 5:				\
	    ((char *) (dst))[Byte0] = (pixel);	\
	    ((char *) (dst))[Byte2] = (pixel);	\
	    break;			\
	case 6:				\
	    ((char *) (dst))[Byte1] = (pixel);	\
	    ((char *) (dst))[Byte2] = (pixel);	\
	    break;			\
	case 7:				\
	    ((short *) (dst))[Short0] = (pixel);	\
	    ((char *) (dst))[Byte2] = (pixel);	\
	    break;			\
	case 8:				\
	    ((char *) (dst))[Byte3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((char *) (dst))[Byte0] = (pixel);	\
	    ((char *) (dst))[Byte3] = (pixel);	\
	    break;			\
	case 10:			\
	    ((char *) (dst))[Byte1] = (pixel);	\
	    ((char *) (dst))[Byte3] = (pixel);	\
	    break;			\
	case 11:			\
	    ((short *) (dst))[Short0] = (pixel);	\
	    ((char *) (dst))[Byte3] = (pixel);	\
	    break;			\
	case 12:			\
	    ((short *) (dst))[Short1] = (pixel);	\
	    break;			\
	case 13:			\
	    ((char *) (dst))[Byte0] = (pixel);	\
	    ((short *) (dst))[Short1] = (pixel);	\
	    break;			\
	case 14:			\
	    ((char *) (dst))[Byte1] = (pixel);	\
	    ((short *) (dst))[Short1] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[0] = (pixel);	\
	    break;			\
	}


#define SwitchFourBits(dst,pixel,bits) { \
	switch (bits) { \
	case 0: \
       	    break; \
	case 1: \
	    SwitchBitsLoop (((char *) (dst))[Byte0] = (pixel);) \
	    break; \
	case 2: \
	    SwitchBitsLoop (((char *) (dst))[Byte1] = (pixel);) \
	    break; \
	case 3: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel);) \
	    break; \
	case 4: \
	    SwitchBitsLoop (((char *) (dst))[Byte2] = (pixel);) \
	    break; \
	case 5: \
	    SwitchBitsLoop (((char *) (dst))[Byte0] = (pixel); \
		     ((char *) (dst))[Byte2] = (pixel);) \
	    break; \
	case 6: \
	    SwitchBitsLoop (((char *) (dst))[Byte1] = (pixel); \
		     ((char *) (dst))[Byte2] = (pixel);) \
	    break; \
	case 7: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel); \
		     ((char *) (dst))[Byte2] = (pixel);) \
	    break; \
	case 8: \
	    SwitchBitsLoop (((char *) (dst))[Byte3] = (pixel);) \
	    break; \
	case 9: \
	    SwitchBitsLoop (((char *) (dst))[Byte0] = (pixel); \
		     ((char *) (dst))[Byte3] = (pixel);) \
	    break; \
	case 10: \
	    SwitchBitsLoop (((char *) (dst))[Byte1] = (pixel); \
		     ((char *) (dst))[Byte3] = (pixel);) \
	    break; \
	case 11: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel); \
		     ((char *) (dst))[Byte3] = (pixel);) \
	    break; \
	case 12: \
	    SwitchBitsLoop (((short *) (dst))[Short1] = (pixel);) \
	    break; \
	case 13: \
	    SwitchBitsLoop (((char *) (dst))[Byte0] = (pixel); \
		     ((short *) (dst))[Short1] = (pixel);) \
	    break; \
	case 14: \
	    SwitchBitsLoop (((char *) (dst))[Byte1] = (pixel); \
		     ((short *) (dst))[Short1] = (pixel);) \
	    break; \
	case 15: \
	    SwitchBitsLoop (((long *) (dst))[0] = (pixel);) \
	    break; \
	} \
}
#endif /* PSZ == 8 */

#if PSZ == 16

#define WriteFourBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((short *) (dst))[Short0] = (pixel);	\
	    break;			\
	case 2:				\
	    ((short *) (dst))[Short1] = (pixel);	\
	    break;			\
	case 3:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    break;			\
	case 4:				\
	    ((short *) (dst))[Short2] = (pixel);	\
	    break;			\
	case 5:				\
	    ((short *) (dst))[Short0] = (pixel);	\
	    ((short *) (dst))[Short2] = (pixel);	\
	    break;			\
	case 6:				\
	    ((short *) (dst))[Short1] = (pixel);	\
	    ((short *) (dst))[Short2] = (pixel);	\
	    break;			\
	case 7:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((short *) (dst))[Short2] = (pixel);	\
	    break;			\
	case 8:				\
	    ((short *) (dst))[Short3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((short *) (dst))[Short0] = (pixel);	\
	    ((short *) (dst))[Short3] = (pixel);	\
	    break;			\
	case 10:			\
	    ((short *) (dst))[Short1] = (pixel);	\
	    ((short *) (dst))[Short3] = (pixel);	\
	    break;			\
	case 11:			\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((short *) (dst))[Short3] = (pixel);	\
	    break;			\
	case 12:			\
	    ((long *) (dst))[Long1] = (pixel);	\
	    break;			\
	case 13:			\
	    ((short *) (dst))[Short0] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    break;			\
	case 14:			\
	    ((short *) (dst))[Short1] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    break;			\
	}

#define SwitchFourBits(dst,pixel,bits) { \
	switch (bits) { \
	case 0: \
       	    break; \
	case 1: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel);) \
	    break; \
	case 2: \
	    SwitchBitsLoop (((short *) (dst))[Short1] = (pixel);) \
	    break; \
	case 3: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel);) \
	    break; \
	case 4: \
	    SwitchBitsLoop (((short *) (dst))[Short2] = (pixel);) \
	    break; \
	case 5: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel); \
		     ((short *) (dst))[Short2] = (pixel);) \
	    break; \
	case 6: \
	    SwitchBitsLoop (((short *) (dst))[Short1] = (pixel); \
		     ((short *) (dst))[Short2] = (pixel);) \
	    break; \
	case 7: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
		     ((short *) (dst))[Short2] = (pixel);) \
	    break; \
	case 8: \
	    SwitchBitsLoop (((short *) (dst))[Short3] = (pixel);) \
	    break; \
	case 9: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel); \
		     ((short *) (dst))[Short3] = (pixel);) \
	    break; \
	case 10: \
	    SwitchBitsLoop (((short *) (dst))[Short1] = (pixel); \
		     ((short *) (dst))[Short3] = (pixel);) \
	    break; \
	case 11: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
		     ((short *) (dst))[Short3] = (pixel);) \
	    break; \
	case 12: \
	    SwitchBitsLoop (((long *) (dst))[Long1] = (pixel);) \
	    break; \
	case 13: \
	    SwitchBitsLoop (((short *) (dst))[Short0] = (pixel); \
		     ((long *) (dst))[Long1] = (pixel);) \
	    break; \
	case 14: \
	    SwitchBitsLoop (((short *) (dst))[Short1] = (pixel); \
		     ((long *) (dst))[Long1] = (pixel);) \
	    break; \
	case 15: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long1] = (pixel);) \
	    break; \
	} \
}

#endif /* PSZ == 16 */
#if PSZ == 32

#define WriteFourBits(dst,pixel,bits) \
	switch (bits) {			\
	case 0:				\
	    break;			\
	case 1:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    break;			\
	case 2:				\
	    ((long *) (dst))[Long1] = (pixel);	\
	    break;			\
	case 3:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    break;			\
	case 4:				\
	    ((long *) (dst))[Long2] = (pixel);	\
	    break;			\
	case 5:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long2] = (pixel);	\
	    break;			\
	case 6:				\
	    ((long *) (dst))[Long1] = (pixel);	\
	    ((long *) (dst))[Long2] = (pixel);	\
	    break;			\
	case 7:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    ((long *) (dst))[Long2] = (pixel);	\
	    break;			\
	case 8:				\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 9:				\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 10:			\
	    ((long *) (dst))[Long1] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 11:			\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 12:			\
	    ((long *) (dst))[Long2] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 13:			\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long2] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 14:			\
	    ((long *) (dst))[Long1] = (pixel);	\
	    ((long *) (dst))[Long2] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	case 15:			\
	    ((long *) (dst))[Long0] = (pixel);	\
	    ((long *) (dst))[Long1] = (pixel);	\
	    ((long *) (dst))[Long2] = (pixel);	\
	    ((long *) (dst))[Long3] = (pixel);	\
	    break;			\
	}

#define SwitchFourBits(dst,pixel,bits) { \
	switch (bits) { \
	case 0: \
       	    break; \
	case 1: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel);) \
	    break; \
	case 2: \
	    SwitchBitsLoop (((long *) (dst))[Long1] = (pixel);) \
	    break; \
	case 3: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long1] = (pixel);) \
	    break; \
	case 4: \
	    SwitchBitsLoop (((long *) (dst))[Long2] = (pixel);) \
	    break; \
	case 5: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long2] = (pixel);) \
	    break; \
	case 6: \
	    SwitchBitsLoop (((long *) (dst))[Long1] = (pixel); \
			    ((long *) (dst))[Long2] = (pixel);) \
	    break; \
	case 7: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long1] = (pixel); \
			    ((long *) (dst))[Long2] = (pixel);) \
	    break; \
	case 8: \
	    SwitchBitsLoop (((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 9: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 10: \
	    SwitchBitsLoop (((long *) (dst))[Long1] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 11: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long1] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 12: \
	    SwitchBitsLoop (((long *) (dst))[Long2] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 13: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long2] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 14: \
	    SwitchBitsLoop (((long *) (dst))[Long1] = (pixel); \
			    ((long *) (dst))[Long2] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	case 15: \
	    SwitchBitsLoop (((long *) (dst))[Long0] = (pixel); \
			    ((long *) (dst))[Long1] = (pixel); \
			    ((long *) (dst))[Long2] = (pixel); \
			    ((long *) (dst))[Long3] = (pixel);) \
	    break; \
	} \
}

#endif /* PSZ == 32 */
#endif /* AVOID_MEMORY_READ */

extern unsigned long	cfb8BitLenMasks[32];
extern int		cfb8ComputeClipMasks32 ();

