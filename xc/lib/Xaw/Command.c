#ifndef lint
static char *sccsid = "@(#)Command.c	1.15	2/25/87";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/*
 * Command.c - Command button widget
 *
 * Rewritten for beta toolkit
 * Author:      Mark Ackerman
 *              MIT/Project Athena
 * Date:        August 27, 1987
 *
 * from Command.c (XToolkit, alpha version)
 *              Charles Haynes
 *              Digital Equipment Corporation
 *              Western Research Laboratory
 */

#define XtStrlen(s)	((s) ? strlen(s) : 0)

  /* The following are defined for the reader's convenience.  Any
     Xt..Field macro in this code just refers to some field in
     one of the substructures of the WidgetRec.  */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Intrinsic.h"
#include "Label.h"
#include "LabelPrivate.h"
#include "Command.h"
#include "CommandPrivate.h"
#include "CommandInternal.h"
#include "Atoms.h"

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char *defaultTranslation[] = {
    "<ButtonPress>:       set\n",
    "<ButtonRelease>:     notify unset\n",
    "<EnterWindow>:       highlight\n",
    "<LeaveWindow>:       unhighlight unset\n",
    NULL
};
static caddr_t defaultTranslations = (caddr_t)defaultTranslation;
static Resource resources[] = { 

   {XtNfunction, XtCFunction, XtRFunction, sizeof(CallbackProc), 
      Offset(CommandWidget, command.callback), XtRFunction, (caddr_t)NULL},
   {XtNparameter, XtCParameter, XtRPointer, sizeof(caddr_t), 
      Offset(CommandWidget,command.closure), XtRPointer, (caddr_t)NULL},

   {XtNhighlightThickness, XtCThickness, XrmRInt, sizeof(Dimension),
      Offset(CommandWidget,command.highlightThickness), XrmRString,"1"},
   {XtNtranslations, XtCTranslations, XtRTranslationTable,
      sizeof(Translations),
      Offset(CommandWidget, core.translations),XtRString,
      (caddr_t)&defaultTranslations},
 };  

static XtActionsRec actionsList[] =
{
  {"set", (caddr_t) Set},
  {"notify", (caddr_t) Notify},
  {"highlight", (caddr_t) Highlight},
  {"unset", (caddr_t) Unset},
  {"unhighlight", (caddr_t) Unhighlight},
};

  /* ...ClassData must be initialized at compile time.  Must
     initialize all substructures.  (Actually, last two here
     need not be initialized since not used.)
  */
