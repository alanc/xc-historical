#ifndef lint
static char Xrcsid[] = "$XConsortium: Toggle.c,v 1.46 88/11/01 16:57:33 swick Exp $";
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
 * Toggle.c - Toggle button widget
 *
 * Author: Chris D. Peterson
 *         MIT X Consortium / Project Athena
 *         kit@athena.mit.edu
 *  
 * Date:   January 12, 1988
 *
 * NOTE: Internal radio groups are called radio lists, Sorry for
 *       any confusion that is caused by this, but it didn't seem
 *       worth changing all of the internal references.
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <X11/XawMisc.h>
#include <X11/ToggleP.h>

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

/* This is a hack, see the comments in ClassInit(). */

extern XtActionList xt_command_actions_list;

/* 
 * The order of toggle and notify are important, as the state has
 * to be set when we call the notify proc.
 */

static char defaultTranslations[] =
    "<EnterWindow>:	    highlight(always)	\n\
     <LeaveWindow>:	    unhighlight()	\n\
     <Btn1Down>,<Btn1Up>:   toggle() notify()";

#define offset(field) XtOffset(ToggleWidget, field)

static XtResource resources[] = { 
   {XtNstate, XtCState, XtRBoolean, sizeof(Boolean), 
      offset(toggle.state), XtRString, "off"},
   {XtNradioGroup, XtCWidget, XtRWidget, sizeof(Widget), 
      offset(toggle.widget), XtRWidget, (caddr_t) NULL },
   {XtNradioData, XtCRadioData, XtRPointer, sizeof(caddr_t), 
      offset(toggle.radio_data), XtRPointer, (caddr_t) NULL },
};

#undef offset

/* Action proceedures retrieved from the command widget */

static XtActionProc Set, Unset;

static void Toggle(), Initialize(), Notify(), ToggleSet();
static void ToggleDestroy(), ClassInit();
static Boolean SetValues();

/* Functions for handling the Radio List. */

static RadioList * GetRadioList();
static void CreateRadioList(), AddToRadioList(), TurnOffRadioSiblings();
static void RemoveFromRadioList();

static XtActionsRec actionsList[] =
{
  {"toggle",	        Toggle},
  {"notify",	        Notify},
  {"set",	        ToggleSet},
};

#define SuperClass ((CommandWidgetClass)&commandClassRec)

ToggleClassRec toggleClassRec = {
  {
    (WidgetClass) SuperClass,		/* superclass		  */	
    "Toggle",				/* class_name		  */
    sizeof(ToggleRec),			/* size			  */
    ClassInit,				/* class_initialize	  */
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
    NULL,         			/* destroy		  */
    XtInheritResize,			/* resize		  */
    XtInheritExpose,			/* expose		  */
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
  },  /* CommmanClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* ToggleClass fields initialization */
};

  /* for public consumption */
WidgetClass toggleWidgetClass = (WidgetClass) &toggleClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void
ClassInit()
{
  XtActionList actions;
  Cardinal i;
  Set = Unset = NULL;

/* actions = SuperClass->core_class.actions; */

/* The actions table should really be retrieved from the toggle widget's
 * Superclass, but this information is munged by the R3 intrinsics so the
 * I have hacked the Athena command widget to export its action table
 * as a global variable.
 *
 * Chris D. Peterson 12/28/88.
 */

   actions = xt_command_actions_list;

/* 
 * Find the set and unset actions in the command widget's action table. 
 */

  for (i = 0 ; i < SuperClass->core_class.num_actions ; i++) {
    if (streq(actions[i].string, "set")) Set = actions[i].proc;
    if (streq(actions[i].string, "unset")) Unset = actions[i].proc;
    if ( (Set != NULL) && (Unset != NULL) ) return;
  }  

/* We should never get here. */
  if (Set == NULL)
    XtWarning(
     "Toggle could not find action Proceedure Set() in the Command Widget.");
  if (Unset == NULL)
    XtWarning(
     "Toggle could not find action Proceedure Unset() in the Command Widget.");
  XtError("Aborting, due to errors in Toggle widget.");
}

/* ARGSUSED */
static void Initialize(request, new)
 Widget request, new;
{
    ToggleWidget tw = (ToggleWidget) new;

    tw->toggle.radio_list = NULL;

    if (tw->toggle.radio_data == NULL) 
      tw->toggle.radio_data = (caddr_t) new->core.name;

    if (tw->toggle.widget != NULL) {
      if ( GetRadioList(tw->toggle.widget) == NULL) 
	CreateRadioList(new, tw->toggle.widget);
      else
	AddToRadioList( GetRadioList(tw->toggle.widget), new);
    }      
    XtAddCallback(new, XtNdestroyCallback, ToggleDestroy, NULL);

/*
 * Command widget assumes that the widget is unset, so we only 
 * have to handle the case where it needs to be set.
 *
 * If this widget is in a radio list then it may cause another
 * widget to be unset, thus calling the notify proceedure.
 */

    if (tw->command.set)
      ToggleSet(new, NULL, NULL, 0);
}

