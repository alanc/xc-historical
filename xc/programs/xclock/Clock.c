#ifndef lint
static char *sccsid = "@(#)Clock.c	1.0	2/25/87";
#endif lint

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/param.h>
#include <math.h>

#include "Xlib.h"
#include "Xutil.h"
#include "Intrinsic.h"
#include "Atoms.h"
#include "Clock.h"
#include "ClockPrivate.h"

extern long time();
static void clock_tic(), DrawHand(), DrawSecond(), SetSeg(), DrawClockFace();
	
/* Private Definitions */

#define VERTICES_IN_HANDS	6	/* to draw triangle */
#define PI			3.14159265358979
#define TWOPI			(2. * PI)

#define SECOND_HAND_FRACT	90
#define MINUTE_HAND_FRACT	70
#define HOUR_HAND_FRACT		40
#define HAND_WIDTH_FRACT	7
#define SECOND_WIDTH_FRACT	5
#define SECOND_HAND_TIME	30

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define abs(a) ((a) < 0 ? -(a) : (a))


/* Full instance record declaration */
typedef struct {
   Core core;
   Clock clock;
   } ClockData, *ClockWidget;

/* Initialization of defaults */

#define offset(field) Offset(ClockWidget,clock.field)

static Resource resources[] = {
    {XtNupdate, XtCInterval, XrmRInt, sizeof(int), 
        offset(update), XrmRString, "60" },
    {XtNforeground, XtCForeground, XrmRPixel, sizeof(Pixel),
        offset(fgpixel), XrmRString, "Black"},
    {XtNhand, XtCForeground, XrmRPixel, sizeof(Pixel),
        offset(Hdpixel), XrmRString, "Black"},
    {XtNhigh, XtCForeground, XrmRPixel, sizeof(Pixel),
        offset(Hipixel), XrmRString, "Black"},
    {XtNanalog, XtCBoolean, XrmRBoolean, sizeof(Boolean),
        offset(analog), XrmRString, "TRUE"},
    {XtNchime, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	offset(chime), XrmRString, "FALSE" },
    {XtNpadding, XtCMargin, XrmRInt, sizeof(int),
        offset(padding), XrmRString, "8"},
    {XtNfont, XtCFont, XrmRFontStruct, sizeof(XFontStruct *),
        offset(font), XrmRString, "6x10"},
};

#undef offset

static void Initialize(), Realize(), Resize(), Redisplay(), SetValues();

ClockWidgetClassData clockWidgetClassData = {
  { /* core fields */
    /* superclass */ 	&widgetClassData,
    /* class_name */ 	"Clock",
    /* size */		sizeof(ClockWidgetClassData),
    /* initialize */	Initialize,
    /* realize */	Realize,
    /* actions */	NULL,
    /* resources */	resources,
    /* resource_count*/ XtNumber(resources),
    /* xrm_extra */     NULL,
    /* xrm_class */     NULL,
    /* visible_interest */ FALSE,
    /* destroy */       NULL,
    /* resize */        Resize,
    /* expose */	Redisplay,
    /* set_values */    SetValues,
    /* accepts_focus */ FALSE,
    }
};

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/


static Atom XtTimerExpired = -1;  /* XXXXXXXXXXXXXXXXXX */

static void EventHandler(w, event)
    ClockWidget w;
    XEvent *event;
{
    if (event->type == ClientMessage && event->xclient.message_type == XtTimerExpired)
        clock_tic(w);
}

static void Resize (w) 
    ClockWidget w;
{
    /* don't do this computation if window hasn't been realized yet. */
    if(w->core.window && w->clock.analog) {
        w->clock.radius = (min(w->core.width, w->core.height)-(2 * w->clock.padding)) / 2;
        w->clock.second_hand_length = ((SECOND_HAND_FRACT * w->clock.radius) / 100);
        w->clock.minute_hand_length = ((MINUTE_HAND_FRACT * w->clock.radius) / 100);
        w->clock.hour_hand_length = ((HOUR_HAND_FRACT * w->clock.radius) / 100);
        w->clock.hand_width = ((HAND_WIDTH_FRACT * w->clock.radius) / 100);
        w->clock.second_hand_width = ((SECOND_WIDTH_FRACT * w->clock.radius) / 100);
        w->clock.centerX = w->core.width / 2;
        w->clock.centerY = w->core.height / 2;
    }
}

static void Redisplay (w)
    ClockWidget w;
{
    if (w->clock.analog)
        DrawClockFace(w);
    clock_tic(w);
}


