/* $XConsortium: popup.c,v 2.10 89/06/12 18:14:56 converse Exp $ */
/* popup.c -- Handle pop-up widgets. */

#include "xmh.h"
#include <X11/Xaw/Cardinals.h>

typedef char* Pointer;

typedef struct _PopupStatus {
	Widget popup;
	struct _LastInput lastInput;
} PopupStatusRec, *PopupStatus;

/*
static Widget confirmwidget = NULL;
static char *confirmstring;

static Widget confirmparent;
static count = 0;

static Widget promptwidget;
static int (*promptfunc)();
static char promptstring[210];

extern TellPrompt();
void DestroyPromptWidget();

ArgList arglist, promptarglist;
*/

static Scrn lastscrn = NULL;
static char laststr[500];

static Widget confirmwidget = NULL;
static int buttoncount = 0;
static Widget promptwidget = NULL;
static void (*promptfunction)();


void CenterWidget(parent, child)
Widget parent, child;
{
    int x, y;
    x = (GetWidth(parent) - GetWidth(child)) / 2;
    y = (GetHeight(parent) - GetHeight(child)) / 2;
    if (x < 0) x = 0;
    XtMoveWidget(child, x, y);
}


void DestroyConfirmWidget()
{
    lastscrn = NULL;
    *laststr = 0;
    if (confirmwidget) {
	XtDestroyWidget(confirmwidget);
	confirmwidget = NULL;
    }
}


int Confirm(scrn, str)
Scrn scrn;
char *str;
{
    Arg args[1];
    extern void RedoLastButton();
    if (lastscrn == scrn && strcmp(str, laststr) == 0) {
	DestroyConfirmWidget();
	return TRUE;
    }
    DestroyConfirmWidget();
    lastscrn = scrn;
    scrn = LastButtonScreen();
    (void) strcpy(laststr, str);
    XtSetArg( args[0], XtNlabel, str );
    confirmwidget = XtCreateWidget( "confirm", dialogWidgetClass,
				    scrn->widget, args, ONE );
    XawDialogAddButton(confirmwidget, "yes", RedoLastButton, (caddr_t)NULL);
    XawDialogAddButton(confirmwidget, "no", DestroyConfirmWidget,(caddr_t)NULL);
    XtRealizeWidget( confirmwidget );
    CenterWidget(scrn->widget, confirmwidget);
    XtMapWidget( confirmwidget );
    buttoncount = 0;
    return FALSE;
}


HandleConfirmEvent(event)
XEvent *event;
{
    if (confirmwidget &&
	    (event->type == ButtonRelease || event->type == KeyPress)) {
	if (++buttoncount > 1)
	    DestroyConfirmWidget();
    }
}


/* ARGSUSED */
void DestroyPromptWidget(widget, client_data, call_data)
    Widget widget;		/* unused */
    caddr_t client_data;	/* scrn */
    caddr_t call_data;		/* unused */
{
    if (promptwidget) {
	Scrn scrn = (Scrn)client_data;
	XtSetKeyboardFocus(scrn->parent, scrn->viewwidget);
	XtDestroyWidget(promptwidget);
	promptwidget = NULL;
    }
}


/* ARGSUSED */
void TellPrompt(widget, client_data, call_data)
    Widget widget;
    caddr_t client_data;	/* scrn */
    caddr_t call_data;

{
    (*promptfunction)(XawDialogGetValueString(promptwidget));
    DestroyPromptWidget(widget, client_data, call_data);
}

MakePrompt(scrn, prompt, func)
Scrn scrn;
char *prompt;
void (*func)();
{
    static Arg args[] = {
	{XtNlabel, NULL},
	{XtNvalue, NULL},
    };
    args[0].value = (XtArgVal)prompt;
    args[1].value = (XtArgVal)"";
    DestroyPromptWidget((Widget)NULL, (caddr_t)scrn, NULL);
    promptwidget = XtCreateWidget( "prompt", dialogWidgetClass, scrn->widget,
				   args, TWO );
    XawDialogAddButton(promptwidget, "goAhead", TellPrompt, (caddr_t)scrn);
    XawDialogAddButton(promptwidget, "cancel", DestroyPromptWidget, (caddr_t)scrn);
    XtRealizeWidget(promptwidget);
    XtSetKeyboardFocus(promptwidget, XtNameToWidget(promptwidget,"value"));
    XtSetKeyboardFocus(scrn->parent, (Widget)None);
    CenterWidget(scrn->widget, promptwidget);
    XtMapWidget( promptwidget );
    promptfunction = func;
}

