/*
 * $XConsortium: Bitmap.c,v 1.15 90/10/31 18:26:21 dave Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Davor Matic, MIT X Consortium
 */



#include <X11/copyright.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xmu/Converters.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include "BitmapP.h"
    
#include <stdio.h>
#include <string.h>
#include <math.h>

#define XtStrlen(s)                   ((s) ? strlen(s) : 0)
#define abs(x)                        (((x) > 0) ? (x) : -(x))
#define min(x, y)                     (((x) < (y)) ? (x) : (y))
#define max(x, y)                     (((x) > (y)) ? (x) : (y))


static Boolean DEBUG = False;

#define DefaultGridTolerance 5
#define DefaultBitmapWidth   16
#define DefaultBitmapHeight  16
#define DefaultGrid          TRUE
#define DefaultDashed        TRUE
#define DefaultStippled      TRUE
#define DefaultProportional  TRUE
#define DefaultAxes          FALSE
#define DefaultDistance      10
#define DefaultSquareSize    20

static XtResource resources[] = {
#define offset(field) XtOffset(BitmapWidget, bitmap.field)
{XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
     offset(foreground_pixel), XtRString, XtDefaultForeground},
{XtNhighlight, XtCHighlight, XtRPixel, sizeof(Pixel),
     offset(highlight_pixel), XtRString, XtDefaultForeground},
{XtNframing, XtCFraming, XtRPixel, sizeof(Pixel),
     offset(framing_pixel), XtRString, XtDefaultForeground},
{XtNgridTolerance, XtCGridTolerance, XtRDimension, sizeof(Dimension),
     offset(grid_tolerance), XtRImmediate, (XtPointer) DefaultGridTolerance},
{XtNbitmapWidth, XtCBitmapWidth, XtRDimension, sizeof(Dimension),
     offset(width), XtRImmediate, (XtPointer) DefaultBitmapWidth},
{XtNbitmapHeight, XtCBitmapHeight, XtRDimension, sizeof(Dimension),
     offset(height), XtRImmediate, (XtPointer) DefaultBitmapHeight},
{XtNdashed, XtCDashed, XtRBoolean, sizeof(Boolean),
     offset(dashed), XtRImmediate, (XtPointer) DefaultDashed},
{XtNgrid, XtCGrid, XtRBoolean, sizeof(Boolean),
     offset(grid), XtRImmediate, (XtPointer) DefaultGrid},
{XtNstippled, XtCStippled, XtRBoolean, sizeof(Boolean),
     offset(stippled), XtRImmediate, (XtPointer) DefaultStippled},
{XtNproportional, XtCProportional, XtRBoolean, sizeof(Boolean),
     offset(proportional), XtRImmediate, (XtPointer) DefaultProportional},
{XtNaxes, XtCAxes, XtRBoolean, sizeof(Boolean),
     offset(axes), XtRImmediate, (XtPointer) DefaultAxes},
{XtNsquareSize, XtCSquareSize, XtRDimension, sizeof(Dimension),
     offset(squareW), XtRImmediate, (XtPointer) DefaultSquareSize},
{XtNdistance, XtCDistance, XtRDimension, sizeof(Dimension),
     offset(distance), XtRImmediate, (XtPointer) DefaultDistance},
{XtNsquareSize, XtCSquareSize, XtRDimension, sizeof(Dimension),
     offset(squareH), XtRImmediate, (XtPointer) DefaultSquareSize},
{XtNxHot, XtCXHot, XtRPosition, sizeof(Position),
     offset(hot.x), XtRImmediate, (XtPointer) NotSet},
{XtNyHot, XtCYHot, XtRPosition, sizeof(Position),
     offset(hot.y), XtRImmediate, (XtPointer) NotSet},
{XtNbutton1Action, XtCButton1Action, XtRInt, sizeof(int),
     offset(button_action[0]), XtRImmediate, (XtPointer) Set},
{XtNbutton2Action, XtCButton2Action, XtRInt, sizeof(int),
     offset(button_action[1]), XtRImmediate, (XtPointer) Invert},
{XtNbutton3Action, XtCButton3Action, XtRInt, sizeof(int),
     offset(button_action[2]), XtRImmediate, (XtPointer) Clear},
{XtNbutton4Action, XtCButton4Action, XtRInt, sizeof(int),
     offset(button_action[3]), XtRImmediate, (XtPointer) Invert},
{XtNbutton5Action, XtCButton5Action, XtRInt, sizeof(int),
     offset(button_action[4]), XtRImmediate, (XtPointer) Invert},
{XtNfilename, XtCFilename, XtRString, sizeof(String),
     offset(filename), XtRImmediate, (XtPointer) XtNscratch},
{XtNbasename, XtCBasename, XtRString, sizeof(String),
     offset(basename), XtRImmediate, (XtPointer) ""},
{XtNdashes, XtCDashes, XtRBitmap, sizeof(Pixmap),
     offset(dashes), XtRImmediate, (XtPointer) XtUnspecifiedPixmap},
{XtNstipple, XtCStipple, XtRBitmap, sizeof(Pixmap),
     offset(stipple), XtRImmediate, (XtPointer) XtUnspecifiedPixmap},
#undef offset
};

void BWDebug();
void BWChangeNotify();
void BWSetChanged();
void BWTerminate();
void BWUp();
void BWDown();
void BWLeft();
void BWRight();
void BWFold();
void BWFlipHoriz();
void BWFlipVert();
void BWRotateRight();
void BWRotateLeft();
void BWSet();
void BWClear();
void BWInvert();
void BWUndo();
void BWRedraw();
void BWTMark();
void BWTUnmark();
void BWTPaste();

