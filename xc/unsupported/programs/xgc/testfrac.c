/*
** testfrac.c
**
** How to make a widget to choose the fraction of tests to be run.
**
*/

#include <X11/Intrinsic.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Scroll.h>
#include <X11/StringDefs.h>
#include "xgc.h"

#define SCROLLBAR_LENGTH 125
#define SLIDER_LENGTH 0.2	/* proportion of scrollbar taken up
				   by the slider */

static Widget label;		/* the label */
static Widget slider;		/* the scrollbar */
static Widget percent;	/* label with chosen percentage */

/* slider_jump(w,data,position)
** ----------------------------
** This function is called if the user moves the scrollbar to a new
** position (generally, by using the middle button).  It updates
** information about where the scrollbar is.
*/

/*ARGSUSED*/
static void
slider_jump(w, data, position)
     Widget w;
     caddr_t data;
     caddr_t position;
{
  static Arg percentargs[] = {
    {XtNlabel,   (XtArgVal) NULL}
  };

  float oldpercent;		/* where the scrollbar is */
  float newpercent;		/* normalized scrollbar */
  char snewpercent[3];		/* string representation of scrollbar */

  oldpercent = *(float *) position;

  /* We want the scrollbar to be at 100% when the right edge of the slider
  ** hits the end of the scrollbar, not the left edge.  When the right edge
  ** is at 1.0, the left edge is at 1.0 - SLIDER_LENGTH.  Normalize
  ** accordingly.  */
   
  newpercent = oldpercent / (1.0 - SLIDER_LENGTH);

  /* If the slider's partially out of the scrollbar, move it back in. */

  if (newpercent > 1.0) {
    newpercent = 1.0;
    XawScrollBarSetThumb( slider, 1.0 - SLIDER_LENGTH, SLIDER_LENGTH);
  }

  /* Store the position of the silder where it can be found */

  *(float *)data = newpercent;

  /* Update the label widget */

  sprintf(snewpercent,"%d",(int)(newpercent*100));
  percentargs[0].value = (XtArgVal) snewpercent;
  XtSetValues(percent, percentargs, XtNumber(percentargs));
}

/* slider_scroll(w,data,position)
** ------------------------------
** This function is called when the user does incremental scrolling, 
** generally with the left or right button.  Right now it just ignores it.
*/

/*ARGSUSED*/
static void
slider_scroll(w, data, position)
     Widget w;
     caddr_t data;
     caddr_t position;
{}

/* create_testfrac_choice(w,stored)
** -------------------------
** Inside w (a form widget), creates:
**   1. A label "Percentage of Test"
**   2. A scrollbar for the user to choose the percentage (from 0 to 100)
**   3. A label with the current percentage displayed on it.
** The percentage starts at 100.
** The current percentage is stored in *stored.
*/

void
create_testfrac_choice(w,stored)
     Widget w;
     float *stored;
{
  static XtCallbackRec jumpcallbacks[] = {
    {(XtCallbackProc) slider_jump, NULL},
    {NULL,                         NULL}
  };

  static XtCallbackRec scrollcallbacks[] = {
    {(XtCallbackProc) slider_scroll, NULL},
    {NULL,                           NULL}
  };

  static Arg labelargs[] = {
    {XtNborderWidth,  (XtArgVal) 0},
    {XtNjustify,      (XtArgVal) XtJustifyRight},
    {XtNvertDistance, (XtArgVal) 4}
  };

  static Arg percentargs[] = {
    {XtNborderWidth,    (XtArgVal) 1},
    {XtNhorizDistance,  (XtArgVal) 10},
    {XtNfromHoriz,      (XtArgVal) NULL}
  };

  static Arg scrollargs[] = {
    {XtNorientation,     (XtArgVal) XtorientHorizontal},
    {XtNlength,          (XtArgVal) SCROLLBAR_LENGTH},
    {XtNthickness,       (XtArgVal) 10},
    {XtNshown,           (XtArgVal) 10},
    {XtNhorizDistance,   (XtArgVal) 10},
    {XtNfromHoriz,       (XtArgVal) NULL},
    {XtNjumpProc,        (XtArgVal) NULL},
    {XtNscrollProc,      (XtArgVal) NULL}
  };
    
  /* Let the scrollbar know where to store information where we
  ** can see it */

  jumpcallbacks[0].closure = (caddr_t) stored;

  label = XtCreateManagedWidget("Percentage of Test",labelWidgetClass,w,
				labelargs,XtNumber(labelargs));

  percentargs[2].value = (XtArgVal) label;

  percent = XtCreateManagedWidget("100",labelWidgetClass,w,
				  percentargs,XtNumber(percentargs));

  scrollargs[5].value = (XtArgVal) percent;
  scrollargs[6].value = (XtArgVal) jumpcallbacks;
  scrollargs[7].value = (XtArgVal) scrollcallbacks;

  slider = XtCreateManagedWidget("Slider",scrollbarWidgetClass,w,
				 scrollargs,XtNumber(scrollargs));

  /* Start the thumb out at 100% */

  XawScrollBarSetThumb(slider, 1.0 - SLIDER_LENGTH, SLIDER_LENGTH);
}
