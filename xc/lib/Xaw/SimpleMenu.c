#if ( !defined(lint) && !defined(SABER) )
static char Xrcsid[] = "$XConsortium: SimpleMenu.c,v 1.4 89/05/02 21:09:03 kit Exp $";
#endif 

/***********************************************************
Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

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
 * SimpleMenu.c - Source code file for SimpleMenu widget.
 *
 * Date:    April 3, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium / Project Athena.
 *          kit@athena.mit.edu
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/SimpleMenuP.h>
#include <X11/Xmu.h>

#define streq(a, b)        ( strcmp((a), (b)) == 0 )

#define offset(field) XtOffset(SimpleMenuWidget, simple_menu.field)

static XtResource menu_resources[] = { 
  {XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
     offset(cursor), XtRImmediate, (caddr_t) None},
  {XtNlabel,  XtCLabel, XtRString, sizeof(String),
     offset(label), XtRString, NULL},
  {XtNrowHeight,  XtCRowHeight, XtRDimension, sizeof(Dimension),
     offset(row_height), XtRImmediate, (caddr_t) 0},
  {XtNcolumnWidth,  XtCColumnWidth, XtRDimension, sizeof(Dimension),
     offset(column_width), XtRImmediate, (caddr_t) 0},
  {XtNresize,  XtCResize, XtRBoolean, sizeof(Boolean),
     offset(auto_resize), XtRImmediate, (caddr_t) TRUE},
  {XtNverticalSpace,  XtCVerticalSpace, XtRDimension, sizeof(Dimension),
     offset(vertical_space), XtRImmediate, (caddr_t) 4},
  {XtNtopMargin,  XtCVerticalMargins, XtRDimension, sizeof(Dimension),
     offset(top_margin), XtRImmediate, (caddr_t) MAGIC_DIMENSION},
  {XtNbottomMargin,  XtCVerticalMargins, XtRDimension, sizeof(Dimension),
     offset(bottom_margin), XtRImmediate, (caddr_t) MAGIC_DIMENSION},
  {XtNleftMargin,  XtCHorizontalMargins, XtRDimension, sizeof(Dimension),
     offset(left_margin), XtRImmediate, (caddr_t) 4},
  {XtNrightMargin,  XtCHorizontalMargins, XtRDimension, sizeof(Dimension),
     offset(right_margin), XtRImmediate, (caddr_t) 4},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     offset(foreground), XtRString, "XtDefaultForeground"},
  {XtNfont,  XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     offset(font),XtRString, "XtDefaultFont"},
};  
#undef offset

#define offset(field) XtOffset(MenuEntry *, field)
static XtResource entry_resources[] = {
  {XtNlabel,  XtCLabel, XtRString, sizeof(String),
     offset(label), XtRString, NULL},
  {XtNsensitive, XtCSensitive, XtRBoolean, sizeof(Boolean),
     offset(sensitive), XtRImmediate, (caddr_t) True},
  {XtNleftBitmap, XtCLeftBitmap, XtRPixmap, sizeof(Pixmap),
     offset(left_bitmap), XtRImmediate, (caddr_t)None},
  {XtNrightBitmap, XtCRightBitmap, XtRPixmap, sizeof(Pixmap),
     offset(right_bitmap), XtRImmediate, (caddr_t)None},
  {XtNtype, XtCType, XtRSimpleMenuType, sizeof(XawMenuEntryType),
     offset(type), XtRImmediate, (caddr_t) XawMenuText},
  {XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t),
     offset(callbacks), XtRCallback, (caddr_t)NULL},
};   
#undef offset

static char defaultTranslations[] =
    "<EnterWindow>:     highlight()             \n\
     <LeaveWindow>:     unhighlight()           \n\
     <BtnMotion>:       highlight()             \n\
     <BtnUp>:           notify() MenuPopdown() unhighlight()"; 

#define superclass (&overrideShellClassRec)

/*
 * Semi Public function definitions. 
 */

static void Redisplay(), Realize(), Resize(), Destroy(), Scream();
static void Initialize(), ClassInitialize();
static Boolean SetValues(), SetValuesHook();

/*
 * Action Routine Definitions
 */

static void Highlight(), Unhighlight(), Notify();

/* 
 * Private Function Definitions.
 */

static void CalculateNewSize(), RemoveEntry(), DrawBitmaps(), FlipColors();
static void MakeSetValuesRequest(), RefreshEntry(), DestroyEntry();
static void SetEntryInfo(), ChangeEntryInfo(), CreateGCs(), DestroyGCs();
static Dimension GetMenuWidth(), GetMenuHeight();
static MenuEntry * GetEventEntry(), * GetMenuEntry();
static void XawCvtStringToMenuType(), MaybeCopyCallbacks();

static XtActionsRec actionsList[] =
{
  {"notify",            Notify},
  {"highlight",         Highlight},
  {"unhighlight",       Unhighlight},
};