static XtActionsRec actions[] =
{
{"mark",               BWTMark},
{"unmark",             BWTUnmark},
{"paste",              BWTPaste},
{"bw-debug",           BWDebug},
{"terminate",          BWTerminate},
{"store-to-buffer",    BWStoreToBuffer},
{"change-notify",      BWChangeNotify},
{"set-changed",        BWSetChanged},
{"up",                 BWUp},
{"down",               BWDown},
{"left",               BWLeft},
{"right",              BWRight},
{"fold",               BWFold},
{"flip-horiz",         BWFlipHoriz},
{"flip-vert",          BWFlipVert},
{"rotate-right",       BWRotateRight},
{"rotate-left",        BWRotateLeft},
{"set",                BWSet},
{"clear",              BWClear},
{"invert",             BWInvert},
{"undo",               BWUndo},
{"redraw",             BWRedraw},
};

static char translations[] =
"\
 Ctrl<Btn1Down>:   mark()\n\
 Ctrl<Btn2Down>:   paste()\n\
 Ctrl<Btn3Down>:   unmark()\n\
 Ctrl<Key>l:       redraw()\n\
 <Key>d:           bw-debug()\n\
 <Key>t:           terminate()\n\
 <Key>Up:          store-to-buffer()\
                   up()\
                   change-notify()\
                   set-changed()\n\
 <Key>Down:        store-to-buffer()\
                   down()\
                   change-notify()\
                   set-changed()\n\
 <Key>Left:        store-to-buffer()\
                   left()\
                   change-notify()\
                   set-changed()\n\
 <Key>Right:       store-to-buffer()\
                   right()\
                   change-notify()\
                   set-changed()\n\
 <Key>f:           store-to-buffer()\
                   fold()\
                   change-notify()\
                   set-changed()\n\
 <Key>h:           store-to-buffer()\
                   flip-horiz()\
                   change-notify()\
                   set-changed()\n\
 <Key>v:           store-to-buffer()\
                   flip-vert()\
                   change-notify()\
                   set-changed()\n\
 <Key>r:           store-to-buffer()\
                   rotate-right()\
                   change-notify()\
                   set-changed()\n\
 <Key>l:           store-to-buffer()\
                   rotate-left()\
                   change-notify()\
                   set-changed()\n\
 <Key>s:           store-to-buffer()\
                   set()\
                   change-notify()\
                   set-changed()\n\
 <Key>c:           store-to-buffer()\
                   clear()\
                   change-notify()\
                   set-changed()\n\
 <Key>i:           store-to-buffer()\
                   invert()\
                   change-notify()\
                   set-changed()\n\
 <Key>u:           undo()\
                   change-notify()\
                   set-changed()\n\
";

Atom targets[] = {
    XA_BITMAP,
    XA_PIXMAP,
    XA_STRING,
};

#include "Requests.h"

static void ClassInitialize();
static void Initialize();
static void Redisplay();
static void Resize();
static void Destroy();
static Boolean SetValues();
 
BitmapClassRec bitmapClassRec = {
{   /* core fields */
    /* superclass		*/	(WidgetClass) &simpleClassRec,
    /* class_name		*/	"Bitmap",
    /* widget_size		*/	sizeof(BitmapRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	FALSE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	TRUE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	translations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL,
  },
  { 
    /* empty			*/	XtInheritChangeSensitive,
  },
  {
    /* targets                  */      targets,
    /* num_trets                */      XtNumber(targets),
    /* requests                 */      requests,
    /* num_requests             */      XtNumber(requests),
  }
};
 
WidgetClass bitmapWidgetClass = (WidgetClass) &bitmapClassRec;
    
/* ARGSUSED */
void BWDebug(w)
    Widget w;
{
    DEBUG ^= True;
}

Pixmap BWGetPixmap(w) 
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
 
    return GetPixmap(BW, BW->bitmap.zoom.image);
}

Pixmap BWGetUnzoomedPixmap(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    GC gc;
    Pixmap pix;
    
    if (BW->bitmap.zooming) {    
	pix = XCreatePixmap(XtDisplay(w), XtWindow(w), 
			    BW->bitmap.zoom.image->width, 
			    BW->bitmap.zoom.image->height, 1);
	if (!(gc = XCreateGC(XtDisplay(w), pix, 
			     (unsigned long) 0, (XGCValues *) 0)))
	    return (Pixmap) None;
	
	XPutImage(XtDisplay(w), pix, gc, 
		  BW->bitmap.zoom.image, 
		  0, 0, 0, 0, 
		  BW->bitmap.zoom.image->width, 
		  BW->bitmap.zoom.image->height);
	XPutImage(XtDisplay(w), pix, gc, 
		  BW->bitmap.image, 
		  0, 0, 
		  BW->bitmap.zoom.at_x,
		  BW->bitmap.zoom.at_y,
		  BW->bitmap.image->width, 
		  BW->bitmap.image->height);
    }
    else {
	pix = XCreatePixmap(XtDisplay(w), XtWindow(w), 
			    BW->bitmap.image->width, 
			    BW->bitmap.image->height, 1);
	if (! (gc = XCreateGC(XtDisplay(w), pix, 
			      (unsigned long) 0, (XGCValues *) 0)))
	    return (Pixmap) None;
	
	XPutImage(XtDisplay(w), pix, gc, 
		  BW->bitmap.image, 
		  0, 0, 0, 0,
		  BW->bitmap.image->width, 
		  BW->bitmap.image->height);
    }
    XFreeGC(XtDisplay(w), gc);
    return(pix);
}

XImage *CreateBitmapImage();

XImage *ConvertToBitmapImage();

XImage *GetImage(BW, pixmap)
    BitmapWidget BW;
    Pixmap pixmap;
{
    Window root;
    int x, y;
    unsigned int width, height, border_width, depth;
    XImage *source, *image;

    XGetGeometry(XtDisplay(BW), pixmap, &root, &x, &y,
		 &width, &height, &border_width, &depth);

    source = XGetImage(XtDisplay(BW), pixmap, x, y, width, height,
		     1, XYPixmap);

    image = ConvertToBitmapImage(BW, source);

    return image;
}

