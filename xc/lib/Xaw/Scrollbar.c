#ifndef lint
static char rcsid[] = "$Header: Scroll.c,v 1.10 87/12/08 11:08:22 swick Locked $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* ScrollBar.c */
/* created by weissman, Mon Jul  7 13:20:03 1986 */
/* converted by swick, Thu Aug 27 1987 */

#include "Xlib.h"
#include "Xresource.h"
#include "Intrinsic.h"
#include "Scroll.h"
#include "ScrollP.h"
#include "Atoms.h"
#include "cursorfont.h"

/* Private definitions. */

static char *defaultTranslationTable[] = {
    "<Btn1Down>:   StartScroll(Forward)",
    "<Btn2Down>:   StartScroll(Continuous) DoThumb()",
    "<Btn3Down>:   StartScroll(Backward)",
    "<Btn2Motion>: DoThumb()",
    "Any<BtnUp>:   DoScroll(Proportional) EndScroll()", /* ||| 'Any' should default */
#ifdef bogusScrollKeys
    /* examples */
    "<KeyPress>f:  StartScroll(Forward) DoScroll(FullLength) EndScroll()",
    "<KeyPress>b:  StartScroll(Backward) DoScroll(FullLength) EndScroll()",
#endif
    NULL
};

/* grodyness needed because Xrm wants pointer to thing, not thing... */
static caddr_t defaultTranslations = (caddr_t)defaultTranslationTable;
static float floatZero = 0.0;

#define Offset(field) XtOffset(ScrollbarWidget, field)

static XtResource resources[] = {
  {XtNwidth, XtCWidth, XrmRInt, sizeof(int),
	     Offset(core.width), XtRString, "10"},
  {XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
	     Offset(scrollbar.orientation), XtRString, "vertical"},
  {XtNscrollProc, XtCCallback, XtRPointer, sizeof(caddr_t),
	     Offset(scrollbar.scrollProc), XtRPointer, NULL},
  {XtNthumbProc, XtCCallback, XtRPointer, sizeof(caddr_t),
	     Offset(scrollbar.thumbProc), XtRPointer, NULL},
  {XtNthumb, XtCThumb, XtRPixmap, sizeof(Pixmap),
	     Offset(scrollbar.thumb), XtRPixmap, NULL},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	     Offset(scrollbar.foreground), XtRString, "black"},
  {XtNshown, XtCShown, XtRFloat, sizeof(float),
	     Offset(scrollbar.shown), XtRFloat, (caddr_t)&floatZero},
  {XtNtop, XtCTop, XtRFloat, sizeof(float),
	     Offset(scrollbar.top), XtRFloat, (caddr_t)&floatZero},
  {XtNscrollVCursor, XtCScrollVCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.verCursor), XtRString, "sb_v_double_arrow"},
  {XtNscrollHCursor, XtCScrollHCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.horCursor), XtRString, "sb_h_double_arrow"},
  {XtNscrollUCursor, XtCScrollUCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.upCursor), XtRString, "sb_up_arrow"},
  {XtNscrollDCursor, XtCScrollDCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.downCursor), XtRString, "sb_down_arrow"},
  {XtNscrollLCursor, XtCScrollLCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.leftCursor), XtRString, "sb_left_arrow"},
  {XtNscrollRCursor, XtCScrollRCursor, XtRCursor, sizeof(Cursor),
	     Offset(scrollbar.rightCursor), XtRString, "sb_right_arrow"},
  {XtNtranslations, XtCTranslations, XtRTranslationTable,
	     sizeof(XtTranslations), Offset(core.translations),
	     XtRTranslationTable, (caddr_t)&defaultTranslations},
};

static void ClassInitialize();
static void Initialize();
static void Realize();
static void Resize();
static void Redisplay();
static Boolean SetValues();

static void StartScroll();
static void DoThumb();
static void DoScroll();
static void EndScroll();

static XtActionsRec actions[] = {
	{"StartScroll",		StartScroll},
	{"DoThumb",		DoThumb},
	{"DoScroll",		DoScroll},
	{"EndScroll",		EndScroll},
	{NULL,NULL}
};


