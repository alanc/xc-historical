#ifndef lint
static char rcsid[] = "$XConsortium: StrToBmap.c,v 1.4 89/03/31 10:35:32 jim Exp $";
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
#include	"Xmu.h"

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

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps"
#endif

/*ARGSUSED*/
void XmuCvtStringToPixmap(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Pixmap pixmap;
    char *name = (char *)fromVal->addr;
    Screen *screen;
    static char *bitmap_file_path = NULL;
    static Bool try_default_path = True;
    Bool try_plain_name = True;
    char filename[MAXPATHLEN];
    static XColor bgColor = {0, 0, 0, 0};
    static XColor fgColor = {0, ~0, ~0, ~0};
    int width, height, xhot, yhot;
    int i;
    Bool gotit;
    Display *dpy;
    Window root;

    if (*num_args != 1)
     XtErrorMsg("wrongParameters","cvtStringToPixmap","XtToolkitError",
             "String to pixmap conversion needs screen argument",
              (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);
    dpy = DisplayOfScreen (screen);
    root = RootWindowOfScreen (screen);

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

    /*
     * the following will only work on single displays....
     */
    if (bitmap_file_path == NULL) {
	XrmName xrm_name[2];
	XrmClass xrm_class[2];
	XrmRepresentation rep_type;
	XrmValue value;
	xrm_name[0] = XrmStringToName( "bitmapFilePath" );
	xrm_name[1] = NULL;
	xrm_class[0] = XrmStringToClass( "BitmapFilePath" );
	xrm_class[1] = NULL;
	if (XrmQGetResource (XtDatabase(dpy), xrm_name, xrm_class, 
			     &rep_type, &value) &&
	    rep_type == XrmStringToQuark(XtRString))
	    bitmap_file_path = value.addr;
	else {
	    bitmap_file_path = BITMAPDIR;
	    try_default_path = False;
	}
    }

    /*
     * Search order:
     *    1.  name if it begins with / or ./
     *    2.  prefix/name
     *    3.  BITMAPDIR/name
     *    4.  name if didn't begin with / or .
     */

    gotit = False;
    for (i = 1; i <= 4; i++) {
	char *fn = filename;

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
	    gotit = True;
	    break;
	}
    }

    if (gotit) {
	done (&pixmap, Pixmap);
    } else {
	XtStringConversionWarning (name, "Pixmap");
	return;
    }
}
