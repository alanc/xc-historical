/*
 * $XConsortium: AsciiText.h,v 1.17 94/01/31 10:50:24 kaleb Exp $ 
 */

/***********************************************************

Copyright (c) 1987, 1988, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
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

/***********************************************************************
 *
 * AsciiText Widget
 *
 ***********************************************************************/

/*
 * AsciiText.c - Public header file for AsciiText Widget.
 *
 * This Widget is intended to be used as a simple front end to the 
 * text widget with an ascii source and ascii sink attached to it.
 *
 * Date:    June 29, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#ifndef _AsciiText_h
#define _AsciiText_h

/****************************************************************
 *
 * AsciiText widgets
 *
 ****************************************************************/

#include <X11/Xaw/Text.h>		/* AsciiText is a subclass of Text */
#include <X11/Xaw/AsciiSrc.h>
#include <X11/Xaw/MultiSrc.h>

/* Resources:

 Name		     Class		RepType		  Default Value
 ----		     -----		-------		  -------------
 autoFill	     AutoFill		Boolean		  False
 background	     Background		Pixel		  XtDefaultBackground
 border		     BorderColor	Pixel		  XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	  1
 bottomMargin	     Margin		Position	  2
 cursor		     Cursor		Cursor		  xterm
 destroyCallback     Callback		Pointer		  NULL
 displayCaret	     Output		Boolean		  True
 displayPosition     TextPosition	int		  0
 editType	     EditType		XawTextEditType	  XawtextRead
 font		     Font		XFontStruct*	  Fixed
 foreground	     Foreground		Pixel		  Black
 height		     Height		Dimension	  font height
 insertPosition	     TextPosition	int		  0
 international	     International		Boolean		false
 leftMargin	     Margin		Position	  2
 mappedWhenManaged   MappedWhenManaged	Boolean		  True
 resize		     Resize		XawTextResizeMode XawtextResizeNever
 rightMargin	     Margin		Position	  4
 scrollHorizontal    Scroll		XawTextScrollMode XawtextScrollNever
 scrollVertical	     Scroll		XawTextScrollMode XawtextScrollNever
 selectTypes	     SelectTypes	Pointer		  pos/word/line/par/all
 selection	     Selection		Pointer		  (empty selection)
 sensitive	     Sensitive		Boolean		  True
 sink		     TextSink		Widget		  (none)
 source		     TextSource		Widget		  (none)
 string		     String		String		  NULL
 topMargin	     Margin		Position	  2
 width		     Width		Dimension	  100
 wrap		     Wrap		XawTextWrapMode	  XawtextWrapNever
 x		     Position		Position	  0
 y		     Position		Position	  0

 (see also *Src.h and *Sink.h)
*/

/*
 * Everything else we need is in StringDefs.h or Text.h
 */

typedef struct _AsciiTextClassRec	*AsciiTextWidgetClass;
typedef struct _AsciiRec	        *AsciiWidget;

extern WidgetClass asciiTextWidgetClass;

/************************************************************
 *
 * Disk and String Emulation Info.
 * 
 ************************************************************/

#ifdef ASCII_STRING
extern WidgetClass asciiStringWidgetClass;
#endif

#ifdef ASCII_DISK
extern WidgetClass asciiDiskWidgetClass;
#endif

#endif /* _AsciiText_h */