static ScrollbarClassRec scrollbarClassRec = {
/* core fields */
    /* superclass       */      (WidgetClass) &widgetClassRec,
    /* class_name       */      "Scroll",
    /* size             */      sizeof(ScrollbarRec),
    /* class_initialize	*/	ClassInitialize,
    /* class_inited	*/	FALSE,
    /* initialize       */      Initialize,
    /* realize          */      Realize,
    /* actions          */      actions,
    /* num_actions	*/	XtNumber(actions),
    /* resources        */      resources,
    /* num_resources    */      XtNumber(resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion	*/	TRUE,
    /* compress_exposure*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      NULL,
    /* resize           */      Resize,
    /* expose           */      Redisplay,
    /* set_values       */      SetValues,
    /* accept_focus     */      NULL,
    /* callback_private */      NULL,
    /* reserved_private */      NULL,
};

WidgetClass scrollbarWidgetClass = (WidgetClass)&scrollbarClassRec;

#define MINBARHEIGHT	7     /* How many pixels of scrollbar to always show */
#define NoButton -1
#define PICKLENGTH(widget, x, y) \
    ((widget->scrollbar.orientation == XtorientHorizontal) ? x : y)
#define PICKTHICKNESS(widget, x, y) \
    ((widget->scrollbar.orientation == XtorientHorizontal) ? y : x)
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))


/* Orientation enumeration constants */

static	XrmQuark  XtQEhorizontal;
static	XrmQuark  XtQEvertical;

/*ARGSUSED*/
#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

extern void _XLowerCase();

static void CvtStringToOrientation(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XtOrientation orient;
    XrmQuark	q;
    char	lowerName[1000];

/* ||| where to put LowerCase */
    _XLowerCase((char *) fromVal->addr, lowerName);
    q = XrmAtomToQuark(lowerName);
    if (q == XtQEhorizontal) {
    	orient = XtorientHorizontal;
	done(&orient, XtOrientation);
	return;
    }
    if (q == XtQEvertical) {
    	orient = XtorientVertical;
	done(&orient, XtOrientation);
	return;
    }
};


static void ClassInitialize()
{
    XtQEhorizontal = XrmAtomToQuark(XtEhorizontal);
    XtQEvertical   = XrmAtomToQuark(XtEvertical);
    XrmRegisterTypeConverter(XrmRString, XtROrientation, CvtStringToOrientation);
}



/*
 * Make sure the first number is within the range specified by the other
 * two numbers.
 */

static int InRange(num, small, big)
int num, small, big;
{
    return (num < small) ? small : ((num > big) ? big : num);
}

/*
 * Same as above, but for floating numbers. 
 */

static float FloatInRange(num, small, big)
float num, small, big;
{
    return (num < small) ? small : ((num > big) ? big : num);
}


/* Fill the area specified by top and bottom with the given pattern. */
static float FractionLoc(w, x, y)
  ScrollbarWidget w;
  int x, y;
{
    float   result;

    result = PICKLENGTH(w, (float) x/w->core.width,
			(float) y/w->core.height);
    return FloatInRange(result, 0.0, 1.0);
}


static FillArea(w, top, bottom, thumb)
  ScrollbarWidget w;
  Position top, bottom;
  int thumb;
{
    Dimension length = bottom-top;

    switch(thumb) {
	/* Fill the new Thumb location */
      case 1:
	if (w->scrollbar.orientation == XtorientHorizontal) 
	    XFillRectangle(XtDisplay(w), XtWindow(w),
			   w->scrollbar.gc, top, 1, length,
			   w->core.height-2);
	
	else XFillRectangle(XtDisplay(w), XtWindow(w), w->scrollbar.gc,
			    1, top, w->core.width-2, length);

	break;
	/* Clear the old Thumb location */
      case 0:
	if (w->scrollbar.orientation == XtorientHorizontal) 
	    XClearArea(XtDisplay(w), XtWindow(w), top, 1,
		       length, w->core.height-2, FALSE);
	
	else XClearArea(XtDisplay(w), XtWindow(w), 1,
			top, w->core.width-2, length, FALSE);

    }  
}


/* Paint the thumb in the area specified by w->top and
   w->shown.  The old area is erased.  The painting and
   erasing is done cleverly so that no flickering will occur. */

