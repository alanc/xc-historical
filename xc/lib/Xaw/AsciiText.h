#include <X/copyright.h>

/* $Header: AsciiText.h,v 1.1 87/12/23 16:39:25 swick Locked $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

#ifndef _AsciiText_h
#define _AsciiText_h

/****************************************************************
 *
 * AsciiText widgets
 *
 ****************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 destroyCallback     Callback		Pointer		NULL
 displayPosition     TextPosition	int		0
 file		     File		String		NULL
 height		     Height		int		font height
 insertPosition	     TextPosition	int		0
 leftMargin	     Margin		int		2
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 selectTypes	     SelectTypes	Pointer		(internal)
 selection	     Selection		Pointer		empty selection
 sensitive	     Sensitive		Boolean		True
 string		     String		String		NULL
 textOptions	     TextOptions	int		0
 width		     Width		int		100
 x		     Position		int		0
 y		     Position		int		0

*/


#include <X/Text.h>		/* AsciiText is a subclass of Text */

typedef struct _AsciiStringClassRec	*AsciiStringWidgetClass;
typedef struct _AsciiStringRec		*AsciiStringWidget;

extern WidgetClass asciiStringWidgetClass;

typedef struct _AsciiDiskClassRec	*AsciiDiskWidgetClass;
typedef struct _AsciiDiskRec		*AsciiDiskWidget;

extern WidgetClass asciiDiskWidgetClass;

#endif  _AsciiText_h
