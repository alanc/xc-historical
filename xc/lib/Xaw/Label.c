#ifndef lint
static char *sccsid = "@(#)Label.c	1.15	2/25/87";
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
#include "Intrinsic.h"
#include "Label.h"
#include "LabelPrivate.h"
#include "Atoms.h"


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

#define XtRjustify		"Justify"

static Resource resources[] = {
    {XtNforeground, XtCForeground, XrmRPixel, sizeof(Pixel),
	Offset(LabelWidget, label.foreground), XrmRString, "Black"},
    {XtNfont,  XtCFont, XrmRFontStruct, sizeof(XFontStruct *),
	Offset(LabelWidget, label.font),XrmRString, "Fixed"},
    {XtNlabel,  XtCLabel, XrmRString, sizeof(String),
	Offset(LabelWidget, label.label), XrmRString, NULL},
    {XtNjustify, XtCJustify, XtRJustify, sizeof(XtJustify),
	Offset(LabelWidget, label.justify), XrmRString, "Center"},
    {XtNinternalWidth, XtCWidth, XrmRInt,  sizeof(Dimension),
	Offset(LabelWidget, label.internalWidth),XrmRString, "4"},
    {XtNinternalHeight, XtCHeight, XrmRInt, sizeof(Dimension),
	Offset(LabelWidget, label.internalHeight),XrmRString, "2"},
};

static void Initialize();
static void Realize();
static void Resize();
static void Redisplay();
static void SetValues();
static void ClassInitialize();

