/* $Header: Converters.c,v 1.12 88/01/29 17:05:35 asente Exp $ */
#ifndef lint
static char *sccsid = "@(#)Conversion.c	1.11	3/19/87";
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

#include	"IntrinsicP.h"
#include	"CoreP.h"
#include	"Atoms.h"
#include	<stdio.h>
#include	<sys/file.h>
#include	"cursorfont.h"
#include	"Convert.h"
#include	"Quarks.h"

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

void XtStringConversionWarning(from, toType)
    char *from, *toType;
{
    char message[1000];
    
    (void) sprintf(message, "Cannot convert string \"%s\" to type %s",
	    from, toType);
    XtWarning(message);
}

static void CvtXColorToPixel();
static void CvtDefaultColorToPixel();
static void CvtIntToBoolean();
static void CvtIntToLongBoolean();
static void CvtIntToPixmap();
static void CvtIntToFont();
static void CvtIntOrPixelToXColor();
static void CvtIntToPixel();

static void CvtStringToBoolean();
static void CvtStringToLongBoolean();
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

/*ARGSUSED*/
static void CvtIntToBoolean(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Boolean	b;

    if (*num_args != 0)
	XtWarning("Integer to Boolean conversion needs no extra arguments");
    b = (*(int *)fromVal->addr != 0);
    done(&b, Boolean);
};


/*ARGSUSED*/
static void CvtStringToBoolean(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Boolean b;
    XrmQuark	q;
    char	lowerName[1000];

    if (*num_args != 0)
	XtWarning("String to Boolean conversion needs no extra arguments");

    LowerCase((char *) fromVal->addr, lowerName);
    q = XrmAtomToQuark(lowerName);

    if (q == XtQEtrue || q == XtQEon || q == XtQEyes) {
	b = TRUE;
	done(&b, Boolean);
	return;
    }
    if (q == XtQEfalse || q ==XtQEoff || q == XtQEno) {
	b = FALSE;
	done(&b, Boolean);
	return;
    }

    XtStringConversionWarning((char *) fromVal->addr, "Boolean");
};


/*ARGSUSED*/
static void CvtIntToLongBoolean(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Bool	b;

    if (*num_args != 0)
	XtWarning("Integer to LongBoolean conversion needs no extra arguments");
    b = (*(int *)fromVal->addr != 0);
    done(&b, Bool);
};


/*ARGSUSED*/
static void CvtStringToLongBoolean(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Bool	b;
    XrmQuark	q;
    char	lowerName[1000];

    if (*num_args != 0)
	XtWarning("String to LongBoolean conversion needs no extra arguments");

    LowerCase((char *) fromVal->addr, lowerName);
    q = XrmAtomToQuark(lowerName);

    if (q == XtQEtrue || q == XtQEon || q == XtQEyes) {
	b = TRUE;
	done(&b, Bool);
	return;
    }
    if (q == XtQEfalse || q ==XtQEoff || q == XtQEno) {
	b = FALSE;
	done(&b, Bool);
	return;
    }

    XtStringConversionWarning((char *) fromVal->addr, "LongBoolean");
};


static XtConvertArgRec colorConvertArgs[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen),  sizeof(Screen *)},
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.colormap),sizeof(Colormap)}
};

static void CvtIntOrPixelToXColor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{    
    static XColor   c;
    Screen	    *screen;
    Colormap	    colormap;

    if (*num_args != 2)
      XtError("Pixel to color conversion needs screen and colormap arguments");

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);
    c.pixel = *(int *)fromVal->addr;

    XQueryColor(DisplayOfScreen(screen), colormap, &c);
    done(&c, XColor);
};


/*ARGSUSED*/
static void CvtStringToXColor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XColor   c;
    Screen	    *screen;
    Colormap	    colormap;
    Status	    s;
    char	    message[1000];

    if (*num_args != 2)
     XtError("String to color conversion needs screen and colormap arguments");

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);
    s = XParseColor(DisplayOfScreen(screen), colormap, (char *) fromVal->addr,
	&c);
    if (s == 0) {
	XtStringConversionWarning((char *) fromVal->addr, "Color");
	return;
    }
    s = XAllocColor(DisplayOfScreen(screen), colormap, &c);
    if (s == 0) {
	(void) sprintf(message, "Cannot allocate colormap entry for %s",
	    (char *) fromVal->addr);
	XtWarning(message);
	return;
    }
    done(&c, XColor);
};


static XtConvertArgRec screenConvertArg[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
};

