/*
 * mipointer.h
 *
 */

/* $XConsortium Exp $ */

typedef struct {
    Bool	(*RealizeCursor)();	/* pScreen, pCursor */
    Bool	(*UnrealizeCursor)();	/* pScreen, pCursor */
    void	(*DisplayCursor)();	/* pScreen, pCursor, x, y */
    void	(*UndisplayCursor)();	/* pScreen, pCursor */
} miPointerSpriteFuncRec, *miPointerSpriteFuncPtr;

typedef struct {
    long	(*EventTime)();		/* pScreen */
    Bool	(*CursorOffScreen)();	/* pScreen, x, y */
    void	(*CrossScreen)();	/* pScreen, entering */
    void	(*QueueEvent)();	/* pxE, pPointer, pScreen */
} miPointerCursorFuncRec, *miPointerCursorFuncPtr;

extern void miPointerPosition (),	miRegisterPointerDevice();
extern void miPointerDeltaCursor (),	miPointerMoveCursor();
extern Bool miPointerInitialize ();
