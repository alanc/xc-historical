/*
 * $XConsortium: Panner.c,v 1.5 90/02/12 15:30:03 jim Exp $
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
#include <X11/Xmu/CharSet.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/SimpleP.h>
/* #include <X11/Xaw/PannerP.h> */
#include "PannerP.h"


static char defaultTranslations[] = 
  "<Btn1Down>:    start() \n\
   <Btn1Motion>:  move() \n\
   <Btn1Up>:      notify() stop() \n\
   <Btn2Down>:    abort() \n\
   <Key>KP_Enter: set(rubberband,toggle) ";

static void ActionStart(), ActionStop(), ActionAbort(), ActionMove();
static void ActionNotify(), ActionSet();

static XtActionsRec actions[] = {
    { "start", ActionStart },		/* start tmp graphics */
    { "stop", ActionStop },		/* stop tmp graphics */
    { "abort", ActionAbort },		/* punt */
    { "move", ActionMove },		/* move tmp graphics on Motion event */
    { "notify", ActionNotify },		/* callback new position */
    { "set", ActionSet },		/* set various parameters */
};


/*
 * resources for the panner
 */
#define poff(field) XtOffset(PannerWidget, panner.field)
static XtResource resources[] = {
    { XtNallowOff, XtCAllowOff, XtRBoolean, sizeof(Boolean),
	poff(allow_off), XtRImmediate, FALSE },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	poff(callbacks), XtRCallback, (XtPointer) NULL },
    { XtNdefaultScale, XtCDefaultScale, XtRDimension, sizeof(Dimension),
	poff(default_scale), XtRImmediate, (XtPointer) PANNER_DEFAULT_SCALE },
    { XtNrubberBand, XtCRubberBand, XtRBoolean, sizeof(Boolean),
	poff(rubber_band), XtRImmediate, (XtPointer) TRUE },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel), 
	poff(foreground), XtRImmediate, (XtPointer) "XtDefaultForeground" },
    { XtNlineWidth, XtCLineWidth, XtRDimension, sizeof(Dimension),
	poff(line_width), XtRImmediate, (XtPointer) 0 },
    { XtNcanvasWidth, XtCCanvasWidth, XtRDimension, sizeof(Dimension),
	poff(canvas_width), XtRImmediate, (XtPointer) 0 },
    { XtNcanvasHeight, XtCCanvasHeight, XtRDimension, sizeof(Dimension),
	poff(canvas_height), XtRImmediate, (XtPointer) 0 },
    { XtNsliderColor, XtCBackground, XtRPixel, sizeof(Pixel),
	poff(slider_color), XtRString, (XtPointer) "XtDefaultBackground" },
    { XtNsliderX, XtCSliderX, XtRPosition, sizeof(Position),
	poff(slider_x), XtRImmediate, (XtPointer) 0 },
    { XtNsliderY, XtCSliderY, XtRPosition, sizeof(Position),
	poff(slider_y), XtRImmediate, (XtPointer) 0 },
    { XtNsliderWidth, XtCSliderWidth, XtRDimension, sizeof(Dimension),
	poff(slider_width), XtRImmediate, (XtPointer) 0 },
    { XtNsliderHeight, XtCSliderHeight, XtRDimension, sizeof(Dimension),
	poff(slider_height), XtRImmediate, (XtPointer) 0 },
    { XtNshadow, XtCShadow, XtRBoolean, sizeof(Boolean),
	poff(shadow), XtRImmediate, (XtPointer) TRUE },
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

static void reset_shadow_gc (pw)	/* used when resources change */
    PannerWidget pw;
{
    XtGCMask valuemask = GCForeground;
    XGCValues values;

    if (pw->panner.shadow_gc) XtReleaseGC ((Widget) pw, pw->panner.shadow_gc);

    values.foreground = pw->panner.foreground;
    if (pw->panner.line_width > 0) {
	values.line_width = pw->panner.line_width;
	valuemask |= GCLineWidth;
    }

    pw->panner.shadow_gc = XtGetGC ((Widget) pw, valuemask, &values);
}

static void reset_slider_gc (pw)	/* used when resources change */
    PannerWidget pw;
{
    XtGCMask valuemask = GCForeground;
    XGCValues values;

    if (pw->panner.slider_gc) XtReleaseGC ((Widget) pw, pw->panner.slider_gc);

    values.foreground = pw->panner.slider_color;

    pw->panner.slider_gc = XtGetGC ((Widget) pw, valuemask, &values);
}

