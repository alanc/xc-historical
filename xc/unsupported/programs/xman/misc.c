/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: misc.c,v 1.2 88/09/06 17:48:18 jim Exp $
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
  static char rcs_version[] = "$Athena: misc.c,v 4.0 88/08/31 22:13:01 kit Exp $";
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
PrintWarning(string)
char * string;
{
  fprintf(stderr,"Xman Warning: %s\n",string);
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

/*	Function Name: FindFilename
 *	Description: Opens the entry file given the entry struct.
 *	Arguments: man_globals - the globals info for this manpage.
 *                 name - name of the current manpage.
 *                 entry - the structure containg the info on the file to open.
 *	Returns: fp - the file pointer
 */

FILE *
FindFilename(man_globals, name, section, entry)
ManpageGlobals * man_globals;
char * name;
int section;
struct entry * entry;
{
  FILE * file;
  char local_filename[BUFSIZ];	/* local filename. */

  sprintf(man_globals->manpage_title,
	  "The current manual page is: %s.", name);   
  
  sprintf(man_globals->filename, "%s/%s%c/%s",
	  entry->path, CAT,  manual[section].sect[LCAT], entry->label);

/* if we find the formatted manpage then return it */

  if ( (file = fopen(man_globals->filename,"r")) != NULL)
    return(file);

/* If not then look for the unformatted man page, format it and 
 * write access is allowed to the cat directory, then ask if we
 * want to save it.
 */

  sprintf(local_filename,"%s/%s/%s",
	  entry->path, manual[section].sect, entry->label);
  if ( (file = fopen(local_filename,"r")) == NULL) {
    char error_buf[BUFSIZ];
    /* We Really could not find it, this should never happen, yea right. */
    sprintf(error_buf, "Could open manual page file, %s", local_filename);
    PrintError(error_buf);
  }
  else 
    return(Format(man_globals,file,local_filename, entry, section));
}

/*	Function Name: Format
 *	Description: This funtion formats the manual pages and interfaces
 *                   with the user.
 *	Arguments: man_globals - the psuedo globals
 *                 file - the file pointer to use and return
 *                 local_filename - the name of the file that is 
 *                                  being formatted.
 *                 entry - the current entry struct.
 *                 current_box - The current directory being displayed. 
 *	Returns: none.
 */

/* ARGSUSED */

FILE *
Format(man_globals, file, local_filename, entry, current_box)
ManpageGlobals * man_globals; 
FILE * file;
char * local_filename;
struct entry * entry;
int current_box;		/* The current directory being displayed.  */
{
  Widget w = man_globals->manpagewidgets.directory;
  char cmdbuf[256];
  char tmp[25];
  char catdir[100];
  int x,y;			/* location to pop up whould you 
				   like to save widget. */

  strcpy(tmp,MANTEMP);		/* get a temp file. */
  strcpy(man_globals->tmpfile,mktemp(tmp));

/*
 * Replace with XtPopupSync when this becomes avaliable. 
 * This section of code does not work, and I have not got time
 * to f*ck with it.
 */

/*
  PopUpMenu(w,NULL,NULL);
  while ( !XCheckTypedWindowEvent(XtDisplay(w), 
				 XtWindow(man_globals->standby1), 
				 Expose, &event) );
  XtDispatchEvent( &event );
  while ( !XCheckTypedWindowEvent(XtDisplay(w), 
				 XtWindow(man_globals->standby2), 
				 Expose, &event) );
  XtDispatchEvent( &event );
  XFlush(XtDisplay(w));
*/
/* End replacement. */
/*
  ChangeLabel(man_globals->label, "Formatting Manpage, Please Stand by...");
*/
#ifdef macII
  sprintf(cmdbuf,"cd %s;/usr/bin/pcat %s | /usr/bin/col | /usr/bin/ul -t dumb > %s",
	  entry->path, local_filename, man_globals->tmpfile);
#else
  sprintf(cmdbuf,"cd %s;%s %s > %s",
	  entry->path,
	  FORMAT, local_filename, man_globals->tmpfile);
#endif

  if(system(cmdbuf) != 0) 	/* execute search. */
    PrintError("Something went wrong trying to run the command");

  if ((file = fopen(man_globals->tmpfile,"r")) == NULL) {  
    PrintWarning("Something went wrong in retrieving the temp file");
    PrintError("Try cleaning up /tmp");
  }

/* if the catdir is writeable the ask the user if he/she wants to
   write the man page to it. */

  sprintf(catdir,"%s/%s%c",
	  entry->path,CAT, manual[current_box].sect[LCAT], entry->label);
  
  if( (access(catdir,W_OK)) == 0)  {
    x = Width(man_globals->manpagewidgets.manpage)/2;
    y = Height(man_globals->manpagewidgets.manpage)/2;
    XtTranslateCoords(man_globals->manpagewidgets.manpage, x, y, &x, &y);
    PositionCenter( PopupChild(man_globals->manpagewidgets.manpage, 0),
		   x,y,0,0,0,0);
    XtPopup( PopupChild(man_globals->manpagewidgets.manpage, 0),
	    XtGrabExclusive);
  }
  XtPopdown( PopupChild(w, 0) );
  
  return(file);
}

/*	Function Name: KillManpage
 *	Description: This function kills off a manpage display and cleans up
 *                   after it.
 *	Arguments: man_globals - the psuedo global structure.
 *	Returns: none.
 */

void
KillManpage(man_globals)
ManpageGlobals * man_globals;
{
  if (man_globals->This_Manpage != help_widget) {
    XtDestroyWidget(man_globals->This_Manpage);
    free(man_globals);
    man_pages_shown--;
  }
  else
    XtPopdown(help_widget);
    
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

  if (!XtIsRealized(w)) {
    PrintWarning("Widget is not realized, no cursor added.\n");
    return;
  }
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

