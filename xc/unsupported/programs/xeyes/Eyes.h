/*
* $Header: Eyes.h,v 1.1 88/08/13 09:49:53 jim Exp $
*/

#ifndef _XtEyes_h
#define _XtEyes_h

/***********************************************************************
 *
 * Eyes Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		1
 foreground	     Foreground		Pixel		Black
 outline	     Outline		Pixel		Black
 height		     Height		int		120
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 reverseVideo	     ReverseVideo	Boolean		False
 width		     Width		int		120
 x		     Position		int		0
 y		     Position		int		0

*/

#define XtNoutline	"outline"
#define XtNcenterColor	"center"
#define XtNuseWideLines	"useWideLines"
#define XtNuseBevel	"useBevel"

#define XtCUseWideLines	"UseWideLines"
#define XtCUseBevel	"UseBevel"

typedef struct _EyesRec *EyesWidget;  /* completely defined in EyesPrivate.h */
typedef struct _EyesClassRec *EyesWidgetClass;    /* completely defined in EyesPrivate.h */

extern WidgetClass eyesWidgetClass;

#endif _XtEyes_h
/* DON'T ADD STUFF AFTER THIS #endif */
