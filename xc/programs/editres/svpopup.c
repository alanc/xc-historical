#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>	/* Get standard string definations. */
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Shell.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>	
#include <X11/Xaw/Command.h>	
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>	

#include "editresP.h"

extern void SetMessage(), SetCommand(), GetAllActiveTreeEntries();
extern void GetAllStrings(), AddString(), PopupCentered();
extern WNode * IDToNode();

static void _SetField(), CreateSetValuesPopup();

/*	Function Name: PopupSetValues
 *	Description: This function pops up the setvalues dialog
 *	Arguments: parent - the parent of the setvalues popup.
 *                 event - the event that caused this popup, or NULL.
 *	Returns: none
 */

/* ARGSUSED */
void
PopupSetValues(parent, event)
Widget parent;
XEvent * event;
{
    Arg args[1];

    if (global_tree_info == NULL) {
	SetMessage(global_screen_data.info_label,
		   "No widget Tree is avaliable.");
	return;
    }

/* 
 * Check and possibly create the popup.
 */

    if (global_screen_data.set_values_popup == NULL)
	CreateSetValuesPopup(parent, &global_screen_data);

/*
 * Clear out the old strings, and set the active widget to the name widget.
 */

    XtSetArg(args[0], XtNstring, "");
    XtSetValues(global_screen_data.res_text, args, ONE);
    XtSetValues(global_screen_data.val_text, args, ONE);

    _SetField(global_screen_data.res_text, global_screen_data.val_text);

/*
 * Pop it up.
 */

    PopupCentered(event, global_screen_data.set_values_popup, XtGrabNone);
}

/*	Function Name: ModifySVEntry
 *	Description: Action routine that can be bound to the set values 
 *                   dialog box's Text Widget that will send input to the 
 *                   field specified.
 *	Arguments:   (Standard Action Routine args) 
 *	Returns:     none.
 */

