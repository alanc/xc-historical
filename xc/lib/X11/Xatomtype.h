/* $Header: Xatomtype.h,v 11.8 88/06/29 09:19:30 rws Exp $ */

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
 * This file defines the C wire structures that correspond to built in atoms.
 * see server/dix/BuiltInAtoms for the final word on these....
 */

/* this structure may be extended, but do not change the order */
typedef struct {
    CARD32 flags B32;
    INT32 x B32, y B32;
    CARD32 width B32, height B32;
    CARD32 minWidth B32, minHeight B32;
    CARD32 maxWidth B32, maxHeight B32;
    CARD32 widthInc B32, heightInc B32;
    CARD32 minAspectX B32, minAspectY B32;
    CARD32 maxAspectX B32, maxAspectY B32;
    } xPropSizeHints;
#define NumPropSizeElements 15	/* number of elements in this structure */

/* this structure may be extended, but do not change the order */
typedef struct {
    CARD32 flags B32;
    CARD32 input B32;
    CARD32 initialState B32;
    BITS32 iconPixmap B32;
    BITS32 iconWindow B32;
    INT32  iconX B32;
    INT32  iconY B32;
    BITS32 iconMask B32;
    CARD32 windowGroup B32;
  } xPropWMHints;
#define NumPropWMHintsElements 9 /* number of elements in this structure */

/* this structure defines the icon size hints information */
typedef struct {
    CARD32 minWidth B32, minHeight B32;
    CARD32 maxWidth B32, maxHeight B32;
    CARD32 widthInc B32, heightInc B32;
  } xPropIconSize;
#define NumPropIconSizeElements 6 /* number of elements in this structure */

/* this structure may be extended, but do not change the order */
/* RGB properties */
typedef struct {
	BITS32 colormap B32;
	CARD32 red_max B32;
	CARD32 red_mult B32;
	CARD32 green_max B32;
	CARD32 green_mult B32;
	CARD32 blue_max B32;
	CARD32 blue_mult B32;
	CARD32 base_pixel B32;
} xPropStandardColormap;
#define NumPropStandardColormapElements 8
    
    
    
