/*
** planemask.c
**
** How to make a widget to choose a planemask.
**
** This should really be combined with dashlist.c - they have a lot of code
** in common.
**
** All of these functions and variables have analogous ones in dashlist.c,
** which are commented.
*/

#include <X11/IntrinsicP.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/StringDefs.h>
#include "xgc.h"

void choose_plane();
extern void interpret();

extern XStuff X;

static unsigned long planemask;

void create_planemask_choice(w)
     Widget w;
{
  static XtCallbackRec callbacklist[] = {
    {(XtCallbackProc) choose_plane, NULL},
    {NULL,                          NULL}
  };

  static Arg labelargs[] = {
    {XtNborderWidth,     (XtArgVal) 0},
    {XtNjustify,         (XtArgVal) XtJustifyRight},
    {XtNvertDistance,    (XtArgVal) 4}
  };

  static Arg pmargs[] = {
    {XtNcallback,          (XtArgVal) NULL},
    {XtNhorizDistance,     (XtArgVal) NULL},
    {XtNfromHoriz,         (XtArgVal) NULL},
    {XtNwidth,             (XtArgVal) 10},
    {XtNheight,            (XtArgVal) 10},
    {XtNhighlightThickness,(XtArgVal) 1},
    {XtNforeground,        (XtArgVal) NULL},
    {XtNbackground,        (XtArgVal) NULL}
  };

  static Widget label;
  static Widget *pm;
  static int *pminfo;

  int i, num_planes;

  char buf[40];

  num_planes = PlanesOfScreen(X.scr);

  planemask = (1<<num_planes)-1;
  sprintf(buf,"planemask %d",planemask);
  interpret(buf);

  pm = (Widget *) malloc(num_planes * sizeof(Widget));
  pminfo = (int *) malloc(num_planes * sizeof(int));

  label = XtCreateManagedWidget("planemask",labelWidgetClass,w,
				labelargs,XtNumber(labelargs));

  pmargs[0].value = (XtArgVal) callbacklist;
  pmargs[6].value = (XtArgVal) X.background;
  pmargs[7].value = (XtArgVal) X.foreground;
  
  for (i=0;i<num_planes;++i) {
    if (i==0) {
      pmargs[1].value = (XtArgVal) 10;
      pmargs[2].value = (XtArgVal) label;
    }
    else {
      pmargs[1].value = (XtArgVal) -1;
      pmargs[2].value = (XtArgVal) pm[i-1];
    }

    pminfo[i] = i;
    callbacklist[0].closure = (caddr_t) &pminfo[i];

    pm[i] = XtCreateManagedWidget(NULL,commandWidgetClass,w,
				  pmargs,XtNumber(pmargs));
  }
}

/*ARGSUSED*/
static void choose_plane(w,closure,call_data)
     Widget w;
     caddr_t closure;
     caddr_t call_data;
{
  Boolean on;
  int num;
  char buf[80];

  static Arg args[] = {
    {XtNforeground,  (XtArgVal) NULL},
    {XtNbackground,  (XtArgVal) NULL},
  };

  num = * (int *) closure;
  if (planemask & 1<<num) on = FALSE;
  else on = TRUE;

  if (on) {
    planemask |= 1<<num;
    args[0].value = (XtArgVal) X.background;
    args[1].value = (XtArgVal) X.foreground;
  }
  else {
    planemask &= ~(1<<num);
    args[0].value = (XtArgVal) X.foreground;
    args[1].value = (XtArgVal) X.background;
  }

  XtSetValues(w,args,XtNumber(args));

  (void) sprintf(buf,"planemask %d\n",planemask);
  interpret(buf);
}

void update_planemask(w, mask)
     Widget w;
     long mask;
{
  int i;
  Widget maskwidget;
  CompositeWidget cw;
  static Arg maskargs[] = {
    {XtNforeground, (XtArgVal) NULL},
    {XtNbackground, (XtArgVal) NULL}
  };

  planemask = mask;

  for (i = 0; i < PlanesOfScreen(X.scr); ++i) {
    cw = (CompositeWidget) w;
    maskwidget = cw->composite.children[i+1]; /* the zeroth child
                                                 is the label */

    if (planemask & 1<<i) {        /* if it's set, make it look that way */
      maskargs[0].value = (XtArgVal) X.background;
      maskargs[1].value = (XtArgVal) X.foreground;
    }
    else {
      maskargs[0].value = (XtArgVal) X.foreground;
      maskargs[1].value = (XtArgVal) X.background;
    }

    XtSetValues(maskwidget,maskargs,XtNumber(maskargs));
  }
}
