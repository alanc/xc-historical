/*
 * $XConsortium: BitEdit.c,v 1.10 90/10/31 18:25:49 dave Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Davor Matic, MIT X Consortium
 */




#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/SmeBSB.h>
#include "Bitmap.h"

#include <X11/bitmaps/xlogo16>

static char *usage = "[-options ...] filename\n\
\n\
where options include:\n\
     -size WIDTHxHEIGHT\n\
     -squares dimension\n\
     -grid, +grid\n\
     -axes, +axes\n\
     -dashed, +dashed\n\
     -stippled, +stippled\n\
     -proportional, +proportional\n\
     -dashes filename\n\
     -stipple filename\n\
     -hl color\n\
     -fr color\n\
     -filename filename\n\
     -basename basename\n\
\n\
The default WIDTHxHEIGHT is 16x16.\n";

static char *info ="\n\
Bitmap Was Written by Davor Matic\n\
       X Consortium AnneX\n\
For bugs and suggestions write to:\n\
      dmatic@expo.lcs.mit.edu";

static XrmOptionDescRec options[] = {
  {
    "-squares",
    "*squareSize",
    XrmoptionSepArg,
    NULL},
  {
    "-grid",
    "*grid",
    XrmoptionNoArg,
    "True"},
  {
    "+grid",
    "*grid",
    XrmoptionNoArg,
    "False"},
  {
    "-axes",
    "*axes",
    XrmoptionNoArg,
    "True"},
  {
    "+axes",
    "*axes",
    XrmoptionNoArg,
    "False"},
  {
    "-dashed",
    "*dashed",
    XrmoptionNoArg,
    "True"},
  {
    "+dashed",
    "*dashed",
    XrmoptionNoArg,
    "False"},
  {
    "-dashes",
    "*dashes",
    XrmoptionSepArg,
    NULL},
  {
    "-stippled",
    "*stippled",
    XrmoptionNoArg,
    "True"},
  {
    "+stippled",
    "*stippled",
    XrmoptionNoArg,
    "False"},
  {
    "-stipple",
    "*stipple",
    XrmoptionSepArg,
    NULL},
  {
    "-proportional",
    "*proportional",
    XrmoptionNoArg,
    "True"},
  {
    "+proportional",
    "*proportional",
    XrmoptionNoArg,
    "False"},
  {
    "-hl", 
    "*highlight", 
    XrmoptionSepArg, 
    NULL},
  {
    "-fr", 
    "*framing", 
    XrmoptionSepArg, 
    NULL},
  {
    "-filename", 
    "*filename", 
    XrmoptionSepArg, 
    NULL},
  {
    "-basename", 
    "*basename", 
    XrmoptionSepArg, 
    NULL},
};

typedef struct {
  int             id;
  String          name;
  Boolean         trap;
  Widget          widget;
  } ButtonRec;

static ButtonRec file_menu[] = {
#define Load 26
  {Load, "load", True},
#define Insert 101
  {Insert, "insert", True},
#define Save 27
  {Save, "save", True},
#define SaveAs 28
  {SaveAs, "saveAs", True},
#define Resize 24
  {Resize, "resize", True},
#define Rescale 79
  {Rescale, "rescale", True},
#define Filename 74
  {Filename, "filename", True},
#define Basename 73
  {Basename, "basename", True},
#define Dummy -1
  {Dummy, "line", False},
#define Quit 75
  {Quit, "quit", True},
};

static ButtonRec edit_menu[] = {
#define Image 77
  {Image, "image", True},
#define Grid 23
  {Grid, "grid", True},
#define Dashed 32
  {Dashed, "dashed", True},
#define Axes 34
  {Axes, "axes", True},
#define Stippled 98
  {Stippled, "stippled", True},
#define Proportional 97
  {Proportional, "proportional", True},
#define Zoom 100
  {Zoom, "zoom", True},
/* Dummy */
  {Dummy, "line", False},
#define Cut 30
  {Cut, "cut", True},
#define Copy 31
  {Copy, "copy", True},
#define Paste 4
  {Paste, "paste", True},
};

