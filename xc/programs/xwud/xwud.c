/* Copyright 1985, 1986, 1988 Massachusetts Institute of Technology */
#include <X11/copyright.h>

/* xwud - marginally useful raster image undumper */

#ifndef lint
static char *rcsid = "$XConsortium: xwud.c,v 1.22 88/10/20 17:39:59 jim Exp $";
#endif

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <X11/XWDFile.h>

extern int errno;
extern char *malloc();
unsigned Image_Size();

char *progname;

usage()
{
    fprintf(stderr, "usage: %s [-in <file>] [-geometry <geom>] [-display <display>] [-new] [-raw]\n", progname);
    fprintf(stderr, "            [-help] [-rv] [-plane <number>] [-fg <color>] [-bg <color>]\n");
    exit(1);
}

main(argc, argv)
    int argc;
    char **argv;
{
    Display *dpy;
    int screen;
    register int i;
    XImage in_image, *out_image;
    XSetWindowAttributes attributes;
    XVisualInfo vinfo, *vinfos;
    register char *buffer;
    unsigned long swaptest = 1;
    int count;
    unsigned buffer_size;
    int win_name_size;
    int ncolors;
    char *file_name = NULL;
    char *win_name;
    Bool inverse = False, defvis = False, newmap = False;
    int plane = -1;
    char *display_name = NULL;
    char *fgname = NULL;
    char *bgname = NULL;
    char *geom = NULL;
    int gbits = 0;
    XSizeHints hints;
    XColor *colors, color;
    Window image_win;
    Colormap colormap;
    XEvent event;
    register XExposeEvent *expose = (XExposeEvent *)&event;
    GC gc;
    XGCValues gc_val;
    XWDFileHeader header;
    FILE *in_file = stdin;

    progname = argv[0];

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-bg") == 0) {
	    if (++i >= argc) usage();
	    bgname = argv[i];
	    continue;
	}
	if (strcmp(argv[i], "-display") == 0) {
	    if (++i >= argc) usage();
	    display_name = argv[i];
	    continue;
	}
	if (strcmp(argv[i], "-fg") == 0) {
	    if (++i >= argc) usage();
	    fgname = argv[i];
	    continue;
	}
	if (strcmp(argv[i], "-geometry") == 0) {
	    if (++i >= argc) usage();
	    geom = argv[i];
	    continue;
	}
	if (strcmp(argv[i], "-help") == 0) {
	    usage();
	}
	if (strcmp(argv[i], "-in") == 0) {
	    if (++i >= argc) usage();
	    file_name = argv[i];
	    continue;
	}
	if (strcmp(argv[i], "-inverse") == 0) { /* for compatibility */
	    inverse = True;
	    continue;
	}
	if (strcmp(argv[i], "-new") == 0) {
	    newmap = True;
	    if (defvis) usage();
	    continue;
	}
	if (strcmp(argv[i], "-plane") == 0) {
	    if (++i >= argc) usage();
	    plane = atoi(argv[i]);
	    continue;
	}
	if (strcmp(argv[i], "-raw") == 0) {
	    defvis = True;
	    if (newmap) usage();
	    continue;
	}
	if (strcmp(argv[i], "-rv") == 0) {
	    inverse = True;
	    continue;
	}
	usage();
    }
    
    if (file_name) {
	in_file = fopen(file_name, "r");
	if (in_file == NULL)
	    Error("Can't open output file as specified.");
    }
    
    dpy = XOpenDisplay(display_name);
    if (dpy == NULL) {
	fprintf(stderr, "%s:  unable to open display \"%s\"\n",
		progname, XDisplayName(display_name));
	exit(1);
    }
    screen = DefaultScreen(dpy);

    /*
     * Read in header information.
     */
    if(fread((char *)&header, sizeof(header), 1, in_file) != 1)
      Error("Unable to read dump file header.");

    if (*(char *) &swaptest)
	_swaplong((char *) &header, sizeof(header));

    /* check to see if the dump file is in the proper format */
    if (header.file_version != XWD_FILE_VERSION) {
	fprintf(stderr,"xwud: XWD file format version mismatch.");
	Error("exiting.");
    }
    if (header.header_size < sizeof(header)) {
	fprintf(stderr,"xwud: XWD header size is too small.");
	Error("exiting.");
    }

    /* alloc window name */
    win_name_size = (header.header_size - sizeof(header));
    if((win_name = malloc((unsigned) win_name_size)) == NULL)
      Error("Can't malloc window name storage.");

     /* read in window name */
    if(fread(win_name, sizeof(char), win_name_size, in_file) != win_name_size)
      Error("Unable to read window name from dump file.");

    /* initialize the input image */
    in_image.width = (int) header.pixmap_width;
    in_image.height = (int) header.pixmap_height;
    in_image.xoffset = (int) header.xoffset;
    in_image.format = (int) header.pixmap_format;
    in_image.byte_order = (int) header.byte_order;
    in_image.bitmap_unit = (int) header.bitmap_unit;
    in_image.bitmap_bit_order = (int) header.bitmap_bit_order;
    in_image.bitmap_pad = (int) header.bitmap_pad;
    in_image.depth = (int) header.pixmap_depth;
    in_image.bits_per_pixel = (int) header.bits_per_pixel;
    in_image.bytes_per_line = (int) header.bytes_per_line;
    in_image.red_mask = header.red_mask;
    in_image.green_mask = header.green_mask;
    in_image.blue_mask = header.blue_mask;
    in_image.obdata = NULL;
    _XInitImageFuncPtrs(&in_image);

    /* read in the color map buffer */
    if(ncolors = header.ncolors) {
	colors = (XColor *)malloc((unsigned) ncolors * sizeof(XColor));
	if (!colors)
	    Error("Can't malloc color table");
	if(fread((char *) colors, sizeof(XColor), ncolors, in_file) != ncolors)
	  Error("Unable to read color map from dump file.");
	if (*(char *) &swaptest) {
	    for (i = 0; i < ncolors; i++) {
		_swaplong((char *) &colors[i].pixel, sizeof(long));
		_swapshort((char *) &colors[i].red, 3 * sizeof(short));
	    }
	}
    }

    /* alloc the pixel buffer */
    buffer_size = Image_Size(&in_image);
    if((buffer = malloc(buffer_size)) == NULL)
      Error("Can't malloc data buffer.");

    /* read in the image data */
    count = fread(buffer, sizeof(char), (int)buffer_size, in_file);
    if (count != buffer_size)
        Error("Unable to read pixmap from dump file.");

     /* close the input file */
    (void) fclose(in_file);

    if ((in_image.format == XYPixmap) && (plane >= 0)) {
	if (plane >= in_image.depth)
	    Error("plane number exceeds image depth");
	buffer += in_image.bytes_per_line * in_image.height *
		  (in_image.depth - (plane + 1));
	in_image.depth = 1;
    }
    if (in_image.depth == 1) {
	in_image.format = XYBitmap;
	newmap = False;
	defvis = True;
    }
    in_image.data = buffer;

    /* find the desired visual */
    vinfo.screen = screen;
    if (in_image.depth == 1) {
	vinfo.visualid = XVisualIDFromVisual(DefaultVisual(dpy, screen));
	vinfos = (XVisualInfo *)
		    XGetVisualInfo(dpy, VisualScreenMask|VisualIDMask,
				   &vinfo, &count);
	vinfo = vinfos[0];
    } else if (defvis) {
	vinfo.depth = in_image.depth;
	vinfos = (XVisualInfo *)
		    XGetVisualInfo(dpy, VisualScreenMask|VisualDepthMask,
				   &vinfo, &count);
	if (!count)
	    Error("No visual matches the image depth");
	vinfo = vinfos[0];
    } else {
	vinfo.colormap_size = 0;
	vinfo.depth = 0;
	vinfos = (XVisualInfo *)
		    XGetVisualInfo(dpy, VisualScreenMask, &vinfo, &count);
	/* get the visual with the most entries, preferring matching depth  */
	for (i = 0; i < count; i++) {
	    if (((vinfos[i].depth == in_image.depth) &&
		 (vinfos[i].colormap_size >= ncolors)) ||
		((vinfos[i].colormap_size > vinfo.colormap_size) &&
		 (vinfo.depth != in_image.depth)))
		vinfo = vinfos[i];
	}
	if ((vinfo.class == DirectColor) &&
	    (vinfo.class == header.visual_class) &&
	    (vinfo.depth == in_image.depth) &&
	    (vinfo.red_mask == header.red_mask) &&
	    (vinfo.green_mask == header.green_mask) &&
	    (vinfo.blue_mask == header.blue_mask)) {
	    defvis = True;
	    newmap = False;
	}
    }

    /* get the appropriate colormap */
    if (newmap && (vinfo.class & 1) &&
	(vinfo.depth == in_image.depth) &&
	(vinfo.class == header.visual_class) &&
	(vinfo.colormap_size >= ncolors) &&
	(vinfo.red_mask == header.red_mask) &&
	(vinfo.green_mask == header.green_mask) &&
	(vinfo.blue_mask == header.blue_mask)) {
	colormap = XCreateColormap(dpy, RootWindow(dpy, screen), vinfo.visual,
				   AllocAll);
	XStoreColors(dpy, colormap, colors, ncolors);
    } else {
	if (!newmap && (vinfo.visual == DefaultVisual(dpy, screen)))
	    colormap = DefaultColormap(dpy, screen);
	else
	    colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
				       vinfo.visual, AllocNone);
	newmap = False;
    }

    /* create the output image */
    if (defvis || newmap) {
	out_image = &in_image;
    } else {
	out_image = XCreateImage(dpy, vinfo.visual, vinfo.depth,
				 (vinfo.depth == 1) ? XYBitmap :
						      in_image.format,
				 in_image.xoffset, NULL,
				 in_image.width, in_image.height,
				 XBitmapPad(dpy), 0);
	out_image->data = malloc(Image_Size(out_image));
	if ((header.visual_class == TrueColor) ||
	    (header.visual_class == DirectColor))
	    Do_Direct(dpy, &header, &colormap, ncolors, colors,
		      &in_image, out_image);
	else
	    Do_Pseudo(dpy, &colormap, ncolors, colors, &in_image, out_image);
    }

    if (out_image->depth == 1) {
	gc_val.foreground = BlackPixel (dpy, screen);
	gc_val.background = WhitePixel (dpy, screen); 
	if (fgname &&
	    XParseColor(dpy, colormap, fgname, &color) &&
	    XAllocColor(dpy, colormap, &color))
	    gc_val.foreground = color.pixel;
	if (bgname &&
	    XParseColor(dpy, colormap, bgname, &color) &&
	    XAllocColor(dpy, colormap, &color))
	    gc_val.background = color.pixel;
	if (inverse) {
	    unsigned long tmp;
	    tmp = gc_val.foreground;
	    gc_val.foreground = gc_val.background;
	    gc_val.background = tmp;
	}
    } else {
	gc_val.background = XGetPixel(out_image, 0, 0);
	gc_val.foreground = 0;
    }

    attributes.background_pixel = gc_val.background;
    attributes.bit_gravity = NorthWestGravity;
    attributes.event_mask = ButtonPressMask|ButtonReleaseMask|ExposureMask;
    attributes.colormap = colormap;

    hints.x = header.window_x;
    hints.y = header.window_y;
    hints.width = out_image->width;
    hints.height = out_image->height;
    if (geom)
	gbits = XParseGeometry(geom, &hints.x, &hints.y,
			       &hints.width, &hints.height);
    hints.max_width = (hints.width > out_image->width) ? hints.width :
							 out_image->width;
    hints.max_height = (hints.height > out_image->height) ? hints.height :
							    out_image->height;
    hints.flags = PMaxSize |
		  ((gbits & (XValue|YValue)) ? USPosition : 0) |
		  ((gbits & (HeightValue|WidthValue)) ? USSize : PSize);

    /* create the image window */
    image_win = XCreateWindow(dpy, RootWindow(dpy, screen),
			      hints.x, hints.y, hints.width, hints.height,
			      0, vinfo.depth, InputOutput, vinfo.visual,
			      CWBackPixel|CWColormap|CWEventMask|CWBitGravity,
			      &attributes);
     
     /* store the window name string */
    XStoreName(dpy, image_win, win_name);
    
    /* store size hints */
    XSetNormalHints(dpy, image_win, &hints);

    /* map the image window */
    XMapWindow(dpy, image_win);

    gc = XCreateGC (dpy, image_win, GCForeground|GCBackground, &gc_val);

    while (1) {
	/* wait on mouse input event to terminate */
	XNextEvent(dpy, &event);
	switch(event.type) {
	  case ButtonPress:
	    break;
	  case ButtonRelease:
	    XCloseDisplay(dpy);
	    exit(0);
	  case Expose:
	    if ((expose->x < out_image->width) &&
		(expose->y < out_image->height)) {
		if ((out_image->width - expose->x) < expose->width)
		    expose->width = out_image->width - expose->x;
		if ((out_image->height - expose->y) < expose->height)
		    expose->height = out_image->height - expose->y;
		XPutImage(dpy, image_win, gc, out_image,
			  expose->x, expose->y, expose->x, expose->y,
			  expose->width, expose->height);
	    }
	}
    }
}

