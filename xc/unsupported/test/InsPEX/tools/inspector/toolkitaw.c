
/* $XConsortium: toolkitaw.c,v 5.4 91/05/29 19:27:06 converse Exp $ */

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

static Widget toplevel;		/* Shell Widget returned by XtInitialize */

/* for keeping track of control widget layout.  We make an assumption
 * that widgets in a control window (form widget) are created in order,
 * and use the column argument to detect whether it goes on the
 * same row as the last one.  (col==1 means new row)
 */
static Widget lastwidget=NULL;  	/* last one created */
static Widget lastleftwidget=NULL; 	/* last one beginning a row */
static Widget widgetAbove = NULL;	/* the first one in the previous row */

/*
 * array of string pointers for list, first one null
 */
static char list_strings[1024] = {NULL};


/*
 * tk_init(argc,argv) - initialize the toolkit.  The toolkit may
 * look for toolkit-specific arguments in the command-line input.
 */
void tk_init(argc,argv)
int argc;
char *argv[];
{
    /* 
     * Easier to create the toplevel shell widget now,
     * and keep it around for the tk_create_main_window call.
     * result kept in the static variable toplevel.
     */
    toplevel = XtInitialize(NULL, "Inspector", NULL, 0, &argc, argv);
}


/*
 * TK_Main_Window
 * Create a main application window with given label, height, and width,
 * Returning the display in display_return and window as
 * the return value.
 */
TK_Main_Window tk_create_main_window(label, height, width, display_return)
    char *label; 
    int height,width;
    Display **display_return;
{
    Arg args[5];
    Cardinal i = 0;
    Widget form;

    /* toplevel already created by tk_init: create a form child */

    /* blow off label for now */
    if (height != -1) {
	XtSetArg(args[i], XtNheight, height); i++;
    }
    if (width != -1) {
	XtSetArg(args[i], XtNwidth, width); i++;
    }
    XtSetArg(args[i], XtNresizable, True);	i++;
    form = XtCreateManagedWidget("main", formWidgetClass, toplevel, args, i);
    *display_return = XtDisplay(toplevel);
    return form;
}


/*
 * TK_Control_Window
 * Create a subwindow suitable for buttons, etc. that is a child of
 * the given parent.  A -1 for any of x, y, height or width 
 * yields the default or toolkit-determined value.
 */
TK_Control_Window tk_create_control_window(parent, x, y, height, width)
    TK_Main_Window parent;
    int x, y, height, width;
{
    Arg args[15];
    Cardinal i = 0;
    Widget control_win;
    static int child = 0;	/* evil hack */

    if (height != -1) {
	XtSetArg(args[i], XtNheight, height); i++;
    }
    if (width != -1) {
	XtSetArg(args[i], XtNwidth, width); i++;
    }
    if (x != -1) {
	XtSetArg(args[i], XtNx, x); i++;
    }
    if (y != -1) {
	XtSetArg(args[i], XtNy, y); i++;
    }

    XtSetArg(args[i], XtNfromHoriz, NULL);		i++;
    XtSetArg(args[i], XtNhorizDistance, x);		i++;
    XtSetArg(args[i], XtNfromVert, NULL);		i++;
    XtSetArg(args[i], XtNvertDistance, y);		i++;
    XtSetArg(args[i], XtNresizable, True);		i++;
    if (child == 0) {
	XtSetArg(args[i], XtNbottom, XawChainBottom);	i++;
	XtSetArg(args[i], XtNleft, XawChainLeft);	i++;
	XtSetArg(args[i], XtNright, XawChainRight);	i++;
    } else if (child == 1) {
	XtSetArg(args[i], XtNtop, XawChainTop);		i++;
	XtSetArg(args[i], XtNright, XawChainRight);	i++;
	XtSetArg(args[i], XtNbottom, XawChainBottom);	i++;
    }
    child++;

    control_win = XtCreateManagedWidget("control", formWidgetClass, parent,
					args, i);
    return (control_win);
}

