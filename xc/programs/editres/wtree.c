#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>	
#include <X11/Xaw/Tree.h>

#include "editresP.h"

extern ScreenData global_screen_data;
extern void SetMessage();

static Boolean IsActiveNode();
static Cardinal ParseString();
static void AddChild();

extern TreeInfo *global_tree_info;

/*	Function Name: BuildVisualTree
 *	Description: Creates the Tree and shows it.
 *	Arguments: tree_parent - parent of the tree widget.
 *                 str - the widget tree as a string.
 *	Returns: none.
 */

void
BuildVisualTree(tree_parent, str)
Widget tree_parent;
char * str;
{
    WNode * top;
    TreeInfo *CreateTree();
    void AddTreeNode();
    Widget local;
    char msg[BUFSIZ];

    if (global_tree_info != NULL) {
	XtDestroyWidget(XtParent(global_tree_info->tree_widget));
	XtFree(global_tree_info->active_nodes);
	XtFree(global_tree_info);
    }

    global_tree_info = CreateTree(str);
    top = global_tree_info->top_node;

    local = XtCreateWidget("viewport", viewportWidgetClass, tree_parent,
			   NULL, ZERO);

    global_tree_info->tree_widget = XtCreateManagedWidget("tree", 
							  treeWidgetClass,
							  local, NULL, ZERO);

    AddTreeNode(global_tree_info->tree_widget, top);

    if (XtIsRealized(tree_parent))
	XtRealizeWidget(XtParent(global_tree_info->tree_widget));
    XtManageChild(XtParent(global_tree_info->tree_widget));

    sprintf(msg, "Widget Tree for client %s(%s).", top->name, top->class);
    SetMessage(global_screen_data.info_label, msg);
}

/*	Function Name: AddTreeNode
 *	Description: Adds all nodes below this to the Tree widget.
 *	Arguments: parent - parent of the tree widget.
 *                 top - the top node of the tree.
 *	Returns: the tree widget.
 *
 * NOTE: This is a recursive function.
 */

void
AddTreeNode(tree, top) 
Widget tree;
WNode * top;
{
    int i;
    Arg args[10];
    Cardinal num_args = 0;
    static void TreeToggle();

    if (top->parent != NULL) {
	if (top->parent->widget == NULL) {
	    char msg[BUFSIZ];
	    sprintf( msg, "Loop in tree: node %s's parent (%s) has %s.\n",
		    top->name, top->parent->name, "not been created yet");
	    SetMessage(global_screen_data.info_label, msg);
	}
	XtSetArg(args[num_args], XtNtreeParent, top->parent->widget);
	num_args++;
    }

    top->widget = XtCreateManagedWidget(top->name, toggleWidgetClass, tree,
					args, num_args);
    XtAddCallback(top->widget, XtNcallback, TreeToggle, (XtPointer) top);

    for (i = 0; i < top->num_children; i++) 
	AddTreeNode(tree, top->children[i]);
}

/*	Function Name: TreeToggle
 *	Description: Called whenever a tree node is toggled.
 *	Arguments: w - the tree widget.
 *                 node_ptr - pointer to this node's information.
 *                 state_ptr - state of the toggle.
 *	Returns: none.
 */

/* ARGSUSED */
static void
TreeToggle(w, node_ptr, state_ptr)
Widget w;
caddr_t node_ptr, state_ptr;
{
    static void AddNodeToActiveList(), RemoveNodeFromActiveList();
    Boolean state = (Boolean) state_ptr;
    WNode * node = (WNode *) node_ptr;

    if (state) 
	AddNodeToActiveList(node);
    else
	RemoveNodeFromActiveList(node);
}

/*	Function Name: GetAllActiveTreeEntries
 *	Description: returns a fully specified id list for each active.
 *	Arguments: tree_info - the tree info structure.
 *                 entries - the list of active entries *** RETURNED ***
 *                 num_entries - the number of active entries *** RETURNED ***
 *	Returns: none.
 */

