#ifndef lint
static char rcsid[] = "$Header: Conversion.c,v 1.4 87/11/23 11:08:15 swick Locked $";
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


/* Conversion.c - implementations of resource type conversion procs */

#include	"Xlibint.h"
#include	"Xutil.h"
#include	"Xresource.h"
#include	"XrmConvert.h"
#include	"Conversion.h"
#include	"Quarks.h"
#include	"cursorfont.h"
#include	<sys/file.h>
#include	<stdio.h>

#define	done(address, type) \
	{ \
	  (*toVal).size = sizeof(type); \
	  (*toVal).addr = (caddr_t) address; \
	  return; \
	}

static void _XWarningMessage(message)
    char *message;
{
    fprintf(stderr, "Warning: %s\n", message);
}

static void _XStringConversionWarning(from, toType)
    char *from, *toType;
{
    char message[1000];
    
    sprintf(message, "Cannot convert string \"%s\" to type %s", from, toType);
    _XWarningMessage(message);
}

static void CvtXColorToPixel();

static void CvtGeometryToDims();

static void CvtIntToBoolean();
static void CvtIntToFont();
static void CvtIntOrPixelToXColor();
static void CvtIntToPixel();

static void CvtStringToBoolean();
static void CvtStringToXColor();
static void CvtStringToCursor();
static void CvtStringToDisplay();
static void CvtStringToFile();
static void CvtStringToFont();
static void CvtStringToFontStruct();
static void CvtStringToGeometry();
static void CvtStringToInt();
static void CvtStringToPixel();
static void CvtStringToPixmap();

void _XLowerCase(source, dest)
    register char  *source, *dest;
{
    register char ch;

    for (; (ch = *source) != 0; source++, dest++) {
    	if ('A' <= ch && ch <= 'Z')
	    *dest = ch - 'A' + 'a';
	else
	    *dest = ch;
    }
    *dest = 0;
}


/*ARGSUSED*/
static void CvtIntToBoolean(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static int	b;

    b = (int) (*(int *)fromVal->addr != 0);
    done(&b, int);
}


/*ARGSUSED*/
static void CvtStringToBoolean(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static int	b;
    XrmQuark	q;
    char	lowerName[1000];

    _XLowerCase((char *) fromVal->addr, lowerName);
    q = XrmAtomToQuark(lowerName);

    if (q == XrmQEtrue || q == XrmQEon || q == XrmQEyes) {
        b = 1;
	done(&b, int);
    }

    /* else */
    if (q == XrmQEfalse || q == XrmQEoff || q == XrmQEno) {
        b = 0;
	done(&b, int);
    }

    /* else */
    _XStringConversionWarning((char *) fromVal->addr, "Boolean");
}


