#ifndef lint
static char rcs_id[] = "$XConsortium: commands.c,v 1.20 89/05/11 18:49:22 kit Exp $";
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

#include <stdio.h>
#include "xedit.h"

extern Widget textwindow, labelwindow, filenamewindow;
extern Widget searchstringwindow, replacestringwindow;

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
  if( double_click || !XawAsciiSourceChanged(textwindow) ) {
    exit(0);
  } 
  XeditPrintf("Unsaved changes. Save them, or press Quit again.\n");
  Feep();
  double_click = TRUE;
  AddDoubleClickCallback(textwindow, TRUE);
}

static Boolean
Replace(report_error)
Boolean report_error;
{
  static XawTextPosition Search();
  char * string = GetString(searchstringwindow);
  XawTextPosition pos;
  XawTextBlock text;
  int ret_val;

  if ( (pos = Search(string, XawsdRight, report_error)) == XawTextSearchError)
    return(FALSE);

  text.ptr = GetString(replacestringwindow);
  text.length = strlen(text.ptr);
  text.format = FMT8BIT;
  text.firstPos = 0;

  ret_val = XawTextReplace(textwindow, pos, pos + strlen(string), &text);
  if (ret_val != XawEditDone) {
    XeditPrintf("Replace: Error returned from XtTextReplace.\n");
    Feep();
    return(FALSE);
  }
  
  XawTextSetInsertionPoint(textwindow, pos + text.length);
  XawTextSetSelection(textwindow, pos, pos + text.length);
  return(TRUE);
}

void
DoReplaceOne()
{
  if (!Replace(TRUE)) {
    XeditPrintf("Replace: nothing replaced.\n");
  }
}

void
DoReplaceAll()
{
  int count;

  for ( count = 0; Replace(count == 0) ; count++ );

  if (count == 0) 
    XeditPrintf("ReplaceAll: nothing replaced\n");
  else {
    char buf[BUFSIZ];
    sprintf(buf, "%d Replacement%s made\n", count, ((count>1) ? "s" : "") );
    XeditPrintf(buf);
  }
}

static XawTextPosition
Search(string, direction, report_error)	
char * string;
XawTextScanDirection direction;
Boolean report_error;
{
  XawTextBlock text;
  XawTextPosition pos;

  text.ptr = string;
  text.length = strlen(string);
  text.format = FMT8BIT;
  text.firstPos = 0;

  pos = XawTextSearch(textwindow, direction, &text);
  
  if ((pos == XawTextSearchError) && (report_error)) {
    char buf[BUFSIZ];
    sprintf(buf, "Search: couldn't find ` %s '.\n", text.ptr); 
    XeditPrintf(buf);
    Feep();
  }
  return(pos);
}

void
DoSearchRight()
{
  char * string = GetString(searchstringwindow);
  XawTextPosition pos = Search(string, XawsdRight, TRUE);
  int len;

  if ( pos == XawTextSearchError) return;

  len = strlen(string);
  XawTextSetInsertionPoint(textwindow, pos + len);
  XawTextSetSelection(textwindow, pos, pos + len);
}

void
DoSearchLeft()
{
  char * string = GetString(searchstringwindow);
  XawTextPosition pos = Search(string, XawsdLeft, TRUE);

  if ( pos == XawTextSearchError) return;

  XawTextSetInsertionPoint(textwindow, pos);
  XawTextSetSelection(textwindow, pos, pos + strlen(string));
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
  
  if( !XawAsciiSourceChanged(textwindow) ) {
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
  
  if (XawAsciiSave(textwindow)) 
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

  if ( XawAsciiSourceChanged(textwindow) && !double_click) {
    XeditPrintf("Unsaved changes. Save them, or press Load again.\n");
    Feep();
    double_click = TRUE;
    AddDoubleClickCallback(textwindow, TRUE);
    return;
  }
  double_click = FALSE;

  if ((strlen(filename)&&access(filename, R_OK) == 0)) {
    XtSetArg(args[num_args], XtNstring, filename); num_args++;
    XtSetValues(textwindow, args, num_args);

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

void
DoJump()
{
  XeditPrintf("NIY - CDP 7/9/89.\n");
/*
  XeditPrintf("Please 'Select' a line number and try again.\n");
*/
}