static ButtonRec buttons[] = {
/*#define Clear 1*/
  {Clear, "clear", False},
/*#define Set 2*/
  {Set, "set", False},
/*#define Invert 3*/
  {Invert, "invert", False},
#define CopyImm 102
  {CopyImm, "copy", True},
#define MoveImm 103
  {MoveImm, "move", True},
#define FlipHoriz 11
  {FlipHoriz, "flipHoriz", False},
#define Up 7
  {Up, "up", False},
#define FlipVert 12
  {FlipVert, "flipVert", False},
#define Left 9
  {Left, "left", False},
#define Fold 99
  {Fold, "fold", False},
#define Right 10
  {Right, "right", False},
#define RotateLeft 33
  {RotateLeft, "rotateLeft", False},
#define Down 8
  {Down, "down", False},
#define RotateRight 13
  {RotateRight, "rotateRight", False},
#define Point 14
  {Point, "point", True},
#define Curve 41
  {Curve, "curve", True},
#define Line 15
  {Line, "line", True},
#define Rectangle 16
  {Rectangle, "rectangle", True},
#define FilledRectangle 17
  {FilledRectangle, "filledRectangle", True},
#define Circle 18
  {Circle, "circle", True},
#define FilledCircle 19
  {FilledCircle, "filledCircle", True},
#define FloodFill 20
  {FloodFill, "floodFill", True},
#define SetHotSpot 21
  {SetHotSpot, "setHotSpot", True},
#define ClearHotSpot 22
  {ClearHotSpot, "clearHotSpot", False},
#define Undo 25
  {Undo, "undo", False},
};

#include "Dialog.h"

Widget 
    top_widget, 
    parent_widget,
    formy_widget,
    infoButton_widget,
    fileButton_widget, fileMenu_widget,
    editButton_widget, editMenu_widget,
    status_widget,
    pane_widget, 
    form_widget,
    bitmap_widget,
    image_shell,
    image_widget;
Boolean image_visible = False;
Pixmap check_mark;
Dialog info_dialog, input_dialog, error_dialog, qsave_dialog;
Time btime;
String filename = "", basename = "", format = "";
char message[80];

void ShowImage();
void FixMenu();
void DoQuit();

static XtActionsRec actions_table[] = {
{"image", ShowImage},
{"fix-menu", FixMenu},
{"quit", DoQuit}
};

static Atom wm_delete_window;

void FixImage()
{
    Pixmap image;
    int n;
    Arg wargs[2];

    if (!image_visible) return;
    
    n=0;
    XtSetArg(wargs[n], XtNbitmap, &image); n++;
    XtGetValues(image_widget, wargs, n);
    
    if (image != XtUnspecifiedPixmap)
	XFreePixmap(XtDisplay(bitmap_widget), image);
    
    image = BWGetUnzoomedPixmap(bitmap_widget);
    
    n=0;
    XtSetArg(wargs[n], XtNbitmap, image); n++;
    XtSetValues(image_widget, wargs, n);
}

void FixEntry(w, id)
    Widget w;
    int *id;
{
    int n;
    Arg wargs[2];


    n = 0;
    
    switch (*id) {
	
    case Image:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 image_visible ? check_mark : None); n++;
	break;
	
    case Grid:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 BWQueryGrid(bitmap_widget) ? check_mark : None); n++;
	break;
	
    case Dashed:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 BWQueryDashed(bitmap_widget) ? check_mark : None); n++;
	break;
	
    case Axes:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 BWQueryAxes(bitmap_widget) ? check_mark : None); n++;
	break;
	
    case Stippled:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 BWQueryStippled(bitmap_widget) ? check_mark : None); n++;
	break;
	
    case Proportional:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 BWQueryProportional(bitmap_widget) ? check_mark : None); n++;
	break;
	
    case Zoom:
	XtSetArg(wargs[n], XtNleftBitmap, 
		 BWQueryZooming(bitmap_widget) ? check_mark : None); n++;
	break;

    case Copy:
    case Cut:
	XtSetArg(wargs[n], XtNsensitive, BWQueryMarked(bitmap_widget)); n++;
	break;

    case Paste:
	XtSetArg(wargs[n], XtNsensitive, BWQueryStored(bitmap_widget)); n++;
	break;

    default:
	return;
    }
    
    XtSetValues(w, wargs, n);
}

/* ARGSUSED */
void FixMenu(w, event)
    Widget w;
    XEvent *event;
{
    int i;

    btime = event->xbutton.time;
    
    for (i = 0; i < XtNumber(edit_menu); i++)
	FixEntry(edit_menu[i].widget, &edit_menu[i].id);
}

