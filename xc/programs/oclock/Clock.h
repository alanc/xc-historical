/*
 * $XConsortium: Exp $
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
 face		     Foreground		Pixel		Black
 minute		     Foreground		Pixel		Black
 hour		     Foreground		Pixel		Black
 height		     Height		int		120
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 reverseVideo	     ReverseVideo	Boolean		False
 width		     Width		int		120
 x		     Position		int		0
 y		     Position		int		0

*/

#define XtNface		"face"
#define XtNminute	"minute"
#define XtNhour		"hour"
#define XtNjewel	"jewel"
#define XtNshapeWindow	"shapeWindow"

#define XtCShapeWindow	"ShapeWindow"

typedef struct _ClockRec *ClockWidget;  /* completely defined in ClockPrivate.h */
typedef struct _ClockClassRec *ClockWidgetClass;    /* completely defined in ClockPrivate.h */

extern WidgetClass clockWidgetClass;

#endif _XtClock_h
/* DON'T ADD STUFF AFTER THIS #endif */
