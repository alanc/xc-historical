/*
 * mispritest.h
 *
 * mi sprite structures
 */

/* $XConsortium: mispritest.h,v 5.1 89/06/16 17:02:03 keith Exp $ */

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

# include   "misprite.h"

/*
 * per screen information
 */

typedef struct {
    Bool	    (*CloseScreen)();
    void	    (*GetImage)();
    unsigned int    *(*GetSpans)();
    Bool	    (*CreateGC)();
    Bool	    (*CreateWindow)();
    Bool	    (*ChangeWindowAttributes)();
    void	    (*BlockHandler)();
    void	    (*InstallColormap)();
    void	    (*StoreColors)();
    CursorPtr	    pCursor;
    int		    x;
    int		    y;
    BoxRec	    saved;
    Bool	    isUp;
    Bool	    shouldBeUp;
    Bool	    checkPixels;
    xColorItem	    colors[2];
    ColormapPtr	    pColormap;
    VisualPtr	    pVisual;
    miSpriteCursorFuncPtr    funcs;
} miSpriteScreenRec, *miSpriteScreenPtr;

#define SOURCE_COLOR	0
#define MASK_COLOR	1

typedef struct {
    WindowFuncs		*wrapFuncs;
    BackingStoreFuncs	*wrapBSFuncs;
} miSpriteWindowRec, *miSpriteWindowPtr;

typedef struct {
    GCFuncs		*wrapFuncs;
    GCOps		*wrapOps;
} miSpriteGCRec, *miSpriteGCPtr;

extern void QueryGlyphExtents();

/*
 * Overlap BoxPtr and Box elements
 */
#define BOX_OVERLAP(pCbox,X1,Y1,X2,Y2) \
 	(((pCbox)->x1 <= (X2)) && ((X1) <= (pCbox)->x2) && \
	 ((pCbox)->y1 <= (Y2)) && ((Y1) <= (pCbox)->y2))

/*
 * Overlap BoxPtr, origins, and rectangle
 */
#define ORG_OVERLAP(pCbox,xorg,yorg,x,y,w,h) \
    BOX_OVERLAP((pCbox),(x)+(xorg),(y)+(yorg),(x)+(xorg)+(w),(y)+(yorg)+(h))

/*
 * Overlap BoxPtr, origins and RectPtr
 */
#define ORGRECT_OVERLAP(pCbox,xorg,yorg,pRect) \
    ORG_OVERLAP((pCbox),(xorg),(yorg),(pRect)->x,(pRect)->y, \
		(int)((pRect)->width), (int)((pRect)->height))
/*
 * Overlap BoxPtr and horizontal span
 */
#define SPN_OVERLAP(pCbox,y,x,w) BOX_OVERLAP((pCbox),(x),(y),(x)+(w),(y))

