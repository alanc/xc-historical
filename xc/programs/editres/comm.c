/*
 * This file contains the code to communicate with the client that is
 * being edited.
 */


#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>	/* Get standard string definations. */
#include <X11/Xatom.h>
#include <X11/cursorfont.h>	/* For crosshair cursor. */
#include <X11/Xproto.h>

#include <X11/Xmu/Error.h>

#include "editresP.h"

/*
 * Global variables. 
 */

extern TreeInfo *global_tree_info;
extern CurrentClient global_client;
extern ScreenData global_screen_data;
extern Widget global_tree_parent;

/*
 * static Globals.
 */

static int global_error_code;
static unsigned long global_send_event_serial;
static int (*global_old_error_handler)();

static Atom atom_comm, atom_command, atom_resource_editor, atom_client_value;

/*
 * external function definitions.
 */

extern ResIdent GetNewIdent();
extern void SetMessage(), BuildVisualTree();
extern char * GetFormattedSetValuesError();

static void TellUserAboutMessage();
static Boolean StripReturnValueFromString();

/*	Function Name: ClientTimedOut
 *	Description: Called if the client takes too long to take our selection.
 *	Arguments: data - The widget that owns the client 
 *                        communication selection.
 *                 id - *** UNUSED ***
 *	Returns: none.
 */

/* ARGSUSED */
static void
ClientTimedOut(data, id)
XtPointer data;
XtIntervalId * id;
{
    char msg[BUFSIZ];
    Widget w = (Widget) data;
    

    global_client.ident = NO_IDENT;
    XtDisownSelection(w, global_client.atom, 
		      XtLastTimestampProcessed(XtDisplay(w)));

    if (global_client.value != NULL) {
	XtFree(global_client.value);
	global_client.value = NULL;
    }

    sprintf(msg, "It appears that this client does not %s.",
	    "listen to the Resource Editor");
    SetMessage(global_screen_data.info_label, msg);
}

/*	Function Name: GetClientWindow
 *	Description: Gets the Client's window by asking the user.
 *	Arguments: w - a widget.
 *	Returns: a clients window, or None.
 */

static Window 
GetClientWindow(w)
Widget w;
{
    int status;
    Cursor cursor;
    XEvent event;
    int buttons = 0;
    Display * dpy = XtDisplayOfObject(w);
    Window target_win = None, root = RootWindowOfScreen(XtScreenOfObject(w));
    XtAppContext app = XtWidgetToApplicationContext(w);
    
    /* Make the target cursor */
    cursor = XCreateFontCursor(dpy, XC_crosshair);
    
    /* Grab the pointer using target cursor, letting it room all over */
    status = XGrabPointer(dpy, root, False,
			  ButtonPressMask|ButtonReleaseMask, GrabModeSync,
			  GrabModeAsync, root, cursor, CurrentTime);
    if (status != GrabSuccess) {
	SetMessage(global_screen_data.info_label, "Can't grab the mouse.\n");
	return(None);
    }

    /* Let the user select a window... */
    while ((target_win == None) || (buttons != 0)) {
	/* allow one more event */
	XAllowEvents(dpy, SyncPointer, CurrentTime);
	XtAppNextEvent(app, &event);
	switch (event.type) {
	case ButtonPress:
	    if (event.xbutton.window != root) {
		XtDispatchEvent(&event);
		break;
	    }

	    if (target_win == None) {
		target_win = event.xbutton.subwindow; /* window selected */
	    }
	    buttons++;
	    break;
	case ButtonRelease:
	    if (event.xbutton.window != root) {
		XtDispatchEvent(&event);
		break;
	    }

	    if (buttons > 0) /* There may have been some
				down before we started */
		buttons--;
	    break;
	default:
	    XtDispatchEvent(&event);
	    break;
	}
    } 
    
    XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */
    
    return(target_win);
}

