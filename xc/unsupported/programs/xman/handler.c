/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: handler.c,v 1.10 89/05/09 16:35:00 kit Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   October 29, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: handler.c,v 4.6 89/01/06 12:17:27 kit Exp $";
#endif

#include "globals.h"

static void PutUpManpage();

/*	Function Name: OptionCallback
 *	Description: This is the callback function for the callback menu.
 *	Arguments: w - the widget we are calling back from. 
 *                 globals_pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */
void
OptionCallback(w,pointer,junk)
Widget w;
caddr_t pointer,junk;
{
  static void ToggleBothShownState();
  ManpageGlobals * man_globals; 
  MenuStruct * menu_struct;
  register XrmQuark quark;
  Cardinal num_params = 1;
  String params;

  static XrmQuark search_quark, directory_quark, manpage_quark, help_quark;
  static XrmQuark both_screens_quark, remove_quark, open_quark, quit_quark;
  static XrmQuark version_quark;
  static Boolean quark_initialized = FALSE;

  if (!quark_initialized) {
    both_screens_quark = XrmStringToQuark(BOTH_SCREENS);
    directory_quark    = XrmStringToQuark(DIRECTORY);
    help_quark         = XrmStringToQuark(HELP);
    manpage_quark      = XrmStringToQuark(MANPAGE);
    open_quark         = XrmStringToQuark(OPEN_MANPAGE);
    quit_quark         = XrmStringToQuark(QUIT);
    remove_quark       = XrmStringToQuark(REMOVE_MANPAGE);
    search_quark       = XrmStringToQuark(SEARCH);
    version_quark      = XrmStringToQuark(SHOW_VERSION);
    quark_initialized  = TRUE;
  }
  
  menu_struct = (MenuStruct *) pointer;
  man_globals = (ManpageGlobals *) menu_struct->data;
  quark = menu_struct->quark;

  if (quark == search_quark )
    PopupSearch(w, NULL, NULL, NULL);
  else if (quark == directory_quark) {         /* Put Up Directory */
    params = "Directory";
    GotoPage(w, NULL, &params, &num_params);
  }
  else if (quark == manpage_quark) {           /* Put Up Man Page */
    params = "ManualPage";
    GotoPage(w, NULL, &params, &num_params);
  }
  else if (quark == help_quark)               /* Help */
    PopupHelp(w, NULL, NULL, NULL);
  else if (quark == both_screens_quark)        /* Toggle Both_Shown State. */
    ToggleBothShownState(man_globals);
  else if (quark == remove_quark)             /* Kill the manpage */
    RemoveThisManpage(w, NULL, NULL, NULL);
  else if (quark == open_quark)                /* Open new manpage */
    CreateNewManpage(w, NULL, NULL, NULL);
  else if (quark == version_quark)             /* Get version */
    ShowVersion(w, NULL, NULL, NULL);
  else if (quark == quit_quark)                /* Quit. */
    Quit(w, NULL, NULL, NULL);
}

/*      Function Name: ToggleBothShownState;
 *      Description: toggles the state of the both shown feature.
 *      Arguments: man_globals - the man globals structure.
 *      Returns: none.
 */

/*
 * I did not have a two state widget, which is the way this
 * should really be done.  1/22/88 - CDP.
 */

static void
ToggleBothShownState(man_globals)
ManpageGlobals * man_globals; 
{
  char * label_str;
  Arg arglist[1];

  if (man_globals->both_shown == TRUE) {
    label_str = SHOW_BOTH;
    if (man_globals->dir_shown)
      XtUnmanageChild(man_globals->manpagewidgets.manpage);
    else
      XtUnmanageChild(man_globals->manpagewidgets.directory);
  }
  else {
    Widget manpage = man_globals->manpagewidgets.manpage;
    Widget dir = man_globals->manpagewidgets.directory;
    
    label_str = SHOW_ONE;

    XtSetArg(arglist[0], XtNpreferredPaneSize, resources.directory_height);
    XtSetValues(dir, arglist, (Cardinal) 1);

    if (!man_globals->dir_shown) {
      XtUnmanageChild(manpage);
      XtManageChild(dir);
    }
    XtManageChild(manpage);
  }
  man_globals->both_shown = !man_globals->both_shown;
  
  if (man_globals->dir_shown)
    ChangeLabel(man_globals->label,
		man_globals->section_name[man_globals->current_directory]);
  else
    ChangeLabel(man_globals->label, man_globals->manpage_title);
  
  XtSetArg(arglist[0], XtNlabel, label_str);
  XawSimpleMenuSetEntryValues(man_globals->option_menu, BOTH_SCREENS,
			   arglist, (Cardinal) 1);
  
  /* if both are shown there is no need to switch between the two. */

  XtSetArg(arglist[0], XtNsensitive, !man_globals->both_shown);
  XawSimpleMenuSetEntryValues(man_globals->option_menu, MANPAGE,
			   arglist, (Cardinal) 1);
  XawSimpleMenuSetEntryValues(man_globals->option_menu, DIRECTORY,
			   arglist, (Cardinal) 1);
}