FixStatus()
{
    int n;
    Arg wargs[2];
    String str;

    str = BWUnparseStatus(bitmap_widget);
    
    n = 0;
    XtSetArg(wargs[n], XtNlabel, str); n++;
    XtSetValues(status_widget, wargs, n);
}

void DoQuit()
{
    if (BWQueryChanged(bitmap_widget)) {
	BWGetFilename(bitmap_widget, &filename);
    RetryQuit:
	switch (PopupDialog(qsave_dialog, "Save file before quitting?",
				filename, &filename, XtGrabExclusive)) {
	case Yes:
	    if (BWWriteFile(bitmap_widget, filename, NULL) 
		!= BitmapSuccess) {
		sprintf(message, "Can't write file: %s", filename);
		if (PopupDialog(error_dialog, message, 
				NULL, NULL, XtGrabExclusive) == Retry) 
		    goto RetryQuit;
	    }
	    
	case Cancel:
	    return;
	}
    }
    exit(0);
}

static int zero = 0;
#define Plain  &zero,sizeof(int)
/* ARGSUSED */
void TheCallback(w, id)
     Widget w;
     int   *id;
{
    char x;
    int width, height;
    
    switch (*id) {
	
    case Load:
	if (BWQueryChanged(bitmap_widget)) {
	    BWGetFilename(bitmap_widget, &filename);
	RetryLoadSave:
	    switch (PopupDialog(qsave_dialog, "Save file before loading?",
				filename, &filename, XtGrabExclusive)) {
	    case Yes:
		if (BWWriteFile(bitmap_widget, filename, NULL) 
		    != BitmapSuccess) {
		    sprintf(message, "Can't write file: %s", filename);
		    if (PopupDialog(error_dialog, message, 
				    NULL, NULL, XtGrabExclusive) == Retry) 
			goto RetryLoadSave;
		}

	    case Cancel:
		return;
	    }
	}
	BWGetFilepath(bitmap_widget, &filename);
    RetryLoad:
	if (PopupDialog(input_dialog, "Load file:", 
			filename, &filename, XtGrabExclusive) == Okay) {
	    if (BWReadFile(bitmap_widget, filename, NULL) != BitmapSuccess) {
		sprintf(message, "Can't read file: %s", filename);
		if (PopupDialog(error_dialog, message, 
				NULL, NULL, XtGrabExclusive) == Retry)
		    goto RetryLoad;
	    }
	    else {
		BWChangeNotify(bitmap_widget, NULL, NULL);
		BWClearChanged(bitmap_widget);
		FixStatus();
	    }
	}
	break;
	
    case Insert:
	BWGetFilepath(bitmap_widget, &filename);
    RetryInsert:
	if (PopupDialog(input_dialog, "Insert file:", 
			filename, &filename, XtGrabExclusive) == Okay) {
	    if (BWStoreFile(bitmap_widget, filename, NULL) != BitmapSuccess) {
		sprintf(message, "Can't read file: %s", filename);
		if (PopupDialog(error_dialog, message, 
				NULL, NULL, XtGrabExclusive) == Retry)
		    goto RetryInsert;
	    }
	    else {
		BWEngageRequest(bitmap_widget, RestoreRequest, False, Plain);
	    }
	}
	break;
	
    case Save:
	if (BWWriteFile(bitmap_widget, NULL, NULL) != BitmapSuccess) {
	    sprintf(message, "Can't write file: %s", filename);
	    if (PopupDialog(error_dialog, message, 
			    NULL, NULL, XtGrabExclusive) == Retry) {
		BWGetFilename(bitmap_widget, &filename);
		goto RetrySave;
	    }
	}
	else {
	    BWClearChanged(bitmap_widget);
	}
	break;
	
    case SaveAs:
	BWGetFilename(bitmap_widget, &filename);
    RetrySave:
	if (PopupDialog(input_dialog, "Save file:", 
			filename, &filename, XtGrabExclusive) == Okay) {
	    if (BWWriteFile(bitmap_widget, filename, NULL) != BitmapSuccess) {
		sprintf(message, "Can't write file: %s", filename);
		if (PopupDialog(error_dialog, message, 
				NULL, NULL, XtGrabExclusive) == Retry)
		    goto RetrySave;
	    }
	    else {
		BWClearChanged(bitmap_widget);
		FixStatus();
	    }
	}
	break;

    case Filename:
	BWGetFilename(bitmap_widget, &filename);
	if (PopupDialog(input_dialog, "Change filename:", 
			filename, &filename, XtGrabExclusive) == Okay) {
	    BWChangeFilename(bitmap_widget, filename);
	    FixStatus();
	}
	break;

    case Basename:
	BWGetBasename(bitmap_widget, &basename);
	if (PopupDialog(input_dialog, "Change basename:", 
			basename, &basename, XtGrabExclusive) == Okay) {
	    BWChangeBasename(bitmap_widget, basename);
	    FixStatus();
	}
	break;
	
    case Quit:
	DoQuit();
	break;
	
    case Image:
	if (image_visible) {
	    XtPopdown(image_shell);
	    image_visible = False;
	}
	else {
	    Position image_x, image_y;
	    int n;
	    Arg wargs[3];
	    
	    XtTranslateCoords(bitmap_widget, 
			      10, 10, &image_x, &image_y);

	    n = 0;
	    XtSetArg(wargs[n], XtNx, image_x); n++;
	    XtSetArg(wargs[n], XtNy, image_y); n++;
	    XtSetValues(image_shell, wargs, n);

	    image_visible = True;
	    
	    FixImage();
	    XtPopup(image_shell, XtGrabNone);
	    FixImage();
	}
    break;

    case Grid:
	BWSwitchGrid(bitmap_widget);
	break;
	
    case Dashed:
	BWSwitchDashed(bitmap_widget);
	break;

    case Axes:
	BWSwitchAxes(bitmap_widget);
	break;	

    case Stippled:
	BWSwitchStippled(bitmap_widget);
	break;
	
    case Proportional:
	BWSwitchProportional(bitmap_widget);
	break;
		
    case Zoom:
	if (BWQueryZooming(bitmap_widget)) {
	    BWZoomOut(bitmap_widget);
	    BWChangeNotify(bitmap_widget, NULL, NULL);
	}
	else {
	    if (BWQueryMarked(bitmap_widget)) {
		BWStoreToBuffer(bitmap_widget);
		BWZoomMarked(bitmap_widget);
		BWChangeNotify(bitmap_widget, NULL, NULL);
	    }
	    else {
 		BWEngageRequest(bitmap_widget, ZoomInRequest, False, Plain);
	    }
	}
	break;
	
    case Resize:
	format = "";
    RetryResize:
	if (PopupDialog(input_dialog, "Resize to WIDTHxHEIGHT:", 
			format, &format, XtGrabExclusive) == Okay) {
	    sscanf(format, "%d%c%d", &width, &x, &height);
	    if ((width >0) && (height > 0) && (x == 'x')) {
		BWResize(bitmap_widget, (Dimension)width, (Dimension)height);
		BWChangeNotify(bitmap_widget, NULL, NULL);
		BWSetChanged(bitmap_widget);
		FixStatus();
	    }
	    else {
		sprintf(message, "Wrong format: %s", format);
		if (PopupDialog(error_dialog, message, 
				NULL, NULL, XtGrabExclusive) == Retry)
		    goto RetryResize;
	    }
	}
	break;

    case Rescale:
	format = "";
    RetryRescale:
	if (PopupDialog(input_dialog, "Rescale to WIDTHxHEIGHT:", 
			format,	&format, XtGrabExclusive) == Okay) {
	    sscanf(format, "%d%c%d", &width, &x, &height);
	    if ((width >0) && (height > 0) && (x == 'x')) {
		BWRescale(bitmap_widget, (Dimension)width, (Dimension)height);
		BWChangeNotify(bitmap_widget, NULL, NULL);
		BWSetChanged(bitmap_widget);
		FixStatus();
	    }
	    else {
		sprintf(message, "Wrong format: %s", format);
		if (PopupDialog(error_dialog, message, 
				NULL, NULL, XtGrabExclusive) == Retry)
		    goto RetryRescale;
	    }
	}
	break;

    case Copy:
	BWStore(bitmap_widget);
	BWUnmark(bitmap_widget);
	break;

    case Cut:
	BWStore(bitmap_widget);
	BWClearMarked(bitmap_widget);
	BWUnmark(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	break;

    case Paste:
	/*BWRequestSelection(bitmap_widget, btime, TRUE);*/
	BWEngageRequest(bitmap_widget, RestoreRequest, False, Plain);
	break;
	
    case Clear:
	BWStoreToBuffer(bitmap_widget);
	BWClear(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Set:
	BWStoreToBuffer(bitmap_widget);
	BWSet(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Invert:
	BWStoreToBuffer(bitmap_widget);
	BWInvert(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;

    case CopyImm:
	BWRemoveAllRequests(bitmap_widget);
	if (BWQueryMarked(bitmap_widget)) {
	    BWAddRequest(bitmap_widget, MarkRequest, False, Plain);
	    BWEngageRequest(bitmap_widget, CopyRequest, True, Plain);
	}
	else {
	    BWEngageRequest(bitmap_widget, MarkRequest, False, Plain);
	    BWAddRequest(bitmap_widget, CopyRequest, True, Plain);
	}
	break;
	
    case MoveImm:
	BWRemoveAllRequests(bitmap_widget);
	if (BWQueryMarked(bitmap_widget)) {
	    BWAddRequest(bitmap_widget, MarkRequest, False, Plain);
	    BWEngageRequest(bitmap_widget, MoveRequest, True, Plain);
	}
	else {
	    BWEngageRequest(bitmap_widget, MarkRequest, False, Plain);
	    BWAddRequest(bitmap_widget, MoveRequest, True, Plain);
	}
	break;
		
    case Up:
	BWStoreToBuffer(bitmap_widget);
	BWUp(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Down:
	BWStoreToBuffer(bitmap_widget);
	BWDown(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Left:
	BWStoreToBuffer(bitmap_widget);
	BWLeft(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Right:
	BWStoreToBuffer(bitmap_widget);
	BWRight(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Fold:
	BWStoreToBuffer(bitmap_widget);
	BWFold(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case FlipHoriz:
	BWStoreToBuffer(bitmap_widget);
	BWFlipHoriz(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case FlipVert:
	BWStoreToBuffer(bitmap_widget);
	BWFlipVert(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case RotateRight:
	BWStoreToBuffer(bitmap_widget);
	BWRotateRight(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case RotateLeft:
	BWStoreToBuffer(bitmap_widget);
	BWRotateLeft(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;
	
    case Point:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, PointRequest, True, Plain);
	break;

    case Curve:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, CurveRequest, True, Plain);
	break;
	
    case Line:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, LineRequest, True, Plain);
	break;
	
    case Rectangle:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, RectangleRequest, True, Plain);
	break;
	
    case FilledRectangle:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, FilledRectangleRequest, True, Plain);
	break;
	
    case Circle:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, CircleRequest, True, Plain);
	break;
	
    case FilledCircle:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, FilledCircleRequest, True, Plain);
	break;
	
    case FloodFill:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, FloodFillRequest, True, Plain);
	break;
	
    case SetHotSpot:
	BWRemoveAllRequests(bitmap_widget);
	BWEngageRequest(bitmap_widget, HotSpotRequest, True, Plain);
	break;
	
    case ClearHotSpot:
	BWStoreToBuffer(bitmap_widget);
	BWClearHotSpot(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;

    case Undo:
	BWUndo(bitmap_widget);
	BWChangeNotify(bitmap_widget, NULL, NULL);
	BWSetChanged(bitmap_widget);
	break;	
    }
}

/* ARGSUSED */
void BAPopdownImage(w, call_data, event)
    Widget  w;
    caddr_t call_data;
    XEvent *event;
{
    static int image = Image;
    
    if ((event->type == ButtonRelease) && image_visible)
	TheCallback(w, &image);
}


void ShowImage(w)
    Widget w;
{
    static int id = Image;

    TheCallback(w, &id);
}



void InfoCallback()
{
    PopupDialog(info_dialog, info,
		NULL, NULL, XtGrabExclusive);
}
   
void main(argc, argv)
    int    argc;
    char  *argv[];
{
    int i, n;
    Arg wargs[2];
    Widget w;
    Widget radio_group; caddr_t radio_data;
    
    top_widget = XtInitialize(NULL, "Bitmap", 
			      options, XtNumber(options), 
			      (Cardinal *)&argc, argv);

    if (argc > 1) {
	fprintf(stderr, usage);
	exit (0);
    }

    check_mark = XCreateBitmapFromData(XtDisplay(top_widget),
				      RootWindowOfScreen(XtScreen(top_widget)),
				      xlogo16_bits, 
				      xlogo16_width, 
				      xlogo16_height);

    XtAddActions(actions_table, XtNumber(actions_table));
    XtOverrideTranslations
	(top_widget,XtParseTranslationTable ("<Message>WM_PROTOCOLS: quit()"));
    parent_widget = XtCreateManagedWidget("parent", panedWidgetClass,
					 top_widget, NULL, 0);

    formy_widget = XtCreateManagedWidget("formy", formWidgetClass,
				       parent_widget, NULL, 0);

    infoButton_widget = XtCreateManagedWidget("infoButton",
					commandWidgetClass, 
					formy_widget, NULL, 0);

    XtAddCallback(infoButton_widget, XtNcallback, InfoCallback, NULL);
    
    fileMenu_widget = XtCreatePopupShell("fileMenu", 
					 simpleMenuWidgetClass, 
					 formy_widget, NULL, 0);
    
    fileButton_widget = XtCreateManagedWidget("fileButton",
					      menuButtonWidgetClass, 
					      formy_widget, NULL, 0);

    for (i = 0; i < XtNumber(file_menu); i++) {
	w = XtCreateManagedWidget(file_menu[i].name, 
				  (file_menu[i].trap ? 
				   smeBSBObjectClass : smeLineObjectClass),
				  fileMenu_widget, NULL, 0),
	XtAddCallback(w,
		      XtNcallback,
		      TheCallback,
		      &file_menu[i].id);
	
	file_menu[i].widget = w;
    }
        
    editMenu_widget = XtCreatePopupShell("editMenu", 
					 simpleMenuWidgetClass, 
					 formy_widget, NULL, 0);
    
    editButton_widget = XtCreateManagedWidget("editButton", 
					      menuButtonWidgetClass, 
					      formy_widget, NULL, 0);

    for (i = 0; i < XtNumber(edit_menu); i++) {
	w = XtCreateManagedWidget(edit_menu[i].name, 
				  (edit_menu[i].trap ? 
				   smeBSBObjectClass : smeLineObjectClass),
				  editMenu_widget, NULL, 0),
	XtAddCallback(w,
		      XtNcallback,
		      TheCallback,
		      &edit_menu[i].id);
	
	edit_menu[i].widget = w;
    }

    status_widget = XtCreateManagedWidget("status", labelWidgetClass,
					  formy_widget, NULL, 0);

    pane_widget = XtCreateManagedWidget("pane", panedWidgetClass,
					parent_widget, NULL, 0);

    form_widget = XtCreateManagedWidget("form", formWidgetClass, 
					pane_widget, NULL, 0);
        
    for (i = 0; i < XtNumber(buttons); i++) {
	w = XtCreateManagedWidget(buttons[i].name, 
				  (buttons[i].trap ? 
				   toggleWidgetClass : commandWidgetClass),
				  form_widget, NULL, 0);

	XtAddCallback(w,
		      XtNcallback,
		      TheCallback,
		      &buttons[i].id);

	buttons[i].widget = w;

	if (buttons[i].id == Point) {
	    radio_group = buttons[i].widget;
	    radio_data  = buttons[i].name;
	}
    }
    
    bitmap_widget = XtCreateManagedWidget("bitmap", bitmapWidgetClass,
					  pane_widget, NULL, 0);

    XtRealizeWidget(top_widget);

    wm_delete_window = XInternAtom(XtDisplay(top_widget), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(top_widget), XtWindow(top_widget),
                            &wm_delete_window, 1);

    
    image_shell = XtCreatePopupShell("image", transientShellWidgetClass,
				     top_widget, NULL, 0);

    image_widget = XtCreateManagedWidget("label", labelWidgetClass,
					 image_shell, NULL, 0);
    
    n=0;
    XtSetArg(wargs[n], XtNbitmap, BWGetUnzoomedPixmap(bitmap_widget)); n++;
    XtSetValues(image_widget, wargs, n);
    
    XtRealizeWidget(image_shell);

    Notify(bitmap_widget, FixImage);

    FixStatus();

    info_dialog = CreateDialog(top_widget, "info", Okay);
    input_dialog = CreateDialog(top_widget, "input", Okay | Cancel);
    error_dialog = CreateDialog(top_widget, "error", Abort | Retry);    
    qsave_dialog = CreateDialog(top_widget, "qsave", Yes | No | Cancel);

    XawToggleSetCurrent(radio_group, radio_data);
    BWEngageRequest(bitmap_widget, PointRequest, True, Plain);

    XtMainLoop();
}
