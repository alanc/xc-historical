/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: misc.h,v 1.62 94/01/02 09:17:14 rob Exp $ */
#ifndef MISC_H
#define MISC_H 1
/*
 *  X internal definitions 
 *
 */

extern unsigned long globalSerialNumber;
extern unsigned long serverGeneration;

#include <X11/Xosdefs.h>
#include <X11/Xfuncproto.h>
#include <X11/Xmd.h>

#ifndef NULL
#ifndef X_NOT_STDC_ENV
#include <stddef.h>
#else
#define NULL            0
#endif
#endif

#ifndef MAXSCREENS
#define MAXSCREENS	3
#endif
#define MAXCLIENTS	128
#define MAXFORMATS	8
#define MAXVISUALS_PER_SCREEN 50

#if NeedFunctionPrototypes
typedef void *pointer;
#else
typedef unsigned char *pointer;
#endif
typedef int Bool;
typedef unsigned long PIXEL;
typedef unsigned long ATOM;


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef _XTYPEDEF_FONTPTR
typedef struct _Font *FontPtr; /* also in fonts/include/font.h */
#define _XTYPEDEF_FONTPTR
#endif

#ifndef _XTYPEDEF_CLIENTPTR
typedef struct _Client *ClientPtr; /* also in dix.h */
#define _XTYPEDEF_CLIENTPTR
#endif

#include "os.h" 	/* for ALLOCATE_LOCAL and DEALLOCATE_LOCAL */
#include <X11/Xfuncs.h> /* for bcopy, bzero, and bcmp */

#define NullBox ((BoxPtr)0)
#define MILLI_PER_MIN (1000 * 60)
#define MILLI_PER_SECOND (1000)

    /* this next is used with None and ParentRelative to tell
       PaintWin() what to use to paint the background. Also used
       in the macro IS_VALID_PIXMAP */

#define USE_BACKGROUND_PIXEL 3
#define USE_BORDER_PIXEL 3

#ifdef MTX
#define NOSWAP 0
#define SWAP   1
#define UNKNOWN 2
#endif



/* byte swap a 32-bit literal */
#define lswapl(x) ((((x) & 0xff) << 24) |\
		   (((x) & 0xff00) << 8) |\
		   (((x) & 0xff0000) >> 8) |\
		   (((x) >> 24) & 0xff))

/* byte swap a short literal */
#define lswaps(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#ifndef abs
#define abs(a) ((a) > 0 ? (a) : -(a))
#endif
#ifndef fabs
#define fabs(a) ((a) > 0.0 ? (a) : -(a))	/* floating absolute value */
#endif
#define sign(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
/* this assumes b > 0 */
#define modulus(a, b, d)    if (((d) = (a) % (b)) < 0) (d) += (b)
/*
 * return the least significant bit in x which is set
 *
 * This works on 1's complement and 2's complement machines.
 * If you care about the extra instruction on 2's complement
 * machines, change to ((x) & (-(x)))
 */
#define lowbit(x) ((x) & (~(x) + 1))

#define MAXSHORT 32767
#define MINSHORT -MAXSHORT 


/* some macros to help swap requests, replies, and events */

#define LengthRestB(stuff) \
    ((client->req_len << 2) - sizeof(*stuff))

#define LengthRestS(stuff) \
    ((client->req_len << 1) - (sizeof(*stuff) >> 1))

#define LengthRestL(stuff) \
    (client->req_len - (sizeof(*stuff) >> 2))

#define SwapRestS(stuff) \
    SwapShorts((short *)(stuff + 1), LengthRestS(stuff))

#define SwapRestL(stuff) \
    SwapLongs((CARD32 *)(stuff + 1), LengthRestL(stuff))

/* byte swap a 32-bit value */
#define swapl(x, n) n = ((char *) (x))[0];\
		 ((char *) (x))[0] = ((char *) (x))[3];\
		 ((char *) (x))[3] = n;\
		 n = ((char *) (x))[1];\
		 ((char *) (x))[1] = ((char *) (x))[2];\
		 ((char *) (x))[2] = n;

/* byte swap a short */
#define swaps(x, n) n = ((char *) (x))[0];\
		 ((char *) (x))[0] = ((char *) (x))[1];\
		 ((char *) (x))[1] = n

/* copy 32-bit value from src to dst byteswapping on the way */
#define cpswapl(src, dst) \
                 ((char *)&(dst))[0] = ((char *) &(src))[3];\
                 ((char *)&(dst))[1] = ((char *) &(src))[2];\
                 ((char *)&(dst))[2] = ((char *) &(src))[1];\
                 ((char *)&(dst))[3] = ((char *) &(src))[0];

/* copy short from src to dst byteswapping on the way */
#define cpswaps(src, dst)\
		 ((char *) &(dst))[0] = ((char *) &(src))[1];\
		 ((char *) &(dst))[1] = ((char *) &(src))[0];

extern void SwapLongs(
#if NeedFunctionPrototypes
    CARD32 *list,
    unsigned long count
#endif
);

extern void SwapShorts(
#if NeedFunctionPrototypes
    short *list,
    unsigned long count
#endif
);

typedef struct _xPoint *DDXPointPtr;
typedef struct _Box *BoxPtr;
typedef struct _xEvent *xEventPtr;
typedef struct _xRectangle *xRectanglePtr;

/*  typedefs from other places - duplicated here to minimize the amount
 *  of unnecessary junk that one would normally have to include to get
 *  these symbols defined
 */

#ifndef _XTYPEDEF_CHARINFOPTR
typedef struct _CharInfo *CharInfoPtr; /* also in fonts/include/font.h */
#define _XTYPEDEF_CHARINFOPTR
#endif

#endif /* MISC_H */
