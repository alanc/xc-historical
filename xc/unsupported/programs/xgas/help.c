/*
 * help.c
 *   xgas: Copyright 1991 Larry Medwin: @(#)gas.c	1.9 2/9/90
 *   Larry Medwin -- April 11, 1991, added help stuff.
 */

#include "xgas.h"
#include <X11/Shell.h>
extern char quick[];
extern char man[];
extern char doc1[], doc2[], doc3[], doc4[], doc5[], doc6[], doc7[];

/* static to allow show_text callback to set its args. */
static Widget helpText;

/* Temporarily put the help callbacs here*/
void help_callback(w, widget_to_popup, call_data) /* ARGSUSED */
     Widget     w;
     Widget     widget_to_popup;
     caddr_t    call_data;
{
     XtPopup( widget_to_popup, XtGrabNone);
}

void show_text(w, text, call_data) /* ARGSUSED */
    Widget      w;
    char    	*text;
    caddr_t     call_data;
{
    Arg wargs[1];

    XtSetArg( wargs[0], XtNstring, text);
    XtSetValues( helpText, wargs, 1);
}

void pop_down(w, widget_to_popdown, call_data) /* ARGSUSED */
     Widget     w;
     Widget     widget_to_popdown;
     caddr_t    call_data;
{
     XtPopdown( widget_to_popdown);
}


createHelpWidgets( parent )
     Widget parent;
{
  Widget help, blank1, blank2;
  Widget helpShell, helpFrame, helpQuick, helpMan;
  Widget helpDoc, helpBlank, helpQuit;
  
  /* HELP POPUP */
  helpShell = XtVaCreatePopupShell("helpShell",
				topLevelShellWidgetClass, parent,
				 NULL);
  /* Now add callback for help button */
  XtAddCallback(parent, XtNcallback, (XtCallbackProc)help_callback,
		(XtPointer)helpShell);
  
  /* HELP FRAME */
  helpFrame = XtVaCreateManagedWidget("helpFrame",
				formWidgetClass, helpShell,
				 NULL);

  /* HELP COMMAND BUTTON: QUIT */
  helpQuit = XtVaCreateManagedWidget("helpQuit", commandWidgetClass,
			helpFrame,
			NULL);
  XtAddCallback(helpQuit, XtNcallback, (XtCallbackProc)pop_down, 
		(XtPointer)helpShell);

  /* HELP COMMAND BUTTON: QUICK */
  helpQuick = XtVaCreateManagedWidget("helpQuick", commandWidgetClass,
			helpFrame,
			XtNfromHoriz, (XtPointer)helpQuit,
			XtNhorizDistance, 100,
			NULL);
  XtAddCallback(helpQuick, XtNcallback, (XtCallbackProc)show_text, 
		(char*)quick);
  
  /* HELP COMMAND BUTTON: MAN */
  helpMan = XtVaCreateManagedWidget("helpMan", commandWidgetClass, helpFrame,
			XtNfromHoriz, (XtPointer)helpQuick,
			NULL);
  XtAddCallback(helpMan, XtNcallback, (XtCallbackProc)show_text, (char*)man);
  
  /* HELP COMMAND BUTTON: DOC */
  helpDoc = XtVaCreateManagedWidget("helpDoc", commandWidgetClass, helpFrame,
			XtNfromHoriz, (XtPointer)helpMan,
			NULL);
  XtAddCallback(helpDoc, XtNcallback, (XtCallbackProc)show_text, 
		(char*)strcat(doc1, strcat(doc2, strcat(doc3, strcat(doc4,
			 strcat(doc5, strcat(doc6, doc7)))))));
  
  /* HELP TEXT */
  helpText = XtVaCreateManagedWidget("helpText",
			asciiTextWidgetClass, helpFrame,
			XtNfromVert, (XtPointer)helpQuit,
			/*XtNtype, XawAsciiString,*/
			/*XtNuseStringInPlace, True,*/
			NULL);
}
