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
 * ButtonBox.c - Button box composite widget
 *
 * Author:      Charles Haynes
 *              Digital Equipment Corporation
 *              Western Research Laboratory
 * Date:        Sat Jan 24 1987
 *
 * Converted to classing toolkit on Wed Aug 26 by Charles Haynes
 */

#include <stdio.h>
#include <string.h>
#include "Xlib.h"
#include "Xresource.h"	/* register type converter */
#include "Xrm.h"	/* quarks */
#include "Conversion.h" /* lower case proc */
#include "Intrinsic.h"
#include "Label.h"
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
    {XtNforeground, XtCColor, XrmRPixel, sizeof(Pixel),
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
extern void Reconfigure();
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
    /* reconfigure	*/	Reconfigure,
    /* expose		*/	Redisplay,
    /* set_values	*/	SetValues,
    /* accepts_focus	*/	FALSE,
    /* accept_focus	*/	NULL,
};

LabelWidgetClass labelWidgetClass = &labelWidgetClassData;

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
    XtJustify	e;
    XrmQuark    q;
    char        lowerName[1000];

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

    LowerCase((char *) fromVal.addr, lowerName);
    q = XrmAtomToQuark(lowerName);

    if (q == XrmQEleft)   { e = XtjustifyLeft;   done(&e, XtJustify); return; }
    if (q == XrmQEcenter) { e = XtjustifyCenter; done(&e, XtJustify); return; }
    if (q == XrmQEright)  { e = XtjustifyRight;  done(&e, XtJustify); return; }
};

/*
 * Calculate width and height of displayed text in pixels
 */

static void SetTextWidthAndHeight(lw)
    LabelWidget lw;
{
    register XFontStruct	*fs = lw->label.font;

    lw->label.labelHeight = fs->max_bounds.ascent + fs->max_bounds.descent;
    lw->label.labelWidth = XTextWidth(
	fs, lw->label.label, strlen(lw->label.label));
}

static void GetGC(lw)
    LabelWidget lw;
{
    XGCValues	values;

    values.foreground	= lw->label.foreground;
    values.font		= lw->label.font->fid;
    values.background	= lw->core.background_pixel;

    lw->label.gc = XtGetGC(lw->core.display, lw->core.window,
    	GCForeground | GCFont | GCBackground, &values);
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

    if (lw->core.width == 0)
        lw->core.width = lw->label.labelWidth + 2 * lw->label.internalWidth;
    if (lw->core.height == 0)
        lw->core.height = lw->label.labelHeight + 2 * lw->label.internalHeight;

/* labels want exposure compression */
/*     lw->core.compress_expose = TRUE; */

} /* Initialize */



/* ||| Should Realize just return a modified mask and attributes?  Or will some
   of the other parameters change from class to class? */
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
    Position x, y;

    /* Calculate text position within window given window width and height  */
    switch (lw->label.justify) {
	case XtjustifyLeft   :
	    x = lw->label.internalWidth;
	    break;
	case XtjustifyRight  :
	    x = lw->core.width -
		(lw->label.labelWidth + lw->label.internalWidth);
	    break;
	case XtjustifyCenter :
	    x = (lw->core.width - lw->label.labelWidth) / 2;
	    break;
    }
    if (x < 0) x = 0;
    y = (lw->core.height - lw->label.labelHeight) / 2
	+ lw->label.font->max_bounds.ascent;

   XDrawString(lw->core.display, lw->core.window, lw->label.gc, x, y,
       lw->label.label, strlen(lw->label.label));
}


static void Reconfigure()
{
/* !!! */
}

/*
 * Set specified arguments into widget
 */

void SetValues(old, new)
    Widget old, new;
{
    LabelWidget oldlw = (LabelWidget) old;
    LabelWidget newlw = (LabelWidget) new;
    unsigned int len;
    int	width, height;
    XtGeometryReturnCode reply;

    if (strcmp(oldlw->label.label, newlw->label.label) != 0
	  || oldlw->label.font != newlw->label.font) {

	len = strlen(newlw->label.label);
	newlw->label.label = strcpy(
	    XtMalloc ((unsigned) len + 1), newlw->label.label);
        XtFree ((char *) oldlw->label.label);

	/* obtain text dimensions and calculate the window size */
	SetTextWidthAndHeight(newlw);
	width = newlw->label.labelWidth + 2*newlw->label.internalWidth;
	height = newlw->label.labelHeight + 2*newlw->label.internalHeight;
	reply = XtMakeResizeRequest(newlw, width, height, &width, &height);
	if (reply == XtgeometryAlmost) {
	    reply = XtMakeResizeRequest(newlw, width, height, &width, &height);
	}
	if (reply == XtgeometryYes) {
	    newlw->core.width = width;
	    newlw->core.height = height;
	}
    }

    if (oldlw->label.foreground != newlw->label.foreground
    	|| oldlw->core.background_pixel != newlw->core.background_pixel
	|| oldlw->label.font->fid != newlw->label.font->fid) {

	GetGC(newlw);
    }
    XClearWindow(oldlw->core.display, oldlw->core.window);
    *oldlw = *newlw;
    Redisplay(oldlw);
}
