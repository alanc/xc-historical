#include <X11/copyright.h>

/* Copyright 1987 Massachusetts Institute of Technology */

/*
 * xwd.c MIT Project Athena, X Window system window raster image dumper.
 *
 * This program will dump a raster image of the contents of a window into a 
 * file for output on graphics printers or for other uses.
 *
 *  Author:	Tony Della Fera, DEC
 *		17-Jun-85
 * 
 *  Modification history:
 *
 *  11/14/86 Bill Wyatt, Smithsonian Astrophysical Observatory
 *    - Removed Z format option, changing it to an XY option. Monochrome 
 *      windows will always dump in XY format. Color windows will dump
 *      in Z format by default, but can be dumped in XY format with the
 *      -xy option.
 *
 *  11/18/86 Bill Wyatt
 *    - VERSION 6 is same as version 5 for monchrome. For colors, the 
 *      appropriate number of Color structs are dumped after the header,
 *      which has the number of colors (=0 for monochrome) in place of the
 *      V5 padding at the end. Up to 16-bit displays are supported. I
 *      don't yet know how 24- to 32-bit displays will be handled under
 *      the Version 11 protocol.
 *
 *  6/15/87 David Krikorian, MIT Project Athena
 *    - VERSION 7 runs under the X Version 11 servers, while the previous
 *      versions of xwd were are for X Version 10.  This version is based
 *      on xwd version 6, and should eventually have the same color
 *      abilities. (Xwd V7 has yet to be tested on a color machine, so
 *      all color-related code is commented out until color support
 *      becomes practical.)
 */

#ifndef lint
static char *rcsid_xwd_c = "$Header: xwd.c,v 1.24 87/08/19 20:30:09 dkk Locked $";
#endif

/*%
 *%    This is the format for commenting out color-related code until
 *%  color can be supported.
%*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>
#include <stdio.h>
#include <strings.h>

#define FAILURE 0

#define FEEP_VOLUME 0

/* Include routines to do parsing */
#include "dsimple.h"

/* Setable Options */

int format = XYPixmap;
Bool nobdrs = False;
Bool standard_out = True;
Bool debug = False;

extern int (*_XErrorFunction)();
extern int _XDefaultError();

main(argc, argv)
    int argc;
    char **argv;
{
    register i;
    Window target_win;
    FILE *out_file = stdout;

    INIT_NAME;

    Setup_Display_And_Screen(&argc, argv);
    if (DisplayPlanes(dpy, 0) > 1)
        format = ZPixmap;

    /* Get window select on command line, if any */
    target_win = Select_Window_Args(&argc, argv);

    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-nobdrs")) {
	    nobdrs = True;
	    continue;
	}
	if (!strcmp(argv[i], "-debug")) {
	    debug = True;
	    continue;
	}
	if (!strcmp(argv[i], "-help"))
	  usage();
	if (!strcmp(argv[i], "-out")) {
	    if (++i >= argc) usage();
	    if (!(out_file = fopen(argv[i], "w")))
	      Error("Can't open output file as specified.");
	    standard_out = False;
	    continue;
	}
	if (!strcmp(argv[i], "-xy")) {
	    format = XYPixmap;
	    continue;
	}
	usage();
    }
    
    /*
     * Let the user select the target window.
     */
    if (!target_win)
      target_win = Select_Window(dpy);

    /*
     * Dump it!
     */
    Window_Dump(target_win, out_file);

    fclose(out_file);
}


/*
 * Window_Dump: dump a window to a file which must already be open for
 *              writting.
 */

char *calloc();

#include "X11/XWDFile.h"

