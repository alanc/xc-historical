/*
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
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * xmag.c -- toolkit version.
 * 11-27-90 dms creation.
 */



#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Shell.h>
#include "RootWindow.h"
#include "Scale.h"
#include <X11/cursorfont.h>



#define min(a, b) a < b ? a : b

#define HLINTERVAL  10
typedef enum { drag, resize, done } hlMode;
typedef struct { 
  Boolean   newScale;
  hlMode    selectMode;
  GC        gc;
  Position  homeX, homeY, x, y;
  Dimension width, height;
  Widget    scaleShell, scaleInstance, pixShell, pixLabel;
  } hlStruct, *hlPtr;
static XtIntervalId hlId;



static XtAppContext app;
static Cursor ulAngle, urAngle, lrAngle, llAngle;
static Display *dpy;
static int scr;
static GC selectGC; 
static XGCValues selectGCV;
static Widget toplevel, root;
static Atom wm_delete_window;
static void CloseAP(), NewAP(), ReplaceAP(),PopupPixelAP(), UpdatePixelAP(), 
  PopdownPixelAP(), SelectRegionAP(), CheckPoints(), HighlightTO(),
  CloseCB(), ReplaceCB(), NewCB(), SetupGC(), ResizeEH(), DragEH(), 
  StartRootPtrGrab(), CreateRoot(), InitCursors(), GetImage(),
  PopupNewScale(), ParseSourceGeom();
static int numXmags = 0;
static int srcStat, srcX, srcY;
static unsigned int srcWidth, srcHeight;



typedef struct 
{ String source, mag; } OptionsRec;

static OptionsRec options;

#define Offset(field) XtOffsetOf(OptionsRec, field)

static XtResource resources[] = {
  {"source", "Source", XtRString, sizeof(String),
     Offset(source), XtRString, (XtPointer)"64x64"},
  {"mag", "Mag", XtRString, sizeof(String),
     Offset(mag), XtRString, (XtPointer)"5.0"},
};

#undef Offset

static XrmOptionDescRec optionDesc[] = {
  {"-source", "*source", XrmoptionSepArg, (XtPointer)NULL},
  {"-mag", "*mag", XrmoptionSepArg, (XtPointer)NULL},
};



static XtActionsRec actions_table[] = {
  {"close", CloseAP},
  {"new", NewAP},
  {"replace", ReplaceAP},
  {"popup-pixel", PopupPixelAP},
  {"update-pixel", UpdatePixelAP},
  {"popdown-pixel", PopdownPixelAP},
  {"select-region", SelectRegionAP}
};



/*
 * CloseAP() -- Close this dialog.  If its the last one exit the program.
 *          
 */
static void			/* ARGSUSED */
CloseAP(w, event)
    Widget w; XEvent *event;
{
  Arg wargs[2]; int n; hlPtr data;
  if (!--numXmags) exit(0);
  if (event->type != ClientMessage) {
    n = 0;			/* get user data */
    XtSetArg(wargs[0], XtNuserData, &data); n++;
    XtGetValues(w, wargs, n); 
    w = data->scaleShell;
  }
  XtPopdown(w);
  XtDestroyWidget(w);
};



/*
 * NewAP() -- Create an additional xmag dialog. THIS IS A COPY OF NewEH
 *                                              FIND A BETTER WAY....
 */
static void			/* ARGSUSED */
NewAP(w, event)
     Widget w; XEvent event;
{
  StartRootPtrGrab(True, NULL);
}

/*
 * ReplaceCB() -- Replace this particular xmag dialog.
 */
static void                     /* ARGSUSED */
ReplaceAP(w, event)
     Widget w; XEvent *event;
{
  Arg wargs[2]; int n; hlPtr data;
  n = 0;			/* get user data */
  XtSetArg(wargs[0], XtNuserData, &data); n++;
  XtGetValues(w, wargs, n); 
  StartRootPtrGrab(False, data);
}



/*
 * PopupPixelAP() -- Show pixel information.
 */
