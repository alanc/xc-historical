#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: Text.c,v 1.92 89/07/07 14:28:43 kit Exp $";
#endif /* lint && SABER */

/***********************************************************
Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h> 

#include <X11/Xmu/Xmu.h>
#include <X11/Xmu/Misc.h>

#include <X11/Xaw/TextP.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Toggle.h>

extern int errno, sys_nerr;
extern char* sys_errlist[];

#define INSERT_FILE ("Enter Filename:")

#define SEARCH_LABEL  ("Enter String To Search For:")
#define FORM_NAME     ("form")
#define LABEL_NAME    ("label")
#define TEXT_NAME     ("text")
#define REPLACE_NAME  ("replace")
#define LEFT_NAME     ("left")
#define RIGHT_NAME    ("right")

#define R_OFFSET      1

static void CenterWidgetOnPoint();

static void CancelSearch(), DoSearch(), DoReplace(), DoInsert();
static void AddSearchChildren(), AddInsertFileChildren();
static void InitializeSearchWidget();
static Boolean SetResourceByName();
static String GetString();

/************************************************************
 * 
 * Functions internal to text widget, but used by other moduals.
 *
 ************************************************************/

/* ARGSUSED */
void 
_XawTextInsertFileFromDialog(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal num_params;
{
  DoInsert(w, (caddr_t) XtParent(XtParent(XtParent(w))), NULL);
}

/* ARGSUSED */
void 
_XawTextDoSearchAction(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal num_params;
{
  DoSearch(w, (caddr_t) XtParent(XtParent(XtParent(w))), NULL);
}

void 
_XawTextInsertFile(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal * num_params;
{
  TextWidget ctx = (TextWidget)w;
  Position x, y;
  static Widget CreateFileDialog();
  char * ptr;

  if (ctx->text.source->edit_mode != XawtextEdit) {
    XBell(XtDisplay(w), 0);
    return;
  }
  
  _XawTextStartAction(ctx, event);

  if (*num_params == 0) 
    ptr = "";
  else 
    ptr = params[0];
    
  XtTranslateCoords(w, ctx->text.ev_x, ctx->text.ev_y, &x, &y);
  
  if (!ctx->text.file_insert) {
    ctx->text.file_insert = CreateFileDialog(w, ptr);
    XtRealizeWidget(ctx->text.file_insert);
  }

  CenterWidgetOnPoint(ctx->text.file_insert, x, y);
  XtPopup(ctx->text.file_insert, XtGrabNone);

  _XawTextEndAction(ctx);
}

#define SEARCH_HEADER ("Text Widget - Search():")

void 
_XawTextSearch(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal * num_params;
{
  TextWidget ctx = (TextWidget)w;
  Position x, y;
  static Widget CreateSearchDialog();
  XawTextScanDirection dir;
  char * ptr, buf[BUFSIZ];

  if (*num_params < 1) {
    sprintf(buf, "%s %s\n%s", SEARCH_HEADER, "This action must have only", 
	    "one or two parameters");
    XtAppWarning(XtWidgetToApplicationContext(w), buf);
    return;
  }
  else if (*num_params == 1) 
    ptr = "";
  else 
    ptr = params[1];

  switch(params[0][0]) {
  case 'l':			/* Left. */
  case 'L':
    dir = XawsdLeft;
    break;
  case 'r':			/* Right. */
  case 'R':
    dir = XawsdRight;
    break;
  default:
    sprintf(buf, "%s %s\n%s", SEARCH_HEADER, "The first parameter must be",
	    "Either 'left' or 'right'");
    XtAppWarning(XtWidgetToApplicationContext(w), buf);
    return;
  }

  _XawTextStartAction(ctx, event);    
  
  if (ctx->text.search_widget == NULL) {
    ctx->text.search_widget = CreateSearchDialog(w, ptr);
    XtRealizeWidget(ctx->text.search_widget);
  }

#ifdef notdef
  InitializeSearchWidget(ctx->text.search_widget, dir, 
			 (ctx->text.source->edit_mode == XawtextEdit) );
#else
  InitializeSearchWidget(ctx->text.search_widget, dir, FALSE);
#endif

  XtTranslateCoords(w, ctx->text.ev_x, ctx->text.ev_y, &x, &y);
  CenterWidgetOnPoint(ctx->text.search_widget, x, y);

  XtPopup(ctx->text.search_widget, XtGrabNone);

  _XawTextEndAction(ctx);
}

/************************************************************
 *
 * Routines local to this module.
 *
 *************************************************************/

/* ARGSUSED */
static void 
PopDownFileInsert(w, closure, call_data)
Widget w;			/* The Dialog Button Pressed. */
caddr_t closure;		/* Text Widget. */
caddr_t call_data;		/* unused */
{
  TextWidget ctx = (TextWidget) closure;

  XtPopdown( ctx->text.file_insert );
  (void) SetResourceByName(XtParent(w), LABEL_NAME, 
			   XtNlabel, (XtArgVal) INSERT_FILE);
}

/* 
 * Insert a file of the given name into the text.  Returns TRUE if
 * file found, False if it wasn't.
 */

static Boolean
InsertFileNamed(ctx, str)
TextWidget ctx;
char *str;
{
  int fid;
  XawTextBlock text;
  char    buf[BUFSIZ];
  XawTextPosition * pos = &(ctx->text.insertPos);

  if ( (str == NULL) || (strlen(str) == 0) || 
       ((fid = open(str, O_RDONLY)) <= 0))
    return(FALSE);

  _XawTextPrepareToUpdate(ctx);
  text.firstPos = 0;
  text.format = FMT8BIT;
  while ((text.length = read(fid, buf, BUFSIZ)) > 0) {
    text.ptr = buf;
    (void) _XawTextReplace(ctx, *pos, *pos, &text);
    *pos = (*ctx->text.source->Scan)(ctx->text.source, *pos, XawstPositions,
				     XawsdRight, text.length, TRUE);
  }
  (void) close(fid);
  _XawTextExecuteUpdate(ctx);
  return(TRUE);
}

/* ARGSUSED */
static void 
DoInsert(w, closure, call_data)
Widget w;			/* The Dialog Button Pressed. */
caddr_t closure;		/* Text Widget */
caddr_t call_data;		/* unused */
{
  TextWidget ctx = (TextWidget) closure;
  
  if ( !InsertFileNamed(ctx, GetString(XtParent(w))) ) {
    char msg[BUFSIZ];

    sprintf( msg, "*** Error: %s ***",
	    (errno > 0 && errno < sys_nerr) ?
	    sys_errlist[errno] : "Can't open file" );

    (void) SetResourceByName(XtParent(w), LABEL_NAME,
			     XtNlabel, (XtArgVal) msg);
    XBell(XtDisplay(w), 0);
  }
  else 
    PopDownFileInsert(w, closure, call_data);
}

/*
 * This function moves the widget so that its center is on
 * the point passed.  This function assumes that widget's width
 * and height are fixed, thus this should be called after the widget
 * is realized.
 */

static void
CenterWidgetOnPoint(w, x, y)
Widget w;
Position x, y;
{
  Arg args[3];
  Cardinal num_args;
  Dimension width, height, b_width;
  Position max_x, max_y;

  num_args = 0;
  XtSetArg(args[num_args], XtNwidth, &width); num_args++;
  XtSetArg(args[num_args], XtNheight, &height); num_args++;
  XtSetArg(args[num_args], XtNborderWidth, &b_width); num_args++;
  XtGetValues(w, args, num_args);

  width += 2 * b_width;
  height += 2 * b_width;

  x -= ( (Position) width/2 );
  if (x < 0) x = 0;
  if ( x > (max_x = (Position) (XtScreen(w)->width - width)) ) x = max_x;

  y -= ( (Position) height/2 );
  if (y < 0) y = 0;
  if ( y > (max_y = (Position) (XtScreen(w)->height - height)) ) y = max_y;
  
  num_args = 0;
  XtSetArg(args[num_args], XtNx, x); num_args++;
  XtSetArg(args[num_args], XtNy, y); num_args++;
  XtSetValues(w, args, num_args);
}

static Widget
CreateFileDialog(w, ptr)
Widget w;
String ptr;
{
  Widget popup, form;
  Arg args[3];
  Cardinal num_args;

  num_args = 0;
  XtSetArg(args[num_args], XtNiconName, "insertFile"); num_args++;
  XtSetArg(args[num_args], XtNgeometry, NULL); num_args++;
  XtSetArg(args[num_args], XtNallowShellResize, TRUE); num_args++;
  popup = XtCreatePopupShell("insertFile", transientShellWidgetClass, w,
			     args, XtNumber(args) );
  
  form = XtCreateManagedWidget(FORM_NAME, formWidgetClass, popup,
				 NULL, ZERO);

  AddInsertFileChildren(form, ptr, w);
  return(popup);
}

static void
AddInsertFileChildren(form, ptr, tw)
Widget form, tw;
char * ptr;
{
  Arg args[10];
  Cardinal num_args;
  Widget label, text, cancel, insert;
  XtTranslations trans;

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, INSERT_FILE);num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNresizable, TRUE ); num_args++;
  XtSetArg(args[num_args], XtNborderWidth, 0 ); num_args++;
  label = XtCreateManagedWidget(LABEL_NAME, labelWidgetClass, form,
				args, num_args);
  
  num_args = 0;
  XtSetArg(args[num_args], XtNfromVert, label); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNeditType, XawtextEdit); num_args++;
  XtSetArg(args[num_args], XtNresizable, TRUE); num_args++;
  XtSetArg(args[num_args], XtNtextOptions, resizeWidth); num_args++;
  XtSetArg(args[num_args], XtNstring, ptr); num_args++;
  text = XtCreateManagedWidget(TEXT_NAME, asciiTextWidgetClass, form,
				args, num_args);

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Cancel"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, text); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  cancel = XtCreateManagedWidget("cancel", commandWidgetClass, form,
				 args, num_args);

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Insert File"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, text); num_args++;
  XtSetArg(args[num_args], XtNfromHoriz, cancel); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  insert = XtCreateManagedWidget("insert", commandWidgetClass, form,
				 args, num_args);

  XtAddCallback(cancel, XtNcallback, PopDownFileInsert, (caddr_t) tw);
  XtAddCallback(insert, XtNcallback, DoInsert, (caddr_t) tw);

  XtSetKeyboardFocus(form, text);

/*
 * Bind <CR> to insert file.
 */

  trans = XtParseTranslationTable("<Key>0xFF0D: InsertFileFromDialog()");
  XtOverrideTranslations(text, trans);

}

