#ifndef lint
static char rcsid[] = "$Header: Label.c,v 1.32 88/01/22 13:45:12 swick Locked $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/*
 * Label.c - Label widget
 *
 * Author:      Charles Haynes
 *              Digital Equipment Corporation
 *              Western Research Laboratory
 * Date:        Sat Jan 24 1987
 *
 * Converted to classing toolkit on Wed Aug 26 by Charles Haynes
 */

#define XtStrlen(s)	((s) ? strlen(s) : 0)

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <X/Intrinsic.h>
#include <X/Atoms.h>
#include <X/Label.h>
#include "LabelP.h"

#define IsSensitive(w)	((w)->core.sensitive && (w)->core.ancestor_sensitive)

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

#define XtRjustify		"Justify"

static XtResource resources[] = {
    {XtNforeground, XtCForeground, XrmRPixel, sizeof(Pixel),
	XtOffset(LabelWidget, label.foreground), XrmRString, "Black"},
    {XtNfont,  XtCFont, XrmRFontStruct, sizeof(XFontStruct *),
	XtOffset(LabelWidget, label.font),XrmRString, "Fixed"},
    {XtNlabel,  XtCLabel, XrmRString, sizeof(String),
	XtOffset(LabelWidget, label.label), XrmRString, NULL},
    {XtNjustify, XtCJustify, XtRJustify, sizeof(XtJustify),
	XtOffset(LabelWidget, label.justify), XrmRString, "Center"},
    {XtNinternalWidth, XtCWidth, XrmRInt,  sizeof(Dimension),
	XtOffset(LabelWidget, label.internal_width),XrmRString, "4"},
    {XtNinternalHeight, XtCHeight, XrmRInt, sizeof(Dimension),
	XtOffset(LabelWidget, label.internal_height),XrmRString, "2"},
};

static void Initialize();
static void Realize();
static void Resize();
static void Redisplay();
static Boolean SetValues();
static void ClassInitialize();

LabelClassRec labelClassRec = {
  {
/* core_class fields */	
    /* superclass	  */	(WidgetClass) &widgetClassRec,
    /* class_name	  */	"Label",
    /* widget_size	  */	sizeof(LabelRec),
    /* class_initialize   */    ClassInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* realize		  */	Realize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	TRUE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	Resize,
    /* expose		  */	Redisplay,
    /* set_values	  */	SetValues,
    /* accept_focus	  */	NULL,
    /* callback_private   */	NULL,
    /* reserved_private   */	NULL,
  }
};
WidgetClass labelWidgetClass = (WidgetClass)&labelClassRec;
/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void CvtStringToJustify();

static XrmQuark	XrmQEleft;
static XrmQuark	XrmQEcenter;
static XrmQuark	XrmQEright;

static void ClassInitialize()
{

    XrmQEleft   = XrmAtomToQuark("left");
    XrmQEcenter = XrmAtomToQuark("center");
    XrmQEright  = XrmAtomToQuark("right");

    XrmRegisterTypeConverter(XrmRString, XtRJustify, CvtStringToJustify);
} /* ClassInitialize */

/* ARGSUSED */
static void CvtStringToJustify(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static XtJustify	e;
    XrmQuark    q;
    char	*s = (char *) fromVal->addr;
    char        lowerName[1000];
    int		i;

    if (s == NULL) return;

    for (i=0; i<=strlen(s); i++) {
        char c = s[i];
	lowerName[i] = isupper(c) ? (char) tolower(c) : c;
    }

    q = XrmAtomToQuark(lowerName);

    toVal->size = sizeof(XtJustify);
    toVal->addr = (caddr_t) &e;

    if (q == XrmQEleft)   { e = XtJustifyLeft;   return; }
    if (q == XrmQEcenter) { e = XtJustifyCenter; return; }
    if (q == XrmQEright)  { e = XtJustifyRight;  return; }

    toVal->size = 0;
    toVal->addr = NULL;
};

/*
 * Calculate width and height of displayed text in pixels
 */