static void Initialize (w)
    ClockWidget w;
{
    GCMask		valuemask;
    XGCValues	myXGCV;

    if(!w->clock.analog) {
       char *str;
       struct tm tm, *localtime();
       long time_value;
       int min_height, min_width;
       (void) time(&time_value);
       tm = *localtime(&time_value);
       str = asctime(&tm);
       min_width = XTextWidth(w->clock.font, str, strlen(str)) +
	  2 * w->clock.padding;
       min_height = w->clock.font->ascent +
	  w->clock.font->descent + 2 * w->clock.padding;
       if (w->core.width < min_width) w->core.width = min_width;
       if (w->core.height < min_height) w->core.width = min_height;
    }
    valuemask = GCForeground | GCBackground | GCFont | GCLineWidth;
    myXGCV.foreground = w->clock.fgpixel;
    myXGCV.background = w->core.background_pixel;
    myXGCV.font = w->clock.font->fid;
    myXGCV.line_width = 0;
    w->clock.myGC = XtGetGC(w, valuemask, &myXGCV);

    valuemask = GCForeground | GCLineWidth ;
    myXGCV.foreground = w->core.background_pixel;
    w->clock.EraseGC = XtGetGC(w, valuemask, &myXGCV);

    myXGCV.foreground = w->clock.Hipixel;
    w->clock.HighGC = XtGetGC(w, valuemask, &myXGCV);

    valuemask = GCForeground;
    myXGCV.foreground = w->clock.Hdpixel;
    w->clock.HandGC = XtGetGC(w, valuemask, &myXGCV);

    XtSetEventHandler (w, EventHandler, 0, TRUE);

    w->clock.interval_id = XtAddTimeOut(w, w->clock.update*1000);
    w->clock.show_second_hand = (w->clock.update <= SECOND_HAND_TIME);
}

static void Realize (w, valueMask, attrs)
     ClockWidget w;
     ValueMask valueMask;
     XSetWindowAttributes *attrs;
{
     w->core.window = XCreateWindow (w->core.display, w->core.parent->core.window,
	  w->core.x, w->core.y, w->core.width, w->core.height, w->core.border_width,
          CopyFromParent, InputOutput, CopyFromParent, valueMask, attrs);
     Resize(w);
}

