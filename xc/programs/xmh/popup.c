/*
 * $XConsortium: popup.c,v 2.21 89/10/06 15:03:22 converse Exp $
 *
 *
 *			  COPYRIGHT 1989
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/* popup.c -- Handle pop-up widgets. */

#include "xmh.h"
#include <X11/Xaw/Cardinals.h>


typedef struct _PopupStatus {
	Widget popup;
	struct _LastInput lastInput;
} PopupStatusRec, *PopupStatus;


static void DeterminePopupPosition(x_ptr, y_ptr)
    Position	*x_ptr, *y_ptr;
{
    Arg		args[3];
    Widget	source;
    Dimension	width, height;

    if (lastInput.win != -1) 
	source = XtWindowToWidget( XtDisplay(toplevel), lastInput.win);
    else
	source = toplevel;	/* %%% need to keep track of last screen */

    if (source != (Widget)NULL) {
	XtSetArg( args[0], XtNwidth, &width );
	XtSetArg( args[1], XtNheight, &height );
	XtGetValues( source, args, TWO );
	XtTranslateCoords( source, (Position) (width / 2),
			  (Position) (height / 2), x_ptr, y_ptr);
    } else {
	*x_ptr = lastInput.x;
	*y_ptr = lastInput.y;
    }
}

static Boolean PositionThePopup(popup, x, y)
    Widget	popup;
    Position	x, y;
{
    /* Hack.  Fix up the position of the popup.  The xmh app defaults file
     * contains an Xmh*Geometry specification; the effects of that on 
     * popups, and the lack of any user-supplied geometry specification for
     * popups, are mitigated here, by giving the popup shell a position.
     * (Xmh*Geometry is needed in case there is no user-supplied default.)
     * Returns True if an explicit geometry was inferred; false if the
     * widget was repositioned to (x,y).
     */

    Arg		args[4];
    String 	top_geom, pop_geom;

    XtSetArg( args[0], XtNgeometry, &top_geom );
    XtGetValues( toplevel, args, ONE );
    XtSetArg( args[0], XtNgeometry, &pop_geom );
    XtGetValues( popup, args, ONE );

    if (pop_geom == NULL || pop_geom == top_geom) {
	/* if same db entry, then ... */
	XtSetArg( args[0], XtNgeometry, (String) NULL);
	XtSetArg( args[1], XtNx, x);
	XtSetArg( args[2], XtNy, y);
	XtSetArg( args[3], XtNwinGravity, SouthWestGravity);
	XtSetValues( popup, args, FOUR);
	return False;
    }
    return True;
}


static void CenterPopupPosition(widget, popup, px, py)
    Widget	widget;
    Widget	popup;
    Position	px, py;
{
    Position	x, y;
    Position	nx, ny;
    Arg		args[3];

    if (widget == NULL) return;
    XtSetArg(args[0], XtNx, &x);
    XtSetArg(args[1], XtNy, &y);
    XtGetValues(popup, args, TWO);
    if (x == px && y == py) {

	/* Program sets geometry.  Correct our earlier calculations. */

	nx = (GetWidth(widget) - GetWidth(popup)) / 2;
	ny = (GetHeight(widget) - GetHeight(popup)) / 2;
	if (nx < 0) nx = 0;
	if (ny < 0) ny = 0;
	XtTranslateCoords(widget, nx, ny, &x, &y);
	XtSetArg(args[0], XtNx, x);
	XtSetArg(args[1], XtNy, y);
	XtSetArg(args[2], XtNwinGravity, CenterGravity);
	XtSetValues(popup, args, THREE);
    }
}
	 

/* Insure that the popup is wholly showing on the screen.
   Optionally center the widget horizontally and/or vertically
   on current position.
 */

