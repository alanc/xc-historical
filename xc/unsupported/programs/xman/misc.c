/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: misc.c,v 1.12 89/04/28 15:05:48 kit Exp $
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
 * Created:   October 27, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: misc.c,v 4.6 88/12/19 13:48:01 kit Exp $";
#endif

#include "globals.h"

/*
 * It would be very nice if these would pop up their own windows for 
 * error messages, whould anyone like to implement this???
 */

/*	Function Name: PrintWarning
 *	Description: This function prints a warning message to stderr.
 *	Arguments: string - the specific warning string.
 *	Returns: none
 */

void
PrintWarning(man_globals, string)
ManpageGlobals * man_globals;
char * string;
{
  char buffer[BUFSIZ];

  sprintf( buffer, "Xman Warning: %s", string);

  if (man_globals != NULL) 
    ChangeLabel(man_globals->label, buffer);

  fprintf(stderr, "%s\n", buffer);
}

/*	Function Name: PrintError
 *	Description: This Function prints an error message and exits.
 *	Arguments: string - the specific message.
 *	Returns: none. - exits tho.
 */

void
PrintError(string)
char * string;
{
  fprintf(stderr,"Xman Error: %s\n",string);
#ifdef DEBUG
  fprintf(stderr,"\n\nbye,bye\n\n\n\n\nsniff...\n");
#endif
  exit(42);
}

/*	Function Name: OpenFile
 *	Description: Assignes a file to the manpage.
 *	Arguments: man_globals - global structure.
 *                 file        - the file pointer.
 *	Returns: none
 */

void
OpenFile(man_globals, file)
ManpageGlobals * man_globals;
FILE * file;
{
  Arg arglist[1];
  Cardinal num_args = 0;

  XtSetArg(arglist[num_args], XtNfile, file); num_args++;
  XtSetValues(man_globals->manpagewidgets.manpage, arglist, num_args);
}


/*	Function Name: FindFilename
 *	Description: Opens the entry file given the entry struct.
 *	Arguments: man_globals - the globals info for this manpage.
 *                 entry - the structure containg the info on the file to open.
 *	Returns: fp - the file pointer
 */

FILE *
FindFilename(man_globals, entry)
ManpageGlobals * man_globals;
char * entry;
{
  FILE * file;
  char path[BUFSIZ], page[BUFSIZ], section[BUFSIZ], *temp;

  temp = CreateManpageName(entry);
  sprintf(man_globals->manpage_title, "The current manual page is: %s.", temp);
  XtFree(temp);
  
  ParseEntry(entry, path, section, page);
  sprintf(man_globals->filename, "%s/%s%c/%s", path, CAT, section[LCAT], page);

/* if we find the formatted manpage then return it */

  if ( (file = fopen(man_globals->filename,"r")) != NULL)
    return(file);

  return(Format(man_globals, entry));
}

/*	Function Name: Format
 *	Description: This funtion formats the manual pages and interfaces
 *                   with the user.
 *	Arguments: man_globals - the psuedo globals
 *                 file - the file pointer to use and return
 *                 entry - the current entry struct.
 *                 current_box - The current directory being displayed. 
 *	Returns: none.
 */

/* ARGSUSED */

