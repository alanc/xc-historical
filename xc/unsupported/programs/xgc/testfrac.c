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

#define SLIDER_LENGTH 100

static Widget label;		/* the label */
static Widget slider;		/* the scrollbar */
static Widget percent;	/* label with chosen percentage */

/*ARGSUSED*/
void slider_jump(w, data, position)
     Widget w;
     caddr_t data;
     caddr_t position;
{
  static Arg percentargs[] = {
    {XtNlabel,   (XtArgVal) NULL}
  };

  float newpercent;
  char snewpercent[3];

  newpercent = *(float *) position;
  *(float *) data = newpercent;
  sprintf(snewpercent,"%d",(int)(newpercent*100));

  percentargs[0].value = (XtArgVal) snewpercent;

  XtSetValues(percent, percentargs, XtNumber(percentargs));
}

/*ARGSUSED*/
void slider_scroll(w, data, position)
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

void create_testfrac_choice(w,stored)
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
    {XtNlength,          (XtArgVal) SLIDER_LENGTH},
    {XtNthickness,       (XtArgVal) 10},
    {XtNshown,           (XtArgVal) 10},
    {XtNhorizDistance,   (XtArgVal) 10},
    {XtNfromHoriz,       (XtArgVal) NULL},
    {XtNjumpProc,        (XtArgVal) NULL},
    {XtNscrollProc,      (XtArgVal) NULL}
  };
    
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

  XawScrollBarSetThumb(slider, 0.9, 0.1);

}
