/*
* $XConsortium: Command.h,v 1.15 88/09/06 16:41:09 jim Exp $
*/


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#ifndef _XtCommand_h
#define _XtCommand_h

/***********************************************************************
 *
 * Command Widget
 *
 ***********************************************************************/

#include <X11/Label.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		White
 bitmap		     Pixmap		Pixmap		None
 border		     BorderColor	Pixel		Black
 borderWidth	     BorderWidth	Dimension	1
 callback	     Callback		Pointer		NULL
 cursor		     Cursor		Cursor		None
 destroyCallback     Callback		Pointer		NULL
 font		     Font		XFontInfo*	fixed
 foreground	     Foreground		Pixel		Black
 height		     Height		Dimension	text height
 highlightThickness  Thickness		Dimension	2
 insensitiveBorder   Insensitive	Pixmap		Gray
 internalHeight	     Height		Dimension	2
 internalWidth	     Width		Dimension	4
 justify	     Justify		XtJustify	Center
 label		     Label		String		NULL
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	text width
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define XtNcallback		"callback"
#define XtNhighlightThickness   "highlightThickness"
#define XtNtranslations		"translations"

extern WidgetClass     commandWidgetClass;

typedef struct _CommandClassRec   *CommandWidgetClass;
typedef struct _CommandRec        *CommandWidget;


#endif _XtCommand_h
/* DON'T ADD STUFF AFTER THIS */
