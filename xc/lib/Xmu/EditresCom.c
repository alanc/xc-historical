/*
 * $XConsortium: EditResCom.c,v 1.6 90/03/16 13:58:29 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>	/* To get into the composite and core widget
				   structures. */
#include <X11/ObjectP.h>	/* For XtIs<Classname> macros. */
#include <X11/StringDefs.h>	/* for XtRString. */
#include <X11/ShellP.h>		/* for Application Shell Widget class. */

#include <X11/Xatom.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/EditRes.h>

#ifdef notdef
#  include <X11/Xaw/Text.h>
#endif

#define CURRENT_PROTOCOL_VERSION 2L

#define streq(a,b) (strcmp( (a), (b) ) == 0)

static Atom res_editor_command, client_value;

static char * global_selection_command;

typedef struct _Message {
    char * str;			/* The Error String. */
    int len, alloc;		/*length of message, and amount alloced space*/
    Boolean formatted;		/* is this a formatted error message. */
} Message;

static Message * global_error;
static char *global_widget_str, *global_resource_name, *global_resource_value;
static Boolean ParseCommand();
static void AddError(), ExecuteCommand(), SendReturnCommand();
static void SendBackError(), SendCommand();

/************************************************************
 *
 * Resource Editor Communication Code
 *
 ************************************************************/

/*	Function Name: CheckMessages
 *	Description: This callback routine is set on all shell widgets,
 *                   and checks to see of a client message event
 *                   has come from the resource editor.
 *	Arguments: w - the shell widget.
 *                 data - *** UNUSED ***
 *                 event - The X Event that triggered this handler.
 *                 cont - *** UNUSED ***.
 *	Returns: none.
 */

/* ARGSUSED */
void
_EditResCheckMessages(w, data, event, cont)
Widget w;
XtPointer data;
XEvent *event;
Boolean *cont;
{
    Time time;
    ResIdent ident;
    static Boolean first_time = FALSE;
    static Atom res_editor, res_comm;
    static void GetCommand();
    Display * disp;

    if (!first_time) {
	disp = XtDisplay(w);

	first_time = TRUE;
	res_editor = XInternAtom(disp, EDITRES_NAME, False);
	res_editor_command = XInternAtom(disp, EDITRES_COMMAND_ATOM, False);

	/*
	 * Used in later proceedures. 
	 */

	client_value = XInternAtom(disp, EDITRES_CLIENT_VALUE, False);
    }

    if (event->type == ClientMessage) {
	XClientMessageEvent * c_event = (XClientMessageEvent *) event;

	if ((c_event->message_type != res_editor) ||
	    (c_event->format != EDITRES_SEND_EVENT_FORMAT))
	    return;

	disp = XtDisplay(w);
	time = c_event->data.l[0];
	res_comm = c_event->data.l[1];
	ident = (ResIdent) c_event->data.l[2];
	if (c_event->data.l[3] != CURRENT_PROTOCOL_VERSION) {
	    char buf[BUFSIZ];

	    sprintf(buf, "%ld", CURRENT_PROTOCOL_VERSION);
	    SendCommand(w, res_comm, ident, ProtocolResError, buf);
	    return;
	}

	XtGetSelectionValue(w, res_comm, res_editor_command,
			    GetCommand, (XtPointer) ident, time);
    }
}

/*	Function Name: GetCommand
 *	Description: Gets the Command out of the selection asserted by the
 *                   resource manager.
 *	Arguments: (See Xt XtConvertSelectionProc)
 *                 data - contains the ident number for the command.
 *	Returns: none.
 */

/* ARGSUSED */
static void
GetCommand(w, data, selection, type, value, length, format)
Widget w;
XtPointer data, value;
Atom *selection, *type;
unsigned long *length;
int * format;
{
    char * command, *command_value;
    ResIdent ident = (ResIdent) data;

    if ( (*type != XA_STRING) || (*format != EDITRES_FORMAT) )
	return;

    if (ParseCommand(w, *selection, (char *) value, ident,
		     &command, &command_value))
	ExecuteCommand(w, *selection, ident, command, command_value);
}

/*	Function Name: ParseCommand
 *	Description: Parses the command string into a command and value.
 *	Arguments: str - string to parse.
 *                 ident - ident to match.
 *                 command - command to execute.
 *                 value - value associated with the command.
 *	Returns: True if string is formatted well and idents match.
 */

#define FORMAT_ERROR "Resource Editor command sent in wrong format."

