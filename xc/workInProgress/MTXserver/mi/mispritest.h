/*
 * mispritest.h
 *
 * mi sprite structures
 */

/* $XConsortium: mispritest.h,v 1.1 93/12/27 13:21:20 rob Exp $ */

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

Copyright 1992, 1993 Data General Corporation;
Copyright 1992, 1993 OMRON Corporation  

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
neither the name OMRON or DATA GENERAL be used in advertising or publicity
pertaining to distribution of the software without specific, written prior
permission of the party whose name is to be used.  Neither OMRON or 
DATA GENERAL make any representation about the suitability of this software
for any purpose.  It is provided "as is" without express or implied warranty.  

OMRON AND DATA GENERAL EACH DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL OMRON OR DATA GENERAL BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
OF THIS SOFTWARE.

*/

# include   "misprite.h"

/*
 * per screen information
 */

typedef struct {
    CloseScreenProcPtr			CloseScreen;
    GetImageProcPtr			GetImage;
    GetSpansProcPtr			GetSpans;
#ifndef MTX
    SourceValidateProcPtr		SourceValidate;
#endif /* not MTX */
    CreateGCProcPtr			CreateGC;
#ifndef MTX
    ScreenBlockHandlerProcPtr		BlockHandler;
#endif /* not MTX */
    InstallColormapProcPtr		InstallColormap;
    StoreColorsProcPtr			StoreColors;
    PaintWindowBackgroundProcPtr	PaintWindowBackground;
    PaintWindowBorderProcPtr		PaintWindowBorder;
    CopyWindowProcPtr			CopyWindow;
    ClearToBackgroundProcPtr		ClearToBackground;
    SaveDoomedAreasProcPtr		SaveDoomedAreas;
    RestoreAreasProcPtr			RestoreAreas;

    CursorPtr	    pCursor;
    int		    x;
    int		    y;
    BoxRec	    saved;
    Bool	    isUp;
#ifndef MTX
    Bool	    shouldBeUp;
    WindowPtr	    pCacheWin;
    Bool	    isInCacheWin;
#endif /* not MTX */
    Bool	    checkPixels;
    xColorItem	    colors[2];
    ColormapPtr	    pInstalledMap;
    ColormapPtr	    pColormap;
    VisualPtr	    pVisual;
    miSpriteCursorFuncPtr    funcs;
} miSpriteScreenRec, *miSpriteScreenPtr;

#define SOURCE_COLOR	0
#define MASK_COLOR	1

/*
 * GC op wrappers
 */

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

#define LINE_SORT(x1,y1,x2,y2) \
{ int _t; \
  if (x1 > x2) { _t = x1; x1 = x2; x2 = _t; } \
  if (y1 > y2) { _t = y1; y1 = y2; y2 = _t; } }

#define LINE_OVERLAP(pCbox,x1,y1,x2,y2,lw2) \
    BOX_OVERLAP((pCbox), (x1)-(lw2), (y1)-(lw2), (x2)+(lw2), (y2)+(lw2))
