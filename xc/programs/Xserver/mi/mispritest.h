/*
 * mispritestr.h
 *
 * mi sprite structures
 */

/* $XConsortium: mispritestr.h,v 1.1 89/06/09 14:53:53 keith Exp $ */

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
    CursorPtr	    pCursor;
    int		    x;
    int		    y;
    BoxRec	    saved;
    Bool	    isUp;
    Bool	    shouldBeUp;
    miSpriteCursorFuncPtr    funcs;
} miSpriteScreenRec, *miSpriteScreenPtr;

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