static Boolean
ParseCommand(w, sel, str, ident, command, value) 
Widget w;
Atom sel;
char * str;
ResIdent ident;
char ** command, **value;
{
    char * ptr;

    if ( (ptr = index(str, ID_SEPARATOR)) == NULL) {
	SendBackError(w, sel, ident, FORMAT_ERROR);
	return(FALSE);
    }
    
    *ptr = '\0';

    if (ident != atoi(str)) {
#ifdef DEBUG	
	printf("Ident numbers don't match.\n");
#endif 
	return(FALSE);
    }

    *command = ptr + 1;
    
    if ( (ptr = index(*command, COMMAND_SEPARATOR)) == NULL) {
	SendBackError(w, sel, ident, FORMAT_ERROR);
	return(FALSE);
    }
    
    *ptr = '\0';
    *value = ptr + 1;

    return(TRUE);
}

/*	Function Name: ExecuteCommand
 *	Description: Executes a command string received from the 
 *                   resource editor.
 *	Arguments: w       - a widget.
 *                 command - the command to execute.
 *                 value - the associated with the command.
 *	Returns: none.
 *
 * NOTES:  munges str
 */

/* ARGSUSED */    
static void
ExecuteCommand(w, sel, ident, command, value)
Widget w;
Atom sel;
ResIdent ident;
char * command, *value;
{
    Message error_message;
    char * DumpWidgets(), msg[BUFSIZ];
    Boolean set_values, ret_val;

    if (streq(EDITRES_SEND_WIDGET_TREE, command)) {
	char * comm_str = DumpWidgets(w);

	SendReturnCommand(w, sel, ident, comm_str);
	XtFree(comm_str);
    }
    else if ( (set_values = streq(EDITRES_SET_VALUES, command)) ||
	      streq(EDITRES_GET_GEOMETRY, command) ) {
	static void DoSetValues();
	static Boolean DoGetGeometry();
       
	bzero((char *) &error_message, sizeof(Message));

	if (set_values) {
	    ret_val = FALSE;	/* Do not send error message back 
				   as acceptable value. */
	    DoSetValues(w, value, &error_message);
	}
	else 
	    ret_val = DoGetGeometry(w, value, &error_message);
		
	if (error_message.str == NULL) {
	    sprintf(msg, "%s was sucessful.", command);
	    SendReturnCommand(w, sel, ident, msg);
	}
	else {
	    if (ret_val) 
		SendReturnCommand(w, sel, ident, error_message.str);
	    else {
		SendCommand(w, sel, ident, 
			    (error_message.formatted ? FormattedResError 
			                             : UnformattedResError),
			    error_message.str);	    
	    }

	    XtFree(error_message.str);
	}
    }
    else if (streq(EDITRES_FIND_CHILD, command)) {
	static ResourceError DoFindChild();

	SendCommand(w, sel, ident, DoFindChild(w, value, msg), msg);
    }
    else {
	sprintf(msg, "Unknown Command: %s", command);
	SendBackError(w, sel, ident, msg);
    }
}

/*	Function Name: ConvertReturnCommand
 *	Description: Converts a selection.
 *	Arguments: w - the widget that owns the selection.
 *                 selection - selection to convert.
 *                 target - target type for this selection.
 *                 type_ret - type of the selection.
 *                 value_ret - selection value;
 *                 length_ret - lenght of this selection.
 *                 format_ret - the format the selection is in.
 *	Returns: True if conversion was sucessful.
 */
    
/* ARGSUSED */
static Boolean
ConvertReturnCommand(w, selection, target,
		     type_ret, value_ret, length_ret, format_ret)
Widget w;
Atom * selection, * target, * type_ret;
XtPointer *value_ret;
unsigned long * length_ret;
int * format_ret;
{
    char * DumpWidgets();

    /*
     * I assume the intrinsics give me the correct selection back.
     */

    if ((*target != client_value))
	return(FALSE);

    *type_ret = XA_STRING;
    *format_ret = EDITRES_FORMAT;

    *value_ret = (XtPointer) global_selection_command;
    *length_ret = strlen((String) *value_ret) + 1;

    return(TRUE);
}

/*	Function Name: CommandDone
 *	Description: done with the selection.
 *	Arguments: *** UNUSED ***
 *	Returns: none.
 */

static void
CommandDone()
{
    /* Keep the toolkit from automaticaly freeing the selection value */
}

/*	Function Name: LoseReturnCommand
 *	Description: lose selection proc called when command selection is lost.
 *	Arguments: *** UNUSED ***.
 *	Returns: none.
 */

