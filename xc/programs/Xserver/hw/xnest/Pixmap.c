/* $XConsortium: xnestPixmap.c,v 1.1 93/06/23 16:23:36 dmatic Exp $ */
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
#include "miscstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "regionstr.h"
#include "gc.h"
#include "servermd.h"
#include "mi.h"

#define GC XlibGC
#include "Xlib.h"
#include "Xutil.h"
#undef GC

#include "Display.h"
#include "Screen.h"
#include "Pixmap.h"

PixmapPtr xnestCreatePixmap(pScreen, width, height, depth)
    ScreenPtr   pScreen;
    int         width;
    int         height;
    int         depth;
{
  PixmapPtr pPixmap;

  pPixmap = (PixmapPtr)xalloc(sizeof(PixmapRec) + sizeof(xnestPrivPixmap));
  if (!pPixmap)
    return NullPixmap;
  pPixmap->drawable.type = DRAWABLE_PIXMAP;
  pPixmap->drawable.class = 0;
  pPixmap->drawable.depth = depth;
  pPixmap->drawable.bitsPerPixel = depth;
  pPixmap->drawable.id = 0;
  pPixmap->drawable.x = 0;
  pPixmap->drawable.y = 0;
  pPixmap->drawable.width = width;
  pPixmap->drawable.height = height;
  pPixmap->drawable.pScreen = pScreen;
  pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
  pPixmap->refcnt = 1;
  pPixmap->devKind = PixmapBytePad(width, depth);
  pPixmap->devPrivate.ptr = (pointer)(pPixmap + 1);
  if (width && height)
      xnestPixmapPriv(pPixmap)->pixmap = 
	  XCreatePixmap(xnestDisplay, 
			xnestDefaultWindows[pScreen->myNum],
			width, height, depth);
  else
      xnestPixmapPriv(pPixmap)->pixmap = 0;
  
  return pPixmap;
}

Bool xnestDestroyPixmap(pPixmap)
     PixmapPtr pPixmap;
{
  if(--pPixmap->refcnt)
    return TRUE;
  XFreePixmap(xnestDisplay, xnestPixmap(pPixmap));
  xfree(pPixmap);
  return TRUE;
}

RegionPtr xnestPixmapToRegion(pPixmap)
     PixmapPtr pPixmap;
{
  XImage *ximage;
  register RegionPtr pReg, pTmpReg;
  register int x, y;
  unsigned long previousPixel, currentPixel;
  BoxRec Box;
  int nWidth;
  Bool overlap;
  
  ximage = XGetImage(xnestDisplay, xnestPixmap(pPixmap), 0, 0,
		     pPixmap->drawable.width, pPixmap->drawable.height,
		     1, XYPixmap);
  
  pReg = (*pPixmap->drawable.pScreen->RegionCreate)(NULL, 1);
  pTmpReg = (*pPixmap->drawable.pScreen->RegionCreate)(NULL, 1);
  if(!pReg || !pTmpReg) return NullRegion;
  
  for (y = 0; y < pPixmap->drawable.height; y++) {
    Box.y1 = y;
    Box.y2 = y + 1;
    previousPixel = 0L;
    for (x = 0; x < pPixmap->drawable.width; x++) {
      currentPixel = XGetPixel(ximage, x, y);
      if (previousPixel != currentPixel) {
	if (previousPixel == 0L) { 
	  /* left edge */
	  Box.x1 = x;
	}
	else if (currentPixel == 0L) {
	  /* right edge */
	  Box.x2 = x;
	  (*pPixmap->drawable.pScreen->RegionReset)(pTmpReg, &Box);
	  (*pPixmap->drawable.pScreen->RegionAppend)(pReg, pTmpReg);
	}
	previousPixel = currentPixel;
      }
    }
    if (previousPixel != 0L) {
      /* right edge because of the end of pixmap */
      Box.x2 = pPixmap->drawable.width;
      (*pPixmap->drawable.pScreen->RegionReset)(pTmpReg, &Box);
      (*pPixmap->drawable.pScreen->RegionAppend)(pReg, pTmpReg);
    }
  }
  
  (*pPixmap->drawable.pScreen->RegionDestroy)(pTmpReg);
  XDestroyImage(ximage);

  (*pPixmap->drawable.pScreen->RegionValidate)(pReg, &overlap);

  return(pReg);
}
