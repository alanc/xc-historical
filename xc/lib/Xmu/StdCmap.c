/* $XConsortium */

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
#define MAX_GREENS	256
#define MAX_BLUES	256

extern char	*calloc();
extern void	free();

static Status best_map();
static Status red_map();
static Status green_map();
static Status blue_map();
static Status gray_map();
static Status default_map();
static Status allocate();

/*	Create a standard colormap for the given screen and visual,
 *	with the given red, green, and blue maximum values, with the
 *	given standard property name.  If flag is true and the property
 *	is already defined, do not redefine it.  Return 0 on failure,
 *	non-zero on success.
 */

Status XmuStandardColormap(dpy, screen, visual, red_max, green_max, blue_max,
		      property, flag)
    Display		*dpy;
    int			screen; 
    Visual		*visual;
    unsigned long	red_max, green_max, blue_max;
    Atom		property;
    int			flag;	/* 0: replace; non-zero: do not replace */
{
    XStandardColormap	colormap;
    Status		status = 0;
    
    /* XXX ICCCM Xlib changes; and, should I check to see if the property
     * has the desired allocation?
     */
    if (flag && XGetStandardColormap(dpy, RootWindow(dpy, screen), &colormap,
				     property))
	return 1;

    colormap.colormap = NULL;
    colormap.red_max = red_max;
    if (red_max > 0)
	colormap.red_mult = (green_max + 1) * (blue_max + 1);
    else
	colormap.red_mult = 1;
    colormap.green_max = green_max;
    if (green_max > 0)
	colormap.green_mult = blue_max + 1;
    else
	colormap.green_mult = 1;
    colormap.blue_max = blue_max;
    colormap.blue_mult = 1;
    colormap.base_pixel = 0;

    switch (property)
    {
      case XA_RGB_BEST_MAP:
	status = best_map(dpy, screen, visual, &colormap);
	break;
      case XA_RGB_RED_MAP:
	status = red_map(dpy, screen, visual, &colormap);
	break;
      case XA_RGB_GREEN_MAP:
	status = green_map(dpy, screen, visual, &colormap);
	break;
      case XA_RGB_BLUE_MAP:
	status = blue_map(dpy, screen, visual, &colormap);
	break;
      case XA_RGB_GRAY_MAP:
	status = gray_map(dpy, screen, visual, &colormap);
	break;
      case XA_RGB_DEFAULT_MAP:
	status =default_map(dpy, screen, visual, &colormap);
	break;
    }
    
    if (status)	/* XXX ICCCM Xlib changes */
    {
	XStandardColormap	std_colormap;
	
	XGrabServer(dpy);
	if (! XGetStandardColormap(dpy, RootWindow(dpy, screen),
				   &std_colormap, property))
	    XSetStandardColormap(dpy, RootWindow(dpy, screen), &colormap,
				 property);
	XUngrabServer(dpy);
    }
    return status;
}

/*	Create an RGB_BEST_MAP standard colormap.
 */
static Status best_map(dpy, screen, visual, colormap)
    Display			*dpy;
    Visual			*visual;
    XStandardColormap	*colormap;
{
    register int	n, i, j, k;
    XColor		*color;
    

    if (colormap->green_max > MAX_GREENS || colormap->blue_max > MAX_BLUES)
	return 0;
    if (allocate(dpy, screen, visual, colormap, &color) == 0)
	return 0;
    
    if (visual->class == PseudoColor || visual->class == DirectColor)
    {
	unsigned short	red, green[MAX_GREENS], blue[MAX_BLUES];

	green[0] = blue[0] = 0;
	for (i=1; i <= colormap->green_max; i++)
	    green[i] = (unsigned short) ((i * 65535) / colormap->green_max);
	for (i=1; i <= colormap->blue_max; i++)
	    blue[i] = (unsigned short) ((i * 65535) / colormap->blue_max);
	
	n = 0;
	for (i=0; i <= colormap->red_max; i++) 
	{
	    red = (unsigned short) ((i * 65535) / colormap->red_max);
	    for (j=0; j <= colormap->green_max; j++)
		for (k=0; k <= colormap->blue_max; k++) 
		{
		    color[n].pixel = (unsigned long) n;
		    color[n].flags = DoRed | DoGreen | DoBlue;
		    color[n].red = red;
		    color[n].green = green[j];
		    color[n++].blue = blue[k];
		}
	}
	
	n = MIN(n, visual->map_entries);
	XStoreColors(dpy, colormap->colormap, color, n);
	free((char *) color);
	return 1;
    }
    else if (visual->class == StaticColor || visual->class == TrueColor)
    {
	unsigned short	red, green;
	
	XQueryColors(dpy, DefaultColormap(dpy, screen), color,
		     visual->map_entries);
	n = 0;
	for (i=0; i <= colormap->red_max; i++)
	{
	    red = (i * 65535) / colormap->red_max;
	    for (j=0; j <= colormap->green_max; j++)
	    {
		green = (j * 65535) / colormap->green_max;
		for (k=0; k <= colormap->blue_max; k++)
		{
		    if (color[n].red != red)
			goto failure;
		    if (color[n].green != green)
			goto failure;
		    if (color[n].blue != ((k * 65535) / colormap->blue_max))
			goto failure;
		    n++;
		}
	    }
	}
	free((char *) color);
	return 1;
    }
  failure:
    free((char *) color);
    return 0;
}