XImage *CreateBitmapImage(BW, data, width, height)
    BitmapWidget BW;
    char *data;
    Dimension width, height;
{
    XImage *image = XCreateImage(XtDisplay(BW),
				 DefaultVisual(XtDisplay(BW), 
					       DefaultScreen(XtDisplay(BW))),
				 1, XYBitmap, 0, 
				 data, width, height,
				 8, (width + 7) / 8);

    image->height = height;
    image->width = width;
    image->depth = 1;
    image->xoffset = 0;
    image->format = XYBitmap;
    image->data = (char *)data;
    image->byte_order = LSBFirst;
    image->bitmap_unit = 8;
    image->bitmap_bit_order = LSBFirst;
    image->bitmap_pad = 8;
    image->bytes_per_line = (width + 7) / 8;

    return image;
}

void DestroyBitmapImage(image)
    XImage **image;
{
    /*XDestroyImage(*image);*/
    if (image) {
	if (*image) {
	    if ((*image)->data)
		XtFree((*image)->data);
	    XtFree((char *)*image);
	}
	*image = NULL;
    }
}

XImage *BWGetImage(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return BW->bitmap.image;
}

void BWChangeNotify(w, client_data, call_data)
     Widget       w;
     caddr_t      client_data;
     caddr_t      call_data;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (BW->bitmap.notify)
	(*BW->bitmap.notify)(w, client_data, call_data);
}

void Notify(w, proc)
     Widget   w;
     void   (*proc)();
{
    BitmapWidget BW = (BitmapWidget) w;

    BW->bitmap.notify = proc;
}

void BWSetChanged(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
	
    BW->bitmap.changed = True;
}

Boolean BWQueryChanged(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
	
    return BW->bitmap.changed;
}

void BWClearChanged(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    BW->bitmap.changed = False;
}

Boolean BWQueryStored(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    return (BW->bitmap.storage != NULL);
}

Boolean BWQueryStippled(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return BW->bitmap.stippled;
}

void BWSwitchStippled(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    BWRedrawMark(w);
    
    BW->bitmap.stippled ^= True;
    XSetFillStyle(XtDisplay(BW), BW->bitmap.highlighting_gc,
		  (BW->bitmap.stippled ? FillStippled : FillSolid));

    BWRedrawMark(w);
}

void BWSelect(w, from_x, from_y, to_x, to_y, btime)
    Widget w;
    Position from_x, from_y,
	     to_x, to_y;
    Time btime;
{
    BWMark(w, from_x, from_y, to_x, to_y);

    BWGrabSelection(w, btime);
}

Boolean BWQueryAxes(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return BW->bitmap.axes;
}

void BWSwitchAxes(w)
     Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    BW->bitmap.axes ^= True;
    BWHighlightAxes(w);
}

void BWAxes(w, _switch)
    Widget w;
    Boolean _switch;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.axes != _switch)
	BWSwitchAxes(w);
}

void BWRedrawAxes(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.axes)
	BWHighlightAxes(w);
}

void BWPutImage(w, display, drawable, gc, x, y)
     BitmapWidget w;
     Display     *display;
     Drawable     drawable;
     GC           gc;
     Position     x, y;
{
    BitmapWidget BW = (BitmapWidget) w;

  XPutImage(display, drawable, gc, BW->bitmap.image,
	    0, 0, x, y, BW->bitmap.image->width, BW->bitmap.image->height);
}


String StripFilename(filename)
    String filename;
{
    char *begin = rindex (filename, '/');
    char *end, *result;
    int length;
    
    if (filename) {
	begin = (begin ? begin + 1 : filename);
	end = index (begin, '.');     /* change to rindex to allow longer names */
	length = (end ? (end - begin) : strlen (begin));
	result = (char *) XtMalloc (length + 1);
	strncpy (result, begin, length);
	result [length] = '\0';
	return (result);
    }
    else
	return (XtNdummy);
}

int XmuWriteBitmapDataToFile (filename, basename, 
			      width, height, datap, x_hot, y_hot)
    String filename, basename;
    int width, height;
    char *datap;
    int x_hot, y_hot;
{
    FILE *file;
    int i, data_length;
    
    data_length = Length(width, height);
    
    if(!filename)    
	file = stdout;
    else
    	file = fopen(filename, "w+");
    
    if (!basename)
	basename = StripFilename(filename);
    
    if (file) {
	fprintf(file, "#define %s_width %d\n", basename, width);
	fprintf(file, "#define %s_height %d\n", basename, height);
	if (QuerySet(x_hot, y_hot)) {
	    fprintf(file, "#define %s_x_hot %d\n", basename, x_hot);
	    fprintf(file, "#define %s_y_hot %d\n", basename, y_hot);
	}
	fprintf(file, "static char %s_bits[] = {\n   0x%02x",
		basename, (unsigned char) datap[0]);
	for(i = 1; i < data_length; i++) {
	    fprintf(file, ",");
	    fprintf(file, (i % 12) ? " " : "\n   ");
	    fprintf(file, "0x%02x", (unsigned char) datap[i]);
	}
	fprintf(file, "};\n");
	
	if (file != stdout)
	    fclose(file);

	return BitmapSuccess;
    }
    
    return 1;
}

void Refresh();

static void ClassInitialize()
{
  static XtConvertArgRec screenConvertArg[] = {
  {XtWidgetBaseOffset, (caddr_t) XtOffset(Widget, core.screen),
       sizeof(Screen *)}
  };

  XawInitializeWidgetSet();

  XtAddConverter(XtRString, XtRBitmap, XmuCvtStringToBitmap,
		 screenConvertArg, XtNumber(screenConvertArg));
  
}

