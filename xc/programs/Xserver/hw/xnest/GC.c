/* $XConsortium: GC.c,v 1.2 93/09/03 08:13:04 dpw Exp $ */
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
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "fontstruct.h"
#include "mistruct.h"
#include "region.h"

#define GC XlibGC
#include "Xlib.h"
#include "Xutil.h"
#undef GC

#include "Display.h"
#include "GC.h" 
#include "GCOps.h"
#include "Drawable.h"
#include "Font.h"
#include "Color.h"

int xnestGCPrivateIndex;

static GCFuncs xnestFuncs = {
  xnestValidateGC,
  xnestChangeGC,
  xnestCopyGC,
  xnestDestroyGC,
  xnestChangeClip,
  xnestDestroyClip,
  xnestCopyClip,
};

static GCOps xnestOps = {
  xnestFillSpans,
  xnestSetSpans,
  xnestPutImage,
  xnestCopyArea, 
  xnestCopyPlane,
  xnestPolyPoint,
  xnestPolylines,
  xnestPolySegment,
  xnestPolyRectangle,
  xnestPolyArc,
  xnestFillPolygon,
  xnestPolyFillRect,
  xnestPolyFillArc,
  xnestPolyText8, 
  xnestPolyText16,
  xnestImageText8,
  xnestImageText16,
  xnestImageGlyphBlt,
  xnestPolyGlyphBlt,
  xnestPushPixels
};

Bool xnestCreateGC(pGC)
     GCPtr pGC;
{
  pGC->clientClipType = CT_NONE;
  pGC->clientClip = NULL;
  
  pGC->funcs = &xnestFuncs;
  pGC->ops = &xnestOps;
  
  pGC->miTranslate = 1;
   
  xnestGCPriv(pGC)->gc = XCreateGC(xnestDisplay, 
				   xnestDefaultDrawables[pGC->depth], 
				   0L, NULL);
  xnestGCPriv(pGC)->nClipRects = 0;

  return True;
}

void xnestValidateGC(pGC, changes, pDrawable)
     GCPtr pGC;
     Mask changes;
     DrawablePtr pDrawable;
{
  pGC->lastWinOrg.x = pDrawable->x;
  pGC->lastWinOrg.y = pDrawable->y;
}

void xnestChangeGC(pGC, mask)
     GC *pGC;
     Mask mask;
{
  XGCValues values;
  
  if (mask & GCFunction)
    values.function = pGC->alu;
  
  if (mask & GCPlaneMask)
    values.plane_mask = pGC->planemask;
  
  if (mask & GCForeground)
    values.foreground = xnestPixel(pGC->fgPixel);
  
  if (mask & GCBackground)
    values.background = xnestPixel(pGC->bgPixel);

  if (mask & GCLineWidth)
    values.line_width = pGC->lineWidth;
    
  if (mask & GCLineStyle)
    values.line_style = pGC->lineStyle;

  if (mask & GCCapStyle)
    values.cap_style = pGC->capStyle;

  if (mask & GCJoinStyle)
    values.join_style = pGC->joinStyle;

  if (mask & GCFillStyle)
    values.fill_style = pGC->fillStyle;

  if (mask & GCFillRule)
    values.fill_rule = pGC->fillRule;
  
  if (mask & GCTile)
    if (pGC->tileIsPixel)
      mask &= ~GCTile;
    else
      values.tile = xnestPixmap(pGC->tile.pixmap);

  if (mask & GCStipple)
    values.stipple = xnestPixmap(pGC->stipple);

  if (mask & GCTileStipXOrigin)
    values.ts_x_origin = pGC->patOrg.x;

  if (mask & GCTileStipYOrigin)
    values.ts_y_origin = pGC->patOrg.y;

  if (mask & GCFont)
    values.font = xnestFont(pGC->font);

  if (mask & GCSubwindowMode)
    values.subwindow_mode = pGC->subWindowMode;

  if (mask & GCGraphicsExposures)
    values.graphics_exposures = pGC->graphicsExposures;

  if (mask & GCClipXOrigin)
    values.clip_x_origin = pGC->clipOrg.x;

  if (mask & GCClipYOrigin)
    values.clip_y_origin = pGC->clipOrg.y;

  if (mask & GCClipMask) /* this is handled in change clip */
    mask &= ~GCClipMask;

  if (mask & GCDashOffset)
    values.dash_offset = pGC->dashOffset;

  if (mask & GCDashList) {
    mask &= ~GCDashList;
    XSetDashes(xnestDisplay, xnestGC(pGC), 
	       pGC->dashOffset, (char *)pGC->dash, pGC->numInDashList);
  }

  if (mask & GCArcMode)
    values.arc_mode = pGC->arcMode;

  if (mask)
    XChangeGC(xnestDisplay, xnestGC(pGC), mask, &values);
}

void xnestCopyGC(pGCSrc, mask, pGCDst)
     GCPtr pGCSrc;
     Mask mask;
     GCPtr pGCDst;
{
  XCopyGC(xnestDisplay, xnestGC(pGCSrc), mask, xnestGC(pGCDst));
}