static void clock_tic(w)
        ClockWidget w;
{
    
	struct tm *localtime();
	struct tm tm; 
	long	time_value;
	char	time_string[28];
	char	*time_ptr = time_string;
        register Display *dpy = w->core.display;

	(void) time(&time_value);
	tm = *localtime(&time_value);
	/*
	 * Beep on the half hour; double-beep on the hour.
	 */
	if (w->clock.chime == TRUE) {
	    if (w->clock.beeped && (tm.tm_min != 30) &&
		(tm.tm_min != 0))
	      w->clock.beeped = FALSE;
	    if (((tm.tm_min == 30) || (tm.tm_min == 0)) 
		&& (!w->clock.beeped)) {
		w->clock.beeped = TRUE;
		XBell(dpy, 50);	
		if (tm.tm_min == 0)
		  XBell(dpy, 50);
	    }
	}
	if( w->clock.analog == FALSE ) {
	    time_ptr = asctime(&tm);
	    time_ptr[strlen(time_ptr) - 1] = 0;
	    XDrawImageString (dpy, w->core.window, w->clock.myGC,
			     2+w->clock.padding, 2+w->clock.font->ascent+w->clock.padding,
			     time_ptr, strlen(time_ptr));
	} else {
			/*
			 * The second (or minute) hand is sec (or min) 
			 * sixtieths around the clock face. The hour hand is
			 * (hour + min/60) twelfths of the way around the
			 * clock-face.  The derivation is left as an excercise
			 * for the reader.
			 */

			/*
			 * 12 hour clock.
			 */
			if(tm.tm_hour > 12)
				tm.tm_hour -= 12;

			/*
			 * Erase old hands.
			 */
			if(w->clock.numseg > 0) {
			    if (w->clock.show_second_hand == TRUE) {
				XDrawLines(dpy, w->core.window,
					w->clock.EraseGC,
					w->clock.sec,
					VERTICES_IN_HANDS-1,
					CoordModeOrigin);
				if(w->clock.Hdpixel != w->core.background_pixel) {
				    XFillPolygon(dpy,
					w->core.window, w->clock.EraseGC,
					w->clock.sec,
					VERTICES_IN_HANDS-2,
					Convex, CoordModeOrigin
				    );
				}
			    }
			    if(	tm.tm_min != w->clock.otm.tm_min ||
				tm.tm_hour != w->clock.otm.tm_hour ) {
				XDrawLines( dpy,
					   w->core.window,
					   w->clock.EraseGC,
					   w->clock.segbuff,
					   VERTICES_IN_HANDS,
					   CoordModeOrigin);
				XDrawLines( dpy,
					   w->core.window,
					   w->clock.EraseGC,
					   w->clock.hour,
					   VERTICES_IN_HANDS,
					   CoordModeOrigin);
				if(w->clock.Hdpixel != w->core.background_pixel) {
				    XFillPolygon( dpy,
					w->core.window, w->clock.EraseGC,
					w->clock.segbuff, VERTICES_IN_HANDS,
					Convex, CoordModeOrigin
				    );
				    XFillPolygon(dpy,
					w->core.window, w->clock.EraseGC,
					w->clock.hour,
					VERTICES_IN_HANDS,
					Convex, CoordModeOrigin
				    );
				}
			    }
		    }

		    if (w->clock.numseg == 0 ||
			tm.tm_min != w->clock.otm.tm_min ||
			tm.tm_hour != w->clock.otm.tm_hour) {
			    w->clock.segbuffptr = w->clock.segbuff;
			    w->clock.numseg = 0;
			    /*
			     * Calculate the hour hand, fill it in with its
			     * color and then outline it.  Next, do the same
			     * with the minute hand.  This is a cheap hidden
			     * line algorithm.
			     */
			    DrawHand(w,
				w->clock.minute_hand_length, w->clock.hand_width,
				((double) tm.tm_min)/60.0
			    );
			    if(w->clock.Hdpixel != w->core.background_pixel)
				XFillPolygon( dpy,
				    w->core.window, w->clock.HandGC,
				    w->clock.segbuff, VERTICES_IN_HANDS,
				    Convex, CoordModeOrigin
				);
			    XDrawLines( dpy,
				w->core.window, w->clock.HighGC,
				w->clock.segbuff, VERTICES_IN_HANDS,
				       CoordModeOrigin);
			    w->clock.hour = w->clock.segbuffptr;
			    DrawHand(w, 
				w->clock.hour_hand_length, w->clock.hand_width,
				((((double)tm.tm_hour) + 
				    (((double)tm.tm_min)/60.0)) / 12.0)
			    );
			    if(w->clock.Hdpixel != w->core.background_pixel) {
			      XFillPolygon(dpy,
					   w->core.window, w->clock.HandGC,
					   w->clock.hour,
					   VERTICES_IN_HANDS,
					   Convex, CoordModeOrigin
					   );
			    }
			    XDrawLines( dpy,
				       w->core.window, w->clock.HighGC,
				       w->clock.hour, VERTICES_IN_HANDS,
				       CoordModeOrigin );

			    w->clock.sec = w->clock.segbuffptr;
		    }
		    if (w->clock.show_second_hand == TRUE) {
			    w->clock.segbuffptr = w->clock.sec;
			    DrawSecond(w,
				w->clock.second_hand_length - 2, 
				w->clock.second_hand_width,
				w->clock.minute_hand_length + 2,
				((double) tm.tm_sec)/60.0
			    );
			    if(w->clock.Hdpixel != w->core.background_pixel)
				XFillPolygon( dpy,
				    w->core.window, w->clock.HandGC,
				    w->clock.sec,
				    VERTICES_IN_HANDS -2,
				    Convex, CoordModeOrigin
			    );
			    XDrawLines( dpy,
				       w->core.window, w->clock.HighGC,
				       w->clock.sec,
				       VERTICES_IN_HANDS-1,
				       CoordModeOrigin
				        );

			}
			w->clock.otm = tm;
			
		}
}
	
/*
 * DrawLine - Draws a line.
 *
 * blank_length is the distance from the center which the line begins.
 * length is the maximum length of the hand.
 * Fraction_of_a_circle is a fraction between 0 and 1 (inclusive) indicating
 * how far around the circle (clockwise) from high noon.
 *
 * The blank_length feature is because I wanted to draw tick-marks around the
 * circle (for seconds).  The obvious means of drawing lines from the center
 * to the perimeter, then erasing all but the outside most pixels doesn't
 * work because of round-off error (sigh).
 */
static void DrawLine(w, blank_length, length, fraction_of_a_circle)
ClockWidget w;
Dimension blank_length;
Dimension length;
double fraction_of_a_circle;
{
	register double angle, cosangle, sinangle;
	double cos();
	double sin();

	/*
	 *  A full circle is 2 PI radians.
	 *  Angles are measured from 12 o'clock, clockwise increasing.
	 *  Since in X, +x is to the right and +y is downward:
	 *
	 *	x = x0 + r * sin(theta)
	 *	y = y0 - r * cos(theta)
	 *
	 */
	angle = TWOPI * fraction_of_a_circle;
	cosangle = cos(angle);
	sinangle = sin(angle);

	SetSeg(w, 
	       w->clock.centerX + (int)(blank_length * sinangle),
	       w->clock.centerY - (int)(blank_length * cosangle),
	       w->clock.centerX + (int)(length * sinangle),
	       w->clock.centerY - (int)(length * cosangle));
}