/*	Function Name: Popup
 *	Description: This function pops up the given widget under the cursor.
 *	Arguments: w - the widget to popup.
 *                 grab_kind - the kind of grab to register.
 *	Returns: none
 */

/* How far off the top of the widget to have the initial cursor postion. */

#define OFF_OF_TOP 25

void
Popup(w, grab_kind)
Widget w;
XtGrabKind grab_kind;
{
  int x_root,y_root,y_pos,garbage;
  unsigned int mask;
  Window junk_window;

  XQueryPointer(XtDisplay(w), XtWindow(w), &junk_window, &junk_window,
		&x_root, &y_root, &garbage, &garbage, &mask);

  y_pos = OFF_OF_TOP - Height(w)/2 - BorderWidth(w);
  PositionCenter(w, x_root, y_root, y_pos, 0, 2, 2);
  XtPopup(w, grab_kind);
}

/*	Function Name: PutUpManpage
 *	Description: Puts the manpage on the display.
 *	Arguments: man_globals - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 file - the file to display.
 *	Returns: none.
 */

static void
PutUpManpage(man_globals, file)
ManpageGlobals * man_globals;
FILE * file;
{
  String params = "ManualPage";
  Cardinal num_params = 1;
  
  if (file == NULL)
    return;

  OpenFile(man_globals, file);
  fclose(file);

  if (!man_globals->both_shown) {
    Arg arglist[1];
    XtSetArg(arglist[0], XtNsensitive, TRUE);
    XawSimpleMenuSetEntryValues(man_globals->option_menu, MANPAGE,
			     arglist, (Cardinal) 1);
    XawSimpleMenuSetEntryValues(man_globals->option_menu, BOTH_SCREENS,
			     arglist, (Cardinal) 1);
  }
  GotoPage(man_globals->manpagewidgets.manpage, NULL, &params, &num_params);
}

/*	Function Name: DirectoryHandler
 *	Description: This is the callback function for the directory listings.
 *	Arguments: w - the widget we are calling back from. 
 *                 global_pointer - the pointer to the psuedo global structure
 *                                  associated with this manpage.
 *                 ret_val - return value from the list widget.
 *	Returns: none.
 */

/* ARGSUSED */
void
DirectoryHandler(w, global_pointer, ret_val)
Widget w;
caddr_t global_pointer, ret_val;
{
  FILE * file;			/* The manpage file. */
  ManpageGlobals * man_globals = (ManpageGlobals *) global_pointer;
  XawListReturnStruct * ret_struct = (XawListReturnStruct *) ret_val;

  file = FindFilename(man_globals,
       manual[man_globals->current_directory].entries[ret_struct->list_index]);
  PutUpManpage(man_globals, file);
}

/*	Function Name: DirPopupCallback
 *	Description: This is the callback function for the callback menu.
 *	Arguments: w - the widget we are calling back from. 
 *                 pointer - a pointer to the psuedo globals structure
 *                                  for this manpage.
 *                 junk - (call data) not used.
 *	Returns: none.
 */