SimpleMenuClassRec simpleMenuClassRec = {
  {
    /* superclass         */    (WidgetClass) superclass,
    /* class_name         */    "SimpleMenu",
    /* size               */    sizeof(SimpleMenuRec),
    /* Class Initializer  */	ClassInitialize,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    Realize,
    /* actions            */    actionsList,
    /* num_actions        */    XtNumber(actionsList),
    /* resources          */    menu_resources,
    /* resource_count     */	XtNumber(menu_resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE, 
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/ 	TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    Destroy,
    /* resize             */    Resize,
    /* expose             */    Redisplay,
    /* set_values         */    SetValues,
    /* set_values_hook    */	SetValuesHook,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    defaultTranslations,
    /* query_geometry	  */    NULL,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* geometry_manager   */    (XtGeometryHandler) Scream,
    /* change_managed     */    (XtWidgetProc) Scream,
    /* insert_child	  */	(XtWidgetProc) Scream,
    /* delete_child	  */	(XtWidgetProc) Scream,
    /* extension	  */    NULL
  },{
    /* Shell extension	  */    NULL
  },{
    /* Override extension */    NULL
  },{
    /* Simple Menu extension*/  NULL
  }
};

WidgetClass simpleMenuWidgetClass = (WidgetClass)&simpleMenuClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/*      Function Name: ClassInitialize
 *      Description: Class Initialize routine, called only once.
 *      Arguments: none.
 *      Returns: none.
 */

static void
ClassInitialize()
{
  XtAddConverter( XtRString, XtRSimpleMenuType, XawCvtStringToMenuType,
		 NULL, (Cardinal) 0 );
}

/*      Function Name: Scream
 *      Description:   Complain to the programmer who tries to add children
 *                     to a menu.
 *      Arguments:     w - the menu widget.
 *                     *** All others are ignored ****
 *      Returns:       none.
 */

static void
Scream(w)
Widget w;
{
  XtAppWarning(XtWidgetToApplicationContext(w),
	       "This widget is not allowed to have children.");
}

/*      Function Name: Initialize
 *      Description: Initializes the simple menu widget
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 */

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) new;

/*
 * If top or bottom margins are zero then use vertical_space.
 */

  if (smw->simple_menu.top_margin == MAGIC_DIMENSION)
    smw->simple_menu.top_margin = smw->simple_menu.vertical_space;

  if (smw->simple_menu.bottom_margin == MAGIC_DIMENSION)
    smw->simple_menu.bottom_margin = smw->simple_menu.vertical_space;

  smw->simple_menu.entry_set = NO_ENTRY;
  smw->simple_menu.entries = NULL;
  smw->simple_menu.recursive_set_values = FALSE;
  smw->simple_menu.num_entries = 
                       (Cardinal) ( (smw->simple_menu.label == NULL) ? 0 : 1 );
}

/*      Function Name: Destroy
 *      Description: Called at destroy time, cleans up.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
Destroy(w)
Widget w;
{
  DestroyGCs(w);
}

/*      Function Name: Redisplay
 *      Description: Redisplays the contents of the widget.
 *      Arguments: w - the simple menu widget.
 *                 event - the X event that caused this redisplay.
 *                 region - the region the needs to be repainted. 
 *      Returns: none.
 */

/* ARGSUSED */
static void
Redisplay(w, event, region)
Widget w;
XEvent * event;
Region region;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  GC gc;
  MenuEntry * entry;
  int i;
  Dimension y = smw->simple_menu.top_margin;
  Dimension width = w->core.width;
  Dimension y_temp;

/*
 * Check to see if we should paint label.
 */

  y_temp = y;

  if (smw->simple_menu.label != NULL ) {
    switch(XRectInRegion(region, 0, y_temp, width,
			 smw->simple_menu.row_height)) {
    case RectangleIn:
    case RectanglePart:

      if (XtIsSensitive(w)) {
	XFillRectangle(XtDisplay(w), XtWindow(w), smw->simple_menu.norm_gc,
		       0, y_temp, width, smw->simple_menu.row_height);
	gc = smw->simple_menu.rev_gc;
      }
      else
	gc = smw->simple_menu.norm_grey_gc;

/*
 * Trust me on this one :-) 
 */
      y_temp += (smw->simple_menu.row_height -
		 smw->simple_menu.font->max_bounds.descent +
		 smw->simple_menu.font->max_bounds.ascent)/2;

      XDrawString(XtDisplay(w), XtWindow(w), gc,
		  smw->simple_menu.left_margin, y_temp,
		  smw->simple_menu.label, strlen(smw->simple_menu.label));
      break;
    default:
      break;
    }
    y += smw->simple_menu.row_height + smw->simple_menu.vertical_space;
  }

 /*
 * Check each of the entries.
 */

  for (i = 0, entry = smw->simple_menu.entries; entry != NULL;
       entry = entry->next, i++) {
    Dimension height = (smw->simple_menu.row_height + 
			smw->simple_menu.vertical_space);

    if (entry->type != XawMenuBlank) {
      switch(XRectInRegion(region, 0, y, width, height)) {
	      
      case RectangleIn:
      case RectanglePart:
	y_temp = y;
	switch (entry->type) {
	case XawMenuSeparator:
	  y_temp += smw->simple_menu.row_height/2; 
	  XDrawLine(XtDisplay(w), XtWindow(w), smw->simple_menu.norm_gc,
		    0, y_temp, smw->core.width, y_temp);
	  break;
	  
	case XawMenuText:
	  if (entry->sensitive && XtIsSensitive(w) ) {
	    if (i == smw->simple_menu.entry_set) {
	      XFillRectangle(XtDisplay(w), XtWindow(w), 
			     smw->simple_menu.norm_gc, 0, y_temp,
			     width, smw->simple_menu.row_height);
	      gc = smw->simple_menu.rev_gc;
	    }
	    else
	      gc = smw->simple_menu.norm_gc;
	  }
	  else
	    gc = smw->simple_menu.norm_grey_gc;

/*
 * Trust me on this one :-) 
 */
	  y_temp += (smw->simple_menu.row_height -
		     smw->simple_menu.font->max_bounds.descent +
		     smw->simple_menu.font->max_bounds.ascent)/2;

	  XDrawString(XtDisplay(w), XtWindow(w), gc,
		      smw->simple_menu.left_margin, y_temp,
		      entry->label, strlen(entry->label));

	  DrawBitmaps(w, gc, entry, y);
	default:			/* falling through... */
	  break;
	} /* switch (entry->type) */
      
      default:			/* falling through... */
	break;
      } /* switch(XRectInRegion(region, 0, y, width, height)) */
    } /* if (entry->type != XawMenuBlank) */

    y += height;
  }  
}

