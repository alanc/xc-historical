/***********************************************************
Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

 "$XConsortium: SmeLine.h,v 1.1 89/09/29 19:04:03 kit Exp $";

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

#ifndef _SmeLine_h
#define _SmeLine_h

#include <X11/Xaw/Sme.h>
#include <X11/Xmu/Converters.h>

/****************************************************************
 *
 * SmeLine Object
 *
 ****************************************************************/

/*
 * SmeLine.h - Public Header file for SmeLine object.
 *
 * This is the public header file for the Athena SmeLine object.
 * It is intended to be used with the simple menu widget.  
 *
 * Date:    April 3, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

/* Menu Entry Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 callback            Callback		Pointer		NULL
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0n
 y		     Position		Position	0

*/

#define XtCLineWidth "LineWidth"
#define XtCStipple "Stipple"

#define XtNlineWidth "lineWidth"
#define XtNstipple "stipple"

typedef struct _SmeLineClassRec*	SmeLineObjectClass;
typedef struct _SmeLineRec*	        SmeLineObject;

extern WidgetClass smeLineObjectClass;

#endif /* _SmeLine_h */
