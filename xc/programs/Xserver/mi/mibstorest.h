/*
 * mibstorest.h
 *
 * internal structure definitions for mi backing store
 */

/* $XConsortium: mibstorest.h,v 5.1 89/06/16 17:02:11 keith Exp $ */

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

# include   "mibstore.h"

/*
 * One of these structures is allocated per GC used with a backing-store
 * drawable.
 */

typedef struct {
    GCPtr	    pBackingGC;	    /* Copy of the GC but with graphicsExposures
				     * set FALSE and the clientClip set to
				     * clip output to the valid regions of the
				     * backing pixmap. */
    int		    guarantee;      /* GuaranteeNothing, etc. */
    unsigned long   serialNumber;   /* clientClip computed time */
    unsigned long   stateChanges;   /* changes in parent gc since last copy */
    GCOps	    *wrapOps;	    /* wrapped ops */
    GCFuncs	    *wrapFuncs;	    /* wrapped funcs */
} miBSGCRec, *miBSGCPtr;

/*
 * one of these structures is allocated per Window with backing store
 */

typedef struct {
    GCPtr   	  pgcBlt;   	    /* GC for drawing onto screen */
    PixmapPtr	  pBackingPixmap;   /* Pixmap for saved areas */
    RegionPtr	  pSavedRegion;	    /* Valid area in pBackingPixmap */
    Bool    	  viewable; 	    /* Tracks pWin->viewable so pSavedRegion may
				     * be initialized correctly when the window
				     * is first mapped */
    int    	  status;    	    /* StatusNoPixmap, etc. */
    int		  backgroundState;  /* background type */
    PixUnion	  background;	    /* background pattern */
} miBSWindowRec, *miBSWindowPtr;

#define StatusNoPixmap	1	/* pixmap has not been created */
#define StatusVirtual	2	/* pixmap is virtual, tiled with background */
#define StatusVDirty	3	/* pixmap is virtual, visiblt has contents */
#define StatusExists	4	/* pixmap is created, no valid contents */
#define StatusContents	5	/* pixmap is created, has valid contents */

typedef struct {
    /*
     * screen func wrappers
     */
    Bool	    (*CloseScreen)();
    void	    (*GetImage)();
    unsigned int    *(*GetSpans)();
    Bool	    (*ChangeWindowAttributes)();
    Bool	    (*CreateGC)();
    Bool	    (*DestroyWindow)();
    /*
     * pointer to vector of device-specific backing store functions
     */
    miBSFuncPtr	    funcs;
} miBSScreenRec, *miBSScreenPtr;
