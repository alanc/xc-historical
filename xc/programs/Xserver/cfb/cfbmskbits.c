/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $XConsortium: cfbmskbits.c,v 4.9 93/01/28 20:31:23 rws Exp $ */

/*
 * ==========================================================================
 * Converted to Color Frame Buffer by smarks@sun, April-May 1987.  The "bit 
 * numbering" in the doc below really means "byte numbering" now.
 * ==========================================================================
 */

/*
   these tables are used by several macros in the cfb code.

   the vax numbers everything left to right, so bit indices on the
screen match bit indices in longwords.  the pc-rt and Sun number
bits on the screen the way they would be written on paper,
(i.e. msb to the left), and so a bit index n on the screen is
bit index 32-n in a longword

   see also cfbmskbits.h
*/
#include	<X.h>
#include	<Xmd.h>
#include	<servermd.h>
#include	"cfb.h"

#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define _cfbBits(a) a##U
#else
#define _cfbBits(a) ((unsigned int)a)
#endif

#if	(BITMAP_BIT_ORDER == MSBFirst)
#define cfbBits(v)	_cfbBits(v)
#else
#define cfbFlip2(a)	((((a) & 0x1) << 1) | (((a) & 0x2) >> 1))
#define cfbFlip4(a)	((cfbFlip2(a) << 2) | cfbFlip2(a >> 2))
#define cfbFlip8(a)	((cfbFlip4(a) << 4) | cfbFlip4(a >> 4))
#define cfbFlip16(a)	((cfbFlip8(a) << 8) | cfbFlip8(a >> 8))
#define cfbFlip32(a)	((cfbFlip16(a) << 16) | cfbFlip16(a >> 16))
#define cfbBits(a)	cfbFlip32(_cfbBits(a))
#endif

/* NOTE:
the first element in starttab could be 0xffffffff.  making it 0
lets us deal with a full first word in the middle loop, rather
than having to do the multiple reads and masks that we'd
have to do if we thought it was partial.
*/
#if PSZ == 4
unsigned int cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x0FFFFFFF),
	cfbBits(0x00FFFFFF),
	cfbBits(0x000FFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x00000FFF),
	cfbBits(0x000000FF),
	cfbBits(0x0000000F)
    };
unsigned int cfbendtab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0xF0000000),
	cfbBits(0xFF000000),
	cfbBits(0xFFF00000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFF000),
	cfbBits(0xFFFFFF00),
	cfbBits(0xFFFFFFF0)
    };
#endif

#if PSZ == 8
unsigned int cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x00FFFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x000000FF)
    };
unsigned int cfbendtab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0xFF000000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFFF00)
    };
#endif
#if PSZ == 16
unsigned int cfbstarttab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0x0000FFFF),
    };
unsigned int cfbendtab[] =
    {
	cfbBits(0x00000000),
	cfbBits(0xFFFF0000),
    };
#endif
#if PSZ == 32
unsigned int cfbstarttab[] =
    {
	cfbBits(0x00000000),
    };
unsigned int cfbendtab[] = 
    {
	cfbBits(0x00000000),
    };
#endif

/* a hack, for now, since the entries for 0 need to be all
   1 bits, not all zeros.
   this means the code DOES NOT WORK for segments of length
   0 (which is only a problem in the horizontal line code.)
*/
#if PSZ == 4
unsigned int cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x0FFFFFFF),
	cfbBits(0x00FFFFFF),
	cfbBits(0x000FFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x00000FFF),
	cfbBits(0x000000FF),
	cfbBits(0x0000000F)
    };

unsigned int cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xF0000000),
	cfbBits(0xFF000000),
	cfbBits(0xFFF00000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFF000),
	cfbBits(0xFFFFFF00),
	cfbBits(0xFFFFFFF0)
    };
#endif
#if PSZ == 8
unsigned int cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x00FFFFFF),
	cfbBits(0x0000FFFF),
	cfbBits(0x000000FF)
    };

unsigned int cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xFF000000),
	cfbBits(0xFFFF0000),
	cfbBits(0xFFFFFF00)
    };
#endif
#if PSZ == 16
unsigned int cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0x0000FFFF),
    };

unsigned int cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
	cfbBits(0xFFFF0000),
    };
#endif
#if PSZ == 32
unsigned int cfbstartpartial[] =
    {
	cfbBits(0xFFFFFFFF),
    };

unsigned int cfbendpartial[] =
    {
	cfbBits(0xFFFFFFFF),
    };