/*ARGSUSED*/
static void CvtStringToCursor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static struct _CursorName {
	char		*name;
	unsigned int	shape;
	Cursor		cursor;
    } cursor_names[] = {
			{"X_cursor",		XC_X_cursor,		NULL},
			{"arrow",		XC_arrow,		NULL},
			{"based_arrow_down",	XC_based_arrow_down,    NULL},
			{"based_arrow_up",	XC_based_arrow_up,      NULL},
			{"boat",		XC_boat,		NULL},
			{"bogosity",		XC_bogosity,		NULL},
			{"bottom_left_corner",	XC_bottom_left_corner,  NULL},
			{"bottom_right_corner",	XC_bottom_right_corner, NULL},
			{"bottom_side",		XC_bottom_side,		NULL},
			{"bottom_tee",		XC_bottom_tee,		NULL},
			{"box_spiral",		XC_box_spiral,		NULL},
			{"center_ptr",		XC_center_ptr,		NULL},
			{"circle",		XC_circle,		NULL},
			{"clock",		XC_clock,		NULL},
			{"coffee_mug",		XC_coffee_mug,		NULL},
			{"cross",		XC_cross,		NULL},
			{"cross_reverse",	XC_cross_reverse,       NULL},
			{"crosshair",		XC_crosshair,		NULL},
			{"diamond_cross",	XC_diamond_cross,       NULL},
			{"dot",			XC_dot,			NULL},
#ifdef XC_dotbox
			{"dotbox",		XC_dotbox,		NULL},
#endif
			{"double_arrow",	XC_double_arrow,	NULL},
			{"draft_large",		XC_draft_large,		NULL},
			{"draft_small",		XC_draft_small,		NULL},
			{"draped_box",		XC_draped_box,		NULL},
			{"exchange",		XC_exchange,		NULL},
			{"fleur",		XC_fleur,		NULL},
			{"gobbler",		XC_gobbler,		NULL},
			{"gumby",		XC_gumby,		NULL},
#ifdef XC_hand1
			{"hand1",		XC_hand1,		NULL},
#endif
#ifdef XC_hand2
			{"hand2",		XC_hand2,		NULL},
#endif
			{"heart",		XC_heart,		NULL},
			{"icon",		XC_icon,		NULL},
			{"iron_cross",		XC_iron_cross,		NULL},
			{"left_ptr",		XC_left_ptr,		NULL},
			{"left_side",		XC_left_side,		NULL},
			{"left_tee",		XC_left_tee,		NULL},
			{"leftbutton",		XC_leftbutton,		NULL},
			{"ll_angle",		XC_ll_angle,		NULL},
			{"lr_angle",		XC_lr_angle,		NULL},
			{"man",			XC_man,			NULL},
			{"middlebutton",	XC_middlebutton,	NULL},
			{"mouse",		XC_mouse,		NULL},
			{"pencil",		XC_pencil,		NULL},
			{"pirate",		XC_pirate,		NULL},
			{"plus",		XC_plus,		NULL},
			{"question_arrow",	XC_question_arrow,	NULL},
			{"right_ptr",		XC_right_ptr,		NULL},
			{"right_side",		XC_right_side,		NULL},
			{"right_tee",		XC_right_tee,		NULL},
			{"rightbutton",		XC_rightbutton,		NULL},
			{"rtl_logo",		XC_rtl_logo,		NULL},
			{"sailboat",		XC_sailboat,		NULL},
			{"sb_down_arrow",	XC_sb_down_arrow,       NULL},
			{"sb_h_double_arrow",	XC_sb_h_double_arrow,   NULL},
			{"sb_left_arrow",	XC_sb_left_arrow,       NULL},
			{"sb_right_arrow",	XC_sb_right_arrow,      NULL},
			{"sb_up_arrow",		XC_sb_up_arrow,		NULL},
			{"sb_v_double_arrow",	XC_sb_v_double_arrow,   NULL},
			{"shuttle",		XC_shuttle,		NULL},
			{"sizing",		XC_sizing,		NULL},
			{"spider",		XC_spider,		NULL},
			{"spraycan",		XC_spraycan,		NULL},
			{"star",		XC_star,		NULL},
			{"target",		XC_target,		NULL},
			{"tcross",		XC_tcross,		NULL},
			{"top_left_arrow",	XC_top_left_arrow,      NULL},
			{"top_left_corner",	XC_top_left_corner,	NULL},
			{"top_right_corner",	XC_top_right_corner,    NULL},
			{"top_side",		XC_top_side,		NULL},
			{"top_tee",		XC_top_tee,		NULL},
			{"trek",		XC_trek,		NULL},
			{"ul_angle",		XC_ul_angle,		NULL},
			{"umbrella",		XC_umbrella,		NULL},
			{"ur_angle",		XC_ur_angle,		NULL},
			{"watch",		XC_watch,		NULL},
			{"xterm",		XC_xterm,		NULL},
    };
    struct _CursorName *cache;
    char *name = (char *)fromVal->addr;
    register int i;
    Screen	    *screen;

    if (*num_args != 1)
     XtError("String to cursor conversion needs screen argument");

    screen = *((Screen **) args[0].addr);
    for (i=0, cache=cursor_names; i < XtNumber(cursor_names); i++, cache++ ) {
	if (strcmp(name, cache->name) == 0) {
	    if (!cache->cursor)
		cache->cursor =
		    XCreateFontCursor(DisplayOfScreen(screen), cache->shape );
	    done(&(cache->cursor), Cursor);
	    return;
	}
    }
    XtStringConversionWarning(name, "Cursor");
};