/*      Function Name: Realize
 *      Description: Realizes the widget.
 *      Arguments: w - the simple menu widget.
 *                 mask - value mask for the window to create.
 *                 attrs - attributes for the window to create.
 *      Returns: none
 */

static void
Realize(w, mask, attrs)
Widget w;
XtValueMask * mask;
XSetWindowAttributes * attrs;
{
  CreateGCs(w);
  CalculateNewSize(w);

  attrs->cursor = ((SimpleMenuWidget)w)->simple_menu.cursor;
  *mask |= CWCursor;
  (*superclass->core_class.realize) (w, mask, attrs);
}

/*      Function Name: Resize
 *      Description: Handle the menu being resized bigger.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
Resize(w)
Widget w;
{
  RefreshEntry(w, NULL, XawErefreshAll);
}

/*      Function Name: SetValues
 *      Description: Relayout the menu when one of the resources is changed.
 *      Arguments: current - current state of the widget.
 *                 request - what was requested.
 *                 new - what the widget will become.
 *      Returns: none
 */

/* ARGSUSED */
static Boolean
SetValues(current, request, new)
Widget current, request, new;
{
  SimpleMenuWidget smw_old = (SimpleMenuWidget) current;
  SimpleMenuWidget smw_new = (SimpleMenuWidget) new;
  Boolean ret_val = FALSE;
  
  if (!XtIsRealized(current)) return(FALSE);

  if (smw_old->simple_menu.cursor != smw_new->simple_menu.cursor)
    XDefineCursor(XtDisplay(new), XtWindow(new), smw_new->simple_menu.cursor);

  if (smw_old->simple_menu.font != smw_new->simple_menu.font) {
    DestroyGCs(current);
    CreateGCs(new);
    CalculateNewSize(new);
    return(TRUE);		/* full relayout, might as well return. */
  }
  
  if (smw_old->simple_menu.foreground != smw_new->simple_menu.foreground) {
    DestroyGCs(current);
    CreateGCs(new);
    ret_val = TRUE;
  }
  
  if (smw_old->simple_menu.label != smw_new->simple_menu.label) {
    CalculateNewSize(new);
    RefreshEntry(new, NULL, XawErefreshLabel);
  }

  if ((smw_old->simple_menu.left_margin != smw_new->simple_menu.left_margin) ||
      (smw_old->simple_menu.right_margin!=smw_new->simple_menu.right_margin) ||
      (smw_old->simple_menu.top_margin != smw_new->simple_menu.top_margin)   ||
      (smw_old->simple_menu.vertical_space != 
       smw_new->simple_menu.vertical_space)                                  ||
      (smw_old->simple_menu.bottom_margin != 
       smw_new->simple_menu.bottom_margin) /* filler.....................*/ ) {
    CalculateNewSize(new);
    ret_val = TRUE;
  }
  return(ret_val);
}

/*      Function Name: SetValuesHook
 *      Description: To handle a special case, this is passed the
 *                   actual arguments.
 *      Arguments: w - the menu widget.
 *                 arglist - the argument list passed to XtSetValues.
 *                 num_args - the number of args.
 *      Returns: none
 */

/* 
 * If the user actually passed a width and height to the widget
 * then this MUST be used, rather than out newly calculated width and
 * height.
 */

static Boolean
SetValuesHook(w, arglist, num_args)
Widget w;
ArgList arglist;
Cardinal *num_args;
{
  Cardinal i;
  Dimension width, height;

  width = w->core.width;
  height = w->core.height;

  for ( i = 0 ; i < *num_args ; i++) {
    if ( streq(arglist[i].name, XtNwidth) )
      width = (Dimension) arglist[i].value;
    if ( streq(arglist[i].name, XtNheight) )
      height = (Dimension) arglist[i].value;
  }
  MakeSetValuesRequest(w, width, height);
  if ((width != w->core.width) || (height != w->core.height)) 
    return(TRUE);
  return(FALSE);
}

/************************************************************
 *
 * Action Routines.
 * 
 ************************************************************/

/*      Function Name: Unhighlight
 *      Description: Unhighlights current entry.
 *      Arguments: w - the simple menu widget.
 *                 event - the event that caused this action.
 *                 params, num_params - ** NOT USED **
 *      Returns: none
 */