static void InsureVisibility(popup, popup_child, x, y, centerX, centerY)
    Widget	popup, popup_child;
    Position	x, y;		/* assert: current position = (x,y) */
    Boolean	centerX, centerY;
{
    Position	root_x, root_y, new_x, new_y;
    Dimension	width, height, border;
    Arg		args[3];


    XtSetArg( args[0], XtNwidth, &width );
    XtSetArg( args[1], XtNheight, &height );
    XtSetArg( args[2], XtNborderWidth, &border );
    XtGetValues( popup, args, THREE );

    XtTranslateCoords(popup_child, (Position)0, (Position)0, &root_x, &root_y);
    if (centerX) root_x -= width/2 + border;
    if (centerY) root_y -= height/2 + border;
    if (root_x < 0) root_x = 0;
    if (root_y < 0) root_y = 0;
    border <<= 1;

    if ((root_x + width + border) > WidthOfScreen(XtScreen(toplevel))) {
	root_x = WidthOfScreen(XtScreen(toplevel)) - width - border;
    }
    if ((root_y + height + border) > HeightOfScreen(XtScreen(toplevel))) {
	root_y = HeightOfScreen(XtScreen(toplevel)) - height - border;
    }

    if (root_x != x || root_y != y) {
	XtSetArg( args[0], XtNx, root_x );
	XtSetArg( args[1], XtNy, root_y );
	XtSetValues( popup, args, TWO );
    }
}


/*ARGSUSED*/
static void DestroyPopup(widget, client_data, call_data)
    Widget		widget;		/* unused */
    XtPointer		client_data;
    XtPointer		call_data;	/* unused */
{
    Widget		popup = (Widget) client_data;
    XtPopdown(popup);
    XtDestroyWidget(popup);
}


/*ARGSUSED*/
static void TellPrompt(okayButton, client_data, call_data)
    Widget		okayButton;	/* unused */
    XtPointer		client_data;
    XtPointer		call_data;	/* unused */
{
    /* call the affirmative callback with the string typed in by the user */

    XtCallbackList	callbacks = (XtCallbackList) client_data;
    XtCallbackProc	function = callbacks[0].callback;
    Widget		dialog = (Widget) callbacks[0].closure;

    (*function) (XawDialogGetValueString(dialog));
}


#define OKAY_BUTTON_NAME "okay"

/*ARGSUSED*/
void XmhPromptOkayAction(w, event, params, num_params)
    Widget	w;		/* the "value" widget in the Dialog box */
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    XtCallCallbacks(XtNameToWidget(XtParent(w), OKAY_BUTTON_NAME), XtNcallback,
		    (XtPointer) NULL);
}


void PopupPrompt(question, okayCallback)
    String		question;		/* the prompting string */
    XtCallbackProc	okayCallback;		/* CreateFolder() */
{
    Arg			args[2];
    Widget		popup;
    Widget		dialog;
    Position		x, y;
    Boolean		positioned;
    static XtCallbackRec ok_callbacks[] = {
	{(XtCallbackProc) TellPrompt,	(XtPointer) NULL},
	{(XtCallbackProc) DestroyPopup,	(XtPointer) NULL},
	{(XtCallbackProc) NULL,		(XtPointer) NULL}
    };
    static XtCallbackRec tell_prompt_closure[] = {
	{(XtCallbackProc) NULL,		(XtPointer) NULL},
	{(XtCallbackProc) NULL,		(XtPointer) NULL}
    };
    static String text_translations = "<Key>Return: XmhPromptOkayAction()\n";

    DeterminePopupPosition(&x, &y);
    XtSetArg(args[0], XtNallowShellResize, True);
    XtSetArg(args[1], XtNinput, True);
    popup = XtCreatePopupShell("prompt", transientShellWidgetClass, toplevel,
			       args, TWO);
    positioned = PositionThePopup(popup, x, y);

    XtSetArg(args[0], XtNlabel, question);
    XtSetArg(args[1], XtNvalue, "");
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, popup, args,
				   TWO);
    XtOverrideTranslations(XtNameToWidget(dialog, "value"), 
			   XtParseTranslationTable(text_translations));

    tell_prompt_closure[0].callback = okayCallback;
    tell_prompt_closure[0].closure = (XtPointer) dialog;
    ok_callbacks[0].closure = (XtPointer) tell_prompt_closure;
    ok_callbacks[1].closure = (XtPointer) popup;
    XtSetArg(args[0], XtNcallback, ok_callbacks);
    XtCreateManagedWidget(OKAY_BUTTON_NAME, commandWidgetClass, dialog, args,
			  ONE);
    XawDialogAddButton(dialog, "cancel", DestroyPopup, (XtPointer)popup);
    XtInstallAllAccelerators(popup, popup);
    XtRealizeWidget(popup);
    InsureVisibility(popup, dialog, x, y, positioned ? False : True, False);
    XDefineCursor(XtDisplay(popup), XtWindow(popup), app_resources.cursor);
    XtPopup(popup, XtGrabNone);
}