static void reset_xor_gc (pw)		/* used when resources change */
    PannerWidget pw;
{
    if (pw->panner.xor_gc) XtReleaseGC ((Widget) pw, pw->panner.xor_gc);

    if (pw->panner.rubber_band) {
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
    } else {
	pw->panner.xor_gc = NULL;
    }
}

static void check_knob (pw, knob)
    register PannerWidget pw;
    Boolean knob;
{
    Position maxx = (Position) (((long) pw->core.width) -
				((long) pw->panner.knob_width));
    Position maxy = (Position) (((long) pw->core.height) -
				((long) pw->panner.knob_height));
    Position *x = (knob ? &pw->panner.knob_x : &pw->panner.tmp.x);
    Position *y = (knob ? &pw->panner.knob_y : &pw->panner.tmp.y);

    if (*x < 0) *x = 0;
    if (*x > maxx) *x = maxx;

    if (*y < 0) *y = 0;
    if (*y > maxy) *y = maxy;

    if (knob) {
	pw->panner.slider_x = (Position) (((float) pw->panner.knob_x) /
					  pw->panner.haspect + 0.5);
	pw->panner.slider_y = (Position) (((float) pw->panner.knob_y) /
					  pw->panner.vaspect + 0.5);
	pw->panner.last_x = pw->panner.last_y = PANNER_OUTOFRANGE;
    }
}