/* ARGSUSED */
static void
Unhighlight(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{ 
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  register MenuEntry * entry;
  register int i;
 
  if (smw->simple_menu.entry_set == NO_ENTRY) return;

  for ( i = 0, entry = smw->simple_menu.entries ; 
       ( (entry != NULL) && ( i != smw->simple_menu.entry_set) );
       entry = entry->next, i++) {}

  if (entry == NULL)
    XtAppError( XtWidgetToApplicationContext(w), 
       "SimpleMenuWidget: Could not find entry in Unhighlight action Proc.");


  smw->simple_menu.entry_set = NO_ENTRY;
  if (entry->type == XawMenuText) 
    FlipColors(w, entry);
}

/*      Function Name: Highlight
 *      Description: Highlights current entry.
 *      Arguments: w - the simple menu widget.
 *                 event - the event that caused this action.
 *                 params, num_params - ** NOT USED **
 *      Returns: none
 */

/* ARGSUSED */
static void
Highlight(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  register MenuEntry * entry, * event_entry;
  register int i;
  
  if ( (!XtIsSensitive(w)) ||
      ((event_entry = GetEventEntry(w, event)) == NULL) ) {
    Unhighlight(w, event, params, num_params);    
    return;
  }

  for ( i = 0, entry = smw->simple_menu.entries ; 
       ( (entry != NULL) && (entry != event_entry) ) ;
       entry = entry->next, i++) {}

  if (i == smw->simple_menu.entry_set) return;

  Unhighlight(w, event, params, num_params);  

  if ( (event_entry == NULL) || 
       !(event_entry->sensitive) || (event_entry->type != XawMenuText) ) 
    return;

  smw->simple_menu.entry_set = i;
  FlipColors(w, entry);
}

/*      Function Name: Notify
 *      Description: Notify user of current entry.
 *      Arguments: w - the simple menu widget.
 *                 event - the event that caused this action.
 *                 params, num_params - ** NOT USED **
 *      Returns: none
 */

/* ARGSUSED */
static void
Notify(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  register MenuEntry * entry;
  XtCallbackList callbacks;
  register int i;

  if (smw->simple_menu.entry_set == NO_ENTRY) return;
  
  for ( i = 0, entry = smw->simple_menu.entries ; 
       ( (entry != NULL) && ( i != smw->simple_menu.entry_set) );
       entry = entry->next, i++) {}

  if ( (entry == NULL) ||
       (entry->callbacks == NULL) ||
       (entry->type != XawMenuText) ) return;

/*
 * Call all callback proceedures.
 */

  for (callbacks = entry->callbacks ; callbacks->callback != NULL
       ; callbacks++)
    (callbacks->callback) (w, callbacks->closure, NULL);
}

/************************************************************
 *
 * Public Functions.
 *
 ************************************************************/
 
/*      Function Name: XawSimpleMenuAddEntryCallback
 *      Description: Adds an entry to the callback list.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *                 proc - the callback proceedure.
 *                 data - the callback (client) data.
 *      Returns: none.
 */

void
XawSimpleMenuAddEntryCallback(w, name, proc, data)
Widget w;
char * name;
XtCallbackProc proc;
caddr_t data;
{
  MenuEntry * entry = GetMenuEntry(w, name);
  XtCallbackList callbacks;
  int number;

  if (entry->callbacks == NULL)
    number = 1;
  else {
    for (number = 0 ; entry->callbacks[number].callback != NULL ; number++) {}
    number++;
  }

  number++;
  callbacks = (XtCallbackList) XtCalloc(number, sizeof(XtCallbackRec));
  bcopy(entry->callbacks, callbacks, sizeof(XtCallbackRec) * (number - 2) );
  callbacks[number - 2].callback = proc;
  callbacks[number - 2].closure = data;
  XtFree(entry->callbacks);
  entry->callbacks = callbacks;
}
  
/*      Function Name: XawSimpleMenuAddEntry
 *      Description: Adds an item to the menu.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *                 args - the argument list.
 *                 num_args -  number of arguments.
 *      Returns: none.
 */

void
XawSimpleMenuAddEntry(w, name , args, num_args)
Widget w;
char * name;
ArgList args;
Cardinal num_args;
{
  MenuEntry * entry;

  entry = GetMenuEntry(w, NULL);
  entry->name = XtNewString(name);
  SetEntryInfo(w, entry, args, num_args);
  if (XtIsRealized(w))
    CalculateNewSize(w);
  RefreshEntry(w, NULL, XawErefreshAll);
}
  
/*      Function Name: XawSimpleMenuRemoveEntry
 *      Description: Adds an item to the menu.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *      Returns: none.
 */

void
XawSimpleMenuRemoveEntry(w, name)
Widget w;
char * name;
{
  RemoveEntry(w, name);
  if (XtIsRealized(w))
    CalculateNewSize(w);
  RefreshEntry(w, NULL, XawErefreshAll);
}
  
/*      Function Name: XawSimpleMenuSetEntryValues
 *      Description: Adds an item to the menu.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *                 args - the argument list.
 *                 num_args -  number of arguments.
 *      Returns: none.
 *      NOTE: figureing out whether or not to redisplay is an annoying
 *            amount of code and time.  Since the only resource that would
 *            not cause a redisplay is the callback list, I will just 
 *            always redisplay - CDP 3/22/89.
 */

void
XawSimpleMenuSetEntryValues(w, name , args, num_args)
Widget w;
char * name;
ArgList args;
Cardinal num_args;
{
  MenuEntry * entry;

  entry = GetMenuEntry(w, name);
  ChangeEntryInfo(w, entry, args, num_args);
  if (XtIsRealized(w))
    CalculateNewSize(w);	/* This only changes the width. */
  RefreshEntry(w, name, XawErefreshEntry);
}

  
/*      Function Name: XawSimpleMenuGetEntryValues
 *      Description: gets the current values for an entry.
 *      Arguments: w - the menu widget
 *                 name - name of new menu item.
 *                 args - the argument list. (name/address pairs)
 *                 num_args -  number of arguments.
 *      Returns: none.
 */

void
XawSimpleMenuGetEntryValues(w, name , args, num_args)
Widget w;
char * name;
ArgList args;
Cardinal num_args;
{
  MenuEntry * entry;

  entry = GetMenuEntry(w, name);
  XtGetSubvalues((char *) entry, entry_resources, XtNumber(entry_resources),
		 args, num_args);
}

/************************************************************
 *
 * Private Functions.
 *
 ************************************************************/

/*      Function Name: CalculateNewSize
 *      Description: Finds the new size for the simple menu widget.
 *      Arguments: w - the menu widget.
 *      Returns: none.
 */

static void
CalculateNewSize(w)
Widget w;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;

  if ( (w->core.width == 0) || (w->core.height == 0) ||
       (smw->shell.allow_shell_resize) ) 
    MakeSetValuesRequest(w, GetMenuWidth(w), GetMenuHeight(w));
}