/* ARGSUSED */
static void Initialize(request, new, argv, argc)
    BitmapWidget request, new;
    ArgList argv;
    Cardinal argc;
{
    XGCValues  values;
    XtGCMask   mask;
    char *image_data, *buffer_data;

    new->bitmap.notify = NULL;
    new->bitmap.cardinal = 0;
    new->bitmap.current = 0;
    new->bitmap.fold = False;
    new->bitmap.changed = False;
    new->bitmap.zooming = False;
    new->bitmap.selection.own = False;
    new->bitmap.selection.limbo = False;

    new->bitmap.request_stack = (BWRequestStack *)
	XtMalloc(sizeof(BWRequestStack));
    new->bitmap.request_stack[0].request = NULL;
    new->bitmap.request_stack[0].call_data = NULL;
    new->bitmap.request_stack[0].trap = False;

    new->core.width = new->bitmap.width * new->bitmap.squareW + 
	2 * new->bitmap.distance;
    new->core.height = new->bitmap.height * new->bitmap.squareH + 
	2 * new->bitmap.distance;
  
    new->bitmap.hot.x = new->bitmap.hot.y = NotSet;
    new->bitmap.buffer_hot.x = new->bitmap.buffer_hot.y = NotSet;
  
    new->bitmap.mark.from_x = new->bitmap.mark.from_y = NotSet;
    new->bitmap.mark.to_x = new->bitmap.mark.to_y = NotSet;
    new->bitmap.buffer_mark.from_x = new->bitmap.buffer_mark.from_y = NotSet;
    new->bitmap.buffer_mark.to_x = new->bitmap.buffer_mark.to_y = NotSet;

    values.foreground = new->bitmap.foreground_pixel;
    values.background = new->core.background_pixel;
    values.foreground ^= values.background;
    values.function = GXxor;
    mask = GCForeground | GCBackground | GCFunction;
    new->bitmap.drawing_gc = XCreateGC(XtDisplay(new), 
				       RootWindow(XtDisplay(new), 
				       DefaultScreen(XtDisplay(new))),
				       mask, &values);

    values.foreground = new->bitmap.highlight_pixel;
    values.background = new->core.background_pixel;
    values.foreground ^= values.background;
    values.function = GXxor;
    mask = GCForeground | GCBackground | GCFunction;
    if (new->bitmap.stipple != XtUnspecifiedPixmap)
    {
	values.stipple = new->bitmap.stipple;
	mask |= GCStipple | GCFillStyle;
    }
    values.fill_style = (new->bitmap.stippled ? FillStippled : FillSolid);

    new->bitmap.highlighting_gc = XCreateGC(XtDisplay(new), 
					    RootWindow(XtDisplay(new), 
					       DefaultScreen(XtDisplay(new))), 
					    mask, &values);


    values.foreground = new->bitmap.framing_pixel;
    values.background = new->core.background_pixel;
    values.foreground ^= values.background;
    mask = GCForeground | GCBackground | GCFunction;
    if (new->bitmap.dashes != XtUnspecifiedPixmap)
    {
	values.stipple = new->bitmap.dashes;
	mask |= GCStipple | GCFillStyle;
    }
    values.fill_style = (new->bitmap.dashed ? FillStippled : FillSolid);

    new->bitmap.framing_gc = XCreateGC(XtDisplay(new), 
				     RootWindow(XtDisplay(new), 
						DefaultScreen(XtDisplay(new))),
				     mask, &values);

    values.foreground = new->bitmap.highlight_pixel;
    values.background = new->core.background_pixel;
    values.foreground ^= values.background;
    mask = GCForeground | GCBackground | GCFunction;
    new->bitmap.axes_gc = XCreateGC(XtDisplay(new), 
				     RootWindow(XtDisplay(new), 
						DefaultScreen(XtDisplay(new))),
				     mask, &values);

    image_data = CreateCleanData(Length(new->bitmap.width, new->bitmap.height));
    buffer_data = CreateCleanData(Length(new->bitmap.width, new->bitmap.height));

    new->bitmap.storage = NULL;
    
    new->bitmap.image = CreateBitmapImage(new, 
					  image_data,
					  new->bitmap.width,
					  new->bitmap.height);
    
    new->bitmap.buffer = CreateBitmapImage(new, 
					   buffer_data,
					   new->bitmap.width,
					   new->bitmap.height);

    /* Read file */
    {
	int status;
	XImage *image, *buffer;
	char *image_data, *buffer_data;
	unsigned int width, height;
	int x_hot, y_hot;
	
	status = XmuReadBitmapDataFromFile(new->bitmap.filename, 
					   &width, &height, &image_data,
					   &x_hot, &y_hot);
	if (status == BitmapSuccess) {
	    
	    buffer_data = CreateCleanData(Length(width, height));
	    
	    image = CreateBitmapImage(new, image_data, width, height);
	    buffer = CreateBitmapImage(new, buffer_data, width, height);
	
	    TransferImageData(new->bitmap.image, buffer);
	
	    DestroyBitmapImage(&new->bitmap.image);
	    DestroyBitmapImage(&new->bitmap.buffer);
	
	    new->bitmap.image = image;
	    new->bitmap.buffer = buffer;
	    new->bitmap.width = width;
	    new->bitmap.height = height;
	    
	    new->bitmap.hot.x = x_hot;
	    new->bitmap.hot.y = y_hot;
	    
	    new->bitmap.changed = False;
	    new->bitmap.zooming = False;
	}
	if (!strcmp(new->bitmap.basename, "")) {
	    new->bitmap.basename = StripFilename(new->bitmap.filename);
	}
    }
    Resize(new);
}

Boolean BWQueryMarked(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y);
}