CommandClassRec commandClassRec = {
  {
    (WidgetClass) &labelClassRec,    /* superclass	*/    
    "Command",                               /* class_name	*/
    sizeof(CommandRec),                 /* size		*/
    ClassInitialize,                       /* class initialize  */
    FALSE,                                 /* class_inited      */
    Initialize,                            /* initialize	*/
    Realize,                               /* realize		*/
    actionsList,                           /* actions		*/
    XtNumber(actionsList),
    resources,                             /* resources	        */
    XtNumber(resources),                   /* resource_count	*/
    NULLQUARK,                             /* xrm_class	        */
    FALSE,
    FALSE,
    FALSE,                                 /* visible_interest	*/
    Destroy,                               /* destroy		*/
    Resize,                                /* resize		*/
    Redisplay,                             /* expose		*/
    SetValues,                             /* set_values	*/
    NULL,                                  /* accept_focus	*/
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

XtCallbackType  activateCommand;   /* for public consumption*/
/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/


static void ClassInitialize()
{
  /*activateCommand = XtRegisterCallbackType(commandWidgetClass,
					   Offset(CommandWidget,
						  command.callbackList));
						  */
} 

static void Get_inverseGC(cbw)
    CommandWidget cbw;
{
    XGCValues	values;

    /* Set up a GC for inverse (set) state */

    values.background	= ComWforeground;
    values.foreground   = ComWbackground;
    values.font		= ComWfont->fid;
    values.fill_style   = FillSolid;

    ComWinverseGC = XtGetGC((Widget)cbw,
    	GCBackground | GCForeground | GCFont | GCFillStyle, &values);
}

static void Get_highlightGC(cbw)
    CommandWidget cbw;
{
    XGCValues	values;
    
    /* Set up a GC for highlighted state.  It has a thicker
       line width for the highlight border */

    values.background   = ComWbackground;
    values.line_width   = ComWhighlightThickness;

    ComWhighlightGC = XtGetGC((Widget)cbw,
    	GCForeground | GCLineWidth, &values);
}


static void Initialize(w)
 Widget w;
{
    CommandWidget cbw = (CommandWidget) w;

      /*** MAKE SURE core_class and coreClass standardized in
	             Label&Command   Intrinsic
      ****/
    XtCallParentProcedure(initialize,w);
        /* The above call will set all of the label fields such as
	   label text and internal width and height. */
    Get_inverseGC(cbw);
    Get_highlightGC(cbw);
      /* Start the callback list if the client specified one in
	 the arglist */
/*    if (ComWcallback != NULL)
      XtAddCallback(activateCommand,ComWcallback,ComWclosure);
*/
      /* init flags for state */
    ComWset = FALSE;
    ComWhighlighted = FALSE;  
    ComWdisplayHighlighted = FALSE;
    ComWdisplaySet = FALSE;

} 

static void Realize(w, valueMask, attributes) 
     /* This is the same as LabelWidget */
    register Widget w;
    Mask valueMask;
    XSetWindowAttributes *attributes;
{
  XtCallParentProcedure3Args(realize,w,valueMask,attributes);
} 

/*
static void AddCallback(widget,resourceName,callback,closure,position)
     Widget widget;
     XrmAtom resourceName;
     Callback callback;
     caddr_t closure;
     enum {Head, Tail} position;
{
  CommandWidget cbw = (CommandWidget)widget;
  XtAddSingleCallback(ComWcallbackList,resourceName,
		      callback,closure,position);
}
*/

/***************************
*
*  EVENT HANDLERS
*
***************************/

static void Set(w,event)
     Widget w;
     XEvent *event;
{
  CommandWidget cbw = (CommandWidget)w;
  ComWset = TRUE;
  Redisplay(w);
}

static void Unset(w,event)
     Widget w;
     XEvent *event;
{
  CommandWidget cbw = (CommandWidget)w;
  ComWset = FALSE;
  Redisplay(w);
}

static void Highlight(w,event)
     Widget w;
     XEvent *event;
{
  CommandWidget cbw = (CommandWidget)w;
  ComWhighlighted = TRUE;
  Redisplay(w);
}

static void Unhighlight(w,event)
     Widget w;
     XEvent *event;
{
  CommandWidget cbw = (CommandWidget)w;
  ComWhighlighted = FALSE;
  Redisplay(w);
}

static void Notify(w,event)
     Widget w;
     XEvent *event;
{
  CommandWidget cbw = (CommandWidget)w;
/*  XtCallCallback(cbw,activateCommand,NULL);*/
}
/*
 * Repaint the widget window
 */

/************************
*
*  REDISPLAY (DRAW)
*
************************/

static void Redisplay(w)
    Widget w;
{
   CommandWidget cbw = (CommandWidget) w;
   XSetWindowAttributes window_attributes;

   /* Here's the scoop:  If the command button button is normal,
      you show the text.  If the command button is highlighted but 
      not set, you draw a thick border and show the text.
      If the command button is set, draw the button and text
      in inverse. */

   /* Just to make sure everything is okay, check for sensitivity,
      too.  If non-sensitive, then gray out the border. */

   /* Note that Redisplay must remember the state of its last
      draw to determine whether to erase the window before
      redrawing to avoid flicker.  If the state is the same,
      the window just needs to redraw (even on an expose). */

   if (!ComWsensitive && ComWdisplaySensitive) 
      {
	  /* change border to gray */
	window_attributes.border_pixmap = ComWgrayPixmap;
	XChangeWindowAttributes(XtDisplay(w),XtWindow(w),
				CWBorderPixmap,&window_attributes);
      }
   else if (ComWsensitive && !ComWdisplaySensitive)
     {
       /* change border to black */
       window_attributes.border_pixel = ComWforeground;
       XChangeWindowAttributes(XtDisplay(w),XtWindow(w),
			       CWBorderPixel,&window_attributes);
     }

   if (ComWhighlighted != ComWdisplayHighlighted ||
       ComWsensitive != ComWdisplaySensitive ||
       (!ComWset && ComWdisplaySet))
     XClearWindow(XtDisplay(w),XtWindow(w));
     /* Don't clear the window if the button's in a set condition;
	instead, fill it with black to avoid flicker. */
   else if (ComWset == ComWdisplaySet ||  (ComWset && !ComWdisplaySet))
     XFillRectangle(XtDisplay(w),XtWindow(w), ComWinverseGC,
		    0,0,ComWlabelWidth,ComWlabelHeight);

     /* check whether border is taken out of size of rectangle or
	is outside of rectangle */
   if (ComWhighlighted)
     XDrawRectangle(XtDisplay(w),XtWindow(w), ComWhighlightGC,
		    0,0,ComWlabelWidth,ComWlabelHeight);

     /* draw the string:  there are three different "styles" for it,
	all in separate GCs */
   XDrawString(XtDisplay(w),XtWindow(w),
	       (ComWset ?  ComWinverseGC : 
		    (ComWsensitive ? ComWnormalGC : ComWgrayGC)),
		ComWlabelX, ComWlabelY, ComWlabel, ComWlabelLen);

   ComWdisplayHighlighted = ComWhighlighted;
   ComWdisplaySet = ComWset;
   ComWdisplaySensitive = ComWsensitive;
}


static void Resize(w)
    Widget	w;
{
    /* Nothing changes specific to command.  Label must change. */
  XtCallParentProcedure(resize,w);
}

static void Destroy()
{
  /* must free GCs and pixmaps */
}


/*
 * Set specified arguments into widget
 */
static void SetValues(old, new)
    Widget old, new;
{
    CommandWidget cbw = (CommandWidget) old;
    CommandWidget newcbw = (CommandWidget) new;

    XtCallParentProcedure2Args(set_values,old,new);

     /* XtDestroyGC */
     if (XtLField(newcbw,foreground) != ComWforeground)
       {
	 Get_inverseGC(newcbw);
	 Get_highlightGC(newcbw);
       }
    else 
      {
	if (XtCField(newcbw,background_pixel) != ComWbackground ||
	     XtLField(newcbw,font) != ComWfont)
	  Get_inverseGC(newcbw);
	if (XtCBField(newcbw,highlightThickness) != ComWhighlightThickness)
	  Get_highlightGC(newcbw);
      }
     
    /*  NEED TO RESET PROC AND CLOSURE */

     /* ACTIONS */
    /* Change Label to remove ClearWindow and Redisplay */
    /* Change Label to change GCs if foreground, etc */

    *cbw = *newcbw;
    /**** how to get a redisplay without triggering redisplays
      in superclasses ****/
}
