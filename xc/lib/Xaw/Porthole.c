/*
 * $XConsortium: Porthole.c,v 1.1 90/02/28 18:07:29 jim Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
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
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <X11/IntrinsicP.h>		/* get basic toolkit stuff */
#include <X11/StringDefs.h>		/* get XtN and XtC defines */
#include <X11/Xaw/XawInit.h>		/* get Xaw initialize stuff */
#include <X11/Xaw/PortholeP.h>		/* get porthole structs */


/*
 * resources for the porthole
 */
static XtResource resources[] = {
#define poff(field) XtOffset(PortholeWidget, porthole.field)
    { XtNreportCallback, XtCReportCallback, XtRCallback, sizeof(XtPointer),
	poff(report_callbacks), XtRCallback, (XtPointer) NULL },
#undef poff
};


/*
 * widget class methods used below
 */
static void Realize();			/* set gravity and upcall */
static void Resize();			/* report new size */
static XtGeometryResult GeometryManager();  /* deal with child requests */
static void ChangeManaged();		/* somebody added a new widget */
static XtGeometryResult QueryGeometry();  /* say how big would like to be */

PortholeClassRec portholeClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &compositeClassRec,
    /* class_name		*/	"Porthole",
    /* widget_size		*/	sizeof(PortholeRec),
    /* class_initialize		*/	XawInitializeWidgetSet,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	NULL,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	Resize,
    /* expose			*/	NULL,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* composite fields */
    /* geometry_manager		*/	GeometryManager,
    /* change_managed		*/	ChangeManaged,
    /* insert_child		*/	XtInheritInsertChild,
    /* delete_child		*/	XtInheritDeleteChild,
    /* extension		*/	NULL
  },
  { /* porthole fields */
    /* ignore                   */	0
  }
};

WidgetClass portholeWidgetClass = (WidgetClass) &portholeClassRec;


/*****************************************************************************
 *                                                                           *
 *			       utility routines                              *
 *                                                                           *
 *****************************************************************************/

static void SendReport (pw, changed)
    PortholeWidget pw;
    unsigned int changed;
{
    if (pw->porthole.report_callbacks) {
	XawPannerReport prep;
	Widget child = pw->composite.children[0];

	prep.changed = changed;
	prep.slider_x = -child->core.x;	/* porthole is "inner" */
	prep.slider_y = -child->core.y;	/* child is outer since it is larger */
	prep.slider_width = pw->core.width;
	prep.slider_height = pw->core.height;
	prep.canvas_width = child->core.width;
	prep.canvas_height = child->core.height;
	XtCallCallbackList (pw, pw->porthole.report_callbacks,
			    (caddr_t) &prep);
    }
}


/*****************************************************************************
 *                                                                           *
 *			 Porthole Widget Class Methods                       *
 *                                                                           *
 *****************************************************************************/


static void Realize (gw, valueMask, attributes)
    register Widget gw;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    attributes->bit_gravity = NorthWestGravity;
    *valueMask |= CWBitGravity;

    if (gw->core.width < 1) gw->core.width = 1;
    if (gw->core.height < 1) gw->core.height = 1;
    (*(XtSuperclass(gw)->core_class.realize)) (gw, valueMask, attributes);
}


static void Resize (gw)
    Widget gw;
{
    SendReport ((PortholeWidget) gw,
		(unsigned int) (XawPRCanvasWidth | XawPRCanvasHeight));
}

static XtGeometryResult QueryGeometry (gw, intended, preferred)
    Widget gw;
    XtWidgetGeometry *intended, *preferred;
{
    register PortholeWidget pw = (PortholeWidget) gw;

    if (pw->composite.num_children > 0) {
	Widget child = pw->composite.children[0];

#define SIZEONLY (CWWidth | CWHeight)
	preferred->request_mode = SIZEONLY;
	preferred->width = child->core.width;
	preferred->height = child->core.height;

	if (((intended->request_mode & SIZEONLY) == SIZEONLY) &&
	    intended->width == preferred->width &&
	    intended->height == preferred->height)
	  return XtGeometryYes;
	else if (preferred->width == pw->core.width &&
		 preferred->height == pw->core.height)
	  return XtGeometryNo;
	else
	  return XtGeometryAlmost;
#undef SIZEONLY
    } 
    return XtGeometryNo;
}


static XtGeometryResult GeometryManager (w, req, reply)
    Widget w;
    XtWidgetGeometry *req, *reply;
{
    PortholeWidget pw = (PortholeWidget) w->core.parent;
    unsigned int changed = 0;

    /*
     * check to see if this is the right child
     */
    if (pw->composite.num_children < 1 || w != pw->composite.children[0])
      return XtGeometryNo;

    if (req->request_mode == CWBorderWidth) return XtGeometryNo;

    /*
     * XXX - need to handle query only...
     */
    *reply = *req;
    if (req->request_mode & CWBorderWidth) {
	reply->request_mode &= ~CWBorderWidth;
	reply->border_width = 0;
	return XtGeometryAlmost;
    }

    /*
     * Allow all size changes; allow location changes that remain within
     * the visible region.
     */
    if (req->request_mode & CWX && w->core.x != req->x) {
	changed |= XawPRSliderX;
	w->core.x = req->x;
    }
    if (req->request_mode & CWY && w->core.y != req->y) {
	changed |= XawPRSliderY;
	w->core.y = req->y;
    }
    if (req->request_mode & CWWidth && w->core.width != req->width) {
	changed |= XawPRSliderWidth;
	w->core.width = req->width;
    }
    if (req->request_mode & CWHeight && w->core.height != req->height) {
	changed |= XawPRSliderHeight;
	w->core.height = req->height;
    }

    SendReport (pw, changed);

    return XtGeometryYes;
}


static void ChangeManaged (gw)
    Widget gw;
{
    PortholeWidget pw = (PortholeWidget) gw;
    Widget child;

    if (pw->composite.num_children > 1) {
#include <stdio.h>
	fprintf (stderr, "Porthole:  got %d children.\n",
		 pw->composite.num_children);
	return;
    }
    

    child = pw->composite.children[0];
    child->core.border_width = 0;

    if (!XtIsRealized (gw)) {
	if (child) {
	    XtWidgetGeometry geom, retgeom;

	    geom.request_mode = 0;
	    if (pw->core.width == 0) {
		geom.width = child->core.width;
		geom.request_mode |= CWWidth;
	    }
	    if (pw->core.height == 0) {
		geom.height = child->core.height;
		geom.request_mode |= CWHeight;
	    }
	    if (geom.request_mode &&
		XtMakeGeometryRequest (gw, &geom, &retgeom) ==
		XtGeometryAlmost)
	      (void) XtMakeGeometryRequest (gw, &retgeom, NULL);
	}
    }
}
