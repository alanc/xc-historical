/*
 * $XConsortium: Xmu.h,v 1.1 88/09/05 11:26:20 jim Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_H_
#define _XMU_H_
/*
 * public entry points
 */

void XmuCopyISOLatin1Lowered();
void XmuCvtFunctionToCallback();
void XmuCvtStringToBackingStore();
void XmuCvtStringToJustify();
void XmuCvtStringToOrientation();
int XmuReadBitmapDataFromFile();

typedef enum {
    XtJustifyLeft,       /* justify text to left side of button   */
    XtJustifyCenter,     /* justify text in center of button      */
    XtJustifyRight       /* justify text to right side of button  */
} XtJustify;

typedef enum {XtorientHorizontal, XtorientVertical} XtOrientation;

#define XtNbackingStore		"backingStore"

#define XtCBackingStore		"BackingStore"

#define XtRBackingStore		"BackingStore"

/* BackingStore constants */
#define XtEnotUseful		"notUseful"
#define XtEwhenMapped		"whenMapped"
#define XtEalways		"always"
#define XtEdefault		"default"

/* Justify constants */
#define XtEleft			"left"
#define XtEcenter		"center"
#define XtEright		"right"

#endif /* _XMU_H_ */

