/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 *
 * $NCDId: @(#)colormap.h,v 1.1 1993/12/01 23:46:32 lemke Exp $
 */

#ifndef COLORMAP_H_
#define COLORMAP_H_

#include	<X11/X.h>
#include	<X11/Xproto.h>
#include	"os.h"

typedef struct _rgbentry {
    char       *name;
    int         namelen;
    Colormap    cmap;
    int         xred,
                xblue,
                xgreen;		/* exact */
    int         vred,
                vblue,
                vgreen;		/* visual */
}           RGBEntryRec, *RGBEntryPtr;

typedef unsigned long Pixel;

extern RGBEntryPtr FindColorName();
extern Bool AddColorName();

extern int  CreateVisual();

extern int  FindPixel();
extern int  FindNamedPixel();
extern int  StorePixel();
extern int  StoreNamedPixel();
extern int  FreePixels();
extern int  CreateColormap();
extern int  FreeColormap();
extern int  CopyAndFreeColormap();

#endif				/* COLORMAP_H_ */
