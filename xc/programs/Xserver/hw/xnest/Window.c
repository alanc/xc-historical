/* $XConsortium: Window.c,v 1.4 94/02/06 17:53:04 rws Exp $ */
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
#include "Xproto.h"
#include "gcstruct.h"
#include "window.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "colormapst.h"
#include "scrnintstr.h"
#include "region.h"

#define GC XlibGC
#include "Xlib.h"
#include "Xutil.h"
#include <X11/extensions/shape.h>
#undef GC

#include "Display.h"
#include "Screen.h"
#include "GC.h"
#include "Drawable.h"
#include "Color.h"
#include "Visual.h"
#include "Events.h"

int xnestWindowPrivateIndex;

static int xnestFindWindowMatch(pWin, ptr)
     WindowPtr pWin;
     pointer ptr;
{
  xnestWindowMatch *wm = (xnestWindowMatch *)ptr;
  if (wm->window == xnestWindow(pWin)) {
    wm->pWin = pWin;
    return WT_STOPWALKING;
  }
  else
    return WT_WALKCHILDREN;
}

WindowPtr xnestWindowPtr(window)
     Window window;
{
  xnestWindowMatch wm;
  int i;

  wm.pWin = NullWindow;
  wm.window = window;

  for (i = 0; i < screenInfo.numScreens; i++) {
    WalkTree(screenInfo.screens[i], xnestFindWindowMatch, (pointer) &wm);
    if (wm.pWin) break;
  }
  
  return wm.pWin;
}
    
Bool xnestCreateWindow(pWin)
     WindowPtr pWin;
{
  unsigned long mask;
  XSetWindowAttributes attributes;
  Visual *visual;
  ColormapPtr pCmap;

  if (pWin->drawable.class == InputOnly) {
    mask = 0L;
    visual = CopyFromParent;
  }
  else {
    mask = CWEventMask | CWBackingStore;
    attributes.event_mask = ExposureMask;
    attributes.backing_store = NotUseful;
    
    if (pWin->parent) {
      if (pWin->optional && pWin->optional->visual != wVisual(pWin->parent)) {
	visual = xnestVisualFromID(pWin->drawable.pScreen, wVisual(pWin));
	mask |= CWColormap;
	if (pWin->optional->colormap) {
	  pCmap = (ColormapPtr)LookupIDByType(wColormap(pWin), RT_COLORMAP);
	  attributes.colormap = xnestColormap(pCmap);
	}
	else
	  attributes.colormap = xnestDefaultVisualColormap(visual);
      }
      else 
	visual = CopyFromParent;
    }
    else { /* root windows have their own colormaps at creation time */
      visual = xnestVisualFromID(pWin->drawable.pScreen, wVisual(pWin));      
      pCmap = (ColormapPtr)LookupIDByType(wColormap(pWin), RT_COLORMAP);
      mask |= CWColormap;
      attributes.colormap = xnestColormap(pCmap);
    }
  }
  
  xnestWindowPriv(pWin)->window = XCreateWindow(xnestDisplay,
						xnestWindowParent(pWin),
						pWin->origin.x - 
						wBorderWidth(pWin),
						pWin->origin.y - 
						wBorderWidth(pWin),
						pWin->drawable.width,
						pWin->drawable.height,
						pWin->borderWidth,
						pWin->drawable.depth, 
						pWin->drawable.class,
						visual,
						mask, &attributes);
  xnestWindowPriv(pWin)->parent = xnestWindowParent(pWin);
  xnestWindowPriv(pWin)->x = pWin->origin.x - wBorderWidth(pWin);
  xnestWindowPriv(pWin)->y = pWin->origin.y - wBorderWidth(pWin);
  xnestWindowPriv(pWin)->width = pWin->drawable.width;
  xnestWindowPriv(pWin)->height = pWin->drawable.height;
  xnestWindowPriv(pWin)->border_width = pWin->borderWidth;
  xnestWindowPriv(pWin)->sibling_above = None;
  if (pWin->nextSib)
    xnestWindowPriv(pWin->nextSib)->sibling_above = xnestWindow(pWin);
#ifdef SHAPE
  xnestWindowPriv(pWin)->bounding_shape = 
    REGION_CREATE(pWin->drawable.pScreen, NULL, 1);
  xnestWindowPriv(pWin)->clip_shape = 
    REGION_CREATE(pWin->drawable.pScreen, NULL, 1);
#endif /* SHAPE */

  if (!pWin->parent) /* only the root window will have the right colormap */
    xnestSetInstalledColormapWindows(pWin->drawable.pScreen);
  
  return True;
}

