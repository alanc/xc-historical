/*
 * Clock.c
 *
 * a NeWS clone clock
 */

# include <X11/Xos.h>
# include <stdio.h>
# include <X11/IntrinsicP.h>
# include <X11/StringDefs.h>
# include <X11/Xmu.h>
# include "ClockP.h"
# include <math.h>
# include <X11/extensions/shape.h>

#define offset(field) XtOffset(ClockWidget,clock.field)
#define goffset(field) XtOffset(Widget,core.field)

static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	goffset(width), XtRString, "120"},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	goffset(height), XtRString, "120"},
    {XtNface, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(face), XtRString, "Black"},
    {XtNminute, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(minute), XtRString, "Black"},
    {XtNhour, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(hour), XtRString, "Black"},
    {XtNjewel, XtCForeground, XtRPixel, sizeof (Pixel),
	offset(jewel), XtRString, "Black"},
    {XtNreverseVideo, XtCReverseVideo, XtRBoolean, sizeof (Boolean),
	offset (reverse_video), XtRString, "FALSE"},
    {XtNbackingStore, XtCBackingStore, XtRBackingStore, sizeof (int),
    	offset (backing_store), XtRString, "default"},
    {XtNshapeWindow, XtCShapeWindow, XtRBoolean, sizeof (Boolean),
	offset (shape_window), XtRString, "TRUE"},
};

#undef offset
#undef goffset

static void Initialize(), Realize(), Destroy(), Redisplay();
static Boolean SetValues();
static int repaint_window();
static int draw_it ();

# define BORDER_WIDTH	(0.1)
# define WINDOW_WIDTH	(2.0 - BORDER_WIDTH*2)
# define WINDOW_HEIGHT	(2.0 - BORDER_WIDTH*2)
# define MINUTE_WIDTH	(BORDER_WIDTH / 2.0)
# define HOUR_WIDTH	(BORDER_WIDTH / 2.0)
# define JEWEL_SIZE	(BORDER_WIDTH * 0.75)
# define MINUTE_LENGTH	(0.8)
# define HOUR_LENGTH	(0.5)
# define JEWEL_X	(0.0)
# define JEWEL_Y	(1.0 - (BORDER_WIDTH + JEWEL_SIZE))

static void ClassInitialize();