/*	Function Name: SetCommand
 *	Description: Causes this widget to own the resource editor's 
 *                   command selection.
 *	Arguments: w - the widget that will own the selection.
 *                 command - command to send to client.
 *                 value - value string to pass with the command.
 *                 msg - message to prompt the user to select a client.
 *	Returns: none.
 */

/* ARGSUSED */
void
SetCommand(w, command, value, msg)
Widget w;
ResCommand command;
char * value, * msg;
{
    Time time; 
    XClientMessageEvent client_event;
    Display * dpy = XtDisplay(w);
    static void ClientTimedOut(), LoseSelection();
    static int HandleXErrors();
    static Boolean ConvertCommand();
    
    if (msg == NULL) 
	msg = "Click the mouse pointer on any Xaw client.";

    SetMessage(global_screen_data.info_label, msg);
	      
    if (global_client.window == None) {
	Window win;

	if ( (win = GetClientWindow(w)) == None) 
	    return;

	global_client.window = XmuClientWindow(dpy, win);
    }

    time = XtLastTimestampProcessed(dpy);
    global_client.ident = GetNewIdent();
    
    global_client.command = command;
    global_client.value = value;
    global_client.atom = atom_comm;

    if (!XtOwnSelection(w, global_client.atom, time, ConvertCommand, 
			LoseSelection, NULL))
	SetMessage(global_screen_data.info_label,
		   "Unable to own the Resource Selection");

    client_event.window = global_client.window;
    client_event.type = ClientMessage;
    client_event.message_type = atom_resource_editor;
    client_event.format = RES_EDIT_SEND_EVENT_FORMAT;
    client_event.data.l[0] = time;
    client_event.data.l[1] = global_client.atom;
    client_event.data.l[2] = (long) global_client.ident;

    global_error_code = NO_ERROR;                 /* Reset Error code. */
    global_old_error_handler = XSetErrorHandler(HandleXErrors);
    global_send_event_serial = NextRequest(dpy);

    XSendEvent(dpy, global_client.window, FALSE, (long) 0, 
	       (XEvent *) &client_event);

    XSync(dpy, FALSE);
    XSetErrorHandler(global_old_error_handler);
    if (global_error_code == NO_SEND_EVENT_WINDOW) {
	char error_buf[BUFSIZ];
	
	global_error_code = NO_ERROR;	/* Reset Error code. */
	sprintf(error_buf, "The communication window with%s%s.",
		" application is no longer avaliable\n",
		"Please select the application manually");

	global_client.window = None;
	SetCommand(w, command, value, error_buf);
	return;
    }   
		   
    TellUserAboutMessage(global_screen_data.info_label, command);
    global_client.timeout = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
					    CLIENT_TIME_OUT, 
					    ClientTimedOut, (XtPointer) w);
}

/*	Function Name: TellUserAboutMessage
 *	Description: Informs the user that we have sent a message to the client
 *	Arguments: label - the info label.
 *                 command - command that we have executed.
 *	Returns: none.
 */

static void
TellUserAboutMessage(label, command)
Widget label;
ResCommand command;
{
    char msg[BUFSIZ], *str;

    switch(command) {
    case SendWidgetTree:
	str = " asking for widget tree";
	break;
    case SetValues:
	str = " asking it to perform SetValues()";
	break;
    case GetValues:
	str = " asking it to perform GetValues()";
	break;
    default:
	str = "";
	break;
    }

    sprintf(msg, "Message sent to client%s.", str);
    SetMessage(label, msg);
}

/*	Function Name: ConvertCommand
 *	Description: Converts the command string into a selection that can
 *                   be sent to the client.
 *	Arguments: (see Xt)
 *	Returns: TRUE if we could convert the selection and target asked for.
 */

#define EXTRA_SPACE 30		/* Space for ident as a string, and 
				   a few other characters. */

