/*
** dashlist.c
**
** How to make a widget to choose a dashlist.
**
** This and planemask.c can/should probably be combined.
*/

#include <X11/Intrinsic.h>
#include <X11/Form.h>
#include <X11/Label.h>
#include <X11/Command.h>
#include <X11/StringDefs.h>
#include "xgc.h"

#define DASHLENGTH 8		/* How many bits in the dash description */

extern XStuff X;

static short dashlist = 240;	/* in binary, becomes the dashlist
				   (240 = XXXX____) */

/* create_dashlist_choice(w)
** -------------------------
** Inside w (a form widget), creates a bunch of little command buttons
** in a row, representing the dash list.  There's also a label so
** the user knows what it is.
*/

void create_dashlist_choice(w)
     Widget w;
{
  static XtCallbackRec callbacklist[] = {
    {(XtCallbackProc) change_dashlist, NULL},
    {NULL,                             NULL}
  };

  static Arg labelargs[] = {
    {XtNborderWidth,  (XtArgVal) 0},
    {XtNjustify,      (XtArgVal) XtJustifyRight},
    {XtNvertDistance, (XtArgVal) 4}
  };

  static Arg dashargs[] = {
    {XtNcallback,         (XtArgVal) NULL},
    {XtNhorizDistance,    (XtArgVal) NULL},
    {XtNfromHoriz,        (XtArgVal) NULL},
    {XtNwidth,             (XtArgVal) 10},
    {XtNheight,            (XtArgVal) 10},
    {XtNhighlightThickness,(XtArgVal) 1},
    {XtNforeground,        (XtArgVal) NULL},
    {XtNbackground,        (XtArgVal) NULL}
  };

  static Widget label;		/* the label, of course */
  static Widget *dashes;	/* the command buttons */
  static int *dashinfo;		/* contains integers saying which bit
				   a particular button is; sent to
				   change_dashlist to tell it which
				   bit got changed */

  int i;			/* counter */

  /* allocate space for stuff that we don't know the size of yet */
  dashes = (Widget *) malloc(DASHLENGTH * sizeof(Widget));
  dashinfo = (int *) malloc(DASHLENGTH * sizeof(int));

  /* make the label widget */
  label = XtCreateManagedWidget("dashlist",labelWidgetClass,w,
				labelargs,XtNumber(labelargs));

  dashargs[0].value = (XtArgVal) callbacklist;

  for (i=0;i<DASHLENGTH;++i) {	/* go through all the buttons */
    if (i==0) {			/* offset the first one from the label */
      dashargs[1].value = (XtArgVal) 10;
      dashargs[2].value = (XtArgVal) label;
    }
    else {			/* put it directly to the right of the
				   last one, no space in between */
      dashargs[1].value = (XtArgVal) -1;
      dashargs[2].value = (XtArgVal) dashes[i-1];
    }

    if (dashlist&1<<i) {	/* if it's set, make it look that way */
      dashargs[6].value = (XtArgVal) X.background;
      dashargs[7].value = (XtArgVal) X.foreground;
    }
    else {			/* OK, don't */
      dashargs[6].value = (XtArgVal) X.foreground;
      dashargs[7].value = (XtArgVal) X.background;
    }

    dashinfo[i] = i;		/* which bit we're on; this is needed
				   in change_dashlist (the callback) */
    callbacklist[0].closure = (caddr_t) &dashinfo[i];

    /* make the button.  Note it has no name */
    dashes[i] = XtCreateManagedWidget(NULL,commandWidgetClass,w,
				  dashargs,XtNumber(dashargs));
  }
}

/* change_dashlist(w,closure,call_data)
** ------------------------------------
** Makes it obvious to the user that he has toggled this button.  Also
** makes the appropriate change to the dashlist and sends it off
** to interpret().
** Funny args are because it's a callback.
*/

/*ARGSUSED*/
void change_dashlist(w,closure,call_data)
     Widget w;
     caddr_t closure;
     caddr_t call_data;
{
  Boolean on;			/* whether we're turning it on or off */
  int num;			/* what number button it is */
  char buf[80];			/* string to send to interpret() */

  static Arg args[] = {
    {XtNforeground,  (XtArgVal) NULL},
    {XtNbackground,  (XtArgVal) NULL},
  };

  num = * (int *) closure;	/* we put it here back in the last function */
  if (dashlist & 1<<num) on = FALSE;
  else on = TRUE;

  if (on) {			/* we're turning it on */
    dashlist |= 1<<num;		/* add it to the dashlist */
    args[0].value = (XtArgVal) X.background; /* make it look on */
    args[1].value = (XtArgVal) X.foreground;
  }
  else {			/* we're turning it off */
    dashlist &= ~(1<<num);	/* take it off the dashlist */
    args[0].value = (XtArgVal) X.foreground; /* make it look off */
    args[1].value = (XtArgVal) X.background;
  }

  XtSetValues(w,args,XtNumber(args));

  /* now tell interpret() about it */
  sprintf(buf,"dashlist %d\n",dashlist); 
  interpret(buf,FALSE);
}