Bool xnestDestroyWindow(pWin)
     WindowPtr pWin;
{
  if (pWin->nextSib)
    xnestWindowPriv(pWin->nextSib)->sibling_above = 
      xnestWindowPriv(pWin)->sibling_above;
#ifdef SHAPE
  REGION_DESTROY(pWin->drawable.pScreen, 
				xnestWindowPriv(pWin)->bounding_shape);
  REGION_DESTROY(pWin->drawable.pScreen, 
				xnestWindowPriv(pWin)->clip_shape);
#endif
  XDestroyWindow(xnestDisplay, xnestWindow(pWin));
  xnestWindowPriv(pWin)->window = None;

  if (pWin->optional && pWin->optional->colormap && pWin->parent)
    xnestSetInstalledColormapWindows(pWin->drawable.pScreen);

  return True;
}

Bool xnestPositionWindow(pWin, x, y)
     WindowPtr pWin;
     int x, y;
{
  xnestConfigureWindow(pWin, 
		       CWParent |
		       CWX | CWY | 
		       CWWidth | CWHeight | 
		       CWBorderWidth);
  
  return True;
}

void xnestConfigureWindow(pWin, mask)
     WindowPtr pWin;
     unsigned int mask;
{
  unsigned int valuemask;
  XWindowChanges values;

  if (mask & CWParent &&
      xnestWindowPriv(pWin)->parent != xnestWindowParent(pWin)) {
    XReparentWindow(xnestDisplay, xnestWindow(pWin), 
		    xnestWindowParent(pWin), 
		    pWin->origin.x - wBorderWidth(pWin),
		    pWin->origin.y - wBorderWidth(pWin));
    xnestWindowPriv(pWin)->parent = xnestWindowParent(pWin);
    xnestWindowPriv(pWin)->x = pWin->origin.x - wBorderWidth(pWin);
    xnestWindowPriv(pWin)->y = pWin->origin.y - wBorderWidth(pWin);
    xnestWindowPriv(pWin)->sibling_above = None;
    if (pWin->nextSib)
      xnestWindowPriv(pWin->nextSib)->sibling_above = xnestWindow(pWin);
  }
  
  valuemask = 0;
  
  if (mask & CWX &&
      xnestWindowPriv(pWin)->x != pWin->origin.x - wBorderWidth(pWin)) {
    valuemask |= CWX;
    values.x =
      xnestWindowPriv(pWin)->x = 
	pWin->origin.x - wBorderWidth(pWin);
  }

  if (mask & CWY &&
      xnestWindowPriv(pWin)->y != pWin->origin.y - wBorderWidth(pWin)) {
    valuemask |= CWY;
    values.y =
      xnestWindowPriv(pWin)->y = 
	pWin->origin.y - wBorderWidth(pWin);
  }

  if (mask & CWWidth &&
      xnestWindowPriv(pWin)->width != pWin->drawable.width) {
    valuemask |= CWWidth;
    values.width = 
      xnestWindowPriv(pWin)->width = 
	pWin->drawable.width;
  }
  
  if (mask & CWHeight &&
      xnestWindowPriv(pWin)->height != pWin->drawable.height) {
    valuemask |= CWHeight;
    values.height = 
      xnestWindowPriv(pWin)->height = 
	pWin->drawable.height;
  }
  
  if (mask & CWBorderWidth &&
      xnestWindowPriv(pWin)->border_width != pWin->borderWidth) {
    valuemask |= CWBorderWidth;
    values.border_width = 
      xnestWindowPriv(pWin)->border_width = 
	pWin->borderWidth;
  }
 
  if (valuemask)
    XConfigureWindow(xnestDisplay, xnestWindow(pWin), valuemask, &values);  
  
  if (mask & CWStackingOrder &&
      xnestWindowPriv(pWin)->sibling_above != xnestWindowSiblingAbove(pWin)) {
    WindowPtr pSib;
    
    /* find the top sibling */
    for (pSib = pWin; pSib->prevSib != NullWindow; pSib = pSib->prevSib);
    
    /* the top sibling */
    valuemask = CWStackMode;
    values.stack_mode = Above;
    XConfigureWindow(xnestDisplay, xnestWindow(pSib), valuemask, &values); 
    xnestWindowPriv(pSib)->sibling_above = None;

    /* the rest of siblings */
    for (pSib = pSib->nextSib; pSib != NullWindow; pSib = pSib->nextSib) {
      valuemask = CWSibling | CWStackMode;
      values.sibling = xnestWindowSiblingAbove(pSib);
      values.stack_mode = Below;
      XConfigureWindow(xnestDisplay, xnestWindow(pSib), valuemask, &values);
      xnestWindowPriv(pSib)->sibling_above = xnestWindowSiblingAbove(pSib);
    }
  }
}

