/*
 * $XConsortium: Clock.h,v 1.5 90/04/30 13:52:13 keith Exp $
 */

#ifndef _XtClock_h
#define _XtClock_h

/***********************************************************************
 *
 * Clock Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 minute		     Foreground		Pixel		Black
 hour		     Foreground		Pixel		Black
 height		     Height		int		120
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 width		     Width		int		120
 x		     Position		int		0
 y		     Position		int		0

*/

#define XtNminute	"minute"
#define XtNhour		"hour"
#define XtNjewel	"jewel"

#ifdef SHAPE
#define XtNshapeWindow	"shapeWindow"
#define XtCShapeWindow	"ShapeWindow"
#define XtNtransparent	"transparent"
#define XtCTransparent	"Transparent"
#endif

#define XtNjewelSize	"jewelSize"
#define XtNborderSize	"borderSize"
#define XtCBorderSize	"BorderSize"

typedef struct _ClockRec *ClockWidget;  /* completely defined in ClockPrivate.h */
typedef struct _ClockClassRec *ClockWidgetClass;    /* completely defined in ClockPrivate.h */

extern WidgetClass clockWidgetClass;

#endif /* _XtClock_h */
/* DON'T ADD STUFF AFTER THIS #endif */