/* ARGSUSED */
static void
LoseReturnCommand(w, sel)
Widget w;
Atom *sel;
{
    XtFree(global_selection_command);
    global_selection_command = NULL;
}

/*	Function Name: SendReturnCommand
 *	Description: builds a return command in the proper format.
 *	Arguments: w - the widget that will own the selection.
 *                 ident - the unique ident.
 *                 value - command value.
 *	Returns: none.
 */

static void
SendReturnCommand(w, sel, ident, value)
Widget w;
Atom sel;
ResIdent ident;
char * value;
{
    SendCommand(w, sel, ident, NoResError, value);
}

/*	Function Name: SendBackError
 *	Description: sends back and error message.
 *	Arguments: w - the widget to own the selection.
 *                 ident - the unique ident.
 *                 value - command value.
 *	Returns: none.
 */

static void
SendBackError(w, sel, ident, value)
Widget w;
Atom sel;
ResIdent ident;
char * value;
{
    SendCommand(w, sel, ident, UnformattedResError, value);
}

/*	Function Name: SendCommand
 *	Description: Builds a return command line.
 *	Arguments: ident - the identifier.
 *                 error - TRUE if this is an error message.
 *                 str - string to pass as the value or error message.
 *	Returns: none.
 */

static void
SendCommand(w, sel, ident, error, str)
Widget w;
Atom sel;
ResIdent ident;
ResourceError error;
char * str;
{
    int len = strlen(str);
    char * command;

    command = XtMalloc(sizeof(char) * (len + 20));
    
    sprintf(command, "%d%c%d%c%s", ident, ID_SEPARATOR, (int) error, 
	    COMMAND_SEPARATOR, str);
    
    if (global_selection_command != NULL) 
	XtFree(global_selection_command);

    global_selection_command = command;
    
/*
 * I REALLY want to own the selection.  Since this was not triggered
 * by a user action, and I am the only one using this atom it is safe to
 * use CurrentTime.
 */

    XtOwnSelection(w, sel, CurrentTime,
		   ConvertReturnCommand, LoseReturnCommand, CommandDone);
}

/************************************************************
 *
 * Generic Utility Functions.
 *
 ************************************************************/


/*	Function Name: FindAllChildren
 *	Description: Retuns all children (popup, normal and otherwise)
 *                   of this widget
 *	Arguments: parent - the parent widget.
 *                 children - the list of children.
 *	Returns: the number of children.
 */

static int
FindAllChildren(parent, children)
Widget parent, **children;
{
    CompositeWidget cw = (CompositeWidget) parent;
    int i, num_children, current = 0;
#ifdef TEXT_WIDGET
    Arg args[2];
    Widget sink, source;
#endif /* TEXT_WIDGET */

    if (!XtIsWidget(parent)) {	/* objects never have children of any kind. */
	*children = NULL; 
	return(0);
    }

    num_children = parent->core.num_popups;
	
    if (XtIsComposite(parent)) 
	num_children += cw->composite.num_children; 

#ifdef TEXT_WIDGET
	if (XtIsSubclass(parent, textWidgetClass)) {
	    XtSetArg(args[0], XtNtextSink, &sink);
	    XtSetArg(args[1], XtNtextSource, &source);
	    XtGetValues(parent, args, TWO);

	    if (sink != NULL) 
		num_children++;
	    if (source != NULL)
		num_children++;
	}
#endif /* TEXT_WIDGET */

    *children =(Widget*) XtMalloc((Cardinal) sizeof(Widget) * num_children);

    if (XtIsComposite(parent))
	for (i = 0; i < cw->composite.num_children; i++,current++) 
	    (*children)[current] = cw->composite.children[i]; 

    for ( i = 0; i < parent->core.num_popups; i++, current++) 
	(*children)[current] = parent->core.popup_list[i];

#ifdef TEXT_WIDGET
	if (XtIsSubclass(w, textWidgetClass)) {
	    if (sink != NULL) {
		(*children)[current] = sink;
		current++;
	    }
	    if (source != NULL) {
		(*children)[current] = source;
		current++;
	    }
	}
#endif /* TEXT_WIDGET */

    return(num_children);
}

/************************************************************
 *
 * Code to Perform SetValues operations.
 *
 ************************************************************/

/*	Function Name: 	DoSetValues
 *	Description: performs the setvalues requested.
 *	Arguments: w - a widget in the tree.
 *                 value - the value part of the set values command.
 *                 ret_str - return string send to the client.
 *	Returns: True if Set Value was completely sucessful.
 */