/*
 * DrawHand - Draws a hand.
 *
 * length is the maximum length of the hand.
 * width is the half-width of the hand.
 * Fraction_of_a_circle is a fraction between 0 and 1 (inclusive) indicating
 * how far around the circle (clockwise) from high noon.
 *
 */
static void DrawHand(w, length, width, fraction_of_a_circle)
ClockWidget w;
Dimension length, width;
double fraction_of_a_circle;
{

	register double angle, cosangle, sinangle;
	register double ws, wc;
	Position x, y, x1, y1, x2, y2;
	double cos();
	double sin();

	/*
	 *  A full circle is 2 PI radians.
	 *  Angles are measured from 12 o'clock, clockwise increasing.
	 *  Since in X, +x is to the right and +y is downward:
	 *
	 *	x = x0 + r * sin(theta)
	 *	y = y0 - r * cos(theta)
	 *
	 */
	angle = TWOPI * fraction_of_a_circle;
	cosangle = cos(angle);
	sinangle = sin(angle);
	/*
	 * Order of points when drawing the hand.
	 *
	 *		1,4
	 *		/ \
	 *	       /   \
	 *	      /     \
	 *	    2 ------- 3
	 */
	wc = width * cosangle;
	ws = width * sinangle;
	SetSeg(w,
	       x = w->clock.centerX + round(length * sinangle),
	       y = w->clock.centerY - round(length * cosangle),
	       x1 = w->clock.centerX - round(ws + wc), 
	       y1 = w->clock.centerY + round(wc - ws));  /* 1 ---- 2 */
	/* 2 */
	SetSeg(w, x1, y1, 
	       x2 = w->clock.centerX - round(ws - wc), 
	       y2 = w->clock.centerY + round(wc + ws));  /* 2 ----- 3 */

	SetSeg(w, x2, y2, x, y);	/* 3 ----- 1(4) */
}

/*
 * DrawSecond - Draws the second hand (diamond).
 *
 * length is the maximum length of the hand.
 * width is the half-width of the hand.
 * offset is direct distance from center to tail end.
 * Fraction_of_a_circle is a fraction between 0 and 1 (inclusive) indicating
 * how far around the circle (clockwise) from high noon.
 *
 */
static void DrawSecond(w, length, width, offset, fraction_of_a_circle)
ClockWidget w;
Dimension length, width, offset;
double fraction_of_a_circle;
{

	register double angle, cosangle, sinangle;
	register double ms, mc, ws, wc;
	register int mid;
	Position x, y;
	double cos();
	double sin();

	/*
	 *  A full circle is 2 PI radians.
	 *  Angles are measured from 12 o'clock, clockwise increasing.
	 *  Since in X, +x is to the right and +y is downward:
	 *
	 *	x = x0 + r * sin(theta)
	 *	y = y0 - r * cos(theta)
	 *
	 */
	angle = TWOPI * fraction_of_a_circle;
	cosangle = cos(angle);
	sinangle = sin(angle);
	/*
	 * Order of points when drawing the hand.
	 *
	 *		1,5
	 *		/ \
	 *	       /   \
	 *	      /     \
	 *	    2<       >4
	 *	      \     /
	 *	       \   /
	 *		\ /
	 *	-	 3
	 *	|
	 *	|
	 *   offset
	 *	|
	 *	|
	 *	-	 + center
	 */

	mid = (length + offset) / 2;
	mc = mid * cosangle;
	ms = mid * sinangle;
	wc = width * cosangle;
	ws = width * sinangle;
	/*1 ---- 2 */
	SetSeg(w,
	       x = w->clock.centerX + round(length * sinangle),
	       y = w->clock.centerY - round(length * cosangle),
	       w->clock.centerX + round(ms - wc),
	       w->clock.centerY - round(mc + ws) );
	SetSeg(w, w->clock.centerX + round(offset *sinangle),
	       w->clock.centerY - round(offset * cosangle), /* 2-----3 */
	       w->clock.centerX + round(ms + wc), 
	       w->clock.centerY - round(mc - ws));
	w->clock.segbuffptr->x = x;
	w->clock.segbuffptr++->y = y;
	w->clock.numseg ++;
}

