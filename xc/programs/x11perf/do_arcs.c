#include "x11perf.h"

static XArc *arcs;
static GC bggc, fggc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitSizedCircles(d, p, size)
    Display *d;
    Parms   p;
    int     size;
{
    int i;
    int x, y;		/* base of square to draw the circle in		    */
    int xorg, yorg;     /* Used to get from column to column or row to row  */

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    xorg = 0; yorg = 0;
    x    = 0; y    = 0;

    for (i = 0; i != p->objects; i++)
    {    
	arcs[i].x = x;
	arcs[i].y = y;
	arcs[i].width = size;
	arcs[i].height = size;
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;

	y += size;
	if (y >= HEIGHT - size) {
	    /* Go to next column */
	    yorg++;
	    if (yorg >= size || yorg >= HEIGHT - size) {
		yorg = 0;
		xorg++;
		if (xorg >= size || xorg >= WIDTH - size) {
		    xorg = 0;
		}
	    }
	    y = yorg;
	    x += size;
	    if (x >= WIDTH - size) {
		x = xorg;
	    }
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
}

void InitCircles1(d, p)
    Display *d;
    Parms p;
{
    InitSizedCircles(d, p, 1);
}


void InitCircles10(d, p)
    Display *d;
    Parms p;
{
    InitSizedCircles(d, p, 10);
}


void InitCircles100(d, p)
    Display *d;
    Parms p;
{
    InitSizedCircles(d, p, 100);
}


void InitCircles500(d, p)
    Display *d;
    Parms p;
{
    InitSizedCircles(d, p, 500);
}


void InitSizedEllipses(d, p, size)
    Display *d;
    Parms   p;
    int     size;
{
    int i;
    int x, y;		/* base of square to draw ellipse in		*/
    int vsize, vsizeinc;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    vsize = 1;
    vsizeinc = size / p->objects;
    if (vsizeinc == 0) vsizeinc = 1;

    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    x = 0; y = 0;

    for (i = 0; i != p->objects; i++)
    {    
	arcs[i].x = x;
	arcs[i].y = y;
	if (i & 1) {
	    /* Make vertical axis longer */
	    arcs[i].width = vsize;
	    arcs[i].height = size;
	} else {
	    /* Make horizontal axis longer */
	    arcs[i].width = size;
	    arcs[i].height = vsize;
	}
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;

	y += size;
	if (y >= HEIGHT - size) {
	    /* Go to next column */
	    y = 0;
	    x += size;
	    if (x >= WIDTH - size) {
		x = 0;
	    }
	}
	
	vsize += vsizeinc;
	if (vsize > size) vsize -= size;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
}

void InitEllipses1(d, p)
    Display *d;
    Parms p;
{
    InitSizedEllipses(d, p, 1);
}


void InitEllipses10(d, p)
    Display *d;
    Parms p;
{
    InitSizedEllipses(d, p, 10);
}


void InitEllipses100(d, p)
    Display *d;
    Parms p;
{
    InitSizedEllipses(d, p, 100);
}


void InitEllipses500(d, p)
    Display *d;
    Parms p;
{
    InitSizedEllipses(d, p, 500);
}



void DoArcs(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = bggc;
    for (i=0; i<p->reps; i++)
    {
        XDrawArcs(d, w[0], pgc, arcs, p->objects);
        if (pgc == bggc)
            pgc = fggc;
        else
            pgc = bggc;
    }
}

void EndArcs(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < 4; i++)
	if (w[i] != None)
	    XDestroyWindow(d, w[i]);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(arcs);
}