/*ARGSUSED*/
static void CvtStringToDisplay(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Display	*d;

    if (*num_args != 0)
	XtWarning("String to Display conversion needs no extra arguments");

    d = XOpenDisplay((char *)fromVal->addr);
    if (d != NULL) {
	done(d, Display);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Display");
    }
};


/*ARGSUSED*/
static void CvtStringToFile(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static FILE	*f;

    if (*num_args != 0)
	XtWarning("String to File conversion needs no extra arguments");

    f = fopen((char *)fromVal->addr, "r");
    if (f != NULL) {
	done(f, FILE);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "File");
    }
};


/*ARGSUSED*/
static void CvtStringToFont(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Font	f;
    Screen	    *screen;

    if (*num_args != 1)
     XtError("String to font conversion needs screen argument");

    screen = *((Screen **) args[0].addr);
    f = XLoadFont(DisplayOfScreen(screen), (char *)fromVal->addr);
    if (f != 0) {
	done(&f, Font);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Font");
    }
}


/*ARGSUSED*/
static void CvtIntToFont(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarning("Integer to Font conversion needs no extra arguments");
    done(fromVal->addr, int);
};


/*ARGSUSED*/
static void CvtStringToFontStruct(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XFontStruct	*f;
    Screen	    *screen;

    if (*num_args != 1)
     XtError("String to cursor conversion needs screen argument");

    screen = *((Screen **) args[0].addr);
    f = XLoadQueryFont(DisplayOfScreen(screen), (char *)fromVal->addr);
    if (f != NULL) {
	done(&f, XFontStruct *);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "XFontStruct");
    }
}

/*ARGSUSED*/
static void CvtStringToInt(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static int	i;

    if (*num_args != 0)
	XtWarning("String to Integer conversion needs no extra arguments");
    if (sscanf((char *)fromVal->addr, "%d", &i) == 1) {
	done(&i, int);
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Integer");
    }
}


/*ARGSUSED*/
static void CvtStringToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    XtDirectConvert((XtConverter) CvtStringToXColor, args, *num_args,
	    fromVal, toVal);
    if ((*toVal).addr == NULL) return;
    done(&((XColor *)((*toVal).addr))->pixel, int)
};


/*ARGSUSED*/
static void CvtXColorToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarning("Color to Pixel conversion needs no extra arguments");
    done(&((XColor *)fromVal->addr)->pixel, int);
};

/*ARGSUSED*/
static void CvtDefaultColorToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    Screen* screen;
    if (*num_args != 1)
        XtWarning("DefaultColor to Pixel conversion needs screen argument");
    screen = *((Screen **) args[0].addr);
    if (*fromVal->addr == 0)
        done(&screen->black_pixel,int)
    else if (*fromVal->addr == 1)
        done(&screen->white_pixel,int)


};

/*ARGSUSED*/
static void CvtIntToPixel(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    if (*num_args != 0)
	XtWarning("Integer to Pixel conversion needs no extra arguments");
    done(fromVal->addr, int);
};

/*ARGSUSED*/
static void CvtIntToPixmap(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    if (*num_args != 0)
        XtWarning("Integer to Pixmap conversion needs no extra arguments");
    done(fromVal->addr, int);
};



void LowerCase(source, dest)
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

XrmQuark  XtQBoolean;
XrmQuark  XtQLongBoolean;
XrmQuark  XtQColor;
XrmQuark  XtQCursor;
XrmQuark  XtQDims;
XrmQuark  XtQDisplay;
XrmQuark  XtQFile;
XrmQuark  XtQFloat;
XrmQuark  XtQFont;
XrmQuark  XtQFontStruct;
XrmQuark  XtQGeometry;
XrmQuark  XtQInt;
XrmQuark  XtQPixel;
XrmQuark  XtQPixmap;
XrmQuark  XtQPointer;
XrmQuark  XtQString;
XrmQuark  XtQWindow;
XrmQuark  XtQDefaultColor;

