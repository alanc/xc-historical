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

