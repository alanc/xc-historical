#if !defined(lint) && !defined(SABER)
static char rcs_id[]=
    "$XConsortium: popup.c,v 2.17 89/09/05 16:29:26 converse Exp $";
#endif

/*
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

typedef char* Pointer;

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

    if (lastInput.win != -1) {
	source = XtWindowToWidget( XtDisplay(toplevel), lastInput.win);
    }
    else
	source = toplevel;	/* %%% need to keep track of last screen */

    if (source != (Widget)NULL) {
	XtSetArg( args[0], XtNwidth, &width );
	XtSetArg( args[1], XtNheight, &height );
	XtGetValues( source, args, TWO );
	XtTranslateCoords( source, (Position)0, (Position)0, x_ptr, y_ptr);
	*x_ptr += width/2;
	*y_ptr += height/2;
    } else {
	*x_ptr = lastInput.x;
	*y_ptr = lastInput.y;
    }
}


static void PositionThePopup(popup, x, y)
    Widget	popup;
    Position	x, y;
{
    /* Hack.  Fix up the position of the popup.  The xmh app defaults file
     * contains an Xmh*Geometry specification; the effects of that on 
     * popups, and the lack of any user-supplied geometry specification for
     * popups, are mitigated here, by giving the popup shell a position.
     * (Xmh*Geometry is needed in case there is no user-supplied default.)
     */

    Arg		args[3];
    String 	g1, g2;
    XtSetArg( args[0], XtNgeometry, &g1 );
    XtGetValues( toplevel, args, ONE );
    XtSetArg( args[0], XtNgeometry, &g2 );
    XtGetValues( popup, args, ONE );

    if (g2 == NULL || g2 == g1) { /* if same db entry, then ... */
	XtSetArg( args[0], XtNgeometry, (String) NULL);
	XtSetArg( args[1], XtNx, x);
	XtSetArg( args[2], XtNy, y);
	XtSetValues( popup, args, THREE);
    }
}


/* Insure that the popup is wholly showing on the screen. */

static void InsureVisibility(popup, popup_child, x, y)
    Widget	popup, popup_child;
    Position	x, y;
{
    static Position	root_x, root_y;
    Dimension	width, height, border;
    Arg		args[3];	


    XtSetArg( args[0], XtNwidth, &width );
    XtSetArg( args[1], XtNheight, &height );
    XtSetArg( args[2], XtNborderWidth, &border );
    XtGetValues( popup, args, THREE );

    border <<= 1;
    XtTranslateCoords(popup_child, (Position)0, (Position)0, &root_x, &root_y);
    if ((root_x + width + border) > WidthOfScreen(XtScreen(toplevel))) {
	root_x = WidthOfScreen(XtScreen(toplevel)) - width - border;
    } else root_x = x;
    if ((root_y + height + border) > HeightOfScreen(XtScreen(toplevel))) {
	root_y = HeightOfScreen(XtScreen(toplevel)) - height - border;
    } else root_y = y;

    if (root_x != x || root_y != y) {
	XtSetArg( args[0], XtNx, root_x );
	XtSetArg( args[1], XtNy, root_y );
	XtSetValues( popup, args, TWO );
    }
}


/*ARGSUSED*/
static void DestroyPopupPrompt(widget, client_data, call_data)
    Widget		widget;		/* unused */
    caddr_t		client_data;
    caddr_t		call_data;	/* unused */
{
    Widget		popup = (Widget) client_data;
    XtPopdown(popup);
    XtDestroyWidget(popup);
}


/*ARGSUSED*/
static void TellPrompt(okayButton, client_data, call_data)
    Widget		okayButton;	/* unused */
    caddr_t		client_data;
    caddr_t		call_data;	/* unused */
{
    /* call the affirmative callback with the string typed in by the user */

    XtCallbackList	callbacks = (XtCallbackList) client_data;
    XtCallbackProc	function = callbacks[0].callback;
    Widget		dialog = (Widget) callbacks[0].closure;

    (*function) (XawDialogGetValueString(dialog));
}


#define OkayButtonName "okay"

/*ARGSUSED*/
void XmhPromptOkayAction(w, event, params, num_params)
    Widget	w;		/* the "value" widget in the Dialog box */
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    XtCallCallbacks(XtNameToWidget(XtParent(w), OkayButtonName), XtNcallback,
		    (caddr_t) NULL);
}


void PopupPrompt(question, okayCallback)
    char		*question;		/* the prompting string */
    XtCallbackProc	okayCallback;		/* CreateNewFolder() */
{
    Arg			args[2];
    Widget		popup;
    Widget		dialog;
    Position		x, y;
    static XtCallbackRec ok_callbacks[] = {
	{(XtCallbackProc) TellPrompt,		(caddr_t) NULL},
	{(XtCallbackProc) DestroyPopupPrompt,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,			(caddr_t) NULL}
    };
    static XtCallbackRec tell_prompt_closure[] = {
	{(XtCallbackProc) NULL,			(caddr_t) NULL},
	{(XtCallbackProc) NULL,			(caddr_t) NULL}
    };
    static String text_translations = "<Key>Return: XmhPromptOkayAction()\n";

    DeterminePopupPosition(&x, &y);
    XtSetArg(args[0], XtNallowShellResize, True);
    XtSetArg(args[1], XtNinput, True);
    popup = XtCreatePopupShell("prompt", transientShellWidgetClass, toplevel,
			       args, TWO);
    PositionThePopup(popup, x, y);

    XtSetArg(args[0], XtNlabel, question);
    XtSetArg(args[1], XtNvalue, "");
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, popup, args,
				   TWO);
    XtOverrideTranslations(XtNameToWidget(dialog, "value"), 
			   XtParseTranslationTable(text_translations));

    tell_prompt_closure[0].callback = okayCallback;
    tell_prompt_closure[0].closure = (caddr_t) dialog;
    ok_callbacks[0].closure = (caddr_t) tell_prompt_closure;
    ok_callbacks[1].closure = (caddr_t) popup;
    XtSetArg(args[0], XtNcallback, ok_callbacks);
    XtCreateManagedWidget(OkayButtonName, commandWidgetClass, dialog, args,
			  ONE);
    XawDialogAddButton(dialog, "cancel", DestroyPopupPrompt, (caddr_t) popup);

    XtRealizeWidget(popup);
    InsureVisibility(popup, dialog, x, y);
    XDefineCursor(XtDisplay(popup), XtWindow(popup), app_resources.cursor);
    XtPopup(popup, XtGrabNone);
}

