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
static char *rcsid_xwd_c = "$Header: xwd.c,v 1.12 87/06/16 00:50:43 chariot Locked $";
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

char *calloc();

#include "XWDFile.h"

#define UBPS (sizeof(short)/2) /* useful bytes per short */
#define BitmapSize(width, height) (((((width) + 15) >> 3) &~ 1) * (height) * UBPS)
#define XYPixmapSize(width, height, planes) (BitmapSize(width, height) * (planes))

/*%
#define BZPixmapSize(width, height) ((width) * (height))
#define WZPixmapSize(width, height) (((width) * (height)) << 1)
%*/

#define FAILURE 0

#define FEEP_VOLUME 0

#define DONT_KNOW_YET 17

extern int errno;

/* Include routines to do parsing */
#include "jdisplay.h"

main(argc, argv)
    int argc;
    char **argv;
{
    register int i, *histbuffer;
    register u_short *wbuffer;
    register char *buffer, *cbuffer;
    
    unsigned buffer_size;
    unsigned int virt_width, virt_height;
    int virt_x, virt_y;
    int win_name_size;
    int header_size;
/*%    int ncolors = 0;   %*/
    int depth;
    int format;
    int offset;
    long plane_mask;
    char *file_name;
    char win_name[32];
    Bool nobdrs = False;
    Bool debug = False;
    Bool standard_out = True;

/*%
    XColor *scolor;
    XColor *bcolor;
    XColor *pixcolors;
%*/
    Window target_win;
    Window rootwin;
    XWindowAttributes win_info;
    XImage *image;

    XWDFileHeader header;

    FILE *out_file = stdout;

/*%    *bcolor = *scolor = DONT_KNOW_YET    %*/

    INIT_NAME;

    Setup_Display_And_Screen(&argc, argv);

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
	    file_name = argv[i];
	    standard_out = False;
	    continue;
	}
	if (!strcmp(argv[i], "-xy")) {
	    format = XYPixmap;
	    continue;
	}
	usage();
    }
    
    if (!standard_out) {
	/*
	 * Open the output file.
	 */
	if((out_file = fopen(file_name, "w")) == NULL)
	  Error("Can't open output file as specified.");
    }

    rootwin = RootWindow(dpy, screen);

    /*
     * Set the right pixmap format for the display type.
     */
    if(DisplayPlanes(dpy, screen) == 1) {
	format = XYBitmap;
    }
    else if(format != XYPixmap) {
	    format = ZPixmap;
    }

    /*
     * Let the user select the target window.
     */
    target_win = Select_Window(dpy);

    /*
     * Inform the user not to alter the screen.
     */
    Beep();

    /*
     * Get the parameters of the window being dumped.
     */
    if (debug) fprintf(stderr,"xwd: Getting target window information.\n");

    if(XGetWindowAttributes(dpy, target_win, &win_info) == FAILURE) 
      Fatal_Error("Can't query target window.\n");

    XFetchName(dpy, target_win, win_name);
    if (!win_name[0])
      strcpy(win_name, "xwdump");

    /*
     * sizeof(char) is included for the null string terminator.
     */
    win_name_size = strlen(win_name) + sizeof(char);

    /*
     * Calculate the virtual x, y, width and height of the window pane image
     * (this depends on whether or not the borders are included.
     */
    if (nobdrs) {
	if (debug) fprintf(stderr,"xwd: Image without borders selected.\n");
	virt_x = 0;
	virt_y = 0;
	virt_width = win_info.width;
	virt_height = win_info.height;
    }
    else {
	if (debug) fprintf(stderr,"xwd: Image with borders selected.\n");
	virt_x = win_info.x;
	virt_y = win_info.y;
	virt_width = win_info.width + (win_info.border_width << 1);
    	virt_height = win_info.height + (win_info.border_width << 1);
    }

    /*
     * Determine the pixmap size.
     */
    if (format == XYBitmap)
      buffer_size = BitmapSize(virt_width, virt_height);

