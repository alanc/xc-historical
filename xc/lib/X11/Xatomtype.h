/* $Header: Xatomtype.h,v 11.9 88/08/11 17:27:54 jim Exp $ */

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

/*
 * This files defines crock C structures for calling XGetWindowProperty and
 * XChangeProperty.  All fields must be longs as the semantics of property
 * routines will handle conversion to and from actual 32 bit objects.  If your
 * compiler doesn't treat &structoflongs the same as &arrayoflongs[0], you
 * will have some work to do.
 *
 * See server/dix/BuiltInAtoms for additional information.
 */


/* this structure may be extended, but do not change the order */
typedef struct {
    CARD32 flags;
    INT32 x, y;			/* XXX - watch for sign extension problems */
    CARD32 width, height;
    CARD32 minWidth, minHeight;
    CARD32 maxWidth, maxHeight;
    CARD32 widthInc, heightInc;
    CARD32 minAspectX, minAspectY;
    CARD32 maxAspectX, maxAspectY;
    } xPropSizeHints;
#define NumPropSizeElements 15	/* number of elements in this structure */

/* this structure may be extended, but do not change the order */
typedef struct {
    CARD32 flags;
    CARD32 input;
    CARD32 initialState;
    BITS32 iconPixmap;
    BITS32 iconWindow;
    INT32  iconX;		/* XXX - watch for sign extension problems */
    INT32  iconY;		/* XXX - watch for sign extension problems */
    BITS32 iconMask;
    CARD32 windowGroup;
  } xPropWMHints;
#define NumPropWMHintsElements 9 /* number of elements in this structure */

/* this structure defines the icon size hints information */
typedef struct {
    CARD32 minWidth, minHeight;
    CARD32 maxWidth, maxHeight;
    CARD32 widthInc, heightInc;
  } xPropIconSize;
#define NumPropIconSizeElements 6 /* number of elements in this structure */

/* this structure may be extended, but do not change the order */
/* RGB properties */
typedef struct {
	BITS32 colormap;
	CARD32 red_max;
	CARD32 red_mult;
	CARD32 green_max;
	CARD32 green_mult;
	CARD32 blue_max;
	CARD32 blue_mult;
	CARD32 base_pixel;
} xPropStandardColormap;
#define NumPropStandardColormapElements 8
    
    
    
