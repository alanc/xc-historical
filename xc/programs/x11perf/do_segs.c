#include "x11perf.h"

static Window   w;
static XSegment *segments;
static GC bggc, fggc;

Bool InitSegs(d, p)
    Display *d;
    Parms   p;
{
    int     size;
    int i;
    int x, y;		/* base of square to draw in			*/
    int x1, y1;		/* offsets into square				*/
    int x1inc, y1inc;   /* How to get to next x1, y1			*/
    int minorphase;     /* # iterations left with current x1inc, y1inc  */
    int majorphase;     /* count 0..3 for which type of x1inc, y1inc    */

    size = p->special;

    segments = (XSegment *)malloc((p->objects) * sizeof(XSegment));

    size--;		/* Because endcap counts as 1 pixel		*/

    /* All this x, x1, x1inc, etc. stuff is to create a pattern that
	(1) scans down the screen vertically, with each new segment going
	    into a square of size x size.

	(2) rotates the endpoints clockwise around the square

    */

    x     = 0;  y     = 0;
    x1    = 0;  y1    = 0;
    x1inc = 1;  y1inc = 0;
    minorphase = size;
    majorphase = 0;

    for (i = 0; i < (p->objects); i++) {    
	segments[i].x1 = x + x1;
	segments[i].y1 = y + y1;
	segments[i].x2 = x + size - x1;
	segments[i].y2 = y + size - y1;

	/* Change square to draw segment in */
	y += size + 1;
	if (y >= HEIGHT - size) {
	    /* Go to next column */
	    y = 0;
	    x += size + 1;
	    if (x >= WIDTH - size) {
		x = 0;
	    }
	}

	/* Change coordinates of offsets in square */
	x1 += x1inc;
	y1 += y1inc;

	/* Change increments if needed */
	minorphase--;
	if (minorphase <= 0) {
	    minorphase = size;
	    majorphase = (majorphase + 1) % 4;
	    switch (majorphase) {
		case 0: x1inc =  1; y1inc =  0; break;
		case 1: x1inc =  0; y1inc =  1; break;
		case 2: x1inc = -1; y1inc =  0; break;
		case 3: x1inc =  0; y1inc = -1; break;
	    }
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    return True;
}
   

Bool InitDashedSegs(d, p)
    Display *d;
    Parms   p;
{
    char dashes[2];

    (void)InitSegs(d, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(d, bggc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(d, fggc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 1;   dashes[1] = 3;
    XSetDashes(d, fggc, 0, dashes, 2);
    XSetDashes(d, bggc, 0, dashes, 2);
    return True;
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
        XDrawSegments(d, w, pgc, segments, p->objects);
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
    XDestroyWindow(d, w);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(segments);
}

