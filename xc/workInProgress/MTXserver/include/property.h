/* $XConsortium: property.h,v 1.4 93/09/20 16:52:51 dpw Exp $ */
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
#ifndef PROPERTY_H
#define PROPERTY_H 
typedef struct _Property *PropertyPtr;

extern int ChangeWindowProperty(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    Atom /*property*/,
    Atom /*type*/,
    int /*format*/,
    int /*mode*/,
    unsigned long /*len*/,
    pointer /*value*/,
    Bool /*sendevent*/
#endif
);

extern int DeleteProperty(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/,
    Atom /*propName*/
#endif
);

extern void DeleteAllWindowProperties(
#if NeedFunctionPrototypes
    WindowPtr /*pWin*/
#endif
);

#endif  /* PROPERTY_H */
