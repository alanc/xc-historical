
/* $XConsortium: toolkitaw.c,v 5.1 91/02/16 09:59:44 rws Exp $ */

/*****************************************************************
Copyright (c) 1989,1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


/*
 * toolkitaw.c  -- implements the toolkit-independent interface used
 * by the comparison tool.  This implementation uses Athena Widgets
 *
 * See top comment of toolkit.c for comments on the design
 * of this interface.
 * For all height, width, row, or column arguments, supplying a value
 * of -1 directs the function to use default or toolkit-determined
 * values.
 *
 * The application defines notification procedures as having no
 * arguments since we can't be sure all toolkits will call them
 * with the same arguments. Unfortunately, this forces greater use
 * of globals...
 *
 */

#include <stdio.h>
#include "toolkitaw.h"

/* 
 * Static variables for Athena implementation
 */

static Widget topLevel;		/* Shell Widget returned by XtInitialize */

/* for keeping track of control widget layout.  We make an assumption
 * that widgets in a control window (form widget) are created in order,
 * and use the column argument to detect whether it goes on the
 * same row as the last one.  (col==1 means new row)
 */
static Widget lastwidget=NULL;  	/* last one created */
static Widget lastleftwidget=NULL; 	/* last one beginning a row */

/*
 * array of string pointers for list, first one null
 */
static char list_strings[1024] = {NULL};

/*
 * tk_init(argc,argv) - initialize the toolkit.  The toolkit may
 * look for toolkit-specific arguments in the command-line input.
 */
void
tk_init(argc,argv)
int argc;
char *argv[];
{
    /* 
     * Easier to create the topLevel shell widget now,
     * and keep it around for the tk_create_main_window call.
     * result kept in the static variable topLevel.
     */
    topLevel = XtInitialize(
			    argv[0],
			    "Inspector",
			    NULL,
			    0,
			    &argc,
			    argv);
}
/*
 * TK_Main_Window
 * tk_create_main_window(label,height,width, display_return)
 *  Create a main application window with given label, height, and width,
 * Returning the display in display_return and window as
 * the return value.
 */
TK_Main_Window 
tk_create_main_window(label,height,width, display_return)
    char *label; 
    int height,width;
    Display **display_return;
{
    Arg args[2];
    int i = 0;
    Widget box;

    /* topLevel already created by tk_init: create a "box" child */

    /* blow off label for now */
    if (height != -1) {
	XtSetArg(args[i], XtNheight, (XtArgVal)height); i++;
    }
    if (width != -1) {
	XtSetArg(args[i], XtNwidth, (XtArgVal)width); i++;
    }
    box = XtCreateManagedWidget("main",
				boxWidgetClass,
				topLevel,
				args,
				i);
    *display_return = XtDisplay(topLevel);
    return (box);
}


/*
 * TK_Control_Window
 * tk_create_control_window(parent,x, y, height,width)
 * Create a subwindow suitable for buttons, etc. that is a child of
 * the given parent.  A -1 for any of x, y, height or width 
 * yields the default or toolkit-determined value.
 */
TK_Control_Window
tk_create_control_window(parent,x,y,height,width)
    TK_Main_Window parent;
    int x,y,height,width;
{
    Arg args[4];
    int i = 0;
    Widget control_win;

    if (height != -1) {
	XtSetArg(args[i], XtNheight, (XtArgVal)height); i++;
    }
    if (width != -1) {
	XtSetArg(args[i], XtNwidth, (XtArgVal)width); i++;
    }
    if (x != -1) {
	XtSetArg(args[i], XtNx, (XtArgVal)x); i++;
    }
    if (y != -1) {
	XtSetArg(args[i], XtNy, (XtArgVal)y); i++;
    }
    control_win = XtCreateManagedWidget("control",
				formWidgetClass,
				parent,
				args,
				i);
    return (control_win);
}


/*
 * Drawable
 * tk_create_X_drawable_window(parent,height,width,
 *			repaint_proc,display_return);
 * Create an X window as a subwindow of the supplied parent window,
 * returning its id.  *display_return is set to the current default display. 
 */
Drawable
tk_create_X_drawable_window(parent,height,width,
			repaint_proc,display_return)
    TK_Main_Window parent;
    int height,width,(*repaint_proc)();
    Display **display_return;
{
    /* create a Core widget and return its Xid */
    Arg args[4];
    int i = 0;
    Widget core_widget;
    Drawable drawable;

    if (height != -1) {
	XtSetArg(args[i], XtNheight, (XtArgVal)height); i++;
    }
    if (width != -1) {
	XtSetArg(args[i], XtNwidth, (XtArgVal)width); i++;
    }
/*
    if (x != -1) {
	XtSetArg(args[i], XtNx, (XtArgVal)x); i++;
    }
    if (y != -1) {
	XtSetArg(args[i], XtNy, (XtArgVal)y); i++;
    }
*/
    core_widget = XtCreateManagedWidget("control",
				boxWidgetClass,
				parent,
				args,
				i);
    *display_return = XtDisplay(parent);
    drawable = XtWindow(core_widget);
    return(drawable);
    
}