void FixMark(BW)
    BitmapWidget BW;
{
    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y)) {
	BW->bitmap.mark.from_x = min(BW->bitmap.mark.from_x, 
				     BW->bitmap.image->width);
	BW->bitmap.mark.from_y = min(BW->bitmap.mark.from_y, 
				     BW->bitmap.image->height);
	BW->bitmap.mark.to_x = min(BW->bitmap.mark.to_x, 
				   BW->bitmap.image->width);
	BW->bitmap.mark.to_y = min(BW->bitmap.mark.to_y, 
				   BW->bitmap.image->height);
	
	if((BW->bitmap.mark.from_x == BW->bitmap.mark.from_y) &&
	   (BW->bitmap.mark.to_x   == BW->bitmap.mark.to_y))
	    BW->bitmap.mark.from_x = 
		BW->bitmap.mark.from_y =
		    BW->bitmap.mark.to_x = 
			BW->bitmap.mark.to_y = NotSet;
    }
}

/* ARGSUSED */
int BWStoreFile(w, filename, basename)
    Widget w;
    String filename, *basename;
{
    BitmapWidget BW = (BitmapWidget) w;
    int status;
    char *storage_data;
    unsigned int width, height;
    int x_hot, y_hot;
    
    status = XmuReadBitmapDataFromFile(filename, &width, &height, &storage_data,
				       &x_hot, &y_hot);
    if (status == BitmapSuccess) {

	DestroyBitmapImage(&BW->bitmap.storage);
	
	BW->bitmap.storage = CreateBitmapImage(BW, storage_data, width, height);

	return BitmapSuccess;
    }
    else
	XtWarning(" read file failed.  BitmapWidget");
    
    return status;
}

String BWUnparseStatus(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    sprintf(BW->bitmap.status, 
	    "Filename: %s Basename:%s Size:%dx%d",
	    BW->bitmap.filename, BW->bitmap.basename, 
	    BW->bitmap.width, BW->bitmap.height);

    return BW->bitmap.status;
}

    
void BWChangeFilename(w, str)
    Widget w;
    String str;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (str)
	if (strcmp(str, "")) {
	    XtFree(BW->bitmap.filename);
	    BW->bitmap.filename = XtNewString( str);
	}
}

void BWChangeBasename(w, str)
    Widget w;
    String str;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (str)
	if (strcmp(str, "")) {
	    XtFree(BW->bitmap.basename);
	    BW->bitmap.basename = XtNewString(str);
	}
}


int BWReadFile(w, filename, basename)
    Widget w;
    String filename, basename;
{
    BitmapWidget BW = (BitmapWidget) w;
    int status;
    XImage *image, *buffer;
    char *image_data, *buffer_data;
    unsigned int width, height;
    int x_hot, y_hot;
    
    if (!filename)
	filename = BW->bitmap.filename;

    status = XmuReadBitmapDataFromFile(filename, &width, &height, &image_data,
				       &x_hot, &y_hot);
    if (status == BitmapSuccess) {
	
	buffer_data = CreateCleanData(Length(width, height));
	
	image = CreateBitmapImage(BW, image_data, width, height);
	buffer = CreateBitmapImage(BW, buffer_data, width, height);
	
	TransferImageData(BW->bitmap.image, buffer);
	
	DestroyBitmapImage(&BW->bitmap.image);
	DestroyBitmapImage(&BW->bitmap.buffer);
	
	BW->bitmap.image = image;
	BW->bitmap.buffer = buffer;
	BW->bitmap.width = width;
	BW->bitmap.height = height;
	
	BW->bitmap.hot.x = x_hot;
	BW->bitmap.hot.y = y_hot;
	
	BW->bitmap.changed = False;
	BW->bitmap.zooming = False;
	
	XtFree(BW->bitmap.filename);
	BW->bitmap.filename = XtNewString(filename);
	XtFree(BW->bitmap.basename);
	BW->bitmap.basename= XtNewString(StripFilename(filename));
	BWChangeBasename(w, basename);

	BWUnmark(w);
	
	Resize(BW);

	if (BW->core.visible) {
	    XClearArea(XtDisplay(BW), XtWindow(BW),
		       0, 0, 
		       BW->core.width, BW->core.height,
		       True);
	}
	
	return BitmapSuccess;
    }
    else
	XtWarning(" read file failed.  BitmapWidget");
    
    return status;
}

int BWSetImage(w, image)
    Widget w;
    XImage *image;
{
    BitmapWidget BW = (BitmapWidget) w;
    XImage *buffer;
    char *buffer_data;
    
    buffer_data = CreateCleanData(Length(image->width, image->height));
    buffer = CreateBitmapImage(BW, buffer_data, 
			       (Dimension) image->width, 
			       (Dimension) image->height);
    
    TransferImageData(BW->bitmap.image, buffer);
    
    DestroyBitmapImage(&BW->bitmap.image);
    DestroyBitmapImage(&BW->bitmap.buffer);
    
    BW->bitmap.image = image;
    BW->bitmap.buffer = buffer;
    BW->bitmap.width = image->width;
    BW->bitmap.height = image->height;
    
    Resize(BW);
    
    if (BW->core.visible) {
	XClearArea(XtDisplay(BW), XtWindow(BW),
		   0, 0, 
		   BW->core.width, BW->core.height,
		   True);    
    }
}

