#include "x11perf.h"

static XRectangle *rects;
static GC bggc, fggc;
static Window w;

static unsigned char bitmap8x8[] = {
    0xCC, 0x66, 0x33, 0x99, 0xCC, 0x66, 0x33, 0x99
/*
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA
*/
};

static unsigned char bitmap4x4[] = {
   0x03, 0x06, 0x0c, 0x09
};


Bool InitRects(d, p)
    Display *d;
    Parms p;
{
    int i, x, y;
    int width = p->special;
    int cols = COLS;

	/* ||| This has to be changed */
    if (width == 50) {
        cols = 10;
    } else if (width == 500) {
	cols = 1;
    }
    rects = (XRectangle *)malloc(p->objects * sizeof(XRectangle));
    i = 0;
    for (x=0; x < cols; x++)
        for (y = 0; y < p->objects/cols; y++) {
#ifdef spacedrectangles
	    /* This code actually lets you see the separate rectangles... */
	    rects[i].x = x*(width + 1);
            rects[i].y = y*(width + 1);
#else
	    /* ... but this code makes stipple/tile alignment bugs obvious. */
	    rects[i].x = x*(width);
            rects[i].y = y*(width);
#endif
	    rects[i].width = rects[i].height = width;
	    i++;
	}
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);

    if (p->fillStyle == FillStippled || p->fillStyle == FillOpaqueStippled) {
	Pixmap  stipple;
	XGCValues gcv;

	stipple = XCreateBitmapFromData(d, w, bitmap8x8, 8, 8);
	gcv.stipple = stipple;
	gcv.fill_style = p->fillStyle;
	XChangeGC(d, fggc, GCFillStyle | GCStipple, &gcv);
	XChangeGC(d, bggc, GCFillStyle | GCStipple, &gcv);
	XFreePixmap(d, stipple);

    } else if (p->fillStyle == FillTiled) {
	Pixmap tile;
	XGCValues gcv;

	gcv.fill_style = FillTiled;
	tile = XCreatePixmapFromBitmapData(d, w, bitmap4x4, 4, 4,
		    fgPixel, bgPixel,  DefaultDepth(d, DefaultScreen(d)));
	gcv.tile = tile;
	XChangeGC(d, fggc, GCFillStyle | GCTile, &gcv);
	XFreePixmap(d, tile);
	tile = XCreatePixmapFromBitmapData(d, w, bitmap4x4, 4, 4,
		    bgPixel, fgPixel, DefaultDepth(d, DefaultScreen(d)));
	gcv.fill_style = FillTiled;
	gcv.tile = tile;
	XChangeGC(d, bggc, GCFillStyle | GCTile, &gcv);
	XFreePixmap(d, tile);
    }
    return True;
}

void DoRects(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = bggc;
    for (i=0; i < p->reps; i++)
    {
        XFillRectangles(d, w, pgc, rects, p->objects);
        if (pgc == bggc)
            pgc = fggc;
        else
            pgc = bggc;
    }
}

void EndRects(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(rects);
}

