#include <X/mit-copyright.h>

/* Copyright 1985, 1986, Massachusetts Institute of Technology */

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
 */

#ifndef lint
static char *rcsid_xwd_c = "$Header: xwd.c,v 1.3 87/05/18 18:20:30 dkk Locked $";
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <strings.h>

char *calloc();

/*  typedef enum _bool {FALSE, TRUE} Bool;   %%*/

#include "target.cursor"
#include "target_mask.cursor"

#include "XWDFile.h"

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define ABS(a) (a) < 0 ? -(a) : (a)

#define UBPS (sizeof(short)/2) /* useful bytes per short */
#define BitmapSize(width, height) (((((width) + 15) >> 3) &~ 1) * (height) * UBPS)
#define XYPixmapSize(width, height, planes) (BitmapSize(width, height) * (planes))
#define BZPixmapSize(width, height) ((width) * (height))
#define WZPixmapSize(width, height) (((width) * (height)) << 1)

#define FAILURE 0

#define FEEP_VOLUME 0

#define DONT_KNOW_YET 17


extern int errno;

main(argc, argv)
    int argc;
    char **argv;
{
    register int i, *histbuffer;
    register u_short *wbuffer;
    register char *buffer, *cbuffer;

    unsigned buffer_size;
    unsigned int x, y;
    int virt_x, virt_y;
    int virt_width, virt_height;
    int pixmap_format = -1;
    int win_name_size;
    int header_size;
    int ncolors = 0;
    int pointer_mode, keyboard_mode;
    int screen;
    int depth;
    int format;
    int offset;
    int width, height;
    long plane_mask;
    char *str_index;
    char *file_name;
    char display[256];
    char *win_name;
    char *data;
    Bool nobdrs = False;
    Bool debug = False;
    Bool standard_out = True;
    Bool owner_events = True;

    Pixmap source;
    Pixmap mask;
    XColor *scolor;
    XColor *bcolor;
    XColor *pixcolors;
    Display *dpy;
    Window target_win;
    Window confine_to;
    Window rootwin;
    XWindowAttributes win_info;
    Visual *visual;
    Drawable image_win;
    XImage *image;
    Cursor cursor;
    XButtonEvent rep;

    XWDFileHeader header;

    FILE *out_file = stdout;

    pointer_mode = keyboard_mode = GrabModeSync;

/*    *bcolor = *scolor = DONT_KNOW_YET    %%*/

    for (i = 1; i < argc; i++) {
	str_index = (char *)index (argv[i], ':');
	if(str_index != (char *)NULL) {
	    (void) strncpy(display,argv[i],sizeof(display));
	    continue;
        }
	str_index = (char *) index (argv [i], '-');
	if (str_index == (char *)NULL) Syntax(argv[0]);
	if (strncmp(argv[i], "-nobdrs", 6) == 0) {
	    nobdrs = True;
	    continue;
	}
	if (strncmp(argv[i], "-debug", 6) == 0) {
	    debug = True;
	    continue;
	}
	if (strncmp(argv[i], "-help", 5) == 0) {
	    Syntax(argv[0]);
	}
	if (strncmp(argv[i], "-out", 4) == 0) {
	    if (++i >= argc) Syntax(argv[0]);
	    file_name = argv[i];
	    standard_out = False;
	    continue;
	}
	if(strncmp(argv[i], "-xy") == 0) {
	    pixmap_format = XYPixmap;
	    continue;
	}
	Syntax(argv[0]);
    }
    
    if (!standard_out) {
	/*
	 * Open the output file.
	 */
	if((out_file = fopen(file_name, "w")) == NULL)
	  Error("Can't open output file as specified.");
    }

    /*
     * Open the display.
     */
    if ((dpy = XOpenDisplay(display)) == NULL) {
        fprintf(stderr, "%s: Can't open display '%s'\n",
		argv[0], XDisplayName(display));
	exit(1);
      }

    screen = DefaultScreen(dpy);
    rootwin = RootWindow(dpy, screen);

    /*
     * Store the cursor incase we need it.
     */
    if (debug) fprintf(stderr,"xwd: Storing target cursor.\n");
    if((cursor = XCreateCursor(dpy, source, mask, scolor, bcolor,
	8, 8)) == FAILURE)
/*    	target_width, target_height, 
 *   	target_bits, target_mask_bits, %%*/

/*	BlackPixel, WhitePixel,
 *	GXcopy  %%*/

	Error("Error occured while trying to store target cursor.");

    /*
     * Set the right pixmap format for the display type.
     */
    if(DisplayPlanes(dpy, screen) == 1) {
        pixmap_format = XYPixmap;
	format = XYBitmap;
    }
    else {
            format = XYPixmap;
	if(pixmap_format != XYPixmap) {
	    pixmap_format = ZPixmap;
	    format = ZPixmap;
	  }
    }

    /*
     * Let the user select the target window.
     */
    if(XGrabPointer(dpy, rootwin, owner_events, ButtonPress,
		    pointer_mode, keyboard_mode, confine_to, cursor,
		    CurrentTime) == FAILURE)
      Error("Can't grab the mouse.");
    XNextEvent(&rep);
    XUngrabPointer(dpy, CurrentTime);
    target_win = rep.subwindow;
    if (target_win == 0) {
	/*
	 * The user must have indicated the root window.
	 */
	if (debug) fprintf(stderr,"xwd: Root window selected as target.\n");
	target_win = rootwin;
    }
    else if (debug) 
     fprintf(stderr,
	     "xwd: Window 0x%x slected as target.\n", target_win);

    /*
     * Inform the user not to alter the screen.
     */
    XBell(dpy, 50);

    /*
     * Get the parameters of the window being dumped.
     */
    if (debug) fprintf(stderr,"xwd: Getting target window information.\n");

    if(XGetWindowAttributes(dpy, target_win, &win_info) == FAILURE) 
     Error("Can't query target window.");
    if(XFetchName(target_win, &win_name) == FAILURE)
     Error("Can't fetch target window name.");

    /* sizeof(char) is included for the null string terminator. */
    win_name_size = strlen(win_name) + sizeof(char);

    /*
     * Calculate the virtual x, y, width and height of the window pane image
     * (this depends on whether or not the borders are included.
     */
    if (nobdrs) {
	if (debug) fprintf(stderr,"xwd: Image without borders selected.\n");
	image_win = target_win;
	virt_x = 0;
	virt_y = 0;
	virt_width = win_info.width;
	virt_height = win_info.height;
    }
    else {
	if (debug) fprintf(stderr,"xwd: Image with borders selected.\n");
	image_win = rootwin;
	virt_x = win_info.x;
	virt_y = win_info.y;
	virt_width = win_info.width + (win_info.border_width << 1);
    	virt_height = win_info.height + (win_info.border_width << 1);
    }

    /*
     * Determine the pixmap size.
     */
    if (pixmap_format == XYPixmap) {
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


    /*
     * Calloc the buffer.
     */
/*  if (debug) fprintf(stderr,"xwd: Calloc'ing data buffer.\n");
    if((buffer = calloc(buffer_size , 1)) == NULL)
       Error("Can't calloc data buffer.");
 %%*/
    /*
     * Snarf the pixmap out of the frame buffer.
     * Color windows get snarfed in Z format first to check the color
     * map allocations before resnarfing if XY format selected.
     */

    visual = DefaultVisual(dpy, screen);
    depth = DefaultDepth(dpy, screen);
    offset = virt_x;
    width = virt_width;
    height = virt_height;

    image = XCreateImage ( dpy, visual, depth, format, offset,
			   data, width, height);

/*    XGetImage ( dpy, image_win, virt_x, virt_y, width, height, 
	       plane_mask, format);
 %%*/

    if (debug) fprintf(stderr,"xwd: Getting pixmap.\n");

/*    if (DisplayPlanes(dpy, screen) == 1) {
	(void) XGetImage(
	    dpy, image_win,
	    virt_x, virt_y,
	    virt_width, virt_height,
	    plane_mask,
	    format
 %%*/
/*	    (short *)buffer  %%*/
/*	);
    }
    else {
	(void) XPixmapGetZ(
	    image_win,
	    virt_x, virt_y,
	    virt_width, virt_height,
	    (caddr_t)buffer
	);
    }
 %%*/
    /*
     * Find the number of colors used, then write them out to the file.
     */
    ncolors = 0;
/*  if(DisplayPlanes(dpy, screen) > 1) {
	if(DisplayPlanes(dpy, screen) < 9) {
	    histbuffer = (int *)calloc(256, sizeof(int));
	    bzero(histbuffer, 256*sizeof(int));
	    pixcolors = (XColor *)calloc(1, sizeof(XColor));
	    for(i=0; i<buffer_size; i++) {
 %%*/
		/* if previously found, skip color query */
/*		if(histbuffer[(int)buffer[i]] == 0) {
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
 %%*/
		/* if previously found, skip color query */
/*
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
 %%*/
/*	else %%*/  
if(DisplayPlanes(dpy, screen) > 16)
	  Error("Unable to handle more than 16 planes at this time");

	/* reread in XY format if necessary */
/*	if(pixmap_format == XYPixmap) {
	    (void) XPixmapGetXY(image_win,
				virt_x, virt_y,
				virt_width, virt_height,
				(short *)buffer);
	}
 %%*/

	free(histbuffer);
/*
 * }
 %%*/    

    /*
     * Inform the user that the image has been retrieved.
     */
    XBell(dpy, FEEP_VOLUME);
    XBell(dpy, FEEP_VOLUME);
    XFlush();

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
    header.display_type = DisplayType(dpy, screen);
    header.display_planes = DisplayPlanes(dpy, screen);
    header.pixmap_format = pixmap_format;
    header.pixmap_width = virt_width;
    header.pixmap_height = virt_height;
    header.window_width = win_info.width;
    header.window_height = win_info.height;
    header.window_x = win_info.x;
    header.window_y = win_info.y;
    header.window_bdrwidth = win_info.border_width;
    header.window_ncolors = ncolors;

    (void) fwrite((char *)&header, sizeof(header), 1, out_file);
    (void) fwrite(win_name, win_name_size, 1, out_file);

    /*
     * Write out the color maps, if any
     */
    if (debug) fprintf(stderr,"xwd: Dumping %d colors.\n",ncolors);
    (void) fwrite(pixcolors, sizeof(XColor), ncolors, out_file);

    /*
     * Write out the buffer.
     */
    if (debug) fprintf(stderr,"xwd: Dumping pixmap.\n");
    (void) fwrite(buffer, (int) buffer_size, 1, out_file);

    /*
     * Close the output file.
     */
    if (debug) fprintf(stderr,"xwd: Closing output file.\n");
    (void) fclose(out_file);

    /*
     * free the color buffer.
     */
    if(debug && ncolors > 0) fprintf(stderr,"xwd: Freeing color map.\n");
    if(ncolors > 0) free(pixcolors);

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
    exit(0);
}

/*
 * Report the syntax for calling xwd.
 */
Syntax(call)
    char *call;
{
    fprintf(
	stderr,
	"xwd: %s [-debug] [-help] [-nobdrs] [-out <file>]\n",
    	call
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
	fprintf(stderr, "\nxwd: Error => %s", string);
	if (errno != 0) {
		perror("xwd");
		fprintf(stderr, "\n");
	}

	exit(1);
}

/* End of xwd.c */
