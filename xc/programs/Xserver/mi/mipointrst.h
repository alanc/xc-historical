/*
 * mipointerstr.h
 *
 */

/* $XConsortium Exp $ */

# include   <mipointer.h>
# include   <input.h>

typedef struct {
    ScreenPtr		    pScreen;    /* current screen */
    CursorPtr		    pCursor;    /* current cursor */
    BoxRec		    limits;	/* current constraints */
    int			    x, y;	/* hot spot location */
    DevicePtr		    pPointer;   /* pointer device structure */
    miPointerCursorFuncPtr  funcs;	/* device-specific methods */
} miPointerRec, *miPointerPtr;

typedef struct {
    miPointerSpriteFuncPtr  funcs;
    miPointerPtr	    pPointer;
    Bool		    (*CloseScreen)();
} miPointerScreenRec, *miPointerScreenPtr;
