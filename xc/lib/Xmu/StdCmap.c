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

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

extern char	*calloc();
extern void	free();

static Status default_map();
static Status rgb_map();
static Status static_map();

/*
 * To create any one standard colormap, use XmuStandardColormap().
 *
 * Create a standard colormap for the given screen, visualid, and visual
 * depth, with the given red, green, and blue maximum values, with the
 * given standard property name.  If replace is true and the property is
 * already defined, redefine it.  Return 0 on failure, non-zero on success.
 *
 * All colormaps are created with read only allocations, with the exception
 * of read only allocations of colors in the RGB_DEFAULT_MAP which fail to
 * return the expected pixel value, and these are individually defined as
 * read/write allocations.  This is done so that the all the cells defined
 * in the default map are contiguous, for use in image processing.
 *
 * Standard colormaps of static visuals are defined if the map of the
 * static visual matches the definition of an appropriate standard map.
 * Resources created by this function are not made permanent; that is the
 * caller's responsibility.
 */

Status XmuStandardColormap(dpy, screen, visualid, depth, red_max,
			   green_max, blue_max, property, replace)
    Display		*dpy;
    int			screen; 
    VisualID		visualid;
    unsigned int	depth;
    unsigned long	red_max, green_max, blue_max;
    Atom		property;
    Bool		replace;
{
    XStandardColormap	colormap;
    XStandardColormap	stdmap;
    Status		status = 0;
    XVisualInfo		vinfo_template, *vinfo;
    long		vinfo_mask;
    int			n;
    unsigned long	ncolors;
    
    if (!replace && XGetStandardColormap(dpy, RootWindow(dpy, screen),
					 &colormap, property))
	return 1;

    vinfo_template.visualid = visualid;	
    vinfo_template.screen = screen;
    vinfo_template.depth = depth;
    vinfo_mask = VisualIDMask | VisualScreenMask | VisualDepthMask;
    if ((vinfo = XGetVisualInfo(dpy, vinfo_mask, &vinfo_template, &n)) == NULL)
	return 0;

    /* determine that the number of colors requested is <= map size */
    ncolors = (red_max + 1) * (green_max + 1) * (blue_max + 1);
    if (ncolors > vinfo->colormap_size)
    {
	XFree((char *) vinfo);
	return 0;
    }
    
    switch (property)
    {
      case XA_RGB_DEFAULT_MAP:
	if ((red_max == 0 || green_max == 0 || blue_max == 0) ||
	    (vinfo->class != PseudoColor && vinfo->class != DirectColor))
	{
	    XFree((char *) vinfo);
	    return 0;
	}
	break;
      case XA_RGB_RED_MAP:
	if ((vinfo->class != PseudoColor && vinfo->class != DirectColor)
	    || (red_max == 0))
	{	
	    XFree((char *) vinfo);
	    return 0;
	}
	green_max = blue_max = 0;
	break;
      case XA_RGB_GREEN_MAP:
	if ((vinfo->class != PseudoColor && vinfo->class != DirectColor) ||
	    (green_max == 0))
	{	
	    XFree((char *) vinfo);
	    return 0;
	}
	red_max = blue_max = 0;
      case XA_RGB_BLUE_MAP:	
	if ((vinfo->class != PseudoColor && vinfo->class != DirectColor) ||
	    blue_max == 0)
	{
	    XFree((char *) vinfo);
	    return 0;
	}
	red_max = green_max = 0;
	break;
      case XA_RGB_BEST_MAP:
	if (vinfo->class == GrayScale || vinfo->class == StaticGray ||
	    red_max == 0 || green_max == 0 || blue_max == 0)
	{
	    XFree((char *) vinfo);
	    return 0;
	}
	break;
      case XA_RGB_GRAY_MAP:
	if (vinfo->class == StaticColor || vinfo->class == TrueColor ||
	    red_max == 0)
	{
	    XFree((char *) vinfo);
	    return 0;
	}
	green_max = blue_max = 0;
	break;
      default:
	XFree((char *) vinfo);
	return 0;
    }

    /* fill in the standard colormap structure */
    colormap.colormap = (property == XA_RGB_DEFAULT_MAP)
     ? DefaultColormap(dpy, screen)
     : XCreateColormap(dpy, RootWindow(dpy, screen), vinfo->visual, AllocNone);
    colormap.red_max = red_max;
    colormap.red_mult = (red_max > 0) ? (green_max + 1) * (blue_max + 1) : 1;
    colormap.green_max = green_max;
    colormap.green_mult = (green_max > 0) ? blue_max + 1 : 1;
    colormap.blue_max = blue_max;
    colormap.blue_mult = 1;
    colormap.base_pixel = 0;	/* base pixel may change for RGB_DEFAULT_MAP */

    if (property == XA_RGB_DEFAULT_MAP)
	status = default_map(dpy, vinfo, &colormap);
    else if (vinfo->class == PseudoColor || vinfo->class == DirectColor ||
	     vinfo->class == GrayScale)
	status = rgb_map(dpy, vinfo, &colormap, property);
    else
	status = static_map(dpy, &colormap, property);

    if (status)
    {
	XGrabServer(dpy);
	if (replace || (! XGetStandardColormap(dpy, RootWindow(dpy, screen),
					       &stdmap, property)))
	    XSetStandardColormap(dpy, RootWindow(dpy, screen), &colormap,
				 property);
	XUngrabServer(dpy);
    }
    else
	XFreeColormap(dpy, colormap.colormap);

    XFree((char *) vinfo);
    return status;
}