static void
DoSetValues(w, value, error)
Widget w;
char * value;
Message * error;
{
    Cardinal i, num_commands;
    static void ExecuteSetValues();
    register char * ptr, **commands;

    ptr = value;
    num_commands = 0;
    while ((ptr = index(ptr, EOL_SEPARATOR)) != NULL) {
	ptr++;
	num_commands++;
    }

    commands = (char **) XtMalloc(sizeof(char **) * num_commands);
    
    ptr = value;
    for (i = 0; i < num_commands; i++) {
	commands[i] = ptr;
	if ( (ptr = index(ptr, EOL_SEPARATOR)) == NULL) {
	    AddError(error, NULL, 
		     "Internal Client Error: incorrect line count.");
	    return;
	}
	*ptr = '\0';
	ptr++;
    }

    for (i = 0; i < num_commands; i++) {
	ExecuteSetValues(w, commands[i], error);
    }
}
		
/*	Function Name: IsChild
 *	Description: check to see of child is a child of parent.
 *	Arguments: top - the top of the tree.
 *                 parent - the parent widget.
 *                 child - the child.
 *	Returns: none.
 */

static Boolean
IsChild(top, parent, child)
Widget top, parent, child;
{
    int i, num_children;
    Widget * children;

    if (parent == NULL)
	return(top == child);

    num_children = FindAllChildren(parent, &children);

    for (i = 0; i < num_children; i++) {
	if (children[i] == child) {
	    XtFree(children);
	    return(TRUE);
	}
    }

    XtFree(children);
    return(FALSE);
}

/*	Function Name: ExecuteSetValues
 *	Description: Performs a setvalues for a given command.
 *	Arguments: w - any widget in the tree.
 *                 command - command to execute.
 *                 error - the error message info.
 *	Returns: NULL if all widgets check out, otherwise an allocated 
 *               error message.
 */

static void
ExecuteSetValues(w, command, error)
Widget w;
char * command;
Message * error;
{
    Widget widget;
    static Widget VerifyWidget();
    static void HandleToolkitErrors();
    XtErrorMsgHandler old;
    char * begin,* end, *bp, name[100], value[100];

    if ((widget = VerifyWidget(w, command, error, WID_RES_SEPARATOR)) == NULL)
	return;

    begin = index(command, WID_RES_SEPARATOR);
    end = index(command, NAME_VAL_SEPARATOR);

    if ((begin == NULL) || (end == NULL)) {
	char error_buf[BUFSIZ];

	if (begin == NULL) 
	    sprintf(error_buf, "Improperly formatted SetValues %s`%c'.", 
		    "Command, could not find a ", WID_RES_SEPARATOR);
	else
	    sprintf(error_buf, "Improperly formatted SetValues %s`%c'.", 
		    "Command, could not find a ", NAME_VAL_SEPARATOR);

	AddError(error, NULL, error_buf);
	return;
    }
	
    begin++;

    for ( bp = name; begin < end; begin++, bp++)
	*bp = *begin;
    *bp = '\0';

    strcpy(value, (end + 1));

    global_error = error;	/* No data can be passed to Error Handlers */
    global_widget_str = command; /* so I have to use globals (YUCK) */
    global_resource_name = name;
    global_resource_value = value;

    old = XtAppSetWarningMsgHandler(XtWidgetToApplicationContext(widget),
				    HandleToolkitErrors);

    XtVaSetValues(widget, XtVaTypedArg,
		  name, XtRString, value, strlen(value) + 1,
		  NULL);

    (void)XtAppSetWarningMsgHandler(XtWidgetToApplicationContext(widget), old);
}

/*	Function Name: VerifyWidget
 *	Description: Makes sure all the widgets still exist.
 *	Arguments: w - any widget in the tree.
 *                 command - the command containing the widget list to verify.
 *                 error - the error message info.
 *                 c - character that terminates the widget name.
 *	Returns: the widget id of the leaf if it exists, otherwise NULL.
 */