#if 0 /* never called */
/*
 * Create an X window as a subwindow of the supplied parent window,
 * returning its id.  *display_return is set to the current default display. 
 */
Drawable tk_create_X_drawable_window(parent,height,width,
				     repaint_proc,display_return)
    TK_Main_Window parent;
    int height,width,(*repaint_proc)();
    Display **display_return;
{
    /* create a Core widget and return its Xid */
    Arg args[4];
    Cardinal i = 0;
    Widget core_widget;
    Drawable drawable;

    if (height != -1) {
	XtSetArg(args[i], XtNheight, height); i++;
    }
    if (width != -1) {
	XtSetArg(args[i], XtNwidth, width); i++;
    }
    core_widget = XtCreateManagedWidget("canvas", coreWidgetClass, parent,
					args, i);
    *display_return = XtDisplay(parent);
    XtRealizeWidget(core_widget);
    drawable = XtWindow(core_widget);
    return(drawable);
    
}
#endif

XID tk_get_xid(window)
    TK_Main_Window window;
{
    XtRealizeWidget(toplevel);
    return (XID) XtWindow(window);
}


/*
 * Create a button in the given control window, at designated row & col.
 * proc identifies a procedure to be called when the button is pressed.
 */
TK_Button tk_create_button(control_win, row, col, label, proc, active_status)
    TK_Control_Window control_win;
    int row, col;
    char *label;
    void (*proc)();
    int active_status;
{
    Cardinal i=0;
    Widget button;
    Arg args[5];

    /*
     * Set up geometry constraints for Form widget parent
     */
    if (row == 1 && col == 1) {
	/* the first widget in the control window */
	XtSetArg(args[i], XtNleft, XtChainLeft); 	i++;
	XtSetArg(args[i], XtNtop, XtChainTop);		i++;
    } else if (col == 1) {
	/* widget is in new row */
	XtSetArg(args[i], XtNfromVert, lastleftwidget);	i++;
	XtSetArg(args[i], XtNleft, XtChainLeft);	i++;
    } else {
	/* widget is in same row as last widget */
	XtSetArg(args[i], XtNfromHoriz, lastwidget);	i++;
	XtSetArg(args[i], XtNfromVert, widgetAbove);	i++;
    }
    XtSetArg(args[i], XtNlabel, label);			i++;
    if (active_status != TK_BUTTON_ACTIVE) {
	XtSetArg(args[i], XtNsensitive, False);		i++;
    }
    button = XtCreateManagedWidget("button", commandWidgetClass,
				   control_win, args, i);
    XtAddCallback(button, XtNcallback, proc, (XtPointer) NULL);

    lastwidget = button;
    if (col == 1) {
	widgetAbove = lastleftwidget;
	lastleftwidget = button;
    }
    return (button);
}


/*
 * tk_activate_button(button) - sets the active status of the button
 * to TK_BUTTON_ACTIVE
 */
void tk_activate_button(button)
    TK_Button button;
{
    XtSetSensitive(button, TRUE);
}


/*
 * tk_deactivate_button(button) - sets the active status of the button
 * to TK_BUTTON_INACTIVE
 */
void tk_deactivate_button(button)
    TK_Button button;
{
    XtSetSensitive(button, FALSE);
}


/*
 * tk_set_button_label(button,label) - sets the button label to
 * the label given.
 */
void tk_set_button_label(button,label)
    TK_Button button;
    char *label;
{
    Arg arg[1];
    XtSetArg(arg[0], XtNlabel, label);
    XtSetValues(button, arg, (Cardinal) 1);
}


/*
 * TK_Message_Item
 * create a text output item  in the given control window, 
 * at designated row & col, with the initial contents init_string.
 */