#define MORE_MEM 200

void
GetAllActiveTreeEntries(tree_info, entries, num_entries)
TreeInfo * tree_info;
char *** entries;
Cardinal * num_entries;
{
    WNode * node;
    Cardinal i, t_len, alloc;

    *num_entries = tree_info->num_nodes;
    *entries = (char **) XtMalloc(sizeof(char *) * (*num_entries));

    for (i = 0; i < *num_entries; i++) {
	t_len = 1;
	alloc = 0;
	node = tree_info->active_nodes[i];
	(*entries)[i] = NULL;

	for ( ; node != NULL; node = node->parent) {
	    char buf[BUFSIZ], *ptr;
	    Cardinal len;
	    
	    sprintf(buf, "%c%ld", NAME_SEPARATOR, node->id);

	    len = strlen(buf);

	    if (t_len + len >= alloc) {
		alloc += (len > MORE_MEM) ? len : MORE_MEM;
		(*entries)[i] = XtRealloc((*entries)[i], sizeof(char) * alloc);
	    }
	    
	    ptr = (*entries)[i] + t_len - 1;
	    strcpy(ptr, buf);
	
	    t_len += len;
	}
    }
}

/*	Function Name: AddNodeToActiveList
 *	Description: Adds this node to the list of active toggles.
 *	Arguments: node - node to add.
 *	Returns: none.
 */

static void
AddNodeToActiveList(node)
WNode * node;
{
    TreeInfo * info = node->tree_info;

    if (IsActiveNode(node))	/* node already active. */
	return;

    if (info->num_nodes >= info->alloc_nodes) {
	info->alloc_nodes += NUM_INC;
	info->active_nodes =(WNode **)XtRealloc((XtPointer) info->active_nodes,
						sizeof(WNode *) * 
						     info->alloc_nodes);
    }

    info->active_nodes[info->num_nodes++] = node;
}

/*	Function Name: RemoveNodeFromActiveList
 *	Description: Removes a node from the active list.
 *	Arguments: node - node to remove.
 *	Returns: none.
 */

static void
RemoveNodeFromActiveList(node)
WNode * node;
{
    TreeInfo * info = node->tree_info;
    Boolean found_node = FALSE;
    int i;

    if (!IsActiveNode(node))	/* This node is not active. */
	return;

    for (i = 0; i < info->num_nodes; i++) {
	if (found_node)
	    info->active_nodes[i - 1] = info->active_nodes[i];
	else if (info->active_nodes[i] == node) 
	    found_node = TRUE;
    }

    info->num_nodes--;
}

/*	Function Name: IsActiveNode
 *	Description: returns TRUE is this node is on the active list.
 *	Arguments: node - node to check.
 *	Returns: see above.
 */

static Boolean
IsActiveNode(node)
WNode * node;
{
    TreeInfo * info = node->tree_info;
    int i;

    for (i = 0; i < info->num_nodes; i++) 
	if (info->active_nodes[i] == node)
	    return(TRUE);

    return(FALSE);
}
    
/*	Function Name: CreateTree
 *	Description: Creates a widget tree give a list of names and classes.
 *	Arguments: str - string containing widget names and classes.
 *                 tree_info - global information on this tree.
 *	Returns: The tree_info about this new tree.
 *
 *               Sting is formatted in the format.
 *
 *               name\tid.name\tid.name\tid:Class\n
 */
    
TreeInfo *
CreateTree(str)
char * str;
{
    static void AddNode();
    void GetAllStrings();
    char ** strings;
    int num_strings, i;

    TreeInfo * tree_info;

    tree_info = (TreeInfo *) XtMalloc( (Cardinal) sizeof(TreeInfo));

    tree_info->tree_widget = NULL;
    tree_info->top_node = NULL;
    tree_info->active_nodes = NULL;
    tree_info->num_nodes = tree_info->alloc_nodes = 0;

    GetAllStrings(str, EOL_SEPARATOR, &strings, &num_strings);

    for ( i = 0; i < num_strings; i++)
	AddNode(&(tree_info->top_node), strings[i], tree_info);

    XtFree(strings);

    return(tree_info);
}

