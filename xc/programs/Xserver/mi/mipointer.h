/*
 * mipointer.h
 *
 */

/* $XConsortium: mipointer.h,v 5.5 93/10/12 11:41:57 dpw Exp $ */

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

typedef struct _miPointerSpriteFuncRec {
    Bool	(*RealizeCursor)();	/* pScreen, pCursor */
    Bool	(*UnrealizeCursor)();	/* pScreen, pCursor */
    void	(*SetCursor)();		/* pScreen, pCursor, x, y */
    void	(*MoveCursor)();	/* pScreen, x, y */
} miPointerSpriteFuncRec, *miPointerSpriteFuncPtr;

typedef struct _miPointerScreenFuncRec {
    Bool	(*CursorOffScreen)();	/* ppScreen, px, py */
    void	(*CrossScreen)();	/* pScreen, entering */
    void	(*WarpCursor)();	/* pScreen, x, y */
    void	(*EnqueueEvent)();	/* xEvent */
    void	(*NewEventScreen)();	/* pScreen */
} miPointerScreenFuncRec, *miPointerScreenFuncPtr;

extern Bool miDCInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miPointerScreenFuncPtr /*screenFuncs*/
#endif
);

extern Bool miPointerInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miPointerSpriteFuncPtr /*spriteFuncs*/,
    miPointerScreenFuncPtr /*screenFuncs*/,
    Bool /*waitForUpdate*/
#endif
);

extern void miPointerWarpCursor(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    int /*x*/,
    int /*y*/
#endif
);

extern int miPointerGetMotionBufferSize(
#if NeedFunctionPrototypes
    void
#endif
);

extern int miPointerGetMotionEvents(
#if NeedFunctionPrototypes
    DeviceIntPtr /*pPtr*/,
    xTimecoord * /*coords*/,
    unsigned long /*start*/,
    unsigned long /*stop*/,
    ScreenPtr /*pScreen*/
#endif
);

extern void miPointerUpdate(
#if NeedFunctionPrototypes
    void
#endif
);

extern void miPointerDeltaCursor(
#if NeedFunctionPrototypes
    int /*dx*/,
    int /*dy*/,
    unsigned long /*time*/
#endif
);

extern void miPointerAbsoluteCursor(
#if NeedFunctionPrototypes
    int /*x*/,
    int /*y*/,
    unsigned long /*time*/
#endif
);

extern void miPointerPosition(
#if NeedFunctionPrototypes
    int * /*x*/,
    int * /*y*/
#endif
);

extern void miRegisterPointerDevice(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    DevicePtr /*pDevice*/
#endif
);