TK_Message_Item tk_create_message_item(control_win, row, col, label)
    TK_Control_Window control_win;
    int row, col;
    char *label;
{
    Arg args[5];
    Cardinal i=0;
    Widget message;

    /*
     * Set up geometry constraints for Form widget parent
     */
    if (row == 1 && col == 1) {
	/* the first widget in the control window */
	XtSetArg(args[i], XtNleft, XawChainLeft);	i++;
	XtSetArg(args[i], XtNtop, XawChainTop);		i++;
    } else if (col == 1) {
	/* widget is in new row */
	XtSetArg(args[i], XtNfromVert, lastleftwidget);	i++;
	XtSetArg(args[i], XtNleft, XawChainLeft);	i++;
    } else if (row != -1) {
	/* widget is in same row as last widget */
	XtSetArg(args[i], XtNfromHoriz, lastwidget);	i++;
	XtSetArg(args[i], XtNfromVert, lastleftwidget);	i++;
    } else {
	/* widget is not in the control panel */ 
	XtSetArg(args[i], XtNfromVert, NULL);		i++;
	if (col == 60) {
	    XtSetArg(args[i], XtNwidth, col);		i++;
	    XtSetArg(args[i], XtNfromHoriz,lastwidget);	i++;
	}
    }

    /* set up other button attributes */
    XtSetArg(args[i], XtNlabel, label);			i++;
    message = XtCreateManagedWidget("label", labelWidgetClass,
				    control_win, args, i);

    lastwidget = message;
    if (col == 1) {
	widgetAbove = lastleftwidget;
	lastleftwidget = message;
    }
    return (message);
}


/*
 * tk_set_message(msg_item, text) - sets the text tring displayed
 * in msg_item to "text".
 */
void tk_set_message(msg_item, text)
    TK_Message_Item msg_item;
    char *text;
{
    XtVaSetValues(msg_item,
		  XtNlabel, text,
		  NULL);
}


/* 
 * TK_List
 * tk_create_list(control_win, row, col, notify_proc)
 */
TK_List tk_create_list(control_win,row,col,notify_proc)
    TK_Control_Window control_win;
    int row,col, (*notify_proc)();
{
    Arg args[4];
    Cardinal i=0;
    Widget viewport, list;

    /*
     * Set up geometry constraints for Form widget parent of viewport
     */
    if (row == 1 && col == 1) {
	/* the first widget in the control window */
	XtSetArg(args[i], XtNleft,  XtChainLeft); i++;
	XtSetArg(args[i], XtNtop,  XtChainTop); i++;
    } else if (col == 1) {
	/* widget is in new row */
	XtSetArg(args[i], XtNfromVert,  lastleftwidget); i++;
	XtSetArg(args[i], XtNleft,  XtChainLeft); i++;
    } else {
	/* widget is in same row as last widget */
	XtSetArg(args[i], XtNfromHoriz,  lastwidget); i++;
    }

    /* create the viewport */
    viewport = XtCreateManagedWidget("viewport", viewportWidgetClass,
				     control_win, args, i);

    /* set up attributes of list widget */
    i=0;
    XtSetArg(args[i], XtNverticalList,  TRUE); i++;
    XtSetArg(args[i], XtNheight,  100); i++;

    list = XtCreateManagedWidget("list", listWidgetClass, viewport, args, i);
    XtAddCallback(list, XtNcallback, notify_proc, (XtPointer) NULL);
    return (list);
}


/*
 * Return a pointer to the
 * string of the currently selected list item, and set index_return
 * to that element's index.  If no list item is selected, return NULL.
 */
char * tk_get_selected_list_item(list, index_return)
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
char * tk_get_list_item(list,index)
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
int tk_get_list_length(list)
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
void tk_set_list_item(list,index,string)
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

void tk_insert_list_item(list,index,string)
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
void tk_delete_list_item(list,index)
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
void tk_main_loop(main_win)
    TK_Main_Window main_win;
{
    XtMainLoop();
}


static fatal(s)	/* this function is not used */
    char *s;
{
    fprintf(stderr, "%s\n",s);
    exit(1);
}
