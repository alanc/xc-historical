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

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    
    for (i = 0; i != p->objects; i++)
    {    
	arcs[i].x = rand() % (WIDTH - size);
	arcs[i].y = rand() % (HEIGHT - size);
	arcs[i].width = size;
	arcs[i].height = size;
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
}

void InitSmallCircles(d, p)
    Display *d;
    Parms p;
{
    InitSizedCircles(d, p, 1);
}


void InitMedCircles(d, p)
    Display *d;
    Parms p;
{
    InitSizedCircles(d, p, 10);
}


void InitCircles(d, p)
    Display *d;
    Parms   p;
{
    int     MAXSIZE;
    int     size, sizeinc;
    int i;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    size = 0;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    
    if (HEIGHT > WIDTH) MAXSIZE = WIDTH; else MAXSIZE = HEIGHT;
    sizeinc = MAXSIZE / p->objects;
    if (sizeinc == 0) sizeinc = 1;

    for (i = 0; i != p->objects; i++)
    {    
	arcs[i].x = rand() % (WIDTH - size);
	arcs[i].y = rand() % (HEIGHT - size);
	arcs[i].width = size;
	arcs[i].height = size;
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;
	size += sizeinc;
	if (size > MAXSIZE) size -= MAXSIZE;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
}

void InitSizedEllipses(d, p, size)
    Display *d;
    Parms   p;
    int     size;
{
    int i;
    int vsize;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    vsize = 0;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    
    for (i = 0; i != p->objects; i++)
    {    
	arcs[i].x = rand() % (WIDTH - size);
	arcs[i].y = rand() % (HEIGHT - size);
	if (i & 1) {
	    /* Make vertical axis longer */
	    arcs[i].width = vsize;
	    arcs[i].height = size;
	} else {
	    /* Make horizontal axis longer */
	    arcs[i].width = size;
	    arcs[i].height = vsize;
	}
	vsize++;
	if (vsize > size) vsize = 0;
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
}

void InitSmallEllipses(d, p)
    Display *d;
    Parms p;
{
    InitSizedEllipses(d, p, 1);
}


void InitMedEllipses(d, p)
    Display *d;
    Parms p;
{
    InitSizedEllipses(d, p, 10);
}


extern double sqrt();

void InitEllipses(d, p, size)
    Display *d;
    Parms   p;
{
    int i;
    int xsize, ysize;
    int steps, xsizeinc, ysizeinc;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    xsize = 0;
    ysize = 0;
    arcs = (XArc *)malloc((p->objects) * sizeof(XArc));
    
    steps = (int) sqrt((double) p->objects);
    xsizeinc = WIDTH / steps;
    ysizeinc = HEIGHT / steps;

    for (i = 0; i != p->objects; i++)
    {    
	arcs[i].x = rand() % (WIDTH - xsize);
	arcs[i].y = rand() % (HEIGHT - ysize);
	arcs[i].width = xsize;
	arcs[i].height = ysize;
	arcs[i].angle1 = 0;
	arcs[i].angle2 = 360*64;
	xsize += xsizeinc;
	if (xsize > WIDTH) {
	    xsize -= WIDTH;
	    ysize += ysizeinc;
	    if (ysize > HEIGHT) ysize -= HEIGHT;
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
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