int BWWriteFile(w, filename, basename)
    Widget w;
    String filename, basename;
{
    BitmapWidget BW = (BitmapWidget) w;
    char *data;
    XImage *image;
    XPoint hot;
    int status;
    
    if (BW->bitmap.zooming) {
        data = XtMalloc(Length(BW->bitmap.zoom.image->width, 
			       BW->bitmap.zoom.image->height));
	bcopy(BW->bitmap.zoom.image->data, data,
	      Length(BW->bitmap.zoom.image->width, 
		     BW->bitmap.zoom.image->height));
	image = CreateBitmapImage(BW, data,
				  (Dimension) BW->bitmap.zoom.image->width,
				  (Dimension) BW->bitmap.zoom.image->height);
	CopyImageData(BW->bitmap.image, image, 
		      0, 0, 
		      BW->bitmap.image->width - 1,
		      BW->bitmap.image->height - 1,
		      BW->bitmap.zoom.at_x, BW->bitmap.zoom.at_y);
	
	if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)) {
	    hot.x = BW->bitmap.hot.x + BW->bitmap.zoom.at_x;
	    hot.y = BW->bitmap.hot.y + BW->bitmap.zoom.at_y;
	}
	else
	    hot = BW->bitmap.zoom.hot;
    }
    else {
	image = BW->bitmap.image;
	hot = BW->bitmap.hot;
    }
    
    if (!filename) filename = BW->bitmap.filename;
    else {
	XtFree(BW->bitmap.filename);
	BW->bitmap.filename = XtNewString(filename);
	XtFree(BW->bitmap.basename);
	BW->bitmap.basename= XtNewString(StripFilename(filename));
    }
    if (!basename) basename = BW->bitmap.basename;
    else {
	XtFree(BW->bitmap.basename);
	BW->bitmap.basename = XtNewString(basename);
    }

    if (DEBUG)
	fprintf(stderr, "Saving filename: %s\n", filename);

    status = XmuWriteBitmapDataToFile(filename, basename,
				      image->width, image->height, image->data,
				      hot.x, hot.y);
    if (BW->bitmap.zooming)
	DestroyBitmapImage(&image);
    
    if (status == BitmapSuccess)
	BW->bitmap.changed = False;
    
    return status;
}

String BWGetFilename(w, str)
    Widget w;
    String *str;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    *str = XtNewString(BW->bitmap.filename);

    return *str;
}

String BWGetFilepath(w, str)
    Widget w;
    String *str;
{
    BitmapWidget BW = (BitmapWidget) w;
    String end;

    *str = XtNewString(BW->bitmap.filename);
    end = rindex(*str, '/');

    if (end)
	*(end + 1) = '\0';
    else 
	**str = '\0';

    return *str;
}


String BWGetBasename(w, str)
    Widget w;
    String *str;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    *str = XtNewString(BW->bitmap.basename);

    return *str;
}

void FixHotSpot(BW)
    BitmapWidget BW;
{
    if (!QueryInBitmap(BW, BW->bitmap.hot.x, BW->bitmap.hot.y))
	BW->bitmap.hot.x = BW->bitmap.hot.y = NotSet;
}

void ZoomOut(BW)
    BitmapWidget BW;
{
    CopyImageData(BW->bitmap.image, BW->bitmap.zoom.image, 
		  0, 0, 
		  BW->bitmap.image->width - 1,
		  BW->bitmap.image->height - 1,
		  BW->bitmap.zoom.at_x, BW->bitmap.zoom.at_y);
    
    DestroyBitmapImage(&BW->bitmap.image);
    DestroyBitmapImage(&BW->bitmap.buffer);
    
    BW->bitmap.image = BW->bitmap.zoom.image;
    BW->bitmap.buffer = BW->bitmap.zoom.buffer;
    BW->bitmap.width = BW->bitmap.image->width;
    BW->bitmap.height = BW->bitmap.image->height;
    BW->bitmap.fold = BW->bitmap.zoom.fold;
    BW->bitmap.changed |= BW->bitmap.zoom.changed;
    BW->bitmap.grid = BW->bitmap.zoom.grid;

    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)) {
	BW->bitmap.hot.x += BW->bitmap.zoom.at_x;
	BW->bitmap.hot.y += BW->bitmap.zoom.at_y;
    }
    else
	BW->bitmap.hot = BW->bitmap.zoom.hot;
    
    BW->bitmap.mark.from_x = NotSet;
    BW->bitmap.mark.from_y = NotSet;
    BW->bitmap.mark.to_x = NotSet;
    BW->bitmap.mark.to_y = NotSet;
    BW->bitmap.zooming = False;
}    

void BWZoomOut(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.zooming) {
	ZoomOut(BW);
	
	Resize(BW);
	if (BW->core.visible)
	    XClearArea(XtDisplay(BW), XtWindow(BW),
		       0, 0, 
		       BW->core.width, BW->core.height,
		       True);
    }
}

void BWZoomIn();

void BWZoomMarked(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    BWZoomIn(w, 
	     BW->bitmap.mark.from_x, BW->bitmap.mark.from_y,
	     BW->bitmap.mark.to_x,   BW->bitmap.mark.to_y);
}

void BWZoomIn(w, from_x, from_y, to_x, to_y)
    Widget w;
    Position from_x, from_y,
	     to_x, to_y;
{
    BitmapWidget BW = (BitmapWidget) w;
    XImage *image, *buffer;    
    Dimension width, height;
    char *image_data, *buffer_data;
  
    if (BW->bitmap.zooming)
	ZoomOut(BW);
    
    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
    from_x = max(0, from_x);
    from_y = max(0, from_y);
    to_x = min(BW->bitmap.width - 1, to_x);
    to_y = min(BW->bitmap.height - 1, to_y);
    
    width = to_x - from_x + 1;
    height = to_y - from_y + 1;

    image_data = CreateCleanData(Length(width, height));
    buffer_data = CreateCleanData(Length(width, height));

    image = CreateBitmapImage(BW, image_data, width, height);
    buffer = CreateBitmapImage(BW, buffer_data, width, height);

    CopyImageData(BW->bitmap.image, image, from_x, from_y, to_x, to_y, 0, 0);
    CopyImageData(BW->bitmap.buffer, buffer, from_x, from_y, to_x, to_y, 0, 0);
    
    BW->bitmap.zoom.image = BW->bitmap.image;
    BW->bitmap.zoom.buffer = BW->bitmap.buffer;
    BW->bitmap.zoom.at_x = from_x;
    BW->bitmap.zoom.at_y = from_y;
    BW->bitmap.zoom.fold = BW->bitmap.fold;
    BW->bitmap.zoom.changed = BW->bitmap.changed;
    BW->bitmap.zoom.hot = BW->bitmap.hot;
    BW->bitmap.zoom.grid = BW->bitmap.grid;

    BW->bitmap.image = image;
    BW->bitmap.buffer = buffer;
    BW->bitmap.width = width;
    BW->bitmap.height = height;
    BW->bitmap.changed = False;
    BW->bitmap.hot.x -= from_x;
    BW->bitmap.hot.y -= from_y;
    BW->bitmap.mark.from_x = NotSet;
    BW->bitmap.mark.from_y = NotSet;
    BW->bitmap.mark.to_x = NotSet;
    BW->bitmap.mark.to_y = NotSet;
    BW->bitmap.zooming = True;
    BW->bitmap.grid = True; /* potencially true, could use a resource here */

    FixHotSpot(BW);

    Resize(BW);
    if (BW->core.visible)
	XClearArea(XtDisplay(BW), XtWindow(BW),
		   0, 0, 
		   BW->core.width, BW->core.height,
		   True);
}

