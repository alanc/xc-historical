#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: commands.c,v 1.22 89/07/21 19:52:50 kit Exp $";
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

extern Widget textwindow, labelwindow, filenamewindow;

static Boolean double_click = FALSE;

/*	Function Name: AddDoubleClickCallback(w)
 *	Description: Adds a callback that will reset the double_click flag
 *                   to false when the text is changed.
 *	Arguments: w - widget to set callback upon.
 *                 state - If true add the callback, else remove it.
 *	Returns: none.
 */

AddDoubleClickCallback(w, state)
Widget w;
Boolean state;
{
  static void ResetDC();
  Arg args[1];
  static XtCallbackRec cb[] = { {NULL, NULL}, {NULL, NULL} };
 
  if (state) 
    cb[0].callback = ResetDC;
  else
    cb[0].callback = NULL;

  XtSetArg(args[0], XtNcallback, cb);
  XtSetValues(w, args, ONE);
}
  
/*	Function Name: ResetDC
 *	Description: Resets the double click flag.
 *	Arguments: w - the text widget.
 *                 junk, garbage - *** NOT USED ***
 *	Returns: none.
 */

/* ARGSUSED */
static void
ResetDC(w, junk, garbage)
Widget w;
caddr_t junk, garbage;
{
  double_click = FALSE;

  AddDoubleClickCallback(w, FALSE);
}

void
DoQuit()
{
  if( double_click || !XawAsciiSourceChanged(XawTextGetSource(textwindow)) ) {
    exit(0); 
  } 
  XeditPrintf("Unsaved changes. Save them, or press Quit again.\n");
  Feep();
  double_click = TRUE;
  AddDoubleClickCallback(textwindow, TRUE);
}

char *
makeBackupName(buf, filename)
String buf, filename;
{
  sprintf(buf, "%s%s%s", app_resources.backupNamePrefix,
	  filename, app_resources.backupNameSuffix);
  return (buf);
}
  
void
DoSave()
{
  String filename = GetString(textwindow);
  char buf[BUFSIZ];

  if( (filename == NULL) || (strlen(filename) == 0) ){
    XeditPrintf("Save:  no filename specified -- nothing saved\n");
    Feep();
    return;
  }
  
  if( !XawAsciiSourceChanged(XawTextGetSource(textwindow)) ) {
    XeditPrintf("Save:  no changes to save -- nothing saved\n");
    Feep();
    return;
  }
  
  if (app_resources.enableBackups) {
    char backup_file[BUFSIZ];
    makeBackupName(backup_file, filename);

    if (rename(filename, backup_file) != 0) {
      sprintf(buf, "error backing up file:  %s\n",  backup_file); 
      XeditPrintf(buf);
    }
  }
  
  if ( XawAsciiSave(XawTextGetSource(textwindow)) ) 
    sprintf(buf, "Saved file:  %s\n", filename);
  else 
    sprintf(buf, "Error saving file:  %s\n",  filename);

  XeditPrintf(buf);
}

void
DoLoad()
{
  Arg args[5];
  Cardinal num_args = 0;
  String filename = GetString(filenamewindow);

  if ( XawAsciiSourceChanged(XawTextGetSource(textwindow)) && !double_click) {
    XeditPrintf("Unsaved changes. Save them, or press Load again.\n");
    Feep();
    double_click = TRUE;
    AddDoubleClickCallback(textwindow, TRUE);
    return;
  }
  double_click = FALSE;

  if ((strlen(filename)&&access(filename, R_OK) == 0)) {
    XtSetArg(args[num_args], XtNstring, filename); num_args++;
    XtSetValues( textwindow, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNlabel, filename); num_args++;
    XtSetValues( labelwindow, args, num_args);
  }
  else {
    char buf[BUFSIZ];
    sprintf(buf, "Load: couldn't access file ` %s '.\n", filename);
    XeditPrintf(buf);
    Feep();
  }
}
