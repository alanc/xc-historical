#include "x11perf.h"

/* define CONTINUOUS if you want each line segment to join with the previous */
#define CONTINUOUS


static XSegment *segments;
static GC bggc, fggc;
static Window w[4];
static XRectangle ws[3] = {
    {100, 100, 200, 200},
    {150, 150, 200, 200},
    {200, 200, 200, 200}
  };

void InitSizedSegs(d, p, size)
    Display *d;
    Parms   p;
    int     size;
{
    int i, incr;
    int prevx, prevy;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    prevx = WIDTH / 2;
    prevy = HEIGHT / 2;
    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));
    for (i = 0; i < (p->objects); i++)
    {    
#ifdef CONTINUOUS
	segments[i].x1 = prevx;
	segments[i].y1 = prevy;
#else
	segments[i].x1 = rand() % WIDTH;
	segments[i].y1 = rand() % HEIGHT;
#endif
	if (segments[i].x1 < size || ((rand() >> 12) & 1)) {
	    segments[i].x2 = segments[i].x1 + size;
	} else {
	    segments[i].x2 = segments[i].x1 - size;
	}
	if (segments[i].y1 < size || ((rand() >> 12) & 1)) {
	    segments[i].y2 = segments[i].y1 + size;
	} else {
	    segments[i].y2 = segments[i].y1 - size;
	}
#ifdef CONTINUOUS
	prevx = segments[i].x2;
	prevy = segments[i].y2;
#endif
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}
   
void InitSmallSegs(d, p)
    Display *d;
    Parms p;
{
    InitSizedSegs(d, p, 1);
}


void InitMedSegs(d, p)
    Display *d;
    Parms p;
{
    InitSizedSegs(d, p, 10);
}


void InitSegs(d, p)
    Display *d;
    Parms p;
{
    int i;
    int prevx, prevy;

    for (i = 0; i < 4; i++)
	w[i] = None;
    i = 0;
    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));
    prevx = rand() % WIDTH;
    prevy = rand() % HEIGHT;
    for (i = 0; i < (p->objects); i++)
    {    
#ifdef CONTINUOUS
	segments[i].x1 = prevx;
	segments[i].y1 = prevy;
#else
        segments[i].x1 = rand() % WIDTH;
        segments[i].y1 = rand() % HEIGHT;
#endif
        segments[i].x2 = prevx = rand() % WIDTH;
        segments[i].y2 = prevy = rand() % HEIGHT;
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, w, &bggc, &fggc);
    for (i = 0; i < p->special; i++)
	w[i+1] = CreatePerfWindow(
	    d, ws[i].x, ws[i].y, ws[i].width, ws[i].height);
    
}

void DoSegs(d, p)
    Display *d;
    Parms p;
{
    GC pgc;
    int i;

    pgc = bggc;
    for (i=0; i<p->reps; i++)
    {
        XDrawSegments(d, w[0], pgc, segments, p->objects);
        if (pgc == bggc)
            pgc = fggc;
        else
            pgc = bggc;
    }
}

void EndSegs(d, p)
    Display *d;
    Parms p;
{
    int i;
    for (i = 0; i < 4; i++)
	if (w[i] != None)
	    XDestroyWindow(d, w[i]);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(segments);
}

