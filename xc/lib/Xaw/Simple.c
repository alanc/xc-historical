#ifndef lint
static char Xrcsid[] = "$XConsortium: Simple.c,v 1.18 89/03/30 16:54:36 jim Exp $";
#endif /* lint */

/* Copyright	Massachusetts Institute of Technology	1987 */

#include <X11/IntrinsicP.h>
#include <X11/copyright.h>
#include <X11/StringDefs.h>
#include <X11/SimpleP.h>
#include <X11/Vendor.h>		/* hack; force Xaw/Vendor.o to be loaded */

#define UnspecifiedPixmap 2	/* %%% should be NULL, according to the spec */

static XtResource resources[] = {
#define offset(field) XtOffset(SimpleWidget, simple.field)
  {XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
     offset(cursor), XtRImmediate, (caddr_t) None},
  {XtNinsensitiveBorder, XtCInsensitive, XtRPixmap, sizeof(Pixmap),
     offset(insensitive_border), XtRImmediate, (caddr_t) NULL}
#undef offset
};

static void ClassInitialize(), ClassPartInitialize(), Realize();
static Boolean SetValues(), ChangeSensitive();

SimpleClassRec simpleClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	"Simple",
    /* widget_size		*/	sizeof(SimpleRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	ClassPartInitialize,
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
    /* resize			*/	NULL,
    /* expose			*/	NULL,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* simple fields */
    /* change_sensitive		*/	ChangeSensitive
  }
};

WidgetClass simpleWidgetClass = (WidgetClass)&simpleClassRec;

static void ClassInitialize()
{
    /* this silliness causes the linker to include the VendorShell
     * module from Xaw, rather than the one from Xt.  Better
     * optimizers require bigger hacks to hide the fact that this
     * is a no-op.
     */
    if (vendorShellWidgetClass == NULL)
	simpleClassRec.simple_class.change_sensitive = ChangeSensitive;
}

static void ClassPartInitialize(class)
    WidgetClass class;
{
    register SimpleWidgetClass c = (SimpleWidgetClass)class;

    if (c->simple_class.change_sensitive == XtInheritChangeSensitive)
	c->simple_class.change_sensitive = ChangeSensitive;
}


/* ARGSUSED */
static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    Pixmap border_pixmap;

    if (!XtIsSensitive(w)) {
	/* change border to gray; have to remember the old one,
	 * so XtDestroyWidget deletes the proper one */
	if (((SimpleWidget)w)->simple.insensitive_border == NULL)
	    ((SimpleWidget)w)->simple.insensitive_border =
		XmuCreateStippledPixmap(XtScreen(w),
					w->core.border_pixel, 
					w->core.background_pixel,
					w->core.depth);
        border_pixmap = w->core.border_pixmap;
	attributes->border_pixmap =
	  w->core.border_pixmap = ((SimpleWidget)w)->simple.insensitive_border;

	*valueMask |= CWBorderPixmap;
	*valueMask &= ~CWBorderPixel;
    }

    if ((attributes->cursor = ((SimpleWidget)w)->simple.cursor) != None)
	*valueMask |= CWCursor;

    XtCreateWindow( w, (unsigned int)InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes );

    if (!XtIsSensitive(w))
	w->core.border_pixmap = border_pixmap;

}


/* ARGSUSED */
static Boolean SetValues(current, request, new)
    Widget current, request, new;
{
    SimpleWidget s_old = (SimpleWidget) current;
    SimpleWidget s_new = (SimpleWidget) new;

    if ( XtIsSensitive(current) != XtIsSensitive(new) )
	ChangeSensitive( new );

    if ( (s_old->simple.cursor != s_new->simple.cursor) && XtIsRealized(new))
        XDefineCursor(XtDisplay(new), XtWindow(new), s_new->simple.cursor);

    return False;   
}


static Boolean ChangeSensitive(w)
    register Widget w;
{
    if (XtIsRealized(w)) {
	if (XtIsSensitive(w))
	    if (w->core.border_pixmap != UnspecifiedPixmap)
		XSetWindowBorderPixmap( XtDisplay(w), XtWindow(w),
				        w->core.border_pixmap );
	    else
		XSetWindowBorder( XtDisplay(w), XtWindow(w), 
				  w->core.border_pixel );
	else {
	    if (((SimpleWidget)w)->simple.insensitive_border == NULL)
		((SimpleWidget)w)->simple.insensitive_border =
		    XmuCreateStippledPixmap(XtScreen(w),
					    w->core.border_pixel, 
					    w->core.background_pixel,
					    w->core.depth);
	    XSetWindowBorderPixmap( XtDisplay(w), XtWindow(w),
				    ((SimpleWidget)w)->
				        simple.insensitive_border );
	}
    }
    return False;
}