/*	Function Name: PrintNodes
 *	Description: Prints all nodes.
 *	Arguments: top - the top node.
 *	Returns: none.
 */

void
PrintNodes(top)
WNode * top;
{
    int i;

    if (top->parent == NULL) 
	printf("Top of Tree, Name: %10s, ID: %10ld, Class: %10s\n", 
	       top->name, top->id, top->class);
    else
	printf("Parent %10s, Name: %10s, ID: %10ld, Class: %10s\n", 
	       top->parent->name, top->name, top->id, top->class);

    for (i = 0; i < top->num_children; i++) 
	PrintNodes(top->children[i]);
}

/************************************************************
 *
 * Non - Exported Functions. 
 *
 ************************************************************/

WNode * FindNode();

/*	Function Name: AddNode
 *	Description: adds a node to the widget tree.
 *	Arguments: top_node - a pointer to the current top node.
 *                 str - string containing the name of this widget.
 *                 tree_info - global information on this tree.
 *	Returns: none.
 */

static void
AddNode(top_node, str, tree_info)
WNode ** top_node;
char *str;
TreeInfo * tree_info;
{
    WNode *node, *parent;
    char **names, *class;
    unsigned long *ids;
    Boolean early_break = FALSE;
    Cardinal number;

    if ((number = ParseString(str, &class, &names, &ids)) == 0)
	return;			/* parsing error. */

    if ( (node = FindNode(*top_node, ids, number)) == NULL) {
	node = (WNode *) XtCalloc(sizeof(WNode), ONE);

	node->class = class;	            /* already allocated. */
	node->name = names[number - 1];	    /* already allocated. */
	node->id = ids[number - 1];
	node->tree_info = tree_info;

	for ( number--; number > 0; number--, node = parent) {
	    if ( (parent = FindNode(*top_node, ids, number)) == NULL) {
		parent = (WNode *) XtCalloc(sizeof(WNode), ONE);
		parent->id = ids[number - 1];
		parent->name = names[number - 1];
	    }
	    else
		early_break = TRUE;

	    AddChild(parent, node);

	    if (early_break) 
		break;
	}

	if (!early_break) {
	    if (node->parent == NULL)
		*top_node = node;
	    else
		*top_node = node->parent;
	}
    }
    else
	node->class = class;

    XtFree(ids);
    XtFree(names);
}

/*	Function Name: ParseString
 *	Description: parses a string for the name and id of each widget.
 *                   Also returns the class of the specific widget.
 *	Arguments: str - string to parse.
 *                 class - pre-allocated string for class.
 *                 names, ids - list of names and ids.
 *	Returns: the number of names and ids.
 *
 * names/ids come out backward:   child, parent, grandparent...
 */

static Cardinal
ParseString(str, class, names, ids)
char * str, ** class, ***names;
unsigned long ** ids;
{
    int number, i;
    char * temp, *ptr, *bufp, buf[BUFSIZ];
    static Boolean GetNameAndID();

    /*
     * Strip off the class name.
     */
    
    if ((temp = rindex(str, CLASS_SEPARATOR)) == NULL) 
	return(0);
    else 
	*class = XtNewString(temp+1);

    for (bufp = buf, ptr = str; ptr < temp; ptr++, bufp++) 
	*bufp = *ptr;
    *bufp = '\0';
    
    GetAllStrings(buf, NAME_SEPARATOR, names, &number);

/*
 * Names are now good, all I have to do is collect the id numbers,
 * and I'll be all set.
 */

    *ids = (unsigned long *) XtMalloc(sizeof(unsigned long) * number);

    for (i = 0; i < number; i++) {
	if (!GetNameAndID(*names + i, *ids + i))
	    return(0);
    }
    return( (Cardinal) number);
}	
    
