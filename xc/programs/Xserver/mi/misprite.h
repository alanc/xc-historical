/*
 * misprite.h
 *
 * software-sprite/sprite drawing interface spec
 *
 * mi versions of these routines exist.
 */

/* $XConsortium: misprite.h,v 5.3 91/04/26 21:46:02 keith Exp $ */

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

typedef struct {
    Bool	(*RealizeCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/
#endif
);
    Bool	(*UnrealizeCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/
#endif
);
    Bool	(*PutUpCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/,
		int /*x*/,
		int /*y*/,
		unsigned long /*source*/,
		unsigned long /*mask*/
#endif
);
    Bool	(*SaveUnderCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/
#endif
);
    Bool	(*RestoreUnderCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/
#endif
);
    Bool	(*MoveCursor)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		CursorPtr /*pCursor*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/,
		int /*dx*/,
		int /*dy*/,
		unsigned long /*source*/,
		unsigned long /*mask*/
#endif
);
    Bool	(*ChangeSave)(
#if NeedNestedPrototypes
		ScreenPtr /*pScreen*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/,
		int /*dx*/,
		int /*dy*/
#endif
);

} miSpriteCursorFuncRec, *miSpriteCursorFuncPtr;

extern Bool miSpriteInitialize(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miSpriteCursorFuncPtr /*cursorFuncs*/,
    miPointerScreenFuncPtr /*screenFuncs*/
#endif
);
