#ifndef lint
static char rcsid[] = "$XConsortium: StrToBmap.c,v 1.5 89/04/13 17:12:26 jim Exp $";
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
#include	<sys/param.h>		/* just to get MAXPATHLEN */
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif
#include	<X11/Xmu/Converters.h>
#include	<X11/Xmu/CvtCache.h>
#include	<X11/Xmu/Drawing.h>


/*
 * XmuConvertStringToPixmap:
 *
 * creates a depth-1 Pixmap suitable for window manager icons.
 * "string" represents a bitmap(1) filename which may be absolute,
 * or relative to the global resource bitmapFilePath, class
 * BitmapFilePath.  If the resource is not defined, the default
 * value is the build symbol BITMAPDIR.
 *
 * shares lots of code with XmuConvertStringToCursor.  
 *
 * To use, include the following in your ClassInitialize procedure:

static XtConvertArgRec screenConvertArg[] = {
    {XtBaseOffset, (caddr_t) XtOffset(Widget, core.screen), sizeof(Screen *)}
};

    XtAddConverter("String", "Pixmap", XmuCvtStringToPixmap,
		   screenConvertArg, XtNumber(screenConvertArg));
 *
 */

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }


/*ARGSUSED*/
void XmuCvtStringToPixmap(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Pixmap pixmap;		/* static for cvt magic */
    char *name = (char *)fromVal->addr;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
             "String to pixmap conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    if (strcmp(name, "None") == 0) {
	pixmap = None;
	done(&pixmap, Pixmap);
	return;
    }

    if (strcmp(name, "ParentRelative") == 0) {
	pixmap = ParentRelative;
	done(&pixmap, Pixmap);
	return;
    }

    pixmap = XmuLocateBitmapFile (*((Screen **) args[0].addr), name,
				  NULL, 0, NULL, NULL, NULL, NULL);

    if (pixmap != None) {
	done (&pixmap, Pixmap);
    } else {
	XtStringConversionWarning (name, "Pixmap");
	return;
    }
}


/*
 * XmuLocateBitmapFile - read a bitmap file using the normal defaults
 */

Pixmap XmuLocateBitmapFile (screen, name, srcname, srcnamelen,
			    widthp, heightp, xhotp, yhotp)
    Screen *screen;
    char *name;
    char *srcname;			/* RETURN */
    int srcnamelen;			/* RETURN */
    int *widthp, *heightp, *xhotp, *yhotp;  /* RETURN */
{
    Display *dpy = DisplayOfScreen (screen);
    Window root = RootWindowOfScreen (screen);
    XmuCvtCache *cache = XmuCCLookupDisplay (dpy);
    char *bitmap_file_path;
    Bool try_default_path, try_plain_name = True;
    char filename[MAXPATHLEN];
    int width, height, xhot, yhot;
    int i;


#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif

    /*
     * look in cache for bitmap path
     */
    if (cache) {
	if (!cache->string_to_bitmap.file_path) {
	    XrmName xrm_name[2];
	    XrmClass xrm_class[2];
	    XrmRepresentation rep_type;
	    XrmValue value;

	    xrm_name[0] = XrmStringToName ("bitmapFilePath");
	    xrm_name[1] = NULL;
	    xrm_class[0] = XrmStringToClass ("BitmapFilePath");
	    xrm_class[1] = NULL;
	    if (XrmQGetResource (XtDatabase(dpy), xrm_name, xrm_class, 
				 &rep_type, &value) &&
		rep_type == XrmStringToQuark(XtRString)) {
		cache->string_to_bitmap.file_path = value.addr;
		cache->string_to_bitmap.try_default_path = True;
	    } else {
		cache->string_to_bitmap.file_path = BITMAPDIR;
		cache->string_to_bitmap.try_default_path = False;
	    }
	}
	bitmap_file_path = cache->string_to_bitmap.file_path;
	try_default_path = cache->string_to_bitmap.try_default_path;
    } else {
	bitmap_file_path = BITMAPDIR;
	try_default_path = False;
    }


    /*
     * Search order:
     *    1.  name if it begins with / or ./
     *    2.  prefix/name
     *    3.  BITMAPDIR/name
     *    4.  name if didn't begin with / or .
     */

    for (i = 1; i <= 4; i++) {
	char *fn = filename;
	Pixmap pixmap;

	switch (i) {
	  case 1:
	    if (!(name[0] == '/' || (name[0] == '.') && name[1] == '/')) 
	      continue;
	    fn = name;
	    try_plain_name = False;
	    break;
	  case 2:
	    sprintf (filename, "%s/%s", bitmap_file_path, name);
	    break;
	  case 3:
	    if (!try_default_path) continue;
	    sprintf (filename, "%s/%s", BITMAPDIR, name);
	    break;
	  case 4:
	    if (!try_plain_name) continue;
	    fn = name;
	    break;
	}

	if (XReadBitmapFile (dpy, root, fn, &width, &height, 
			     &pixmap, &xhot, &yhot) == BitmapSuccess) {
	    if (widthp) *widthp = width;
	    if (heightp) *heightp = height;
	    if (xhotp) *xhotp = xhot;
	    if (yhotp) *yhotp = yhot;
	    if (srcname && srcnamelen > 0) {
		strncpy (srcname, fn, srcnamelen - 1);
		srcname[srcnamelen - 1] = '\0';
	    }
	    return pixmap;
	}
    }

    return None;
}
