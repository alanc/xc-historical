/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include "x11perf.h"

static Window *children;
static XPoint *positions;
static Window cover;
static int rows;
static x_offset, y_offset;  /* Private global data for DoMoveWins */
static int xmax, ymax;
static delta1;		    /* Private global data for DoResizeWins */

#define STACK (4*(HEIGHT-10)/CHILDSIZE)

Bool InitMoveWins(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i = 0;

    rows = (p->objects + MAXCOLS - 1) / MAXCOLS;
    
    x_offset = 0;
    y_offset = 0;
    delta1   = 1;

    children = (Window *) malloc (p->objects*sizeof (Window));
    positions = (XPoint *) malloc(p->objects*sizeof(XPoint));

    xmax = (CHILDSIZE+CHILDSPACE) * (rows > 1 ? MAXCOLS : p->objects);
    ymax = rows * (CHILDSIZE+CHILDSPACE);

    for (i = 0; i != p->objects; i++) {
	positions[i].x = (CHILDSIZE+CHILDSPACE) * (i/rows) + CHILDSPACE/2;
	positions[i].y = (CHILDSIZE+CHILDSPACE) * (i%rows) + CHILDSPACE/2;
	children[i] = XCreateSimpleWindow(xp->d, xp->w,
	    positions[i].x, positions[i].y,
	    CHILDSIZE, CHILDSIZE, 0, xp->foreground, xp->foreground);
    }
    if (p->special)
	XMapSubwindows (xp->d, xp->w);
    return True;
}

void DoMoveWins(xp, p)
    XParms  xp;
    Parms p;
{
    int     i, j, x, y;

    for (i = 0; i != p->reps; i++) {
	x_offset += 1;
	y_offset += 3;
	if (y_offset + ymax > HEIGHT)
	    y_offset = 0;
	if (x_offset + xmax > WIDTH)
	    x_offset = 0;
	for (j = 0; j != p->objects; j++) {
	    XMoveWindow(xp->d, children[j],
	    positions[j].x + x_offset, positions[j].y + y_offset);
	}
    }
}

void EndMoveWins(xp, p)
    XParms  xp;
    Parms   p;
{
    free(children);
    free(positions);
}

void DoResizeWins(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i, j, delta2;

    for (i = 0; i != p->reps; i++) {
	delta1 = -delta1;
	delta2 = delta1;
	for (j = 0; j != p->objects; j++) {
	    delta2 = -delta2;
	    XResizeWindow(xp->d, children[j],
		CHILDSIZE+delta2, CHILDSIZE-delta2);
	}
    }
}

Bool InitCircWins(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i;
    int     pos;
    int     color;

    children = (Window *) malloc (p->objects * sizeof (Window));
    for (i = 0; i != p->objects; i++) {
	pos = i % STACK;
	color = (i & 1 ? xp->foreground : xp->background);
	children[i] = XCreateSimpleWindow (xp->d, xp->w, 
	    pos*CHILDSIZE/4 + (i/STACK)*2*CHILDSIZE, pos*CHILDSIZE/4,
	    CHILDSIZE, CHILDSIZE, 0, color, color);
    }
    if (p->special)
	XMapSubwindows (xp->d, xp->w);
    return True;
}

void DoCircWins(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i, j;

    for (i = 0; i != p->reps; i++)
	for (j = 0; j != p->objects; j++)
	    XCirculateSubwindows (xp->d, xp->w, RaiseLowest);
}

void EndCircWins(xp, p)
    XParms  xp;
    Parms   p;
{
    free(children);
}


Bool InitMoveTree(xp, p)
    XParms  xp;
    Parms   p;
{
    int     i = 0;

    rows = (p->objects + MAXCOLS - 1) / MAXCOLS;
    
    x_offset = 0;
    y_offset = 0;
    delta1   = 1;

    children = (Window *) malloc (p->objects*sizeof (Window));
    positions = (XPoint *) malloc(p->objects*sizeof(XPoint));

    xmax = (CHILDSIZE+CHILDSPACE) * (rows > 1 ? MAXCOLS : p->objects);
    ymax = rows * (CHILDSIZE+CHILDSPACE);

    cover = XCreateSimpleWindow(xp->d, xp->w,
				0, 0, xmax, ymax, 0,
				xp->background, xp->background);
				
    for (i = 0; i != p->objects; i++) {
	positions[i].x = (CHILDSIZE+CHILDSPACE) * (i/rows) + CHILDSPACE/2;
	positions[i].y = (CHILDSIZE+CHILDSPACE) * (i%rows) + CHILDSPACE/2;
	children[i] = XCreateSimpleWindow(xp->d, cover,
	    positions[i].x, positions[i].y,
	    CHILDSIZE, CHILDSIZE, 0, xp->foreground, xp->foreground);
    }
    XMapSubwindows (xp->d, cover);
    XMapWindow (xp->d, cover);
    return True;
}

void DoMoveTree(xp, p)
    XParms  xp;
    Parms p;
{
    int     i, j;

    for (i = 0; i != p->reps; i++) {
	x_offset += 1;
	y_offset += 3;
	if (y_offset + ymax > HEIGHT)
	    y_offset = 0;
	if (x_offset + xmax > WIDTH)
	    x_offset = 0;
	XMoveWindow(xp->d, cover, x_offset, y_offset);
    }
}

void EndMoveTree(xp, p)
    XParms  xp;
    Parms   p;
{
    XDestroyWindow(xp->d, cover);
    free(children);
    free(positions);
}
