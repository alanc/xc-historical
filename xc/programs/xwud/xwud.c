/* 
 * $Locker: rws $ 
 */ 
static char	*rcsid = "$Header: xwud.c,v 1.11 87/09/05 22:35:26 rws Locked $";
#include <X11/copyright.h>

/* Copyright 1985, 1986, Massachusetts Institute of Technology */

/*
 * xwud.c - MIT Project Athena, X Window system window raster image
 *	    undumper.
 *
 * This program will read a raster image of a window from stdin or a file
 * and display it on an X display.
 *
 *  Author:	Tony Della Fera, DEC
 *
 *  Modified 11/14/86 by William F. Wyatt,
 *                        Smithsonian Astrophysical Observatory
 *    allows writing of monochrome XYFormat window dump files on a color
 *    display, using default WhitePixel for 1's and BlackPixel for 0's.
 *
 *  Modified 11/20/86 WFW
 *    VERSION 6 - same as V5 for monochrome, but expects color map info
 *    in the file for color images. Checks to see if the requested
 *    colors are already in the display's map (e.g. if the window dump
 *    and undump are contemporaneous to the same display). If so,
 *    undump immediately. If not, request new colors, alter the 
 *    pixels to the new values, then write the pixmap. Note that
 *    multi-plane XY format undumps don't work if the pixel values
 *    corresponding to the requested colors have to be changed.
 */

#ifndef lint
static char *rcsid_xwud_c = "$Header: xwud.c,v 1.11 87/09/05 22:35:26 rws Locked $";
#endif

#include <X11/Xlib.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
extern char *calloc();
#include "dsimple.h"

#include <X11/XWDFile.h>   /*  Note:  XWDFile.h (in xwd now) 
			        must be moved to include (or linked) %%*/

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define ABS(a) (a) < 0 ? -(a) : (a)

#define FAILURE 0

extern int errno;

usage()
{
    outl("%s: %s [-help][-debug][-inverse][-in <file>][[host]:vs]\n",
	 program_name, program_name);
    exit(1);
}