/* ARGSUSED */
static Boolean
ConvertCommand(w,selection,target,type_ret, value_ret, length_ret, format_ret)
Widget w;
Atom * selection, * target, * type_ret;
XtPointer *value_ret;
unsigned long * length_ret;
int * format_ret;
{
    int len;
    char * command, *command_str, *value;

    if ((*selection != atom_comm) || (*target != atom_command))
	return(FALSE);

    *type_ret = XA_STRING;

    switch(global_client.command) {
    case SendWidgetTree:
	command_str = SEND_WIDGET_TREE;
	break;
    case SetValues:
	command_str = SET_VALUES;
	break;
    default:
	SetMessage(global_screen_data.info_label,
		   "Internal Error: Unknown client command.");
	return(FALSE);
    }

    if (global_client.value == NULL)
	value = "";
    else
	value = global_client.value;

    len = strlen(command_str) + strlen(value) + EXTRA_SPACE;
    command = XtMalloc(sizeof(char) * len);

    sprintf(command, "%d%c%s%c%s", global_client.ident, ID_SEPARATOR, 
	    command_str, COMMAND_SEPARATOR, value);

#ifdef DEBUG
    printf("To Client:\n%s\n\n", command);
#endif 

    *value_ret = (XtPointer) command;
    *length_ret = strlen(command) + 1;
    *format_ret = RES_EDITOR_FORMAT;
    
    return(TRUE);
}

/*	Function Name: LoseSelection
 *	Description: Called when we have lost the selection, asks client
 *                   for the selection value.
 *	Arguments: w - the widget that just lost the selection.
 *                 sel - the selection.
 *	Returns: none.
 */

static void
LoseSelection(w, sel)
Widget w;
Atom * sel;
{
    static void GetClientValue();

    if (global_client.timeout != 0) {
	XtRemoveTimeOut(global_client.timeout);
	global_client.timeout = 0;
    }

    if (global_client.value != NULL) {
	XtFree(global_client.value);
	global_client.value = NULL;
    }

    XtGetSelectionValue(w, *sel, atom_client_value, GetClientValue,
			NULL, XtLastTimestampProcessed(XtDisplay(w)));
}

/*	Function Name: GetClientValue
 *	Description: Gets the value out of the client, and does good things
 *                   to it.
 *	Arguments: w - the widget that asked for the selection.
 *                 data - client_data *** UNUSED ***.
 *                 sel - the selection.
 *                 type - the type of the selection.
 *                 value - the selection's value.
 *                 length - the length of the selection's value.
 *                 format - the format of the selection.
 *	Returns: none.
 */


/* ARGSUSED */
static void
GetClientValue(w, data, selection, type, value, length, format)
Widget w;
XtPointer data, value;
Atom *selection, *type;
unsigned long *length;
int * format;
{
    Time time;
    char *string, *local_value, msg[BUFSIZ];
    ResIdent ident;
    ResourceError error;

    if ( (*type != XA_STRING) || (*format != RES_EDITOR_FORMAT) ) 
	return;

    string = (char *) value;

#ifdef DEBUG
    printf("FromClient:\n%s\n\n", string);
#endif 

    if (!StripReturnValueFromString(string, &ident, &error, &local_value)) {
	sprintf(msg, "Error while parsing client string:\n%s\n", local_value);
	SetMessage(global_screen_data.info_label, msg);
    }

    if ( (error != NoResError) || (ident != global_client.ident)) {
	if (error == UnformattedResError) {
	    sprintf(msg, "Error message received from client:\n%s\n", 
		    local_value);
	    SetMessage(global_screen_data.info_label, msg);
	}
	else if (error == FormattedResError) {
	    char * error_msg = NULL;

	    switch(global_client.command) {
	    case SendWidgetTree:
		sprintf(msg, "Error message received from client:\n%s\n", 
			local_value);
		break;
	    case SetValues:
		error_msg = GetFormattedSetValuesError(local_value);
		break;
	    default:
		sprintf(msg, "Internal error: Unknown command %d.", 
			global_client.command);
		break;
	    }
		
	    if (error_msg == NULL) 
		SetMessage(global_screen_data.info_label, msg);
	    else {
		SetMessage(global_screen_data.info_label, error_msg);
		XtFree(error_msg);
	    }
	}
#ifdef DEBUG
	else
	    printf("Incorrect ident from client.\n");
#endif 

	/*
	 * global state is still active, re-assert selection.
	 */
	  
	time = XtLastTimestampProcessed(XtDisplay(w));

	if (!XtOwnSelection(w, *selection, time, ConvertCommand, 
			    LoseSelection, NULL))
	    SetMessage(global_screen_data.info_label,
		       "Unable to own the Resource Selection");
	return;
    }	

    switch(global_client.command) {
    case SendWidgetTree:
	BuildVisualTree(global_tree_parent, local_value);
	break;
    case SetValues:
	SetMessage(global_screen_data.info_label, local_value);
	break;
    }
}