static int compare(e1, e2)
    unsigned long	*e1, *e2;
{
    if (*e1 < *e2)
	return -1;
    if (*e1 > *e2)
	return 1;
    return 0;
}

static Status default_map(dpy, vinfo, colormap)
    Display		*dpy;
    XVisualInfo		*vinfo;
    XStandardColormap	*colormap;
{
    XColor		color;
    int			n;
    int			limit;
    int			i, r, g, gg;
    int			first_index;
    int			remainder;
    int 		count;
    int			*val;
    unsigned int	npixels;
    unsigned long	*pixels;
    unsigned long	pixel;
    unsigned long	ncolors;
    
    
    /* allocate the entire map so that we can free individual pixels */
    npixels = vinfo->colormap_size;
    pixels = (unsigned long *) calloc(npixels, sizeof(unsigned long));
    if (pixels == NULL)
	return 0;
    ncolors = (colormap->red_max + 1) * (colormap->green_max + 1) *
	(colormap->blue_max + 1);
    for (n=npixels; n >= ncolors; n--)
	if (XAllocColorCells(dpy, colormap->colormap, 1,
			     (unsigned long *) NULL, (unsigned int) 0,
			     pixels, (unsigned int) n))
	    break;
    
    if (n < ncolors)
	return 0;

    /* insure that pixel values are given in increasing order */
    qsort((char *) pixels, n, sizeof(unsigned long), compare);

    /* we have enough cells, now are they contiguous? */
    i = count = 1;
    remainder = n - 1;
    first_index = 0;
    while (count < ncolors && ncolors - count <= remainder)
    {
	if (pixels[i-1] + 1 == pixels[i])
	    count++;
	else
	{	count = 1;
		first_index = i;
	    }
	i++;
	remainder--;
    }
    if (count != ncolors)
    {	/* can't find enough contiguous cells, give up */
	XFreeColors(dpy, colormap->colormap, pixels, n, (unsigned long) 0);
	free((char *) pixels);
	return 0;
    }
    colormap->base_pixel = pixels[first_index];
    
    /* n = number of pixel allocations we actually got, pixels[0..(n-1)]
     * ncolors = number of colors we want to define
     * npixels = size of the colormap
     * ncolors <= n
     */
    
    limit = (colormap->red_max > colormap->green_max)
	? colormap->red_max : colormap->green_max;
    limit = (colormap->blue_max > limit) ? colormap->blue_max : limit;
    if ((val = (int *) calloc((unsigned int) limit, sizeof(int))) == NULL)
    {
	XFreeColors(dpy, colormap->colormap, pixels, n, (unsigned long) 0);
	free((char *) pixels);
	return 0;
    }
    val[0] = 0;
    for (i=1; i <= limit; i++)
	val[i] = val[i-1] + 65535;


    color.flags = DoRed | DoGreen | DoBlue;
    r = colormap->red_mult;
    g = colormap->green_mult;
    gg = colormap->green_max + 1;
    for (n=colormap->base_pixel+1, i=1; i < ncolors - 1; i++, n++)
    {
	color.pixel = pixel = (unsigned long) n;
	XFreeColors(dpy, colormap->colormap, &pixel, 1, (unsigned long) 0);
	color.red = (unsigned short) (val[i/r] / colormap->red_max);
	color.green = (unsigned short) (val[(i/g)%gg] / colormap->green_max);
	color.blue = (unsigned short) (val[i%g] / colormap->blue_max);
	
	if (! XAllocColor(dpy, colormap->colormap, &color))
	{
	    XFreeColors(dpy, colormap->colormap, pixels, n, (unsigned long) 0);
	    free((char *) pixels);
	    free((char *) val);
	    return 0;
	}
	if (color.pixel != pixel)
        {
	    XFreeColors(dpy, colormap->colormap, &(color.pixel), 1,
			(unsigned long) 0);
	    if (! XAllocColorCells(dpy, colormap->colormap, (Bool) 0,
				   (unsigned long *) NULL, (unsigned int) 0,
				   &pixel, (unsigned int) 1) 
		|| pixel != n)
	    {
		XFreeColors(dpy, colormap->colormap, pixels, n,
			    (unsigned long) 0);
		free((char *) pixels);
		free((char *) val);
		return 0;
	    }
	    color.pixel = pixel;
	    XStoreColors(dpy, colormap->colormap, &color, 1);
	}
    }
    
    /* We have a read-only RGB_COLOR_MAP - now free unused cells */
    /* free pixels occuring before the contiguous sequence begins */
    if (first_index)
	XFreeColors(dpy, colormap->colormap, pixels, first_index, 
		    (unsigned long) 0);
    /* free pixels following the contiguous sequence */
    if (remainder)
	XFreeColors(dpy, colormap->colormap,
		    &(pixels[first_index + ncolors]), remainder,
		    (unsigned long) 0);

    free((char *) pixels);
    free((char *) val);
    return 1;
}