FILE *
Format(man_globals, entry)
ManpageGlobals * man_globals; 
char * entry;
{
  FILE * file;
  Widget w = man_globals->manpagewidgets.directory;
  Widget manpage = man_globals->manpagewidgets.manpage;
  char cmdbuf[BUFSIZ], tmp[BUFSIZ], catdir[BUFSIZ];
  char path[BUFSIZ], section[BUFSIZ], error_buf[BUFSIZ];
  XEvent event;
  Position x,y;			/* location to pop up the
				   "would you like to save" widget. */

  strcpy(tmp,MANTEMP);		/* get a temp file. */
  strcpy(man_globals->tmpfile,mktemp(tmp));

/*
 * Replace with XtPopupSync when this becomes avaliable. 
 */

  Popup(XtParent(man_globals->standby), XtGrabExclusive);
  while ( !XCheckTypedWindowEvent(XtDisplay(w), 
				 XtWindow(man_globals->standby), 
				 Expose, &event) );
  XtDispatchEvent( &event );
  XFlush(XtDisplay(w));

/* End replacement. */

  if ( (file = fopen( entry , "r")) == NULL) {
    /* We Really could not find it, this should never happen, yea right. */
    sprintf(error_buf, "Could open manual page file, %s", entry);
    PrintWarning(man_globals, error_buf);
    return(NULL);
  }

  ParseEntry(entry, path, section, NULL);

#ifdef macII
  sprintf(cmdbuf,
        "cd %s;/usr/bin/pcat %s | /usr/bin/col | /usr/bin/ul -t dumb > %s %s",
	path, entry, man_globals->tmpfile, "2> /dev/null");
#else
  sprintf(cmdbuf,"cd %s ; %s %s %s > %s %s", path, TBL,
	  entry, FORMAT, man_globals->tmpfile, "2> /dev/null");
#endif

  if(system(cmdbuf) != 0) {	/* execute search. */
    sprintf(error_buf,
	    "Something went wrong trying to run the command: %s", cmdbuf);
    PrintWarning(man_globals, error_buf);
    return(NULL);
  }

  if ((file = fopen(man_globals->tmpfile,"r")) == NULL) {  
    sprintf(error_buf, "Something went wrong in retrieving the temp file, %s",
	    "Try cleaning up /tmp");
    PrintWarning(man_globals, error_buf);
    return(NULL);
  }

/*
 * If the catdir is writeable then ask the user if he/she wants to
 * write the man page to it. 
 */

  sprintf(catdir,"%s/%s%c", path, CAT, section[LCAT]);

  XtPopdown( XtParent(man_globals->standby) );
  
  if ( (man_globals->save != NULL) && 
       (man_globals->manpagewidgets.manpage != NULL) &&
       ((access(catdir,W_OK)) == 0) )  {
    x = (Position) Width(man_globals->manpagewidgets.manpage)/2;
    y = (Position) Height(man_globals->manpagewidgets.manpage)/2;
    XtTranslateCoords(manpage, x, y, &x, &y);
    PositionCenter( man_globals->save, (int) x, (int) y, 0, 0, 0, 0);
    XtPopup( man_globals->save, XtGrabExclusive);
  }
  else {

/*
 * We do not need the filename anymore, and have the fd open.
 * We will unlink it.     
 */
    unlink(man_globals->tmpfile);
    man_globals->tmpfile[0] = '\0'; /* remove name of tmpfile. */
  }
    
  return(file);
}

/*	Function Name: AddCursor
 *	Description: This function adds the cursor to the window.
 *	Arguments: w - the widget to add the cursor to.
 *                 cursor - the cursor to add to this widget.
 *	Returns: none
 */

void
AddCursor(w,cursor)
Widget w;
Cursor cursor;
{
  XColor colors[2];
  Arg args[10];
  Cardinal num_args = 0;
  Colormap c_map;
  Pixel bg;
  
  if (!XtIsRealized(w)) {
    PrintWarning(NULL, "Widget is not realized, no cursor added.\n");
    return;
  }

  XtSetArg( args[num_args], XtNbackground, &bg); num_args++;
  XtSetArg( args[num_args], XtNcolormap, &c_map); num_args++;
  XtGetValues( w, args, num_args);

  colors[0].pixel = resources.cursors.color;
  colors[1].pixel = bg;

  XQueryColors (XtDisplay(w), c_map, colors, 2);
  XRecolorCursor(XtDisplay(w), cursor, colors, colors+1);
  XDefineCursor(XtDisplay(w),XtWindow(w),cursor);
}

/*	Function Name: ChangeLabel
 *	Description: This function changes the label field of the
 *                   given widget to the string in str.
 *	Arguments: w - the widget.
 *                 str - the string to change the label to.
 *	Returns: none
 */

void
ChangeLabel(w,str)
Widget w;
char * str;
{
  Arg arglist[3];		/* An argument list. */

  if (w == NULL) return;

  XtSetArg(arglist[0], XtNlabel, str);

/* shouldn't really have to do this. */
  XtSetArg(arglist[1], XtNwidth, 0);
  XtSetArg(arglist[2], XtNheight, 0);

  XtSetValues(w, arglist, (Cardinal) 1);
}

/*
 * In an ideal world this would be part of the XToolkit, and I would not
 * have to do it, but such is life sometimes.  Perhaps in X11R3.
 */

/*	Function Name: PositionCenter
 *	Description: This fuction positions the given widgets center
 *                   in the following location.
 *	Arguments: widget - the widget widget to postion
 *                 x,y - The location for the center of the widget
 *                 above - number of pixels above center to locate this widget
 *                 left - number of pixels left of center to locate this widget
 *                 h_space, v_space - how close to get to the edges of the
 *                                    parent window.
 *	Returns: none
 *      Note:  This should only be used with a popup widget that has override
 *             redirect set.
 */