static void move_shadow (pw)
    PannerWidget pw;
{
    XRectangle *r = pw->panner.shadow_rects;
    int lw = (pw->panner.line_width == 0 ? 1 : pw->panner.line_width) * 4;

    if (pw->panner.knob_height > lw && pw->panner.knob_width > lw) {
#define SHADOW_WIDTH 2
	r->x = (short) (pw->panner.knob_x + pw->panner.knob_width);
	r->y = (short) (pw->panner.knob_y + lw);
	r->width = SHADOW_WIDTH;
	r->height = (unsigned short) (pw->panner.knob_height - lw);
	r++;
	r->x = (short) (pw->panner.knob_x + lw);
	r->y = (short) (pw->panner.knob_y + pw->panner.knob_height);
	r->width = (unsigned short) (pw->panner.knob_width - lw +
				     SHADOW_WIDTH);
	r->height = SHADOW_WIDTH;
	pw->panner.shadow_valid = TRUE;
#undef SHADOW_WIDTH
    } else {
	pw->panner.shadow_valid = FALSE;
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
    if (!pw->panner.allow_off) check_knob (pw, TRUE);
    if (pw->panner.shadow) move_shadow (pw);
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

    new->panner.shadow_gc = NULL;
    reset_shadow_gc (new);		/* foreground */
    new->panner.slider_gc = NULL;
    reset_slider_gc (new);		/* sliderColor */
    new->panner.xor_gc = NULL;
    reset_xor_gc (new);			/* foreground ^ background */

    rescale (new);			/* does a position check */
    new->panner.tmp.doing = FALSE;
    new->panner.tmp.showing = FALSE;
}

static void Destroy (gw)
    Widget gw;
{
    PannerWidget pw = (PannerWidget) gw;

    XtReleaseGC (gw, pw->panner.shadow_gc);
    XtReleaseGC (gw, pw->panner.slider_gc);
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

    pw->panner.tmp.showing = FALSE;
    XClearArea (XtDisplay(pw), XtWindow(pw), 
		pw->panner.last_x - pw->panner.line_width, 
		pw->panner.last_y - pw->panner.line_width, 
		pw->panner.knob_width + 2 + pw->panner.line_width * 2, 
		pw->panner.knob_height + 2 + pw->panner.line_width * 2, 
		False);
    pw->panner.last_x = pw->panner.knob_x;
    pw->panner.last_y = pw->panner.knob_y;

    XFillRectangle (dpy, w, pw->panner.slider_gc,
		    pw->panner.knob_x, pw->panner.knob_y,
		    pw->panner.knob_width - 1, pw->panner.knob_height - 1);

    XDrawRectangle (dpy, w, pw->panner.shadow_gc,
		    pw->panner.knob_x, pw->panner.knob_y,
		    pw->panner.knob_width - 1, pw->panner.knob_height - 1);

    if (pw->panner.shadow && pw->panner.shadow_valid) {
	XFillRectangles (dpy, w, pw->panner.shadow_gc,
			 pw->panner.shadow_rects, 2);
    }
    if (pw->panner.tmp.doing && pw->panner.rubber_band) DRAW_TMP (pw);
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
	reset_shadow_gc (new);
	reset_xor_gc (new);
	redisplay = TRUE;
    } else if (cur->panner.line_width != new->panner.line_width ||
	       cur->core.background_pixel != new->core.background_pixel) {
	reset_xor_gc (new);
	redisplay = TRUE;
    }
    if (cur->panner.slider_color != new->panner.slider_color) {
	reset_slider_gc (new);
	redisplay = TRUE;
    }
    if (cur->panner.shadow != new->panner.shadow && new->panner.shadow) {
	move_shadow (new);
	redisplay = TRUE;
    }
    if (cur->panner.rubber_band != new->panner.rubber_band) {
	reset_xor_gc (new);
	if (new->panner.tmp.doing) redisplay = TRUE;
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
	if (loc || siz ||
	    (cur->panner.allow_off != new->panner.allow_off &&
	     new->panner.allow_off)) {
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

static void ActionStart (gw, event, params, num_params)
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
    if (pw->panner.rubber_band) DRAW_TMP (pw);
}

static void ActionStop (gw, event, params, num_params)
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
	if (!pw->panner.allow_off) check_knob (pw, FALSE);
    }
    if (pw->panner.rubber_band) UNDRAW_TMP (pw);
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

    if (pw->panner.rubber_band) UNDRAW_TMP (pw);

    if (!pw->panner.rubber_band) {		/* restore old position */
	pw->panner.tmp.x = pw->panner.tmp.startx;
	pw->panner.tmp.y = pw->panner.tmp.starty;
	ActionNotify (gw, event, params, num_params);
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

    if (pw->panner.rubber_band) UNDRAW_TMP (pw);
    pw->panner.tmp.x = ((Position) x) - pw->panner.tmp.dx;
    pw->panner.tmp.y = ((Position) y) - pw->panner.tmp.dy;

    if (!pw->panner.rubber_band) {
	ActionNotify (gw, event, params, num_params);  /* does a check */
    } else {
	if (!pw->panner.allow_off) check_knob (pw, FALSE);
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

    if (!pw->panner.allow_off) check_knob (pw, FALSE);
    pw->panner.knob_x = pw->panner.tmp.x;
    pw->panner.knob_y = pw->panner.tmp.y;
    if (pw->panner.shadow) move_shadow (pw);

    pw->panner.slider_x = (Position) (((float) pw->panner.knob_x) /
				      pw->panner.haspect + 0.5);
    pw->panner.slider_y = (Position) (((float) pw->panner.knob_y) /
				      pw->panner.vaspect + 0.5);

    if (pw->panner.last_x != pw->panner.knob_x ||
	pw->panner.last_y != pw->panner.knob_y) {
	XawPannerReport rep;

	Redisplay (gw, (XEvent*) NULL, (Region) NULL);
	rep.x = pw->panner.slider_x;
	rep.y = pw->panner.slider_y;
	XtCallCallbackList (gw, pw->panner.callbacks, (caddr_t) &rep);
    }
}

static void ActionSet (gw, event, params, num_params)
    Widget gw;
    XEvent *event;			/* unused */
    String *params;			/* unused */
    Cardinal *num_params;		/* unused */
{
    PannerWidget pw = (PannerWidget) gw;
    Boolean rb;

    if (*num_params < 2 ||
	XmuCompareISOLatin1 (params[0], "rubberband") != 0) {
	XBell (XtDisplay(gw), 0);
	return;
    }

    if (XmuCompareISOLatin1 (params[1], "on") == 0) {
	rb = TRUE;
    } else if (XmuCompareISOLatin1 (params[1], "off") == 0) {
	rb = FALSE;
    } else if (XmuCompareISOLatin1 (params[1], "toggle") == 0) {
	rb = !pw->panner.rubber_band;
    } else {
	XBell (XtDisplay(gw), 0);
	return;
    }

    if (rb != pw->panner.rubber_band) {
	Arg args[1];
	XtSetArg (args[0], XtNrubberBand, rb);
	XtSetValues (gw, args, (Cardinal) 1);
    }
}