static void 
PopupPixelAP(w, event)
    Widget w;
    XEvent *event;
{
    Position scale_x, scale_y;
    Dimension scale_height;
    Position label_x, label_y;
    Dimension label_height;
    int n;
    Arg wargs[3];
    hlPtr data;

    n = 0;			/* get user data */
    XtSetArg(wargs[0], XtNuserData, &data); n++;
    XtGetValues(w, wargs, n);

    n = 0;
    XtSetArg(wargs[n], XtNheight, &scale_height); n++;
    XtGetValues(w, wargs, n);
    XtTranslateCoords(w, -1, -1, &scale_x, &scale_y);
    
    XtRealizeWidget(data->pixShell); /* to get the right hight  */

    n = 0;
    XtSetArg(wargs[n], XtNheight, &label_height); n++;
    XtGetValues(data->pixShell, wargs, n);
    
    if ((double) event->xbutton.y / (double) scale_height > 0.5) {
	label_x = scale_x;
	label_y = scale_y;
    }
    else {
	label_x = scale_x;
	label_y = scale_y + scale_height - label_height;
    }

    n = 0;
    XtSetArg(wargs[n], XtNx, label_x); n++;
    XtSetArg(wargs[n], XtNy, label_y); n++;
    XtSetValues(data->pixShell, wargs, n);
    
    UpdatePixelAP(w, event);
}



/*
 * UpdatePixelAP() -- Update pixel information.
 */
static void 
UpdatePixelAP(w, event)
    Widget w;
    XEvent *event;
{
    Position x, y;
    Pixel pixel;
    XColor color;
    int n;
    Arg wargs[3];
    char string[80];
    hlPtr data;

    n = 0;
    XtSetArg(wargs[0], XtNuserData, &data); n++;
    XtGetValues(w, wargs, n);

    if (SWGetImagePixel(w, event->xbutton.x, event->xbutton.y, &x, &y, &pixel))
	XtPopdown(data->pixShell);
    else {
	color.pixel = pixel;
	XQueryColor(XtDisplay(w), DefaultColormap(XtDisplay(w), 
						  DefaultScreen(XtDisplay(w))),
		    &color);
	sprintf(string, "Pixel %d at (%d,%d) colored (%x,%x,%x).", 
		pixel, x + data->x, y + data->y,
		color.red, color.green, color.blue);
	
	n = 0;
	XtSetArg(wargs[n], XtNlabel, string); n++;    
	XtSetValues(data->pixLabel, wargs, n);
	XtPopup(data->pixShell, XtGrabNone);
    }
}



/*
 * PopdownPixel() -- Remove pixel info.
 */
static void 
PopdownPixelAP(w)
    Widget w;
{
  int n;
  Arg wargs[3];
  hlPtr data;
  
  n = 0;
  XtSetArg(wargs[0], XtNuserData, &data); n++;
  XtGetValues(w, wargs, n);
  XtPopdown(data->pixShell);
}



static void			/* ARGSUSED */
SelectRegionAP(w)
    Widget w;
{    
/***** NOT SURE WHAT TO DO WITH THIS 
    if (app_resources.unmap)
	XtUnmapWidget(toplevel);
    Redisplay(XtDisplay(w), RootWindow(XtDisplay(w),
				       DefaultScreen(XtDisplay(w))),
	      source.width, source.height, 
	      app_resources.freq, app_resources.puls, 
	      ul_angle, lr_angle,
	      app_resources.grab);

    if (app_resources.unmap)
	XtMapWidget(toplevel);
******/
}



/* 
 * CheckPoints() -- Change the cursor for the correct quadrant.
 *                  Make sure the first point is less than the second 
 *                  for drawing the selection rectangle.
 */
static void 
CheckPoints(x1, x2, y1, y2)
     Position *x1, *x2, *y1, *y2;
{
  Position tmp; 
  Boolean above, left;
  Cursor newC;
  above = (*y2 < *y1); left = (*x2 < *x1); 
  if (above&&left) newC = ulAngle;
  else if (above&&!left) newC = urAngle;
  else if (!above&&!left) newC = lrAngle;
  else newC = llAngle;
  XChangeActivePointerGrab
    (dpy, PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
     newC, CurrentTime);
  if (*x2 < *x1) { tmp = *x1; *x1 = *x2; *x2 = tmp; }
  if (*y2 < *y1) { tmp = *y1; *y1 = *y2; *y2 = tmp; }
}



