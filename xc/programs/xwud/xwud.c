/* 
 * $Locker: chariot $ 
 */ 
static char	*rcsid = "$Header: xwud.c,v 1.6 87/06/23 14:41:07 chariot Locked $";
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
static char *rcsid_xwud_c = "$Header: xwud.c,v 1.6 87/06/23 14:41:07 chariot Locked $";
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

#define UBPS (sizeof(short)/2) /* useful bytes per short */
#define BitmapSize(width, height) (((((width) + 15) >> 3) &~ 1) * (height) * UBPS)
#define XYPixmapSize(width, height, planes) (BitmapSize(width, height) * (planes))
#define BZPixmapSize(width, height) ((width) * (height))
#define WZPixmapSize(width, height) (((width) * (height)) << 1)


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
    Display *dpy;
    XImage *image;
    XImage *XCreateImage();
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
    unsigned buffer_size, total_buffer_size;
    unsigned long gc_value_mask = 0;
    unsigned long *cplanes, *cpixels;
    long tmp;
    int win_name_size;
    char *str_index;
    char *file_name;
    char display[256];
    char *win_name;
    Bool standard_in = True;
    Bool newcolors = False, debug = False, inverse = False;

    XColor *pixcolors, *newpixcolors;
    Window image_win;
    Pixmap image_pixmap;
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
	if((header.file_version == 5 ||header.file_version == 6)
	   && header.display_planes == 1)
	  fprintf(stderr,"\n      (monochrome works anyway)\n");
	else Error("exiting.");
      }

    colormap = DefaultColormap(dpy, screen);
    if((planeno = DisplayPlanes(dpy, screen)) < header.display_planes)
      Error("Windump has more planes than display.");

    /*
     * Check to see if we are in the right pixmap format for the
     * display type.
     */
    if ((planeno != 1) && ((header.pixmap_format != XYPixmap))){
	Error(
	 "Windump is in ZFormat which is not valid on a monochrome display.");
    }
    else if ((planeno == 1) && ((header.pixmap_format != XYBitmap))) {
      header.pixmap_format = XYBitmap;
      /*  This is a terrible kludge.  xwd should give the right format
	  value, but do to some confusion, monochrome windows (bitmaps)
	  are being dumped as Z-format pixmaps.                    %%*/
    }

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

    /*
     * Determine the pixmap size.
     */
    if (header.pixmap_format == XYBitmap) {
        total_buffer_size = buffer_size = BitmapSize(
	    header.pixmap_width,
	    header.pixmap_height);
    }
    else if (header.pixmap_format == XYPixmap) {
	buffer_size =
	  XYPixmapSize(
	    header.pixmap_width,
	    header.pixmap_height,
	    header.display_planes);
	total_buffer_size = 
	  XYPixmapSize(
	    header.pixmap_width,
	    header.pixmap_height,
	    planeno );
    }
    else if (header.display_planes < 9) {
	total_buffer_size = buffer_size = BZPixmapSize(
	    header.pixmap_width,
	    header.pixmap_height
	);
    }
    else if(header.display_planes < 17) {
	total_buffer_size = buffer_size = WZPixmapSize(
	    header.pixmap_width,
	    header.pixmap_height
	);
    }
    else {
	Error("Can't undump pixmaps more than 16 bits deep.\n");
    } 


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
    if((buffer = calloc(total_buffer_size, 1)) == NULL)
      Error("Can't calloc data buffer.");
    bzero(buffer,total_buffer_size);

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


    /*
     * Create the image window.
     */

    attributes.override_redirect = True;
    attributes.background_pixmap = BlackPixel(dpy, screen);

    image_win = XCreateWindow(dpy,
	RootWindow(dpy, screen),
	header.window_x, header.window_y,
	header.pixmap_width, header.pixmap_height,
	0, 0, CopyFromParent, CopyFromParent,
	CWOverrideRedirect + CWBackPixmap, &attributes);

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

    /*
     * Set up a while loop to maintain the image.
     */
    while (True) {
	int i, nbytes;
	/*
	 * Wait on mouse input event to terminate.
	 */
	XNextEvent(dpy, &event);
	if (event.type == ButtonPress) break;

	gc_value_mask |= GCForeground;
	gc_value_mask |= GCBackground;

	gc_val.foreground = (unsigned long) WhitePixel (dpy, screen); 
	                                                 /*  Default  */
	gc_val.background = (unsigned long) BlackPixel (dpy, screen); 
	                                                 /*  Default  */
	gc = XCreateGC (dpy, image_win, gc_value_mask, &gc_val);

	visual = DefaultVisual(dpy, screen);
	offset = 0;

	image = XCreateImage(dpy, visual, 1 /* depth %%*/,
		     header.pixmap_format, offset, buffer,
		     header.pixmap_width, header.pixmap_height,
		     dpy->bitmap_pad, 0);

	switch((int)event.type) {
/*	  case ExposeWindow: %*/ /* Copy the data into the window. */
	  case Expose:  /* simpler to copy from x=0 for full width */
/*   %%*/
	    if(header.pixmap_format == XYBitmap) {
	        onebufsize = BitmapSize(header.pixmap_width,
					header.pixmap_height);
		nbytes = BitmapSize(header.pixmap_width, 1);
	        forepixel = WhitePixel(dpy, screen);
		backpixel = BlackPixel(dpy, screen);
		planes = AllPlanes;
		for(j=0; j<header.display_planes; j++) {	
		    if(header.display_planes > 1)
			planes >>= 1; /* shift down a bit */
/* (just once:)		    for(i=0; i<xevent->height; i+=100) {  %%*/
		      if(inverse) {
			tmp = gc_val.foreground;
/*  Here we reverse video.
%*/
			gc_val.foreground = gc_val.background;
			gc_val.background = tmp;

			XChangeGC(dpy, gc, gc_value_mask, &gc_val);
			XPutImage(dpy, image_win, gc, image, 0, 0, 0, 0,
				       (unsigned int) header.pixmap_width, 
				       (unsigned int) header.pixmap_height);
		      }
		      else {
			XPutImage(dpy, image_win, gc, image, 0, 0, 0, 0,
				       (unsigned int) header.pixmap_width, 
				       (unsigned int) header.pixmap_height);
		      }
/*		  } %%*/
	        }
	      }
	    else if(header.pixmap_format == XYPixmap) {
		onebufsize =  /* size of each bitmap */
		  XYPixmapSize(header.pixmap_width,
			       header.pixmap_height, 1);
		nbytes = BitmapSize(header.pixmap_width,1);
/*		if(header.display_planes > 1) {
		    forepixel = -1;
		    backpixel = 0;
		    planes = 1<<(planeno);%*/ /* MSB << 1 */
/*		}
		else {
		    forepixel = WhitePixel(dpy, screen);
		    backpixel = BlackPixel(dpy, screen);
		    planes = AllPlanes;

		}
%*/
		for(j=0; j<header.display_planes; j++) {	
		    if(header.display_planes > 1)
			planes >>= 1; /* shift down a bit */
		    for(i=0; i<xevent->height; i+=100)
		      if(inverse) {
			tmp = gc_val.foreground;
/*  Here we reverse video.
%*/
			gc_val.foreground = gc_val.background;
			gc_val.background = tmp;

			XChangeGC(dpy, gc, gc_value_mask, &gc_val);
			XPutImage(dpy, image_win, gc, image, 0, 0, 0, 0,
				       header.pixmap_width, 
				       header.pixmap_height);
		      }
		      else
			XPutImage(dpy, image_win, gc, image, 0, 0, 0, 0,
				       header.pixmap_width, 
				       header.pixmap_height);

/*			XBitmapBitsPut(image_win,
				       0, i + xevent->y,
				       header.pixmap_width, 
				       MIN(100, xevent->height - i),
				       buffer+((i+xevent->y)* nbytes)
				          + (onebufsize * j), 
				       forepixel, backpixel,
				       0, GXcopy, planes);
%*/
		}
	    } 
	    else if(planeno < 9) {
/*		nbytes = BZPixmapSize(header.pixmap_width,1);
		for(i=0; i<xevent->height; i+=100)
		  XPixmapBitsPutZ(image_win, 
				  0, i + xevent->y,
				  header.pixmap_width,
				  MIN(100, xevent->height - i),
				  buffer+((i+xevent->y)* nbytes),
				  0, GXcopy, AllPlanes);
%*/
	    }
	    else {  /* Display Planes > 8 */
/*		nbytes = WZPixmapSize(header.pixmap_width, 1);
		for(i=0; i<xevent->height; i+=100)
		  XPixmapBitsPutZ(image_win, 
				  0, i + xevent->y,
				  header.pixmap_width,
				  MIN(50, xevent->height - i),
				  buffer+((i+xevent->y)* nbytes),
				  0, GXcopy, AllPlanes);
%*/
	    }
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
