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
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

static int default_allocation();
static void best_allocation();

/*	Define all appropriate standard colormaps for the given visual.
 *	Return 0 on failure, non-zero on success.
 */
Status XmuVisualStandardColormaps(dpy, screen, visual, flag)
    Display	*dpy;
    int		screen;
    Visual	*visual;
    int		flag;	/* 0: replace, non-zero: do not replace property */
{
    unsigned long	red_max, green_max, blue_max;
    unsigned long	max;
    Status		status;
    
    max = (unsigned long) (visual->map_entries - 1);
    switch (visual->class)
    {
      case PseudoColor:
      case DirectColor:
	if (default_allocation(visual, &red_max, &green_max, &blue_max))
	{
	    status = XmuStandardColormap(dpy, screen, visual, red_max,
					 green_max, blue_max,
					 XA_RGB_DEFAULT_MAP, flag);
	    if (!status) break;
	}
	status = XmuStandardColormap(dpy, screen, visual, max, 0L, 0L,
				     XA_RGB_GRAY_MAP, flag);
	if (!status) break;
	status = XmuStandardColormap(dpy, screen, visual, max, 0L, 0L,
				     XA_RGB_RED_MAP, flag);
	if (!status) break;
	status = XmuStandardColormap(dpy, screen, visual, 0L, max, 0L,
				     XA_RGB_GREEN_MAP, flag);
	if (!status) break;
	status = XmuStandardColormap(dpy, screen, visual, 0L, 0L, max,
				     XA_RGB_BLUE_MAP, flag);
	if (!status) break;
	/* fall through */
      case StaticColor:
      case TrueColor:
	best_allocation(visual, &red_max, &green_max, &blue_max);
	status = XmuStandardColormap(dpy, screen, visual, red_max, green_max,
				     blue_max, XA_RGB_BEST_MAP, flag);
	break;
      case StaticGray:
      case GrayScale:
	status = XmuStandardColormap(dpy, screen, visual, max, 0L, 0L,
				     XA_RGB_GRAY_MAP, flag);
	break;
    }
    return status;
}

/*	Determine an appropriate color allocation for the RGB_DEFAULT_MAP.
 */
static int default_allocation(visual, red, green, blue)
    Visual		*visual;
    unsigned long	*red, *green, *blue;
{
    /* If a map has less than 254 definable entries, we do not produce an
     * allocation for an RGB_DEFAULT_MAP.  To determine the default rgb
     * allocation, let n = the number of colormap entries.
     * Then maximum red value = floor(cube_root(n - 125)) - 1
     * Maximum green and maximum blue values are identical to maximum red.
     * Return 0 if an allocation has been determined, non-zero otherwise.
     */
    if (visual->map_entries < 254)	/* skip it */
	return 0;

    *red = *green = *blue = (unsigned long)
	floor(pow((double) (visual->map_entries - 125), (double) 1.0/3.0)) - 1;
    return 1;
}

/*	Determine an appropriate color allocation for the RGB_BEST_MAP.
 *
 *	For a DirectColor or TrueColor visual, the allocation is determined
 *	by the red_mask, green_mask, and blue_mask members of the visual.
 *
 *	Otherwise, if the colormap size is an integral power of 2, determine
 *	the allocation according to the number of bits given to each color,
 *	with green getting more than red, and red more than blue, if there
 *	are to be inequities in the distribution.  If the colormap size is
 *	not an integral power of 2, let n = the number of colormap entries.
 *	Then	maximum red value =	floor(cube_root(n)) - 1;
 *		maximum blue value =	floor(cube_root(n)) - 1;
 *		maximum green value =	n / ((max red value) * (max green val))
 *	Which, on a GPX, allows for 252 entries in the best map, out of 254
 * 	defineable colormap entries.
 */
 
static void best_allocation(visual, red, green, blue)
    Visual		*visual;
    unsigned long	*red, *green, *blue;
{

    if (visual->class == DirectColor ||	visual->class == TrueColor)
    {
	*red = visual->red_mask;
	while ((*red & 01) == 0)
	    *red >>= 1;
	*green = visual->green_mask;
	while ((*green & 01) == 0)
	    *green >>=1;
	*blue = visual->blue_mask;
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
	while (visual->map_entries > n)
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
	if (n == visual->map_entries)
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
	    *red = (int) floor(pow((double) visual->map_entries, 
				   (double) 1.0/3.0));
	    *blue = *red;	
	    *green = (visual->map_entries / ((*red) * (*blue)));
	}
	(*red)--;
	(*green)--;
	(*blue)--;
    }
    return;
}