Do_Pseudo(dpy, colormap, ncolors, colors, in_image, out_image)
    Display *dpy;
    Colormap *colormap;
    int ncolors;
    XColor *colors;
    XImage *in_image, *out_image;
{
    register int i, x, y;
    register XColor *color;

    for (i = 0; i < ncolors; i++)
	colors[i].flags = 0;
    for (y = 0; y < in_image->height; y++) {
	for (x = 0; x < in_image->width; x++) {
	    color = &colors[XGetPixel(in_image, x, y)];
	    if (!color->flags) {
		color->flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(dpy, *colormap, color)) {
		    *colormap = XCopyColormapAndFree(dpy, *colormap);
		    XAllocColor(dpy, *colormap, color);
		}
	    }
	    XPutPixel(out_image, x, y, color->pixel);
	}
    }
}

Do_Direct(dpy, header, colormap, ncolors, colors, in_image, out_image)
    Display *dpy;
    XWDFileHeader *header;
    Colormap *colormap;
    int ncolors;
    XColor *colors;
    XImage *in_image, *out_image;
{
    register int x, y;
    XColor color;
    int direct = 0;
    unsigned long rmask, gmask, bmask;
    int rshift = 0, gshift = 0, bshift = 0;

    rmask = header->red_mask;
    while (!(rmask & 1)) {
	rmask >>= 1;
	rshift++;
    }
    gmask = header->green_mask;
    while (!(gmask & 1)) {
	gmask >>= 1;
	gshift++;
    }
    bmask = header->blue_mask;
    while (!(bmask & 1)) {
	bmask >>= 1;
	bshift++;
    }
    if ((ncolors == 0) || (header->visual_class = DirectColor))
	direct = 1;
    color.flags = DoRed | DoGreen | DoBlue;
    for (y = 0; y < in_image->height; y++) {
	for (x = 0; x < in_image->width; x++) {
	    color.pixel = XGetPixel(in_image, x, y);
	    color.red = (color.pixel >> rshift) & rmask;
	    color.green = (color.pixel >> gshift) & gmask;
	    color.blue = (color.pixel >> bshift) & bmask;
	    if (!direct) {
		color.red = colors[color.red].red;
		color.green = colors[color.green].green;
		color.blue = colors[color.blue].blue;
	    }
	    /* XXX - very slow - needs work */
	    if (!XAllocColor(dpy, *colormap, &color)) {
		*colormap = XCopyColormapAndFree(dpy, *colormap);
		XAllocColor(dpy, *colormap, &color);
	    }
	    XPutPixel(out_image, x, y, color.pixel);
	}
    }
}

unsigned Image_Size(image)
     XImage *image;
{
    if (image->format != ZPixmap)
      return(image->bytes_per_line * image->height * image->depth);

    return((unsigned)image->bytes_per_line * image->height);
}

Error(string)
	char *string;
{
	fprintf(stderr, "xwud: Error => %s\n", string);
	if (errno != 0) {
		perror("xwud");
		fprintf(stderr, "\n");
	}
	exit(1);
}

_swapshort (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
	c = *bp;
	*bp = *(bp + 1);
	bp++;
	*bp++ = c;
    }
}

_swaplong (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while (bp < ep) {
	sp = bp + 3;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	sp = bp + 1;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	bp += 2;
    }
}
