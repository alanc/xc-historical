/*
 * misprite.h
 *
 * software-sprite/sprite drawing interface spec
 *
 * mi versions of these routines exist.
 */

/* $XConsortium: Exp $ */

typedef struct {
    Bool	(*RealizeCursor)();	/* pScreen, pCursor */
    Bool	(*UnrealizeCursor)();	/* pScreen, pCursor */
    Bool	(*PutUpCursor)();	/* pScreen, pCursor, x, y */
    Bool	(*SaveUnderCursor)();	/* pScreen, x, y, w, h */
    Bool	(*RestoreUnderCursor)();/* pScreen, x, y, w, h */
    Bool	(*MoveCursor)();	/* pScreen, pCursor, x, y, w, h, dx, dy */
} miSpriteCursorFuncRec, *miSpriteCursorFuncPtr;