XImage *ScaleBitmapImage();

void BWRescale(w, width, height)
    Widget w;
    Dimension width, height;
{
    BitmapWidget BW = (BitmapWidget) w;
    XImage *image, *buffer;
    char *buffer_data;

    if (BW->bitmap.zooming)
	ZoomOut(BW);
        
    image = ScaleBitmapImage(BW, BW->bitmap.image, 
		       (double) width / (double) BW->bitmap.image->width,
		       (double) height / (double) BW->bitmap.image->height);

    buffer_data = CreateCleanData(Length(image->width, image->height));
    buffer = CreateBitmapImage(BW, buffer_data, 
			       (Dimension) image->width, 
			       (Dimension) image->height);
    
    TransferImageData(BW->bitmap.buffer, buffer);

    DestroyBitmapImage(&BW->bitmap.image);
    DestroyBitmapImage(&BW->bitmap.buffer);
    
    BW->bitmap.image = image;
    BW->bitmap.buffer = buffer;
    BW->bitmap.width = image->width;
    BW->bitmap.height = image->height;
    
    FixHotSpot(BW);
    FixMark(BW);

    Resize(BW);
    if (BW->core.visible)
	XClearArea(XtDisplay(BW), XtWindow(BW),
		   0, 0, 
		   BW->core.width, BW->core.height,
		   True);
}

Boolean BWQueryZooming(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return BW->bitmap.zooming;
}

void BWResize(w, width, height)
    Widget w;
    Dimension width, height;
{
    BitmapWidget BW = (BitmapWidget) w;
    XImage *image, *buffer;
    char *image_data, *buffer_data;

    if (BW->bitmap.zooming)
	ZoomOut(BW);

    image_data = CreateCleanData(Length(width, height));
    buffer_data = CreateCleanData(Length(width, height));

    image = CreateBitmapImage(BW, image_data, width, height);
    buffer = CreateBitmapImage(BW, buffer_data, width, height);

    TransferImageData(BW->bitmap.image, image);
    TransferImageData(BW->bitmap.buffer, buffer);
    
    DestroyBitmapImage(&BW->bitmap.image);
    DestroyBitmapImage(&BW->bitmap.buffer);

    BW->bitmap.image = image;
    BW->bitmap.buffer = buffer;
    BW->bitmap.width = width;
    BW->bitmap.height = height;

    
    FixHotSpot(BW);
    FixMark(BW);

    Resize(BW);
    if (BW->core.visible)
	XClearArea(XtDisplay(BW), XtWindow(BW),
		   0, 0, 
		   BW->core.width, BW->core.height,
		   True);
}

static void Destroy(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    XtDestroyGC(BW->bitmap.drawing_gc);
    XtDestroyGC(BW->bitmap.highlighting_gc);
    XtDestroyGC(BW->bitmap.framing_gc);
    XtDestroyGC(BW->bitmap.axes_gc);
    BWRemoveAllRequests(w);
}


static void Resize(BW)
     BitmapWidget BW;
{
    Dimension squareW, squareH;

    squareW = max(1, ((int)BW->core.width - 2 * (int)BW->bitmap.distance) / 
		  (int)BW->bitmap.width);
    squareH = max(1, ((int)BW->core.height - 2 * (int)BW->bitmap.distance) / 
		  (int)BW->bitmap.height);

    if (BW->bitmap.proportional)
	BW->bitmap.squareW = BW->bitmap.squareH = min(squareW, squareH);
    else {
	BW->bitmap.squareW = squareW;
	BW->bitmap.squareH = squareH;
    }
    
    BW->bitmap.horizOffset = max((Position)BW->bitmap.distance, 
				 (Position)(BW->core.width - 
					    BW->bitmap.width * 
					    BW->bitmap.squareW) / 2);
    BW->bitmap.vertOffset = max((Position)BW->bitmap.distance, 
				(Position)(BW->core.height - 
					   BW->bitmap.height * 
					   BW->bitmap.squareH) / 2);

    BW->bitmap.grid &= ((BW->bitmap.squareW > BW->bitmap.grid_tolerance) && 
			(BW->bitmap.squareH > BW->bitmap.grid_tolerance));
}

/* ARGSUSED */
static void Redisplay(BW, event, region)
     BitmapWidget BW;
     XEvent      *event;
     Region       region;
{
    if(event->type == Expose) {
	if (BW->core.visible) {  
	    Refresh(BW, 
		    event->xexpose.x, event->xexpose.y,
		    event->xexpose.width, event->xexpose.height);
	}
    }
}