static void SetSeg(w, x1, y1, x2, y2)
ClockWidget w;
int x1, y1, x2, y2;
{
	w->clock.segbuffptr->x = x1;
	w->clock.segbuffptr++->y = y1;
	w->clock.segbuffptr->x = x2;
	w->clock.segbuffptr++->y = y2;
	w->clock.numseg += 2;
}

/*
 *  Draw the clock face (every fifth tick-mark is longer
 *  than the others).
 */
static void DrawClockFace(w)
ClockWidget w;
{
	register int i;
	register int delta = (w->clock.radius - w->clock.second_hand_length) / 3;
	
	XClearWindow(w->core.display, w->core.window);
	w->clock.segbuffptr = w->clock.segbuff;
	w->clock.numseg = 0;
	for (i = 0; i < 60; i++)
		DrawLine(w, (i % 5) == 0 ? w->clock.second_hand_length : (w->clock.radius - delta),
                   w->clock.radius, ((double) i)/60.);
	/*
	 * Go ahead and draw it.
	 */
	XDrawSegments(w->core.display, w->core.window,
		      w->clock.myGC, (XSegment *) &(w->clock.segbuff[0]),
		      w->clock.numseg/2);
	
	w->clock.segbuffptr = w->clock.segbuff;
	w->clock.numseg = 0;
}

static int round(x)
double x;
{
	return(x >= 0.0 ? (int)(x + .5) : (int)(x - .5));
}

static void SetValues (w, newvals)
    ClockWidget w, newvals;
{
      int redisplay = FALSE;
      GCMask valuemask;
      XGCValues	myXGCV;

      /* first check for changes to clock-specific resources.  We'll accept all
         the changes, but may need to do some computations first. */

      if (newvals->clock.update != w->clock.update) {
	    XtRemoveTimeOut (newvals->clock.interval_id);
	    newvals->clock.interval_id = XtAddTimeOut(w, newvals->clock.update*1000);
	    newvals->clock.show_second_hand = (newvals->clock.update <= SECOND_HAND_TIME);
      }

      if (newvals->clock.analog != w->clock.analog)
	   redisplay = TRUE;

      if (newvals->clock.padding != w->clock.padding) {
	   Resize(w);
	   redisplay = TRUE;
	   }

      if (newvals->clock.font != w->clock.font)
	   redisplay = TRUE;

      if ((newvals->clock.fgpixel != w->clock.fgpixel)
          || (newvals->core.background_pixel != w->core.background_pixel)) {
          valuemask = GCForeground | GCBackground | GCFont | GCLineWidth;
	  myXGCV.foreground = w->clock.fgpixel;
	  myXGCV.background = w->core.background_pixel;
          myXGCV.font = w->clock.font->fid;
	  myXGCV.line_width = 0;
	  XtDestroyGC (w, w->clock.myGC);
	  newvals->clock.myGC = XtGetGC(w, valuemask, &myXGCV);
	  redisplay = TRUE;
          }

      if (newvals->clock.Hipixel != w->clock.Hipixel) {
          valuemask = GCForeground | GCLineWidth;
	  myXGCV.foreground = w->clock.fgpixel;
          myXGCV.font = w->clock.font->fid;
	  myXGCV.line_width = 0;
	  XtDestroyGC (w, w->clock.HighGC);
	  newvals->clock.HighGC = XtGetGC(w, valuemask, &myXGCV);
	  redisplay = TRUE;
          }

      if (newvals->clock.Hdpixel != w->clock.Hdpixel) {
          valuemask = GCForeground;
	  myXGCV.foreground = w->clock.fgpixel;
	  XtDestroyGC (w, w->clock.HandGC);
	  newvals->clock.HandGC = XtGetGC(w, valuemask, &myXGCV);
	  redisplay = TRUE;
          }

      if (newvals->core.background_pixel != w->core.background_pixel) {
          valuemask = GCForeground | GCLineWidth;
	  myXGCV.foreground = w->core.background_pixel;
	  myXGCV.line_width = 0;
	  XtDestroyGC (w, w->clock.EraseGC);
	  newvals->clock.EraseGC = XtGetGC(w, valuemask, &myXGCV);
	  redisplay = TRUE;
	  }

     w->clock = newvals->clock;

     if ((newvals->core.x != w->core.x)
       || (newvals->core.y != w->core.y)
       || (newvals->core.width != w->core.width)
       || (newvals->core.height != w->core.height))
         redisplay = TRUE;

     (*w->core.widget_class->coreClass.superclass->coreClass.set_values)(w, newvals);

     if(redisplay) {
	XClearWindow(w->core.display, w->core.window);
	Redisplay(w);
        }
}