static void SetTextWidthAndHeight(lw)
    LabelWidget lw;
{
    register XFontStruct	*fs = lw->label.font;

    lw->label.label_len = XtStrlen(lw->label.label);
    lw->label.label_height = fs->max_bounds.ascent + fs->max_bounds.descent;
    lw->label.label_width = XTextWidth(
	fs, lw->label.label, (int) lw->label.label_len);
}

static void GetnormalGC(lw)
    LabelWidget lw;
{
    XGCValues	values;

    values.foreground	= lw->label.foreground;
    values.font		= lw->label.font->fid;

    lw->label.normal_GC = XtGetGC(
	(Widget)lw,
	(unsigned) GCForeground | GCFont,
	&values);
}

static void GetgrayGC(lw)
    LabelWidget lw;
{
    XGCValues	values;
    
    lw->label.gray_pixmap = XtGrayPixmap(XtScreen((Widget)lw));

    values.foreground	= lw->label.foreground;
    values.font		= lw->label.font->fid;
    values.tile       = lw->label.gray_pixmap;
    values.fill_style = FillTiled;

    lw->label.gray_GC = XtGetGC(
	(Widget)lw, 
	(unsigned) GCForeground | GCFont | GCTile | GCFillStyle, 
	&values);
}

static void Initialize(request, new, args, num_args)
 Widget request, new;
 ArgList args;
 Cardinal *num_args;
{
    LabelWidget lw = (LabelWidget) new;

    if (lw->label.label == NULL) 
        lw->label.label = lw->core.name;
    else {
        lw->label.label = strcpy( XtMalloc( strlen(lw->label.label) + 1 ),
				  lw->label.label );
    }

    GetnormalGC(lw);
    GetgrayGC(lw);

    SetTextWidthAndHeight(lw);

    if (lw->core.width == 0)
        lw->core.width = lw->label.label_width + 2 * lw->label.internal_width;
    if (lw->core.height == 0)
        lw->core.height = lw->label.label_height + 2*lw->label.internal_height;

    Resize((Widget)lw);

} /* Initialize */


static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    LabelWidget lw = (LabelWidget)w;
    Pixmap border_pixmap;

    *valueMask |= CWBitGravity;
    switch (lw->label.justify) {
	case XtJustifyLeft:	attributes->bit_gravity = WestGravity;   break;
	case XtJustifyCenter:	attributes->bit_gravity = CenterGravity; break;
	case XtJustifyRight:	attributes->bit_gravity = EastGravity;   break;
    }
    
    if (!IsSensitive(w))
      {
	/* change border to gray; have to remember the old one, so
	 * XtDestroyWidget deletes the proper one */
        border_pixmap = w->core.border_pixmap;
	w->core.border_pixmap = lw->label.gray_pixmap;
	attributes->border_pixmap = lw->label.gray_pixmap;
	*valueMask |= CWBorderPixmap;
	*valueMask &= ~CWBorderPixel;
      }
    

    XtCreateWindow( w, (unsigned int)InputOutput, (Visual *)CopyFromParent,
		    *valueMask, attributes );

    if (!IsSensitive(w))
	w->core.border_pixmap = border_pixmap;

} /* Realize */



/*
 * Repaint the widget window
 */

/* ARGSUSED */
static void Redisplay(w, event)
    Widget w;
    XEvent *event;
{
   LabelWidget lw = (LabelWidget) w;

   XDrawString(
	XtDisplay(w), XtWindow(w),
	IsSensitive(lw) ? lw->label.normal_GC : lw->label.gray_GC,
	lw->label.label_x, lw->label.label_y,
	lw->label.label, (int) lw->label.label_len);
}


static void Resize(w)
    Widget w;
{
    LabelWidget lw = (LabelWidget) w;

    switch (lw->label.justify) {

	case XtJustifyLeft   :
	    lw->label.label_x = lw->label.internal_width;
	    break;

	case XtJustifyRight  :
	    lw->label.label_x = lw->core.width -
		(lw->label.label_width + lw->label.internal_width);
	    break;

	case XtJustifyCenter :
	    lw->label.label_x = (lw->core.width - lw->label.label_width) / 2;
	    break;
    }
    if (lw->label.label_x < 0) lw->label.label_x = 0;
    lw->label.label_y = (lw->core.height - lw->label.label_height) / 2
	+ lw->label.font->max_bounds.ascent;
}

