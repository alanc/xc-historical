#ifndef lint
static char rcsid[] = "$XConsortium: StrToCurs.c,v 1.6 89/06/30 09:00:50 swick Exp $";
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

#include	<X11/IntrinsicP.h>	/* 'cause CoreP.h needs it */
#include	<X11/CoreP.h>		/* just to do XtConvert() */
#include	<X11/StringDefs.h>
#include	<sys/param.h>		/* just to get MAXPATHLEN */
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#include	<X11/Xmu/Converters.h>


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

    i = XmuCursorNameToIndex (name);
    if (i != -1) {
	cursor = XCreateFontCursor (DisplayOfScreen(screen), i);
	done(&cursor, Cursor);
	return;
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