Bool xnestChangeWindowAttributes(pWin, mask)
     WindowPtr pWin;
     unsigned long mask;
{
  XSetWindowAttributes attributes;
  
  if (mask & CWBackPixmap)
    switch (pWin->backgroundState) {
    case None:
      attributes.background_pixmap = None;
      break;
      
    case ParentRelative:
      attributes.background_pixmap = ParentRelative;
      break;
      
    case BackgroundPixmap:
      attributes.background_pixmap = xnestPixmap(pWin->background.pixmap);
      break;

    case BackgroundPixel:
      mask &= ~CWBackPixmap;  
      break;
    }

  if (mask & CWBackPixel)
    if (pWin->backgroundState == BackgroundPixel)
      attributes.background_pixel = xnestPixel(pWin->background.pixel);
    else
      mask &= ~CWBackPixel;
  
  if (mask & CWBorderPixmap)
    if (pWin->borderIsPixel)
      mask &= ~CWBorderPixmap;
    else
      attributes.border_pixmap = xnestPixmap(pWin->border.pixmap);
  
  if (mask & CWBorderPixel)
    if (pWin->borderIsPixel)
      attributes.border_pixel = xnestPixel(pWin->border.pixel);
    else
      mask &= ~CWBorderPixel;
  
  if (mask & CWBitGravity) 
    attributes.bit_gravity = pWin->bitGravity;
  
  if (mask & CWWinGravity) /* dix does this for us */
    mask &= ~CWWinGravity;

  if (mask & CWBackingStore) /* this is really not useful */
    mask &= ~CWBackingStore;

  if (mask & CWBackingPlanes) /* this is really not useful */
    mask &= ~CWBackingPlanes;

  if (mask & CWBackingPixel) /* this is really not useful */ 
    mask &= ~CWBackingPixel;

  if (mask & CWOverrideRedirect)
    attributes.override_redirect = pWin->overrideRedirect;

  if (mask & CWSaveUnder) /* this is really not useful */
    mask &= ~CWSaveUnder;

  if (mask & CWEventMask) /* events are handled elsewhere */
    mask &= ~CWEventMask;

  if (mask & CWDontPropagate) /* events are handled elsewhere */
    mask &= ~CWDontPropagate; 

  if (mask & CWColormap) {
    ColormapPtr pCmap;
    
    pCmap = (ColormapPtr)LookupIDByType(wColormap(pWin), RT_COLORMAP);

    attributes.colormap = xnestColormap(pCmap);

    xnestSetInstalledColormapWindows(pWin->drawable.pScreen);
  }

  if (mask & CWCursor) /* this is handeled in cursor code */
    mask &= ~CWCursor;

  if (mask)
    XChangeWindowAttributes(xnestDisplay, xnestWindow(pWin),
			    mask, &attributes);
  
  return True;
}	  

Bool xnestRealizeWindow(pWin)
     WindowPtr pWin;
{
  xnestConfigureWindow(pWin, CWStackingOrder);
#ifdef SHAPE
  xnestShapeWindow(pWin);
#endif /* SHAPE */
  XMapWindow(xnestDisplay, xnestWindow(pWin));

  return True;
}

Bool xnestUnrealizeWindow(pWin)
    WindowPtr pWin;
{
  XUnmapWindow(xnestDisplay, xnestWindow(pWin));

  return True;
}

void xnestPaintWindowBackground(pWin, pRegion, what)
     WindowPtr pWin;
     RegionPtr pRegion;
     int what;
{
  int i;
  BoxPtr pBox;

  xnestConfigureWindow(pWin, CWWidth | CWHeight);

  pBox = REGION_RECTS(pRegion);
  for (i = 0; i < REGION_NUM_RECTS(pRegion); i++)
    XClearArea(xnestDisplay, xnestWindow(pWin),
	       pBox[i].x1 - pWin->drawable.x,
	       pBox[i].y1 - pWin->drawable.y,
	       pBox[i].x2 - pBox[i].x1, 
	       pBox[i].y2 - pBox[i].y1,
	       False);
}

void xnestPaintWindowBorder(pWin, pRegion, what)
     WindowPtr pWin;
     RegionPtr pRegion;
     int what;
{
  xnestConfigureWindow(pWin, CWBorderWidth);
}

void xnestCopyWindow(pWin, oldOrigin, oldRegion)
     WindowPtr pWin;
     xPoint oldOrigin;
     RegionPtr oldRegion;
{
}

void xnestClipNotify(pWin, dx, dy)
     WindowPtr pWin;
     int dx, dy;
{
  xnestConfigureWindow(pWin, CWStackingOrder); 
#ifdef SHAPE
  xnestShapeWindow(pWin);
#endif /* SHAPE */
}

static Bool xnestWindowExposurePredicate(display, event, ptr)
     Display *display;
     XEvent *event;
     XPointer ptr;
{
  return (event->type == Expose && event->xexpose.window == *(Window *)ptr);
}

