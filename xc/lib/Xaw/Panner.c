/*
 * $XConsortium: Panner.c,v 1.1 90/02/08 17:45:52 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/SimpleP.h>
/* #include <X11/Xaw/PannerP.h> */
#include "PannerP.h"


static char defaultTranslations[] = 
  "<Btn1Down>:    set() \n\
   <Btn1Motion>:  move() \n\
   <Btn1Up>:      notify() unset() \n\
   <Btn2Down>:    abort()";

static void ActionSet(), ActionUnset(), ActionAbort(), ActionMove();
static void ActionNotify();

static XtActionsRec actions[] = {
    { "set", ActionSet },		/* start tmp graphics */
    { "unset", ActionUnset },		/* stop tmp graphics */
    { "abort", ActionAbort },		/* punt */
    { "move", ActionMove },		/* move tmp graphics on Motion event */
    { "notify", ActionNotify },		/* callback new position */
};


/*
 * resources for the panner
 */
#define poff(field) XtOffset(PannerWidget, panner.field)
static XtResource resources[] = {
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	poff(callbacks), XtRCallback, (XtPointer) NULL },
    { XtNdefaultScale, XtCDefaultScale, XtRDimension, sizeof(Dimension),
	poff(default_scale), XtRImmediate, (XtPointer) PANNER_DEFAULT_SCALE },
    { XtNdynamic, XtCDynamic, XtRBoolean, sizeof(Boolean),
	poff(dynamic), XtRImmediate, (XtPointer) FALSE },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	poff(foreground), XtRString, (XtPointer) "XtDefaultForeground" },
    { XtNlineWidth, XtCLineWidth, XtRDimension, sizeof(Dimension),
	poff(line_width), XtRImmediate, (XtPointer) 0 },
    { XtNcanvasWidth, XtCCanvasWidth, XtRDimension, sizeof(Dimension),
	poff(canvas_width), XtRImmediate, (XtPointer) 0 },
    { XtNcanvasHeight, XtCCanvasHeight, XtRDimension, sizeof(Dimension),
	poff(canvas_height), XtRImmediate, (XtPointer) 0 },
    { XtNsliderX, XtCSliderX, XtRPosition, sizeof(Position),
	poff(slider_x), XtRImmediate, (XtPointer) 0 },
    { XtNsliderY, XtCSliderY, XtRPosition, sizeof(Position),
	poff(slider_y), XtRImmediate, (XtPointer) 0 },
    { XtNsliderWidth, XtCSliderWidth, XtRDimension, sizeof(Dimension),
	poff(slider_width), XtRImmediate, (XtPointer) 0 },
    { XtNsliderHeight, XtCSliderHeight, XtRDimension, sizeof(Dimension),
	poff(slider_height), XtRImmediate, (XtPointer) 0 },
};
#undef poff


/*
 * widget class methods used below
 */
static void Initialize();		/* create gc's */
static void Destroy();			/* clean up widget */
static void Resize();			/* need to rescale ourselves */
static void Redisplay();		/* draw ourselves */
static Boolean SetValues();		/* set all of the resources */
static XtGeometryResult QueryGeometry();  /* say how big we would like to be */

PannerClassRec pannerClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	"Panner",
    /* widget_size		*/	sizeof(PannerRec),
    /* class_initialize		*/	XawInitializeWidgetSet,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	defaultTranslations,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	XtInheritChangeSensitive
  },
  { /* panner fields */
    /* ignore                   */	0
  }
};

WidgetClass pannerWidgetClass = (WidgetClass) &pannerClassRec;


/*****************************************************************************
 *                                                                           *
 *			    panner utility routines                          *
 *                                                                           *
 *****************************************************************************/

static void reset_draw_gc (pw)		/* used when resources change */
    PannerWidget pw;
{
    XtGCMask valuemask = (GCForeground | GCFunction);
    XGCValues values;

    if (pw->panner.draw_gc) XtReleaseGC ((Widget) pw, pw->panner.draw_gc);

    values.foreground = pw->panner.foreground;
    values.function = GXcopy;

    pw->panner.draw_gc = XtGetGC ((Widget) pw, valuemask, &values);
}

static void reset_xor_gc (pw)		/* used when resources change */
    PannerWidget pw;
{
    if (pw->panner.xor_gc) XtReleaseGC ((Widget) pw, pw->panner.xor_gc);

    if (pw->panner.dynamic) {
	pw->panner.xor_gc = NULL;
    } else {
	XtGCMask valuemask = (GCForeground | GCFunction);
	XGCValues values;

	values.foreground = (pw->panner.foreground ^
			     pw->core.background_pixel);
	values.function = GXxor;
	if (pw->panner.line_width > 0) {
	    valuemask |= GCLineWidth;
	    values.line_width = pw->panner.line_width;
	}
	pw->panner.xor_gc = XtGetGC ((Widget) pw, valuemask, &values);
    }
}