void BWClip(w, from_x, from_y, to_x, to_y)
    Widget  w;
    Position      from_x, from_y,
                  to_x, to_y;
{
    BitmapWidget BW = (BitmapWidget) w;
    XRectangle rectangle;
  
    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
    from_x = max(0, from_x);
    from_y = max(0, from_y);
    to_x = min(BW->bitmap.width - 1, to_x);
    to_y = min(BW->bitmap.height - 1, to_y);

    rectangle.x = InWindowX(BW, from_x);
    rectangle.y = InWindowY(BW, from_y);
    rectangle.width = InWindowX(BW, to_x  + 1) - InWindowX(BW, from_x);
    rectangle.height = InWindowY(BW, to_y + 1) - InWindowY(BW, from_y);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.highlighting_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.drawing_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.framing_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.axes_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
}

void BWUnclip(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    XRectangle rectangle;
  
    rectangle.x = InWindowX(BW, 0);
    rectangle.y = InWindowY(BW, 0);
    rectangle.width = InWindowX(BW, BW->bitmap.width) - InWindowX(BW, 0);
    rectangle.height = InWindowY(BW, BW->bitmap.height) - InWindowY(BW, 0);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.highlighting_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.drawing_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.framing_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.axes_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);
}

void Refresh(BW, x, y, width, height)
    BitmapWidget BW;
    Position     x, y;
    Dimension    width, height;
{
    XRectangle rectangle;

    rectangle.x = min(x, InWindowX(BW, InBitmapX(BW, x)));
    rectangle.y = min(y, InWindowY(BW, InBitmapY(BW, y)));
    rectangle.width = max(x + width,
		     InWindowX(BW, InBitmapX(BW, x + width)+1)) - rectangle.x;
    rectangle.height = max(y + height,
		     InWindowY(BW, InBitmapY(BW, y + height)+1)) - rectangle.y;
    
    XClearArea(XtDisplay(BW), XtWindow(BW),
	       rectangle.x, rectangle.y,
	       rectangle.width, rectangle.height,
	       False);

    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.framing_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);

    XDrawRectangle(XtDisplay(BW), XtWindow(BW),
		   BW->bitmap.framing_gc,
		   InWindowX(BW, 0) - 1, InWindowY(BW, 0) - 1,
		   InWindowX(BW, BW->bitmap.width) - InWindowX(BW, 0) + 1, 
		   InWindowY(BW, BW->bitmap.height) - InWindowY(BW, 0) + 1);

    BWClip((Widget) BW,
	   InBitmapX(BW, x),InBitmapY(BW, y),
	   InBitmapX(BW, x + width), InBitmapY(BW, y + height));

    BWRedrawGrid((Widget) BW,
		 InBitmapX(BW, x), InBitmapY(BW, y),
		 InBitmapX(BW, x + width), InBitmapY(BW, y + height));

    BWRedrawSquares((Widget) BW,
		    InBitmapX(BW, x), InBitmapY(BW, y),
		    InBitmapX(BW, x + width), InBitmapY(BW, y + height));

    BWRedrawMark((Widget) BW);
    BWRedrawHotSpot((Widget) BW);
    BWRedrawAxes((Widget) BW);
    BWUnclip((Widget) BW);
}

Boolean BWQueryGrid(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return BW->bitmap.grid;
}

void BWSwitchGrid(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    BW->bitmap.grid ^= TRUE;
    BWDrawGrid(w,
	       0, 0,
	       BW->bitmap.image->width - 1, BW->bitmap.image->height - 1);
}

void BWGrid(w, _switch)
    Widget w;
    Boolean _switch;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.grid != _switch)
	BWSwitchGrid(w);
}

Boolean BWQueryDashed(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return (BW->bitmap.dashed);
}

void BWSwitchDashed(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    XRectangle rectangle;

    BWRedrawGrid(w,
		 0, 0,
		 (Position) BW->bitmap.width - 1, 
		 (Position) BW->bitmap.height - 1);

    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = BW->core.width;
    rectangle.height = BW->core.height;

    XSetClipRectangles(XtDisplay(BW),
		       BW->bitmap.framing_gc,
		       0, 0,
		       &rectangle, 1,
		       Unsorted);

    XDrawRectangle(XtDisplay(BW), XtWindow(BW),
		   BW->bitmap.framing_gc,
		   InWindowX(BW, 0) - 1, InWindowY(BW, 0) - 1,
		   InWindowX(BW, BW->bitmap.width) - InWindowX(BW, 0) + 1, 
		   InWindowY(BW, BW->bitmap.height) - InWindowY(BW, 0) + 1);
    
    BW->bitmap.dashed ^= True;
    XSetFillStyle(XtDisplay(BW), BW->bitmap.framing_gc,
		  (BW->bitmap.dashed ? FillStippled : FillSolid));
 
    XDrawRectangle(XtDisplay(BW), XtWindow(BW),
		   BW->bitmap.framing_gc,
		   InWindowX(BW, 0) - 1, InWindowY(BW, 0) - 1,
		   InWindowX(BW, BW->bitmap.width) - InWindowX(BW, 0) + 1, 
		   InWindowY(BW, BW->bitmap.height) - InWindowY(BW, 0) + 1);

    BWUnclip(w);
   
    BWRedrawGrid(w,
		 0, 0,
		 (Position) BW->bitmap.width - 1, 
		 (Position) BW->bitmap.height - 1);
}

void BWDashed(w, _switch)
    Widget w;
    Boolean _switch;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.dashed != _switch)
	BWSwitchDashed(w);
}

static Boolean SetValues()
{
    return FALSE;
}

Boolean BWQueryProportional(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    return (BW->bitmap.proportional);
}

void BWSwitchProportional(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    BW->bitmap.proportional ^= True;

    Resize(BW);
    if (BW->core.visible)
	XClearArea(XtDisplay(BW), XtWindow(BW),
		   0, 0, 
		   BW->core.width, BW->core.height,
		   True);
}

void BWProportional(w, _switch)
    Widget w;
    Boolean _switch;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (BW->bitmap.proportional != _switch)
	BWSwitchProportional(w);
}


