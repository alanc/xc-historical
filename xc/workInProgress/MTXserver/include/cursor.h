/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: cursor.h,v 1.21 94/02/23 15:47:03 dpw Exp $ */
#ifndef CURSOR_H
#define CURSOR_H 

#include "misc.h"
#include "screenint.h"
#include "window.h"

#define NullCursor ((CursorPtr)NULL)

typedef struct _Cursor *CursorPtr;
typedef struct _CursorMetric *CursorMetricPtr;

extern CursorPtr rootCursor;

extern int FreeCursor(
#if NeedFunctionPrototypes
    pointer /*pCurs*/,
    XID /*cid*/
#endif
);

extern CursorPtr AllocCursor(
#if NeedFunctionPrototypes
    unsigned char* /*psrcbits*/,
    unsigned char* /*pmaskbits*/,
    CursorMetricPtr /*cm*/,
    unsigned /*foreRed*/,
    unsigned /*foreGreen*/,
    unsigned /*foreBlue*/,
    unsigned /*backRed*/,
    unsigned /*backGreen*/,
    unsigned /*backBlue*/
#endif
);

extern int AllocGlyphCursor(
#if NeedFunctionPrototypes
    Font /*source*/,
    unsigned int /*sourceChar*/,
    Font /*mask*/,
    unsigned int /*maskChar*/,
    unsigned /*foreRed*/,
    unsigned /*foreGreen*/,
    unsigned /*foreBlue*/,
    unsigned /*backRed*/,
    unsigned /*backGreen*/,
    unsigned /*backBlue*/,
    CursorPtr* /*ppCurs*/,
    ClientPtr /*client*/
#endif
);

extern CursorPtr CreateRootCursor(
#if NeedFunctionPrototypes
    char* /*pfilename*/,
    unsigned int /*glyph*/
#endif
);

extern int ServerBitsFromGlyph(
#if NeedFunctionPrototypes
    FontPtr /*pfont*/,
    unsigned int /*ch*/,
    register CursorMetricPtr /*cm*/,
    unsigned char ** /*ppbits*/
#endif
);

extern Bool CursorMetricsFromGlyph(
#if NeedFunctionPrototypes
    FontPtr /*pfont*/,
    unsigned /*ch*/,
    CursorMetricPtr /*cm*/
#endif
);

extern void CheckCursorConfinement(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void NewCurrentScreen(
#if NeedFunctionPrototypes
    ScreenPtr /*newScreen*/,
    int /*x*/,
    int /*y*/
#endif
);

extern Bool PointerConfinedToScreen(
#if NeedFunctionPrototypes
    void
#endif
);

extern void GetSpritePosition(
#if NeedFunctionPrototypes
    int * /*px*/,
    int * /*py*/
#endif
);

#endif /* CURSOR_H */