/* ARGSUSED */
void
DirPopupCallback(w,pointer,junk)
Widget w;
caddr_t pointer,junk;
{
  ManpageGlobals * man_globals; 
  MenuStruct * menu_struct;
  Widget parent;
  int number;
  int current_box;

  menu_struct = (MenuStruct *) pointer;
  man_globals = (ManpageGlobals *) menu_struct->data;

  number = menu_struct->number;
  current_box = man_globals->current_directory;

  /* We have used this guy, pop down the menu. */
  
  if (number != current_box) {
    /* This is the only one that we know has a parent. */
    parent = XtParent(man_globals->manpagewidgets.box[INITIAL_DIR]);

    MakeDirectoryBox(man_globals, parent,
		     man_globals->manpagewidgets.box + number, number);
    XtUnmanageChild(man_globals->manpagewidgets.box[current_box]);
    XtManageChild(man_globals->manpagewidgets.box[number]);

    XawListUnhighlight(man_globals->manpagewidgets.box[current_box]);
    ChangeLabel(man_globals->label, man_globals->section_name[number]);
    man_globals->current_directory = number;
  }

  /* put up directory. */
  if (!man_globals->both_shown) {
    XtUnmanageChild(man_globals->manpagewidgets.manpage);
    XtManageChild(man_globals->manpagewidgets.directory);
  }
}
    
/************************************************************
 *
 * Action Routines.
 *
 ************************************************************/

/*	Function Name: SaveFormattedPage
 *	Description: This is the action routine may save the manpage.
 *      Arguments: w - any widget in the widget tree.
 *                 event - NOT USED.
 *                 params, num_params - the parameters paseed to the action
 *                                      routine, can be either Manpage or
 *                                      Directory.
 *      Returns: none.
 */

/* ARGSUSED */
void
SaveFormattedPage(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ManpageGlobals * man_globals;
  char str[BUFSIZ], error_buf[BUFSIZ];

  if (*num_params != 1) {
    XtAppWarning(XtWidgetToApplicationContext(w), 
       "Xman - SaveFormattedPage: This action routine requires one argument.");
    return;
  }

  man_globals = GetGlobals(w);

/*
 * If we are not active then take no action.
 */

  if (man_globals->tmpfile == '\0') return;

  switch (params[0][0]) {
  case 'S':
  case 's':
    sprintf(str,"%s %s %s", COPY, man_globals->tmpfile, man_globals->filename);
    if(system(str) != 0)		/* execute copy. */
      PrintError("Something went wrong trying to copy temp file to cat dir.");
    break;
  case 'C':
  case 'c':
    break;
  default:
    sprintf(error_buf,"%s %s", "Xman - SaveFormattedPagee:",
	    "Unknown argument must be either 'Save' or 'Cancel'.");
    XtAppWarning(XtWidgetToApplicationContext(w), error_buf);
    return;
  }
    
/*
 * We do not need the filename anymore, and have the fd open.
 * We will unlink it.     
 */

  unlink(man_globals->tmpfile);
  man_globals->tmpfile[0] = '\0'; /* remove name of tmpfile. */

  XtPopdown( XtParent(XtParent(w)) );
}

/*      Function Name: GotoPage
 *      Description: The Action routine that switches over to the manpage
 *                   or directory.
 *      Arguments: w - any widget in the widget tree.
 *                 event - NOT USED.
 *                 params, num_params - the parameters paseed to the action
 *                                      routine, can be either Manpage or
 *                                      Directory.
 *      Returns: none.
 */

/* ARGSUSED */
void
GotoPage(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ManpageGlobals * man_globals;
  char error_buf[BUFSIZ];
  Arg arglist[1];
  Boolean sensitive;

  if (*num_params != 1) {
    XtAppWarning(XtWidgetToApplicationContext(w), 
		"Xman - GotoPage: This action routine requires one argument.");
    return;
  }

  man_globals = GetGlobals(w);

  if (man_globals->both_shown) {
    ChangeLabel(man_globals->label, 
		man_globals->section_name[man_globals->current_directory]);
    return;
  }

  switch (params[0][0]) {
  case 'M':
  case 'm':
    XtSetArg(arglist[0], XtNsensitive, &sensitive);
    XawSimpleMenuGetEntryValues(man_globals->option_menu, MANPAGE,
			   arglist, (Cardinal) 1);
    if (sensitive) {
      ChangeLabel(man_globals->label,man_globals->manpage_title);
      XtUnmanageChild(man_globals->manpagewidgets.directory);
      XtManageChild(man_globals->manpagewidgets.manpage);
      man_globals->dir_shown = FALSE;
    }
    break;
  case 'D':
  case 'd':
    ChangeLabel(man_globals->label,
		man_globals->section_name[man_globals->current_directory]);
    XtUnmanageChild(man_globals->manpagewidgets.manpage);
    XtManageChild(man_globals->manpagewidgets.directory);  
    man_globals->dir_shown = TRUE;
    break;
  default:
    sprintf(error_buf,"%s %s", "Xman - GotoPage: Unknown argument must be",
	    "either Manpage or Directory.");
    XtAppWarning(XtWidgetToApplicationContext(w), error_buf);
    return;
  }
}