/*      Function Name: MakeSetValuesRequest
 *      Description: Makes a (possibly recursive) call to SetValues,
 *                   I take great pains to not go into an infinite loop.
 *      Arguments: w - the simple menu widget.
 *                 width, height - the size of the ask for.
 *      Returns: none
 */

static void
MakeSetValuesRequest(w, width, height)
Widget w;
Dimension width, height;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  Arg arglist[2];
  Cardinal num_args = (Cardinal) 0;
  
  if ( !smw->simple_menu.recursive_set_values ) {
    smw->simple_menu.recursive_set_values = TRUE;
    XtSetArg(arglist[num_args], XtNwidth, width);   num_args++;
    XtSetArg(arglist[num_args], XtNheight, height); num_args++;
    XtSetValues(w, arglist, num_args);
  }
  smw->simple_menu.recursive_set_values = FALSE;
}

/*      Function Name: GetMenuWidth
 *      Description: Sets the length of the widest entry in pixels.
 *      Arguments: w - the simple menu widget.
 *      Returns: width of menu.
 */

static Dimension
GetMenuWidth(w)
Widget w;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  Dimension width, widest = (Dimension) 0;
  MenuEntry * entry;

  if ( (smw->simple_menu.auto_resize) ||
       (smw->simple_menu.column_width == 0) ) {
    if (smw->simple_menu.label != NULL)
      widest = (Dimension) XTextWidth(smw->simple_menu.font, 
				      smw->simple_menu.label, 
				      strlen(smw->simple_menu.label));
    
    for ( entry = smw->simple_menu.entries ; entry != NULL ;
	 entry = entry->next ) {
      if (entry->type == XawMenuText) {
	width = (Dimension) XTextWidth(smw->simple_menu.font, 
				       entry->label, strlen(entry->label));
	if ( width > widest )
	  widest = width;
      }
    }

    smw->simple_menu.column_width = widest;
  }
  else 
    widest = smw->simple_menu.column_width;

  widest += smw->simple_menu.left_margin + smw->simple_menu.right_margin;
  return(widest);
}

/*      Function Name: GetMenuHeight
 *      Description: Sets the length of the widest entry in pixels.
 *      Arguments: w - the simple menu widget.
 *      Returns: width of menu.
 */

static Dimension
GetMenuHeight(w)
Widget w;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  Dimension height;

  if ( (smw->simple_menu.auto_resize) || (smw->simple_menu.row_height == 0) )
    smw->simple_menu.row_height = (smw->simple_menu.font->max_bounds.ascent +
				   smw->simple_menu.font->max_bounds.descent);

  height = (smw->simple_menu.num_entries - 1)* smw->simple_menu.vertical_space;
  height += smw->simple_menu.num_entries * smw->simple_menu.row_height;
  height += smw->simple_menu.top_margin + smw->simple_menu.bottom_margin;
  return(height);
}

/*      Function Name: FlipColors
 *      Description: Invert the colors of the current entry.
 *      Arguments: w - the menu widget.
 *                 entry - entry to flip.
 *      Returns: none.
 */

static void 
FlipColors(w, entry)
Widget w;
MenuEntry * entry;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  register Position height, y = smw->simple_menu.top_margin;
  register MenuEntry * temp_entry;

  height = (Position) (smw->simple_menu.row_height + 
		       smw->simple_menu.vertical_space);

  if ( !XtIsRealized(w) ) return;

  if (smw->simple_menu.label != NULL)
    y += height;
  for ( temp_entry = smw->simple_menu.entries ; 
        (temp_entry != NULL) && (entry != temp_entry) ;
        temp_entry = temp_entry->next ) 
      y += height;

  if (temp_entry == NULL)
    XtAppError( XtWidgetToApplicationContext(w), 
	       "SimpleMenuWidget: Could not find entry in FlipColors.");

/*  y += (height - smw->simple_menu.font->max_bounds.ascent)/2; */
  XFillRectangle(XtDisplay(w), XtWindow(w), smw->simple_menu.invert_gc,
		 0, y, w->core.width, smw->simple_menu.row_height);
}