static Widget
CreateSearchDialog(w, ptr)
Widget w;
String ptr;
{
  Widget popup, form;
  Arg args[3];
  Cardinal num_args;

  num_args = 0;
  XtSetArg(args[num_args], XtNiconName, "search"); num_args++;
  XtSetArg(args[num_args], XtNgeometry, NULL); num_args++;
  XtSetArg(args[num_args], XtNallowShellResize, TRUE); num_args++;
  popup = XtCreatePopupShell("search", transientShellWidgetClass, w,
			     args, XtNumber(args) );
  
  num_args = 0;
  form = XtCreateManagedWidget(FORM_NAME, formWidgetClass, popup,
				 args, num_args);

  AddSearchChildren(form, ptr, w);

  return(popup);
}

static void
AddSearchChildren(form, ptr, tw)
Widget form, tw;
char * ptr;
{
  Arg args[10];
  Cardinal num_args;
  Widget label, left, text, cancel, search, replace;
  XtTranslations trans;

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, SEARCH_LABEL);num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNresizable, TRUE ); num_args++;
  XtSetArg(args[num_args], XtNborderWidth, 0 ); num_args++;
  label = XtCreateManagedWidget(LABEL_NAME, labelWidgetClass, form,
				args, num_args);
  
/* 
 * We need to add R_OFFSET to the radio_data, because the value zero (0)
 * has special meaning.
 */

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Left"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, label); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNradioData, (caddr_t) XawsdLeft + R_OFFSET);
  num_args++;
  left = XtCreateManagedWidget(LEFT_NAME, toggleWidgetClass, form,
				args, num_args);


  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Right"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, label); num_args++;
  XtSetArg(args[num_args], XtNfromHoriz, left); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNradioGroup, left); num_args++;
  XtSetArg(args[num_args], XtNradioData, (caddr_t) XawsdRight + R_OFFSET);
  num_args++;
  XtCreateManagedWidget(RIGHT_NAME, toggleWidgetClass, form,
				args, num_args);

  num_args = 0;
  XtSetArg(args[num_args], XtNfromVert, left); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNeditType, XawtextEdit); num_args++;
  XtSetArg(args[num_args], XtNresizable, TRUE); num_args++;
  XtSetArg(args[num_args], XtNtextOptions, resizeWidth); num_args++;
  XtSetArg(args[num_args], XtNstring, ptr); num_args++;
  text = XtCreateManagedWidget(TEXT_NAME, asciiTextWidgetClass, form,
				args, num_args);

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Cancel"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, text); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  cancel = XtCreateManagedWidget("cancel", commandWidgetClass, form,
				 args, num_args);

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Do Search"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, text); num_args++;
  XtSetArg(args[num_args], XtNfromHoriz, cancel); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  search = XtCreateManagedWidget("search", commandWidgetClass, form,
				 args, num_args);

  num_args = 0;
  XtSetArg(args[num_args], XtNlabel, "Do Replace"); num_args++;
  XtSetArg(args[num_args], XtNfromVert, text); num_args++;
  XtSetArg(args[num_args], XtNfromHoriz, search); num_args++;
  XtSetArg(args[num_args], XtNleft, XtChainLeft); num_args++;
  XtSetArg(args[num_args], XtNright, XtChainLeft); num_args++;
  replace = XtCreateManagedWidget("replace", commandWidgetClass, form,
				 args, num_args);

  XtAddCallback(cancel, XtNcallback, CancelSearch, (caddr_t) tw);
  XtAddCallback(search, XtNcallback, DoSearch, (caddr_t) tw);
  XtAddCallback(replace, XtNcallback, DoReplace, (caddr_t) tw);

  XtSetKeyboardFocus(form, text);