static void PaintThumb( w )
  ScrollbarWidget w;
{
    int length, oldtop, oldbot, newtop, newbot;

    length = PICKLENGTH(w, w->core.width, w->core.height);
    oldtop = w->scrollbar.topLoc;
    oldbot = oldtop + w->scrollbar.shownLength;
    newtop = length * w->scrollbar.top;
    newbot = newtop + length * (w->scrollbar.shown);
    if (newbot < newtop + MINBARHEIGHT) newbot = newtop + MINBARHEIGHT;
    w->scrollbar.topLoc = newtop;
    w->scrollbar.shownLength = newbot - newtop;

    if (XtIsRealized(w)) {
	if (newtop < oldtop) FillArea(w, newtop, MIN(newbot, oldtop), 1);
	if (newtop > oldtop) FillArea(w, oldtop, MIN(newtop, oldbot), 0);
	if (newbot < oldbot) FillArea(w, MAX(newbot, oldtop), oldbot, 0);
	if (newbot > oldbot) FillArea(w, MAX(newtop, oldbot), newbot, 1);
    }
}


/* ARGSUSED */
static void Initialize( request, new, args, num_args )
   Widget request;		/* what the client asked for */
   Widget new;			/* what we're going to give him */
   ArgList args;
   Cardinal *num_args;
{
    ScrollbarWidget w = (ScrollbarWidget) new;
    XGCValues gcValues;

    if (w->scrollbar.thumb == NULL) {
        w->scrollbar.thumb = XtGrayPixmap( XtScreen(w) );
    }

    gcValues.foreground = w->scrollbar.foreground;
    gcValues.fill_style = FillTiled;
    gcValues.tile = w->scrollbar.thumb;
    w->scrollbar.gc = XtGetGC( new,
			       GCForeground | GCFillStyle | GCTile,
			       &gcValues);

    if (w->core.width == 0)  w->core.width = 1;
    if (w->core.height == 0) w->core.height = 1;

}

static void Realize( gw, valueMask, attributes )
   Widget gw;
   Mask *valueMask;
   XSetWindowAttributes *attributes;
{
    ScrollbarWidget w = (ScrollbarWidget) gw;

    w->scrollbar.inactiveCursor =
      (w->scrollbar.orientation == XtorientVertical)
	? w->scrollbar.verCursor
	: w->scrollbar.horCursor;

    attributes->cursor = w->scrollbar.inactiveCursor;
    *valueMask |= CWCursor;
    
    XtCreateWindow( gw, InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes );
}


/* ARGSUSED */
static Boolean SetValues( current, request, desired, last )
   Widget current,		/* what I am */
          request,		/* what he wants me to be */
          desired;		/* what I will become */
   Boolean last;		/* TRUE if not called by subclass */
{
    ScrollbarWidget w = (ScrollbarWidget) current;
    ScrollbarWidget rw = (ScrollbarWidget) request;
    ScrollbarWidget dw = (ScrollbarWidget) desired;
    Boolean redraw;

    redraw = FALSE;

    if (XtSetValuesGeometryRequest( current, desired, (XtWidgetGeometry*)NULL )
		== XtGeometryYes)
        /* no need to Resize() here, as ForgetGravity and Redisplay()
	 * will take care of what needs to be done. */
        redraw = TRUE;

    /* Core make take care of the following... we'll have to see */
    if (w->core.border_pixel != dw->core.border_pixel) {
	if (w->core.border_width != 0)
	    XSetWindowBorder(XtDisplay(w), XtWindow(w), w->core.border_pixel);
    }

    if (w->scrollbar.foreground != rw->scrollbar.foreground ||
	w->core.background_pixel != rw->core.background_pixel)
        redraw = TRUE;

    if (w->scrollbar.top != dw->scrollbar.top ||
        w->scrollbar.shown != dw->scrollbar.shown)
	redraw = TRUE;

    return( redraw );
}

/* ARGSUSED */
static void Resize( gw )
   Widget gw;
{
    /* ForgetGravity has taken care of background, but thumb may
     * have to move as a result of the new size. */
    Redisplay( gw, (XEvent*)NULL );
}


