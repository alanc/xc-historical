/* $XConsortium: xnestEvents.c,v 1.1 93/06/23 16:23:29 dmatic Exp $ */
/*

Copyright 1993 by Davor Matic

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.  Davor Matic makes no representations about
the suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

*/
#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "screenint.h"
#include "input.h"
#include "misc.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"

#define GC XlibGC
#include "Xlib.h"
#include "Xutil.h"
#undef GC

#include "Display.h"
#include "Screen.h"
#include "Window.h"
#include "Events.h"

unsigned lastEventTime = 0;

void ProcessInputEvents()
{
  mieqProcessInputEvents();
}

int TimeSinceLastInputEvent()
{
    if (lastEventTime == 0)
        lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
}

void SetTimeSinceLastInputEvent()
{
  lastEventTime = GetTimeInMillis();
}

static Bool xnestExposurePredicate(display, event, args)
     Display *display;
     XEvent *event;
     char *args;
{
  return (event->type == Expose || event->type == ProcessedExpose);
}

static Bool xnestNotExposurePredicate(display, event, args)
     Display *display;
     XEvent *event;
     char *args;
{
  return !xnestExposurePredicate(display, event, args);
}

void xnestCollectExposures()
{
  XEvent X;
  WindowPtr pWin;
  RegionRec Rgn;
  BoxRec Box;
  
  while (XCheckIfEvent(xnestDisplay, &X, xnestExposurePredicate, NULL)) {
    pWin = xnestWindowPtr(X.xexpose.window);
    
    if (pWin) {
      Box.x1 = pWin->drawable.x + wBorderWidth(pWin) + X.xexpose.x;
      Box.y1 = pWin->drawable.y + wBorderWidth(pWin) + X.xexpose.y;
      Box.x2 = Box.x1 + X.xexpose.width;
      Box.y2 = Box.y1 + X.xexpose.height;
      
      (*pWin->drawable.pScreen->RegionInit)(&Rgn, &Box, 1);
      
      miWindowExposures(pWin, &Rgn, NullRegion); 
    }
  }
}

void xnestCollectEvents()
{
  XEvent X;
  xEvent x;
  ScreenPtr pScreen;

  while (XCheckIfEvent(xnestDisplay, &X, xnestNotExposurePredicate, NULL)) {
    switch (X.type) {
    case KeyPress:
      x.u.u.type = KeyPress;
      x.u.u.detail = X.xkey.keycode;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
    case KeyRelease:
      x.u.u.type = KeyRelease;
      x.u.u.detail = X.xkey.keycode;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
    case ButtonPress:
      x.u.u.type = ButtonPress;
      x.u.u.detail = X.xbutton.button;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
    case ButtonRelease:
      x.u.u.type = ButtonRelease;
      x.u.u.detail = X.xbutton.button;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
    case MotionNotify:
      x.u.u.type = MotionNotify;
      x.u.keyButtonPointer.rootX = X.xmotion.x;
      x.u.keyButtonPointer.rootY = X.xmotion.y;
      x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
      mieqEnqueue(&x);
      break;
      
    case FocusIn:
      if (X.xfocus.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xfocus.window);
	if (pScreen)
	  xnestDirectInstallColormaps(pScreen);
      }
      break;
   
    case FocusOut:
      if (X.xfocus.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xfocus.window);
	if (pScreen)
	  xnestDirectUninstallColormaps(pScreen);
      }
      break;

    case KeymapNotify:
      break;

    case EnterNotify:
      if (X.xcrossing.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xcrossing.window);
	if (pScreen) {
	  NewCurrentScreen(pScreen, X.xcrossing.x, X.xcrossing.y);
	  x.u.u.type = MotionNotify;
	  x.u.keyButtonPointer.rootX = X.xcrossing.x;
	  x.u.keyButtonPointer.rootY = X.xcrossing.y;
	  x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis();
	  mieqEnqueue(&x);
	  xnestDirectInstallColormaps(pScreen);
	}
      }
      break;
      
    case LeaveNotify:
      if (X.xcrossing.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xcrossing.window);
	if (pScreen) {
	  xnestDirectUninstallColormaps(pScreen);
	}	
      }
      break;
      
      
    default:
      ErrorF("xnest warning: unhandled event\n");
      break;
    }
  }
}