void xnestWindowExposures(pWin, pRgn, other_exposed)
     WindowPtr pWin;
     RegionPtr pRgn, other_exposed;
{
  XEvent event;
  Window window;
  BoxRec Box;
  
  XSync(xnestDisplay, False);
  
  window = xnestWindow(pWin);
  
  while (XCheckIfEvent(xnestDisplay, &event, 
		       xnestWindowExposurePredicate, (char *)&window)) {
    
    Box.x1 = pWin->drawable.x + wBorderWidth(pWin) + event.xexpose.x;
    Box.y1 = pWin->drawable.y + wBorderWidth(pWin) + event.xexpose.y;
    Box.x2 = Box.x1 + event.xexpose.width;
    Box.y2 = Box.y1 + event.xexpose.height;
	
    event.xexpose.type = ProcessedExpose;
	
    if (RECT_IN_REGION(pWin->drawable.pScreen, pRgn, &Box) != rgnIN)
      XPutBackEvent(xnestDisplay, &event);
  }
  
  miWindowExposures(pWin, pRgn, other_exposed);
}

#ifdef SHAPE
static Bool xnestRegionEqual(pReg1, pReg2)
     RegionPtr pReg1, pReg2;
{
  BoxPtr pBox1, pBox2;
  unsigned int n1, n2;

  if (pReg1 == pReg2) return True;

  if (pReg1 == NullRegion || pReg2 == NullRegion) return False;

  pBox1 = REGION_RECTS(pReg1);
  n1 = REGION_NUM_RECTS(pReg1);

  pBox2 = REGION_RECTS(pReg2);
  n2 = REGION_NUM_RECTS(pReg2);

  if (n1 != n2) return False;

  if (pBox1 == pBox2) return True;

  if (memcmp(pBox1, pBox2, n1 * sizeof(BoxRec))) return False;

  return True;
}

void xnestShapeWindow(pWin)
     WindowPtr pWin;
{
  Region reg;
  BoxPtr pBox;
  XRectangle rect;
  int i;
  Bool overlap;

  if (!xnestRegionEqual(xnestWindowPriv(pWin)->bounding_shape,
			wBoundingShape(pWin))) {
    
    if (wBoundingShape(pWin)) {
      REGION_COPY(pWin->drawable.pScreen, 
		xnestWindowPriv(pWin)->bounding_shape, wBoundingShape(pWin));
      
      reg = XCreateRegion();
      pBox = REGION_RECTS(xnestWindowPriv(pWin)->bounding_shape);
      for (i = 0; 
	   i < REGION_NUM_RECTS(xnestWindowPriv(pWin)->bounding_shape);
	   i++) {
        rect.x = pBox[i].x1;
        rect.y = pBox[i].y1;
        rect.width = pBox[i].x2 - pBox[i].x1;
        rect.height = pBox[i].y2 - pBox[i].y1;
        XUnionRectWithRegion(&rect, reg, reg);
      }
      XShapeCombineRegion(xnestDisplay, xnestWindow(pWin),
			  ShapeBounding, 0, 0, reg, ShapeSet);
      XDestroyRegion(reg);
    }
    else {
      REGION_EMPTY(pWin->drawable.pScreen, 
				xnestWindowPriv(pWin)->bounding_shape);
      
      XShapeCombineMask(xnestDisplay, xnestWindow(pWin),
			ShapeBounding, 0, 0, None, ShapeSet);
    }
  }
  
  if (!xnestRegionEqual(xnestWindowPriv(pWin)->clip_shape,
			wClipShape(pWin))) {
    
    if (wClipShape(pWin)) {
      REGION_COPY(pWin->drawable.pScreen, 
			xnestWindowPriv(pWin)->clip_shape, wClipShape(pWin));
      
      reg = XCreateRegion();
      pBox = REGION_RECTS(xnestWindowPriv(pWin)->clip_shape);
      for (i = 0; 
	   i < REGION_NUM_RECTS(xnestWindowPriv(pWin)->clip_shape);
	   i++) {
        rect.x = pBox[i].x1;
        rect.y = pBox[i].y1;
        rect.width = pBox[i].x2 - pBox[i].x1;
        rect.height = pBox[i].y2 - pBox[i].y1;
        XUnionRectWithRegion(&rect, reg, reg);
      }
      XShapeCombineRegion(xnestDisplay, xnestWindow(pWin),
			  ShapeClip, 0, 0, reg, ShapeSet);
      XDestroyRegion(reg);
    }
    else {
      REGION_EMPTY(pWin->drawable.pScreen, 
				     xnestWindowPriv(pWin)->clip_shape);
      
      XShapeCombineMask(xnestDisplay, xnestWindow(pWin),
			ShapeClip, 0, 0, None, ShapeSet);
    }
  }
}
#endif /* SHAPE */