XrmQuark  XtQEoff;
XrmQuark  XtQEfalse;
XrmQuark  XtQEno;
XrmQuark  XtQEon;
XrmQuark  XtQEtrue;
XrmQuark  XtQEyes;

static Boolean initialized = FALSE;

void _XtConvertInitialize()
{
    if (initialized) return;
    initialized = TRUE;

/* Representation types */

    XtQBoolean		= XrmAtomToQuark(XtRBoolean);
    XtQLongBoolean	= XrmAtomToQuark(XtRLongBoolean);
    XtQColor		= XrmAtomToQuark(XtRColor);
    XtQCursor		= XrmAtomToQuark(XtRCursor);
    XtQDisplay		= XrmAtomToQuark(XtRDisplay);
    XtQFile		= XrmAtomToQuark(XtRFile);
    XtQFont		= XrmAtomToQuark(XtRFont);
    XtQFontStruct	= XrmAtomToQuark(XtRFontStruct);
    XtQGeometry		= XrmAtomToQuark(XtRGeometry);
    XtQInt		= XrmAtomToQuark(XtRInt);
    XtQPixel		= XrmAtomToQuark(XtRPixel);
    XtQPixmap		= XrmAtomToQuark(XtRPixmap);
    XtQPointer		= XrmAtomToQuark(XtRPointer);
    XtQString		= XrmAtomToQuark(XtRString);
    XtQWindow		= XrmAtomToQuark(XtRWindow);
    XtQDefaultColor     = XrmAtomToQuark(XtRDefaultColor);

/* Boolean enumeration constants */

    XtQEfalse		= XrmAtomToQuark(XtEfalse);
    XtQEno		= XrmAtomToQuark(XtEno);
    XtQEoff		= XrmAtomToQuark(XtEoff);
    XtQEon		= XrmAtomToQuark(XtEon);
    XtQEtrue		= XrmAtomToQuark(XtEtrue);
    XtQEyes		= XrmAtomToQuark(XtEyes);

#define Add(from, to, proc, convert_args, num_args) \
    _XtAddConverter(from, to, (XtConverter) proc, \
	    (XtConvertArgList) convert_args, num_args)

    Add(XtQColor,   XtQPixel,       CvtXColorToPixel,	    NULL, 0);
    Add(XtQDefaultColor, XtQPixel,  CvtDefaultColorToPixel,
       screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQInt,     XtQBoolean,     CvtIntToBoolean,	    NULL, 0);
    Add(XtQInt,     XtQLongBoolean, CvtIntToLongBoolean,    NULL, 0);
    Add(XtQInt,     XtQPixel,       CvtIntToPixel,          NULL, 0);
    Add(XtQInt,     XtQPixmap,      CvtIntToPixmap,	    NULL, 0);
    Add(XtQInt,     XtQFont,	    CvtIntToFont,	    NULL, 0);
    Add(XtQInt,     XtQColor,       CvtIntOrPixelToXColor,  
	colorConvertArgs, XtNumber(colorConvertArgs));

    Add(XtQString,  XtQBoolean,     CvtStringToBoolean,     NULL, 0);
    Add(XtQString,  XtQLongBoolean, CvtStringToLongBoolean, NULL, 0);
    Add(XtQString,  XtQColor,       CvtStringToXColor,      
	colorConvertArgs, XtNumber(colorConvertArgs));
    Add(XtQString,  XtQCursor,      CvtStringToCursor,      
	screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQString,  XtQDisplay,     CvtStringToDisplay,     NULL, 0);
    Add(XtQString,  XtQFile,	    CvtStringToFile,	    NULL, 0);
    Add(XtQString,  XtQFont,	    CvtStringToFont,	    
	screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQString,  XtQFontStruct,  CvtStringToFontStruct,  
	screenConvertArg, XtNumber(screenConvertArg));
    Add(XtQString,  XtQInt,	    CvtStringToInt,	    NULL, 0);
    Add(XtQString,  XtQPixel,       CvtStringToPixel,       
	colorConvertArgs, XtNumber(colorConvertArgs));

    Add(XtQPixel,   XtQColor,       CvtIntOrPixelToXColor,  
	colorConvertArgs, XtNumber(colorConvertArgs));

}