/*%
    else if (format == XYPixmap) {
	buffer_size = XYPixmapSize(virt_width, virt_height,
				   DisplayPlanes(dpy, screen));
	if (debug) {
	    fprintf(stderr,
		    "xwd: Pixmap in XYFormat, size %d bytes.\n", buffer_size);
	}
    }
    else if (DisplayPlanes(dpy, screen) < 9) {
	buffer_size = BZPixmapSize(virt_width, virt_height);
	if (debug) {
	    fprintf(stderr,
	      "xwd: Pixmap in byte ZFormat, size %d bytes.\n", buffer_size);
	}
    }
    else {
	buffer_size = WZPixmapSize(virt_width, virt_height);
	if (debug) {
	    fprintf(stderr,
	      "xwd: Pixmap in word ZFormat, size %d bytes.\n", buffer_size);
	}
    }
%*/

    /*
     * Snarf the pixmap with XGetImage.
     * Color windows get snarfed in Z format first to check the color
     * map allocations before resnarfing if XY format selected.
     */
    plane_mask = 1;

/*    XGetImage() calls XCreateImage() internally, and this, in turn
 *  does the memory allocation so we don't have to.
 */

    image = XGetImage ( dpy, target_win, 0, 0, virt_width,
		       virt_height, plane_mask, format);

    if (debug) fprintf(stderr,"xwd: Getting pixmap.\n");

    /*
     * Find the number of colors used, then write them out to the file.
     */

/*%    ncolors = 0;
  if(DisplayPlanes(dpy, screen) > 1) {
	if(DisplayPlanes(dpy, screen) < 9) {
	    histbuffer = (int *)calloc(256, sizeof(int));
	    bzero(histbuffer, 256*sizeof(int));
	    pixcolors = (XColor *)calloc(1, sizeof(XColor));
	    for(i=0; i<buffer_size; i++) {
%*/
		/* if previously found, skip color query */
/*%
		if(histbuffer[(int)buffer[i]] == 0) {
		    pixcolors = 
		      (XColor *)realloc(pixcolors, sizeof(XColor)*(++ncolors));
		    if(debug)
		      fprintf(stderr,"Color %3d at pixel val %5d, i= %5d =",
			      ncolors, buffer[i], i);
		    histbuffer[(int)buffer[i]]++;
		    pixcolors[ncolors-1].pixel = (int)buffer[i];
		    if(XQueryColor(&pixcolors[ncolors-1]) == 0) 
		      Error("Unable to query color table?");
		    if(debug) fprintf(stderr,"%5d %5d %5d\n",
				      pixcolors[ncolors-1].red,
				      pixcolors[ncolors-1].green,
				      pixcolors[ncolors-1].blue);
		}
	    }
	}
	else if(DisplayPlanes(dpy, screen) < 17) {
	    wbuffer = (u_short *)buffer;
	    histbuffer = (int *)calloc(65536, sizeof(int));
	    bzero(histbuffer, 65536*sizeof(int));
	    pixcolors = (XColor *)calloc(1, sizeof(XColor));
	    for(i=0; i<(buffer_size/sizeof(u_short)); i++) {
%*/
		/* if previously found, skip color query */
/*%
		if(histbuffer[(int)wbuffer[i]] == 0) {
		    pixcolors = 
		      (XColor *)realloc(pixcolors, sizeof(XColor)*(++ncolors));
		    if(debug)
		      fprintf(stderr,"Color %2d at pixel val %d, i= %d =",
			      ncolors, wbuffer[i], i);
		    histbuffer[(int)wbuffer[i]]++;
		    pixcolors[ncolors-1].pixel = (int)wbuffer[i];
		    if(XQueryColor(&pixcolors[ncolors-1]) == 0) 
		      Error("Unable to query color table?");
		    if(debug) fprintf(stderr,"%d %d %d\n",
				      pixcolors[ncolors-1].red,
				      pixcolors[ncolors-1].green,
				      pixcolors[ncolors-1].blue);
		}
	    }
	} 

	else
if(DisplayPlanes(dpy, screen) > 16)
	  Error("Unable to handle more than 16 planes at this time");
%*/
	/* reread in XY format if necessary */