/*
 * HighlightTO() -- Timer to highlight the selection box
 */
static void 
HighlightTO(data, id)	/* ARGSUSED */
     hlPtr data; XtIntervalId *id;
{
  XGrabServer(dpy);
  if (data->selectMode == drag) {
    XDrawRectangle(dpy, DefaultRootWindow(dpy), data->gc, 
		   data->x, data->y, data->width, data->height);
    XDrawRectangle(dpy, DefaultRootWindow(dpy), data->gc, 
		   data->x, data->y, data->width, data->height);
  }
  else if (data->selectMode == resize) {	
    Position x1 = data->homeX,
             x2 = data->x,
             y1 = data->homeY,
             y2 = data->y;
    CheckPoints(&x1, &x2, &y1, &y2);
    XDrawRectangle(dpy, DefaultRootWindow(dpy), data->gc, 
		   x1, y1, x2 - x1, y2 - y1);
    XDrawRectangle(dpy, DefaultRootWindow(dpy), data->gc, 
		   x1, y1, x2 - x1, y2 - y1);
  }
  XUngrabServer(dpy);
  if (data->selectMode != done)
    XtAppAddTimeOut(app, HLINTERVAL, HighlightTO, (XtPointer)data);
}



/*
 * CloseCB() -- Delete this xmag dialog.  If its the only one on the screen
 *             then exit.
 */
static void			/* ARGSUSED */
CloseCB(w, shell, callData)
     Widget w; Widget shell; XtPointer callData;
{
  
  if (!--numXmags) exit(0);
  XtPopdown(shell);
  XtDestroyWidget(shell);
}



/*
 * ReplaceCB() -- Replace this particular xmag dialog.
 */
static void                     /* ARGSUSED */
ReplaceCB(w, data, callData)
     Widget w; hlPtr data; XtPointer callData;
{
  StartRootPtrGrab(False, data);
}

/*
 * NewCB() -- Create an additional xmag dialog.
 */
static void			/* ARGSUSED */
NewCB(w, clientData, callData)
     Widget w; XtPointer clientData, callData;
{
  StartRootPtrGrab(True, NULL);
}



/*
 * SetupGC() -- Graphics context for magnification selection.
 */
static void 
SetupGC()
{
    selectGCV.function = GXxor;
    selectGCV.foreground = 1L;
    selectGCV.subwindow_mode = IncludeInferiors;
    selectGC = XtGetGC(toplevel, GCFunction|GCForeground|GCSubwindowMode,
		       &selectGCV);
}  



/*
 * ResizeEH() -- Event Handler for resize of selection box.
 */
static void 
ResizeEH(w, data, event)	/* ARGSUSED */
     Widget w; hlPtr data; XEvent *event;
{
  switch (event->type) {
  case MotionNotify:
    data->x = event->xmotion.x_root;
    data->y = event->xmotion.y_root; 
    break;
  case ButtonRelease:
    if (data->newScale)
      PopupNewScale(data);
    GetImage(min(data->homeX,event->xbutton.x_root),
	     min(data->homeY,event->xbutton.y_root),
	     abs(data->homeX - event->xbutton.x_root),
	     abs(data->homeY - event->xbutton.y_root),
	     data);
    XtUngrabPointer(w, CurrentTime);
    XtRemoveEventHandler(w, PointerMotionMask|ButtonReleaseMask,
			 True, ResizeEH, (XtPointer)data);
    data->selectMode = done;
    XUndefineCursor(dpy, XtWindow(w));  
    break;
  }
}



/*
 * DragEH() -- Event Handler for draging selection box.
 */
