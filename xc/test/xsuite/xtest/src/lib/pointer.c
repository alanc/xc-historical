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
/*LINTLIBRARY*/

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	<unistd.h>

/*
 * Warp the pointer to the specified location.  Return pointer to
 * structure contain old and new location information.  Register
 * structure with resource registration system.
 */
PointerPlace *
warppointer(display, dest_w, dest_x, dest_y)
Display	*display;
Window	dest_w;
int	dest_x;
int	dest_y;
{
	PointerPlace *ptr;
	Window	wtmp;
	int	itmp;
	unsigned int uitmp;

	ptr = (PointerPlace *) malloc(sizeof(*ptr));
	if (ptr == (PointerPlace *) NULL) {
		delete("Memory allocation failed in warppointer: %d bytes",
			sizeof(*ptr));
		return((PointerPlace *) NULL);
	}

	/* Save initial pointer location. */
	(void) XQueryPointer(display, DRW(display), &(ptr->oroot), &wtmp, &(ptr->ox), &(ptr->oy), &itmp, &itmp, &uitmp);
	/* Warp pointer to window. */
	XWarpPointer(display, None, dest_w, 0, 0, 0, 0, dest_x, dest_y);
	/* Get new pointer location. */
	(void) XQueryPointer(display, DRW(display), &(ptr->nroot), &wtmp, &(ptr->nx), &(ptr->ny), &itmp, &itmp, &uitmp);

	regid(display, (union regtypes *)&ptr, REG_POINTER);
	return(ptr);
}

/*
 * Return True if pointer location has changed from the specified location.
 * No reporting is performed.
 */
Bool
pointermoved(display, ptr)
Display	*display;
PointerPlace	*ptr;
{
	Window	root;
	int	x, y;
	Window	wtmp;
	int	itmp;
	unsigned int uitmp;

	(void) XQueryPointer(display, DRW(display), &root, &wtmp, &x, &y, &itmp, &itmp, &uitmp);
	if (root != ptr->nroot || x != ptr->nx || y != ptr->ny)
		return(True);
	else
		return(False);
}

/*
 * Return the pointer to saved location and free data structure.
 */
void
unwarppointer(display, ptr)
Display	*display;
PointerPlace *ptr;
{
	XWarpPointer(display, None, ptr->oroot, 0, 0, 0, 0, ptr->ox, ptr->oy);
	free(ptr);
}

/*
 * Return pointer information relative to the root window.
 */

void
pointerrootpos(display, ptr)
Display	*display;
PointerPlace	*ptr;
{
	Window	root;
	int	x, y;
	Window	wtmp;
	int	itmp;
	unsigned int uitmp;

	(void) XQueryPointer(display, DRW(display), &root, &wtmp, &x, &y, &itmp, &itmp, &uitmp);
	ptr->nx = x;
	ptr->ny = y;
}

