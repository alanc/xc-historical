#ifndef XMD_H
#define XMD_H 1
/* $Header: Xmd.h,v 1.15 87/06/15 13:58:49 toddb Locked $ */
/*
 *  MACHINE DEPENDENT DECLARATIONS.
 *
 *  These work on the SUN, the Apollo, the RT and the VAX.
 *
 */

/*
 * Machine dependent values:
 */

#ifdef vax

#define IMAGE_BYTE_ORDER	LSBFirst        /* Values for the VAX only */
#define BITMAP_BIT_ORDER	LSBFirst
#define B16
#define B32

#else
# ifdef sun

#define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the SUN only */
#define BITMAP_BIT_ORDER	MSBFirst
#define B16
#define B32

# else
#  ifdef apollo

#define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the Apollo only*/
#define BITMAP_BIT_ORDER	MSBFirst
#define B16
#define B32

#  else
#   ifdef ibm032

#define IMAGE_BYTE_ORDER	MSBFirst        /* Values for the RT only*/
#define BITMAP_BIT_ORDER	MSBFirst
#define B16
#define B32

#   endif
#  endif
# endif
#endif

/*
 * Values the same for SUN, APOLLO and the VAX:
 */

typedef long           INT32;
typedef short          INT16;
typedef char           INT8;

typedef unsigned long CARD32;
typedef unsigned short CARD16;
typedef unsigned char  CARD8;

typedef unsigned long		BITS32;
typedef unsigned short		BITS16;
typedef unsigned char		BYTE;

typedef unsigned char            BOOL;

/* size of buffer to use with GetImage, measured in bytes. There's obviously
 * a trade-off between the amount of stack (or whatever ALLOCATE_LOCAL gives
 * you) used and the number of times the ddx routine has to be called.
 * 
 * for a 1024 x 864 bit monochrome screen  with a 32 bit word we get 
 * 8192/4 words per buffer 
 * (1024/32) = 32 words per scanline
 * 2048 words per buffer / 32 words per scanline = 64 scanlines per buffer
 * 864 scanlines / 64 scanlines = 14 buffers to draw a full screen
 */
#define IMAGE_BUFSIZE		8192

/* for a VAX/SUN/APOLLO, pad scanline to a longword */
#define BITMAP_SCANLINE_UNIT	32
#define BITMAP_SCANLINE_PAD  32

#define LOG2_BITMAP_PAD		5
#define LOG2_BYTES_PER_SCANLINE_PAD	2

/* 
	this returns the number of padding units, for depth d and width w.
    for bitmaps this can be calculated with the macros above.
    other depths require either grovelling over the formats field of the
    screenInfo or hardwired constants.
	Even though 1 isn't REALLY a legal pixmap depth, this macro
    allows it.  this makes some of the machine-independent code
    easier.
*/

#define PixmapWidthInPadUnits(w, d) \
    (((w) + BITMAP_SCANLINE_PAD-1) >> LOG2_BITMAP_PAD)

/*
	Return the number of bytes to which a scanline of the given
    depth and width will be padded.
   	Even though 1 isn't REALLY a legal pixmap depth, this macro
    allows it.  this makes some of the machine-independent code
    easier.
    (NOTE: for these two examples, you have to change some of the
    constants above.  the constants for pixmaps are hardwired to
    avoid looking them up in the screen structure.)

	if you support pixmaps of depths 1 and 8, padded to byte boundaries,
    you might use

    #define PixmapBytePad(w, d) \
        ((d) == 1 ? (PixmapWidthInPadUnits(w) << LOG2_BYTES_PER_SCANLINE_PAD) \
	          : (w))

    or, if you support depths of 1, 8, and 16, padded to 16 bits for all
    depths, you could use

    #define PixmapBytePad(w, d) \
        (((d) == 1) ? (PixmapWidthInPadUnits(w)<<LOG2_BYTES_PER_SCANLINE_PAD):\
	         ((d) == 8) ? ((w) +  ((w)&1))  \
	            : ((w) << 1)) 

*/
#define PixmapBytePad(w, d) \
    (PixmapWidthInPadUnits(w, d) << LOG2_BYTES_PER_SCANLINE_PAD)

/* Get a bit from a pixel.  pixel is a pixel of the specified depth and we
 * want the bit in position plane (where 0 is the least significant plane
 * and depth -1 is the most significant.) 
 * How you extract the bit depends on image-byte-order and depth.
 * You can assume that pixel is big enough to  hold a pixel depth bits deep.
 *
 * The result is cast to be an unsigned char.  This seems like the least
 * common denominator, no matter what depth you have, you're only getting a
 * bit out.  
 * 
 * (Currently this is used only by miGetPlane.)
 * The one bit case, shown below, is obvious.  The least significant bit
 * is the one we want.  */

#define GetBitFromPixel(pixel, plane, depth) \
    ((unsigned char) ((pixel) & (1 << (plane))))

#endif /* XMD_H */