/*      Function Name: DrawBitmaps
 *      Description: Draws left and right bitmaps.
 *      Arguments: w - the simple menu widget.
 *                 gc - graphics context to use for drawing.
 *                 entry - entry location for drawing.
 *                 y     - y location of top of the drawing area.
 *      Returns: none
 */


static void
DrawBitmaps(w, gc, entry, y)
Widget w;
GC gc;
MenuEntry * entry;
Position y;
{
  int x_loc, y_loc;
  SimpleMenuWidget smw = (SimpleMenuWidget) w;

  if ( (entry->left_bitmap == None) && (entry->right_bitmap == None))
    return;

  x_loc = (smw->simple_menu.left_margin - entry->left_bitmap_width)/2;
  y_loc = y + (smw->simple_menu.row_height - entry->left_bitmap_height)/2;

/*
 * Draw Left Bitmap.
 */

  if (entry->left_bitmap != None) {
    XCopyPlane(XtDisplay(w), entry->left_bitmap, XtWindow(w), gc,
	       0, 0, entry->left_bitmap_width, entry->left_bitmap_height,
	       x_loc, y_loc, 1);
  }

/*
 * Draw Right Bitmap.
 */

  if (entry->right_bitmap != None) {
    x_loc += smw->simple_menu.column_width + smw->simple_menu.left_margin;
    XCopyPlane(XtDisplay(w), entry->right_bitmap, XtWindow(w), gc,
	       0, 0, entry->right_bitmap_width, entry->right_bitmap_height,
	       x_loc, y_loc, 1);
  }
}

/*      Function Name: GetBitmapInfo
 *      Description: Gets the bitmap information from either of the bitmaps.
 *      Arguments: w - the simple menu widget.
 *                 entry - the entry we are testing.
 *                 is_left - TRUE if we are testing left bitmap,
 *                           FALSE if we are testing the right bitmap.
 *      Returns: none
 */

static void
GetBitmapInfo(w, entry, is_left)
Widget w;
MenuEntry * entry;
Boolean is_left;
{
  unsigned int depth, bw;
  Window root;
  int x, y;

  if (is_left) {
    if (entry->left_bitmap != None) {
      if (!XGetGeometry(XtDisplay(w), entry->left_bitmap, &root, &x, &y,
			&(entry->left_bitmap_width), 
			&(entry->left_bitmap_height), &bw, &depth)) {
	char buf[BUFSIZ];
	sprintf(buf, "SimpleMenu Widget: %s %s \"%s\".", "Could",
		"not get Left Bitmap geometry information for menu entry ",
		entry->name);
	XtAppError(XtWidgetToApplicationContext(w), buf);
      }
      if (depth != 1) {
	char buf[BUFSIZ];
	sprintf(buf, "SimpleMenu Widget: %s \"%s\".", 
		"Left Bitmap of entry ", entry->name, 
		" is not one bit deep.");
	XtAppError(XtWidgetToApplicationContext(w), buf);
      }
    }
  }
  else if (entry->right_bitmap != None) {
    if (!XGetGeometry(XtDisplay(w), entry->right_bitmap, &root, &x, &y,
		      &(entry->right_bitmap_width), 
		      &(entry->right_bitmap_height), &bw, &depth)) {
      char buf[BUFSIZ];
      sprintf(buf, "SimpleMenu Widget: %s \"%s\".", 
	     "Could not get Right Bitmap geometry information for menu entry ",
	      entry->name);
      XtAppError(XtWidgetToApplicationContext(w), buf);
    }
    if (depth != 1) {
      char buf[BUFSIZ];
      sprintf(buf, "SimpleMenu Widget: %s \"%s\".", 
	      "Right Bitmap of entry ", entry->name, 
	      " is not one bit deep.");
      XtAppError(XtWidgetToApplicationContext(w), buf);
    }
  }
}      

/*      Function Name: RefreshEntry.
 *      Description: Refreshes the entry passed to it.
 *      Arguments: w - the simple menu widget.
 *                 name - name of the entry to refresh, or NULL.
 *                 type - if true then refresh label only.
 *      Returns: none.
 *      NOTES:     if name == NULL then refresh the entire menu.
 */

static void
RefreshEntry(w, name, type)
Widget w;
char * name;
XawRefreshTypes type;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  register Position real_row_height, y;
  Dimension height;
  MenuEntry * entry;

  if ( !XtIsRealized(w) ) return;

  y = smw->simple_menu.top_margin;

  switch (type) {
  case XawErefreshAll:
    height = w->core.height;
    break;
  case XawErefreshEntry:
    real_row_height = (Position) (smw->simple_menu.row_height + 
				  smw->simple_menu.vertical_space);

    if (smw->simple_menu.label != NULL)
      y += real_row_height;		/* Leave space for label. */
    for ( entry = smw->simple_menu.entries ; entry != NULL ;
	 entry = entry->next ) {
      if (streq(name, entry->name)) break;
      y += real_row_height;
    }
  case XawErefreshLabel:	/* fall through. */
    height = smw->simple_menu.row_height + smw->simple_menu.vertical_space;
    break;
  default:
    XtAppError( XtWidgetToApplicationContext(w), "Unknow refresh type.");
  }

/*
 * Clear the area and generate an exposure event.
 */

  XClearArea(XtDisplay(w), XtWindow(w), 0, y,
	     w->core.width, height, TRUE);
}

