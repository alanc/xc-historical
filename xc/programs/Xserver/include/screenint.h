/* $XConsortium: screenint.h,v 1.3 93/07/12 09:44:38 dpw Exp $ */
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
#ifndef SCREENINT_H
#define SCREENINT_H

#include "misc.h"

typedef struct _PixmapFormat *PixmapFormatPtr;
typedef struct _Visual *VisualPtr;
typedef struct _Depth  *DepthPtr;
typedef struct _Screen *ScreenPtr;

extern void ResetScreenPrivates(
#if NeedFunctionPrototypes
    void
#endif
);

extern int AllocateScreenPrivateIndex(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ResetWindowPrivates(
#if NeedFunctionPrototypes
    void
#endif
);

extern int AllocateWindowPrivateIndex(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool AllocateWindowPrivate(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    int /* index */,
    unsigned /* amount */
#endif
);

extern void ResetGCPrivates(
#if NeedFunctionPrototypes
    void
#endif
);

extern int AllocateGCPrivateIndex(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool AllocateGCPrivate(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    int /* index */,
    unsigned /* amount */
#endif
);

extern int AddScreen(
#if NeedFunctionPrototypes
    Bool (* /*pfnInit*/)(
#if NeedNestedPrototypes
	int /*index*/,
	ScreenPtr /*pScreen*/,
	int /*argc*/,
	char ** /*argv*/
#endif
    ),
    int /*argc*/,
    char** /*argv*/
#endif
);

#ifdef PIXPRIV

extern void ResetPixmapPrivates(
#if NeedFunctionPrototypes
    void
#endif
);

extern int AllocatePixmapPrivateIndex(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool AllocatePixmapPrivate(
#if NeedFunctionPrototypes
    ScreenPtr /* pScreen */,
    int /* index */,
    unsigned /* amount */
#endif
);

#endif /* PIXPRIV */

#endif /* SCREENINT_H */