/* ARGSUSED */
void 
ModifySVEntry(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal * num_params;
{
    Widget new, old;
    char msg[BUFSIZ];
    
    if (*num_params != 1) {
	strcpy(msg, 
	       "Error: SVActiveEntry Action must have exactly one argument.");
	SetMessage(global_screen_data.info_label, msg);
	return;
    }
    
    switch (params[0][0]) {
    case 'r':
    case 'R':
	new = global_screen_data.res_text;
	old = global_screen_data.val_text;
	break;
    case 'v':
    case 'V':
	new = global_screen_data.val_text;
	old = global_screen_data.res_text;
	break;
    default:
	sprintf(msg, "%s %s", "Error: SVActiveEntry Action's first Argument",
		"must be either 'Resource' or 'Value'.");
	SetMessage(global_screen_data.info_label, msg);
	return;
    }
    
    _SetField(new, old);
}

/************************************************************
 *
 * Private Functions
 *
 ************************************************************/

/*	Function Name: _SetField
 *	Description: Sets the current text entry field.
 *	Arguments: new, old - new and old text fields.
 *	Returns: none
 */

static void
_SetField(new, old)
Widget new, old;
{
    Arg args[2];
    Pixel new_border, old_border, old_bg;
    
    if (!XtIsSensitive(new)) {
	XBell(XtDisplay(old), 0); /* Don't set field to an inactive Widget. */
	return;
    }
    
    XtSetKeyboardFocus(XtParent(new), new); 
    
    XtSetArg(args[0], XtNborderColor, &old_border);
    XtSetArg(args[1], XtNbackground, &old_bg);
    XtGetValues(new, args, TWO);
    
    XtSetArg(args[0], XtNborderColor, &new_border);
    XtGetValues(old, args, ONE);
    
    if (old_border != old_bg)	/* Colors are already correct, return. */
	return;

    XtSetArg(args[0], XtNborderColor, old_border);
    XtSetValues(old, args, ONE);

    XtSetArg(args[0], XtNborderColor, new_border);
    XtSetValues(new, args, ONE);
}

/*	Function Name: CreateSetValuesPopup
 *	Description: Creates the setvalues popup.
 *	Arguments: parent - the parent of the popup.
 *                 scr_data - the data about this screen.
 *	Returns: the set values popup.
 */

static void
CreateSetValuesPopup(parent, scr_data)
Widget parent;
ScreenData * scr_data;
{
    static void DoSetValues(), CancelSetValues();
    Widget form, cancel, do_it, label;
    Widget res_label;
    Arg args[10];
    Cardinal num_args;
    
    scr_data->set_values_popup = XtCreatePopupShell("setValuesPopup", 
						    transientShellWidgetClass, 
						    parent, NULL, ZERO);

    form = XtCreateManagedWidget("form", formWidgetClass, 
				 scr_data->set_values_popup, NULL, ZERO);

    num_args = 0;
    label = XtCreateManagedWidget("label", labelWidgetClass,
				  form, args, num_args);


    num_args = 0;
    XtSetArg(args[num_args], XtNfromVert, label); num_args++;
    res_label = XtCreateManagedWidget("resourceLabel", labelWidgetClass,
				  form, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNfromVert, label); num_args++;
    XtSetArg(args[num_args], XtNfromHoriz, res_label); num_args++;
    scr_data->res_text = XtCreateManagedWidget("resourceText", 
						  asciiTextWidgetClass,
						  form, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNfromVert, scr_data->res_text); num_args++;
    (void)  XtCreateManagedWidget("valueLabel", labelWidgetClass,
				  form, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNfromHoriz, res_label); num_args++;
    XtSetArg(args[num_args], XtNfromVert, scr_data->res_text); num_args++;
    scr_data->val_text = XtCreateManagedWidget("valueText", 
						  asciiTextWidgetClass,
						  form, args, num_args);
  
    num_args = 0;
    XtSetArg(args[num_args], XtNfromVert, scr_data->val_text); num_args++;
    do_it = XtCreateManagedWidget("setValues", commandWidgetClass, 
					  form, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNfromVert, scr_data->val_text); num_args++;
    XtSetArg(args[num_args], XtNfromHoriz, do_it); num_args++;
    cancel = XtCreateManagedWidget("cancel", commandWidgetClass,
				   form, args, num_args);

    XtAddCallback(do_it, XtNcallback, DoSetValues, NULL);
    XtAddCallback(cancel, XtNcallback, CancelSetValues, NULL);

/*
 * Initialize the text entry fields.
 */

    {
	Pixel color;

	num_args = 0;
	XtSetArg(args[num_args], XtNbackground, &color); num_args++;
	XtGetValues(scr_data->val_text, args, num_args);

	num_args = 0;
	XtSetArg(args[num_args], XtNborderColor, color); num_args++;
	XtSetValues(scr_data->val_text, args, num_args);

	XtSetKeyboardFocus(form, scr_data->res_text);
    }
}

/*	Function Name: DoSetValues
 *	Description: Performs a SetValues.
 *	Arguments: w - the widget that called this.
 *                 junk, garbage - ** UNUSED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
DoSetValues(w, junk, garbage)
Widget w;
caddr_t junk, garbage;
{
    char ** entries, *res_name, *res_value, *command_value;
    Arg args[1];
    Cardinal num_entries, i, res_len, total_len, alloc_len;

    GetAllActiveTreeEntries(global_tree_info, &entries, &num_entries);

    if (num_entries == 0) {
	SetMessage(global_screen_data.info_label,
		   "There are no currently active widgets.");
	return;
    }
		
    XtSetArg(args[0], XtNstring, &res_name);
    XtGetValues(global_screen_data.res_text, args, ONE);

    XtSetArg(args[0], XtNstring, &res_value);
    XtGetValues(global_screen_data.val_text, args, ONE);
    
    res_len = strlen(res_name) + strlen(res_value); 

    command_value = NULL;
    total_len = 1;		/* space for '\0' */
    alloc_len = 0;
    
    for (i = 0; i < num_entries; i++) {
	Cardinal len;
	char * ptr;

	/*
	 * The '3' is for the three separators is the sprintf below.
	 */

	len = strlen(entries[i]) + res_len + 3;
			   
	if ((total_len + len) >= alloc_len) {
	    alloc_len += BUFSIZ;
	    command_value = XtRealloc(command_value, alloc_len);
	}

	ptr = command_value + total_len - 1;
	sprintf(ptr, "%s%c%s%c%s%c", entries[i], WID_RES_SEPARATOR, res_name,
		NAME_VAL_SEPARATOR, res_value, EOL_SEPARATOR);

	XtFree(entries[i]);
	total_len += len;
    }

    XtFree(entries);

    SetCommand(w, SetValues, command_value, NULL);
    XtFree(command_value);
}

/*	Function Name: CancelSetValues
 *	Description: Pops down the setvalues popup.
 *	Arguments: w - any grandchild of the popup.
 *                 junk, garbage - ** UNUSED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
CancelSetValues(w, junk, garbage)
Widget w;
caddr_t junk, garbage;
{
    XtPopdown(XtParent(XtParent(w))); 
}

/*	Function Name: GetFormattedSetValuesError
 *	Description: Returns a formatted string for a formatted 
 *                   setvalues error.
 *	Arguments: value - the formatted error string.
 *	Returns: printable error message.
 */

char * 
GetFormattedSetValuesError(value)
char * value;
{
    char buf[BUFSIZ], ** strings, *ret_val;
    int num_strings, i;
    WNode *node;

    GetAllStrings(value, EOL_SEPARATOR, &strings, &num_strings);

    ret_val = NULL;

    for (i = 0; i < num_strings; i++) {
	char *ptr;

	ptr = index(strings[i], NAME_VAL_SEPARATOR);
	if (ptr == NULL) {
	    sprintf(buf, "Incorrectly formatted entry, no `%c'\n",
		    NAME_VAL_SEPARATOR);
	    AddString(&ret_val, buf); 
	    continue;
	}

	if (ptr != strings[i]) {
	    *ptr++ = '\0';
	    node = IDToNode(global_tree_info->top_node, strings[i]);
	    
	    sprintf(buf, "%s(0x%lx) - %s\n", node->name, node->id, ptr);
	}
	else
	    strcpy(buf, (ptr + 1));

	AddString(&ret_val, buf);
    }
    
    XtFree(strings);
    return(ret_val);
}