static Widget
VerifyWidget(w, command, error, c)
Widget w;
char * command;
Message * error;
char c;
{
    Widget top;
    static Boolean IsChild();
    char buf[BUFSIZ], *ptr, *end;
    Widget parent, child; 

    strcpy(buf, command);

    if (c != '\0') {
	if ((end = index(buf, c)) == NULL) {
	    sprintf(buf, "Formatting error is VerifyWidget, no %d %s",
		    c, "in command.");
	    AddError(error, command, buf);
	    return(NULL);
	}
	*end = '\0';
    }

    for (top = w; XtParent(top) != NULL; top = XtParent(top)) {}

    parent = NULL;
    while (TRUE) {
	if ( (ptr = rindex(buf,  NAME_SEPARATOR)) == NULL) {
	    sprintf(buf, "Formatting error is VerifyWidget, no %d %s",
		    NAME_SEPARATOR, "in command.");
	    AddError(error, command, buf);
	    return(NULL);
	}

	*ptr = '\0';
	
	child = (Widget) atol(ptr + 1);
	
	if (!IsChild(top, parent, child)) {
	    AddError(error, command, "This widget does not exist.");
	    return(FALSE);
	}

	if (ptr == buf)
	    break;
	
	parent = child;
    }
    return(child);
}

/************************************************************
 *
 * Code for Creating and dumping widget tree.
 *
 ************************************************************/

/*	Function Name: DumpWidgets
 *	Description: Give a widget it returns a string containing all the
 *		     widget names and classes.
 *	Arguments: w  - any widget.
 *	Returns: 	This function retuns a list of widget names as a
 *			single character string in the following format:
 *
 *                      name\tid.name\tid.name\tid:Class\n
 */

char * 
DumpWidgets(w)
Widget w;
{
    static void DumpChildren();

    char * list = NULL;
    Cardinal bytes = 0, end = 0;
    
    /* Find Tree's root. */
    for ( ; XtParent(w) != NULL; w = XtParent(w));
    
    DumpChildren(w, "", &list, &end, &bytes);
    
    return(list);
}

/*	Function Name: DumpChildren
 *	Description: Adds a child's name to the list.
 *	Arguments: w - the widget to dump.
 *                 parent_name - full name of this widget's parent.
 *                 list - the list of all children.
 *                 end - byte position of the list's end.
 *                 bytes - number of bytes in this string.
 *	Returns: none.
 */

static void
DumpChildren(w, parent_name, list, end, bytes)
Widget w;
char *parent_name, **list;
Cardinal *end, *bytes;
{
    int i, num_children;
    char my_name[BUFSIZ * 3];	/* beats me how big it should be... */
    static void DumpName();
    Widget * children;

    DumpName(w, parent_name, list, end, bytes);

    if (XtName(w) == NULL)
	sprintf(my_name, "%s%c%c%ld", parent_name, NAME_SEPARATOR,
		ID_SEPARATOR, (unsigned long) w);
    else
	sprintf(my_name, "%s%c%s%c%ld", parent_name, NAME_SEPARATOR,
		XtName(w), ID_SEPARATOR, (unsigned long) w);

    num_children = FindAllChildren(w, &children);

    for (i = 0; i < num_children; i++) 
	DumpChildren(children[i], my_name, list, end, bytes);

    XtFree(children);
}

/*	Function Name: DumpName
 *	Description: Prints the name of this widget 
 *	Arguments: w - the widget to dump.
 *                 parent_name - name of this widget's parent.
 *                 list - the list of all children.
 *                 end - byte position of the list's end.
 *                 bytes - number of bytes in this string.
 *	Returns: none.
 */

#define EXTRA_CHARS 5		/* Space for the static characters. */

static void
DumpName(w, parent_name, list, end, bytes)
Widget w;
char *parent_name, **list;
Cardinal *end, *bytes;
{
    char * name, *class, *ptr, id[BUFSIZ], window[BUFSIZ];
    int len, more_mem;

    sprintf(id, "%ld", (long) w);

    if (XtIsWidget(w)) 
	if (XtIsRealized(w))
	    sprintf(window, "%ld", (long) XtWindow(w));
	else
	    sprintf(window, "%d", EDITRES_IS_UNREALIZED);	    
    else
	sprintf(window, "%d", EDITRES_IS_OBJECT);

    name = XtName(w);

    if (XtIsApplicationShell(w)) {
	ApplicationShellWidget a = (ApplicationShellWidget) w;
	class = a->application.class;
    }
    else
	class = XtClass(w)->core_class.class_name;

    len = strlen(parent_name) + strlen(class) + 
	  strlen(id) + strlen(window) + EXTRA_CHARS + 1; 

    if (name != NULL)
	len += strlen(name);

    more_mem = len > BUFSIZ ? len : BUFSIZ;

    if ( (*end + len) > *bytes) 
	*list = XtRealloc(*list, (Cardinal) (*bytes += more_mem));
	
    ptr = *list + *end;
    
    if (name == NULL)
	sprintf(ptr, "%s%c%c%s%c%s%c%s\n", parent_name, NAME_SEPARATOR, 
		ID_SEPARATOR, id, CLASS_SEPARATOR, class,
		WINDOW_SEPARATOR, window);
    else
	sprintf(ptr, "%s%c%s%c%s%c%s%c%s\n", parent_name, NAME_SEPARATOR,
		name, ID_SEPARATOR, id, CLASS_SEPARATOR, class,
		WINDOW_SEPARATOR, window);

    *end += (len - 1);
}

