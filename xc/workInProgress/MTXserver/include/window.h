/* $XConsortium: window.h,v 1.6 93/09/29 17:12:04 dpw Exp $ */
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

#ifndef WINDOW_H
#define WINDOW_H

#include "misc.h"
#include "region.h"
#include "screenint.h"

#define TOTALLY_OBSCURED 0
#define UNOBSCURED 1
#define OBSCURED 2

#define VisibilityNotViewable	3

/* return values for tree-walking callback procedures */
#define WT_STOPWALKING		0
#define WT_WALKCHILDREN		1
#define WT_DONTWALKCHILDREN	2
#define WT_NOMATCH 3
#define NullWindow ((WindowPtr) 0)

typedef struct _BackingStore *BackingStorePtr;
typedef struct _Window *WindowPtr;

typedef int (*VisitWindowProcPtr)(
#if NeedNestedPrototypes
    WindowPtr /*pWin*/,
    pointer /*data*/
#endif
);

extern int TraverseTree(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    VisitWindowProcPtr /*func*/,
    pointer /*data*/
#endif
);

extern int WalkTree(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    VisitWindowProcPtr /*func*/,
    pointer /*data*/
#endif
);

extern WindowPtr AllocateWindow(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/
#endif
);

extern Bool CreateRootWindow(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/
#endif
);

extern void InitRootWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void ClippedRegionFromBox(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    RegionPtr /*Rgn*/,
    int /*x*/,
    int /*y*/,
    int /*w*/,
    int /*h*/
#endif
);

extern WindowPtr RealChildHead(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern WindowPtr CreateWindow(
#if NeedFunctionPrototypes
    Window /*wid*/,
    WindowPtr /*pParent*/,
    int /*x*/,
    int /*y*/,
    unsigned int /*w*/,
    unsigned int /*h*/,
    unsigned int /*bw*/,
    unsigned int /*class*/,
    Mask /*vmask*/,
    XID* /*vlist*/,
    int /*depth*/,
    ClientPtr /*client*/,
    VisualID /*visual*/,
    int* /*error*/
#endif
);

extern int DeleteWindow(
#if NeedFunctionPrototypes
    pointer /*pWin*/,
    XID /*wid*/
#endif
);

extern void DestroySubwindows(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    ClientPtr /*client*/
#endif
);

extern int ChangeWindowAttributes(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    Mask /*vmask*/,
    XID* /*vlist*/,
    ClientPtr /*client*/
#endif
);

extern void GetWindowAttributes(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    ClientPtr /*client*/
#endif
);

extern RegionPtr CreateUnclippedWinSize(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void GravityTranslate(
#if NeedFunctionPrototypes
    int /*x*/,
    int /*y*/,
    int /*oldx*/,
    int /*oldy*/,
    int /*dw*/,
    int /*dh*/,
    unsigned /*gravity*/,
    int* /*destx*/,
    int* /*desty*/
#endif
);

extern int ConfigureWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    Mask /*mask*/,
    XID* /*vlist*/,
    ClientPtr /*client*/
#endif
);

extern int CirculateWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pParent*/,
    int /*direction*/,
    ClientPtr /*client*/
#endif
);

extern int ReparentWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    WindowPtr /*pParent*/,
    int /*x*/,
    int /*y*/,
    ClientPtr /*client*/
#endif
);

extern int MapWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    ClientPtr /*client*/
#endif
);

extern void MapSubwindows(
#if NeedFunctionPrototypes
    WindowPtr /*pParent*/,
    ClientPtr /*client*/
#endif
);

extern int UnmapWindow(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    Bool /*fromConfigure*/
#endif
);

extern void UnmapSubwindows(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void HandleSaveSet(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern Bool VisibleBoundingBoxFromPoint(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/,
    BoxPtr /*box*/
#endif
);

extern Bool PointInWindowIsVisible(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*x*/,
    int /*y*/
#endif
);

extern RegionPtr NotClippedByChildren(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void SendVisibilityNotify(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void SaveScreens(
#if NeedFunctionPrototypes
    int /*on*/,
    int /*mode*/
#endif
);

extern WindowPtr FindWindowWithOptional(
#if NeedFunctionPrototypes
    WindowPtr /*w*/
#endif
);

extern void CheckWindowOptionalNeed(
#if NeedFunctionPrototypes
    WindowPtr /*w*/
#endif
);

extern Bool MakeWindowOptional(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern void DisposeWindowOptional(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

extern WindowPtr MoveWindowInStack(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    WindowPtr /*pNextSib*/
#endif
);

void SetWinSize(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

void SetBorderSize(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

void ResizeChildrenWinSize(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    int /*dx*/,
    int /*dy*/,
    int /*dw*/,
    int /*dh*/
#endif
);

#endif /* WINDOW_H */
