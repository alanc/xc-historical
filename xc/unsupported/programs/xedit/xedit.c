#ifndef lint
static char rcs_id[] = "$XConsortium: xedit.c,v 1.18 89/05/11 18:49:41 kit Exp $";
#endif
 
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be 
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

#include "xedit.h"

Widget textwindow, messwidget, labelwindow, filenamewindow;
Widget searchstringwindow, replacestringwindow;

Display *CurDpy;

struct _app_resources app_resources;

#define offset(field) XtOffset(struct _app_resources*, field)
static XtResource resources[] = {
   {"enableBackups", "EnableBackups", XtRBoolean, sizeof(Boolean),
         offset(enableBackups), XtRImmediate, FALSE},
   {"backupNamePrefix", "BackupNamePrefix", XtRString, sizeof(char *),
         offset(backupNamePrefix),XtRString, ""},
   {"backupNameSuffix", "BackupNameSuffix", XtRString, sizeof(char *),
         offset(backupNameSuffix),XtRString, ".BAK"}
};
#undef offset

void
main(argc, argv)
int argc;
char **argv;
{
  Widget toplevel;
  String filename = NULL;
  static void makeButtonsAndBoxes();

  toplevel = XtInitialize( "xedit", "Xedit", NULL, 0, &argc, argv);
  
  XtGetApplicationResources(toplevel, &app_resources, resources,
			    XtNumber(resources), NULL, 0);

  CurDpy = XtDisplay(toplevel);

  if (argc > 1) 
    filename = argv[1];
  makeButtonsAndBoxes(toplevel, filename);
  XtRealizeWidget(toplevel);
  XDefineCursor(XtDisplay(toplevel),XtWindow(toplevel),
		XCreateFontCursor( XtDisplay(toplevel), XC_left_ptr));
  XtMainLoop();
}

static void
makeButtonsAndBoxes(parent, filename)
Widget parent;
char * filename;
{
  Widget outer, Row1, Row2;

  Arg arglist[10];
  Cardinal num_args;

  outer = XtCreateManagedWidget( "vpaned", panedWidgetClass, parent,
				NULL, 0 );
  
  Row1 = XtCreateManagedWidget("row1", panedWidgetClass, outer, NULL,0);
  {
    MakeCommandButton(Row1, "quit", DoQuit);
    MakeCommandButton(Row1, "save", DoSave);
    MakeCommandButton(Row1, "load", DoLoad);
    filenamewindow = MakeStringBox(Row1, "filename", filename); 
    MakeCommandButton(Row1, "jump", DoJump);
  }
  Row2 = XtCreateManagedWidget("row2", panedWidgetClass, outer, NULL,0);
  {
    MakeCommandButton(Row2, "searchLeft", DoSearchLeft);
    MakeCommandButton(Row2,"searchRight",DoSearchRight); 
    searchstringwindow = MakeStringBox(Row2, "searchString", NULL); 
    MakeCommandButton(Row2, "replace", DoReplaceOne);
    MakeCommandButton(Row2, "all", DoReplaceAll);
    replacestringwindow = MakeStringBox(Row2, "replaceString", NULL); 
  }
  
  num_args = 0;
  XtSetArg(arglist[num_args], XtNtextOptions, (scrollVertical | wordBreak) );
  num_args++;
  XtSetArg(arglist[num_args], XtNeditType, XawtextEdit); num_args++;
  messwidget = XtCreateManagedWidget("messageWindow", asciiTextWidgetClass,
				      outer, arglist, num_args);

  num_args = 0;
  if (filename != NULL) {
    XtSetArg(arglist[num_args], XtNlabel, filename); num_args++;
  }
  labelwindow = XtCreateManagedWidget("labelWindow",labelWidgetClass, 
				      outer, arglist, num_args);

  num_args = 0;
  XtSetArg(arglist[num_args], XtNtextOptions, (scrollVertical | wordBreak) );
  num_args++;
  XtSetArg(arglist[num_args], XtNstring, filename); num_args++;
  XtSetArg(arglist[num_args], XtNtype, XawAsciiFile); num_args++;
  XtSetArg(arglist[num_args], XtNeditType, XawtextEdit); num_args++;
  textwindow =  XtCreateManagedWidget("editWindow", asciiTextWidgetClass, 
				      outer, arglist, num_args);
}

/*	Function Name: Feep
 *	Description: feeps the bell.
 *	Arguments: none.
 *	Returns: none.
 */

void
Feep()
{
  XBell(CurDpy, 0);
}
