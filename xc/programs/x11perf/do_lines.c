#include "x11perf.h"

static XPoint *points;
static GC bggc, fggc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitSizedLines(d, p, size)
    Display *d;
    Parms   p;
    int     size;
{
    int i;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    points = (XPoint *)malloc((p->objects+1) * sizeof(XPoint));
    points[0].x = WIDTH / 2;
    points[0].y = HEIGHT / 2;
    for (i = 1; i < (p->objects+1); i++)
    {    
	if (points[i-1].x < size || ((rand() >> 12) & 1)) {
	    points[i].x = points[i-1].x + size;
	} else {
	    points[i].x = points[i-1].x - size;
	}
	if (points[i-1].y < size || ((rand() >> 12) & 1)) {
	    points[i].y = points[i-1].y + size;
	} else {
	    points[i].y = points[i-1].y - size;
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
}

void InitSmallLines(d, p)
    Display *d;
    Parms p;
{
    InitSizedLines(d, p, 1);
}


void InitMedLines(d, p)
    Display *d;
    Parms p;
{
    InitSizedLines(d, p, 10);
}


void InitLines(d, p)
    Display *d;
    Parms p;
{
    int i;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    points = (XPoint *)malloc((p->objects+1) * sizeof(XPoint));
    for (i = 0; i < (p->objects+1); i++)
    {    
        points[i].x = rand() % WIDTH;
        points[i].y = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}

void InitDashedLines(d, p)
    Display *d;
    Parms   p;
{
    XGCValues gcv;
    char dashes[2];

    InitLines(d, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(d, bggc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(d, fggc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 1;   dashes[1] = 3;
    XSetDashes(d, fggc, 0, dashes, 2);
    XSetDashes(d, bggc, 0, dashes, 2);
}

void DoLines(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = bggc;
    for (i=0; i<p->reps; i++)
    {
        XDrawLines(d, w[0], pgc, points, p->objects+1, CoordModeOrigin);
        if (pgc == bggc)
            pgc = fggc;
        else
            pgc = bggc;
    }
}

void EndLines(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < 4; i++)
	if (w[i] != None)
	    XDestroyWindow(d, w[i]);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(points);
}