/*
 * Set specified arguments into widget
 */

static Boolean SetValues(current, request, new, last)
    Widget current, request, new;
    Boolean last;
{
    LabelWidget curlw = (LabelWidget) current;
    LabelWidget reqlw = (LabelWidget) request;
    LabelWidget newlw = (LabelWidget) new;
    Boolean was_resized;

    if (newlw->label.label == NULL) {
	newlw->label.label = newlw->core.name;
    }

    if ((curlw->label.label != newlw->label.label)
	|| (curlw->label.font != newlw->label.font)
	|| (curlw->label.justify != newlw->label.justify)) {

	SetTextWidthAndHeight(newlw);

	}

    /* note that there is no way to change the label and force the window */
    /* to keep it's current size (and possibly clip the text) perhaps we */
    /* should make the user set width and height to 0 when they set the */
    /* label if they want the label to recompute size based on the new */
    /* label? */
    if (curlw->label.label != newlw->label.label) {
        if (curlw->label.label != curlw->core.name)
	    XtFree( (char *)curlw->label.label );

	if (newlw->label.label != newlw->core.name) {
	    newlw->label.label = strcpy(
	        XtMalloc((unsigned) newlw->label.label_len + 1),
		newlw->label.label);
	}
    }

    /* calculate the window size */
    if (curlw->core.width == newlw->core.width)
	newlw->core.width =
	    newlw->label.label_width +2*newlw->label.internal_width;

    if (curlw->core.height == newlw->core.height)
	newlw->core.height =
	    newlw->label.label_height + 2*newlw->label.internal_height;

    was_resized = (XtSetValuesGeometryRequest( curlw, newlw,
					       (XtWidgetGeometry *)NULL )
		   == XtGeometryYes);

    if (newlw->core.depth != curlw->core.depth) {
	XtWarning("SetValues: Attempt to change existing widget depth.");
	newlw->core.depth = curlw->core.depth;
    }

    if (((curlw->core.background_pixel != newlw->core.background_pixel)
	 || (curlw->core.border_pixel != newlw->core.border_pixel))
	&& XtIsRealized(newlw)) {

	Mask valueMask = 0;
	XSetWindowAttributes attributes;

	if (curlw->core.background_pixel != newlw->core.background_pixel) {
	    valueMask |= CWBackPixel;
	    attributes.background_pixel = newlw->core.background_pixel;
	}
	if (curlw->core.border_pixel != newlw->core.border_pixel) {
	    valueMask |= CWBorderPixel;
	    attributes.border_pixel = newlw->core.border_pixel;
	}
	XChangeWindowAttributes(
	    XtDisplay(newlw), newlw->core.window, valueMask, &attributes);
    }

    if ((curlw->core.sensitive != newlw->core.sensitive ||
	 curlw->core.ancestor_sensitive != newlw->core.ancestor_sensitive)
	&& XtIsRealized(newlw))
        if (IsSensitive(newlw))
	    XSetWindowBorder( XtDisplay(newlw), XtWindow(newlw), 
			      newlw->core.border_pixel );
	else
	    XSetWindowBorderPixmap(XtDisplay(newlw), XtWindow(newlw),
				   newlw->label.gray_pixmap);

    if (curlw->label.foreground != newlw->label.foreground
	|| curlw->label.font->fid != newlw->label.font->fid) {

	XtDestroyGC(curlw->label.normal_GC);
	XtDestroyGC(curlw->label.gray_GC);
	GetnormalGC(newlw);
	GetgrayGC(newlw);
    }

    if ((curlw->label.internal_width != newlw->label.internal_width)
        || (curlw->label.internal_height != newlw->label.internal_height)
	|| was_resized) {
	Resize((Widget)newlw);
    }

    return( True );		/* want Redisplay */
}