/*      Function Name: Quit.
 *      Description: Quits Xman.
 *      Arguments: w - any widget.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
Quit(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  XCloseDisplay(XtDisplay(w));
  exit(0);
}

/*      Function Name: PopupHelp
 *      Description: Pops up xman's help.
 *      Arguments: w - NOT USED.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
PopupHelp(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  if (MakeHelpWidget())
    XtPopup(help_widget,XtGrabNone);
}

/*      Function Name: PopupSearch
 *      Description: Pops up this manual pages search widget.
 *      Arguments: w - any widget in this manpage.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
PopupSearch(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ManpageGlobals * man_globals = GetGlobals(w);

  Popup(man_globals->search_widget, XtGrabNone);
}

/*      Function Name: CreateNewManpage
 *      Description: Creates A New Manual Page.
 *      Arguments: w - NOT USED.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
CreateNewManpage(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  (void) CreateManpage(NULL);
  man_pages_shown++;
}

/*      Function Name: RemoveThisManpage
 *      Description: Removes a manual page.
 *      Arguments: w - any widget in the manpage.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
RemoveThisManpage(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ManpageGlobals * man_globals = GetGlobals(w);
  
  if (man_globals->This_Manpage != help_widget) {
    RemoveGlobals(man_globals->This_Manpage);
    XtDestroyWidget(man_globals->This_Manpage);

    XtFree( (char *) man_globals->section_name);
    XtFree( (char *) man_globals->manpagewidgets.box);
    XtFree( (char *) man_globals);

    if ( (--man_pages_shown) == 0)
      Quit(w, NULL, NULL, NULL);  
  }
  else
    XtPopdown(help_widget);
}

/*      Function Name: Search
 *      Description: Actually performs a search.
 *      Arguments: w - any widget in the manpage.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
Search(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ManpageGlobals * man_globals = GetGlobals(w);
  FILE * file;

  XtPopdown(  XtParent(XtParent(w)) );       /* popdown the search widget */

  if ( (*num_params < 1) || (*num_params > 2) ) {
    XtAppWarning(XtWidgetToApplicationContext(w), 
      "Xman - Search: This action routine requires one or two arguments.");
    return;
  }

  switch(params[0][0]) {
  case 'a':
  case 'A':
    file = DoSearch(man_globals,APROPOS);
    break;
  case 'm':
  case 'M':
    file = DoSearch(man_globals,MANUAL);
    break;
  case 'c':
  case 'C':
    file = NULL;
    break;
  default:
    XtAppWarning(XtWidgetToApplicationContext(w), 
		 "Xman - Search: First parameter unknown.");
    file = NULL;
    break;
  }

  if ( *num_params == 2 ) 
    switch (params[1][0]) {
    case 'O':
    case 'o':
      if (file != NULL) {
	Widget w;
	char * label;

	w = CreateManpage(file);
	man_pages_shown++;

	/* Put title into new manual page. */

	label = man_globals->manpage_title;
	man_globals = GetGlobals(w);
	strcpy(man_globals->manpage_title, label);
	ChangeLabel(man_globals->label, label);
      }
      break;
    default:
      XtAppWarning(XtWidgetToApplicationContext(w), 
		   "Xman - Search: Second parameter unknown.");
      break;
    }
  else {
    PutUpManpage(man_globals, file);
  }
}

/*      Function Name: ShowVersion
 *      Description: Show current version.
 *      Arguments: w - any widget in the manpage.
 *                 event - NOT USED.
 *                 params, num_params - NOT USED.
 *      Returns: none.
 */

/* ARGSUSED */
void 
ShowVersion(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ManpageGlobals * man_globals = GetGlobals(w);

  ChangeLabel(man_globals->label, XMAN_VERSION);
}
