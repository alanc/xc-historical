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
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct {
    Core	core;
    Label	label;
} LabelData, *LabelWidget;

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

extern void Initialize();
extern void Realize();
extern void Resize();
extern void Redisplay();
extern void SetValues();

LabelWidgetClassData labelWidgetClassData = {
/* core fields */	
    /* superclass	*/	(WidgetClass) &widgetClassData,
    /* class_name	*/	"Label",
    /* size		*/	sizeof(LabelWidgetClassData),
    /* initialize	*/	Initialize,
    /* realize		*/	Realize,
    /* actions		*/	NULL,
    /* resources	*/	resources,
    /* resource_count	*/	XtNumber(resources),
    /* xrm_extra	*/	NULL,
    /* xrm_class	*/	NULLQUARK,
    /* visible_interest	*/	FALSE,
    /* destroy		*/	NULL,
    /* resize		*/	Resize,
    /* expose		*/	Redisplay,
    /* set_values	*/	SetValues,
    /* accepts_focus	*/	FALSE,
    /* accept_focus	*/	NULL,
};

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

extern void CvtStringToJustify();

static XrmQuark	XrmQEleft;
static XrmQuark	XrmQEcenter;
static XrmQuark	XrmQEright;

static void ClassInitialize(w)
 Widget w;
{

    XrmQEleft   = XrmAtomToQuark("left");
    XrmQEcenter = XrmAtomToQuark("center");
    XrmQEright  = XrmAtomToQuark("right");

    XtRegisterTypeConverter(XrmRString, XtRJustify, CvtStringToJustify);
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

static void GetGC(lw)
    LabelWidget lw;
{
    XGCValues	values;

    values.foreground	= lw->label.foreground;
    values.font		= lw->label.font->fid;

    lw->label.gc = XtGetGC(lw->core.display, lw->core.window,
    	GCForeground | GCFont, &values);
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

    GetGC(lw);

    SetTextWidthAndHeight(lw);
    Resize(lw);

    if (lw->core.width == 0)
        lw->core.width = lw->label.labelWidth + 2 * lw->label.internalWidth;
    if (lw->core.height == 0)
        lw->core.height = lw->label.labelHeight + 2 * lw->label.internalHeight;

/* labels want exposure compression !!! */
/*     lw->core.compress_expose = TRUE; */

} /* Initialize */


void Realize(w, valueMask, attributes)
    register Widget w;
    Mask valueMask;
    XSetWindowAttributes *attributes;
{
    valueMask |= CWBitGravity;
    switch (((LabelWidget)w)->label.justify) {
	case XtjustifyLeft:	attributes->bit_gravity = WestGravity;   break;
	case XtjustifyCenter:	attributes->bit_gravity = CenterGravity; break;
	case XtjustifyRight:	attributes->bit_gravity = EastGravity;   break;
    }
    
    w->core.window =
	  XCreateWindow(
		w->core.display, w->core.parent->core.window,
		w->core.x, w->core.y,
		w->core.width, w->core.height, w->core.border_width,
		0, InputOutput, (Visual *)CopyFromParent,
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
	w->core.display, w->core.window, lw->label.gc,
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

void SetValues(old, new)
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

    if (oldlw->label.foreground != newlw->label.foreground
	|| oldlw->label.font->fid != newlw->label.font->fid) {

	XtDestroyGC(oldlw->label.gc);
	GetGC(newlw);
    }

    XClearWindow(oldlw->core.display, oldlw->core.window);
    *oldlw = *newlw;
    Redisplay(oldlw);
}
