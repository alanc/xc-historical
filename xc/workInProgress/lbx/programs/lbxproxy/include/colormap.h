/*
 * Copyright 1988-1993 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */

/* $XConsortium:$ */

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