ClockClassRec clockClassRec = {
    { /* core fields */
    /* superclass		*/	&widgetClassRec,
    /* class_name		*/	"Clock",
    /* size			*/	sizeof(ClockRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULL,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	NULL,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	NULL,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	XtInheritQueryGeometry,
    }
};

static void ClassInitialize()
{
    XtAddConverter( XtRString, XtRBackingStore, XmuCvtStringToBackingStore,
		    NULL, 0 );
}

WidgetClass clockWidgetClass = (WidgetClass) &clockClassRec;

/* ARGSUSED */
static void Initialize (greq, gnew)
    Widget greq, gnew;
{
    ClockWidget w = (ClockWidget)gnew;
    XtGCMask	valuemask;
    XGCValues	myXGCV;

    /*
     * set the colors if reverse video; these are the colors used:
     *
     *     background - paper		white
     *     foreground - hands, face	black
     *     border - border		black (foreground)
     *
     * This doesn't completely work since the parent has already made up a 
     * border.  Sigh.
     */
    if (w->clock.reverse_video) {
	Pixel fg = w->clock.face;
	Pixel bg = w->core.background_pixel;

	if (w->core.border_pixel == fg)
 	    w->core.border_pixel = bg;
	if (w->clock.minute == fg)
	    w->clock.minute = bg;
	if (w->clock.hour == fg)
	    w->clock.hour = bg;
	if (w->clock.jewel == fg)
	    w->clock.jewel = bg;
	w->clock.face = bg;
	w->core.background_pixel = fg;
    }

    valuemask = GCForeground | GCBackground;
    myXGCV.background = w->core.background_pixel;

    myXGCV.foreground = w->clock.face;
    w->clock.faceGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->clock.minute;
    w->clock.minuteGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->clock.hour;
    w->clock.hourGC = XtGetGC(gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->clock.jewel;
    w->clock.jewelGC = XtGetGC (gnew, valuemask, &myXGCV);

    myXGCV.foreground = w->core.background_pixel;
    w->clock.eraseGC = XtGetGC(gnew, valuemask, &myXGCV);

    /* wait for Realize to add the timeout */
    w->clock.interval_id = 0;

    if (w->clock.shape_window && !XShapeQueryExtension (XtDisplay (w)))
	w->clock.shape_window = False;
    w->clock.shape_mask = 0;
    w->clock.shapeGC = 0;
    w->clock.shape_width = 0;
    w->clock.shape_height = 0;
    w->clock.polys_valid = 0;
}

static void Shape (w)
    ClockWidget	w;
{
    XGCValues	xgcv;
    int		mask, first;
    Widget	parent, child;
    XWindowChanges	xwc;
    int		face_width, face_height;
    int		x, y;
    int		border_width;
    int		face_x_extra, face_y_extra;

    if (w->core.width == w->clock.shape_width &&
        w->core.height == w->clock.shape_height)
	return;

    /*
     * reconfigure the widget to split the availible
     * space between the window and the border
     */

    SetTransform (&w->clock.t,
	0, w->core.width,
 	w->core.height, 0,
	-1.0, 1.0,
	-1.0, 1.0);

    face_width = abs (Xwidth (BORDER_WIDTH, BORDER_WIDTH, &w->clock.t));
    face_height = abs (Xheight (BORDER_WIDTH, BORDER_WIDTH, &w->clock.t));
    if (face_width > face_height)
	xwc.border_width = face_height;
    else
	xwc.border_width = face_width;
    xwc.width = w->core.width - xwc.border_width * 2;
    xwc.height = w->core.height - xwc.border_width * 2;
    XConfigureWindow (XtDisplay (w), XtWindow (w),
			CWWidth|CWHeight|CWBorderWidth,
			&xwc);

    /*
     *  shape the windows and borders
     */

    if (w->clock.shape_window) {

	/*
	 * allocate a pixmap to draw shapes in
	 */

    	w->clock.shape_mask = XCreatePixmap (XtDisplay (w), XtWindow (w),
	    	w->core.width, w->core.height, 1);
    	if (!w->clock.shapeGC)
            w->clock.shapeGC = XCreateGC (XtDisplay (w),
		w->clock.shape_mask, 0, &xgcv);

	/*
	 * build a transform for this new pixmap
	 */

	SetTransform (&w->clock.maskt,
			0, w->core.width,
			w->core.height, 0,
			-1.0, 1.0,
 			-1.0, 1.0);

    	XSetForeground (XtDisplay (w), w->clock.shapeGC, 0);
    	XFillRectangle (XtDisplay (w), w->clock.shape_mask, w->clock.shapeGC,
			0, 0, w->core.width, w->core.height);
    	XSetForeground (XtDisplay (w), w->clock.shapeGC, 1);

	/*
	 * draw the window shape
	 */

	TFillArc (XtDisplay (w), w->clock.shape_mask,
			w->clock.shapeGC, &w->clock.maskt,
			-WINDOW_WIDTH/2, -WINDOW_HEIGHT/2,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			0, 360 * 64);

	XShapeCombineMask (XtDisplay (w), XtWindow (w), ShapeWindow, 
		    w->clock.shape_mask, ShapeSet,
		    -xwc.border_width, -xwc.border_width);

	/*
	 * draw the border shape
	 */

	TFillArc (XtDisplay (w), w->clock.shape_mask,
			w->clock.shapeGC, &w->clock.maskt,
			-1.0, -1.0,
			2.0, 2.0,
			0, 360 * 64);

	XShapeCombineMask (XtDisplay (w), XtWindow (w), ShapeBorder,
		w->clock.shape_mask, ShapeSet,
		-xwc.border_width, -xwc.border_width);

	XFreePixmap (XtDisplay (w), w->clock.shape_mask);
	w->clock.shape_mask = 0;

	/*
	 * copy the border shape to any enclosing windows
	 */

	child = (Widget) w;
	border_width = xwc.border_width;
	while (parent = XtParent (child)) {
#ifdef NOTDEF
	    XShapeCombineShape (XtDisplay (parent), XtWindow (parent), ShapeWindow,
			    XtWindow (child), ShapeBorder, ShapeSet,
			    child->core.x + border_width,
			    child->core.y + border_width);
#endif
	    XShapeCombineShape (XtDisplay (parent), XtWindow (parent), ShapeBorder,
			    XtWindow (child), ShapeBorder, ShapeSet,
			    child->core.x + border_width,
			    child->core.y + border_width);
	    child = parent;
	    border_width = child->core.border_width;
	}
	face_x_extra = face_width - xwc.border_width;
	face_y_extra = face_height - xwc.border_width;
	SetTransform (&w->clock.t,
	    face_x_extra, xwc.width - face_x_extra,
	    xwc.height - face_y_extra, face_y_extra,
	    -WINDOW_WIDTH/2, WINDOW_WIDTH/2,
	    -WINDOW_HEIGHT/2, WINDOW_HEIGHT/2);
    
    } else {
    	SetTransform (&w->clock.t,
	    0, xwc.width,
	    xwc.height, 0,
	    -WINDOW_WIDTH/2, WINDOW_WIDTH/2,
	    -WINDOW_HEIGHT/2, WINDOW_HEIGHT/2);
    
    }
    w->clock.shape_width = w->core.width;
    w->clock.shape_height = w->core.height;
}
 
static void Realize (gw, valueMask, attrs)
     Widget gw;
     XtValueMask *valueMask;
     XSetWindowAttributes *attrs;
{
     ClockWidget	w = (ClockWidget)gw;
     static int		new_time();

    if (w->clock.backing_store != Always + WhenMapped + NotUseful) {
     	attrs->backing_store = w->clock.backing_store;
	*valueMask |= CWBackingStore;
    }
    *valueMask |= CWBorderPixel;
    attrs->border_pixel = w->clock.face;
    XtCreateWindow( gw, (unsigned)InputOutput, (Visual *)CopyFromParent,
		     *valueMask, attrs );
    Shape (gw);
    new_time ((caddr_t) gw, 0);
}

static void Destroy (gw)
     Widget gw;
{
     ClockWidget w = (ClockWidget)gw;
     if (w->clock.interval_id) XtRemoveTimeOut (w->clock.interval_id);
     XtDestroyGC (w->clock.faceGC);
     XtDestroyGC (w->clock.minuteGC);
     XtDestroyGC (w->clock.hourGC);
     if (w->clock.shapeGC)
	XtDestroyGC (w->clock.shapeGC);
}

/* ARGSUSED */
static void Redisplay(gw, event, region)
     Widget gw;
     XEvent *event;
     Region region;
{
    int		thick;
    ClockWidget	w;
    XGCValues	myXGCV;
    Display	*dpy;

    w = (ClockWidget) gw;
    Shape (w);
    (void) repaint_window ((ClockWidget)gw);
}

# define PI (3.14159265358979323846)

static double
clock_to_angle (clock)
double	clock;
{
	if (clock >= .75)
		clock -= 1.0;
	return -2.0 * PI * clock + PI / 2.0;
}

/* ARGSUSED */
static int new_time (client_data, id)
     caddr_t client_data;
     XtIntervalId id;		/* unused */
{
        ClockWidget	w = (ClockWidget)client_data;
	long		now;
	struct tm	*localtime (), *tm;
	
	if (w->clock.polys_valid) {
		paint_hands (w, XtWindow (w), w->clock.eraseGC, w->clock.eraseGC);
		check_jewel (w, XtWindow (w), w->clock.jewelGC);
	}
	(void) time (&now);
	tm = localtime (&now);
	if (tm->tm_hour >= 12)
		tm->tm_hour -= 12;
	w->clock.hour_angle = clock_to_angle ((((double) tm->tm_hour) +
				((double) tm->tm_min) / 60.0) / 12.0);
	w->clock.minute_angle =
		clock_to_angle (((double) tm->tm_min) / 60.0);
	w->clock.interval_id = XtAddTimeOut ((60 - tm->tm_sec) * 1000, new_time,
				client_data);
	compute_hands (w);
	paint_hands (w, XtWindow (w), w->clock.minuteGC, w->clock.hourGC);
} /* new_time */

paint_jewel (w, d, gc)
ClockWidget w;
Drawable    d;
GC	    gc;
{
	TFillArc (XtDisplay (w), d, gc, &w->clock.t,
			JEWEL_X - JEWEL_SIZE / 2.0, JEWEL_Y - JEWEL_SIZE / 2.0,
 			JEWEL_SIZE, JEWEL_SIZE, 0, 360 * 64);
}

#define sqr(x)	((x)*(x))

/*
 * check to see if the polygon intersects the circular jewel
 */

check_jewel_poly (w, poly)
ClockWidget	w;
TPoint		poly[POLY_SIZE];
{
	double	a2, b2, c2, d2;
	int	i;

	for (i = 0; i < POLY_SIZE-1; i++) {
		a2 = sqr (poly[i].x - JEWEL_X) + sqr (poly[i].y - JEWEL_Y);
		b2 = sqr (poly[i+1].x - JEWEL_X) + sqr (poly[i+1].y - JEWEL_Y);
		c2 = sqr (poly[i].x - poly[i+1].x) + sqr (poly[i].y - poly[i+1].y);
		d2 = a2 + b2 - c2;
		if (d2 <= sqr (JEWEL_SIZE) &&
		    a2 <= 2 * c2 && b2 <= 2 * c2 ||
 		    a2 <= sqr (JEWEL_SIZE) ||
		    b2 <= sqr (JEWEL_SIZE))
			return 1;
	}
	return 0;
}

check_jewel (w, d, gc)
ClockWidget	w;
Drawable	d;
GC		gc;
{
	int	i;

	if (!w->clock.polys_valid)
		return;
	if ((MINUTE_LENGTH >= (JEWEL_Y - JEWEL_SIZE/2.0) &&
	     check_jewel_poly (w, w->clock.minute_poly)) ||
	    (HOUR_LENGTH >= (JEWEL_Y - JEWEL_SIZE/2.0) &&
	     check_jewel_poly (w, w->clock.minute_poly)))
	{
		paint_jewel (w, d, gc);
	}
}

compute_hand (w, a, l, width, poly)
ClockWidget	w;
double		a, l, width;
TPoint		poly[POLY_SIZE];
{
	double	c, s;

	c = cos(a);
	s = sin(a);
	poly[0].x = c * l;	
	poly[0].y = s * l;
	poly[1].x = (l - width) * c - s * width;
	poly[1].y = (l - width) * s + c * width;
	poly[2].x = (-width) * c - s * width;
	poly[2].y = (-width) * s + c * width;
	poly[3].x = (-width) * c + s * width;
	poly[3].y = (-width) * s - c * width;
	poly[4].x = (l - width) * c + s * width;
	poly[4].y = (l - width) * s - c * width;
	poly[5].x = poly[0].x;
	poly[5].y = poly[0].y;
}

compute_hands (w)
ClockWidget	w;
{
	compute_hand (w, w->clock.minute_angle,
		MINUTE_LENGTH, MINUTE_WIDTH, w->clock.minute_poly);
	compute_hand (w, w->clock.hour_angle,
		HOUR_LENGTH, HOUR_WIDTH, w->clock.hour_poly);
	w->clock.polys_valid = 1;
}

paint_hand (w, d, gc, poly)
ClockWidget	w;
Drawable	d;
GC		gc;
TPoint		poly[POLY_SIZE];
{
	TFillPolygon (XtDisplay (w), d, gc, &w->clock.t, poly, POLY_SIZE, Convex,
			CoordModeOrigin);
}

paint_hands (w, d, minute_gc, hour_gc)
ClockWidget	w;
Drawable	d;
GC		minute_gc, hour_gc;
{
	paint_hand (w, d, hour_gc, w->clock.hour_poly);
	paint_hand (w, d, minute_gc, w->clock.minute_poly);
}

static
repaint_window (w)
    ClockWidget	w;
{
	if (XtIsRealized ((Widget) w)) {
		paint_jewel (w, XtWindow (w), w->clock.jewelGC);
		if (w->clock.polys_valid)
			paint_hands (w, XtWindow (w), w->clock.minuteGC, w->clock.hourGC);
	}
}
    
/* ARGSUSED */
static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    ClockWidget old = (ClockWidget)current;
    ClockWidget w = (ClockWidget)new;
    return( FALSE );
}
