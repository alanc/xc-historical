#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Dialog.h>

#include "editresP.h"

extern void PopupCentered(), PerformTreeToFileDump();


/*	Function Name: ShowMessage(w, str)
 *	Description: shows the message to the user.
 *	Arguments: w - a label widget to show the message in.
 *                 str - the string to show.
 *	Returns: none.
 */

void
SetMessage(w, str)
Widget w;
char * str;
{
    Arg args[1];

    XtSetArg(args[0], XtNlabel, str);
    XtSetValues(w, args, ONE);
}

/*	Function Name: GetAllStrings
 *	Description: Returns a list of strings that have been borken up by
 *                   the character specified.
 *	Arguments: in - the string to parse.
 *                 sep - the separator character.
 *                 out - the strings to send out.
 *                 num - the number of strings in out.
 *	Returns: none
 */

void
GetAllStrings(in, sep, out, num)
char *in, sep, ***out;
int * num;
{
    int size, i;
    char * ptr;

    if (*in == sep)		/* jump over first char if it is the sep. */
	in++;

    /*
     * count the number of strings.
     */

    for (*num = 1, ptr = in; (ptr = index(ptr, sep)) != NULL; (*num)++)
	ptr++;

/*
 * Create Enough space for pointers and string.
 */

    size = (sizeof(char *) * *num) + (sizeof(char) * (strlen(in) + 1));
    *out = (char **) XtMalloc( (Cardinal) size);

    ptr = (char *) (*out + *num);
    strcpy(ptr, in);

/*
 * Change all `sep' characters to '\0' and stuff the pointer into
 * the next pointer slot.
 */

    i = 1;
    (*out)[0] = ptr;
    while (TRUE) {
	if ((ptr = index(ptr, sep)) == NULL)
	    break;

	*ptr++ = '\0';
	(*out)[i++] = ptr;
    }

/*
 * If last string is empty then strip it off.
 */

    if ( *((*out)[i - 1]) == '\0' )
	(*num)--;
}

/*	Function Name: AddString
 *	Description: Mallocs and strcats the string onto the end of
 *                   the given string.
 *	Arguments: str - string to add on to.
 *                 add - string to add.
 *	Returns: none.
 */

void
AddString(str, add)
char ** str, *add;
{
    int len_str, len_add;
    char * ptr;

    if (*str == NULL)
	len_str = NULL;
    else
	len_str = strlen(*str);

    len_add = strlen(add);

    *str = XtRealloc(*str, sizeof(char) * (len_str + len_add + 1));
    ptr = *str + len_str;
    strcpy(ptr, add);
}
 
/*	Function Name: FindNode
 *	Description: Finds a node give the top node, and a node id number.
 *	Arguments: top_node - the top node.
 *                 id - the node id.
 *	Returns: node.
 */

WNode *
FindNode(top_node, ids, number)
WNode *top_node;
unsigned long * ids;
Cardinal number;
{
    int i, j;
    WNode *node;

    if (top_node == NULL)
	return(NULL);

    if (ids[0] != top_node->id)
	return(NULL);

    for (node = top_node, i = 1 ; i < number; i++) {
	Boolean found_it = FALSE;

	for (j = 0; j < node->num_children; j++) {
	    if (node->children[j]->id == ids[i]) {
		node = node->children[j];
		found_it = TRUE;
		break;
	    }
	}
	if (!found_it)
	    return(NULL);
    }	    
    return(node);
}

/*	Function Name: FindWidgetFromWindow
 *	Description: finds a widget in the current tree given its window id.
 *	Arguments: tree_info - information about this tree.
 *                 win - window to search for.
 *	Returns: node - the node corrosponding to this widget.
 */

WNode * 
FindWidgetFromWindow(tree_info, win)
TreeInfo * tree_info;
Window win;
{
    static WNode * FindWidgetFromWindowGivenNode();

    if (tree_info == NULL)
	return(NULL);

    return(FindWidgetFromWindowGivenNode(tree_info->top_node, win));
}

/*	Function Name: FindWidgetFromWindowGivenNode
 *	Description: finds a widget in the current tree given its window id.
 *	Arguments: node - current node.
 *                 win - window to search for.
 *	Returns: node - the node corrosponding to this widget.
 */

static WNode *
FindWidgetFromWindowGivenNode(node, win)
WNode * node;
Window win;
{
    int i;
    WNode * ret_node;

    if (node->window == win)
	return(node);

    for (i = 0; i < node->num_children; i++) {
	ret_node = FindWidgetFromWindowGivenNode(node->children[i], win);
	if (ret_node != NULL)
	    return(ret_node);
    }
    return(NULL);
}

