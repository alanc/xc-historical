#ifndef lint
static char rcs_id[] = "$XConsortium: util.c,v 1.9 88/09/16 14:44:01 swick Exp $";
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
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission.
 */

#include "xedit.h"
XeditPrintf(fmt, arg1, arg2, arg3, arg4)
  char *fmt;
{
  char buf[1024];
  XawTextBlock text;
  
  XawTextPosition pos = (*messsource->Scan)(messsource, 0, XawstAll, XawsdRight,1,0);
    sprintf(buf, fmt, arg1, arg2, arg3, arg4);
    text.length = strlen(buf);
    text.ptr = buf;
    XawTextReplace( messwidget, pos, pos, &text);
    XawTextSetInsertionPoint(messwidget, pos + text.length);
}

Widget makeCommandButton(box, name, function)
  Window box;
  char *name;
  XtCallbackProc function;
{
    Widget w = XtCreateManagedWidget(name, commandWidgetClass, box, NULL, 0);
    XtAddCallback(w, XtNcallback, function, NULL);
    return w;
}


Widget makeStringBox(parentBox, string, length)
  Widget parentBox;
  char *string;
{
  Arg args[5];
  Widget StringW;
  int numargs;
    numargs = 0;
    MakeArg(XtNeditType, (XtArgVal)XawtextEdit );
    MakeArg(XtNtextOptions, (XtArgVal)( resizeWidth)); 
    MakeArg(XtNstring,(XtArgVal)string);     
    MakeArg(XtNwidth,  (XtArgVal)length);
    MakeArg(XtNlength, (XtArgVal)1000);
    StringW = XtCreateManagedWidget("stringthing", asciiStringWidgetClass, 
					parentBox, args, numargs);
    return(StringW);  
}
 
FixScreen(from)
    XawTextPosition from;
{
    XawTextPosition to;
    if(from >= 0){
        to = (*source->Scan)(source, 0, XawstAll, XawsdRight, 0,0) + 10;
	XawTextInvalidate(textwindow, (from > 0 ) ? from -1 : from, to); 
	XawTextSetInsertionPoint(textwindow, from); 
    } else {
	Feep();
    }
}