/*ARGSUSED*/
static void CvtIntOrPixelToXColor(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{    
    static XColor	c;

    c.pixel = *(int *)fromVal->addr;
    XQueryColor(DisplayOfScreen(screen), DefaultColormapOfScreen(screen), &c);
    /* !!! Need some error checking ... ||| */
    done(&c, XColor);
}


/*ARGSUSED*/
static void CvtStringToXColor(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XColor	c;
    Status		s;
    char		message[1000];

    s = XParseColor(DisplayOfScreen(screen), DefaultColormapOfScreen(screen),
			(char *)fromVal->addr, &c);
    if (s == 0) {
        _XStringConversionWarning((char *) fromVal->addr, "Color");
        return;
    }

    s = XAllocColor(DisplayOfScreen(screen), DefaultColormapOfScreen(screen), &c);
    if (s == 0) {
	sprintf(message, "Cannot allocate colormap entry for %s",
		(char *) fromVal->addr);
	_XWarningMessage(message);
        return;
    }
    done(&c, XColor);
}


/*ARGSUSED*/
static void CvtStringToCursor(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
#define XtNumber(array)	(sizeof(array)/sizeof(array[0]))

    static struct _CursorName {
	char		*name;
	unsigned int	shape;
    } cursor_names[] = {
			{"X_cursor",		XC_X_cursor},
			{"arrow",		XC_arrow},
			{"based_arrow_down",	XC_based_arrow_down},
			{"based_arrow_up",	XC_based_arrow_up},
			{"boat",		XC_boat},
			{"bogosity",		XC_bogosity},
			{"bottom_left_corner",	XC_bottom_left_corner},
			{"bottom_right_corner",	XC_bottom_right_corner},
			{"bottom_side",		XC_bottom_side},
			{"bottom_tee",		XC_bottom_tee},
			{"box_spiral",		XC_box_spiral},
			{"center_ptr",		XC_center_ptr},
			{"circle",		XC_circle},
			{"clock",		XC_clock},
			{"coffee_mug",		XC_coffee_mug},
			{"cross",		XC_cross},
			{"cross_reverse",	XC_cross_reverse},
			{"crosshair",		XC_crosshair},
			{"diamond_cross",	XC_diamond_cross},
			{"dot",			XC_dot},
			{"dotbox",		XC_dotbox},
			{"double_arrow",	XC_double_arrow},
			{"draft_large",		XC_draft_large},
			{"draft_small",		XC_draft_small},
			{"draped_box",		XC_draped_box},
			{"exchange",		XC_exchange},
			{"fleur",		XC_fleur},
			{"gobbler",		XC_gobbler},
			{"gumby",		XC_gumby},
			{"hand1",		XC_hand1},
			{"hand2",		XC_hand2},
			{"heart",		XC_heart},
			{"icon",		XC_icon},
			{"iron_cross",		XC_iron_cross},
			{"left_ptr",		XC_left_ptr},
			{"left_side",		XC_left_side},
			{"left_tee",		XC_left_tee},
			{"leftbutton",		XC_leftbutton},
			{"ll_angle",		XC_ll_angle},
			{"lr_angle",		XC_lr_angle},
			{"man",			XC_man},
			{"middlebutton",	XC_middlebutton},
			{"mouse",		XC_mouse},
			{"pencil",		XC_pencil},
			{"pirate",		XC_pirate},
			{"plus",		XC_plus},
			{"question_arrow",	XC_question_arrow},
			{"right_ptr",		XC_right_ptr},
			{"right_side",		XC_right_side},
			{"right_tee",		XC_right_tee},
			{"rightbutton",		XC_rightbutton},
			{"rtl_logo",		XC_rtl_logo},
			{"RTL_logo",		XC_rtl_logo},
			{"sailboat",		XC_sailboat},
			{"sb_down_arrow",	XC_sb_down_arrow},
			{"sb_h_double_arrow",	XC_sb_h_double_arrow},
			{"sb_left_arrow",	XC_sb_left_arrow},
			{"sb_right_arrow",	XC_sb_right_arrow},
			{"sb_up_arrow",		XC_sb_up_arrow},
			{"sb_v_double_arrow",	XC_sb_v_double_arrow},
			{"shuttle",		XC_shuttle},
			{"sizing",		XC_sizing},
			{"spider",		XC_spider},
			{"spraycan",		XC_spraycan},
			{"star",		XC_star},
			{"target",		XC_target},
			{"tcross",		XC_tcross},
			{"top_left_arrow",	XC_top_left_arrow},
			{"top_left_corner",	XC_top_left_corner},
			{"top_right_corner",	XC_top_right_corner},
			{"top_side",		XC_top_side},
			{"top_tee",		XC_top_tee},
			{"trek",		XC_trek},
			{"ul_angle",		XC_ul_angle},
			{"umbrella",		XC_umbrella},
			{"ur_angle",		XC_ur_angle},
			{"watch",		XC_watch},
			{"xterm",		XC_xterm},
    };
    struct _CursorName *cache;
    static Cursor cursor;
    char *name = (char *)fromVal->addr;
    register int i;

    for( i=0, cache=cursor_names; i < XtNumber(cursor_names); i++, cache++ ) {
        /* cacheing is actually done by higher layers of Xrm */
	if (strcmp(name, cache->name) == 0) {
	    cursor = XCreateFontCursor(DisplayOfScreen(screen), cache->shape );
	    done(&cursor, Cursor);
	}
    }
    _XStringConversionWarning(name, "Cursor");
}


/*ARGSUSED*/
static void CvtGeometryToDims(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    done(&((Geometry *)fromVal->addr)->dims, Dims);
}


/*ARGSUSED*/
static void CvtStringToDisplay(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Display	*d;

    d = XOpenDisplay((char *)fromVal->addr);
    if (d != NULL) { done(d, Display); }

    /* else */
    _XStringConversionWarning((char *) fromVal->addr, "Display");
}


/*ARGSUSED*/
static void CvtStringToFile(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static FILE	*f;

    f = fopen((char *)fromVal->addr, "r");
    if (f != NULL) { done(f, FILE); }

    /* else */
    _XStringConversionWarning((char *) fromVal->addr, "File");
}


/*ARGSUSED*/
static void CvtStringToFont(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Font	f;

    f = XLoadFont(DisplayOfScreen(screen), (char *)fromVal->addr);
    if (f != 0) { done(&f, Font); }

    /* else */
    _XStringConversionWarning((char *) fromVal->addr, "Font");
}


/*ARGSUSED*/
static void CvtIntToFont(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    done(fromVal->addr, int);
}


/*ARGSUSED*/
static void CvtStringToFontStruct(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XFontStruct	*f;

    f = XLoadQueryFont(DisplayOfScreen(screen), (char *)fromVal->addr);
    if (f != NULL) { done(&f, XFontStruct *); }

    /* else */
    _XStringConversionWarning((char *) fromVal->addr, "XFontStruct");
}

/*ARGSUSED*/
static void CvtStringToGeometry(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Geometry	g;
    int			i;

    g.pos.xpos = g.pos.ypos = g.dims.width = g.dims.height = 0;
    i = XParseGeometry((char *) fromVal->addr,
	    &g.pos.xpos, &g.pos.ypos, &g.dims.width, &g.dims.height);

    if (i == NoValue) {
        _XStringConversionWarning((char *) fromVal->addr, "Geometry");
        return;
    }

    if (i & XNegative)
	g.pos.xpos += WidthOfScreen(screen);

    if (i & YNegative)
	g.pos.ypos += HeightOfScreen(screen);

    done(&g, Geometry);
}


/*ARGSUSED*/
static void CvtStringToInt(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static int	i;

    if (sscanf((char *)fromVal->addr, "%d", &i) == 1) { done(&i, int); }

    /* else */
    _XStringConversionWarning((char *) fromVal->addr, "Integer");
}


/*ARGSUSED*/
static void CvtStringToPixel(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    _XrmConvert(screen, XrmQString, fromVal, XrmQColor, toVal);
    if (toVal->addr != NULL)
        done(&((XColor *)(toVal->addr))->pixel, int);
}


/*ARGSUSED*/
static void CvtXColorToPixel(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    done(&((XColor *)fromVal->addr)->pixel, int);
}


/*ARGSUSED*/
static void CvtIntToPixel(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
	done(fromVal->addr, int);
}


/*ARGSUSED*/
static void CvtStringToPixmap(screen, fromVal, toVal)
    Screen	*screen;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    XrmValue	pixelVal;

    _XrmConvert(screen, XrmQString, fromVal, XrmQPixel, &pixelVal);
    if (pixelVal.addr == NULL) return;
    _XrmConvert(screen, XrmQPixel, &pixelVal, XrmQPixmap, toVal);
}


