#ifndef lint
static char Xrcsid[] = "$XConsortium: Command.c,v 1.49 89/03/30 16:53:29 jim Exp $";
#endif /* lint */

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


/*
 * Command.c - Command button widget
 *
 */

#define XtStrlen(s)	((s) ? strlen(s) : 0)

  /* The following are defined for the reader's convenience.  Any
     Xt..Field macro in this code just refers to some field in
     one of the substructures of the WidgetRec.  */

#include <stdio.h>
#include <ctype.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Misc.h>
#include <X11/Xaw/CommandP.h>
#include "CommandI.h"

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char defaultTranslations[] =
    "<EnterWindow>:	highlight()		\n\
     <LeaveWindow>:	reset()			\n\
     <Btn1Down>:	set()			\n\
     <Btn1Up>:		notify() unset()	";

#define offset(field) XtOffset(CommandWidget, field)
static XtResource resources[] = { 

   {XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t), 
      offset(command.callbacks), XtRCallback, (caddr_t)NULL},
   {XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      offset(command.highlight_thickness), XtRImmediate, (caddr_t)2},
};
#undef offset

static XtActionsRec actionsList[] =
{
  {"set",		Set},
  {"notify",		Notify},
  {"highlight",		Highlight},
  {"reset",		Reset},
  {"unset",		Unset},
  {"unhighlight",	Unhighlight},
};

/*
 * This is a temporary exported actions list for the command
 * widget I have added this code because there is a bug in the MIT
 * Xtk Intrinsics implementation that does not allow the action table
 * to be retreived from the widget class, Ralph Swick has promised me that
 * this will eventually be fixed, but until then this is how the toggle
 * widget will get the command widget's actions list.
 *
 * Chris D. Peterson - 12/28/88.
 */

XtActionList xaw_command_actions_list = actionsList;

#define SuperClass ((LabelWidgetClass)&labelClassRec)

CommandClassRec commandClassRec = {
  {
    (WidgetClass) SuperClass,		/* superclass		  */	
    "Command",				/* class_name		  */
    sizeof(CommandRec),			/* size			  */
    NULL,				/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    FALSE,				/* class_inited		  */
    Initialize,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* resource_count	  */
    NULLQUARK,				/* xrm_class		  */
    FALSE,				/* compress_motion	  */
    TRUE,				/* compress_exposure	  */
    TRUE,				/* compress_enterleave    */
    FALSE,				/* visible_interest	  */
    Destroy,				/* destroy		  */
    XtInheritResize,			/* resize		  */
    Redisplay,				/* expose		  */
    SetValues,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,		/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL				/* extension		  */
  },  /* CoreClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* LabelClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* CommandClass fields initialization */
};

  /* for public consumption */
WidgetClass commandWidgetClass = (WidgetClass) &commandClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static GC Get_GC(cbw, fg, bg)
    CommandWidget cbw;
    Pixel fg, bg;
{
    XGCValues	values;

    values.foreground   = fg;
    values.background	= bg;
    values.font		= ComWfont->fid;
    values.line_width   = ComWhighlightThickness > 1
			  ? ComWhighlightThickness : 0;

    return XtGetGC((Widget)cbw,
		   GCForeground | GCBackground | GCFont | GCLineWidth,
		   &values);
}


/* ARGSUSED */
static void Initialize(request, new)
 Widget request, new;
{
    CommandWidget cbw = (CommandWidget) new;

    ComWnormalGC = Get_GC(cbw, ComWforeground, ComWbackground);
    ComWinverseGC = Get_GC(cbw, ComWbackground, ComWforeground);
    XtReleaseGC(new, ComWlabelGC);
    ComWlabelGC = ComWnormalGC;

    ComWset = FALSE;
    ComWhighlighted = HighlightNone;
}

static Region HighlightRegion(cbw)
    CommandWidget cbw;
{
    static Region outerRegion = NULL, innerRegion, emptyRegion;
    XRectangle rect;

    if (outerRegion == NULL) {
	/* save time by allocating scratch regions only once. */
	outerRegion = XCreateRegion();
	innerRegion = XCreateRegion();
	emptyRegion = XCreateRegion();
    }

    rect.x = rect.y = 0;
    rect.width = cbw->core.width;
    rect.height = cbw->core.height;
    XUnionRectWithRegion( &rect, emptyRegion, outerRegion );
    rect.x = rect.y = ComWhighlightThickness;
    rect.width -= ComWhighlightThickness * 2;
    rect.height -= ComWhighlightThickness * 2;
    XUnionRectWithRegion( &rect, emptyRegion, innerRegion );
    XSubtractRegion( outerRegion, innerRegion, outerRegion );
    return outerRegion;
}


/***************************
*
*  Action Procedures
*
***************************/

/* ARGSUSED */
static void Set(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  CommandWidget cbw = (CommandWidget)w;
  if (!ComWset) {
      ComWset = TRUE;
      ComWlabelGC = ComWinverseGC;
      if (XtIsRealized(w))
	Redisplay(w, (XEvent *) NULL, (Region) NULL);
  }
}

/* ARGSUSED */
static void Unset(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;
{
  CommandWidget cbw = (CommandWidget)w;
  if (ComWset) {
      ComWset = FALSE;
      ComWlabelGC = ComWnormalGC;
      if (XtIsRealized(w))
	XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 
		   w->core.width, w->core.height, TRUE);
  }
}

