#include "x11perf.h"

static Window w;
static Window *children;
static rows, columns, x_offset, y_offset;
#define INIT_OFFSET 10
#define STACK ((HEIGHT-40)/5)

void InitMoveWins(d, p)
    Display *d;
    Parms p;
{
    int     wp = WhitePixel (d, 0);
    int     bp = BlackPixel (d, 0);
    int     i = 0;
    int     x, y;

    if (p->objects < 50) {
	rows = 1;
	columns = p->objects;
    }
    else {
	columns = COLS;
	rows = p->objects/COLS;
    }
    p->objects = rows*columns;
    x_offset = INIT_OFFSET;
    y_offset = INIT_OFFSET;

    CreatePerfStuff (d, 1, WIDTH, HEIGHT, &w, NULL, NULL);
    children = (Window *) malloc (p->objects*sizeof (Window));
    for (x = 0; x < columns; x++)
	for (y = 0; y < rows; y++) {
	    children[i++] = XCreateSimpleWindow (
		d, w, x*20 + x_offset, y*20 + y_offset, 10, 10, 0, bp, bp);
	}
    if (p->special)
	XMapSubwindows (d, w);
}

void InitCircWins(d, p)
    Display *d;
    Parms p;
{
    int     wp = WhitePixel (d, 0);
    int     bp = BlackPixel (d, 0);
    int     i;

    CreatePerfStuff (d, 1, WIDTH, HEIGHT, &w, NULL, NULL);
    children = (Window *) malloc (p -> objects * sizeof (Window));
    for (i = 0; i < p->objects; i++) {
	register int pos = i % STACK;
	children[i] = XCreateSimpleWindow (
		d, w, 5*pos + 10*(i/STACK), 5*pos, 20, 20, 1, wp, bp);
    }
    if (p -> special)
	XMapSubwindows (d, w);
}

void DoMoveWins(d, p)
    Display *d;
    Parms p;
{
    int     child, i, x, y;

    for (i = 0; i < p -> reps; i++) {
	x_offset += 1;
	y_offset += 3;
	if (y_offset >= HEIGHT)
	    y_offset = INIT_OFFSET;
	child = p -> objects - 1;
	for (x = columns - 1; x >= 0; x--)
	    for (y = rows - 1; y >= 0; y--) {
		XMoveWindow (
		    d, children[child--], x*20 + x_offset, y*20 + y_offset);
	    }
    }
}

void DoCircWins(d, p)
    Display *d;
    Parms p;
{
    int     i, j;

    for (i = 0; i < p -> reps; i++)
	for (j = 0; j < p -> objects; j++)
	    XCirculateSubwindows (d, w, RaiseLowest);
}

void EndMoveWins(d, p)
    Display *d;
    Parms p;
{
    XDestroyWindow(d, w);
    free(children);
}