/*      Function Name: GetMenuEntry
 *      Description: Gets the entry specified by name.
 *      Arguments: w - the simple menu widget.
 *                 name - the name of the menu entry to get, or NULL
 *      Returns: a pointer to the entry.
 *      NOTES:   if name == NULL then return an empty allocated entry, that
 *               is attached to the end of the list.
 */

static MenuEntry *
GetMenuEntry(w, name)
Widget w;
char * name;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  MenuEntry * entry, *new_entry;
  char buf[BUFSIZ];

  if (name == NULL) {
    new_entry = (MenuEntry *) XtMalloc( sizeof(MenuEntry));
    new_entry->next = NULL;	/* always add to the end. */
    if (smw->simple_menu.entries == NULL)
      smw->simple_menu.entries = new_entry;
    else {
      for ( entry = smw->simple_menu.entries ; entry->next != NULL ;
	   entry = entry->next ) {}
      entry->next = new_entry;
    }
    smw->simple_menu.num_entries++;
    return(new_entry);
  }

  for ( entry = smw->simple_menu.entries ; entry != NULL ;
        entry = entry->next )
    if (streq(name, entry->name)) return(entry);

/*
 * Could not find name.
 */

  sprintf(buf, "Could not find the menu entry %s in the menu %s",
	  name, w->core.name);
  XtAppError(XtWidgetToApplicationContext(w), buf);
  return(NULL);		       /* Can never get here, but keeps Saber happy. */
}

/*      Function Name: RemoveEntry
 *      Description: removes and entry from the menu.
 *      Arguments: w - the simple menu widget.
 *                 name - name of the menu entry.
 *      Returns: none.
 */

static void
RemoveEntry(w, name)
Widget w;
char * name;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  MenuEntry * entry, * prev;
  char buf[BUFSIZ];

  prev = NULL;
  for ( entry = smw->simple_menu.entries ; entry != NULL ;
        entry = entry->next ) {
    if ( streq(entry->name, name) ) {
      if (prev == NULL)
	smw->simple_menu.entries = entry->next;
      else
	prev->next = entry->next;
      DestroyEntry(entry);
      smw->simple_menu.num_entries--;
      return;
    }
    prev = entry;
  }

  sprintf(buf, "Could not find the menu entry %s in the menu %s",
	  name, w->core.name);
  XtAppError(XtWidgetToApplicationContext(w), buf);
}

/*      Function Name: DestroyEntry
 *      Description: Blows away the entry data, freeing all resources.
 *      Arguments: entry - then entry to destroy.
 *      Returns: none.
 */

static void
DestroyEntry(entry)
MenuEntry * entry;
{
  XtFree(entry->name);
  if (entry->callbacks != NULL)
    XtFree(entry->callbacks);
  XtFree(entry);
}

/*      Function Name:  SetEntryInfo
 *      Description: Sets the info in the entry
 *      Arguments: w - the menu widget.
 *                 entry - the entry struct for this menu
 *                 args, num_args - the arglist and number.
 *      Returns: none.
 */

static void
SetEntryInfo(w, entry, args, num_args)
Widget w;
MenuEntry * entry;
ArgList args;
Cardinal num_args;
{
  XtGetSubresources(w, (char *) entry,
		    entry->name, XAW_MENU_ENTRY, entry_resources, 
		    XtNumber(entry_resources), args, num_args);

  if (entry->label == NULL)
    entry->label = entry->name;

  MaybeCopyCallbacks(entry);
  GetBitmapInfo(w, entry, TRUE); /* Get info on left and right bitmaps. */
  GetBitmapInfo(w, entry, FALSE);
}

/*      Function Name: ChangeEntryInfo
 *      Description: Changes the info in the entry.
 *      Arguments: w - the menu widget.
 *                 entry - the entry struct for this menu
 *                 args, num_args - the arglist and number.
 *      Returns: none.
 */

static void
ChangeEntryInfo(w, entry, args, num_args)
Widget w;
MenuEntry * entry;
ArgList args;
Cardinal num_args;
{
  MenuEntry old_entry;

  bcopy( (char *) entry, (char *) &old_entry, sizeof(MenuEntry));
  XtSetSubvalues((char *) entry, entry_resources, XtNumber(entry_resources),
		 args, num_args);

  if (entry->label == NULL)
    entry->label = entry->name;

  if (entry->callbacks != old_entry.callbacks) {
    if (old_entry.callbacks != NULL)
      XtFree(old_entry.callbacks);
    MaybeCopyCallbacks(entry);
  }

  if (entry->left_bitmap != old_entry.left_bitmap) 
    GetBitmapInfo(w, entry, TRUE);

  if (entry->right_bitmap != old_entry.right_bitmap)
    GetBitmapInfo(w, entry, FALSE);
}

/*      Function Name: MaybeCopyCallbacks
 *      Description: Allocates space for a callbacks
 *      Arguments: entry - the menu entry.
 *      Returns: none.
 */

static void 
MaybeCopyCallbacks(entry)
MenuEntry * entry;
{
  int size;
  XtCallbackList callbacks;

  if (entry->callbacks == NULL) return;

  for (size = 0 ; entry->callbacks[size].callback != NULL ; size++) {}
  size++;
  size *= sizeof(XtCallbackRec);
  callbacks = (XtCallbackList) XtMalloc(size);
  bcopy( (char *) entry->callbacks, (char *) callbacks, size);
  entry->callbacks = callbacks;
}