#undef OKAY_BUTTON_NAME

/* ARGSUSED */
static void FreePopupStatus( w, closure, call_data )
    Widget w;			/* unused */
    XtPointer closure;
    XtPointer call_data;	/* unused */
{
    PopupStatus popup = (PopupStatus)closure;
    XtPopdown(popup->popup);
    XtDestroyWidget(popup->popup);
    XtFree((char *) closure);
}


void PopupNotice( message, callback, closure )
    char*		message;
    XtCallbackProc	callback;
    XtPointer		closure;
{
    PopupStatus popup_status = (PopupStatus)closure;
    Arg args[5];
    Widget dialog;
    Position x, y;
    char command[65], label[128];

    if (popup_status == (PopupStatus)NULL) {
	popup_status = XtNew(PopupStatusRec);
	popup_status->lastInput = lastInput;
    }
    if (sscanf( message, "%64s", command ) != 1)
	(void) strcpy( command, "system" );
    else {
	int l = strlen(command);
	if (l && command[--l] == ':')
	    command[l] = '\0';
    }
    (void) sprintf( label, "%.64s command returned:", command );

    DeterminePopupPosition(&x, &y);
    XtSetArg( args[0], XtNallowShellResize, True );
    XtSetArg( args[1], XtNinput, True );
    popup_status->popup = XtCreatePopupShell( "notice",
			     transientShellWidgetClass, toplevel, args, TWO );
    PositionThePopup(popup_status->popup, x, y);

    XtSetArg( args[0], XtNlabel, label );
    XtSetArg( args[1], XtNvalue, message );
    dialog = XtCreateManagedWidget( "dialog", dialogWidgetClass,
				   popup_status->popup, args, TWO);

    /* The text area of the dialog box will not be editable. */

    XtSetArg( args[0], XtNeditType, XawtextRead);
    XtSetArg( args[1], XtNdisplayCaret, False);
    XtSetValues( XtNameToWidget(dialog, "value"), args, TWO);

    XawDialogAddButton( dialog, "confirm",
		       ((callback != (XtCallbackProc) NULL)
		          ? callback : (XtCallbackProc) FreePopupStatus), 
		       (XtPointer) popup_status
		      );

    XtRealizeWidget( popup_status->popup );
    XtInstallAllAccelerators(popup_status->popup, popup_status->popup);
    InsureVisibility(popup_status->popup, dialog, x, y, False, False);
    XDefineCursor(XtDisplay(popup_status->popup),
		  XtWindow(popup_status->popup), app_resources.cursor);
    XtPopup(popup_status->popup, XtGrabNone);
}


