#include "x11perf.h"

static XPoint *points;
static GC bggc, fggc;
static Window w;

Bool InitLines(d, p)
    Display *d;
    Parms   p;
{
    int     size;
    int i;
    int x, y;		/* Next point					*/
    int xdir, ydir;	/* Which direction x, y are going		*/
    int x1, y1;		/* offsets to compute next point from current	*/
    int x1inc, y1inc;   /* How to get to next x1, y1			*/
    int minorphase;     /* # iterations left with current x1inc, y1inc  */
    int majorphase;     /* count 0..3 for which type of x1inc, y1inc    */

    size = p->special;

    points = (XPoint *)malloc((p->objects+1) * sizeof(XPoint));

    /* All this x, x1, x1inc, etc. stuff is to create a pattern that
	(1) scans down the screen vertically

	(2) rotates the endpoints through all possible orientations

	(3) bounces off bottom and top of window as needed

    */

    x     = (WIDTH-size)/2;     y     = 0;
    xdir  = 1;     ydir  = 1;
    x1    = size;  y1    = 0;
    x1inc = 0;     y1inc = 1;

    minorphase = size;
    majorphase = 0;

    points[0].x = x;
    points[0].y = y;

    for (i = 1; i < (p->objects+1); i++) {    
	/* Move x left or right by x1 */
	x += (xdir * x1);
	xdir = -xdir;

	/* Update x1 by 0 or 1 */
	x1 += x1inc;

	/* Move on down or up the screen */
	y += (ydir * y1);

	/* If off either top or bottom, backtrack to previous position and go
	   the other way instead */
	if (y < 0 || y >= HEIGHT) {
	    ydir = -ydir;
	    y += (2 * ydir * y1);
	}

	/* Update y1 by 0 or 1 */
	y1 += y1inc;

	points[i].x = x;
	points[i].y = y;

	/* Change increments if needed */
	minorphase--;
	if (minorphase < 0) {
	    minorphase = size;
	    majorphase = (majorphase + 1) % 4;
	    switch (majorphase) {
		case 0: x1 = size; x1inc =  0; y1 =    0; y1inc =  1; break;
		case 1: x1 = size; x1inc = -1; y1 = size; y1inc =  0; break;
		case 2: x1 =    0; x1inc =  1; y1 = size; y1inc =  0; break;
		case 3: x1 = size; x1inc =  0; y1 = size; y1inc = -1; break;
	    }
	}
    }
    CreatePerfStuff(d, 1, WIDTH, HEIGHT, &w, &bggc, &fggc);
    return True;
}
 
Bool InitDashedLines(d, p)
    Display *d;
    Parms   p;
{
    char dashes[2];

    (void)InitLines(d, p);

    /* Modify GCs to draw dashed */
    XSetLineAttributes(d, bggc, 0, LineOnOffDash, CapButt, JoinMiter);
    XSetLineAttributes(d, fggc, 0, LineOnOffDash, CapButt, JoinMiter);
    dashes[0] = 1;   dashes[1] = 3;
    XSetDashes(d, fggc, 0, dashes, 2);
    XSetDashes(d, bggc, 0, dashes, 2);
    return True;
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
        XDrawLines(d, w, pgc, points, p->objects+1, CoordModeOrigin);
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
    XDestroyWindow(d, w);
    XFreeGC(d, bggc);
    XFreeGC(d, fggc);
    free(points);
}