XID
tk_get_xid(window)
    TK_Main_Window window;
{
    XtRealizeWidget(topLevel);
    return (XID) XtWindow(window);
}

/*
 * tk_create_button(control_win,row, col,label,proc,active_status)
 * create a button in the given control window, at designated row & col.
 * proc identifies a procedure to be called when the button is pressed.
 */
TK_Button
tk_create_button(control_win,row,col,label,proc,active_status)
    TK_Control_Window control_win;
    int row, col;
    char *label;
    int (*proc)();
    int active_status;
{
    Arg args[5];
    int i=0;
    Widget buttonwidget;

    /*
     * Set up geometry constraints for Form widget parent
     */
    if (row == 1 && col == 1) {
	/* the first widget in the control window */
	XtSetArg(args[i], XtNleft, (XtArgVal) XtChainLeft); i++;
	XtSetArg(args[i], XtNtop, (XtArgVal) XtChainTop); i++;
    } else if (col == 1) {
	/* widget is in new row */
	XtSetArg(args[i], XtNfromVert, (XtArgVal) lastleftwidget); i++;
	XtSetArg(args[i], XtNleft, (XtArgVal) XtChainLeft); i++;
    } else {
	/* widget is in same row as last widget */
	XtSetArg(args[i], XtNfromHoriz, (XtArgVal) lastwidget); i++;
    }

    /* set up other button attributes */
    XtSetArg(args[i], XtNlabel, (XtArgVal) label); i++;
    XtSetArg(args[i], XtNcallback, (XtArgVal) proc); i++;
    if (active_status == TK_BUTTON_ACTIVE) {
	XtSetArg(args[i], XtNsensitive, (XtArgVal) TRUE); i++;
    } else {
	XtSetArg(args[i], XtNsensitive, (XtArgVal) FALSE); i++;
    }
    buttonwidget = XtCreateManagedWidget(label,
					 commandWidgetClass,
					 control_win,
					 args,
					 i  );
    lastwidget = buttonwidget;
    if (col == 1) lastleftwidget = buttonwidget;
    return (buttonwidget);
}

/*
 * tk_activate_button(button) - sets the active status of the button
 * to TK_BUTTON_ACTIVE
 */
void
tk_activate_button(button)
    TK_Button button;
{
    XtSetSensitive(button, TRUE);
}
/*
 * tk_deactivate_button(button) - sets the active status of the button
 * to TK_BUTTON_INACTIVE
 */
void
tk_deactivate_button(button)
    TK_Button button;
{
    XtSetSensitive(button, FALSE);
}

/*
 * tk_set_button_label(button,label) - sets the button label to
 * the label given.
 */
void
tk_set_button_label(button,label)
    TK_Button button;
    char *label;
{
    Arg arg;

    XtSetArg(arg, XtNlabel, (XtArgVal) label);
    XtSetValues(button, &arg, 1);
}

/*
 * TK_Message_Item
 * tk_create_message_item(control_win,row, col,init_string)
 * create a text output item  in the given control window, 
 * at designated row & col, with the initial contents init_string.
 */
TK_Message_Item
tk_create_message_item(control_win,row,col,init_string)
    TK_Control_Window control_win;
    int row, col;
    char *init_string;
{
    Arg args[5];
    int i=0;
    Widget messagewidget;

    /*
     * Set up geometry constraints for Form widget parent
     */
    if (row == 1 && col == 1) {
	/* the first widget in the control window */
	XtSetArg(args[i], XtNleft, (XtArgVal) XtChainLeft); i++;
	XtSetArg(args[i], XtNtop, (XtArgVal) XtChainTop); i++;
    } else if (col == 1) {
	/* widget is in new row */
	XtSetArg(args[i], XtNfromVert, (XtArgVal) lastleftwidget); i++;
	XtSetArg(args[i], XtNleft, (XtArgVal) XtChainLeft); i++;
    } else {
	/* widget is in same row as last widget */
	XtSetArg(args[i], XtNfromHoriz, (XtArgVal) lastwidget); i++;
    }

    /* set up other button attributes */
    XtSetArg(args[i], XtNlabel, (XtArgVal) init_string); i++;
    messagewidget = XtCreateManagedWidget(init_string,
					 labelWidgetClass,
					 control_win,
					 args,
					 i  );
    lastwidget = messagewidget;
    if (col == 1) lastleftwidget = messagewidget;
    return (messagewidget);
}

/*
 * tk_set_message(msg_item, text) - sets the text tring displayed
 * in msg_item to "text".
 */
void
tk_set_message(msg_item, text)
    TK_Message_Item msg_item;
    char *text;
{
    XtVaSetValues(msg_item,
		  XtNlabel, text,
		  NULL);
}