void PopupConfirm(center_widget, question, affirm_callbacks, negate_callbacks)
    Widget		center_widget;	/* where to center; may be NULL */
    String		question;
    XtCallbackList	affirm_callbacks;
    XtCallbackList	negate_callbacks;
{
    Arg		args[2];
    Widget	popup;
    Widget	dialog;
    Widget	button;
    Position	x, y;
    static XtCallbackRec callbacks[] = {
	{DestroyPopup,		(XtPointer) NULL},
	{(XtCallbackProc) NULL,	(XtPointer) NULL}
    };
    static Arg	shell_args[] = {
	{ XtNallowShellResize,	(XtArgVal) True},
	{ XtNinput,		(XtArgVal) True},
    };

    DeterminePopupPosition(&x, &y);
    popup = XtCreatePopupShell("confirm", transientShellWidgetClass,
			       toplevel, shell_args, XtNumber(shell_args));
    PositionThePopup(popup, x, y); 

    XtSetArg(args[0], XtNlabel, question);
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, popup, args,
				   ONE);
    
    callbacks[0].closure = (XtPointer) popup;
    XtSetArg(args[0], XtNcallback, callbacks);
    button = XtCreateManagedWidget("yes", commandWidgetClass, dialog, 
				   args, ONE);
    if (affirm_callbacks)
	XtAddCallbacks(button, XtNcallback, affirm_callbacks);


    button = XtCreateManagedWidget("no", commandWidgetClass, dialog, 
				   args, ZERO);
    XtAddCallback(button, XtNcallback, DestroyPopup, (XtPointer) popup);
    if (negate_callbacks)
	XtAddCallbacks(button, XtNcallback, negate_callbacks);

    XtRealizeWidget(popup);
    XtInstallAllAccelerators(popup, popup);
    CenterPopupPosition(center_widget, popup, x, y);
    InsureVisibility(popup, dialog, x, y, False, False);
    XDefineCursor(XtDisplay(popup), XtWindow(popup), app_resources.cursor);
    XtPopup(popup, XtGrabNone);
}


void PopupError(message)
    String	message;
{
    Arg		args[3];
    Widget	error_popup, dialog;
    Position	x, y;
    Boolean	positioned;
    static XtCallbackRec callbacks[] = {
	{DestroyPopup,		(XtPointer) NULL},
	{(XtCallbackProc) NULL,	(XtPointer) NULL}
    };

    DeterminePopupPosition(&x, &y);

    XtSetArg(args[0], XtNallowShellResize, True);
    XtSetArg(args[1], XtNinput, True);
    error_popup = XtCreatePopupShell("error", transientShellWidgetClass,
			       toplevel, args, TWO);
    positioned = PositionThePopup(error_popup, x, y);

    XtSetArg(args[0], XtNlabel, message);
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, error_popup,
				   args, ONE);
    callbacks[0].closure = (XtPointer) error_popup;
    XtSetArg(args[0], XtNcallback, callbacks);
    XawDialogAddButton(dialog, "OK", DestroyPopup, (XtPointer) error_popup);
    
    XtRealizeWidget(error_popup);
    XtInstallAllAccelerators(error_popup, error_popup);
    InsureVisibility(error_popup, dialog, x, y,
		     positioned ? False : True, positioned ? False : True);
    XDefineCursor(XtDisplay(error_popup), XtWindow(error_popup), 
		  app_resources.cursor);
    XtPopup(error_popup, XtGrabNone);
}
 
static Widget alert = NULL;
	
void PopdownAlert()
{
    if (alert != NULL) 	XtPopdown(alert);
}

void PopupAlert(message, parent, x, y)
    String	message;
    Widget	parent;
    Position	x, y;
{
    Arg		args[2];

    if (alert == NULL) {
	XtSetArg(args[0], XtNallowShellResize, True);
	alert = XtCreatePopupShell("alert", transientShellWidgetClass,
				   parent, args, ONE);
	XtSetArg(args[0], XtNlabel, message);
	XtCreateManagedWidget("rescanning", labelWidgetClass, alert, args,
			      ONE);
	PositionThePopup(alert, x, y);
	XtRealizeWidget(alert);
	XtInstallAllAccelerators(alert, alert);
    }
    else {
	XtSetArg(args[0], XtNlabel, message);
	XtSetValues(XtNameToWidget(alert, "rescanning"), args, ONE);
    }
    XtPopup(alert, XtGrabNone);
}

