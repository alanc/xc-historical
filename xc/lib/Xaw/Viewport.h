/*
 * $Header: $
 * Public definitions for Viewport widget
 */

#ifndef _Viewport_h
#define _Viewport_h

#include <X/Form.h>

/* fields added to Form */
#define	XtNforceBars		"forceBars"
#define	XtNallowHoriz		"allowHoriz"
#define	XtNallowVert		"allowVert"
#define	XtNuseBottom		"useBottom"
#define	XtNuseRight		"useRight"

extern WidgetClass viewportWidgetClass;

typedef struct _ViewportClassRec *ViewportWidgetClass;
typedef struct _ViewportRec	 *ViewportWidget;

#endif _Viewport_h