/*	Function Name: NodeToID
 *	Description: gets the fully specified node id as a string.
 *	Arguments: node - node to work on.
 *	Returns: an allocated fully specified node id as a string.
 */

#define MORE_MEM 200

char * 
NodeToID(node)
WNode * node;
{
    int t_len = 1, alloc = 0;
    char * str = NULL;

    for ( ; node != NULL; node = node->parent) {
	char buf[BUFSIZ], *ptr;
	Cardinal len;
	    
	sprintf(buf, "%c%ld", NAME_SEPARATOR, node->id);
	
	len = strlen(buf);
	
	if (t_len + len >= alloc) {
	    alloc += (len > MORE_MEM) ? len : MORE_MEM;
	    str = XtRealloc(str, sizeof(char) * alloc);
	}
	    
	ptr = str + t_len - 1;
	strcpy(ptr, buf);
	
	t_len += len;
    }
    return(str);
}

/*	Function Name: IDToNode
 *	Description: gets the node pointer given the id.
 *	Arguments: top_node - the top_node of this tree.
 *                 str - string containng the node id.
 *	Returns: the node pointer of this node.
 */

WNode * 
IDToNode(top_node, str)
WNode * top_node;
char * str;
{
    char **names;
    int num_names, j;
    unsigned long * ids;
    WNode *FindNode(), *node;

    GetAllStrings(str, NAME_SEPARATOR, &names, &num_names);
    
    ids = (unsigned long *) XtMalloc(sizeof(unsigned long) * num_names);
	    
    /*
     * Reverse the order, and store as unsigned longs.
     */
    
    for (j = num_names - 1; j >= 0; j--)
	ids[j] = atol(names[num_names - j - 1]);
    
    node = FindNode(top_node, ids, (Cardinal) num_names);
    
    XtFree(ids);
    XtFree(names);

    return(node);
}

/*	Function Name: HandleXErrors
 *	Description: Handles error codes from the server.
 *	Arguments: display - the display.
 *                 error - error information.
 *	Returns: none.
 */

/* ARGSUSED */
int
HandleXErrors(display, error)
Display * display;
XErrorEvent * error;
{
    if (error->serial != global_serial_num) {
	(*global_old_error_handler) (display, error);
	return(0);
    }

    if (error->error_code == BadWindow)
	global_error_code = NO_WINDOW;    
    else {
	if (XmuPrintDefaultErrorMessage(display, error, stderr) != 0)
	    exit(1);
    }
    return(0);
}

/*	Function Name: _DumpTreeToFile
 *	Description: Dumps the widget tree to a file
 *	Arguments: w - a random widget in the application on the
 *                     currently active display
 *                 tree_ptr - pointer to the widget tree info.
 *                 filename - name of the file.
 *	Returns: none.
 */

/* ARGSUSED */

void
_DumpTreeToFile(w, tree_ptr, filename)
Widget w;
XtPointer tree_ptr;
char * filename;
{
    TreeInfo * tree_info = (TreeInfo *) tree_ptr;
    FILE * fp; 

    if (tree_info == NULL) {
	SetMessage(global_screen_data.info_label,
		   "No widget Tree is avaliable.");
	return;
    }

    if ( (fp = fopen(filename, "w")) == NULL ) {
	char buf[BUFSIZ];

	sprintf(buf, "Unable to open the file `%s' for writing.");
	SetMessage(global_screen_data.info_label, buf);
	return;
    }

    PerformTreeToFileDump(tree_info->top_node, 0, fp);
    fclose(fp);
}

/************************************************************
 * 
 * The file dialog boxes are handled with this code.
 *
 * It automatically calls the function specified when the
 * user selects okay, or hits <CR>.
 *
 * A translation is required in the app-defaults file.
 *
 ************************************************************/

/*	Function Name: _PopupFileDialog
 *	Description: Puts up a dialog box to get the filename.
 *	Arguments: str - message.
 *                 func - function to call when filename has been entered.
 *                 data - generic data to pass to func.
 *	Returns: none
 */

static XContext file_dialog_context = None;

typedef struct _FileDialogInfo {
    void (*func)();
    XtPointer data;
} FileDialogInfo;