/*	Function Name: GetNameAndID
 *	Description: Gets the name and id of the last widget in a list.
 *	Arguments: name - name of node. 
 *                 id - id number of node as string.
 *	Returns: Returns FALSE if there is a parsing error.
 *
 * NOTES:  munges str.
 */
 
static Boolean
GetNameAndID(name, id)
char **name;
unsigned long *id;
{
    char * temp, msg[BUFSIZ];

    if ((temp = rindex(*name, ID_SEPARATOR)) == NULL) {
	sprintf( msg,
		"rightmost name of this string contains no ID_SEPARATOR: %s\n",
		*name);
	SetMessage(global_screen_data.info_label, msg);
	return(FALSE);
    }

    *temp++ = '\0';
    *id = atol(temp);
    *name = XtNewString(*name);
    return(TRUE);
}

/*	Function Name: AddChild
 *	Description: Adds a child to an existing node.
 *	Arguments: parent - parent node.
 *                 child - child node to add.
 *	Returns: none.
 */

static void
AddChild(parent, child)
WNode * parent, * child;
{
    if (parent->num_children >= parent->alloc_children) {
	parent->alloc_children += NUM_INC;
	parent->children = (WNode **) XtRealloc(parent->children, 
				     sizeof(WNode *) * parent->alloc_children);
    }

    parent->children[parent->num_children] = child;
    (parent->num_children)++;

    child->parent = parent;
}

/************************************************************
 *
 *  Functions that operate of the current tree.
 * 
 ************************************************************/

/*	Function Name: _TreeSelect
 *	Description: Modifies the selected elements of the tree
 *	Arguments: type - type of selection to perform
 *	Returns: none.
 */

void
_TreeSelect(type)
SelectTypes type;
{
    static void _TreeSelectNode();
    WNode * top;

    if (global_tree_info == NULL) 
	SetMessage(global_screen_data.info_label,
		   "No widget Tree is avaliable.");
    else {
	top = global_tree_info->top_node;
	_TreeSelectNode(top, type);
    }
}

/*	Function Name: _TreeSelectNode
 *	Description: Modifies the state of a node and all its decendants.
 *	Arguments: node - node to operate on.
 *                 type - type of selection to perform.
 *	Returns: none.
 */

static void
_TreeSelectNode(node, type)
WNode * node;
SelectTypes type;
{
    int i;
    Arg args[1];
    Boolean state;

    switch(type) {
    case SelectAll:
	state = TRUE;
	break;
    case SelectNone:
	state = FALSE;
	break;
    case SelectInvert:
	XtSetArg(args[0], XtNstate, &state);
	XtGetValues(node->widget, args, ONE);
	
	state = !state;
	break;
    default:
	SetMessage(global_screen_data.info_label,
		   "Internal Error: Unknown select type.");
	return;
    }

    XtSetArg(args[0], XtNstate, state);
    XtSetValues(node->widget, args, ONE);
    TreeToggle(node->widget, (XtPointer) node, (XtPointer) state);

    for (i = 0; i < node->num_children; i++) 
	_TreeSelectNode(node->children[i], type);
}

/*	Function Name: _TreeRelabel
 *	Description: Modifies the selected elements of the tree
 *	Arguments: type - type of selection to perform
 *	Returns: none.
 */

void
_TreeRelabel(type)
LabelTypes type;
{
    static void _TreeRelabelNode();
    WNode * top;

    if (global_tree_info == NULL) {
	SetMessage(global_screen_data.info_label,
		   "No widget Tree is avaliable.");
	return;
    }

    top = global_tree_info->top_node;

    XtUnmapWidget(global_tree_info->tree_widget);
    _TreeRelabelNode(top, type);
    XtMapWidget(global_tree_info->tree_widget);
}