/************************************************************
 *
 *  Action Procedures
 *
 ************************************************************/

/* ARGSUSED */
static void 
ToggleSet(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  TurnOffRadioSiblings(w);
  Set(w, event, NULL, 0);
}

/* ARGSUSED */
static void 
Toggle(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  ToggleWidget tw = (ToggleWidget)w;

  if (tw->command.set) 
    Unset(w, event, NULL, 0);
  else 
    ToggleSet(w, event, params, num_params);
}

/* ARGSUSED */
static void Notify(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  ToggleWidget tw = (ToggleWidget) w;
  XtCallCallbacks(w, XtNcallback, tw->command.set);
}

/************************************************************
 *
 * Set specified arguments into widget
 *
 ***********************************************************/

/* ARGSUSED */
static Boolean 
SetValues (current, request, new)
Widget current, request, new;
{
    ToggleWidget oldtw = (ToggleWidget) current;
    ToggleWidget tw = (ToggleWidget) new;

    if (oldtw->toggle.widget != tw->toggle.widget)
      XtToggleChangeRadioGroup(new, tw->toggle.widget);

    if (oldtw->toggle.state != tw->toggle.state) {
      if (tw->toggle.state) {
	ToggleSet(new, NULL, NULL, 0); /* Does a redisplay. */
      }
      else
	Unset(new, NULL, NULL, 0); /* Does a redisplay. */
    }
    return(FALSE);
}

/*	Function Name: ToggleDestroy
 *	Description: Destroy Callback for toggle widget.
 *	Arguments: w - the toggle widget that is being destroyed.
 *                 junk, grabage - not used.
 *	Returns: none.
 */

/* ARGSUSED */
static void
ToggleDestroy(w, junk, garbage)
Widget w;
caddr_t junk, garbage;
{
  RemoveFromRadioList(w);
}

/************************************************************
 *
 * Below are all the private proceedures that handle 
 * radio toggle buttons.
 *
 ************************************************************/

/*	Function Name: GetRadioList
 *	Description: Gets the radio list associated with a give toggle
 *                   widget.
 *	Arguments: w - the toggle widget who's radio list we are getting.
 *	Returns: the radio list associated with this toggle list.
 */

static RadioList *
GetRadioList(w)
Widget w;
{
  ToggleWidget tw = (ToggleWidget) w;

  if (tw == NULL) return(NULL);
  return( tw->toggle.radio_list );
}

/*	Function Name: CreateRadioList
 *	Description: Creates a radio list. give two widgets.
 *	Arguments: w1, w2 - the toggle widgets to add to the radio list.
 *	Returns: none.
 * 
 *      NOTE:  A pointer to the list is added to each widget's radio_list
 *             field.
 */

static void
CreateRadioList(w1, w2)
Widget w1, w2;
{
  char error_buf[BUFSIZ];
  ToggleWidget tw1 = (ToggleWidget) w1;
  ToggleWidget tw2 = (ToggleWidget) w2;

  if ( (tw1->toggle.radio_list != NULL) || (tw2->toggle.radio_list != NULL) ) {
    sprintf(error_buf, "%s %s", "Toggle Widget Error - Attempting",
	    "to create a new toggle list, when one already exists.");
    XtWarning(error_buf);
  }

  AddToRadioList( NULL, w1 );
  AddToRadioList( GetRadioList(w1), w2 );
}

/*	Function Name: AddToRadioList
 *	Description: Adds a toggle to the radio list.
 *	Arguments: list - any element of the radio list the we are adding to.
 *                 w - the new toggle widget to add to the list.
 *	Returns: none.
 */

static void
AddToRadioList(list, w)
RadioList * list;
Widget w;
{
  ToggleWidget tw = (ToggleWidget) w;
  RadioList * local;

  local = (RadioList *) XtMalloc( sizeof(RadioList) );
  local->widget = w;
  tw->toggle.radio_list = local;

  if (list == NULL) {		/* Creating new list. */
    list = local;
    list->next = NULL;
    list->prev = NULL;
    return;
  }
  local->prev = list;		/* Adding to previous list. */
  local->next = list->next;
  list->next = local;
}

/*	Function Name: TurnOffRadioSiblings
 *	Description: Deactivates all radio siblings.
 *	Arguments: widget - a toggle widget.
 *	Returns: none.
 */