/* 
 * TK_List
 * tk_create_list(control_win,row,col,notify_proc)
 */
TK_List
tk_create_list(control_win,row,col,notify_proc)
    TK_Control_Window control_win;
    int row,col, (*notify_proc)();
{
    Arg args[4];
    int i=0;
    Widget viewport, list;

    /*
     * Set up geometry constraints for Form widget parent of viewport
     */
    if (row == 1 && col == 1) {
	/* the first widget in the control window */
	XtSetArg(args[i], XtNleft, (XtArgVal) XtChainLeft); i++;
	XtSetArg(args[i], XtNtop, (XtArgVal) XtChainTop); i++;
    } else if (col == 1) {
	/* widget is in new row */
	XtSetArg(args[i], XtNfromVert, (XtArgVal) lastleftwidget); i++;
	XtSetArg(args[i], XtNleft, (XtArgVal) XtChainLeft); i++;
    } else {
	/* widget is in same row as last widget */
	XtSetArg(args[i], XtNfromHoriz, (XtArgVal) lastwidget); i++;
    }

    /* create the viewport */
    viewport = XtCreateManagedWidget("viewport",
			      viewportWidgetClass,
			      control_win,
			      args,
			      i);

    /* set up attributes of list widget */
    i=0;
    XtSetArg(args[i], XtNverticalList, (XtArgVal) TRUE); i++;
    XtSetArg(args[i], XtNheight, (XtArgVal) 100); i++;
    XtSetArg(args[i], XtNwidth, 100); i++;
    XtSetArg(args[i], XtNcallback, (XtArgVal) notify_proc); i++;

    list = XtCreateManagedWidget("list",
			    listWidgetClass,
			    viewport,
			    args,
			    i);

    return (list);
}


/*
 * char *
 * tk_get_selected_item(list,index_return) - return a pointer to the
 * string of the currently selected list item, and set index_return
 * to that element's index.  If no list item is selected, return NULL.
 */
char *
tk_get_selected_list_item(list,index_return)
    TK_List list;
    int *index_return;
{
    char *string;
    int i, nrows;
    int sel=0;
#if 0
    nrows = (int)xv_get(list, PANEL_LIST_NROWS);
    for (i=0; 
	i<nrows,!(sel=(int)xv_get(list,PANEL_LIST_SELECTED,i));  i++);
    if (sel) {
	*index_return = i;
	return ((char *)xv_get(list,PANEL_LIST_STRING,i));
    } else {
	return (NULL);
    }
#endif
    printf("tk_get_selected_list_item(): returning selected\n");
    return ("selected");
}

/*
 * char *
 * tk_get_list_item(list,index) - return a pointer to the string of
 * of the list item specified by index.
 */
char *
tk_get_list_item(list,index)
    TK_List list;
    int index;
{
#if 0
    return ((char*)xv_get(list, PANEL_LIST_STRING, index));
#endif
    printf("tk_get_list_item(): returning hello\n");
    return("hello");
}

/*
 * int
 * tk_get_list_length(list) - return the number of rows in the list
 */
int
tk_get_list_length(list)
    TK_List list;
{
#if 0
    return ((int)xv_get(list, PANEL_LIST_NROWS));
#endif
    printf("tk_get_list_length() called\n");
    return (1);
}

/*
 * tk_set_list_item(list,index,string): set the indexed list item to
 * have value "string".  Note that this function is only for changing
 * the value; tk_insert_list_item should be used to create the item.
 */
void
tk_set_list_item(list,index,string)
    TK_List list;
    int index;
    char *string;
{
#if 0
    xv_set(list, PANEL_LIST_STRING,index,string,
	    0);
#endif
    printf("tk_set_list_item(list,%d,%s)\n", index, string);
}

void
tk_insert_list_item(list,index,string)
    TK_List list;
    int index;
    char *string;
{
#if 0
    xv_set(list, PANEL_LIST_INSERT,index,
	    0);
    xv_set(list, PANEL_LIST_STRING,index,string,
	    0);
#endif
    printf("tk_insert_list_item(list,%d,%s)\n", index, string);
}

/*
 * tk_delete_list_item: delete the list item indexed by index; if there
 * is an item after it, make that the selected item.
 */
void
tk_delete_list_item(list,index)
    TK_List list;
    int index;
{
#if 0
    xv_set(list, PANEL_LIST_DELETE,index,
	    0);
    if ((int)xv_get(list,PANEL_LIST_NROWS) > index) {
	xv_set(list, PANEL_LIST_SELECT, index, TRUE,
	    0);
    } 
#endif
    printf("tk_delete_list_item(list,%d)\n", index);
}

/*
 * tk_main_loop() - start the main event-handling loop
 */
void
tk_main_loop(main_win)
    TK_Main_Window main_win;
{
    XtMainLoop();
}

static
fatal(s)
    char *s;
{
    fprintf(stderr, "%s\n",s);
    exit(1);
}
