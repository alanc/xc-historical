/* $XConsortium: CutPaste.c,v 1.4 92/02/03 12:23:29 gildea Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Davor Matic, MIT X Consortium
 */

#include <X11/IntrinsicP.h>
#include <X11/Xmu/StdSel.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xatom.h>
#include "ScaleP.h"
#include "Scale.h"
#include <stdio.h>

extern Pixmap SWGetPixmap();
extern void SWAutoscale();

/*ARGSUSED*/
static Boolean
ConvertSelection(w, selection, target, type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    XtPointer *value;
    unsigned long *length;
    int *format;
{
    Boolean success;

    if (*target == XA_PIXMAP || *target == XA_BITMAP) {
	ScaleWidget sw = (ScaleWidget) w;
	Pixmap *pixmap = (Pixmap *) XtMalloc(sizeof(Pixmap));
	*pixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
				sw->scale.image->width, 
				sw->scale.image->height, 
				sw->scale.image->depth);
	XPutImage(XtDisplay(w), *pixmap, sw->scale.gc, sw->scale.image,
		  0, 0, 0, 0, sw->scale.image->width, sw->scale.image->height);
	*type = XA_PIXMAP;
	*value = (XtPointer) pixmap;
	*length = 1;
	*format = 32;
	success = True;
    } else {
	/* Xt will always respond to selection requests for the TIMESTAMP
	   target, so we can pass a bogus time to XmuConvertStandardSelection.
	   In addition to the targets provided by XmuConvertStandardSelection,
	   Xt converts MULTIPLE, and we convert PIXMAP and BITMAP.
	 */
	success = XmuConvertStandardSelection(w, 0L, selection, target,
					      type, value, length, format);
	if (success && *target == XA_TARGETS(XtDisplay(w))) {
	    Atom* tmp;
	    tmp = (Atom *) XtRealloc(*value, (*length + 3) * sizeof(Atom));
	    tmp[(*length)++] = XInternAtom(XtDisplay(w), "MULTIPLE", False);
	    tmp[(*length)++] = XA_PIXMAP;
	    tmp[(*length)++] = XA_BITMAP;
	    *value = (XtPointer) tmp;
	}
    }
    return success;
}

void SWGrabSelection(w, time)
    Widget w;
    Time time;
{
    (void) XtOwnSelection(w, XA_PRIMARY, time, ConvertSelection, NULL, NULL);
}


/* ARGSUSED */
static void
SelectionCallback(w, client_data, selection, type, value, length, format)
    Widget w;
    XtPointer client_data;	/* unused */
    Atom *selection, *type;
    XtPointer value;
    unsigned long *length;
    int *format;
{

    if  (*type == XA_PIXMAP) {
	Pixmap *pixmap;
	XImage *image;
	Window root;
	int x, y;
	unsigned int width, height, border_width, depth;

	pixmap = (Pixmap *) value;
	XGetGeometry(XtDisplay(w), *pixmap, &root, &x, &y,
		     &width, &height, &border_width, &depth);
	image = XGetImage(XtDisplay(w), *pixmap, 0, 0, width, height, 
			  AllPlanes, ZPixmap);
	SWAutoscale(w);
	SWSetImage(w, image);
	XtFree(value);
	XDestroyImage(image);
    }
}

void SWRequestSelection(w, time)
    Widget w;
    Time time;
{
    XtGetSelectionValue(w, XA_PRIMARY, XA_PIXMAP, SelectionCallback, NULL,
			time);
}
