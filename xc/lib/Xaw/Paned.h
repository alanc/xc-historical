/*
* $XConsortium: Paned.h,v 1.20 89/02/10 18:57:36 kit Exp $
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

#ifndef _XtPaned_h
#define _XtPaned_h

#include <X11/Constraint.h>
#include <X11/Xmu.h>

/****************************************************************
 *
 * Vertical Paned Widget (SubClass of CompositeClass)
 *
 ****************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 allowResize	     Boolean		Boolean		False
 background	     Background		Pixel		XtDefaultBackground
 betweenCursor	     Cursor		Cursor		sb_v_double_arrow
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 gripIndent	     GripIndent		Position	16
 internalBorderWidth BorderColor        Pixel           DefaultForeground
 internalBorderColor BorderWidth        Dimension       1
 lowerCursor	     Cursor		Cursor		sb_down_arrow
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 max		     Max		Dimension	unlimited
 min		     Min		Dimension	1
 orientation         Orientation        XtOrientation   XtorientVertical
 refigureMode	     Boolean		Boolean		On
 sensitive	     Sensitive		Boolean		True
 skipAdjust	     Boolean		Boolean		False
 upperCursor	     Cursor		Cursor		sb_up_arrow
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define PANED_ASK_CHILD 0
#define PANED_GRIP_SIZE 0

/* New Fields */
#define XtNallowResize		    "allowResize"
#define XtNbetweenCursor	    "betweenCursor"
#define XtNverticalBetweenCursor    "verticalBetweenCursor"
#define XtNhorizontalBetweenCursor  "horizontalBetweenCursor"
#define XtNgripCursor		    "gripCursor"
#define XtNgripIndent		    "gripIndent"
#define XtNhorizontalGripCursor	    "horizontalGripCursor"
#define XtNinternalBorderColor      "internalBorderColor"
#define XtNinternalBorderWidth      "internalBorderWidth"
#define XtNleftCursor		    "leftCursor"
#define XtNlowerCursor		    "lowerCursor"
#define XtNrefigureMode		    "refigureMode"
#define XtNposition		    "position"
#define XtNmin			    "min"
#define XtNmax			    "max"
#define XtNpreferredPaneSize        "preferredPaneSize"
#define XtNresizeToPreferred        "resizeToPreferred"
#define XtNrightCursor		    "rightCursor"
#define XtNshowGrip                 "showGrip"
#define XtNskipAdjust		    "skipAdjust"
#define XtNupperCursor		    "upperCursor"
#define XtNverticalGripCursor	    "verticalGripCursor"

#define XtCGripIndent		    "GripIndent"
#define XtCMin			    "Min"
#define XtCMax			    "Max"
#define XtCPreferredPaneSize        "PreferredPaneSize"
#define XtCShowGrip                 "ShowGrip"

/* Class record constant */
extern WidgetClass panedWidgetClass;
extern WidgetClass vPanedWidgetClass; /* for compatability. */

typedef struct _PanedClassRec	*PanedWidgetClass;
typedef struct _PanedRec	*PanedWidget;

/* Public Procedures */

extern void XtPanedSetMinMax( /* panedWidget, min, max */ );
    /* Widget panedWidget;	*/
    /* int    min, max;		*/

extern void XtPanedRefigureMode( /* widget, mode */ );
    /* Widget widget;		*/
    /* Boolean  mode;		*/

extern void XtPanedGetMinMax( /* panedWidget, min, max */ );
    /* Widget panedWidget;	*/
    /* int    *min, *max;	*/ /* RETURN */

extern int XtPanedGetNumSub( /* w */ );
    /* Widget w;		*/

#endif _XtPaned_h
/* DON'T ADD STUFF AFTER THIS #endif */
