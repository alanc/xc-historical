/* $XConsortium: VisCmap.c,v 1.3 89/03/24 16:30:02 converse Exp $ */

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
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

static int default_allocation();
static void best_allocation();

/* To determine the best allocation of reds, greens, and blues in a 
 * standard colormap, use XmuGetColormapAllocation.
 *	property	specifies one of the standard colormap property names
 * 	vinfo		specifies visual information for a chosen visual
 * 	rmax		returns maximum red value 
 *      gmax 		returns maximum green value
 * 	bmax 		returns maximum blue value
 *
 * XmuGetColormapAllocation returns 0 on failure, non-zero on success.
 * It is assumed that the visual is appropriate for the colormap property.
 */

Status XmuGetColormapAllocation(property, vinfo, rmax, gmax, bmax)
Atom		property;
XVisualInfo	*vinfo;
unsigned long	*rmax, *gmax, *bmax;
{
    Status 	status = 1;

    if (vinfo->colormap_size <= 2)
	return 0;

    switch (property)
    {
      case XA_RGB_DEFAULT_MAP:
	status = default_allocation(vinfo->colormap_size, rmax, gmax, bmax);
	break;
      case XA_RGB_BEST_MAP:
	best_allocation(vinfo, rmax, gmax, bmax);
	break;
      case XA_RGB_GRAY_MAP:
      case XA_RGB_RED_MAP:
	*rmax = vinfo->colormap_size - 1;
	*gmax = *bmax = 0;
	break;
      case XA_RGB_GREEN_MAP:
	*gmax = vinfo->colormap_size - 1;
	*rmax = *bmax = 0;
	break;
      case XA_RGB_BLUE_MAP:
	*bmax = vinfo->colormap_size - 1;
	*rmax = *gmax = 0;
	break;
      default:
	status = 0;
    }
    return status;
}

/*
 * To create all of the appropriate standard colormaps for a given visual on
 * a given screen, use XmuVisualStandardColormaps.
 * 
 * Define all appropriate standard colormap properties for the given visual.
 * If replace is true, any previous definition will not be retained.  
 * Return 0 on failure, non-zero on success.
 *
 * Not all standard colormaps are meaningful to all visual classes.  This
 * routine will check and define the following properties for the following
 * classes, provided that the size of the colormap is not too small.
 *
 *	DirectColor and PseudoColor
 *	    RGB_DEFAULT_MAP
 *	    RGB_BEST_MAP
 *	    RGB_RED_MAP
 *	    RGB_GREEN_MAP
 * 	    RGB_BLUE_MAP
 *          RGB_GRAY_MAP
 *
 *	TrueColor and StaticColor
 *	    RGB_BEST_MAP
 *
 *	GrayScale and StaticGray
 *	    RGB_GRAY_MAP
 */

Status XmuVisualStandardColormaps(dpy, screen, visualid, depth, replace)
    Display		*dpy;		/* server connection */
    int			screen;		/* screen number */
    VisualID		visualid;	/* for identification of the visual */
    unsigned int	depth;		/* for identification of the visual */
    Bool		replace;	/* if true, replace old definition */
{
    unsigned long	rmax, gmax, bmax;	/* maximum values */
    unsigned long	max;			/* single maximum */
    Status		status;
    int			n;
    long		vinfo_mask;
    XVisualInfo		vinfo_template, *vinfo;
        
    vinfo_template.screen = screen;
    vinfo_template.visualid = visualid;
    vinfo_template.depth = depth;
    vinfo_mask = VisualScreenMask | VisualIDMask | VisualDepthMask;
    if ((vinfo = XGetVisualInfo(dpy, vinfo_mask, &vinfo_template, &n)) == NULL)
	return 0;
    if (vinfo->colormap_size <= 2)
    {			/* monochrome visuals have no standard maps */
	XFree((char *) vinfo);
	return 1;
    }
    max = (unsigned long) (vinfo->colormap_size - 1);

    switch (vinfo->class)
    {
      case PseudoColor:
      case DirectColor:
	if (default_allocation(vinfo->colormap_size, &rmax, &gmax, &bmax))
	{
	    status = XmuStandardColormap(dpy, screen, visualid, depth,
					 rmax, gmax, bmax,
					 XA_RGB_DEFAULT_MAP, replace);
	    if (!status) break;
	}
	status = XmuStandardColormap(dpy, screen, visualid, depth,
				     max, 0L, 0L, XA_RGB_GRAY_MAP, replace);
	if (!status) break;
	status = XmuStandardColormap(dpy, screen, visualid, depth,
				     max, 0L, 0L, XA_RGB_RED_MAP, replace);
	if (!status) break;
	status = XmuStandardColormap(dpy, screen, visualid, depth,
				     0L, max, 0L, XA_RGB_GREEN_MAP, replace);
	if (!status) break;
	status = XmuStandardColormap(dpy, screen, visualid, depth,
				     0L, 0L, max, XA_RGB_BLUE_MAP, replace);
	if (!status) break;
	/* fall through */
      case StaticColor:
      case TrueColor:
	best_allocation(vinfo, &rmax, &gmax, &bmax);
	status = XmuStandardColormap(dpy, screen, visualid, depth,
				     rmax, gmax, bmax,
				     XA_RGB_BEST_MAP, replace);
	break;
      case StaticGray:
      case GrayScale:
	status = XmuStandardColormap(dpy, screen, visualid, depth,
				     max, 0L, 0L, XA_RGB_GRAY_MAP, replace);
	break;
    }
    XFree((char *) vinfo);
    return status;
}

