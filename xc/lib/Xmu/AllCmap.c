/* $XConsortium$ */

/* 
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
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
 

/*	Define and retain all standard colormaps for all visuals of all
 *	screens of the given display.  Return 0 on failure, non-zero on
 *	success.
 */
Status XmuAllStandardColormaps(display)
    Display	*display;	/* Specifies the connection to the X server */
{
    Display	*dpy;
    int 	i, nvisuals, screen, flag;
    Status	status = 0;
    XVisualInfo	template, *vinfo;

    
    /* open a new connection to the server */
    if ((dpy = XOpenDisplay(DisplayString(display))) == NULL) 
    {
	(void) fprintf(stderr,
	         "XmuAllStandardColormaps: cannot open display \"%s\".\n",
	         DisplayString(display));
	return 0;
    }
    
    /* for each screen, determine all visuals of this server */
    for (screen=0; screen < ScreenCount(dpy); screen++)
    {
	template.screen = screen;
	vinfo = XGetVisualInfo(dpy, VisualScreenMask, &template, &nvisuals);
	if (vinfo == NULL) /* a screen with no visuals */
	    continue;
		
	/* Create the appropriate standard colormaps for each visual.
	 * The default visual of this screen has highest priority in the
	 * case of duplicate property names.  Changes to Xlib driven by
         * the ICCCM may allow more than one RGB_BEST_MAP, for example,
	 * to be defined on a single screen.
	 */
	for (i=0; i < nvisuals; i++) 
	{
	    flag = ((vinfo[i].visualid ==
		     XVisualIDFromVisual(DefaultVisual(dpy, screen)))
		    ? 0		/* replace property */
		    : 1);	/* do not replace property*/
	    
	    status = XmuVisualStandardColormaps(dpy, screen, vinfo[i].visual,
						flag);
	}
	XFree((char *) vinfo);
	if (!status) break;
    }
    
    /* if all went well, retain the new properties until the server recycles */
    if (status)
	XSetCloseDownMode(dpy, RetainPermanent);
    XCloseDisplay(dpy);
    return status;
}