#endif

/* used for masking bits in bresenham lines
   mask[n] is used to mask out all but bit n in a longword (n is a
screen position).
   rmask[n] is used to mask out the single bit at position n (n
is a screen posiotion.)
*/

#if PSZ == 4
unsigned int cfbmask[] =
    {
	cfbBits(0xF0000000),
	cfbBits(0x0F000000),
 	cfbBits(0x00F00000),
 	cfbBits(0x000F0000),
 	cfbBits(0x0000F000),
 	cfbBits(0x00000F00),
 	cfbBits(0x000000F0),
 	cfbBits(0x0000000F)
    }; 
unsigned int cfbrmask[] = 
    {
	cfbBits(0x0FFFFFFF),
	cfbBits(0xF0FFFFFF),
 	cfbBits(0xFF0FFFFF),
 	cfbBits(0xFFF0FFFF),
 	cfbBits(0xFFFF0FFF),
 	cfbBits(0xFFFFF0FF),
 	cfbBits(0xFFFFFF0F),
 	cfbBits(0xFFFFFFF0)
    };
#endif
#if PSZ == 8
unsigned int cfbmask[] =
    {
	cfbBits(0xFF000000),
 	cfbBits(0x00FF0000),
 	cfbBits(0x0000FF00),
 	cfbBits(0x000000FF)
    }; 
unsigned int cfbrmask[] = 
    {
	cfbBits(0x00FFFFFF),
 	cfbBits(0xFF00FFFF),
 	cfbBits(0xFFFF00FF),
 	cfbBits(0xFFFFFF00)
    };
#endif
#if PSZ == 16
unsigned int cfbmask[] =
    {
	cfbBits(0xFFFF0000),
 	cfbBits(0x0000FFFF),
    }; 
unsigned int cfbrmask[] = 
    {
	cfbBits(0x0000FFFF),
 	cfbBits(0xFFFF0000),
    };
#endif
#if PSZ == 32
unsigned int cfbmask[] =
    {
	cfbBits(0xFFFFFFFF),
    }; 
unsigned int cfbrmask[] = 
    {
	cfbBits(0xFFFFFFFF),
    };
#endif

/*
 * QuartetBitsTable contains PPW+1 masks whose binary values are masks in the
 * low order quartet that contain the number of bits specified in the
 * index.  This table is used by getstipplepixels.
 */
#if PSZ == 4
unsigned int QuartetBitsTable[] = {
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00000000 */
    0x00000080,				/* 1 - 10000000 */
    0x000000C0,                         /* 2 - 11000000 */
    0x000000E0,                         /* 3 - 11100000 */
    0x000000F0,                         /* 4 - 11110000 */
    0x000000F8,                         /* 5 - 11111000 */
    0x000000FC,                         /* 6 - 11111100 */
    0x000000FE,                         /* 7 - 11111110 */
    0x000000FF                          /* 8 - 11111111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00000000 */
    0x00000001,                         /* 1 - 00000001 */
    0x00000003,                         /* 2 - 00000011 */
    0x00000007,                         /* 3 - 00000111 */
    0x0000000F,                         /* 4 - 00001111 */
    0x0000001F,                         /* 5 - 00011111 */
    0x0000003F,                         /* 6 - 00111111 */
    0x0000007F,                         /* 7 - 01111111 */
    0x000000FF                          /* 8 - 11111111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
};
#endif
#if PSZ == 8
unsigned int QuartetBitsTable[] = {
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0000 */
    0x00000008,                         /* 1 - 1000 */
    0x0000000C,                         /* 2 - 1100 */
    0x0000000E,                         /* 3 - 1110 */
    0x0000000F                          /* 4 - 1111 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0000 */
    0x00000001,                         /* 1 - 0001 */
    0x00000003,                         /* 2 - 0011 */
    0x00000007,                         /* 3 - 0111 */
    0x0000000F                          /* 4 - 1111 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
};
#endif
#if PSZ == 16
unsigned int QuartetBitsTable[] = {
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 00 */
    0x00000002,                         /* 1 - 10 */
    0x00000003,                         /* 2 - 11 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 00 */
    0x00000001,                         /* 1 - 01 */
    0x00000003,                         /* 2 - 11 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
};
#endif
#if PSZ == 32
unsigned int QuartetBitsTable[] = {
#if (BITMAP_BIT_ORDER == MSBFirst)
    0x00000000,                         /* 0 - 0 */
    0x00000001,                         /* 1 - 1 */
#else /* (BITMAP_BIT_ORDER == LSBFirst */
    0x00000000,                         /* 0 - 0 */
    0x00000001,                         /* 1 - 1 */
#endif /* (BITMAP_BIT_ORDER == MSBFirst) */
};
#endif

