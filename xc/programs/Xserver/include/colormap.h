/*
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

*/
/* $XConsortium: colormap.h,v 1.25 93/09/20 16:16:42 dpw Exp $ */
#ifndef CMAP_H
#define CMAP_H 1

#include "X11/Xproto.h"
#include "screenint.h"
#include "window.h"

/* these follow X.h's AllocNone and AllocAll */
#define CM_PSCREEN 2
#define CM_PWIN	   3
/* Passed internally in colormap.c */
#define REDMAP 0
#define GREENMAP 1
#define BLUEMAP 2
#define PSEUDOMAP 3
#define AllocPrivate (-1)
#define AllocTemporary (-2)
#define DynamicClass  1

/* Values for the flags field of a colormap. These should have 1 bit set
 * and not overlap */
#define IsDefault 1
#define AllAllocated 2
#define BeingCreated 4


typedef unsigned long	Pixel;
typedef struct _CMEntry *EntryPtr;
typedef struct _ColormapRec *ColormapPtr;
typedef struct _colorResource *colorResourcePtr;

extern int CreateColormap(
#if NeedFunctionPrototypes
    Colormap /*mid*/,
    ScreenPtr /*pScreen*/,
    VisualPtr /*pVisual*/,
    ColormapPtr* /*ppcmap*/,
    int /*alloc*/,
    int /*client*/
#endif
);

extern int FreeColormap(
#if NeedFunctionPrototypes
    pointer /*pmap*/,
    XID /*mid*/
#endif
);

extern int TellLostMap(
#if NeedFunctionPrototypes
    WindowPtr /*pwin*/,
    pointer /* Colormap *pmid */
#endif
);

extern int TellGainedMap(
#if NeedFunctionPrototypes
    WindowPtr /*pwin*/,
    pointer /* Colormap *pmid */
#endif
);

extern int CopyColormapAndFree(
#if NeedFunctionPrototypes
    Colormap /*mid*/,
    ColormapPtr /*pSrc*/,
    int /*client*/
#endif
);

extern int AllocColor(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    unsigned short* /*pred*/,
    unsigned short* /*pgreen*/,
    unsigned short* /*pblue*/,
    Pixel* /*pPix*/,
    int /*client*/
#endif
);

extern void FakeAllocColor(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    xColorItem * /*item*/
#endif
);

extern void FakeFreeColor(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    Pixel /*pixel*/
#endif
);

typedef int (*ColorCompareProcPtr)(
#if NeedNestedPrototypes
    EntryPtr /*pent*/,
    xrgb * /*prgb*/
#endif
);

extern int FindColor(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    EntryPtr /*pentFirst*/,
    int /*size*/,
    xrgb* /*prgb*/,
    Pixel* /*pPixel*/,
    int /*channel*/,
    int /*client*/,
    ColorCompareProcPtr /*comp*/
#endif
);

extern int QueryColors(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    int /*count*/,
    Pixel* /*ppixIn*/,
    xrgb* /*prgbList*/
#endif
);

extern int FreeClientPixels(
#if NeedFunctionPrototypes
    pointer /*pcr*/,
    XID /*fakeid*/
#endif
);

extern int AllocColorCells(
#if NeedFunctionPrototypes
    int /*client*/,
    ColormapPtr /*pmap*/,
    int /*colors*/,
    int /*planes*/,
    Bool /*contig*/,
    Pixel* /*ppix*/,
    Pixel* /*masks*/
#endif
);

extern int AllocColorPlanes(
#if NeedFunctionPrototypes
    int /*client*/,
    ColormapPtr /*pmap*/,
    int /*colors*/,
    int /*r*/,
    int /*g*/,
    int /*b*/,
    Bool /*contig*/,
    Pixel* /*pixels*/,
    Pixel* /*prmask*/,
    Pixel* /*pgmask*/,
    Pixel* /*pbmask*/
#endif
);

extern int FreeColors(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    int /*client*/,
    int /*count*/,
    Pixel* /*pixels*/,
    Pixel /*mask*/
#endif
);

extern int StoreColors(
#if NeedFunctionPrototypes
    ColormapPtr /*pmap*/,
    int /*count*/,
    xColorItem* /*defs*/
#endif
);

extern int IsMapInstalled(
#if NeedFunctionPrototypes
    Colormap /*map*/,
    WindowPtr /*pWin*/
#endif
);

#endif /* CMAP_H */