static void scale_knob (pw, location, size)  /* set knob size and/or loc */
    PannerWidget pw;
    Boolean location, size;
{
    if (location) {
	pw->panner.knob_x = (Position)
	  PANNER_HSCALE (pw, pw->panner.slider_x);
	pw->panner.knob_y = (Position)
	  PANNER_VSCALE (pw, pw->panner.slider_y);
    }
    if (size) {
	pw->panner.knob_width = (Dimension)
	  PANNER_HSCALE (pw, pw->panner.slider_width);
	pw->panner.knob_height = (Dimension)
	  PANNER_VSCALE (pw, pw->panner.slider_height);
    }
}

static void rescale (pw)
    PannerWidget pw;
{
    if (pw->panner.canvas_width < 1) pw->panner.canvas_width = 1;
    if (pw->panner.canvas_height < 1) pw->panner.canvas_height = 1;
    pw->panner.haspect = (((float) pw->core.width) /
			  ((float) pw->panner.canvas_width));
    pw->panner.vaspect = (((float) pw->core.height) /
			  ((float) pw->panner.canvas_height));
    scale_knob (pw, TRUE, TRUE);
}

static Boolean get_event_xy (event, x, y)
    XEvent *event;
    int *x, *y;
{
    switch (event->type) {
      case ButtonPress:
      case ButtonRelease:
	*x = event->xbutton.x;
	*y = event->xbutton.y;
	return TRUE;

      case KeyPress:
      case KeyRelease:
	*x = event->xkey.x;
	*y = event->xkey.y;
	return TRUE;

      case EnterNotify:
      case LeaveNotify:
	*x = event->xcrossing.x;
	*y = event->xcrossing.y;
	return TRUE;

      case MotionNotify:
	*x = event->xmotion.x;
	*y = event->xmotion.y;
	return TRUE;
    }

    return FALSE;
}

#define DRAW_TMP(pw) \
{ \
    XDrawRectangle (XtDisplay(pw), XtWindow(pw), \
		    pw->panner.xor_gc, \
		    pw->panner.tmp.x, pw->panner.tmp.y, \
		    pw->panner.knob_width - 1, pw->panner.knob_height - 1); \
    pw->panner.tmp.showing = !pw->panner.tmp.showing; \
}

#define UNDRAW_TMP(pw) \
{ \
    if (pw->panner.tmp.showing) DRAW_TMP(pw); \
}


/*****************************************************************************
 *                                                                           *
 * 			     panner class methods                            *
 *                                                                           *
 *****************************************************************************/

static void Initialize (greq, gnew)
    Widget greq, gnew;
{
    PannerWidget req = (PannerWidget) greq, new = (PannerWidget) gnew;

    if (req->panner.canvas_width < 1) new->panner.canvas_width = 1;
    if (req->panner.canvas_height < 1) new->panner.canvas_height = 1;
    if (req->panner.default_scale < 1)
      new->panner.default_scale = PANNER_DEFAULT_SCALE;

    if (req->core.width < 1)
      new->core.width = PANNER_DSCALE (req, req->panner.canvas_width);
    if (req->core.height < 1)
      new->core.height = PANNER_DSCALE (req, req->panner.canvas_height);

    new->panner.draw_gc = NULL;
    reset_draw_gc (new);
    new->panner.xor_gc = NULL;
    reset_xor_gc (new);
    rescale (new);
    new->panner.tmp.doing = FALSE;
    new->panner.tmp.showing = FALSE;
}

static void Destroy (gw)
    Widget gw;
{
    PannerWidget pw = (PannerWidget) gw;

    XtReleaseGC (gw, pw->panner.xor_gc);
}


static void Resize (gw)
    Widget gw;
{
    rescale ((PannerWidget) gw);
}


static void Redisplay (gw, event, region)
    Widget gw;
    XEvent *event;
    Region region;
{
    PannerWidget pw = (PannerWidget) gw;
    Display *dpy = XtDisplay(gw);
    Window w = XtWindow(gw);

    XClearArea (dpy, w, 0, 0, 0, 0, False);
    pw->panner.tmp.showing = FALSE;
    XFillRectangle (dpy, w, pw->panner.draw_gc,
		    pw->panner.knob_x, pw->panner.knob_y,
		    pw->panner.knob_width - 1, pw->panner.knob_height - 1);
    if (pw->panner.tmp.doing && !pw->panner.dynamic) DRAW_TMP (pw);
}


/* ARGSUSED */
static Boolean SetValues (gcur, greq, gnew, args, num_args)
    Widget gcur, greq, gnew;
    ArgList args;
    Cardinal *num_args;
{
    PannerWidget cur = (PannerWidget) gcur;
    PannerWidget req = (PannerWidget) greq;
    PannerWidget new = (PannerWidget) gnew;
    Boolean redisplay = FALSE;

    if (cur->panner.foreground != new->panner.foreground) {
	reset_draw_gc (new);
	reset_xor_gc (new);
	redisplay = TRUE;
    } else if (cur->panner.line_width != new->panner.line_width ||
	       cur->core.background_pixel != new->core.background_pixel) {
	reset_xor_gc (new);
	redisplay = TRUE;
    }

    if (cur->panner.canvas_width != new->panner.canvas_width ||
	cur->panner.canvas_height != new->panner.canvas_height) {
	rescale (new);			/* does a scale_knob as well */
	redisplay = TRUE;
    } else {
	Boolean loc = (cur->panner.slider_x != new->panner.slider_x ||
		       cur->panner.slider_y != new->panner.slider_y);
	Boolean siz = (cur->panner.slider_width != new->panner.slider_width ||
		       cur->panner.slider_height != new->panner.slider_height);
	if (loc || siz) {
	    scale_knob (new, loc, siz);
	    redisplay = TRUE;
	}
    }

    return redisplay;
}