/*************************************************************
 *
 * Error Handling code.
 *
 *************************************************************/

/*	Function Name: _AddMessage
 *	Description: Adds a message to the message structure.
 *	Arguments: message - message structure.
 *                 str - new message to add.
 *	Returns: none
 */

static void
_AddMessage(message, str)
Message * message;
char * str;
{
    char * ptr;
    int len = strlen(str) + 1;	/* leave space for extra '\n' */

    if ((message->len + len) >= message->alloc) {
	message->alloc += ((len + 2) > BUFSIZ) ? (len + 2) : BUFSIZ;
	message->str = XtRealloc(message->str, message->alloc * sizeof(char));
    }

    ptr = message->str + message->len;
    message->len += len;

    sprintf(ptr, "%s\n", str);
}
 
/*	Function Name: AddError
 *	Description: Add a string to the error message in the form:
 *                   <widget id>:<message>
 *	Arguments: error - the error structure.
 *                 widget - the string contatining the widget ids.
 *                 str - the message to append to the buffer.
 *	Returns: none.
 */

static void
AddError(error, widget, str) 
Message * error;
char * widget, *str;
{
    char buf[BUFSIZ], *end, *ptr;

    error->formatted = TRUE;

    if (widget == NULL) {
	buf[0] = NAME_VAL_SEPARATOR;
	buf[1] = '\0';
    }
    else {
	end = index(widget, WID_RES_SEPARATOR);

	if (end == NULL) 
	    strcpy(buf, widget);
	else {
	    for (ptr = buf; widget < end; ptr++, widget++) 
		*ptr = *widget;
	    *ptr = '\0';
	}
	
	*ptr++ = NAME_VAL_SEPARATOR;
	*ptr = '\0';
    }

    strcat(buf, str);

    _AddMessage(error, buf);
}

/*	Function Name: HandleToolkitErrors
 *	Description: Handles X Toolkit Errors.
 *	Arguments: name - name of the error.
 *                 type - type of the error.
 *                 class - class of the error.
 *                 msg - the default message.
 *                 params, num_params - the extra parameters for this message.
 *	Returns: none.
 */

/* ARGSUSED */
static void
HandleToolkitErrors(name, type, class, msg, params, num_params)
String name, type, class, msg, *params;
Cardinal * num_params;
{
    char buf[BUFSIZ];

    if ( streq(name, "unknownType") ) 
	sprintf(buf, "The `%s' resource is not used by this widget.", 
		global_resource_name); 
    else if ( streq(name, "noColormap") ) 
	sprintf(buf, msg, params[0]);
    else if ( streq(name, "conversionFailed") || 
	      streq(name, "conversionError") )
	sprintf(buf, "Could not convert the string `%s' %s `%s' resource.",
		global_resource_value, "to the proper type for the",
		global_resource_name); 
    else 
	sprintf(buf, "Name: %s, Type: %s, Class: %s, Msg: %s\n",
		name, type, class, msg);

    AddError(global_error, global_widget_str, buf);
}

/************************************************************
 *
 * Code for getting the geometry of widgets.
 *
 ************************************************************/

/*	Function Name: AddReturnMessage
 *	Description: adds a return message to the GetGeometry message
 *	Arguments: msg - message structure.
 *                 error_code - must be 0 or 1.
 *                 widget - widget id of widget message is associated with.
 *                 str - message to send.
 *	Returns: none
 */

static void
AddReturnMessage(msg, error_code, widget, str)
Message * msg;
int error_code;
char * widget, * str;
{
    char buf[BUFSIZ];

    msg->formatted = TRUE;

    sprintf(buf, "%1d%s%c%s", error_code, widget, 
	    NAME_VAL_SEPARATOR, str);

    _AddMessage(msg, buf);
}
    
/*	Function Name: 	DoGetGeometry
 *	Description: retreives the Geometry of each specified widget.
 *	Arguments: w - a widget in the tree.
 *                 value - the value part of the set values command.
 *                 ret_str - return string send to the client.
 *	Returns: True if Set Value was completely successful.
 */