void
PositionCenter(widget,x,y,above,left,v_space,h_space)
Widget widget;
int x,y,above,left;
int h_space,v_space;
{
  int x_temp,y_temp;		/* location of the new window. */
  int parent_height,parent_width; /* Height and width of the parent widget or
				   the root window if it has no parent. */

  x_temp = x - left - Width(widget) / 2 + BorderWidth(widget);
  y_temp = y - above -  Height(widget) / 2 + BorderWidth(widget);

  parent_height = HeightOfScreen(XtScreen(widget));
  parent_width = WidthOfScreen(XtScreen(widget));

/*
 * Check to make sure that all edges are within the viewable part of the
 * root window, and if not then force them to be.
 */

  if (x_temp < h_space) 
    x_temp = v_space;
  if (y_temp < v_space)
    (y_temp = 2);

  if ( y_temp + Height(widget) + v_space > parent_height )
      y_temp = parent_height - Height(widget) - v_space; 

  if ( x_temp + Width(widget) + h_space > parent_width )
      x_temp = parent_width - Width(widget) - h_space; 

  XtMoveWidget(widget,x_temp,y_temp);
}  

/*	Function Name: ParseEntry(entry, path, sect, page)
 *	Description: Parses the manual pages entry filenames.
 *	Arguments: str - the full path name.
 *                 path - the path name.      RETURNED
 *                 sect - the section name.   RETURNED
 *                 page - the page name.      RETURNED
 *	Returns: none.
 */

void
ParseEntry(entry, path, sect, page)
char *entry, *path, *page, *sect;
{
  char *c, temp[BUFSIZ];

  strcpy(temp, entry);

  c = rindex(temp, '/');
  if (c == NULL) 
    PrintError("index failure in ParseEntry.");
  *c++ = '\0';
  if (page != NULL)
    strcpy(page, c);

  c = rindex(temp, '/');
  if (c == NULL) 
    PrintError("index failure in ParseEntry.");
  *c++ = '\0';
  if (sect != NULL)
    strcpy(sect, c);

  if (path != NULL)
    strcpy(path, temp);
}

/*      Function Name: GetGlobals
 *      Description: Gets the psuedo globals associated with the
 *                   manpage associated with this widget.
 *      Arguments: w - a widget in the manpage.
 *      Returns: the psuedo globals.
 *      Notes: initial_widget is a globals variable.
 *             manglobals_context is a global variable.
 */

ManpageGlobals *
GetGlobals(w)
Widget w;
{
  Widget temp;
  caddr_t data;

  while ( (temp = XtParent(w)) != initial_widget && (temp != NULL))
    w = temp;

  if (temp == NULL) 
    XtAppError(XtWidgetToApplicationContext(w), 
	       "Xman: Could not locate widget in tree, exiting");

  if (XFindContext(XtDisplay(w), XtWindow(w),
		   manglobals_context, &data) != XCSUCCESS)
    XtAppError(XtWidgetToApplicationContext(w), 
	       "Xman: Could not find global data, exiting");

  return( (ManpageGlobals *) data);
}
  
/*      Function Name: SaveGlobals
 *      Description: Saves the psuedo globals on the widget passed
 *                   to this function, although GetGlobals assumes that
 *                   the data is associated with the popup child of topBox.
 *      Arguments: w - the widget to associate the data with.
 *                 globals - data to associate with this widget.
 *      Returns: none.
 *      Notes: WIDGET MUST BE REALIZED.
 *             manglobals_context is a global variable.
 */

void
SaveGlobals(w, globals)
Widget w;
ManpageGlobals * globals;
{
  if (XSaveContext(XtDisplay(w), XtWindow(w), manglobals_context,
		   (caddr_t) globals) != XCSUCCESS)
    XtAppError(XtWidgetToApplicationContext(w), 
	       "Xman: Could not save global data, are you out of memory?");
}

/*      Function Name: RemoveGlobals
 *      Description: Removes the psuedo globals from the widget passed
 *                   to this function.
 *      Arguments: w - the widget to remove the data from.
 *      Returns: none.
 *      Notes: WIDGET MUST BE REALIZED.
 *             manglobals_context is a global variable.
 */

void
RemoveGlobals(w)
Widget w;
{
  if (XDeleteContext(XtDisplay(w), XtWindow(w), 
		     manglobals_context) != XCSUCCESS)
    XtAppError(XtWidgetToApplicationContext(w), 
	       "Xman: Could not remove global data?");
}