/* Determine an appropriate color allocation for the RGB_DEFAULT_MAP.
 * If a map has less than a minimum number of definable entries, we do not
 * produce an allocation for an RGB_DEFAULT_MAP.  To determine the default
 * rgb allocation, let n = the number of colormap entries.  Then,
 * maximum red value = floor(cube_root(n - 125)) - 1
 * Maximum green and maximum blue values are identical to maximum red.
 * This leaves at least 125 cells which clients can allocate.
 * Return 0 if an allocation has been determined, non-zero otherwise.
 */
static int default_allocation(colormap_size, red, green, blue)
    int			colormap_size;
    unsigned long	*red, *green, *blue;
{
    if (colormap_size < 250)	/* skip it */
	return 0;

    *red = *green = *blue = (unsigned long)
      (floor(pow((double) (colormap_size - 125), (double) 1.0/3.0)) - 1);
    return 1;
}

/* Determine an appropriate color allocation for the RGB_BEST_MAP.
 *
 * For a DirectColor or TrueColor visual, the allocation is determined
 * by the red_mask, green_mask, and blue_mask members of the visual info.
 *
 * Otherwise, if the colormap size is an integral power of 2, determine
 * the allocation according to the number of bits given to each color,
 * with green getting more than red, and red more than blue, if there
 * are to be inequities in the distribution.  If the colormap size is
 * not an integral power of 2, let n = the number of colormap entries.
 * Then maximum red value = floor(cube_root(n)) - 1;
 * 	maximum blue value = floor(cube_root(n)) - 1;
 *	maximum green value = n / ((# red values) * (# blue values)) - 1;
 * Which, on a GPX, allows for 252 entries in the best map, out of 254
 * defineable colormap entries.
 */
 
static void best_allocation(vinfo, red, green, blue)
    XVisualInfo		*vinfo;
    unsigned long	*red, *green, *blue;
{

    if (vinfo->class == DirectColor ||	vinfo->class == TrueColor)
    {
	*red = vinfo->red_mask;
	while ((*red & 01) == 0)
	    *red >>= 1;
	*green = vinfo->green_mask;
	while ((*green & 01) == 0)
	    *green >>=1;
	*blue = vinfo->blue_mask;
	while ((*blue & 01) == 0)
	    *blue >>= 1;
    }
    else
    {
	register int bits, n;
	
	/* Determine n such that n is the least integral power of 2 which is
	 * greater than or equal to the number of entries in the colormap.
         */
	n = 1;
	bits = 0;
	while (vinfo->colormap_size > n)
	{
	    n = n << 1;
	    bits++;
	}
	
	/* If the number of entries in the colormap is a power of 2, determine
	 * the allocation by "dealing" the bits, first to green, then red, then
	 * blue.  If not, find the maximum integral red, green, and blue values
	 * which, when multiplied together, do not exceed the number of 

	 * colormap entries.
	 */
	if (n == vinfo->colormap_size)
	{
	    register int r, g, b;
	    b = bits / 3;
	    g = b + ((bits % 3) ? 1 : 0);
	    r = b + (((bits % 3) == 2) ? 1 : 0);
	    *red = 1 << r;
	    *green = 1 << g;
	    *blue = 1 << b;
	}
	else
	{
	    *red = (int) floor(pow((double) vinfo->colormap_size, 
				   (double) 1.0/3.0));
	    *blue = *red;	
	    *green = (vinfo->colormap_size / ((*red) * (*blue)));
	}
	(*red)--;
	(*green)--;
	(*blue)--;
    }
    return;
}
