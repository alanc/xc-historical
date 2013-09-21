/* $XConsortium: colormap.h,v 1.7 95/05/17 18:23:24 dpw Exp $ */
/*
 * Copyright 1994 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Network Computing Devices, Inc. not be
 * used in advertising or publicity pertaining to distribution of this
 * software without specific, written prior permission.
 *
 * THIS SOFTWARE IS PROVIDED `AS-IS'.  NETWORK COMPUTING DEVICES, INC.,
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NONINFRINGEMENT.  IN NO EVENT SHALL NETWORK
 * COMPUTING DEVICES, INC., BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE, DATA,
 * OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND REGARDLESS OF
 * WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * $NCDId: @(#)colormap.h,v 1.5 1994/03/24 17:55:00 lemke Exp $
 */

#ifndef COLORMAP_H_
#define COLORMAP_H_

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

typedef CARD32 Pixel;


/* XXX may want to change this stuff to be a list of names & RGB values,
 * to allow multiple user-values to map to the same pixel
 */
typedef struct _entry {
    char       *name;
    int         len;
    int         red,			/* requested values */
                green,
                blue;
    int         rep_red,		/* returned values */
                rep_green,
                rep_blue;
    short       refcnt;
    int         pixel;
}           Entry;

extern RGBEntryPtr FindColorName(
#if NeedFunctionPrototypes
    char * /*name*/,
    int /*len*/,
    Colormap /*cmap*/
#endif
);

extern Bool AddColorName(
#if NeedFunctionPrototypes
    char * /*name*/,
    int /*len*/,
    RGBEntryRec * /*rgbe*/
#endif
);

extern int CreateVisual(
#if NeedFunctionPrototypes
    int /*depth*/,
    xVisualType * /*vis*/
#endif
);

extern int FindPixel(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/,
    int /*red*/,
    int /*green*/,
    int /*blue*/,
    Entry ** /*pent*/
#endif
);

extern int FindNamedPixel(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/,
    char * /*name*/,
    int /*namelen*/,
    Entry ** /*pent*/
#endif
);

extern int StorePixel(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/,
    int /*red*/,
    int /*green*/,
    int /*blue*/,
    int /*rep_red*/,
    int /*rep_green*/,
    int /*rep_blue*/,
    Pixel /*pixel*/
#endif
);

extern int StoreNamedPixel(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/,
    char * /*name*/,
    int /*namelen*/,
    int /*xred*/,
    int /*xgreen*/,
    int /*xblue*/,
    int /*vred*/,
    int /*vgreen*/,
    int /*vblue*/,
    Pixel /*pixel*/
#endif
);

extern int FreeClientPixels(
#if NeedFunctionPrototypes
    pointer /*pcr*/,
    XID /*id*/
#endif
);

extern int IncrementPixel(
#if NeedFunctionPrototypes
    ClientPtr /*pclient*/,
    Colormap /*cmap*/,
    Entry * /*pent*/
#endif
);

extern int FreePixels(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/,
    int /*num*/,
    Pixel /*pixels*/[]
#endif
);

extern int CreateColormap(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/,
    Window /*win*/,
    VisualID /*visual*/
#endif
);

extern int DestroyColormap(
#if NeedFunctionPrototypes
    pointer /*pmap*/,
    XID /*id*/
#endif
);

extern int FreeColormap(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*cmap*/
#endif
);

extern int CopyAndFreeColormap(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Colormap /*new*/,
    Colormap /*old*/
#endif
);

extern void FreeColors(
#if NeedFunctionPrototypes
    void
#endif
);

#endif				/* COLORMAP_H_ */