static Boolean
DoGetGeometry(w, value, ret_str)
Widget w;
char * value;
Message * ret_str;
{
    Cardinal i, num_commands;
    static Boolean ExecuteGetGeometry();
    register char * ptr, **commands;
    Boolean ret_val = TRUE;

    ptr = value;
    num_commands = 0;
    while ((ptr = index(ptr, EOL_SEPARATOR)) != NULL) {
	ptr++;
	num_commands++;
    }

    commands = (char **) XtMalloc(sizeof(char **) * num_commands);
    
    for (i = 0, ptr = value; i < num_commands; i++) {
	commands[i] = ptr;
	if ( (ptr = index(ptr, EOL_SEPARATOR)) == NULL) {
	    ret_str->formatted = FALSE;
	    ret_str->str = XtNewString(
			      "Internal Client Error: incorrect line count.");
	    ret_str->len = strlen(ret_str->str);
	    ret_str->alloc = ret_str->len + 1;

	    return(FALSE);
	}
	*ptr = '\0';
	ptr++;
    }

    for (i = 0; i < num_commands; i++)
	ret_val &= ExecuteGetGeometry(w, commands[i], ret_str);

    return(ret_val);
}

/*	Function Name: ExecuteGetGeometry
 *	Description: Gets the geometry for each widget specified.
 *	Arguments: w - any widget in the widget tree.
 *                 command - widget to execute get geom on.
 *                 msg - message containing the return values.
 *	Returns: True if no error occured.
 */

#define NOT_VIS ("NOT_VISABLE")

static Boolean
ExecuteGetGeometry(w, command, msg)
Widget w;
String command;
Message * msg;
{
    Boolean mapped_when_man;
    char buf[100];
    Dimension width, height, border_width;
    Arg args[8];
    Cardinal num_args = 0;
    Position x, y;
    
    if ((w = VerifyWidget(w, command, msg, '\0')) == NULL)
	return(FALSE);

    if ( !XtIsRectObj(w) || (XtIsWidget(w) && !XtIsRealized(w)) ) {
	AddReturnMessage(msg, 0, command, NOT_VIS);
	return(TRUE);
    }

    XtSetArg(args[num_args], XtNwidth, &width); num_args++;
    XtSetArg(args[num_args], XtNheight, &height); num_args++;
    XtSetArg(args[num_args], XtNborderWidth, &border_width); num_args++;
    XtSetArg(args[num_args], XtNmappedWhenManaged, &mapped_when_man);
    num_args++;
    XtGetValues(w, args, num_args);

    if (!(XtIsManaged(w) && mapped_when_man) && XtIsWidget(w)) {
	XWindowAttributes attrs;
	
	/* 
	 * The toolkit does not maintain mapping state, we have
	 * to go to the server.
	 */
	
	if (XGetWindowAttributes(XtDisplay(w), XtWindow(w), &attrs) != 0)
	    if (attrs.map_state != IsViewable) {
		AddReturnMessage(msg, 0, command, NOT_VIS);
		return(TRUE);
	    }
	    else {
		AddReturnMessage(msg, 1, 
				 command, "XGetWindowAttributes failed.");
		return(FALSE);
	    }
    }

    XtTranslateCoords(w, -((int) border_width), -((int) border_width), &x, &y);

    sprintf(buf, "%dx%d%c%d%c%d%c%d", (int) width, (int) height, 
	    ((x < 0) ? '-' : '+'), (int) x, ((y < 0) ? '-' : '+'), (int) y,
	    EDITRES_BORDER_WIDTH_SEPARATOR, (int) border_width);
    
    AddReturnMessage(msg, 0, command, buf);
    return(TRUE);
}

/************************************************************
 *
 * Code for executing FindChild.
 *
 ************************************************************/

/*	Function Name: NodeToID
 *	Description: gets the fully specified node id as a string.
 *	Arguments: node - node to work on.
 *                 buf, len - buffer and size of buffer to stuff id into.
 *	Returns: an allocated fully specified node id as a string.
 */

static ResourceError
WidgetToID(w, buf, len)
Widget w;
char * buf;
int len;
{
    register int local_len, t_len = 1;
    register char * ptr, temp[51];

    for (ptr = buf ; w != NULL; w = XtParent(w)) {
	    
	sprintf(temp, "%c%ld", NAME_SEPARATOR, (long) w);
	
	local_len = strlen(temp);

	if (t_len + local_len >= len) {
	    sprintf(buf, "%s to fit entire name into buffer of length %d",
		    "Editres Client: Unable", len);
	    return(UnformattedResError);
	}
	strcpy(ptr, temp);
	
	ptr += local_len;
	t_len += local_len;
    }
    return(NoResError);
}

