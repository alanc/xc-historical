#ifndef lint
static char rcsid[] = "$XConsortium: StrToCurs.c,v 1.5 89/04/13 17:12:27 jim Exp $";
#endif /* lint */


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/cursorfont.h>
#include	<X11/IntrinsicP.h>	/* 'cause CoreP.h needs it */
#include	<X11/CoreP.h>		/* just to do XtConvert() */
#include	<sys/param.h>		/* just to get MAXPATHLEN */
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#include	"Xmu.h"


extern void CvtStringToCursor();

/*
 * XmuConvertStringToCursor:
 *
 * allows String to specify a standard cursor name (from cursorfont.h), a
 * font name and glyph index of the form "FONT fontname index [[font] index]", 
 * or a bitmap file name (absolute, or relative to the global resource
 * bitmapFilePath, class BitmapFilePath).  If the resource is not
 * defined, the default value is the build symbol BITMAPDIR.
 *
 * shares lots of code with XmuCvtStringToPixmap, but unfortunately
 * can't use it as the hotspot info is lost.
 *
 * To use, include the following in your ClassInitialize procedure:

static XtConvertArgRec screenConvertArg[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
};

    XtAddConverter("String", "Cursor", XmuCvtStringToCursor,      
		   screenConvertArg, XtNumber(screenConvertArg));
 *
 */

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif

#define FONTSPECIFIER		"FONT "

/*ARGSUSED*/
void XmuCvtStringToCursor(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Cursor cursor;
    char *name = (char *)fromVal->addr;
    Screen *screen;
    register int i;
    static char* bitmap_file_path = NULL;
    char filename[MAXPATHLEN], maskname[MAXPATHLEN];
    Pixmap source, mask;
    static XColor bgColor = {0, ~0, ~0, ~0};
    static XColor fgColor = {0, 0, 0, 0};
    int width, height, xhot, yhot;

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
    struct _CursorName *table;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToCursor","XtToolkitError",
             "String to cursor conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);

    if (0 == strncmp(FONTSPECIFIER, name, strlen(FONTSPECIFIER))) {
	char source_name[MAXPATHLEN], mask_name[MAXPATHLEN];
	int source_char, mask_char, fields;
	WidgetRec widgetRec;
	Font source_font, mask_font;
	XrmValue fromString, toFont;

	fields = sscanf(name, "FONT %s %d %s %d",
			source_name, &source_char,
			mask_name, &mask_char);
	if (fields < 2) {
	    XtStringConversionWarning( name, "Cursor" );
	    return;
	}

	/* widgetRec is stupid; we should just use XtDirectConvert,
	 * but the names in Xt/Converters aren't public. */
	widgetRec.core.screen = screen;
	fromString.addr = source_name;
	fromString.size = strlen(source_name);
	XtConvert(&widgetRec, XtRString, &fromString, XtRFont, &toFont);
	if (toFont.addr == NULL) {
	    XtStringConversionWarning( name, "Cursor" );
	    return;
	}
	source_font = *(Font*)toFont.addr;

	switch (fields) {
	  case 2:		/* defaulted mask font & char */
	    mask_font = source_font;
	    mask_char = source_char;
	    break;

	  case 3:		/* defaulted mask font */
	    mask_font = source_font;
	    mask_char = atoi(mask_name);
	    break;

	  case 4:		/* specified mask font & char */
	    fromString.addr = mask_name;
	    fromString.size = strlen(mask_name);
	    XtConvert(&widgetRec, XtRString, &fromString, XtRFont, &toFont);
	    if (toFont.addr == NULL) {
		XtStringConversionWarning( name, "Cursor" );
		return;
	    }
	    mask_font = *(Font*)toFont.addr;
	}

	cursor = XCreateGlyphCursor( DisplayOfScreen(screen), source_font,
				     mask_font, source_char, mask_char,
				     &fgColor, &bgColor );
	done(&cursor, Cursor);
	return;
    }

    for (i=0, table=cursor_names; i < XtNumber(cursor_names); i++, table++ ) {
	if (strcmp(name, table->name) == 0) {
	    cursor = XCreateFontCursor(DisplayOfScreen(screen), table->shape );
	    done(&cursor, Cursor);
	    return;
	}
    }

    /* isn't a standard cursor in cursorfont; try to open a bitmap file */
    if (bitmap_file_path == NULL) {
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;
	xrm_name[0] = XrmStringToName( "bitmapFilePath" );
	xrm_name[1] = NULL;
	xrm_class[0] = XrmStringToClass( "BitmapFilePath" );
	xrm_class[1] = NULL;
	if (XrmQGetResource( XtDatabase(DisplayOfScreen(screen)),
			     xrm_name, xrm_class, &rep_type, &value )
	    && rep_type == XrmStringToQuark(XtRString))
	    bitmap_file_path = value.addr;
	else
	    bitmap_file_path = BITMAPDIR;
    }

    if ( name[0] == '/' || name[0] == '.' )
 	strcpy( filename, name );
    else
	sprintf( filename, "%s/%s", bitmap_file_path, name );

    if (XReadBitmapFile( DisplayOfScreen(screen), RootWindowOfScreen(screen),
			 filename, &width, &height, &source, &xhot, &yhot )
	!= BitmapSuccess) {
	XtStringConversionWarning( name, "Cursor" );
	return;
    }
    (void) strcpy( maskname, filename );
    (void) strcat( maskname, "Mask" );
    if (XReadBitmapFile( DisplayOfScreen(screen), RootWindowOfScreen(screen),
			 maskname, &width, &height, &mask, &width, &height )
	!= BitmapSuccess) {
	(void) strcpy( maskname, filename );
	(void) strcat( maskname, "msk" );
	if (XReadBitmapFile(DisplayOfScreen(screen),RootWindowOfScreen(screen),
			    maskname, &width, &height, &mask, &width, &height )
	    != BitmapSuccess) {
	    mask = None;
	}
    }
    cursor = XCreatePixmapCursor( DisplayOfScreen(screen), source, mask,
				  &fgColor, &bgColor, xhot, yhot );
    XFreePixmap( DisplayOfScreen(screen), source );
    if (mask != None) XFreePixmap( DisplayOfScreen(screen), mask );

    done(&cursor, Cursor);
}
