#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: util.c,v 1.12 89/07/21 19:52:54 kit Exp $";
#endif /* lint && SABER */

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

#include <stdio.h>
#include "xedit.h"

extern Widget messwidget;

void
XeditPrintf(str)
char * str;
{
  XawTextBlock text;
  static XawTextPosition pos = 0;

  text.length = strlen(str);
  text.ptr = str;
  text.firstPos = 0;
  text.format = FMT8BIT;

  XawTextReplace( messwidget, pos, pos, &text);

  pos += text.length;
  XawTextSetInsertionPoint(messwidget, pos);
}

Widget
MakeCommandButton(box, name, function)
Widget box;
char *name;
XtCallbackProc function;
{
  Widget w = XtCreateManagedWidget(name, commandWidgetClass, box, NULL, ZERO);
  if (function != NULL)
    XtAddCallback(w, XtNcallback, function, (caddr_t) NULL);
  return w;
}

Widget 
MakeStringBox(parent, name, string)
Widget parent;
String name, string;
{
  Arg args[5];
  Cardinal numargs = 0;
  Widget StringW;

  XtSetArg(args[numargs], XtNeditType, XawtextEdit); numargs++;
  XtSetArg(args[numargs], XtNstring, string); numargs++;

  StringW = XtCreateManagedWidget(name, asciiTextWidgetClass, 
				  parent, args, numargs);
  return(StringW);  
}
 
/*	Function Name: GetString
 *	Description: retrieves the string from a asciiText widget.
 *	Arguments: w - the ascii text widget.
 *	Returns: the filename.
 */

String
GetString(w)
Widget w;
{
  String str;
  Arg arglist[1];
  
  XtSetArg(arglist[0], XtNstring, &str);
  XtGetValues(w, arglist, ONE);
  return(str);
}