/* ARGSUSED */
static void Redisplay( gw, event )
   Widget gw;
   XEvent *event;		/* NULL if called from Resize() */
{
    ScrollbarWidget w = (ScrollbarWidget) gw;

    w->scrollbar.topLoc = -1000; /* Forces entire thumb to be painted. */
    PaintThumb( w ); 
}


static void StartScroll( gw, event, params, num_params )
  Widget gw;
  XEvent *event;
  String *params;		/* direction: Back|Forward|Smooth */
  Cardinal num_params;		/* we only support 1 */
{
    ScrollbarWidget w = (ScrollbarWidget) gw;
    Cursor cursor;
    char direction;

    if (w->scrollbar.direction != 0) return; /* if we're already scrolling */
    if (num_params > 0) direction = *params[0];
    else		direction = 'C';

    w->scrollbar.direction = direction;

    switch( direction ) {
	case 'B':
	case 'b':	cursor = (w->scrollbar.orientation == XtorientVertical)
				   ? w->scrollbar.upCursor
				   : w->scrollbar.leftCursor; break;

	case 'F':
	case 'f':	cursor = (w->scrollbar.orientation == XtorientVertical)
				   ? w->scrollbar.downCursor
				   : w->scrollbar.rightCursor; break;

	case 'C':
	case 'c':	cursor = (w->scrollbar.orientation == XtorientVertical)
				   ? w->scrollbar.rightCursor
				   : w->scrollbar.upCursor; break;

	default:	return; /* invalid invocation */
    }

    XDefineCursor(XtDisplay(w), XtWindow(w), cursor);

    XFlush(XtDisplay(w));
}


static void DoScroll( gw, event, params, num_params   )
   Widget gw;
   XEvent *event;
   String *params;		/* style: Proportional|FullLength */
   Cardinal num_params;		/* we only support 1 */
{
    ScrollbarWidget w = (ScrollbarWidget) gw;
    int call_data;
    char style;

    if (w->scrollbar.direction == 0) return; /* if no StartScroll */

    if (num_params > 0) style = *params[0];
    else		style = 'P';

    switch( style ) {
        case 'P':    /* Proportional */
        case 'p':    call_data = InRange( PICKLENGTH( w, event->xmotion.x,
						      event->xmotion.y),
					  0,
					  PICKLENGTH( w, w->core.width,
						      w->core.height)); break;

        case 'F':    /* FullLength */
        case 'f':    call_data = PICKLENGTH( w, w->core.width,
					     w->core.height); break;
    }

    switch( w->scrollbar.direction ) {
        case 'B':
        case 'b':    call_data = -call_data;
	  	     /* fall through */
        case 'F':
	case 'f':    XtCallCallbacks( gw, XtNscrollProc, (caddr_t)call_data );
	             break;

        case 'C':
	case 'c':    /* DoThumb has already called the thumbProc(s) */
		     break;
    }
}

/* ARGSUSED */
static void EndScroll(gw, event, params, num_params )
   Widget gw;
   XEvent *event;		/* unused */
   String *params;		/* unused */
   Cardinal num_params;		/* unused */
{
    ScrollbarWidget w = (ScrollbarWidget) gw;

    XDefineCursor(XtDisplay(w), XtWindow(w), w->scrollbar.inactiveCursor);
    XFlush(XtDisplay(w));

    w->scrollbar.direction = 0;
}


/* ARGSUSED */
static void DoThumb( gw, event, params, num_params )
   Widget gw;
   XEvent *event;
   String *params;		/* unused */
   Cardinal num_params;		/* unused */
{
    ScrollbarWidget w = (ScrollbarWidget) gw;

    if (w->scrollbar.direction == 0) return; /* if no StartScroll */

    w->scrollbar.top = FractionLoc(w, event->xmotion.x, event->xmotion.y);
    PaintThumb(w);
    XFlush(XtDisplay(w));

    XtCallCallbacks( gw, XtNthumbProc, w->scrollbar.top);
}



/* Public routines. */

/* Set the scroll bar to the given location. */
extern void XtScrollBarSetThumb( w, top, shown )
  ScrollbarWidget w;
  float top, shown;
{
    w->scrollbar.top = top;
    w->scrollbar.shown = shown;
    PaintThumb( w );
}