void xnestDestroyGC(pGC)
     GC *pGC;
{
  XFreeGC(xnestDisplay, xnestGC(pGC));
}

void xnestChangeClip(pGC, type, pValue, nRects)
     GCPtr pGC;
     int type;
     pointer pValue;
     int nRects;
{
  Region reg;
  int i;
  BoxPtr pBox;
  XRectangle rect;

  xnestDestroyClipHelper(pGC);

  switch(type) 
    {
    case CT_NONE:
      XSetClipMask(xnestDisplay, xnestGC(pGC), None);
      break;
      
    case CT_REGION:
      reg = XCreateRegion();
      pBox = REGION_RECTS((RegionPtr)pValue);
      for (i = 0; i < REGION_NUM_RECTS((RegionPtr)pValue); i++) {
	rect.x = pBox[i].x1;
	rect.y = pBox[i].y1;
	rect.width = pBox[i].x2 - pBox[i].x1;
	rect.height = pBox[i].y2 - pBox[i].y1;
	XUnionRectWithRegion(&rect, reg, reg);
      }
      XSetRegion(xnestDisplay, xnestGC(pGC), reg);
      XDestroyRegion(reg);
      break;

    case CT_PIXMAP:
      XSetClipMask(xnestDisplay, xnestGC(pGC), 
		   xnestPixmap((PixmapPtr)pValue));
      break;

    case CT_UNSORTED:
      XSetClipRectangles(xnestDisplay, xnestGC(pGC), 
			 pGC->clipOrg.x, pGC->clipOrg.y,
			 (XRectangle *)pValue, nRects, Unsorted);
      break;

    case CT_YSORTED:
      XSetClipRectangles(xnestDisplay, xnestGC(pGC), 
			 pGC->clipOrg.x, pGC->clipOrg.y,
			 (XRectangle *)pValue, nRects, YSorted);
      break;

    case CT_YXSORTED:
      XSetClipRectangles(xnestDisplay, xnestGC(pGC), 
			 pGC->clipOrg.x, pGC->clipOrg.y,
			 (XRectangle *)pValue, nRects, YXSorted);
      break;

    case CT_YXBANDED:
      XSetClipRectangles(xnestDisplay, xnestGC(pGC), 
			 pGC->clipOrg.x, pGC->clipOrg.y,
			 (XRectangle *)pValue, nRects, YXBanded);
      break;
    }

  pGC->clientClipType = type;
  pGC->clientClip = pValue;
  xnestGCPriv(pGC)->nClipRects = nRects;
}

void xnestDestroyClip(pGC)
     GCPtr pGC;
{
  xnestDestroyClipHelper(pGC);

  XSetClipMask(xnestDisplay, xnestGC(pGC), None);
 
  pGC->clientClipType = CT_NONE;
  pGC->clientClip = NULL;
  xnestGCPriv(pGC)->nClipRects = 0;
}

void xnestDestroyClipHelper(pGC)
     GCPtr pGC;
{
  switch (pGC->clientClipType)
    {
    case CT_NONE:
      break;
      
    case CT_REGION:
      (*pGC->pScreen->RegionDestroy)(pGC->clientClip); 
      break;
      
    case CT_PIXMAP:
      (*pGC->pScreen->DestroyPixmap)(pGC->clientClip);
      break;
      
    case CT_UNSORTED:
    case CT_YSORTED:
    case CT_YXSORTED:
    case CT_YXBANDED:
      xfree(pGC->clientClip);
      break;
    }
}

void xnestCopyClip(pGCDst, pGCSrc)
     GCPtr pGCSrc;
     GCPtr pGCDst;
{
  RegionPtr pRgn;
  int nRects, size;
  xRectangle *pRects;

  switch (pGCSrc->clientClipType)
    {
    case CT_NONE:
      xnestDestroyClip(pGCDst);
      break;

    case CT_REGION:
      pRgn = (*pGCDst->pScreen->RegionCreate)(NULL, 1);
      (*pGCDst->pScreen->RegionCopy)(pRgn, pGCSrc->clientClip);
      xnestChangeClip(pGCDst, CT_REGION, pRgn, 0);
      break;

    case CT_PIXMAP:
      xnestSharePixmap((PixmapPtr)pGCSrc->clientClip);
      xnestChangeClip(pGCDst, CT_PIXMAP, pGCSrc->clientClip, 0);
      break;

    case CT_UNSORTED:
    case CT_YSORTED:
    case CT_YXSORTED:
    case CT_YXBANDED: 
      nRects = xnestGCPriv(pGCSrc)->nClipRects;
      size = sizeof(xRectangle) * nRects;
      pRects = (xRectangle *)xalloc(size);
      memmove(pRects, pGCSrc->clientClip, size);
      xnestChangeClip(pGCDst, pGCSrc->clientClipType, pRects, nRects);
      break;
    }
}