main(argc, argv)
    int argc;
    char **argv;
{
    register int i;
    register int *histbuffer;
    register u_short *wbuffer;
    XImage image;
    XSetWindowAttributes attributes;
    Visual *visual;
    register char *buffer;

    int j, status;
    int onebufsize;
    int planes;
    int forepixel;
    int backpixel;
    int planeno;      /*  Number of planes from DisplayPlanes() macro */
    int offset;
    unsigned buffer_size;
    unsigned long gc_value_mask = 0;
    unsigned long *cplanes, *cpixels;
    long tmp;
    int win_name_size;
    char *str_index;
    char *file_name;
    char *win_name;
    Bool standard_in = True;
    Bool newcolors = False, debug = False, inverse = False;

    XColor *pixcolors, *newpixcolors;
    Window image_win;
    Colormap colormap;
    XEvent event;
    register XExposeEvent *xevent = (XExposeEvent *)&event;

    GC gc;
    XGCValues gc_val;

    XWDFileHeader header;

    FILE *in_file = stdin;

    INIT_NAME;

    Setup_Display_And_Screen(&argc, argv);

    for (i = 1; i < argc; i++) {
	str_index = (char *) index (argv [i], '-');
	if (str_index == NULL) usage();
	if (strncmp(argv[i], "-help", 5) == 0) {
	    usage();
	}
	if (strncmp(argv[i], "-in", 4) == 0) {
	    if (++i >= argc) usage();
	    file_name = argv[i];
	    standard_in = False;
	    continue;
	}
	if(strcmp(argv[i], "-inverse") == 0) {
	    inverse = True;
	    continue;
	}
	if(strcmp(argv[i], "-debug") == 0) {
	    debug = True;
	    continue;
	}
	usage();
    }
    
    if (!standard_in) {
	/*
	 * Open the output file.
	 */
	in_file = fopen(file_name, "r");
	if (in_file == NULL) {
	    Error("Can't open output file as specified.");
	}
    }
    
    /*
     * Read in header information.
     */
    if(fread((char *)&header, sizeof(header), 1, in_file) != 1)
      Error("Unable to read dump file header.");

    /*
     * check to see if the dump file is in the proper format.
     */
    if (header.file_version != XWD_FILE_VERSION) {
	fprintf(stderr,"xwud: XWD file format version missmatch.");
	Error("exiting.");
      }

    visual = DefaultVisual(dpy, screen);
    colormap = DefaultColormap(dpy, screen);

    /* XXX check for supported window depth */

    /*
     * Calloc window name.
     */
    win_name_size = ABS(header.header_size - sizeof(header));
    if((win_name = calloc((unsigned) win_name_size, sizeof(char))) == NULL)
      Error("Can't calloc window name storage.");

    /*
     * Read in window name.
     */
    if(fread(win_name, sizeof(char), win_name_size, in_file) != win_name_size)
      Error("Unable to read window name from dump file.");

    image.width = header.pixmap_width;
    image.height = header.pixmap_height;
    image.xoffset = header.xoffset;
    image.format = header.pixmap_format;
    image.byte_order = header.byte_order;
    image.bitmap_unit = header.bitmap_unit;
    image.bitmap_bit_order = header.bitmap_bit_order;
    image.bitmap_pad = header.bitmap_pad;
    image.depth = header.pixmap_depth;
    image.bits_per_pixel = header.bits_per_pixel;
    image.bytes_per_line = header.bytes_per_line;
    image.red_mask = header.red_mask;
    image.green_mask = header.green_mask;
    image.blue_mask = header.blue_mask;
    image.obdata = NULL;
    _XInitImageFuncPtrs(&image);


    /* Calloc the color map buffer.
     * Read it in, copy it and use the copy to query for the
     * existing colors at those pixel values.
     */
    if(header.window_ncolors) {
	pixcolors = (XColor *)calloc(header.window_ncolors,sizeof(XColor));
	if(fread(pixcolors,sizeof(XColor),header.window_ncolors, in_file)
	   != header.window_ncolors)
	  Error("Unable to read color map from dump file.");
	if(debug)
	  fprintf(stderr,"Read %d colors\n", header.window_ncolors);
	newpixcolors = (XColor *)calloc(header.window_ncolors,sizeof(XColor));
	bcopy(pixcolors, newpixcolors, sizeof(XColor)*header.window_ncolors);
	if(XQueryColors(dpy, colormap, newpixcolors, header.window_ncolors) == 0)
	  Error("Can't query the color map?");
	for(i=0; i<header.window_ncolors; i++)
	  if(!ColorEqual(&pixcolors[i], &newpixcolors[i])) {
	      newcolors = True;
	      break;
	  }
	if(debug) {
	    if(newcolors)  fprintf(stderr,"New colors needed\n");
	    else fprintf(stderr,"Old colors match!\n");
	}
    }

    /*
     * Calloc the pixel buffer.
     */
    buffer_size = Image_Size(&image);
    if((buffer = calloc(buffer_size, 1)) == NULL)
      Error("Can't calloc data buffer.");
    image.data = buffer;

    /*
     * Read in the pixmap buffer.
     */
    if((status = fread(buffer, sizeof(char), (int)buffer_size, in_file))
       != buffer_size){
/*  Add elaboration on error here. %%*/
      Error("Unable to read pixmap from dump file.");
    }
    /*
     * Close the input file.
     */
    (void) fclose(in_file);

#ifdef notdef
    /*
     * If necessary, get and store the new colors, convert the pixels to the
     * new colors appropriately.
     */
    if(newcolors) {
	cpixels = (unsigned long *)calloc(header.window_ncolors+1,sizeof(int));
	if(XAllocColorCells(dpy, colormap, 0, cplanes, 0, cpixels, 
	     (unsigned int) header.window_ncolors) == 0)

/*  Old arguments (for XGetColorCells() in X10) were: 
               0, header.window_ncolors, 0, &cplanes, cpixels %%*/

	  Error("Can't allocate colors.");
	for(i=0; i<header.window_ncolors; i++) {
	    newpixcolors[i].pixel = cpixels[i];
	    newpixcolors[i].red   = pixcolors[i].red;
	    newpixcolors[i].green = pixcolors[i].green;
	    newpixcolors[i].blue  = pixcolors[i].blue;
	    if(debug) 
	      fprintf(stderr,"Pixel %4d, r = %5d  g = %5d  b = %5d\n",
		      newpixcolors[i].pixel, newpixcolors[i].red,
		      newpixcolors[i].green, newpixcolors[i].blue);
	}
	XStoreColors(header.window_ncolors, newpixcolors);

	/* now, make a lookup table to convert old pixels into the new ones*/
	if(header.pixmap_format == ZPixmap) {
	    if(header.display_planes < 9) {
		histbuffer = (int *)calloc(256, sizeof(int));
		bzero(histbuffer, 256*sizeof(int));
		for(i=0; i<header.window_ncolors; i++)
		  histbuffer[pixcolors[i].pixel] = newpixcolors[i].pixel;
		for(i=0; i<buffer_size; i++)
		  buffer[i] = histbuffer[buffer[i]];
	    }
	    else if(header.display_planes < 17) {
		histbuffer = (int *)calloc(65536, sizeof(int));
		bzero(histbuffer, 65536*sizeof(int));
		for(i=0; i<header.window_ncolors; i++)
		  histbuffer[pixcolors[i].pixel] = newpixcolors[i].pixel;
		wbuffer = (u_short *)buffer;
		for(i=0; i<(buffer_size/sizeof(u_short)); i++)
		  wbuffer[i] = histbuffer[wbuffer[i]];
	    } 
	    else if(header.display_planes > 16) {
		Error("Unable to handle more than 16 planes at this time");
	    }
	    free(histbuffer);
	}
	free(cpixels);
	bcopy(newpixcolors, pixcolors, sizeof(XColor)*header.window_ncolors);
	free(newpixcolors);
    }
#endif


    /*
     * Create the image window.
     */

    attributes.override_redirect = True;
    attributes.background_pixel = BlackPixel(dpy, screen);

    image_win = XCreateWindow(dpy,
	RootWindow(dpy, screen),
	header.window_x, header.window_y,
	header.pixmap_width, header.pixmap_height,
	0, header.pixmap_depth, InputOutput, visual,
	CWOverrideRedirect + CWBackPixel, &attributes);

    if (image_win == FAILURE) Error("Can't create image window.");

    /*
     * Select mouse ButtonPressed on the window, this is how we determine
     * when to stop displaying the window.
     */
    XSelectInput(dpy,image_win, (ButtonPressMask | ExposureMask));
     
    /*
     * Store the window name string.
     */
    XStoreName(dpy, image_win, win_name);
    
    /*
     * Map the image window.
     */
    XMapWindow(dpy, image_win);

    gc_value_mask |= GCForeground;
    gc_value_mask |= GCBackground;

    gc_val.foreground = (unsigned long) WhitePixel (dpy, screen); 
						     /*  Default  */
    gc_val.background = (unsigned long) BlackPixel (dpy, screen); 
						     /*  Default  */
    gc = XCreateGC (dpy, image_win, gc_value_mask, &gc_val);

    /*
     * Set up a while loop to maintain the image.
     */

    while (True) {
	/*
	 * Wait on mouse input event to terminate.
	 */
	XNextEvent(dpy, &event);
	if (event.type == ButtonPress) break;

	switch((int)event.type) {
	  case Expose:  /* simpler to copy from x=0 for full width */
	      XPutImage(dpy, image_win, gc, &image, 0, 0, 0, 0, image.width, image.height);
	}
    }

    /*
     * Destroy the image window.
     */
    XDestroyWindow(dpy, image_win);
    
    /*
     * Free the pixmap buffer.
     */
    free(buffer);

    /*
     * Free window name string.
     */
    free(win_name);
    exit(0);
}

/*
 * test two color map entries for equality
 */
ColorEqual(color1, color2)
     register XColor *color1, *color2;
{
    return(color1->pixel == color2->pixel &&
	   color1->red   == color2->red &&
	   color1->green == color2->green &&
	   color1->blue  == color2->blue);
}


int Image_Size(image)
     XImage *image;
{
    if (image->format != ZPixmap)
      return(image->bytes_per_line * image->height * image->depth);

    return(image->bytes_per_line * image->height);

}

/*
 * Error - Fatal xwud error.
 */
Error(string)
	char *string;	/* Error description string. */
{
	fprintf(stderr, "xwud: Error => %s\n", string);

	if (errno != 0) {
		perror("xwud");
		fprintf(stderr, "\n");
	}

	exit(1);
}

/* End of xwud.c */