static Status red_map(dpy, screen, visual, colormap)
    Display		*dpy;
    int			screen;
    Visual		*visual;
    XStandardColormap	*colormap;
{
    register int	n;
    XColor		*color;

    if (allocate(dpy, screen, visual, colormap, &color) == 0)
	return 0;

    for (n=0; n <= colormap->red_max; n++) 
    {
	color[n].pixel = (unsigned long) n;
	color[n].flags = DoRed;
	color[n].red = (unsigned short) ((n * 65535) / colormap->red_max);
    }

    n = MIN(n, visual->map_entries);
    XStoreColors(dpy, colormap->colormap, color, n);
    free((char *) color);
    return 1;
}

static Status green_map(dpy, screen, visual, colormap)
    Display		*dpy;
    int			screen;
    Visual		*visual;
    XStandardColormap	*colormap;
{
    register int	n;
    XColor		*color;

    if (allocate(dpy, screen, visual, colormap, &color) == 0)
	return 0;

    for (n=0; n <= colormap->green_max; n++) 
    {
	color[n].pixel = (unsigned long) n;
	color[n].flags = DoGreen;
	color[n].green = (unsigned short) ((n * 65535) / colormap->green_max);
    }
    n = MIN(n, visual->map_entries);
    XStoreColors(dpy, colormap->colormap, color, n);
    free((char *) color);
    return 1;
}

static Status blue_map(dpy, screen, visual, colormap)
    Display		*dpy;
    int			screen;
    Visual		*visual;
    XStandardColormap	*colormap;
{
    register int	n;
    XColor		*color;

    if (allocate(dpy, screen, visual, colormap, &color) == 0)
	return 0;

    for (n=0; n <= colormap->blue_max; n++) 
    {
	color[n].pixel = (unsigned long) n;
	color[n].flags = DoBlue;
	color[n].blue = (unsigned short) ((n * 65535) / colormap->blue_max);
    }
    n = MIN(n, visual->map_entries);
    XStoreColors(dpy, colormap->colormap, color, n);
    free((char *) color);
    return 1;
}

static Status gray_map(dpy, screen, visual, colormap)
Display			*dpy;
int			screen;
Visual			*visual;
XStandardColormap	*colormap;
{
    register int	n;
    XColor		*color;

    if (allocate(dpy, screen, visual, colormap, &color) == 0)
	return 0;

    for (n=0; n <= colormap->red_max; n++) 
    {
	color[n].pixel = (unsigned long) n;
	color[n].flags = DoRed | DoGreen | DoBlue;
	color[n].red = (unsigned short) ((n * 65535) / colormap->red_max);
	color[n].green = color[n].blue = color[n].red;
    }
    n = MIN(n, visual->map_entries);
    XStoreColors(dpy, colormap->colormap, color, n);
    free((char *) color);
    return 1;
}

static Status default_map(dpy, screen, visual, colormap)
    Display		*dpy;
    int			screen;
    Visual		*visual;
    XStandardColormap	*colormap;
{
    XColor		*color;

/*	this won't work    
    if (allocate(dpy, screen, visual, colormap, &color) == 0)
	return 0;
 */    
    /* just like best map.  What about allocating from high end of map? */
    return 1;
}

static Status allocate(dpy, screen, visual, colormap, color)
    Display		*dpy;
    int			screen;
    Visual		*visual;
    XStandardColormap	*colormap;
    XColor		**color;
{
    unsigned int	n;
    Status		status;
    unsigned long	pixels;
    int			rbits, gbits, bbits;
    unsigned long	rmask, gmask, bmask;
    unsigned long	b;
    
    
    /* Create the colormap, allocate cells if it is read/write */

    switch (visual->class)
    {
      case PseudoColor:
      case GrayScale:
	colormap->colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
					     visual, AllocAll);
	/* XXX we don't always need/want to alloc all, e.g. gpx */
	break;
      case DirectColor:
	colormap->colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
					     visual, AllocNone);
	rbits = gbits = bbits = 0;
	for (b=colormap->red_max; b > 0; b>>=1)
	    rbits++;
	for (b=colormap->green_max; b > 0; b>>=1)
	    gbits++;
	for (b=colormap->blue_max; b > 0; b>>=1)
	    bbits++;
	status = XAllocColorPlanes(dpy, colormap->colormap, 1, &pixels, 0,
				   rbits, gbits, bbits,
				   &rmask, &gmask, &bmask);
	if (!status) return 0;
	break;
      case TrueColor:
      case StaticColor:
      case StaticGray:
	colormap->colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
					     visual, AllocNone);
	break;
      default:
	return 0;
    }
	    
    n = (colormap->red_max + 1) * (colormap->green_max + 1) *
	(colormap->blue_max + 1);
    if (((*color) = (XColor *) calloc(n, (unsigned) sizeof(XColor))) == NULL)
    {
	(void) fprintf(stderr, "XmuStandardColormap: out of memory\n");
	return 0;
    }
    return 1;
}