/* ARGSUSED */
static void Reset(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;
{
  CommandWidget cbw = (CommandWidget)w;
  if (ComWset) {
      ComWhighlighted = HighlightNone;
      Unset(w,event,params,num_params);
  }
  else {
      Unhighlight(w,event,params,num_params);
  }
}

/* ARGSUSED */
static void Highlight(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		
     Cardinal *num_params;	
{
  CommandWidget cbw = (CommandWidget)w;
  if ( *num_params == (Cardinal) 0) 
    ComWhighlighted = HighlightWhenUnset;
  else {
    if ( *num_params != (Cardinal) 1) 
      XtWarning("Too many parameters passed to highlight action table.");
    switch (params[0][0]) {
    case 'A':
    case 'a':
      ComWhighlighted = HighlightAlways;
      break;
    default:
      ComWhighlighted = HighlightWhenUnset;
      break;
    }
  }

  if (XtIsRealized(w))
    Redisplay(w, (XEvent *) NULL, HighlightRegion(cbw));
}

/* ARGSUSED */
static void Unhighlight(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  CommandWidget cbw = (CommandWidget)w;
  ComWhighlighted = HighlightNone;
  if (XtIsRealized(w))
    Redisplay(w, (XEvent *) NULL, HighlightRegion(cbw));
}

/* ARGSUSED */
static void Notify(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  CommandWidget cbw = (CommandWidget)w; 
  if (ComWset)              /*  Why is this code here??? */
    XtCallCallbacks(w, XtNcallback, NULL);
}
/*
 * Repaint the widget window
 */

/************************
*
*  REDISPLAY (DRAW)
*
************************/

/* ARGSUSED */
static void Redisplay(w, event, region)
    Widget w;
    XEvent *event;
    Region region;
{
   CommandWidget cbw = (CommandWidget) w;
   Boolean very_thick = ComWhighlightThickness > Min(ComWwidth,ComWheight)/2;
   GC norm_gc, rev_gc;

/* 
 * This should probabally be more sophisticated, but it works. 
 *
 * Current behavior causesd the command widget to be less efficient when
 * set.
 */
   
   if (ComWset) {
     XFillRectangle(XtDisplay(w), XtWindow(w), ComWnormalGC,
		    0, 0, ComWwidth, ComWheight);
     region = NULL;
   }

   if (ComWhighlightThickness > 0) {
     switch (ComWhighlighted) {
     case HighlightNone:
       if (ComWset) {
	 norm_gc = ComWinverseGC;
	 rev_gc = ComWnormalGC;
       }
       else {
	 norm_gc = ComWnormalGC;
	 rev_gc = ComWinverseGC;
       }
       break;
     case HighlightWhenUnset:
     case HighlightAlways:
       if (ComWset) {
	 norm_gc = ComWnormalGC;
	 rev_gc = ComWinverseGC;
       }
       else {
	 norm_gc = ComWinverseGC;
	 rev_gc = ComWnormalGC;
       }
       break;
     default:
       XtError("Unknown Highlight state in Command Widget Redisplay.\n");
     }
     if ( !((ComWhighlighted == HighlightWhenUnset) && ComWset) ) {
       if (very_thick) {
	 ComWlabelGC = norm_gc;
	 XFillRectangle(XtDisplay(w),XtWindow(w), rev_gc,
			0, 0, ComWwidth, ComWheight);
       }
       else {
	 /* wide lines are centered on the path, so indent it */
	 int offset = ComWhighlightThickness/2;
	 XDrawRectangle(XtDisplay(w),XtWindow(w), rev_gc, offset,
			offset, ComWwidth - ComWhighlightThickness,
			ComWheight - ComWhighlightThickness);
       }
     }
   }

   (*SuperClass->core_class.expose) (w, event, region);
}


/* ARGSUSED */
static void Destroy(w)
    Widget w;
{
    CommandWidget cbw = (CommandWidget)w;
    ComWlabelGC = ComWnormalGC;	/* so Label can release it */
    XtReleaseGC( w, ComWinverseGC );
}


/*
 * Set specified arguments into widget
 */
/* ARGSUSED */
static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    CommandWidget oldcbw = (CommandWidget) current;
    CommandWidget cbw = (CommandWidget) new;
    Boolean redisplay = False;

    if (XtCField(oldcbw,sensitive) != ComWsensitive && !ComWsensitive) {
	/* about to become insensitive */
	ComWset = FALSE;
	ComWhighlighted = HighlightNone;
    }

    if (XtLField(oldcbw,foreground) != ComWforeground ||
	XtCField(oldcbw, background_pixel) != ComWbackground ||
	XtCBField(oldcbw,highlight_thickness) != ComWhighlightThickness ||
	XtLField(oldcbw,font) != ComWfont)
    {
	if (ComWset)		/* Label has release one of these */
	    XtReleaseGC(new, ComWnormalGC);
	else
	    XtReleaseGC(new, ComWinverseGC);
	ComWnormalGC = Get_GC(cbw, ComWforeground, ComWbackground);
	ComWinverseGC = Get_GC(cbw, ComWbackground, ComWforeground);
	redisplay = True;
    }

    ComWlabelGC = ComWset ? ComWinverseGC : ComWnormalGC;

    return (redisplay ||
	    XtCField(oldcbw, sensitive) != ComWsensitive ||
	    XtCBField(oldcbw, set) != ComWset ||
	    XtCBField(oldcbw, highlighted) != ComWhighlighted);
}