Window_Dump(window, out)
     Window window;
     FILE *out;
{
    XColor *pixcolors;
    unsigned buffer_size;
    int win_name_size;
    int header_size;
    int ncolors = 0;
    char win_name[100];
    XWindowAttributes win_info;
    XImage *image;

    XWDFileHeader header;

    
    /*
     * Inform the user not to alter the screen.
     */
    Beep();

    /*
     * Get the parameters of the window being dumped.
     */
    if (debug) outl("xwd: Getting target window information.\n");
    if(!XGetWindowAttributes(dpy, window, &win_info)) 
      Fatal_Error("Can't get target window attributes.");

    XFetchName(dpy, window, win_name);
    if (!win_name[0])
      strcpy(win_name, "xwdump");

    /* sizeof(char) is included for the null string terminator. */
    win_name_size = strlen(win_name) + sizeof(char);

    /*
     * Snarf the pixmap with XGetImage.
     * Color windows get snarfed in Z format first to check the color
     * map allocations before resnarfing if XY format selected.
     */

    if (nobdrs) {
      	if (debug) outl("xwd: Image without borders selected.\n");
	image = XGetImage ( dpy, window, 0, 0, win_info.width,
			   win_info.height, ~0, format); 
      }
    else {
	if (debug) outl("xwd: Image with borders selected.\n");
	image = XGetImage ( dpy, window,
			   -win_info.border_width, -win_info.border_width, 
			   win_info.width + (win_info.border_width << 1),
			   win_info.height + (win_info.border_width << 1),
			   ~0, format); 
      }
    if (debug) outl("xwd: Getting pixmap.\n");

    /*
     * Determine the pixmap size.
     */
    buffer_size = Image_Size(image);

    /*
     * Get XColors of all pixels used in the pixmap
     */

    if (debug) outl("xwd: Getting Colors.\n");

    ncolors = Get_XColors(win_info, &pixcolors);

    /*
     * Inform the user that the image has been retrieved.
     */
    XBell(dpy, FEEP_VOLUME);
    XBell(dpy, FEEP_VOLUME);
    XFlush(dpy);

    /*
     * Calculate header size.
     */
    if (debug) outl("xwd: Calculating header size.\n");
    header_size = sizeof(header) + win_name_size;

    /*
     * Write out header information.
     */
    if (debug) outl("xwd: Constructing and dumping file header.\n");
    header.header_size = header_size;
    header.file_version = XWD_FILE_VERSION;
    header.display_type = 0; /* DisplayType(dpy, screen);  [obsolete] */
    header.display_planes = DisplayPlanes(dpy, screen);
    header.pixmap_format = format;
    header.window_width = win_info.width;
    header.window_height = win_info.height;
    if (nobdrs) {
      header.pixmap_width = win_info.width;
      header.pixmap_height = win_info.height;
      header.window_x = win_info.x + win_info.border_width;
      header.window_y = win_info.y + win_info.border_width;
    }
    else {
      header.pixmap_width = win_info.width + (win_info.border_width << 1);
      header.pixmap_height = win_info.height + (win_info.border_width << 1);
      header.window_x = win_info.x;
      header.window_y = win_info.y;
    }
    header.window_bdrwidth = win_info.border_width;
    header.window_ncolors = 0;  /*%  = ncolors;  %*/

    (void) fwrite((char *)&header, sizeof(header), 1, out);
    (void) fwrite(win_name, win_name_size, 1, out);

    /*
     * Write out the color maps, if any
     */

    if (debug) outl("xwd: Dumping %d colors.\n",ncolors);
    (void) fwrite(pixcolors, sizeof(XColor), ncolors, out);

    /*
     * Write out the buffer.
     */
    if (debug) outl("xwd: Dumping pixmap.  bufsize=%d\n",buffer_size);

    /*
     *    This copying of the bit stream (data) to a file is to be replaced
     *  by an X server call which hasn't been written yet.  It is not clear
     *  what other functions of xwd will be taken over by this (as yet)
     *  non-existant X function.
     */
    (void) fwrite(image->data, (int) buffer_size, 1, out);

    /*
     * free the color buffer.
     */

    if(debug && ncolors > 0) outl("xwd: Freeing color map.\n");
    if(ncolors > 0) free(pixcolors);

    /*
     * Free window name string.
     */
    if (debug) outl("xwd: Freeing window name string.\n");
    free(win_name);

    /*
     * Free image
     */
    XDestroyImage(image);
}

/*
 * Report the syntax for calling xwd.
 */
usage()
{
    printf("%s: %s [-debug] [-help] %s [-nobdrs] [-out <file>]",
	   program_name, SELECT_USAGE);
    printf(" [-xy] [[host]:vs]\n");
    exit(1);
}


/*
 * Error - Fatal xwd error.
 */
extern int errno;

Error(string)
	char *string;	/* Error description string. */
{
	outl("\nxwd: Error => %s\n", string);
	if (errno != 0) {
		perror("xwd");
		outl("\n");
	}

	exit(1);
}


/*
 * Determine the pixmap size.
 */
#ifdef OLD_JUNK
#define UBPS (sizeof(short)/2) /* useful bytes per short */
#define BitmapSize(width, height) (((((width) + 15) >> 3) &~ 1) * (height) * UBPS)
#define XYPixmapSize(width, height, planes) (BitmapSize(width, height) * (planes))
#define BZPixmapSize(width, height) ((width) * (height))
#define WZPixmapSize(width, height) (((width) * (height)) << 1)
#endif

