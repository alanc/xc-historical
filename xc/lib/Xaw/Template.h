#include <X11/copyright.h>

/* $XConsortium: Simple.h,v 1.5 88/09/06 16:42:22 jim Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987, 1988 */

#ifndef _Template_h
#define _Template_h

/****************************************************************
 *
 * Template widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define XtNtemplate		"template"

#define XtCTemplate		"Template"

typedef struct _TemplateClassRec	*TemplateWidgetClass;
typedef struct _TemplateRec		*TemplateWidget;

extern WidgetClass templateWidgetClass;

#endif  _Template_h