static Status rgb_map(dpy, vinfo, colormap, property)
    Display		*dpy;
    XVisualInfo		*vinfo;
    XStandardColormap	*colormap;
    Atom		property;
{
    int			i, r, g, gg, n, ncolors, limit;
    Status		status;
    XColor		color;
    unsigned long	*pixels, pixel;
    unsigned int	npixels;
    int			*val;

    /* allocate the entire map so that we can free individual pixels */
    npixels = vinfo->colormap_size;
    pixels = (unsigned long *) calloc((unsigned) npixels,
				      sizeof(unsigned long));
    if (pixels == NULL)
	return 0;
    if (! XAllocColorCells(dpy, colormap->colormap, 1, (unsigned long *) NULL,
			   (unsigned int) 0, pixels, npixels))
    {
	free((char *) pixels);
	return 0;
    }

    /* insure that pixel values are given in increasing order */
    qsort((char *) pixels, npixels, sizeof(unsigned long), compare);
    
    /* we know that the pixels array contains the values [0..(mapsize -1)] 
     * ncolors = number of colors we want to define
     * npixels = size of the colormap
     * ncolors <= npixels, because we tested upon determining ncolors
     */
    
    ncolors = (colormap->red_max + 1) * (colormap->green_max + 1) *
	(colormap->blue_max + 1);
    limit = (colormap->red_max > colormap->green_max)
	? colormap->red_max : colormap->green_max;
    limit = (colormap->blue_max > limit) ? colormap->blue_max : limit;
    if ((val = (int *) calloc((unsigned int) limit, sizeof(int))) == NULL)
    {
	free((char *) pixels);
	return 0;
    }
    val[0] = 0;
    for (i=1; i <= limit; i++)
	val[i] = val[i-1] + 65535;
    
    r = colormap->red_mult;
    g = colormap->green_mult;
    gg = colormap->green_max + 1;
    for (n=colormap->base_pixel, i=0; i < ncolors; i++, n++)
    {
	color.pixel = pixel = (unsigned long) n;
	XFreeColors(dpy, colormap->colormap, &pixel, 1, (unsigned long) 0);
	
	switch (property)
	{
	  case XA_RGB_BEST_MAP:
	    color.red = (unsigned short) (val[i/r] / colormap->red_max);
	    color.green = (unsigned short) (val[(i/g)%gg] /
					    colormap->green_max);
	    color.blue = (unsigned short) (val[i%g] / colormap->blue_max);
	    break;
	  case XA_RGB_RED_MAP:
	    color.red = (unsigned short) (val[i] / colormap->red_max);
	    color.green = color.blue = 0;
	    break;
	  case XA_RGB_GREEN_MAP:
	    color.green = (unsigned short) (val[i] / colormap->green_max);
	    color.red = color.blue = 0;
	    break;
	  case XA_RGB_BLUE_MAP:
	    color.blue = (unsigned short) (val[i] / colormap->blue_max);
	    color.red = color.green = 0;
	    break;
	  case XA_RGB_GRAY_MAP:
	    color.blue = color.green = color.red =
		(unsigned short) (val[i] / colormap->red_max);
	    break;
	}
	
	status = XAllocColor(dpy, colormap->colormap, &color);
	if (status == 0 || color.pixel != (unsigned long) n)
	{
	    free((char *) pixels);
	    free((char *) val);
	    return 0;
	}
    }
    
    /* We have a read-only RGB_COLOR_MAP - now free unused cells */
    if (ncolors < npixels)
    {
	limit = npixels - ncolors;
	for (i=0; i < limit; i++)
	    pixels[i] = ncolors + i;
	XFreeColors(dpy, colormap->colormap, pixels, limit, (unsigned long) 0);
    }
    return 1;
	
}

static Status static_map(dpy, colormap, property)
    Display		*dpy;
    XStandardColormap	*colormap;
    Atom		property;
{
    int			i, last_pixel;
    XColor		color;

    last_pixel = (colormap->red_max + 1) * (colormap->green_max + 1) * 
	(colormap->blue_max + 1) + colormap->base_pixel - 1;

    for(i=colormap->base_pixel; i <= last_pixel; i++)
    {
	color.pixel = (unsigned long) i;
	color.red = (unsigned short)
	    (((i/colormap->red_mult) * 65535) / colormap->red_max);

	if (property == XA_RGB_BEST_MAP)
	{
	    color.green = (unsigned short)
		((((i/colormap->green_mult) % (colormap->green_max + 1)) *
		  65535) / colormap->green_max);
	    color.blue = (unsigned short)
		(((i%colormap->green_mult) * 65535) / colormap->blue_max);
	}
	else
	    color.green = color.blue = color.red;

	XAllocColor(dpy, colormap->colormap, &color);
	if (color.pixel != (unsigned long) i)
	    return 0;
    }
    return 1;
}
