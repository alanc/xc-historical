/*
 * mipointrst.h
 *
 */

/* $XConsortium: mipointrst.h,v 5.5 92/04/06 18:16:22 keith Exp $ */

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

# include   <mipointer.h>
# include   <input.h>

#define MOTION_SIZE	256

typedef struct {
    xTimecoord	    event;
    ScreenPtr	    pScreen;
} miHistoryRec, *miHistoryPtr;

typedef struct {
    ScreenPtr		    pScreen;    /* current screen */
    ScreenPtr		    pSpriteScreen;/* screen containing current sprite */
    CursorPtr		    pCursor;    /* current cursor */
    CursorPtr		    pSpriteCursor;/* cursor on screen */
    BoxRec		    limits;	/* current constraints */
    Bool		    confined;	/* pointer can't change screens */
    int			    x, y;	/* hot spot location */
    int			    devx, devy;	/* sprite position */
    DevicePtr		    pPointer;   /* pointer device structure */
    miHistoryRec	    history[MOTION_SIZE];
    int			    history_start, history_end;
} miPointerRec, *miPointerPtr;

typedef struct {
    miPointerSpriteFuncPtr  spriteFuncs;	/* sprite-specific methods */
    miPointerScreenFuncPtr  screenFuncs;	/* screen-specific methods */
    CloseScreenProcPtr	    CloseScreen;
    Bool		    waitForUpdate;	/* don't move cursor in SIGIO */
} miPointerScreenRec, *miPointerScreenPtr;
