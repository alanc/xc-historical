/*
 * $XConsortium: EditResCom.c,v 1.3 90/03/05 17:51:42 kit Exp $
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
#include <X11/Xaw/Text.h>

#define streq(a,b) (strcmp( (a), (b) ) == 0)

static Atom res_editor_command, client_value;

static char * global_selection_command;

typedef struct _ErrorMessage {
    char * str;			/* The Error String. */
    int len, alloc;		/*length of message, and amount alloced space*/
    Boolean formatted;		/* is this a formatted error message. */
} ErrorMessage;

static ErrorMessage * global_error;
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
	res_editor = XInternAtom(disp, RES_EDITOR_NAME, False);
	res_editor_command = XInternAtom(disp, RES_EDITOR_COMMAND_ATOM, False);

	/*
	 * Used in later proceedures. 
	 */

	client_value = XInternAtom(disp, RES_EDITOR_CLIENT_VALUE, False);
    }

    if (event->type == ClientMessage) {
	XClientMessageEvent * c_event = (XClientMessageEvent *) event;

	if ((c_event->message_type != res_editor) ||
	    (c_event->format != RES_EDIT_SEND_EVENT_FORMAT))
	    return;

	disp = XtDisplay(w);
	time = c_event->data.l[0];
	res_comm = c_event->data.l[1];
	ident = (ResIdent) c_event->data.l[2];

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

    if ( (*type != XA_STRING) || (*format != RES_EDITOR_FORMAT) )
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
 *	Description: Executes a command string received from
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
    static void LoseReturnCommand();
    char * DumpWidgets(), *comm_str;

    if (streq(SEND_WIDGET_TREE, command)) {
	comm_str = DumpWidgets(w);

	SendReturnCommand(w, sel, ident, comm_str);
	XtFree(comm_str);
    }
    else if (streq(SET_VALUES, command)) {
	static void DoSetValues();
	ErrorMessage error_message;
       
	bzero((char *) &error_message, sizeof(ErrorMessage));

	DoSetValues(w, value, &error_message);

	if (error_message.str == NULL) 
	    SendReturnCommand(w, sel, ident, "SetValues was sucessful.");
	else {
	    SendCommand(w, sel, ident, 
			 ((error_message.formatted) ? FormattedResError 
			                            :UnformattedResError),
			 error_message.str);	    
	    XtFree(error_message.str);
	}
    }
    else {
	char error_buf[BUFSIZ];

	sprintf(error_buf, "Unknown Command: %s", command);
	SendBackError(w, sel, ident, error_buf);
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
    *format_ret = RES_EDITOR_FORMAT;

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
ErrorMessage * error;
{
    Widget top;
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

    for (top = w; XtParent(top) != NULL; top = XtParent(top));

    for (i = 0; i < num_commands; i++) {
	ExecuteSetValues(top, commands[i], error);
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
    Cardinal i;

    if (parent == NULL)
	return(top == child);

    if (XtIsComposite(parent)) {
	CompositeWidget cw = (CompositeWidget) parent;

	for (i = 0; i < cw->composite.num_children; i++) 
	    if (cw->composite.children[i] == child)
		return(TRUE);
    }

    if (!XtIsWidget(parent))
	return(FALSE);

    for (i = 0; i < parent->core.num_popups; i++) 
	if (parent->core.popup_list[i] == child)
	    return(TRUE);

    if (XtIsSubclass(parent, textWidgetClass)) {
	Arg args[2];
	Widget sink, source;
	
	XtSetArg(args[0], XtNtextSink, &sink);
	XtSetArg(args[1], XtNtextSource, &source);
	XtGetValues(parent, args, TWO);
	
	if ((sink == child) || (source == child))
	    return(TRUE);
    }

    return(FALSE);
}

/*	Function Name: ExecuteSetValues
 *	Description: Performs a setvalues for a given command.
 *	Arguments: top - top of the widget tree.
 *                 command - command to execute.
 *                 error - the error message info.
 *	Returns: NULL if all widgets check out, otherwise an allocated 
 *               error message.
 */

static void
ExecuteSetValues(top, command, error)
Widget top;
char * command;
ErrorMessage * error;
{
    Widget widget;
    static Boolean VerifyWidget();
    static void HandleToolkitErrors();
    XtErrorMsgHandler old;
    char buf[100], *begin,* end, *bp, name[100], value[100];

    if (!VerifyWidget(top, command, error)) 
	return;

    begin = index(command, NAME_SEPARATOR);
    begin++;

    end = index(begin, NAME_SEPARATOR);
    if (end == NULL) {
	end = index(begin, WID_RES_SEPARATOR);
	if (end == NULL) {
	    char error_buf[BUFSIZ];
	    sprintf(error_buf, "Improperly formatted SetValues %s`%c'.", 
		    "Command, could not find a ", WID_RES_SEPARATOR);
	    AddError(error, NULL, error_buf);
	    return;
	}
    }

    for ( bp = buf; begin < end; begin++, bp++)
	*bp = *begin;
    *bp = '\0';

    widget = (Widget) atol(buf);

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
 *	Arguments: top - the root of the widget tree.
 *                 command - the command containing the widget list to verify.
 *                 error - the error message info.
 *	Returns: TRUE of widget exists.
 */

static Boolean
VerifyWidget(top, command, error)
Widget top;
char * command;
ErrorMessage * error;
{
    static Boolean IsChild();
    char buf[BUFSIZ], *ptr, *end;
    Widget parent, child; 

    strcpy(buf, command);

    if ((end = index(buf, WID_RES_SEPARATOR)) == NULL) {
	sprintf(buf, "Formatting error is SetValues, no %d %s",
		WID_RES_SEPARATOR, "in SetValues command.");
	AddError(error, command, buf);
	return(FALSE);
    }
    
    *end = '\0';
    
    parent = NULL;
    while (TRUE) {
	if ( (ptr = rindex(buf,  NAME_SEPARATOR)) == NULL) {
	    sprintf(buf, "Formatting error is SetValues, no %d %s",
		    NAME_SEPARATOR, "in SetValues command.");
	    AddError(error, command, buf);
	    return(FALSE);
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
    return(TRUE);
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
    int i;
    char my_name[BUFSIZ * 3];	/* beats me how big it should be... */
    static void DumpName();

    DumpName(w, parent_name, list, end, bytes);

    if (XtName(w) == NULL)
	sprintf(my_name, "%s%c%c%ld", parent_name, NAME_SEPARATOR,
		ID_SEPARATOR, (unsigned long) w);
    else
	sprintf(my_name, "%s%c%s%c%ld", parent_name, NAME_SEPARATOR,
		XtName(w), ID_SEPARATOR, (unsigned long) w);

    /*
     * Recursively call this function on all out children and popup children.
     */

    if (XtIsWidget(w)) {
	if (XtIsComposite(w)) {
	    CompositeWidget cw = (CompositeWidget) w;

	    for (i = 0; i < cw->composite.num_children; i++) 
		DumpChildren(cw->composite.children[i], my_name, list, 
			     end, bytes);
	}

	for (i = 0; i < w->core.num_popups; i++) 
	    DumpChildren(w->core.popup_list[i], my_name, list, end, bytes);

	if (XtIsSubclass(w, textWidgetClass)) {
	    Arg args[2];
	    Widget sink, source;
	    
	    XtSetArg(args[0], XtNtextSink, &sink);
	    XtSetArg(args[1], XtNtextSource, &source);
	    XtGetValues(w, args, TWO);

	    if (sink != NULL) 
		DumpChildren(sink, my_name, list, end, bytes);
	    if (source != NULL)
		DumpChildren(source, my_name, list, end, bytes);
	}
    }
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
	sprintf(window, "%ld", (long) XtWindow(w));
    else
	strcpy(window, "0");

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
 
/*	Function Name: AddError
 *	Description: Add a string to the error message in the form:
 *                   <widget id> - <message>
 *	Arguments: error - the error structure.
 *                 widget - the string contatining the widget ids.
 *                 str - the message to append to the buffer.
 *	Returns: none.
 */

static void
AddError(error, widget, str) 
ErrorMessage * error;
char * widget, *str;
{
    int len;
    char buf[BUFSIZ], *ptr, *end;

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

    len = strlen(buf) + 1;	/* leave space for extra '\n' */

    if ((error->len + len) >= error->alloc) {
	error->alloc += ((len + 2) > BUFSIZ) ? (len + 2) : BUFSIZ;
	error->str = XtRealloc(error->str, error->alloc * sizeof(char));
    }

    ptr = error->str + error->len;
    error->len += len;

    sprintf(ptr, "%s\n", buf);
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