static void 
DragEH(w, data, event) /* ARGSUSED */
     Widget w; hlPtr data; XEvent *event;
{
  switch (event->type) {
  case MotionNotify:		/* drag mode */
    data->x = event->xmotion.x_root;
    data->y = event->xmotion.y_root;
    break;
  case ButtonRelease:		/* end drag mode */
    if (event->xbutton.button == Button1) { /* get image */
      if (data->newScale)
	PopupNewScale(data);
      GetImage(event->xbutton.x_root, event->xbutton.y_root,
		   srcWidth, srcHeight, data);
      XtUngrabPointer(w, CurrentTime);
      XtRemoveRawEventHandler(w, PointerMotionMask|ButtonPressMask|
			      ButtonReleaseMask, True, DragEH,
			      (XtPointer)data);
      data->selectMode = done;
      XUndefineCursor(dpy, XtWindow(w));
    }

    break;
  case ButtonPress:	
    if (event->xbutton.button == Button2) {	/* turn on resize mode */
      data->homeX = event->xbutton.x_root; 
      data->homeY = event->xbutton.y_root;
      data->x = event->xbutton.x_root + srcWidth;
      data->y = event->xbutton.y_root + srcHeight;      
      data->selectMode = resize;
      XtRemoveEventHandler(w, PointerMotionMask|ButtonPressMask|
			   ButtonReleaseMask, True, DragEH, (XtPointer)data);
      XChangeActivePointerGrab
	(dpy, PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
	 lrAngle, CurrentTime);
      XWarpPointer(dpy, None, None, 0, 0, 0, 0, 
		   srcWidth, srcHeight);
      XtAddEventHandler(w, PointerMotionMask|ButtonReleaseMask, 
			True, ResizeEH, (XtPointer)data);
    }
    break;
  }
}




/*
 * StartRootPtrGrab() -- Bring up the selection box.
 *              
 */
static void
StartRootPtrGrab(new, data)
     Boolean new;		/* do we cretate a new scale instance? */
     hlPtr data;
{
  Window    rootR, childR;
  int       rootX, rootY, winX, winY;
  unsigned  int mask;
  hlPtr hlData;
  XtGrabPointer
    (root, False,
     PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
     GrabModeAsync, GrabModeAsync, None, ulAngle, CurrentTime);
  XQueryPointer(dpy, DefaultRootWindow(dpy), &rootR, &childR, 
		&rootX, &rootY, &winX, &winY, &mask);
  if (new) {
    numXmags++;
    hlData = (hlPtr)XtMalloc(sizeof(hlStruct));
  }
  else hlData = data;
  hlData->newScale   = new;
  hlData->selectMode = drag;
  hlData->x          = rootX;
  hlData->y          = rootY;
  hlData->gc         = selectGC;
  hlData->width      = srcWidth;
  hlData->height     = srcHeight;
  XtAddRawEventHandler
    (root, PointerMotionMask|ButtonPressMask|ButtonReleaseMask, 
     True, DragEH, (XtPointer)hlData);
  hlId = XtAppAddTimeOut(app, HLINTERVAL, HighlightTO, (XtPointer)hlData);
}



/*
 * CreateRoot() -- Make it. If the user specified x and y in his source
 *                 geometry then use this to directly get the image.
 */
static void
CreateRoot()
{
  hlPtr data;
  root = XtCreateWidget("root", rootWindowWidgetClass, toplevel, NULL, 0);
  XtRealizeWidget(root);
  if (XValue & srcStat && YValue &srcStat) { 
    numXmags = 1;
    data = (hlPtr)XtMalloc(sizeof(hlStruct));
    data = data;
    data->newScale   = True;
    data->selectMode = drag;
    data->x          = srcX;
    data->y          = srcY;
    data->gc         = selectGC;
    data->width      = srcWidth;
    data->height     = srcHeight;
    PopupNewScale(data);
    GetImage(srcX, srcY, srcWidth, srcHeight, data);
    return;
  }
}


/* 
 * GetImage() -- Get the image bits from the screen.
 */
static void 
GetImage(x, y, width, height, data)
     int x, y, width, height; hlPtr data;
{
  XImage *image;

  /* avoid off screen pixels */
  if (x < 0) x = 0; if (y < 0) y = 0;
  if (x + width > DisplayWidth(dpy,scr)) x = DisplayWidth(dpy,scr) - width;
  if (y + height > DisplayHeight(dpy,scr)) y = DisplayHeight(dpy,scr) - height;
  data->x = x; data->y = y;
  image = XGetImage (dpy,
		     RootWindow(dpy, scr),
		     x, y,
		     width, height,
		     AllPlanes, ZPixmap);
  SWSetImage(data->scaleInstance, image);
  if (data->newScale) {
    XtPopup(data->scaleShell, XtGrabNone);
    (void) XSetWMProtocols	/* ICCCM delete window */
      (dpy, XtWindow(data->scaleShell), &wm_delete_window, 1);
  }
  XDestroyImage(image);
}