LabelClassRec labelClassRec = {
/* core fields */	
    /* superclass	*/	(WidgetClass) &widgetClassRec,
    /* class_name	*/	"Label",
    /* size		*/	sizeof(LabelRec),
    /* class init proc  */      ClassInitialize,
    /* class init'ed    */	FALSE,
    /* initialize	*/	Initialize,
    /* realize		*/	Realize,
    /* actions		*/	NULL,
				0,
    /* resources	*/	resources,
    /* resource_count	*/	XtNumber(resources),

    /* xrm_class	*/	NULLQUARK,
				FALSE,
				FALSE,
    /* visible_interest	*/	FALSE,
    /* destroy		*/	NULL,
    /* resize		*/	Resize,
    /* expose		*/	Redisplay,
    /* set_values	*/	SetValues,
    /* accepts_focus	*/	FALSE,
    /* accept_focus	*/	NULL,
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

static void ClassInitialize(w)
 Widget w;
{

    XrmQEleft   = XrmAtomToQuark("left");
    XrmQEcenter = XrmAtomToQuark("center");
    XrmQEright  = XrmAtomToQuark("right");

    XrmRegisterTypeConverter(XrmRString, XtRJustify, CvtStringToJustify);
} /* ClassInitialize */

static void CvtStringToJustify(display, fromVal, toVal)
    Display     *display;
    XrmValue    fromVal;
    XrmValue    *toVal;
{
    static XtJustify	e;
    XrmQuark    q;
    char	*s = (char *) fromVal.addr;
    char        lowerName[1000];
    int		i;

    if (s == NULL) return;

    for (i=0; i<=strlen(s); i++) {
	lowerName[i] = tolower((char *)fromVal.addr[i]);
    }

    q = XrmAtomToQuark(lowerName);

    (*toVal).size = sizeof(XtJustify);
    (*toVal).addr = (caddr_t) &e;

    if (q == XrmQEleft)   { e = XtjustifyLeft;   return; }
    if (q == XrmQEcenter) { e = XtjustifyCenter; return; }
    if (q == XrmQEright)  { e = XtjustifyRight;  return; }

    (*toVal).size = 0;
    (*toVal).addr = NULL;
};

/*
 * Calculate width and height of displayed text in pixels
 */

static void SetTextWidthAndHeight(lw)
    LabelWidget lw;
{
    register XFontStruct	*fs = lw->label.font;

    lw->label.labelLen = XtStrlen(lw->label.label);
    lw->label.labelHeight = fs->max_bounds.ascent + fs->max_bounds.descent;
    lw->label.labelWidth = XTextWidth(
	fs, lw->label.label, lw->label.labelLen);
}

static void GetnormalGC(lw)
    LabelWidget lw;
{
    XGCValues	values;

    values.foreground	= lw->label.foreground;
    values.font		= lw->label.font->fid;

    lw->label.normalGC = XtGetGC(lw, GCForeground | GCFont, &values);
}

static void GetgrayGC(lw)
    LabelWidget lw;
{
    XGCValues	values;
    
    lw->label.grayPixmap = XtGrayPixmap(XtScreen((Widget)lw));

    values.foreground	= lw->label.foreground;
    values.font		= lw->label.font->fid;
    values.tile       = lw->label.grayPixmap;
    values.fill_style = FillTiled;

    lw->label.grayGC = XtGetGC(lw, 
				 GCForeground | GCFont | GCTile | GCFillStyle, 
				 &values);
}

static void Initialize(w)
 Widget w;
{
    LabelWidget lw = (LabelWidget) w;

    if (lw->label.label == NULL) {
	unsigned int	len = strlen(lw->core.name);
	lw->label.label = XtMalloc(len+1);
	(void) strcpy(lw->label.label, lw->core.name);
    }

    GetnormalGC(lw);
    GetgrayGC(lw);

    SetTextWidthAndHeight(lw);
    Resize(lw);

    if (lw->core.width == 0)
        lw->core.width = lw->label.labelWidth + 2 * lw->label.internalWidth;
    if (lw->core.height == 0)
        lw->core.height = lw->label.labelHeight + 2 * lw->label.internalHeight;

/* labels want exposure compression !!! */
/*     lw->core.compress_expose = TRUE; */

    lw->label.displaySensitive = FALSE;

} /* Initialize */


static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask valueMask;
    XSetWindowAttributes *attributes;
{
  LabelWidget lw = (LabelWidget)w;

    valueMask |= CWBitGravity;
    switch (((LabelWidget)w)->label.justify) {
	case XtjustifyLeft:	attributes->bit_gravity = WestGravity;   break;
	case XtjustifyCenter:	attributes->bit_gravity = CenterGravity; break;
	case XtjustifyRight:	attributes->bit_gravity = EastGravity;   break;
    }
    
    if (!(w->core.sensitive))
      {
	  /* change border to gray */
	lw->core.border_pixmap = lw->label.grayPixmap;
	attributes->border_pixmap = lw->label.grayPixmap;
	valueMask |= CWBorderPixmap;
	lw->label.displaySensitive = TRUE;
      }
    

    w->core.window =
	  XCreateWindow(
		XtDisplay(w), w->core.parent->core.window,
		w->core.x, w->core.y,
		w->core.width, w->core.height, w->core.border_width,
		w->core.depth, InputOutput, (Visual *)CopyFromParent,	
		valueMask, attributes);
} /* Realize */



/*
 * Repaint the widget window
 */

static void Redisplay(w)
    Widget w;
{
   LabelWidget lw = (LabelWidget) w;

   XDrawString(
	XtDisplay(w), XtWindow(w), lw->label.normalGC,
	lw->label.labelX, lw->label.labelY,
	lw->label.label, lw->label.labelLen);
}


static void Resize(lw)
    LabelWidget	lw;
{
    switch (lw->label.justify) {

	case XtjustifyLeft   :
	    lw->label.labelX = lw->label.internalWidth;
	    break;

	case XtjustifyRight  :
	    lw->label.labelX = lw->core.width -
		(lw->label.labelWidth + lw->label.internalWidth);
	    break;

	case XtjustifyCenter :
	    lw->label.labelX = (lw->core.width - lw->label.labelWidth) / 2;
	    break;
    }
    if (lw->label.labelX < 0) lw->label.labelX = 0;
    lw->label.labelY = (lw->core.height - lw->label.labelHeight) / 2
	+ lw->label.font->max_bounds.ascent;
}

/*
 * Set specified arguments into widget
 */

static void SetValues(old, new)
    Widget old, new;
{
    LabelWidget oldlw = (LabelWidget) old;
    LabelWidget newlw = (LabelWidget) new;
    WidgetGeometry	reqGeo;

    if (newlw->label.label == NULL) {
	/* the string will be copied below... */
	newlw->label.label = newlw->core.name;
    }

    if ((oldlw->label.label != newlw->label.label)
	|| (oldlw->label.font != newlw->label.font)
	|| (oldlw->label.justify != newlw->label.justify)) {

	SetTextWidthAndHeight(newlw);

	}

    /* note that there is no way to change the label and force the window */
    /* to keep it's current size (and possibly clip the text) perhaps we */
    /* should make the user set width and height to 0 when they set the */
    /* label if they want the label to recompute size based on the new */
    /* label? */
    if (oldlw->label.label != newlw->label.label) {
        if (newlw->label.label != NULL) {
	    newlw->label.label = strcpy(
	        XtMalloc((unsigned) newlw->label.labelLen + 1),
		newlw->label.label);
	}
	XtFree ((char *) oldlw->label.label);
    }

    /* calculate the window size */
    if (oldlw->core.width == newlw->core.width)
	newlw->core.width =
	    newlw->label.labelWidth +2*newlw->label.internalWidth;

    if (oldlw->core.height == newlw->core.height)
	newlw->core.height =
	    newlw->label.labelHeight + 2*newlw->label.internalHeight;

    reqGeo.request_mode = NULL;

    if (oldlw->core.x != newlw->core.x) {
	reqGeo.request_mode |= CWX;
	reqGeo.x = newlw->core.x;
    }
    if (oldlw->core.y != newlw->core.y) {
	reqGeo.request_mode |= CWY;
	reqGeo.y = newlw->core.y;
    }
    if (oldlw->core.width != newlw->core.width) {
	reqGeo.request_mode |= CWWidth;
	reqGeo.width = newlw->core.width;
    }
    if (oldlw->core.height != newlw->core.height) {
	reqGeo.request_mode |= CWHeight;
	reqGeo.height = newlw->core.height;
    }
    if (oldlw->core.border_width != newlw->core.border_width) {
	reqGeo.request_mode |= CWBorderWidth;
	reqGeo.border_width = newlw->core.border_width;
    }

    if (reqGeo.request_mode != NULL) {
	/* this will automatically call "Resize" if it succeeds */
	if (XtMakeGeometryRequest(newlw, reqGeo, NULL) != XtgeometryYes) {
	    /* punt, undo requested change */
	    newlw->core.x = oldlw->core.x;
	    newlw->core.y = oldlw->core.y;
	    newlw->core.width = oldlw->core.width;
	    newlw->core.height = oldlw->core.height;
	    newlw->core.border_width = oldlw->core.border_width;
	}
    }

    if (newlw->core.depth != oldlw->core.depth) {
	XtWarning("SetValues: Attempt to change existing widget depth.");
	newlw->core.depth = oldlw->core.depth;
    }

    if ((oldlw->core.background_pixel != newlw->core.background_pixel)
	|| (oldlw->core.border_pixel != newlw->core.border_pixel)) {

	Mask valueMask = 0;
	XSetWindowAttributes attributes;

	if (oldlw->core.background_pixel != newlw->core.background_pixel) {
	    valueMask |= CWBackPixel;
	    attributes.background_pixel = newlw->core.background_pixel;
	}
	if (oldlw->core.border_pixel != newlw->core.border_pixel) {
	    valueMask |= CWBorderPixel;
	    attributes.border_pixel = newlw->core.border_pixel;
	}
	XChangeWindowAttributes(
	    XtDisplay(newlw), newlw->core.window, valueMask, &attributes);
    }

    if (oldlw->core.sensitive != newlw->core.sensitive) {
	XtWarning("Setting Label sensitivity not implemented.");
    }

    if (oldlw->label.foreground != newlw->label.foreground
	|| oldlw->label.font->fid != newlw->label.font->fid) {

	XtDestroyGC(oldlw->label.normalGC);
	GetnormalGC(newlw);
	GetgrayGC(newlw);
    }

    if ((oldlw->label.internalWidth != newlw->label.internalWidth)
        || (oldlw->label.internalHeight != newlw->label.internalHeight)) {
	Resize(newlw);
    }

    XClearWindow(XtDisplay(oldlw), XtWindow(oldlw));
    *oldlw = *newlw;
    Redisplay(oldlw);
}