/*
 * Bind <CR> to insert file.
 */

  trans = XtParseTranslationTable("<Key>0xFF0D: DoSearchAction()");
  XtOverrideTranslations(text, trans);

}

static void
InitializeSearchWidget(w, dir, replace_active)
Widget w;
XawTextScanDirection dir;
Boolean replace_active;
{
  char name[BUFSIZ];
  
  sprintf(name, "%s.%s", FORM_NAME, REPLACE_NAME);
  (void) SetResourceByName(w, name, XtNsensitive, (XtArgVal) replace_active);

  sprintf(name, "%s.", FORM_NAME);
  switch (dir) {
  case XawsdLeft:
    strcat(name, LEFT_NAME);
    break;
  case XawsdRight:
    strcat(name, RIGHT_NAME);
    break;
  default:
    return;
  }

  (void) SetResourceByName(w, name, XtNstate, (XtArgVal) TRUE);
}  

/* ARGSUSED */
static void 
CancelSearch(w, closure, call_data)
Widget w;			/* The Button Pressed. */
caddr_t closure;		/* Text Widget. */
caddr_t call_data;		/* unused */
{
  TextWidget ctx = (TextWidget) closure;

  XtPopdown( ctx->text.search_widget );
  (void) SetResourceByName(XtParent(w), LABEL_NAME, 
			   XtNlabel, (XtArgVal) SEARCH_LABEL);
}