/*%
	if(format == XYPixmap) {
	    image = XGetImage(dpy, target_win, 0, 0, virt_width,
	                      virt_height, plane_mask, format);
%*/

/*%
 *%    Plane mask must be assigned a value.
%*/

/*%	}
	free(histbuffer);
   }
%*/

    /*
     * Inform the user that the image has been retrieved.
     */
    XBell(dpy, FEEP_VOLUME);
    XBell(dpy, FEEP_VOLUME);
    XFlush(dpy);

    /*
     * Calculate header size.
     */
    if (debug) fprintf(stderr,"xwd: Calculating header size.\n");
    header_size = sizeof(header) + win_name_size;

    /*
     * Write out header information.
     */
    if (debug) fprintf(stderr,"xwd: Constructing and dumping file header.\n");
    header.header_size = header_size;
    header.file_version = XWD_FILE_VERSION;
    header.display_type = 0; /* DisplayType(dpy, screen);  [obsolete] */
    header.display_planes = DisplayPlanes(dpy, screen);
    header.pixmap_format = format;
    header.pixmap_width = virt_width;
    header.pixmap_height = virt_height;
    header.window_width = win_info.width;
    header.window_height = win_info.height;
    header.window_x = win_info.x;
    header.window_y = win_info.y;
    header.window_bdrwidth = win_info.border_width;
    header.window_ncolors = 0;  /*%  = ncolors;  %*/

    (void) fwrite((char *)&header, sizeof(header), 1, out_file);
    (void) fwrite(win_name, win_name_size, 1, out_file);

    /*
     * Write out the color maps, if any
     */
/*%
    if (debug) fprintf(stderr,"xwd: Dumping %d colors.\n",ncolors);
    (void) fwrite(pixcolors, sizeof(XColor), ncolors, out_file);
%*/

    /*
     * Write out the buffer.
     */
    if (debug) fprintf(stderr,"xwd: Dumping pixmap.  bufsize=%d\n",buffer_size);

/*
 *    This copying of the bit stream (data) to a file is to be replaced
 *  by an X server call which hasn't been written yet.  It is not clear
 *  what other functions of xwd will be taken over by this (as yet)
 *  non-existant X function.
 */
    (void) fwrite(image->data, (int) buffer_size, 1, out_file);

    /*
     * Close the output file.
     */
    if (debug) fprintf(stderr,"xwd: Closing output file.\n");
    (void) fclose(out_file);

    /*
     * free the color buffer.
     */
/*%
    if(debug && ncolors > 0) fprintf(stderr,"xwd: Freeing color map.\n");
    if(ncolors > 0) free(pixcolors);
%*/

    /*
     * Free the pixmap buffer.
     */
    if (debug) fprintf(stderr,"xwd: Freeing pixmap buffer.\n");
    free(buffer);

    /*
     * Free window name string.
     */
    if (debug) fprintf(stderr,"xwd: Freeing window name string.\n");
    free(win_name);
}

/*
 * Report the syntax for calling xwd.
 */
usage()
{
    fprintf(
	stderr,
	"%s: %s [-debug] [-help] [-nobdrs] [-out <file>]\n",
	    program_name
    );
    fprintf(stderr, "                [-xy] [[host]:vs]\n");
    exit(1);
}


/*
 * Error - Fatal xwd error.
 */
Error(string)
	char *string;	/* Error description string. */
{
	fprintf(stderr, "\nxwd: Error => %s\n", string);
	if (errno != 0) {
		perror("xwd");
		fprintf(stderr, "\n");
	}

	exit(1);
}

/* End of xwd.c */