void
_PopupFileDialog(w, str, func, data)
Widget w;
String str;
void (*func)();
XtPointer data;
{
    FileDialogInfo * file_info;
    Widget shell, dialog;
    Arg args[1];
    void _PopdownFileDialog();

    if (file_dialog_context == None)
	file_dialog_context = XUniqueContext();

    shell = XtCreatePopupShell("fileDialog", transientShellWidgetClass, w,
			       NULL, ZERO);

    XtSetArg(args[0], XtNlabel, str);
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, 
				   shell, args, ONE);

    file_info = XtNew(FileDialogInfo);

    file_info->func = func;
    file_info->data = data;

    if  (XSaveContext(XtDisplay(dialog), (Window) dialog, file_dialog_context, 
		      (caddr_t) file_info) != 0) {
	SetMessage(global_screen_data.info_label,
	    "Error while trying to save Context\nAborting file dialog popup.");
	XtDestroyWidget(shell);
	return;
    }

    XawDialogAddButton(dialog, "okay", _PopdownFileDialog, (XtPointer) TRUE);
    XawDialogAddButton(dialog, "cancel", _PopdownFileDialog,(XtPointer) FALSE);

    PopupCentered(NULL, shell, XtGrabNone);
}

/*	Function Name: PopupCentered
 *	Description: Pops up the window specified under the location passed
 *                   in the event, or under the cursor.
 *	Arguments: event - the event that we should use.
 *                 w - widget to popup.
 *                 mode - mode to pop it up in.
 *	Returns: none
 */

void
PopupCentered(event, w, mode)
XEvent * event;
Widget w;
XtGrabKind mode;
{
    Boolean get_from_cursor = FALSE;
    Arg args[3];
    Cardinal num_args;
    Dimension width, height, b_width;
    int x, y, max_x, max_y;

    XtRealizeWidget(w);

    if (event == NULL)
	get_from_cursor = TRUE;
    else {
	switch (event->type) {
	case ButtonPress:
	case ButtonRelease:
	    x = event->xbutton.x_root;
	    y = event->xbutton.y_root;
	    break;
	case KeyPress:
	case KeyRelease:
	    x = event->xkey.x_root;
	    y = event->xkey.y_root;
	    break;
	default:
	    get_from_cursor = TRUE;
	    break;
	}
    }

    if (get_from_cursor) {
	Window root, child;
	int win_x, win_y;
	unsigned int mask;
	
	XQueryPointer(XtDisplay(w), XtWindow(w),
		      &root, &child, &x, &y, &win_x, &win_y, &mask);
    }

    num_args = 0;
    XtSetArg(args[num_args], XtNwidth, &width); num_args++;
    XtSetArg(args[num_args], XtNheight, &height); num_args++;
    XtSetArg(args[num_args], XtNborderWidth, &b_width); num_args++;
    XtGetValues(w, args, num_args);

    width += 2 * b_width;
    height += 2 * b_width;

    x -= ((int) width/2);
    if (x < 0) 
	x = 0;
    if ( x > (max_x = (int) (XtScreen(w)->width - width)) )
	x = max_x;

    y -= ( (Position) height/2 );
    if (y < 0) 
	y = 0;
    if ( y > (max_y = (int) (XtScreen(w)->height - height)) )
	y = max_y;
  
    num_args = 0;
    XtSetArg(args[num_args], XtNx, x); num_args++;
    XtSetArg(args[num_args], XtNy, y); num_args++;
    XtSetValues(w, args, num_args);

    XtPopup(w, mode);
}

/*	Function Name: _PopdownFileDialog
 *	Description: Destroys the file dialog, and calls the correct function.
 *	Arguments:  w - a child of the dialog widget.
 *                  client_data - TRUE if command was sucessful.
 *                  junk - ** UNUSED **.
 *	Returns: none.
 */

/* ARGSUSED */

void 
_PopdownFileDialog(w, client_data, junk)
Widget w;
XtPointer client_data, junk;
{
    Widget dialog = XtParent(w);
    caddr_t file_info_ptr;
    FileDialogInfo * file_info;

    if (XFindContext(XtDisplay(dialog), (Window) dialog, file_dialog_context,
		     &file_info_ptr) == XCNOENT) {
	SetMessage(global_screen_data.info_label,
		   "Error while trying to find Context\nAborting...");	
    }

    (void) XDeleteContext(XtDisplay(dialog), (Window)dialog, 
			  file_dialog_context);

    file_info = (FileDialogInfo *) file_info_ptr;

    if ( ((Boolean) client_data) == TRUE ) {
	String filename = XawDialogGetValueString(dialog);

	(*file_info->func)(w, file_info->data, filename); /* call handler */
    }

    XtFree( (XtPointer) file_info); /* Free data. */

    XtPopdown(XtParent(dialog));
    XtDestroyWidget(XtParent(dialog)); /* Remove file dialog. */
}