/*	Function Name: StripReturnValueFromString
 *	Description: 
 *	Arguments: string - The string that came from the client.
 *                 ident - the ident number *** RETURNED ***
 *                 error - True if an error has occured *** RETURNED ***
 *                 value - If error is true then this contains 
 *                                the error message.
 *	Returns: True if no error occured while parsing.
 */

#define ERROR_TEMPLATE \
	  "No %s Separator in the return string received from the client"

static Boolean
StripReturnValueFromString(string, ident, error, value)
char * string;
ResIdent * ident;
ResourceError * error;
char ** value;
{
    char * ptr, buf[BUFSIZ];

    *error = NoResError;
    *value = NULL;

    ptr = index(string, ID_SEPARATOR);
    if (ptr == NULL) {
	*error = UnformattedResError;
	*ident = 0;
	sprintf(buf, ERROR_TEMPLATE, "ID");
	*value = XtNewString(buf);
	return(FALSE);
    }
   
    *ptr = '\0';
    *ident = atoi(string);	/* ASSUME that ident is an int. */
    
    string = ptr + 1;		/* now points to error character. */

    ptr = index(string, COMMAND_SEPARATOR);
    if (ptr == NULL) {
	*error = UnformattedResError;
	sprintf(buf, ERROR_TEMPLATE, "COMMAND");
	*value = XtNewString(buf);
	return(FALSE);
    }

    *ptr++ = '\0';
    *error = (ResourceError) atol(string);
    *value = ptr;
  
    return(TRUE);		/* parsed cleanly. */
}
 
/*	Function Name: InternAtoms
 *	Description: interns all static atoms.
 *	Arguments: display - the current display.
 *	Returns: none.
 */

void
InternAtoms(dpy)
Display * dpy;
{
    atom_comm = XInternAtom(dpy, RES_EDITOR_COMM_ATOM, False);
    atom_command = XInternAtom(dpy, RES_EDITOR_COMMAND_ATOM, False);
    atom_resource_editor = XInternAtom(dpy, RES_EDITOR_NAME, False);
    atom_client_value = XInternAtom(dpy, RES_EDITOR_CLIENT_VALUE, False);
}

ResIdent
GetNewIdent()
{
    static ResIdent ident = 1;

    return(ident++);
}

/*	Function Name: HandleXErrors
 *	Description: Handles error codes from the server.
 *	Arguments: display - the display.
 *                 error - error information.
 *	Returns: none.
 */

/* ARGSUSED */
static int
HandleXErrors(display, error)
Display * display;
XErrorEvent * error;
{
    if (error->serial != global_send_event_serial) {
	(*global_old_error_handler) (display, error);
	return(0);
    }

    if ( (error->error_code == BadWindow) && 
	(error->request_code == X_SendEvent) ) 
	global_error_code = NO_SEND_EVENT_WINDOW;    
    else {
	if (XmuPrintDefaultErrorMessage(display, error, stderr) != 0)
	    exit(1);
    }
    return(0);
}