/*      Function Name: CreateGCs
 *      Description: Creates all gc's for the simple menu widget.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
CreateGCs(w)
Widget w;
{
  XGCValues values;
  XtGCMask mask;
  SimpleMenuWidget smw = (SimpleMenuWidget) w;

  values.foreground = smw->core.background_pixel;
  values.background = smw->simple_menu.foreground;
  values.font = smw->simple_menu.font->fid;
  mask        = GCForeground | GCBackground | GCFont;
  smw->simple_menu.rev_gc = XtGetGC(w, mask, &values);

  /* used to draw separater lines, is width == 0 okay? */
  values.foreground = smw->simple_menu.foreground;
  values.background = smw->core.background_pixel;
  smw->simple_menu.norm_gc = XtGetGC(w, mask, &values);

  values.fill_style = FillTiled;
  values.tile    = XmuCreateStippledPixmap(XtScreen(w), 
					   smw->simple_menu.foreground,
					   smw->core.background_pixel,
					   smw->core.depth);
  mask |= GCTile | GCFillStyle;
  smw->simple_menu.norm_grey_gc = XtGetGC(w, mask, &values);

  values.foreground = smw->simple_menu.foreground ^ smw->core.background_pixel;
  values.function = GXxor;
  smw->simple_menu.invert_gc = XtGetGC(w, GCForeground | GCFunction, &values);
}


/*      Function Name: DestroyGCs
 *      Description: Removes all gc's for the simple menu widget.
 *      Arguments: w - the simple menu widget.
 *      Returns: none.
 */

static void
DestroyGCs(w)
Widget w;
{
  SimpleMenuWidget smw = (SimpleMenuWidget) w;

  XtReleaseGC(w, smw->simple_menu.norm_gc);
  XtReleaseGC(w, smw->simple_menu.norm_grey_gc);
  XtReleaseGC(w, smw->simple_menu.rev_gc);
  XtReleaseGC(w, smw->simple_menu.invert_gc);
}

/*      Function Name: GetEventEntry
 *      Description: Gets an entry given an event that has X and Y coords.
 *      Arguments: w - the simple menu widget.
 *                 event - the event.
 *      Returns: the entry that this point is in.
 */

static MenuEntry * 
GetEventEntry(w, event)
Widget w;
XEvent * event;
{
  Position y, x_loc, y_loc;
  SimpleMenuWidget smw = (SimpleMenuWidget) w;
  MenuEntry * entry;

  switch (event->type) {
  case MotionNotify:
    x_loc = event->xmotion.x;
    y_loc = event->xmotion.y;
    break;
  case EnterNotify:
  case LeaveNotify:
    x_loc = event->xcrossing.y;
    y_loc = event->xcrossing.y;
    break;
  case ButtonPress:
  case ButtonRelease:
    x_loc = event->xbutton.x;
    y_loc = event->xbutton.y;
    break;
  default:
    XtAppError(XtWidgetToApplicationContext(w),
	       "Unknown event type in GetEventEntry().");
    break;
  }

  if ( (x_loc < 0) || (x_loc > smw->core.width) || (y_loc < 0) ||
       (y_loc > smw->core.height) )
    return(NULL);

  y = smw->simple_menu.top_margin;
  if (smw->simple_menu.label != NULL)
    y += smw->simple_menu.row_height + smw->simple_menu.vertical_space;

  if (y_loc > y) { 
    for ( entry = smw->simple_menu.entries ; entry != NULL ;
	 entry = entry->next ) {  
      Dimension height = (smw->simple_menu.row_height + 
			  smw->simple_menu.vertical_space);
      
      if ( (y_loc > y) && (y_loc < y + (Position) height) )
	return(entry);
      y += height;
    }
  }
  return(NULL);
}
  
/*      Function Name:  XawCvtStringToMenuType
 *      Description: type converter for menu type.
 *      Arguments: args, num_args - and argument list ** NOT USED **
 *                 fromVal - what it currently is.
 *                 toVal - what we will convert it to. ** RETURNED **
 *      Returns: none
 */

#define done(address) \
        { 

/* ARGSUSED */
static void
XawCvtStringToMenuType(args, num_args, fromVal, toVal)
    XrmValuePtr *args;          /* unused */
    Cardinal    *num_args;      /* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
  static XawMenuEntryType type;
  XrmQuark  XawQMenuBlank;
  XrmQuark  XawQMenuSeparator;
  XrmQuark  XawQMenuText;
  XrmQuark  q;
  char      lowerName[BUFSIZ];
  
  XawQMenuBlank      = XrmStringToQuark(XtEblank);
  XawQMenuSeparator  = XrmStringToQuark(XtEseparator);
  XawQMenuText       = XrmStringToQuark(XtEtext);
  
  XmuCopyISOLatin1Lowered(lowerName, (char *) fromVal->addr);
  q = XrmStringToQuark(lowerName);

  if (q == XawQMenuBlank) 
    type = XawMenuBlank;
  else if (q == XawQMenuSeparator)
    type = XawMenuSeparator;
  else if (q == XawQMenuText) 
    type = XawMenuText;
  else {
/*
 * Reusing a buffer here.
 */
    sprintf(lowerName, 
	    "Could not convert the string %s into a menu entry type.", 
	    (char *) fromVal->addr);
    XtWarning(lowerName);
    return;
  }
  
  toVal->size = sizeof(XawMenuEntryType);
  toVal->addr = (caddr_t) &type; 
  return;
}
