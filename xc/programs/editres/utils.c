#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Dialog.h>

#include "editresP.h"

static WidgetResources * ParseResources();
static int CompareResourceEntries();

extern void PopupCentered(), PerformTreeToFileDump();

Boolean ParseOutWidgetInfo();
void 	CreateResourceBox();

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
 *                 default_value - the default value of the filename;
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
_PopupFileDialog(w, str, default_value, func, data)
Widget w;
String str, default_value;
void (*func)();
XtPointer data;
{
    FileDialogInfo * file_info;
    Widget shell, dialog;
    Arg args[2];
    Cardinal num_args;
    void _PopdownFileDialog();

    if (file_dialog_context == None)
	file_dialog_context = XUniqueContext();

    shell = XtCreatePopupShell("fileDialog", transientShellWidgetClass, w,
			       NULL, ZERO);

    num_args = 0;
    XtSetArg(args[num_args], XtNlabel, str); num_args++;
    XtSetArg(args[num_args], XtNvalue, default_value); num_args++;
    dialog = XtCreateManagedWidget("dialog", dialogWidgetClass, 
				   shell, args, num_args);

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

/************************************************************
 *
 * Functions for dealing with the Resource Box.
 *
 ************************************************************/

/*	Function Name: GetNamesAndClasses
 *	Description: Gets a list of names and classes for this widget.
 *	Arguments: node - this widget's node.
 *                 names, classes - list of names and classes. ** RETURNED **
 *	Returns: none.
 */

void
GetNamesAndClasses(node, names, classes) 
WNode * node;
char *** names, ***classes;
{
    int i, total_widgets;
    WNode * temp = node;

    for (total_widgets = 1 ; temp->parent != NULL ; 
	 total_widgets++, temp = temp->parent) { } /* empty for */

    *names = (char **) XtMalloc(sizeof(char *) * (total_widgets + 1));
    *classes = (char **) XtMalloc(sizeof(char *) * (total_widgets + 1));

    (*names)[total_widgets] = (*classes)[total_widgets] = NULL;

    for ( i = (total_widgets - 1); i >= 0 ; node = node->parent, i--) {
	(*names)[i] = node->name;
	(*classes)[i] = node->class;
    }
}

/*	Function Name: HandleGetResources
 *	Description: Gets the resources.
 *	Arguments: val - the return value from the client.
 *	Returns: none.
 */

char *
HandleGetResources(val)
char * val;
{
    char ** widget_strings, *data, * errors = NULL;
    int i, num;
    WNode * node;

    GetAllStrings(val, EOL_SEPARATOR, &widget_strings, &num);
    
    for (i = 0; i < num; i++) {
	if (ParseOutWidgetInfo(global_tree_info, &errors, widget_strings[i], 
			       &node, &data)) {
	    if (node->resources != NULL) 
		FreeResources(node->resources);
	    node->resources = ParseResources(data, &errors);
	    CreateResourceBox(node);
	}
    }

    XtFree(widget_strings);
    return(errors);
}

/*	Function Name: CreateResourceBox
 *	Description: Creates a resource box for the widget specified.
 *	Arguments: node - the node of the widget in question.
 *	Returns: none.
 */

void
CreateResourceBox(node)
WNode * node;
{
    void CreateResourceBoxWidgets();
    WidgetResources * resources = node->resources;
    char ** names, ** cons_names;
    int i;

    if (resources->num_normal > 0) {
	names = (char **) XtMalloc(sizeof(char *) *
				   (resources->num_normal + 1));
	for (i = 0 ; i < resources->num_normal ; i++) 
	    names[i] = resources->normal[i].name;
	names[i] = NULL;
    }
    else
	names = NULL;

    if (resources->num_constraint > 0) {
	cons_names = (char **) XtMalloc(sizeof(char *) *
					(resources->num_constraint + 1));
	
	for (i = 0 ; i < resources->num_constraint ; i++) 
	    cons_names[i] = resources->constraint[i].name;
	cons_names[i] = NULL;
    }
    else
	cons_names = NULL;

    CreateResourceBoxWidgets(node, names, cons_names);
}

/*	Function Name: ParseResources
 *	Description: Parses the resource values returned from the client
 *                   into a resources structure.
 *	Arguments: str - string containing resources for this widget.
 *	Returns: The resource information.
 */

static WidgetResources * 
ParseResources(str, error)
char * str, **error;
{
    WidgetResources * resources;
    WidgetResourceInfo * normal;
    char ** res_strings;
    int i, num;

    resources = (WidgetResources *) XtMalloc(sizeof(WidgetResources)); 
    
    /*
     * Allocate enough space for both the normal and constraint resources,
     * then add the normal resources from the top, and the constraint resources
     * from the bottom.  This assures that enough memory is allocated, and
     * that there is no overlap.
     */

    GetAllStrings(str, RESOURCE_SEPARATOR, &res_strings, &num);

    resources->normal = (WidgetResourceInfo *) 
	                           XtMalloc(sizeof(WidgetResourceInfo) * num);

    normal = resources->normal;
    resources->constraint = resources->normal + num - 1;

    resources->num_constraint = resources->num_normal = 0;

    for (i = 0; i < num; i++) {
	switch(res_strings[i][0]) {
	case 'n':
	    resources->num_normal++;
	    AddResource(res_strings[i] + 1, error, normal++);
	    break;
	case 'c':
	    resources->num_constraint++;
	    AddResource(res_strings[i] + 1, error, resources->constraint--);
	    break;
	default:
	    {
		char buf[BUFSIZ];
		sprintf(buf, "Improperly formatted resource value: '%s'", 
			res_strings[i]);
		AddString(error, buf);
	    }
	    break;
	}
    }

    /*
     * Sort the resources alphabetically. 
     */

    qsort(resources->normal, resources->num_normal,
	  sizeof(WidgetResourceInfo), CompareResourceEntries);

    if (resources->num_constraint > 0) {
	resources->constraint++;
	qsort(resources->constraint, resources->num_constraint,
	      sizeof(WidgetResourceInfo), CompareResourceEntries);
    }
    else
	resources->constraint = NULL;

    return(resources);
}

/*	Function Name: CompareResourceEntries
 *	Description: Compares two resource entries.
 *	Arguments: e1, e2 - the entries to compare.
 *	Returns: an integer >, < or = 0.
 */

static int 
CompareResourceEntries(e1, e2) 
WidgetResourceInfo *e1, *e2;
{
    return (strcmp(e1->name, e2->name));
}

/*	Function Name: AddResource
 *	Description: Parses the resource string a stuffs in individual
 *                   parts into the resource info struct.
 *	Arguments: str - the string to parse, format is below.
 *                 resource - location to stuff the resource into.
 *	Returns: none.
 *
 * Resources are of the form:
 *
 * <name>:<Class>#type
 */

static void
AddResource(str, error, resource) 
char * str;
char ** error;
WidgetResourceInfo * resource;
{
    char buf[BUFSIZ], *temp, *ptr;
    int len;

    if ((ptr = index(str, NAME_VAL_SEPARATOR)) == NULL) {
	sprintf("Error parsing resource value: '%s', must contain a %c",
		str, NAME_VAL_SEPARATOR);
	AddString(error, buf);
    }
    len = ptr - str;
    resource->name = XtMalloc(sizeof(char) * (len + 1));
    strncpy(resource->name, str, len);
    resource->name[len] = '\0';

    temp = ptr + 1;
    if ((ptr = index(temp, CLASS_TYPE_SEPARATOR)) == NULL) {
	sprintf("Error parsing resource value: '%s', must contain a %c",
		str, CLASS_TYPE_SEPARATOR);
	AddString(error, buf);
    }
    len = ptr - temp;
    resource->class = XtMalloc(sizeof(char) * (len + 1));
    strncpy(resource->class, temp, len);
    resource->class[len] = '\0';
    
    temp = ptr + 1;
    resource->type = XtNewString(temp);
}


/*	Function Name: FreeResources
 *	Description: frees the resource inforation.
 *	Arguments: resources.
 *	Returns: none.
 */

static void
FreeResources(resources) 
WidgetResources * resources;
{
    if (resources->num_normal > 0)
	XFree(resources->normal);
    XFree(resources);
}
	
/*	Function Name: ParseOutWidgetInfo
 *	Description: Parses out the widget info and adds the error code 
 *                   to the error string.
 *	Arguments: tree_info - the tree info.
 *                 error - the string containing the error message.
 *                 str - info to parse. 
 *                 node - node contained in this command.
 *                 data - the data associated with this command.
 *	Returns: True if no error occured and parse was sucessful, 
 *               False otherwise.
 *
 * str is in the form:
 *
 *            <e_val>[id]:<data>\0
 *
 *   e_val	= 1 if an error occured, 0 otherwise.
 *                NOTE: This field is exactly one character wide.
 *
 *   id 	= .<widgetid>.<parentid>.<grandparentid>...
 *
 *   data 	= <string>
 */

Boolean
ParseOutWidgetInfo(tree_info, error, str, node, data)
TreeInfo * tree_info;
char ** error, *str;
WNode ** node;
char ** data;
{
    char ** names, buf[BUFSIZ];
    int num_names;

    GetAllStrings(str + 1, NAME_VAL_SEPARATOR, &names, &num_names);
	    
    if ((*node = IDToNode(tree_info->top_node, names[0])) != NULL) {
	if (str[0] = '0') {	/* no error occured. */
	    *data = str + (names[1] - names[0]) + 1; /* A bit of a hack... 
						        but gets us the 
							correct data. */
	    XtFree(names);
	    return(TRUE);
	}
	
	/* 
	 * error code returned add message to 'error'.
	 */
	
	sprintf(buf, "%s(0x%lx) - %s\n", (*node)->name, (*node)->id, names[1]);
    }
    else 
	sprintf(buf, "%s - %s\n", names[0], names[1]);

    AddString(error, buf);
    XtFree(names);
    return(FALSE);
}

/*	Function Name: CheckDatabase
 *	Description: Checks to see if the node is in the database.
 *	Arguments: db - the db to check
 *                 names, clases - names and clases, represented as quarks.
 *	Returns: True if this entry is found.
 */

Boolean
CheckDatabase(db, names, classes)
XrmDatabase db;
XrmQuarkList names, classes;
{
    XrmRepresentation junk;
    XrmValue garbage;

    return(XrmQGetResource(db, names, classes, &junk, &garbage));
}

/*	Function Name: Quarkify
 *	Description: Quarkifies the string list specifed.
 *	Arguments: list - list of strings to quarkify
 *                 ptr - an additional string to quarkify.
 *	Returns: none.
 */

XrmQuarkList
Quarkify(list, ptr)
char ** list;
char * ptr;
{
    int i;
    char ** tlist;
    XrmQuarkList quarks, tquarks;

    for (i = 0, tlist = list; *tlist != NULL; tlist++, i++) {}
    if (ptr != NULL)
	i++;
    i++;			/* leave space for NULLQUARK */

    quarks = (XrmQuarkList) XtMalloc(sizeof(XrmQuark) * i);

    for (tlist = list, tquarks = quarks; *tlist != NULL; tlist++, tquarks++) 
	*tquarks = XrmStringToQuark(*tlist);

    if (ptr != NULL) 
	*tquarks++ = XrmStringToQuark(ptr);
	
    *tquarks = NULLQUARK;
    return(quarks);
}

/*	Function Name: ExecuteOverAllNodes
 *	Description: Executes the given function over all nodes.
 *	Arguments: top_node - top node of the tree.
 *                 func - the function to execute.
 *                 data - a data pointer to pass to the function.
 *	Returns: none
 */

void
ExecuteOverAllNodes(top_node, func, data)
WNode * top_node;
void (func)();
XtPointer data;
{
    int i;

    (*func)(top_node, data);

    for (i = 0; i < top_node->num_children; i++) 
	ExecuteOverAllNodes(top_node->children[i], func, data);
}