/* void */
PopupNotice( message, callback, closure )
    char*		message;
    XtCallbackProc	callback;
    Pointer		closure;
{
    PopupStatus popup_status = (PopupStatus)closure;
    Arg args[5];
    Widget dialog;
    Widget source;
    Position x, y, root_x, root_y;
    Dimension width, height, border;
    char command[65], label[128];
    static void FreePopupStatus();

    if (popup_status == (PopupStatus)NULL) {
	popup_status = XtNew(PopupStatusRec);
	popup_status->lastInput = lastInput;
    }
    if (sscanf( message, "%64s", command ) != 1)
	strcpy( command, "system" );
    else {
	int l = strlen(command);
	if (l && command[--l] == ':')
	    command[l] = '\0';
    }

    sprintf( label, "%.64s command returned:", command );
    if (lastInput.win != -1) {
	source = XtWindowToWidget( XtDisplay(toplevel),
				   popup_status->lastInput.win
				  );
    }
    else
	source = toplevel;	/* %%% need to keep track of last screen */

    if (source != (Widget)NULL) {
	XtSetArg( args[0], XtNwidth, &width );
	XtSetArg( args[1], XtNheight, &height );
	XtGetValues( source, args, TWO );
	XtTranslateCoords( source, (Position)0, (Position)0, &x, &y );
	x += width/2;
	y += height/2;
    } else {
	x = popup_status->lastInput.x;
	y = popup_status->lastInput.y;
    }
    XtSetArg( args[0], XtNallowShellResize, True );
    popup_status->popup =
	XtCreatePopupShell( "notice", transientShellWidgetClass,
			    toplevel, args, ONE );
    {
	/* hack; now fix up the geometry if old resources */
	String g1, g2;
	XtSetArg( args[0], XtNgeometry, &g1 );
	XtGetValues( toplevel, args, ONE );

	XtSetArg( args[0], XtNgeometry, &g2 );
	XtGetValues( popup_status->popup, args, ONE );

	if (g2 == NULL || g2 == g1) { /* if same db entry, then ... */
	    char geometry[64];
	    sprintf( geometry, "+%d+%d", x, y );
	    XtSetArg( args[0], XtNgeometry, geometry );
	    XtSetValues( popup_status->popup, args, ONE );
	}
    }
    XtSetArg( args[0], XtNlabel, label );
    XtSetArg( args[1], XtNvalue, message );
    dialog =
	XtCreateManagedWidget( "dialog", dialogWidgetClass,
			       popup_status->popup, args, TWO );
    XawDialogAddButton( dialog, "confirm",
		       (callback != (XtCallbackProc)NULL)
		          ? callback : (XtCallbackProc)FreePopupStatus,
		       (Pointer)popup_status
		      );
    XtRealizeWidget( popup_status->popup );
    XtSetArg( args[0], XtNwidth, &width );
    XtSetArg( args[1], XtNheight, &height );
    XtSetArg( args[2], XtNborderWidth, &border );
    XtGetValues( popup_status->popup, args, THREE );
    border <<= 1;
    XtTranslateCoords( dialog, (Position)0, (Position)0, &root_x, &root_y);
    if ((root_x + width + border) > WidthOfScreen(XtScreen(toplevel))) {
	root_x = WidthOfScreen(XtScreen(toplevel)) - width - border;
    } else root_x = x;
    if ((root_y + height + border) > HeightOfScreen(XtScreen(toplevel))) {
	root_y = HeightOfScreen(XtScreen(toplevel)) - height - border;
    } else root_y = y;
    if (root_x != x || root_y != y) {
	XtSetArg( args[0], XtNx, root_x );
	XtSetArg( args[1], XtNy, root_y );
	XtSetValues( popup_status->popup, args, TWO );
    }
    XtPopup(popup_status->popup, XtGrabNone);
}


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
