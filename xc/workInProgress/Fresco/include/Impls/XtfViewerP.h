/*
 * Copyright (c) 1993 2001 S.A.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * 2001 S.A. may not be used in any advertising or publicity relating to the
 * software without the specific, prior written permission of 2001 S.A.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL 2001 S.A. BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef Xtf_ViewerP_h
#define Xtf_ViewerP_h

#include <X11/Fresco/Impls/XtfViewer.h>

#include <X11/Fresco/_enter.h>

typedef struct { int dummy; } XtfViewerClassPart;

/* Full class record declaration. */
typedef struct _XtfViewerClassRec {
    CoreClassPart	core_class;
    XtfViewerClassPart	Xtfviewer_class;
} XtfViewerClassRec;

extern XtfViewerClassRec XtfviewerClassRec;

struct Fresco;
struct Glyph;
struct Viewer;
struct XtfWindowImpl;

typedef struct _XtfViewerPart {
    struct Fresco* fresco;
    struct Viewer* viewer;
    struct Glyph* glyph;
    struct XtfWindowImpl* window;
} XtfViewerPart;

typedef struct _XtfViewerRec {
    CorePart core;
    XtfViewerPart viewer;
} XtfViewerRec;

#endif