int Image_Size(image)
     XImage *image;
{
    if (format != ZPixmap)
      return(image->bytes_per_line * image->height * image->depth);

    return(image->bytes_per_line * image->height);

#ifdef OLD_JUNK
	int buffer_size;
	int width, height;

	width = image->width;
	height = image->height;

	if (format == XYBitmap)
	  buffer_size = BitmapSize(width, height);

#ifdef COLOR

	else if (format == XYPixmap) {
	    buffer_size = XYPixmapSize(width, height,
				       DisplayPlanes(dpy, screen));
	    if (debug)
	      outl("xwd: Pixmap in XYFormat, size %d bytes.\n", buffer_size);
	}
	else if (DisplayPlanes(dpy, screen) < 9) {
	    buffer_size = BZPixmapSize(width, height);
	    if (debug)
	      outl("xwd: Pixmap in byte ZFormat, size %d bytes.\n",
		   buffer_size);
	}
	else {
	    buffer_size = WZPixmapSize(width, height);
	    if (debug)
	      outl("xwd: Pixmap in word ZFormat, size %d bytes.\n",
		   buffer_size);
    }
#endif

	return(buffer_size);
#endif
}


/*
 * Get the XColors of all pixels in image - returns # of colors
 */
int Get_XColors(win_info, pixcolors)
     XWindowAttributes win_info;
     XColor **pixcolors;
{
    int i, ncolors;
#ifndef COLOR
    return(0);
#endif

    ncolors = 1 << (win_info.depth);

    if (!(*pixcolors = (XColor *) malloc( sizeof(XColor) * ncolors )))
      Fatal_Error("Out of memory!");

    for (i=0; i<ncolors; i++)
      (*pixcolors)[i].pixel = i;

    XQueryColors(dpy, win_info.colormap, *pixcolors, ncolors);
    
    return(ncolors);
}

#ifdef OLD_JUNK

int Get_XColors(buffer, pixcolors, buffer_size)
     char *buffer;
     XColor *pixcolors[];       /* RETURNED */
     int buffer_size;
{
    register int i, *histbuffer;
    register u_short *wbuffer;
    register char *buffer;
    int ncolors = 0;

    if (DisplayPlanes(dpy,screen)<2)
      return(0);

#ifdef COLOR

    if(DisplayPlanes(dpy, screen) < 9) {
	histbuffer = (int *)calloc(256, sizeof(int));
	bzero(histbuffer, 256*sizeof(int));
	*pixcolors = (XColor *)calloc(1, sizeof(XColor));
	for(i=0; i<buffer_size; i++) {
	    /* if previously found, skip color query */
	    if(histbuffer[(int)buffer[i]] == 0) {
		*pixcolors = 
		  (XColor *)realloc(*pixcolors, sizeof(XColor)*(++ncolors));
		if(debug)
		  outl("Color %3d at pixel val %5d, i= %5d =",
		       ncolors, buffer[i], i);
		histbuffer[(int)buffer[i]]++;
		(*pixcolors)[ncolors-1].pixel = (int)buffer[i];
		if(XQueryColor(dpy, pixcolors[ncolors-1]) == 0) 
		  Fatal_Error("Unable to query color table?");
		if(debug) outl("%5d %5d %5d\n",
			       (*pixcolors)[ncolors-1].red,
			       (*pixcolors)[ncolors-1].green,
			       (*pixcolors)[ncolors-1].blue);
	    }
	}
    }
    else if(DisplayPlanes(dpy, screen) < 17) {
	wbuffer = (u_short *)buffer;
	histbuffer = (int *)calloc(65536, sizeof(int));
	bzero(histbuffer, 65536*sizeof(int));
	*pixcolors = (XColor *)calloc(1, sizeof(XColor));
	for(i=0; i<(buffer_size/sizeof(u_short)); i++) {

	    /* if previously found, skip color query */

	    if(histbuffer[(int)wbuffer[i]] == 0) {
		*pixcolors = 
		  (XColor *)realloc(*pixcolors, sizeof(XColor)*(++ncolors));
		if(debug)
		  outl("Color %2d at pixel val %d, i= %d =",
		       ncolors, wbuffer[i], i);
		histbuffer[(int)wbuffer[i]]++;
		(*pixcolors)[ncolors-1].pixel = (int)wbuffer[i];
		if(XQueryColor(dpy, pixcolors[ncolors-1]) == 0) 
		  Fatal_Error("Unable to query color table?");
		if(debug) outl("%d %d %d\n",
			       (*pixcolors)[ncolors-1].red,
			       (*pixcolors)[ncolors-1].green,
			       (*pixcolors)[ncolors-1].blue);
	    }
	}
    } 

    else
      if(DisplayPlanes(dpy, screen) > 16)
	Fatal_Error("Unable to handle more than 16 planes at this time");
    
    free(histbuffer);

#endif
    
    return(ncolors);
}
#endif