static Bool initialized = False;

void XrmInitialize()
{
    if (initialized)
    	return;
    initialized = True;

    _XrmRegisterTypeConverter(XrmQColor, XrmQPixel,	CvtXColorToPixel);

    _XrmRegisterTypeConverter(XrmQGeometry, XrmQDims,	CvtGeometryToDims);

    _XrmRegisterTypeConverter(XrmQInt, 	XrmQBoolean,	CvtIntToBoolean);
    _XrmRegisterTypeConverter(XrmQInt, 	XrmQPixel,	CvtIntToPixel);
    _XrmRegisterTypeConverter(XrmQInt, 	XrmQFont,	CvtIntToFont);
    _XrmRegisterTypeConverter(XrmQInt, 	XrmQColor,	CvtIntOrPixelToXColor);

    _XrmRegisterTypeConverter(XrmQString, XrmQBoolean,	CvtStringToBoolean);
    _XrmRegisterTypeConverter(XrmQString, XrmQColor,	CvtStringToXColor);
    _XrmRegisterTypeConverter(XrmQString, XrmQCursor,	CvtStringToCursor);
    _XrmRegisterTypeConverter(XrmQString, XrmQDisplay,	CvtStringToDisplay);
    _XrmRegisterTypeConverter(XrmQString, XrmQFile,	CvtStringToFile);
    _XrmRegisterTypeConverter(XrmQString, XrmQFont,	CvtStringToFont);
    _XrmRegisterTypeConverter(XrmQString, XrmQFontStruct, CvtStringToFontStruct);
    _XrmRegisterTypeConverter(XrmQString, XrmQGeometry,	CvtStringToGeometry);
    _XrmRegisterTypeConverter(XrmQString, XrmQInt,	CvtStringToInt);
    _XrmRegisterTypeConverter(XrmQString, XrmQPixel,	CvtStringToPixel);
    _XrmRegisterTypeConverter(XrmQString, XrmQPixmap,	CvtStringToPixmap);

    _XrmRegisterTypeConverter(XrmQPixel, XrmQColor,	CvtIntOrPixelToXColor);

}

#ifndef lint
/*
 * This hack is to pull in the right error description code from
 * the resource manager library if you have a load line:
 *	cc -o foo *.o -loldRm -lX
 * or
 *	cc -o foo *.o -lnewRm -lX
 * This should be removed as soon as the resource manager makes its way
 * into Xlib.
 */
static kludge()
{
	XGetErrorText();
}
#endif /* lint */
