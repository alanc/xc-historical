#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Cardinals.h>

#include "editresP.h"

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