/*	Function Name: PositionInChild
 *	Description: returns true if this location is in the child.
 *	Arguments: child - the child widget to check.
 *                 x, y - location of point to check in the parent's
 *                        coord space.
 *	Returns: TRUE if the position is in this child.
 */

static Boolean
PositionInChild(child, x, y)
Widget child;
int x, y;
{
    Arg args[6];
    Cardinal num;
    Dimension width, height, border_width;
    Position child_x, child_y;
    Boolean mapped_when_managed;

    if (!XtIsRectObj(child))	/* we must at least be a rect obj. */
	return(FALSE);

    num = 0;
    XtSetArg(args[num], XtNmappedWhenManaged, &mapped_when_managed); num++;
    XtSetArg(args[num], XtNwidth, &width); num++;
    XtSetArg(args[num], XtNheight, &height); num++;
    XtSetArg(args[num], XtNx, &child_x); num++;
    XtSetArg(args[num], XtNy, &child_y); num++;
    XtSetArg(args[num], XtNborderWidth, &border_width); num++;
    XtGetValues(child, args, num);
 
    /*
     * The only way we will know of the widget is mapped is to see if
     * mapped when managed is True and this is a managed child.  Otherwise
     * we will have to ask the server if this window is mapped.
     */

    if (XtIsWidget(child) && !(mapped_when_managed && XtIsManaged(child)) ) {
	XWindowAttributes attrs;

	if (XGetWindowAttributes(XtDisplay(child), 
				 XtWindow(child), &attrs) != 0) {
	    /* oops */
	}
	else if (attrs.map_state != IsViewable)
	    return(FALSE);
    }

    return( (x >= child_x) && (x <= (child_x + width + 2 * border_width)) &&
	    (y >= child_y) && (y <= (child_y + height + 2 * border_width)) );
}

/*	Function Name: FindChild
 *	Description: Finds the child that actually contatians the point shown.
 *	Arguments: parent - a widget that is known to contain the point
 *                 	    specified.
 *                 x, y - The point in coordinates relative to the 
 *                        widget specified.
 *	Returns: none.
 */

static Widget 
FindChild(parent, x, y)
Widget parent;
int x, y;
{
    Widget * children;
    int i = FindAllChildren(parent, &children);

    while (i > 0) {
	i--;

	if (PositionInChild(children[i], x, y)) {
	    Widget child = children[i];
	    
	    XtFree(children);
	    return(FindChild(child, x - child->core.x, y - child->core.y));
	}
    }

    XtFree(children);
    return(parent);
}

/*	Function Name: DoFindChild
 *	Description: finds the child that contains the location specified.
 *	Arguments: w - any widget in the widget tree.
 *                 str - command string.
 *                 ret_val - a string to stuff the return value into.
 *	Returns: the error code.
 */

static ResourceError
DoFindChild(w, str, ret_val)
Widget w;
String str, ret_val;
{
    Message msg;
    char * parse_string;
    Widget parent, child;
    static ResourceError WidgetToID();
    int x_loc, y_loc, mask;
    unsigned int junk;
    Position parent_x, parent_y;

    bzero((char *) &msg, sizeof(Message)); /* initialize to zero. */

    if ((parent = VerifyWidget(w, str, &msg, NAME_VAL_SEPARATOR)) == NULL)
    {
	strcpy(ret_val, msg.str);
	XtFree(msg.str);
	return(UnformattedResError);
    }

    /*
     * There should never be NULL returned here, because VerifyWidget()
     * already checked to make sure a NAME_VAL_SEPARATOR was found.
     */

    parse_string = index(str, NAME_VAL_SEPARATOR) + 1;

    mask = XParseGeometry(parse_string, &x_loc, &y_loc, &junk, &junk);

    if ( !(mask & XValue) || !(mask & YValue)) {
	sprintf(ret_val, "%s `%s' %s %s",
		"The geometry string", parse_string, "passed to the",
		"client process does not contain an X and Y value.");
	return(UnformattedResError);
    }

    XtTranslateCoords(parent, (Position) 0, (Position) 0,
		      &parent_x, &parent_y);
    
    child = FindChild(parent, x_loc - (int) parent_x, y_loc - (int) parent_y);
    
    return(WidgetToID(child, ret_val, BUFSIZ));
}