/*	Function Name: _TreeRelabelNodes
 *	Description: Modifies the node and all its decendants label.
 *	Arguments: node - node to operate on.
 *                 type - type of selection to perform.
 *	Returns: none.
 */

static void
_TreeRelabelNode(node, type)
WNode * node;
LabelTypes type;
{
    int i;
    Arg args[1];
    char buf[30];

    switch(type) {
    case ClassLabel:
	XtSetArg(args[0], XtNlabel, node->class);
	break;
    case NameLabel:
	XtSetArg(args[0], XtNlabel, node->name);
	break;
    case IDLabel:
	sprintf(buf, "0x%lx", node->id);
	XtSetArg(args[0], XtNlabel, buf);
	break;
    default:
	SetMessage(global_screen_data.info_label,
		   "Internal Error: Unknown label type.");
	return;
    }

    XtSetValues(node->widget, args, ONE);

    for (i = 0; i < node->num_children; i++) 
	_TreeRelabelNode(node->children[i], type);
}

/*	Function Name: CopyActiveNodes
 *	Description: returns a copy of the currently selected nodes.
 *	Arguments: tree_info - the tree info struct.
 *	Returns: a copy of the selected nodes.
 */

static WNode ** 
CopyActiveNodes(tree_info)
TreeInfo * tree_info;
{
    WNode ** list;
    int i;

    if ( (tree_info == NULL) || (tree_info->num_nodes == 0))
	return(NULL);

    list = (WNode **) XtMalloc(sizeof(WNode *) * tree_info->num_nodes);

    for (i = 0; i < tree_info->num_nodes; i++)
	list[i] = tree_info->active_nodes[i];

    return(list);
}

/*	Function Name: _TreeActivate
 *	Description: Activates relatives of the active nodes, as specified
 *                   by type.
 *	Arguments: type - type of activate to invode.
 *	Returns: none.
 */

void
_TreeActivate(type)
ActivateTypes type;
{
    static void _TreeActivateNode();
    WNode ** active_nodes;
    Cardinal num_active_nodes;
    int i;

    if (global_tree_info == NULL) {
	SetMessage(global_screen_data.info_label,
		   "No widget Tree is avaliable.");
	return;
    }

    if (global_tree_info->num_nodes == 0) {
	SetMessage(global_screen_data.info_label,
		   "There are no active nodes.");
	return;
    }

    active_nodes = CopyActiveNodes(global_tree_info);
    num_active_nodes = global_tree_info->num_nodes;

    for (i = 0; i < num_active_nodes; i++)
	_TreeActivateNode(active_nodes[i], type);

    XtFree((XtPointer) active_nodes);
}

/*	Function Name: _TreeActivateNode
 *	Description: Activates relatives of the node specfied, as specified
 *                   by type.
 *	Arguments: node - node to opererate on.
 *                 type - type of activate to invode.
 *	Returns: none.
 */

static void
_TreeActivateNode(node, type)
WNode * node;
ActivateTypes type;
{
    Arg args[1];
    int i;

    XtSetArg(args[0], XtNstate, TRUE);

    if ((type == ActivateParents) || (type == ActivateAncestors)) {
	node = node->parent;
	if (node == NULL)
	    return;

	XtSetValues(node->widget, args, ONE);	
	AddNodeToActiveList(node);

	if (type == ActivateAncestors)
	    _TreeActivateNode(node, type);	
    }
    else if ((type == ActivateChildren) || (type == ActivateDescendants)) 
	for (i = 0; i < node->num_children; i++) {
	    AddNodeToActiveList(node->children[i]);
	    XtSetValues(node->children[i]->widget, args, ONE);
	    if (type == ActivateDescendants)
		_TreeActivateNode(node->children[i], type);
	}
    else
	SetMessage(global_screen_data.info_label,
		   "Internal Error: Unknown activate type.");	
}
    