#undef OkayButtonName

/* ARGSUSED */
static void FreePopupStatus( w, closure, call_data )
    Widget w;			/* notused */
    Pointer closure;
    Pointer call_data;		/* notused */
{
    PopupStatus popup = (PopupStatus)closure;
    XtPopdown(popup->popup);
    XtDestroyWidget(popup->popup);
    XtFree(closure);
}


void PopupNotice( message, callback, closure )
    char*		message;
    XtCallbackProc	callback;
    Pointer		closure;
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
    XtSetValues( XtNameToWidget(dialog, "value"), args, ONE);

    XawDialogAddButton( dialog, "confirm",
		       (callback != (XtCallbackProc)NULL)
		          ? callback : (XtCallbackProc)FreePopupStatus,
		       (Pointer)popup_status
		      );

    XtRealizeWidget( popup_status->popup );
    InsureVisibility(popup_status->popup, dialog, x, y);
    XDefineCursor(XtDisplay(popup_status->popup),
		  XtWindow(popup_status->popup), app_resources.cursor);
    XtPopup(popup_status->popup, XtGrabNone);
}


/*ARGSUSED*/
static void DestroyPopupConfirm(widget, closure, call_data)    
    Widget	widget;		/* not used */
    Pointer	closure;
    Pointer	call_data;	/* not used */
{
    Widget	popup = (Widget) closure;
    XtPopdown(popup);
    XtDestroyWidget(popup);
}


void PopupConfirm(question, affirm_callbacks, negate_callbacks)
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
	{DestroyPopupConfirm,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,	(caddr_t) NULL}
    };

    DeterminePopupPosition(&x, &y);
    XtSetArg(args[0], XtNallowShellResize, True);
    XtSetArg(args[1], XtNinput, True);
    popup = XtCreatePopupShell("confirm", transientShellWidgetClass,
			       toplevel, args, TWO);
    PositionThePopup(popup, x, y);

    XtSetArg(args[0], XtNlabel, question);
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, popup, args,
				   ONE);
    
    callbacks[0].closure = (caddr_t) popup;
    XtSetArg(args[0], XtNcallback, callbacks);
    button = XtCreateManagedWidget("yes", commandWidgetClass, dialog, 
				   args, ONE);
    if (affirm_callbacks)
	XtAddCallbacks(button, XtNcallback, affirm_callbacks);


    button = XtCreateManagedWidget("no", commandWidgetClass, dialog, 
				   args, ZERO);
    XtAddCallback(button, XtNcallback, DestroyPopupConfirm, (caddr_t) popup);
    if (negate_callbacks)
	XtAddCallbacks(button, XtNcallback, negate_callbacks);

    XtRealizeWidget(popup);
    InsureVisibility(popup, dialog, x, y);
    XDefineCursor(XtDisplay(popup), XtWindow(popup), app_resources.cursor);
    XtPopup(popup, XtGrabNone);
}

static Widget error_popup = NULL;	/* one at a time */

/*ARGSUSED*/
static void DestroyPopupError(widget, closure, call_data)    
    Widget	widget;		/* not used */
    Pointer	closure;
    Pointer	call_data;	/* not used */
{
    if (error_popup) {
	XtPopdown(error_popup);
	XtDestroyWidget(error_popup);
	error_popup = NULL;
    }
}

void PopupError(message)
    String	message;
{
    Arg		args[3];
    Widget	dialog;
    Position	x, y;
    static XtCallbackRec callbacks[] = {
	{DestroyPopupConfirm,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,	(caddr_t) NULL}
    };

    DeterminePopupPosition(&x, &y);

    if (error_popup != NULL)
	DestroyPopupError((Widget) NULL, (caddr_t) error_popup,
			  (caddr_t) NULL);

    XtSetArg(args[0], XtNallowShellResize, True);
    XtSetArg(args[1], XtNinput, True);
    error_popup = XtCreatePopupShell("error", transientShellWidgetClass,
			       toplevel, args, TWO);
    PositionThePopup(error_popup, x, y);

    XtSetArg(args[0], XtNlabel, message);
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, error_popup,
				   args, ONE);
    callbacks[0].closure = (caddr_t) error_popup;
    XtSetArg(args[0], XtNcallback, callbacks);
    XawDialogAddButton(dialog, "OK", DestroyPopupError,
		       (caddr_t)error_popup);
    
    XtRealizeWidget(error_popup);
    InsureVisibility(error_popup, dialog, x, y);
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
    }
    else {
	XtSetArg(args[0], XtNlabel, message);
	XtSetValues(XtNameToWidget(alert, "rescanning"), args, ONE);
    }
    XtPopup(alert, XtGrabNone);
}

