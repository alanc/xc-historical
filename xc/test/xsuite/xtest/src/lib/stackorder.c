/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */

#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "pixval.h"

/*
 * stackorder() takes as input a window and returns its
 * position in the stacking order, with 0 being the lowest.
 */
int
stackorder(disp, win)
Display	*disp;
Window	win;
{
Window	*children;
Window	parent, root;
Window	dummy;
unsigned int 	nchild;
int 	result;
int 	i;

	/* Get the parent of the input window */
	if(!XQueryTree(disp, win, &root, &parent, &children, &nchild)) {
		debug(2, "stackorder: 1st XQueryTree returns 0");
		return(-1);
	}
	if (nchild != 0 && children)
		XFree((char*)children);	/* Not needed here */
	
	result = -1;

	/*
	 * Now get all the siblings of the input window and
	 * search for the input window among them; return index
	 * if found.
	 */
	debug(2, "win=%d", win);
	if(!XQueryTree(disp, parent, &root, &dummy, &children, &nchild)) {
		debug(2, "stackorder: 2nd XQueryTree returns 0");
		return(-1);
	}
	for (i = 0; i < nchild; i++) {
		debug(2, "child=%d", children[i]);
		if (children[i] == win) {
			result = i;
			break;
		}
	}

	if (children)
		XFree((char*)children);

	return(result);
}