/*
 * QuartetPixelMaskTable is used by getstipplepixels to get a pixel mask
 * corresponding to a quartet of bits.  Note: the bit/byte order dependency
 * is handled by QuartetBitsTable above.
 */
#if PSZ == 4
unsigned int QuartetPixelMaskTable[] = {
    0x00000000,
    0x0000000F,
    0x000000F0,
    0x000000FF,
    0x00000F00,
    0x00000F0F,
    0x00000FF0,
    0x00000FFF,
    0x0000F000,
    0x0000F00F,
    0x0000F0F0,
    0x0000F0FF,
    0x0000FF00,
    0x0000FF0F,
    0x0000FFF0,
    0x0000FFFF,
    0x000F0000,
    0x000F000F,
    0x000F00F0,
    0x000F00FF,
    0x000F0F00,
    0x000F0F0F,
    0x000F0FF0,
    0x000F0FFF,
    0x000FF000,
    0x000FF00F,
    0x000FF0F0,
    0x000FF0FF,
    0x000FFF00,
    0x000FFF0F,
    0x000FFFF0,
    0x000FFFFF,
    0x00F00000,
    0x00F0000F,
    0x00F000F0,
    0x00F000FF,
    0x00F00F00,
    0x00F00F0F,
    0x00F00FF0,
    0x00F00FFF,
    0x00F0F000,
    0x00F0F00F,
    0x00F0F0F0,
    0x00F0F0FF,
    0x00F0FF00,
    0x00F0FF0F,
    0x00F0FFF0,
    0x00F0FFFF,
    0x00FF0000,
    0x00FF000F,
    0x00FF00F0,
    0x00FF00FF,
    0x00FF0F00,
    0x00FF0F0F,
    0x00FF0FF0,
    0x00FF0FFF,
    0x00FFF000,
    0x00FFF00F,
    0x00FFF0F0,
    0x00FFF0FF,
    0x00FFFF00,
    0x00FFFF0F,
    0x00FFFFF0,
    0x00FFFFFF,
    0x0F000000,
    0x0F00000F,
    0x0F0000F0,
    0x0F0000FF,
    0x0F000F00,
    0x0F000F0F,
    0x0F000FF0,
    0x0F000FFF,
    0x0F00F000,
    0x0F00F00F,
    0x0F00F0F0,
    0x0F00F0FF,
    0x0F00FF00,
    0x0F00FF0F,
    0x0F00FFF0,
    0x0F00FFFF,
    0x0F0F0000,
    0x0F0F000F,
    0x0F0F00F0,
    0x0F0F00FF,
    0x0F0F0F00,
    0x0F0F0F0F,
    0x0F0F0FF0,
    0x0F0F0FFF,
    0x0F0FF000,
    0x0F0FF00F,
    0x0F0FF0F0,
    0x0F0FF0FF,
    0x0F0FFF00,
    0x0F0FFF0F,
    0x0F0FFFF0,
    0x0F0FFFFF,
    0x0FF00000,
    0x0FF0000F,
    0x0FF000F0,
    0x0FF000FF,
    0x0FF00F00,
    0x0FF00F0F,
    0x0FF00FF0,
    0x0FF00FFF,
    0x0FF0F000,
    0x0FF0F00F,
    0x0FF0F0F0,
    0x0FF0F0FF,
    0x0FF0FF00,
    0x0FF0FF0F,
    0x0FF0FFF0,
    0x0FF0FFFF,
    0x0FFF0000,
    0x0FFF000F,
    0x0FFF00F0,
    0x0FFF00FF,
    0x0FFF0F00,
    0x0FFF0F0F,
    0x0FFF0FF0,
    0x0FFF0FFF,
    0x0FFFF000,
    0x0FFFF00F,
    0x0FFFF0F0,
    0x0FFFF0FF,
    0x0FFFFF00,
    0x0FFFFF0F,
    0x0FFFFFF0,
    0x0FFFFFFF,
    0xF0000000,
    0xF000000F,
    0xF00000F0,
    0xF00000FF,
    0xF0000F00,
    0xF0000F0F,
    0xF0000FF0,
    0xF0000FFF,
    0xF000F000,
    0xF000F00F,
    0xF000F0F0,
    0xF000F0FF,
    0xF000FF00,
    0xF000FF0F,
    0xF000FFF0,
    0xF000FFFF,
    0xF00F0000,
    0xF00F000F,
    0xF00F00F0,
    0xF00F00FF,
    0xF00F0F00,
    0xF00F0F0F,
    0xF00F0FF0,
    0xF00F0FFF,
    0xF00FF000,
    0xF00FF00F,
    0xF00FF0F0,
    0xF00FF0FF,
    0xF00FFF00,
    0xF00FFF0F,
    0xF00FFFF0,
    0xF00FFFFF,
    0xF0F00000,
    0xF0F0000F,
    0xF0F000F0,
    0xF0F000FF,
    0xF0F00F00,
    0xF0F00F0F,
    0xF0F00FF0,
    0xF0F00FFF,
    0xF0F0F000,
    0xF0F0F00F,
    0xF0F0F0F0,
    0xF0F0F0FF,
    0xF0F0FF00,
    0xF0F0FF0F,
    0xF0F0FFF0,
    0xF0F0FFFF,
    0xF0FF0000,
    0xF0FF000F,
    0xF0FF00F0,
    0xF0FF00FF,
    0xF0FF0F00,
    0xF0FF0F0F,
    0xF0FF0FF0,
    0xF0FF0FFF,
    0xF0FFF000,
    0xF0FFF00F,
    0xF0FFF0F0,
    0xF0FFF0FF,
    0xF0FFFF00,
    0xF0FFFF0F,
    0xF0FFFFF0,
    0xF0FFFFFF,
    0xFF000000,
    0xFF00000F,
    0xFF0000F0,
    0xFF0000FF,
    0xFF000F00,
    0xFF000F0F,
    0xFF000FF0,
    0xFF000FFF,
    0xFF00F000,
    0xFF00F00F,
    0xFF00F0F0,
    0xFF00F0FF,
    0xFF00FF00,
    0xFF00FF0F,
    0xFF00FFF0,
    0xFF00FFFF,
    0xFF0F0000,
    0xFF0F000F,
    0xFF0F00F0,
    0xFF0F00FF,
    0xFF0F0F00,
    0xFF0F0F0F,
    0xFF0F0FF0,
    0xFF0F0FFF,
    0xFF0FF000,
    0xFF0FF00F,
    0xFF0FF0F0,
    0xFF0FF0FF,
    0xFF0FFF00,
    0xFF0FFF0F,
    0xFF0FFFF0,
    0xFF0FFFFF,
    0xFFF00000,
    0xFFF0000F,
    0xFFF000F0,
    0xFFF000FF,
    0xFFF00F00,
    0xFFF00F0F,
    0xFFF00FF0,
    0xFFF00FFF,
    0xFFF0F000,
    0xFFF0F00F,
    0xFFF0F0F0,
    0xFFF0F0FF,
    0xFFF0FF00,
    0xFFF0FF0F,
    0xFFF0FFF0,
    0xFFF0FFFF,
    0xFFFF0000,
    0xFFFF000F,
    0xFFFF00F0,
    0xFFFF00FF,
    0xFFFF0F00,
    0xFFFF0F0F,
    0xFFFF0FF0,
    0xFFFF0FFF,
    0xFFFFF000,
    0xFFFFF00F,
    0xFFFFF0F0,
    0xFFFFF0FF,
    0xFFFFFF00,
    0xFFFFFF0F,
    0xFFFFFFF0,
    0xFFFFFFFF,
};
#endif
#if PSZ == 8
unsigned int QuartetPixelMaskTable[] = {
    0x00000000,
    0x000000FF,
    0x0000FF00,
    0x0000FFFF,
    0x00FF0000,
    0x00FF00FF,
    0x00FFFF00,
    0x00FFFFFF,
    0xFF000000,
    0xFF0000FF,
    0xFF00FF00,
    0xFF00FFFF,
    0xFFFF0000,
    0xFFFF00FF,
    0xFFFFFF00,
    0xFFFFFFFF
};
#endif
#if PSZ == 16
unsigned int QuartetPixelMaskTable[] = {
    0x00000000,
    0x0000FFFF,
    0xFFFF0000,
    0xFFFFFFFF,
};
#endif
#if PSZ == 32
unsigned int QuartetPixelMaskTable[] = {
    0x00000000,
    0xFFFFFFFF,
};
#endif
