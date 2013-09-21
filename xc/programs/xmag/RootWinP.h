/* $XConsortium: RootWinP.h,v 1.3 94/04/17 20:24:55 rws Exp $ */
/*

Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/


#ifndef _RootWindoP_h
#define _RootWindoP_h

#include "RootWin.h"
/* include superclass private header file */
#include <X11/CoreP.h>

typedef struct {
    int empty;
} RootWindowClassPart;

typedef struct _RootWindowClassRec {
    CoreClassPart	core_class;
    RootWindowClassPart	root_class;
} RootWindowClassRec;

extern RootWindowClassRec rootClassRec;

typedef struct {
    /* resources */
    char* resource;
    /* private state */
} RootWindowPart;

typedef struct _RootWindowRec {
    CorePart	core;
    RootWindowPart	root;
} RootWindowRec;

#endif /* _RootWindoP_h */
