/* $XConsortium$ 
 * 
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Donna Converse, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

extern XStandardColormap *XmuStandardColormap();
extern void XmuDeleteStandardColormap();
extern Status XmuGetColormapAllocation();

/*
 * To create a standard colormap if one does not currently exist, or
 * replace the currently existing standard colormap, use 
 * XmuLookupStandardColormap().
 *
 * Given a screen, a visual, and a property, XmuLookupStandardColormap()
 * will determine the best allocation for the property under the specified
 * visual, and determine the whether to create a new colormap or to use
 * the default colormap of the screen.  It will call XmuStandardColormap()
 * to create the standard colormap.
 *
 * If replace is true, any previous definition of the property will be 
 * replaced.  If retain is true, the property and the colormap will be
 * made permanent for the duration of the server session.  However,
 * pre-existing property definitions which are not replaced cannot be made
 * permanent by a call to XmuLookupStandardColormap(); a request to retain 
 * resources pertains to newly created resources.
 *
 * Returns 0 on failure, non-zero on success.  A request to create a 
 * standard colormap upon a visual which cannot support such a map is
 * considered a failure.  An example of this would be requesting any
 * standard colormap property on a monochrome visual, or, requesting an
 * RGB_BEST_MAP on a display whose colormap size is 16.
 */

Status XmuLookupStandardColormap(dpy, screen, visualid, depth, property,
				 replace, retain)
    Display		*dpy;		/* specifies X server connection */
    int			screen; 	/* specifies screen of display */
    VisualID		visualid;	/* specifies the visual type */
    unsigned int	depth;		/* specifies  the visual type */
    Atom		property;	/* a standard colormap property */
    Bool		replace;	/* specifies whether to replace */
    Bool		retain;		/* specifies whether to retain */
{
    Display		*odpy;		/* original display connection */
    XStandardColormap	*stdcmaps, *colormap;	
    XVisualInfo		vinfo_template, *vinfo;	/* visual */
    long		vinfo_mask;
    unsigned long	r_max, g_max, b_max;	/* allocation */
    int			count;	
    Colormap		cmap;			/* colormap ID */


    /* Match the requested visual */
    vinfo_template.visualid = visualid;	
    vinfo_template.screen = screen;
    vinfo_template.depth = depth;
    vinfo_mask = VisualIDMask | VisualScreenMask | VisualDepthMask;
    if ((vinfo = XGetVisualInfo(dpy, vinfo_mask, &vinfo_template, &count)) ==
	NULL)
	return 0;

    if (vinfo->colormap_size <= 2) {
	/* Monochrome visuals have no standard maps */
	XFree((char *) vinfo);
	return 0;	
    }

    if (XGetRGBColormaps(dpy, RootWindow(dpy, screen), &stdcmaps, &count,
			 property)) {
	XFree((char *) stdcmaps);
	if (replace) {
	    /* Free old resources first - we may need them, particularly in 
	     * the default colormap of the screen.  However, because of this,
	     * it is possible that we will destroy the old resource and fail 
	     * to create a new one when a lower lever routine returns a 
	     * failure status.
	     */
	    XmuDeleteStandardColormap(dpy, screen, property);
	}
	else {
	    /* If the requested property already exists on this screen, and, 
	     * if the replace flag has not been set to true, return success.
	     */
	    XFree((char *) vinfo);
	    return 1;
	}
    }

    /* Determine the best allocation for this property under the requested
     * visualid and depth, and determine whether or not to use the default
     * colormap of the screen.
     */
    if (!XmuGetColormapAllocation(vinfo, property, &r_max, &g_max, &b_max)) {
	XFree((char *) vinfo);
	return 0;
    }

    cmap = (property == XA_RGB_DEFAULT_MAP)
	? DefaultColormap(dpy, screen) : None;

    if (retain) {
	/* Open a new connection to the same display server */
	odpy = dpy;
	if ((dpy = XOpenDisplay(XDisplayString(odpy))) == NULL) {
	    XFree((char *) vinfo);
	    dpy = odpy;
	    return 0;
	}
    }

    colormap = XmuStandardColormap(dpy, screen, visualid, depth, property,
				   cmap, r_max, g_max, b_max);q
    if (colormap) {
	XGrabServer(dpy);

	if (XGetRGBColormaps(dpy, RootWindow(dpy, screen), &stdcmaps, &count,
			     property)) {
	    XFree((char *) stdcmaps);
	    if (replace)  {
		/* Someone has defined the property since we last looked.
		 * Release the resources used by that definition, and
		 * attach our definition of the property to the root window.
		 */
		XmuDeleteStandardColormap(dpy, screen, property);
		XSetRGBColormaps(dpy, RootWindow(dpy, screen), colormap, 1,
				 property);
		if (retain)
		    XSetCloseDownMode(dpy, RetainPermanent);
	    }
	    else {
		/* Someone has defined the property since we last looked.
		 * Since we will not replace it, release our own resources.
		 */
		if (colormap->killid == ReleaseByFreeingColormap)
		    XFreeColormap(dpy, colormap->colormap);
	    }
	}
	else {
	    /* The most common case: nothing to free, define the property */
	    XSetRGBColormaps(dpy, RootWindow(dpy, screen), colormap, 1,
			     property);
	    if (retain)
		XSetCloseDownMode(dpy, RetainPermanent);
	}

	XUngrabServer(dpy);
    }

    if (retain) {
	XCloseDisplay(dpy);
	dpy = odpy;
    }
    XFree((char *) vinfo);
    return ((colormap) ? 1 : 0);
}
