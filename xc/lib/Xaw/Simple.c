#ifndef lint
static char rcsid[] = "$Header: Simple.c,v 1.1 88/01/28 07:50:22 swick Locked $";
#endif lint

/* Copyright	Massachusetts Institute of Technology	1987 */

#include <X/copyright.h>
#include <X/Intrinsic.h>
#include <X/Atoms.h>
#include "SimpleP.h"

#define IsSensitive(w) ((w)->core.sensitive && (w)->core.ancestor_sensitive)

static Cursor defaultCursor = None;
static Pixmap defaultPixmap = NULL;

static XtResource resources[] = {
#define offset(field) XtOffset(SimpleWidget, simple.field)
  {XtNcursor, XtCCursor, XrmRCursor, sizeof(Cursor),
     offset(cursor), XrmRCursor, (caddr_t)&defaultCursor},
  {XtNinsensitiveBorder, XtCInsensitive, XrmRPixmap, sizeof(Pixmap),
     offset(insensitive_border), XrmRPixmap, (caddr_t)&defaultPixmap}
#undef offset
};

static void ClassInitialize(), Realize();
static Boolean SetValues(), ChangeSensitive();

SimpleClassRec simpleClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &widgetClassRec,
    /* class_name       */      "Simple",
    /* widget_size      */      sizeof(SimpleRec),
    /* class_initialize */      ClassInitialize,
    /* class_inited     */      FALSE,
    /* initialize       */      NULL,
    /* realize          */      Realize,
    /* actions          */      NULL,
    /* num_actions      */      0,
    /* resources        */      resources,
    /* num_ resource    */      XtNumber(resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      NULL,
    /* resize           */      NULL,
    /* expose           */      NULL,
    /* set_values       */      SetValues,
    /* accept_focus     */      NULL,
    /* callback_private */      NULL,
    /* reserved_private */      NULL
  },
  { /* simple fields */
    /* change_sensitive */	ChangeSensitive
  }
};

WidgetClass simpleWidgetClass = (WidgetClass)&simpleClassRec;

static void ClassInitialize()
{
    extern void _XtCvtStringToCursor();
    XrmRegisterTypeConverter(XrmRString, XtRCursor, _XtCvtStringToCursor);
}


/* ARGSUSED */
static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    Pixmap border_pixmap;

    if (!IsSensitive(w)) {
	/* change border to gray; have to remember the old one,
	 * so XtDestroyWidget deletes the proper one */
	if (!((SimpleWidget)w)->simple.insensitive_border)
	    ((SimpleWidget)w)->simple.insensitive_border =
		XtGrayPixmap( XtScreen(w) );
        border_pixmap = w->core.border_pixmap;
	attributes->border_pixmap =
	  w->core.border_pixmap = ((SimpleWidget)w)->simple.insensitive_border;

	*valueMask |= CWBorderPixmap;
	*valueMask &= ~CWBorderPixel;
    }

    if ((attributes->cursor = ((SimpleWidget)w)->simple.cursor) != None) {
	*valueMask |= CWCursor;

    XtCreateWindow( w, (unsigned int)InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes );

    if (!IsSensitive(w))
	w->core.border_pixmap = border_pixmap;

}


/* ARGSUSED */
static Boolean SetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    if ((current->core.sensitive != new->core.sensitive ||
	 current->core.ancestor_sensitive != new->core.ancestor_sensitive))
	ChangeSensitive( new );

    return False;
}


static Boolean ChangeSensitive(w)
    register Widget w;
{
    if (XtIsRealized(w)) {
	if (IsSensitive(w))
	    if (w->core.border_pixmap)
		XSetWindowBorderPixmap( XtDisplay(w), XtWindow(w),
				        w->core.border_pixmap );
	    else
		XSetWindowBorder( XtDisplay(w), XtWindow(w), 
				  w->core.border_pixel );
	else {
	    if (!((SimpleWidget)w)->simple.insensitive_border)
		((SimpleWidget)w)->simple.insensitive_border =
		    XtGrayPixmap( XtScreen(w) );
	    XSetWindowBorderPixmap( XtDisplay(w), XtWindow(w),
				    ((SimpleWidget)w)->
				        simple.insensitive_border );
	}
    }
    return False;
}