/* ARGSUSED */
static void 
DoSearch(w, closure, call_data)
Widget w;			/* The Button Pressed. */
caddr_t closure;		/* Text Widget. */
caddr_t call_data;		/* unused */
{
  TextWidget ctx = (TextWidget) closure;
  char msg[BUFSIZ];
  Widget temp_widget;
  XawTextPosition pos;
  XawTextScanDirection dir;
  XawTextBlock text;

  if ( (text.ptr = GetString(XtParent(w))) != NULL ) {

    text.length = strlen(text.ptr);
    text.firstPos = 0;
    text.format = FMT8BIT;
    
    if ( (temp_widget = XtNameToWidget(XtParent(w), LEFT_NAME)) != NULL) {
      dir = (XawTextScanDirection) (XawToggleGetCurrent(temp_widget) -
				    R_OFFSET);

      pos = XawTextSearch( (Widget) ctx, dir, &text);
      
      if (pos == XawTextSearchError) 
	sprintf( msg, "*** Error: Could not find string '%s'. ***", text.ptr);
      else {
	if (dir == XawsdRight)
	  XawTextSetInsertionPoint( (Widget) ctx, pos + text.length);
	else
	  XawTextSetInsertionPoint( (Widget) ctx, pos);
	
	XawTextSetSelection( (Widget) ctx, pos, pos + text.length);
	CancelSearch(w, closure, call_data); /* Popdown the window. */
	return;
      }
    }
    else /* LEFT_NAME */
      sprintf( msg, "*** Error: Could not find toggle widgets. ***");
  }
  else /* TEXT_NAME */
    sprintf( msg, "*** Error: Could not find string to search for. ***");

  if (!SetResourceByName(XtParent(w), LABEL_NAME, XtNlabel, (XtArgVal) msg)) 
    XBell(XtDisplay(w), 0);
}

/* ARGSUSED */
static void 
DoReplace(w, closure, call_data)
Widget w;			/* The Button Pressed. */
caddr_t closure;		/* Text Widget. */
caddr_t call_data;		/* unused */
{
}

static Boolean
SetResourceByName(parent, name, res_name, value)
Widget parent;
char * name, * res_name;
XtArgVal value;
{
  Widget temp_widget;

  if ( (temp_widget = XtNameToWidget(parent, name)) != NULL) {
    Arg args[1];

    XtSetArg(args[0], res_name, value);
    XtSetValues( temp_widget, args, ONE );
    return(TRUE);
  }
  return(FALSE);
}

/*	Function Name: GetString
 *	Description:   Gets the value for the string in the popup.
 *	Arguments:     form - the popup form.
 *	Returns:       the string.
 */

static String 
GetString(form)
Widget form;
{
  Widget temp;
  String string;
  Arg args[1];

  if ( (temp = XtNameToWidget(form, TEXT_NAME)) == NULL) return(NULL);

  XtSetArg( args[0], XtNstring, &string );
  XtGetValues( temp, args, ONE );
  return(string);
}