/*
 * PopupNewScale() -- Create and popup a new scale composite.
 */
static void		
PopupNewScale(data)
     hlPtr data;
{
  Widget form, close, replace, new;
  Arg warg;
  data->scaleShell = XtCreatePopupShell("xmag",
				  topLevelShellWidgetClass,
				  toplevel, NULL, 0);
  form = XtCreateManagedWidget("form", formWidgetClass, data->scaleShell,
                               (Arg *) NULL, 0);
  close = XtCreateManagedWidget("close", commandWidgetClass, form,
                               (Arg *) NULL, 0);
  XtAddCallback(close, XtNcallback, CloseCB, (XtPointer)data->scaleShell);
  replace = XtVaCreateManagedWidget("replace", commandWidgetClass, form,
				    XtNfromHoriz, (XtArgVal)close,
				    NULL);
  XtAddCallback(replace, XtNcallback, ReplaceCB, (XtPointer)data);
  new = XtVaCreateManagedWidget("new", commandWidgetClass, form,
				XtNfromHoriz, (XtArgVal)replace,
				NULL);
  XtAddCallback(new, XtNcallback, NewCB, (XtPointer)NULL);
  data->scaleInstance = 
    XtVaCreateManagedWidget("scale", scaleWidgetClass, 
			    form,
			    XtNfromVert, (XtArgVal)close,
			    XtNscaleX, (XtArgVal)options.mag,
			    XtNscaleY, (XtArgVal)options.mag,
			    NULL);
  XtOverrideTranslations
    (data->scaleShell,
     XtParseTranslationTable ("<Message>WM_PROTOCOLS: close()"));
  XtSetArg(warg, XtNuserData, data);
  XtSetValues(data->scaleInstance, &warg, 1);
  data->pixShell = XtVaCreatePopupShell("pixShell", overrideShellWidgetClass, 
					toplevel,
					XtNborderWidth, (XtPointer)0,
					NULL);
  data->pixLabel = 
    XtVaCreateManagedWidget("pixLabel", labelWidgetClass, 
			    data->pixShell, 
			    XtNforeground, (XtPointer)WhitePixel(dpy, scr),
			    XtNbackground, (XtPointer)BlackPixel(dpy, scr),
			    XtNborderWidth, (XtPointer)0,
			    NULL);
  XtInstallAllAccelerators(form, form);	/* install accelerators */
}



/*
 * InitCursors() -- Create our cursors for area selection.
 */
static void
InitCursors()
{
  ulAngle = XCreateFontCursor(dpy, XC_ul_angle);
  urAngle = XCreateFontCursor(dpy, XC_ur_angle);
  lrAngle = XCreateFontCursor(dpy, XC_lr_angle);
  llAngle = XCreateFontCursor(dpy, XC_ll_angle);
}



/*
 * ParseSourceGeom() -- Determin dimensions of area to magnify from resources.
 */
static void 
ParseSourceGeom()
{
				/* source */
  srcStat = 
    XParseGeometry(options.source, &srcX, &srcY, &srcWidth, &srcHeight);
  if (XNegative & srcStat) srcX = DisplayWidth(dpy, scr) - srcX;
  if (YNegative & srcStat) srcY = DisplayHeight(dpy, scr) - srcY;
				/* mag */
}



/*
 * Main program.
 */
main(argc, argv)
     int argc;
     char **argv;
{
				/* SUPPRESS 594 */
  toplevel = XtAppInitialize(&app, "Xmag", optionDesc, XtNumber(optionDesc),
			     (Cardinal *)argc, argv, NULL,
			      NULL, 0);

  dpy = XtDisplay(toplevel);
  scr = DefaultScreen(dpy);
  XtGetApplicationResources(toplevel, (XtPointer) &options, resources,
			    XtNumber(resources), NULL, 0);
  ParseSourceGeom();
  XtAppAddActions(app, actions_table, XtNumber(actions_table));
  InitCursors();
  SetupGC();
  CreateRoot();
  if (!(XValue& srcStat && YValue & srcStat))
    StartRootPtrGrab(True, NULL);
  wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XtAppMainLoop(app);
}
