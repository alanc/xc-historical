#ifndef lint
static char Xrcsid[] = "$XConsortium: Command.c,v 1.40 88/09/06 16:41:07 jim Exp $";
#endif lint

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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include "XawMisc.h"
#include "CommandP.h"
#include "CommandI.h"

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char defaultTranslations[] =
    "<Btn1Down>:	set() \n\
     <Btn1Up>:		notify() unset() \n\
     <EnterWindow>:	highlight() \n\
     <LeaveWindow>:	unset(NoRedisplay) unhighlight()";

static Dimension defHighlight = 2;

#define offset(field) XtOffset(CommandWidget, field)
static XtResource resources[] = { 

   {XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t), 
      offset(command.callbacks), XtRCallback, (caddr_t)NULL},
   {XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      offset(command.highlight_thickness), XtRDimension, (caddr_t)&defHighlight},
};
#undef offset

static XtActionsRec actionsList[] =
{
  {"set",		Set},
  {"notify",		Notify},
  {"highlight",		Highlight},
  {"unset",		Unset},
  {"unhighlight",	Unhighlight},
};

  /* ...ClassData must be initialized at compile time.  Must
     initialize all substructures.  (Actually, last two here
     need not be initialized since not used.)
  */
CommandClassRec commandClassRec = {
  {
    (WidgetClass) &labelClassRec,	/* superclass		  */	
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

      /* init flags for state */
    ComWset = FALSE;
    ComWhighlighted = FALSE;  
    ComWdisplayHighlighted = FALSE;
    ComWdisplaySet = FALSE;

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
  ComWset = TRUE;
  ComWlabelGC = ComWinverseGC;
  Redisplay(w, event, NULL);
}

/* ARGSUSED */
static void Unset(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;
{
  CommandWidget cbw = (CommandWidget)w;
  ComWset = FALSE;
  ComWlabelGC = ComWnormalGC;
  if (*num_params == 0)
      Redisplay(w, event, NULL);
}

/* ARGSUSED */
static void Highlight(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  CommandWidget cbw = (CommandWidget)w;
  ComWhighlighted = TRUE;
  Redisplay(w, event, NULL);
}

/* ARGSUSED */
static void Unhighlight(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  CommandWidget cbw = (CommandWidget)w;
  ComWhighlighted = FALSE;
  Redisplay(w, event, NULL);
}

/* ARGSUSED */
static void Notify(w,event,params,num_params)
     Widget w;
     XEvent *event;
     String *params;		/* unused */
     Cardinal *num_params;	/* unused */
{
  CommandWidget cbw = (CommandWidget)w;
  if (ComWset)
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

   (*XtSuperclass(w)->core_class.expose) (w, event, region);

   if (ComWhighlightThickness > 0) {
       if (very_thick)
	   XFillRectangle(XtDisplay(w),XtWindow(w), 
			  (ComWhighlighted ? ComWnormalGC : ComWinverseGC),
			  0,0,ComWwidth,ComWheight);
       else {
	   /* wide lines are centered on the path, so indent it */
	   int offset = ComWhighlightThickness/2;
	   XDrawRectangle(XtDisplay(w),XtWindow(w),
			  (ComWhighlighted ? ComWnormalGC : ComWinverseGC),
			  offset, offset,
			  ComWwidth - ComWhighlightThickness,
			  ComWheight - ComWhighlightThickness);
       }
   }
  
   ComWdisplayHighlighted = ComWhighlighted;
   ComWdisplaySet = ComWset;
}


/* ARGSUSED */
static void Destroy(w)
    Widget w;
{
    CommandWidget cbw = (CommandWidget)w;
    XtReleaseGC( XtDisplay(w), ComWnormalGC );
    XtReleaseGC( XtDisplay(w), ComWinverseGC );
}


/*
 * Set specified arguments into widget
 */
/* ARGSUSED */
static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    CommandWidget cbw = (CommandWidget) current;
    CommandWidget newcbw = (CommandWidget) new;
    Boolean redisplay = False;

    if (XtCField(newcbw,sensitive) != ComWsensitive &&
	!XtCField(newcbw,sensitive)) {  /* about to become insensitive? */
	XtCBField(newcbw,set) = FALSE;	/* yes, then anticipate unset */
	XtCBField(newcbw,highlighted) = FALSE;
    }

    if (XtLField(newcbw,foreground) != ComWforeground ||
	XtCField(newcbw, background_pixel) != ComWbackground ||
	XtCBField(newcbw,highlight_thickness) != ComWhighlightThickness ||
	XtLField(newcbw,font) != ComWfont)
    {
	XtReleaseGC(XtDisplay(new), ComWnormalGC);	   
	XtReleaseGC(XtDisplay(new), ComWinverseGC);
	ComWnormalGC = Get_GC(newcbw, ComWforeground, ComWbackground);
	ComWinverseGC = Get_GC(newcbw, ComWbackground, ComWforeground);
	redisplay = True;
    }

    return (redisplay ||
	    XtCField(newcbw, sensitive) != ComWsensitive ||
	    XtCBField(newcbw, set) != ComWset ||
	    XtCBField(newcbw, highlighted) != ComWhighlighted);
}