static void
TurnOffRadioSiblings(w)
Widget w;
{
  RadioList * list;

  if ( (list = GetRadioList(w)) == NULL)  /* Punt if there is no list */
    return;

  /* Go to the top of the list. */

  for ( ; list->prev != NULL ; list = list->prev );

  while ( list != NULL ) {
    ToggleWidget local_tog = (ToggleWidget) list->widget;
    if ( local_tog->command.set ) {
      Unset(list->widget, NULL, NULL, 0);
      Notify( list->widget, NULL, NULL, 0);
    }
    list = list->next;
  }
}

/*	Function Name: RemoveFromRadioList
 *	Description: Removes a toggle from a RadioList.
 *	Arguments: w - the toggle widget to remove.
 *	Returns: none.
 */

static void
RemoveFromRadioList(w)
Widget w;
{
  RadioList * list = GetRadioList(w);
  if (list != NULL) {
    if (list->prev != NULL)
      (list->prev)->next = list->next;
    if (list->next != NULL)
      (list->next)->prev = list->prev;
    XtFree(list);
  }
}

/************************************************************
 *
 * Public Routines
 *
 ************************************************************/
   
/*	Function Name: XtToggleChangeRadioGroup
 *	Description: Allows a toggle widget to change radio lists.
 *	Arguments: w - The toggle widget to change lists.
 *                 radio_group - any widget in the new list.
 *	Returns: none.
 */

void
XtToggleChangeRadioGroup(w, radio_group)
Widget w, radio_group;
{
  ToggleWidget tw = (ToggleWidget) w;

  RemoveFromRadioList(w);

/*
 * If the toggle that we are about to add is set then we will 
 * unset all toggles in the new radio group.
 */

  if ( tw->command.set && radio_group != NULL )
    XtToggleUnsetCurrent(radio_group);
  AddToRadioList( GetRadioList(radio_group), w );
}

/*	Function Name: XtToggleGetCurrent
 *	Description: Returns the RadioData associated with the toggle
 *                   widget that is currently active in a toggle list.
 *	Arguments: w - any toggle widget in the toggle list.
 *	Returns: The XtNradioData associated with the toggle widget.
 */

caddr_t
XtToggleGetCurrent(w)
Widget w;
{
  RadioList * list;

  if ( (list = GetRadioList(w)) == NULL) return(NULL);
  for ( ; list->prev != NULL ; list = list->prev);

  while ( list != NULL ) {
    ToggleWidget local_tog = (ToggleWidget) list->widget;
    if ( local_tog->command.set )
      return( local_tog->toggle.radio_data );
    list = list->next;
  }
  return(NULL);
}

/*	Function Name: XtToggleSetCurrent
 *	Description: Sets the Toggle widget associated with the
 *                   radio_data specified.
 *	Arguments: radio_group - any toggle widget in the toggle list.
 *                 radio_data - radio data of the toggle widget to set.
 *	Returns: none.
 */

void
XtToggleSetCurrent(radio_group, radio_data)
Widget radio_group;
caddr_t radio_data;
{
  RadioList * list;
  ToggleWidget local_tog; 

/* Special case case of no radio list. */

  if ( (list = GetRadioList(radio_group)) == NULL) {
    local_tog = (ToggleWidget) radio_group;
    if ( (local_tog->toggle.radio_data == radio_data) )     
      if (!local_tog->command.set) {
	ToggleSet(radio_group, NULL, NULL, 0);
	Notify(radio_group, NULL, NULL, 0);
      }
    return;
  }

/*
 * find top of radio_roup 
 */

  for ( ; list->prev != NULL ; list = list->prev);

/*
 * search for matching radio data.
 */

  while ( list != NULL ) {
    local_tog = (ToggleWidget) list->widget;
    if ( (local_tog->toggle.radio_data == radio_data) ) {
      if (!local_tog->command.set) { /* if not already set. */
	ToggleSet(radio_group, NULL, NULL, 0);
	Notify(radio_group, NULL, NULL, 0);
      }
      return;			/* found it, done */
    }
    list = list->next;
  }
}
 
/*	Function Name: XtToggleUnsetCurrent
 *	Description: Unsets all Toggles in the radio_group specified.
 *	Arguments: radio_group - any toggle widget in the toggle list.
 *	Returns: none.
 */

void
XtToggleUnsetCurrent(radio_group)
Widget radio_group;
{
  /* Special Case no radio list. */

  ToggleWidget local_tog = (ToggleWidget) radio_group;
  if (local_tog->command.set) {
    Unset(radio_group, NULL, NULL, 0);
    Notify(radio_group, NULL, NULL, 0);
  }
  if ( GetRadioList(radio_group) == NULL) return;
  TurnOffRadioSiblings(radio_group);
}
