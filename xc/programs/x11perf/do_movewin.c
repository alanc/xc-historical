#include "x11perf.h"

static Window w;
static Window *children;
static rows, columns, x_offset, y_offset;
#define INIT_OFFSET 10
#define STACK (4*(HEIGHT-10)/CHILDSIZE)

Bool InitMoveWins(d, p)
    Display *d;
    Parms p;
{
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
	    children[i++] = XCreateSimpleWindow (d, w, 
		(CHILDSIZE+CHILDSPACE) * x + CHILDSPACE/2,
		(CHILDSIZE+CHILDSPACE) * y + CHILDSPACE/2,
		CHILDSIZE, CHILDSIZE, 0, fgPixel, fgPixel);
	    if (i == p->objects)
		goto Enough;
	}
Enough:
    if (p->special)
	XMapSubwindows (d, w);
    return True;
}

Bool InitCircWins(d, p)
    Display *d;
    Parms p;
{
    int     i;

    CreatePerfStuff (d, 1, WIDTH, HEIGHT, &w, NULL, NULL);
    children = (Window *) malloc (p -> objects * sizeof (Window));
    for (i = 0; i < p->objects; i++) {
	register int pos = i % STACK;
	children[i] = XCreateSimpleWindow (d, w, 
	pos*CHILDSIZE/4 + (i/STACK)*2*CHILDSIZE, pos*CHILDSIZE/4,
	CHILDSIZE, CHILDSIZE, 1, bgPixel, fgPixel);
    }
    if (p -> special)
	XMapSubwindows (d, w);
    return True;
}

void DoMoveWins(d, p)
    Display *d;
    Parms p;
{
    int     child, i, x, y;

    for (i = 0; i < p -> reps; i++) {
	x_offset += 1;
	y_offset += 3;
	if (y_offset >= HEIGHT - rows*(CHILDSIZE+CHILDSPACE))
	    y_offset = INIT_OFFSET;
	if (x_offset >= WIDTH - columns*(CHILDSIZE+CHILDSPACE))
	    x_offset = INIT_OFFSET;
	child = p -> objects - 1;
	for (x = columns - 1; x >= 0; x--)
	    for (y = rows - 1; y >= 0; y--) {
		XMoveWindow (d, children[child],
		    x*(CHILDSIZE+CHILDSPACE) + x_offset, 
		    y*(CHILDSIZE+CHILDSPACE) + y_offset);
		child--;
	    }
    }
}

void DoResizeWins(d, p)
    Display *d;
    Parms p;
{
    int     i, j, delta1, delta2;

    delta1 = 1;
    for (i = 0; i < p -> reps; i++) {
	delta1 = -delta1;
	delta2 = delta1;
	for (j = 0; j < p -> objects; j++) {
	    delta2 = -delta2;
	    XResizeWindow(d, children[j], CHILDSIZE+delta2, CHILDSIZE-delta2);
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