static XtGeometryResult QueryGeometry (gw, intended, pref)
    Widget gw;
    XtWidgetGeometry *intended, *pref;
{
    PannerWidget pw = (PannerWidget) gw;

    pref->request_mode = (CWWidth | CWHeight);
    pref->width = PANNER_DSCALE (pw, pw->panner.canvas_width);
    pref->height = PANNER_DSCALE (pw, pw->panner.canvas_height);

    if (((intended->request_mode & (CWWidth | CWHeight)) ==
	 (CWWidth | CWHeight)) &&
	intended->width == pref->width &&
	intended->height == pref->height)
      return XtGeometryYes;
    else if (pref->width == pw->core.width && pref->height == pw->core.height)
      return XtGeometryNo;
    else
      return XtGeometryAlmost;
}


/*****************************************************************************
 *                                                                           *
 * 			      panner action procs                            *
 *                                                                           *
 *****************************************************************************/

static void ActionSet (gw, event, params, num_params)
    Widget gw;
    XEvent *event;
    String *params;			/* unused */
    Cardinal *num_params;		/* unused */
{
    PannerWidget pw = (PannerWidget) gw;
    int x, y;

    if (!get_event_xy (event, &x, &y)) {
	XBell (XtDisplay(gw), 0);	/* should do error message */
	return;
    }

    pw->panner.tmp.doing = TRUE;
    pw->panner.tmp.startx = pw->panner.knob_x;
    pw->panner.tmp.starty = pw->panner.knob_y;
    pw->panner.tmp.dx = (((Position) x) - pw->panner.knob_x);
    pw->panner.tmp.dy = (((Position) y) - pw->panner.knob_y);
    pw->panner.tmp.x = pw->panner.knob_x;
    pw->panner.tmp.y = pw->panner.knob_y;
    if (!pw->panner.dynamic) DRAW_TMP (pw);
}

static void ActionUnset (gw, event, params, num_params)
    Widget gw;
    XEvent *event;
    String *params;			/* unused */
    Cardinal *num_params;		/* unused */
{
    PannerWidget pw = (PannerWidget) gw;
    int x, y;

    if (get_event_xy (event, &x, &y)) {
	pw->panner.tmp.x = ((Position) x) - pw->panner.tmp.dx;
	pw->panner.tmp.y = ((Position) y) - pw->panner.tmp.dy;
    }
    if (!pw->panner.dynamic) UNDRAW_TMP (pw);
    pw->panner.tmp.doing = FALSE;
}

static void ActionAbort (gw, event, params, num_params)
    Widget gw;
    XEvent *event;
    String *params;			/* unused */
    Cardinal *num_params;		/* unused */
{
    PannerWidget pw = (PannerWidget) gw;

    if (!pw->panner.tmp.doing) return;

    if (pw->panner.dynamic) {		/* restore old position */
	pw->panner.tmp.x = pw->panner.tmp.startx;
	pw->panner.tmp.y = pw->panner.tmp.starty;
	ActionNotify (gw, event, params, num_params);
    } else {
	UNDRAW_TMP (pw);
    }
    pw->panner.tmp.doing = FALSE;
}



static void ActionMove (gw, event, params, num_params)
    Widget gw;
    XEvent *event;			/* must be a motion event */
    String *params;			/* unused */
    Cardinal *num_params;		/* unused */
{
    PannerWidget pw = (PannerWidget) gw;
    int x, y;

    if (!pw->panner.tmp.doing) return;

    if (!get_event_xy (event, &x, &y)) {
	XBell (XtDisplay(gw), 0);	/* should do error message */
	return;
    }

    if (!pw->panner.dynamic) UNDRAW_TMP (pw);

    pw->panner.tmp.x = ((Position) x) - pw->panner.tmp.dx;
    pw->panner.tmp.y = ((Position) y) - pw->panner.tmp.dy;

    if (pw->panner.dynamic) {
	ActionNotify (gw, event, params, num_params);
    } else {
	DRAW_TMP (pw);
    }
}

static void ActionNotify (gw, event, params, num_params)
    Widget gw;
    XEvent *event;			/* unused */
    String *params;			/* unused */
    Cardinal *num_params;		/* unused */
{
    PannerWidget pw = (PannerWidget) gw;

    if (!pw->panner.tmp.doing) return;

    pw->panner.knob_x = pw->panner.tmp.x;
    pw->panner.knob_y = pw->panner.tmp.y;

    pw->panner.slider_x = (Position) (((float) pw->panner.knob_x) /
				      pw->panner.haspect);
    pw->panner.slider_y = (Position) (((float) pw->panner.knob_y) /
				      pw->panner.vaspect);
    Redisplay (gw);

    /* call callback */
}

