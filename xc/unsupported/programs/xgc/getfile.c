/*
** getfilename.c
**
*/

#include <X11/Intrinsic.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/StringDefs.h>
#include <stdio.h>

#include "xgc.h"

extern XStuff X;
extern Widget topform;

static Widget popupshell;	/* popup dialog box */
char filename[60];		/* the name of the file the user
				   has selected */
extern XtAppContext appcontext;

void kill_popup_shell();

void
get_filename(success,failure) 
     void (*success)();		/* what function to call when a filename is
				   chosen */
     void (*failure)();		/* what function to call when the user
				   cancels */
{
  static Widget popupform;	/* form inside shell */
  static Widget label;		/* "Filename :" */
  static Widget text;		/* where the user edits filename */
  static Widget cancel;		/* command, select to cancel */

  Window dummy1, dummy2;
  int x1,y1,x2,y2,mask;

  /* The translation table for the text widget.  Things such as <RETURN>
  ** confirm the user's choice.  Other keys which would move out of
  ** the range of a one-line window are disabled. */

  static char *translationtable = 
    "Ctrl<Key>J:    KillPopup() Done()\n\
     Ctrl<Key>M:    KillPopup() Done()\n\
     <Key>Linefeed: KillPopup() Done()\n\
     <Key>Return:   KillPopup() Done()\n\
     Ctrl<Key>O:    Nothing()\n\
     Meta<Key>I:    Nothing()\n\
     Ctrl<Key>N:    Nothing()\n\
     Ctrl<Key>P:    Nothing()\n\
     Ctrl<Key>Z:    Nothing()\n\
     Meta<Key>Z:    Nothing()\n\
     Ctrl<Key>V:    Nothing()\n\
     Meta<Key>V:    Nothing()";

  /* What the actions in the translation table correspond to. */

  static XtActionsRec actiontable[] = {
    {"KillPopup", (XtActionProc) kill_popup_shell},
    {"Done",      NULL},
    {"Nothing",   NULL}
  };

  static Arg popupshellargs[] = {     /* Where to put the popup shell. */
    {XtNx,         (XtArgVal) NULL},
    {XtNy,         (XtArgVal) NULL}
  };

  static Arg labelargs[] = {	/* ArgList for the label */
    {XtNborderWidth,    (XtArgVal) 0},
    {XtNjustify,        (XtArgVal) XtJustifyRight}
  };

  static Arg textargs[] = {	/* ArgList for the text widget */
    {XtNeditType,       (XtArgVal) XawtextEdit},
    {XtNstring,         (XtArgVal) NULL},
    {XtNlength,         (XtArgVal) NULL},
    {XtNwidth,          (XtArgVal) NULL},
    {XtNhorizDistance,  (XtArgVal) 10},
    {XtNfromHoriz,      (XtArgVal) NULL},
    {XtNinsertPosition, (XtArgVal) NULL}
  };

  static Arg cancelargs[] = {	/* ArgList for the cancel button */
    {XtNfromHoriz,      (XtArgVal) NULL},
    {XtNcallback,       (XtArgVal) NULL}
  };

  /* Procedures to call when the user selects 'cancel' */
  static XtCallbackRec cancelcallbacklist[] = {
    {(XtCallbackProc) kill_popup_shell, NULL},
    {NULL, NULL},
    {NULL, NULL}
  };

  /* Find out where the pointer is, so we can put the popup window there */

  (void) XQueryPointer(X.dpy,XtWindow(topform),&dummy1,&dummy2,&x1,&y1,
		       &x2,&y2,&mask);
  
  popupshellargs[0].value = (XtArgVal) x2;
  popupshellargs[1].value = (XtArgVal) y2;
  
  popupshell = XtCreatePopupShell("popup",overrideShellWidgetClass,
			 topform,popupshellargs,XtNumber(popupshellargs));

  popupform = XtCreateManagedWidget("form",formWidgetClass,popupshell,
			    NULL, 0);

  label = XtCreateManagedWidget("Filename: ",labelWidgetClass,popupform,
			       labelargs,XtNumber(labelargs));

  textargs[1].value = (XtArgVal) filename;
  textargs[2].value = (XtArgVal) 40;
  textargs[3].value = (XtArgVal) 200;
  textargs[5].value = (XtArgVal) label;
  textargs[6].value = (XtArgVal) strlen(filename);

  text = XtCreateManagedWidget("text",asciiStringWidgetClass,popupform,
			    textargs,XtNumber(textargs));

  /* Complete the action table.  We have to do it here because success
  ** isn't known at compile time. */

  actiontable[1].proc = (XtActionProc) success;

  /* Register actions, translations, callbacks */

  XtAppAddActions(appcontext,actiontable,XtNumber(actiontable));
  XtOverrideTranslations(text,XtParseTranslationTable(translationtable));
  cancelcallbacklist[1].callback = (XtCallbackProc) failure;

  cancelargs[0].value = (XtArgVal) text;
  cancelargs[1].value = (XtArgVal) cancelcallbacklist;

  cancel = XtCreateManagedWidget("Cancel",commandWidgetClass,popupform,
				 cancelargs,XtNumber(cancelargs));

  /* Bring up the popup.  When the user presses cancel or the return key,
  ** the function kill_popup_shell (below) will be called to remove it. */

  XtPopup(popupshell,XtGrabExclusive);
}

/* kill_popup_shell()
** ------------------
** Remove the popup window that get_filename popped up.
*/

static void
kill_popup_shell()
{
  XtPopdown(popupshell);
  XtDestroyWidget(popupshell);
}
