/*
 * $XConsortium: Bitmap.c,v 1.9 90/04/14 03:44:27 dmatic Exp $
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
    
void BWDebug(w)
    Widget w;
{
    DEBUG ^= True;
}

/*****************************************************************************\
 *                               Converters                                  *
\*****************************************************************************/

#define Length(width, height)\
	(int)((int)(((width) + 7) / 8) * (height))

#define InBitmapX(BW, x)\
	(Position) (min((Position)((max(BW->bitmap.horizOffset, x)  -\
				   BW->bitmap.horizOffset) /\
				   BW->bitmap.squareW), BW->bitmap.width - 1))
    
#define InBitmapY(BW, y)\
	(Position) (min((Position)((max(BW->bitmap.vertOffset, y)  -\
				   BW->bitmap.vertOffset) /\
				   BW->bitmap.squareH), BW->bitmap.height - 1))
    
#define InWindowX(BW, x)\
	(Position) (BW->bitmap.horizOffset + ((x) * BW->bitmap.squareW))

#define InWindowY(BW, y)\
	(Position) (BW->bitmap.vertOffset + ((y) * BW->bitmap.squareH))
     
 
#define GetPixmap(BW, image)\
    XCreateBitmapFromData(XtDisplay(BW), XtWindow(BW),\
			  image->data, image->width, image->height)

#define CreateCleanData(length) XtCalloc(length, sizeof(char))
 
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
	    XtFree(*image);
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

/*****************************************************************************\
 * Request Machine: stacks up and handles requests from application calls.   * 
\*****************************************************************************/

/*
 * Searches for a request record of a request specified by its name.
 * Returns a pointer to the record or NULL if the request was not found.
 */
BWRequestRec *FindRequest(name)
    BWRequest name;
{
    int i;

    for (i = 0; i < bitmapClassRec.bitmap_class.num_requests; i++)
	if (!strcmp(name, bitmapClassRec.bitmap_class.requests[i].name))
	    return &bitmapClassRec.bitmap_class.requests[i];
    
    return NULL;
}

/*
 * Adds a request to the request stack and does proper initializations.
 * Returns TRUE if the request was found and FALSE otherwise.
 */
Boolean BWAddRequest(w, name, trap, call_data, call_data_size)
    Widget    w;
    BWRequest name;
    Boolean   trap;
    caddr_t   call_data;
    Cardinal  call_data_size;
{
    BitmapWidget BW = (BitmapWidget) w;
    BWRequestRec *request;
    
    if(request = FindRequest(name)) {
	if (DEBUG)
	  fprintf(stderr, "Adding... Cardinal: %d\n", BW->bitmap.cardinal + 1);

	BW->bitmap.request_stack = (BWRequestStack *)
	    XtRealloc(BW->bitmap.request_stack,
		      (++BW->bitmap.cardinal + 1) * sizeof(BWRequestStack));
	
	BW->bitmap.request_stack[BW->bitmap.cardinal].request = request;
	BW->bitmap.request_stack[BW->bitmap.cardinal].status = 
	    XtMalloc(request->status_size);
	BW->bitmap.request_stack[BW->bitmap.cardinal].trap = trap;
	BW->bitmap.request_stack[BW->bitmap.cardinal].call_data = 
	    XtMalloc(call_data_size);
	bcopy(call_data, 
	      BW->bitmap.request_stack[BW->bitmap.cardinal].call_data,
	      call_data_size);

	return True;
    }
    else {
	XtWarning("bad request name.  BitmapWidget");
	return False;
    }
}

/*
 * Engages the request designated by the current parameter.
 * Returnes TRUE if the request has an engage function and FALSE otherwise.
 */
Boolean Engage(BW, current)
    BitmapWidget BW;
    Cardinal current;
{
    BW->bitmap.current = current;
    
    if (DEBUG)
	fprintf(stderr, "Request: %s\n", 
		BW->bitmap.request_stack[current].request->name);
  
    if (BW->bitmap.request_stack[current].request->engage) {
	(*BW->bitmap.request_stack[current].request->engage)
	    ((Widget) BW,
	     BW->bitmap.request_stack[current].status,
	     BW->bitmap.request_stack[current].request->engage_client_data,
	     BW->bitmap.request_stack[current].call_data);
	return True;
    }
    else
	return False;
}

Boolean BWTerminateRequest();
Boolean BWRemoveRequest();

/*
 * Scans down the request stack removing all requests untill it finds 
 * one to be trapped.
 */
void TrappingLoop(BW)
    BitmapWidget BW;
{

    if (DEBUG)
	fprintf(stderr, "Scanning... Current: %d\n", BW->bitmap.current);
    if ((BW->bitmap.current > 0) 
	&& 
	(!BW->bitmap.request_stack[BW->bitmap.current--].trap)) {
	BWRemoveRequest((Widget) BW);
	TrappingLoop(BW);
    }
    else
	if (BW->bitmap.cardinal > 0) {
	    if (DEBUG)
		fprintf(stderr, "Trapping... Current: %d\n", BW->bitmap.current+1);
	    if(!Engage(BW, ++BW->bitmap.current))
		BWTerminateRequest((Widget) BW, True);
	}
}
/*
 * Terimantes the current request and continues with next request if con = TRUE
 * Returnes TRUE if there is any number of requests left on the stack.
 */
Boolean BWTerminateRequest(w, cont)
    Widget w;
    Boolean cont;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.current > 0) {
	if (DEBUG)
	    fprintf(stderr, "Terminating... Current: %d\n", BW->bitmap.current);
    	if (BW->bitmap.request_stack[BW->bitmap.current].request->terminate)
	    (*BW->bitmap.request_stack[BW->bitmap.current].request->terminate)
		(w,
		 BW->bitmap.request_stack[BW->bitmap.current].status,
		 BW->bitmap.request_stack[BW->bitmap.current].request->terminate_client_data,
		 BW->bitmap.request_stack[BW->bitmap.current].call_data);
	
	if (cont) {
	    if (BW->bitmap.current == BW->bitmap.cardinal)
		TrappingLoop(BW);
	    else {
		if (DEBUG)
		    fprintf(stderr, "Continuing... Current: %d\n", BW->bitmap.current+1);
		if (!Engage(BW, ++BW->bitmap.current))
		    BWTerminateRequest(w, True);
	    }
	}
	else
	    BW->bitmap.current = 0;
    }
    
    return BW->bitmap.current;
}

/*
 * Simple interface to BWTerminateRequest that takes only a widget.
 */
void BWTerminate(w)
    Widget w;
{
    BWTerminateRequest(w, True);
}

/*
 * Removes the top request from the request stack. If the request is active
 * it will terminate it.
 * Returns TRUE if the number of requests left on the stack != 0.
 */
Boolean BWRemoveRequest(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.cardinal > 0) {
	if (DEBUG)
	    fprintf(stderr, "Removing... Cardinal: %d\n", BW->bitmap.cardinal);
	if (BW->bitmap.current == BW->bitmap.cardinal)
	    BWTerminateRequest(w, False);
	
	if (BW->bitmap.request_stack[BW->bitmap.cardinal].request->remove)
	    (*BW->bitmap.request_stack[BW->bitmap.cardinal].request->remove)
		(w,
		 BW->bitmap.request_stack[BW->bitmap.cardinal].status,
		 BW->bitmap.request_stack[BW->bitmap.cardinal].request->remove_client_data,
		 BW->bitmap.request_stack[BW->bitmap.cardinal].call_data);
	
	XtFree(BW->bitmap.request_stack[BW->bitmap.cardinal].status);
	XtFree(BW->bitmap.request_stack[BW->bitmap.cardinal].call_data);
	BW->bitmap.request_stack = (BWRequestStack *)
	    XtRealloc(BW->bitmap.request_stack,
		      (--BW->bitmap.cardinal + 1) * sizeof(BWRequestStack));
	
	return True;
    }
    else 
	return False;
}

void BWRemoveAllRequests(w)
    Widget w;
{
    while (BWRemoveRequest(w)) {/* removes all requests from the stack */}
}

/*
 * Adds the request to the stack and performs engaging ritual.
 * Returns TRUE if the request was found, FALSE otherwise.
 */
Boolean BWEngageRequest(w, name, trap, call_data, call_data_size)
    Widget w;
    BWRequest name;
    Boolean trap;
    caddr_t call_data;
    Cardinal call_data_size;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BWAddRequest(w, name, trap, call_data, call_data_size)) {
	BWTerminateRequest(w, False);
	if (DEBUG)
	    fprintf(stderr, "Engaging... Cardinal: %d\n", BW->bitmap.cardinal);
	if (!Engage(BW, BW->bitmap.cardinal))
	    BWTerminateRequest(w, True);
	
	return True;
    }
    else
	return False;
}

/************************* End of the Request Machine ************************/

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

/*****************************************************************************\
 *                                   Graphics                                *
\*****************************************************************************/


#define QuerySet(x, y) (((x) != NotSet) && ((y) != NotSet))

#define bit int

#define QueryZero(x, y) (((x) == 0) || ((y) == 0))

#define Swap(x, y) {Position t; t = x; x = y; y = t;}

#define QuerySwap(x, y) if(x > y) Swap(x, y)

#define QueryInBitmap(BW, x, y)\
  (((x) >= 0) && ((x) < BW->bitmap.image->width) &&\
   ((y) >= 0) && ((y) < BW->bitmap.image->height))

#define Value(BW, button)   (BW->bitmap.button_action[button - 1])

#define GetBit(image, x, y)\
    ((bit)((*(image->data + (x) / 8 + (y) * image->bytes_per_line) &\
	    (1 << ((x) % 8))) ? 1 : 0))


bit BWGetBit(w, x, y)
    Widget  w;
     Position      x, y;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (QueryInBitmap(BW, x, y))
	return GetBit(BW->bitmap.image, x, y);
    else
	return NotSet;
}


#define InvertBit(image, x, y)\
    (*(image->data + (x) / 8 + (y) * image->bytes_per_line) ^=\
     (bit) (1 << ((x) % 8)))


#define SetBit(image, x, y)\
    (*(image->data + (x) / 8 + (y) * image->bytes_per_line) |=\
     (bit) (1 << ((x) % 8)))

#define ClearBit(image, x, y)\
    (*(image->data + (x) / 8 + (y) * image->bytes_per_line) &=\
     (bit)~(1 << ((x) % 8)))

/*
#define HighlightSquare(BW, x, y)\
    XFillRectangle(XtDisplay(BW), XtWindow(BW),\
                   BW->bitmap.highlighting_gc,\
		   InWindowX(BW, x), InWindowY(BW, y),\
                   BW->bitmap.squareW, BW->bitmap.squareH)
*/
void HighlightSquare(BW, x, y)
    BitmapWidget BW;
    Position x, y;
{
    XFillRectangle(XtDisplay(BW), XtWindow(BW),
                   BW->bitmap.highlighting_gc,
		   InWindowX(BW, x), InWindowY(BW, y),
                   BW->bitmap.squareW, BW->bitmap.squareH);
}

/*
#define DrawSquare(BW, x, y)\
    XFillRectangle(XtDisplay(BW), XtWindow(BW),\
                   BW->bitmap.drawing_gc,\
		   InWindowX(BW, x), InWindowY(BW, y),\
                   BW->bitmap.squareW, BW->bitmap.squareH) 
*/

void DrawSquare(BW, x, y)
    BitmapWidget BW;
    Position x, y;
{
    XFillRectangle(XtDisplay(BW), XtWindow(BW),
                   BW->bitmap.drawing_gc,
		   InWindowX(BW, x), InWindowY(BW, y),
                   BW->bitmap.squareW, BW->bitmap.squareH);
}

#define InvertPoint(BW, x, y)\
    {InvertBit(BW->bitmap.image, x, y); DrawSquare(BW, x, y);}

#define DrawPoint(BW, x, y, value)\
    if (GetBit(BW->bitmap.image, x, y) != value)\
       InvertPoint(BW, x, y)

void BWDrawPoint(w, x, y, value)
    Widget  w;
    Position      x, y;
    bit           value;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (QueryInBitmap(BW, x, y)) {
	if (value == Highlight)
	    HighlightSquare(BW, x, y);
	else
	    DrawPoint(BW, x, y, value);
    }
}

XPoint *HotSpotShape(BW, x ,y)
    BitmapWidget BW;
    Position     x, y;
{
    static XPoint points[5];
  
    points[0].x = InWindowX(BW, x);
    points[0].y = InWindowY(BW, y + 1.0/2);
    points[1].x = InWindowX(BW, x + 1.0/2);
    points[1].y = InWindowY(BW, y + 1);
    points[2].x = InWindowX(BW, x + 1);
    points[2].y = InWindowY(BW, y + 1.0/2);
    points[3].x = InWindowX(BW, x + 1.0/2);
    points[3].y = InWindowY(BW, y);
    points[4].x = InWindowX(BW, x);
    points[4].y = InWindowY(BW, y + 1.0/2);
    
    return points;
}

#define DrawHotSpot(BW, x, y)\
  XFillPolygon(XtDisplay(BW), XtWindow(BW), BW->bitmap.drawing_gc,\
	       HotSpotShape(BW, x, y), 5, Convex, CoordModeOrigin)

#define HighlightHotSpot(BW, x, y)\
  XFillPolygon(XtDisplay(BW), XtWindow(BW), BW->bitmap.highlighting_gc,\
	       HotSpotShape(BW, x, y), 5, Convex, CoordModeOrigin)

XImage *CreateBitmapImage();
void DestroyBitmapImage();

void BWRedrawHotSpot(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y))
	DrawHotSpot(BW, BW->bitmap.hot.x, BW->bitmap.hot.y);
}

void BWClearHotSpot(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    DrawHotSpot(BW, BW->bitmap.hot.x, BW->bitmap.hot.y);
    BW->bitmap.hot.x = BW->bitmap.hot.y = NotSet;
}

void BWDrawHotSpot(w, x, y, value)
    Widget w;
    Position x, y;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (QueryInBitmap(BW, x, y)) {
	if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y) &&
	    ((BW->bitmap.hot.x == x) && (BW->bitmap.hot.y == y))) {
	    if ((value == Clear) || (value == Invert)) {
		BWClearHotSpot(w);
	    }
	}
	else
	    if ((value == Set) || (value == Invert)) {
		BWClearHotSpot(w);
		DrawHotSpot(BW, x, y);
		BW->bitmap.hot.x = x;
		BW->bitmap.hot.y = y;
	    }
	
	if (value == Highlight)
	    HighlightHotSpot(BW, x, y); 
    }
}

void BWSetHotSpot(w, x, y)
    Widget w;
    Position x, y;
{
    if (QuerySet(x, y))
	BWDrawHotSpot(w, x, y, Set);
    else
	BWClearHotSpot(w);
}

void CopyImageData();

/* high level procedures */

void BWRedrawSquares(w, from_x, from_y, to_x, to_y)
    Widget  w;
    Position      from_x, from_y,
                  to_x, to_y;
{
    BitmapWidget BW = (BitmapWidget) w;
    register int x, y;

    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
    from_x = max(0, from_x);
    from_y = max(0, from_y);
    to_x = min(BW->bitmap.image->width - 1, to_x);
    to_y = min(BW->bitmap.image->height - 1, to_y);
  
    for (x = from_x; x <= to_x; x++)
	for (y = from_y; y <= to_y; y++)
	    if (GetBit(BW->bitmap.image, x, y)) DrawSquare(BW, x, y);
}

void BWDrawGrid(w, from_x, from_y, to_x, to_y)
     Widget w;
     Position from_x, from_y,
              to_x, to_y;
{
    BitmapWidget BW = (BitmapWidget) w;
    int i;
  
    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
    from_x = max(0, from_x);
    from_y = max(0, from_y);
    to_x = min(BW->bitmap.image->width - 1, to_x);
    to_y = min(BW->bitmap.image->height - 1, to_y);
  
    for(i = from_x + (from_x == 0); i <= to_x; i++)
	XDrawLine(XtDisplay(BW), XtWindow(BW), 
		  BW->bitmap.framing_gc,
		  InWindowX(BW, i), InWindowY(BW, from_y),
		  InWindowX(BW, i), InWindowY(BW, to_y + 1));
  
    for(i = from_y + (from_y == 0); i <= to_y; i++)
	XDrawLine(XtDisplay(BW), XtWindow(BW), 
		  BW->bitmap.framing_gc,
		  InWindowX(BW, from_x), InWindowY(BW, i),
		  InWindowX(BW, to_x + 1), InWindowY(BW, i));
}


void BWRedrawGrid(w, from_x, from_y, to_x, to_y)
    Widget w;
    Position from_x, from_y,
	to_x, to_y;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (BW->bitmap.grid)
	BWDrawGrid(w, from_x, from_y, to_x, to_y);
}

void BWDrawLine(w, from_x, from_y, to_x, to_y, value)
    Widget  w;
    Position      from_x, from_y,
                  to_x, to_y;
    int           value;
{
    Position i;
    register double x, y;
    double dx, dy, delta;

    dx = to_x - from_x;
    dy = to_y - from_y;
    x = from_x + 0.5;
    y = from_y + 0.5;
    delta = max(abs(dx), abs(dy));
    if (delta > 0) {
	dx /= delta;
	dy /= delta;
	for(i = 0; i <= delta; i++) {
	    BWDrawPoint(w, (Position) x, (Position) y, value);
	    x += dx;
	    y += dy;
	}
    }
    else
	BWDrawPoint(w, from_x, from_y, value);
}

void BWBlindLine(w, from_x, from_y, to_x, to_y, value)
    Widget  w;
    Position      from_x, from_y,
                  to_x, to_y;
    int           value;
{
    Position i;
    register double x, y;
    double dx, dy, delta;

    dx = to_x - from_x;
    dy = to_y - from_y;
    x = from_x + 0.5;
    y = from_y + 0.5;
    delta = max(abs(dx), abs(dy));
    if (delta > 0) {
	dx /= delta;
	dy /= delta;
	x += dx;
	y += dy;
	for(i = 1; i <= delta; i++) {
	    BWDrawPoint(w, (Position) x, (Position) y, value);
	    x += dx;
	    y += dy;
	}
    }
    else
	BWDrawPoint(w, from_x, from_y, value);
}

void BWDrawRectangle(w, from_x, from_y, to_x, to_y, value)
    Widget w;
    Position     from_x, from_y,
	to_x, to_y;
    bit          value;
{
    register Position i;
    Dimension delta, width, height;
    
    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
    
    width = to_x - from_x;
    height = to_y - from_y;

    delta = max(width, height);
    
    if (!QueryZero(width, height)) {
	for (i = 0; i < delta; i++) {
	    if (i < width) {
		BWDrawPoint(w, from_x + i, from_y, value);
		BWDrawPoint(w, to_x - i, to_y, value);
	    }
	    if (i < height) {
		BWDrawPoint(w, from_x, to_y - i, value);
		BWDrawPoint(w, to_x, from_y + i, value);
	    }
	}
    }
    else
	BWDrawLine(w, 
		   from_x, from_y, 
		   to_x, to_y, value);
}
  
void BWDrawFilledRectangle(w, from_x, from_y, to_x, to_y, value)
    Widget   w;
    Position from_x, from_y,
	     to_x, to_y;
    bit      value;
{
    register Position x, y;

    QuerySwap(from_x, to_x);
    QuerySwap(from_y, to_y);
    
    for (x = from_x; x <= to_x; x++)
	for (y = from_y; y <= to_y; y++)
	    BWDrawPoint(w, x, y, value);
}

void BWDrawCircle(w, origin_x, origin_y, point_x, point_y, value)
    Widget  w;
    Position      origin_x, origin_y,
	point_x, point_y;
    bit           value;
{
    register Position i, delta;
    Dimension dx, dy, half;
    double radius;
    
    dx = abs(point_x - origin_x);
    dy = abs(point_y - origin_y);
    radius = sqrt((double) (dx * dx + dy * dy));
    if (radius < 1.0) {
	BWDrawPoint(w, origin_x, origin_y, value);
    }
    else {
	BWDrawPoint(w, origin_x - (Position) floor(radius), origin_y, value);
	BWDrawPoint(w, origin_x + (Position) floor(radius), origin_y, value);
	BWDrawPoint(w, origin_x, origin_y - (Position) floor(radius), value);
	BWDrawPoint(w, origin_x, origin_y + (Position) floor(radius), value);
    }
    half = radius / sqrt(2.0);
    for(i = 1; i <= half; i++) {
	delta = sqrt(radius * radius - i * i);
	BWDrawPoint(w, origin_x - delta, origin_y - i, value);
	BWDrawPoint(w, origin_x - delta, origin_y + i, value);
	BWDrawPoint(w, origin_x + delta, origin_y - i, value);
	BWDrawPoint(w, origin_x + delta, origin_y + i, value);
	if (i != delta) {
	    BWDrawPoint(w, origin_x - i, origin_y - delta, value);
	    BWDrawPoint(w, origin_x - i, origin_y + delta, value);
	    BWDrawPoint(w, origin_x + i, origin_y - delta, value);
	    BWDrawPoint(w, origin_x + i, origin_y + delta, value);
	}
    }
}

void BWDrawFilledCircle(w, origin_x, origin_y, point_x, point_y, value)
    Widget  w;
    Position      origin_x, origin_y,
	          point_x, point_y;
    bit           value;
{
    register Position i, j, delta;
    Dimension dx, dy;
    double radius;
    
    dx = abs(point_x - origin_x);
    dy = abs(point_y - origin_y);
    radius = sqrt((double) (dx * dx + dy * dy));
    for(j = origin_x - (Position) floor(radius); 
	j <= origin_x + (Position) floor(radius); j++)
	BWDrawPoint(w, j, origin_y, value);
    for(i = 1; i <= (Position) floor(radius); i++) {
	delta = sqrt(radius * radius - i * i);
	for(j = origin_x - delta; j <= origin_x + delta; j++) {
	    BWDrawPoint(w, j, origin_y - i, value);
	    BWDrawPoint(w, j, origin_y + i, value);
	}
    }
}

#define QueryFlood(BW, x, y, value)\
    ((GetBit(BW->bitmap.image, x, y) !=\
      (value & 1)) && QueryInBitmap(BW, x, y))

#define Flood(BW, x, y, value)\
    {if (value == Highlight) HighlightSquare(BW, x, y);\
     else InvertPoint(BW, x, y);}

/*
void FloodLoop(BW, x, y, value)
    BitmapWidget  BW;
    Position      x, y;
    bit           value;
{
    if (QueryFlood(BW, x, y, value)) {
	Flood(BW, x, y, value);
	FloodLoop(BW, x, y - 1, value);
	FloodLoop(BW, x - 1, y, value);
	FloodLoop(BW, x, y + 1, value);
	FloodLoop(BW, x + 1, y, value);
    }
}
*/

void FloodLoop(BW, x, y, value)
    BitmapWidget BW;
    Position     x, y;
    bit          value;
{
    Position save_x, save_y, x_left, x_right;
    
    if (QueryFlood(BW, x, y, value)) 
	Flood(BW, x, y, value)


    save_x = x;
    save_y = y;

    x++;
    while (QueryFlood(BW, x, y, value)) {
	Flood(BW, x, y, value);
	x++;
    }
    x_right = --x;

    x = save_x;
    x--;
    while (QueryFlood(BW, x, y, value)) {
	Flood(BW, x, y, value);
	x--;
    }
    x_left = ++x;


    x = x_left;
    y = save_y;
    y++;
    
    while (x <= x_right) {
	Boolean flag = False;
	Position x_enter;
	
	while (QueryFlood(BW, x, y, value) && (x <= x_right)) {
	    flag = True;
	    x++;
	}
	
	if (flag) {
	    if ((x == x_right) && QueryFlood(BW, x, y, value))
		FloodLoop(BW, x, y, value);
	    else
		FloodLoop(BW, x - 1, y, value);
	}
	
	x_enter = x;
	
	while (!QueryFlood(BW, x, y, value) && (x < x_right))
	    x++;
	
	if (x == x_enter) x++;
    }

    x = x_left;
    y = save_y;
    y--;

    while (x <= x_right) {
	Boolean flag = False;
	Position x_enter;
	
	while (QueryFlood(BW, x, y, value) && (x <= x_right)) {
	    flag = True;
	    x++;
	}
	
	if (flag) {
	    if ((x == x_right) && QueryFlood(BW, x, y, value))
		FloodLoop(BW, x, y, value);
	    else
		FloodLoop(BW, x - 1, y, value);
	}
	
	x_enter = x;
	
	while (!QueryFlood(BW, x, y, value) && (x < x_right))
	    x++;
	
	if (x == x_enter) x++;
    }
}

void BWFloodFill(w, x, y, value)
    Widget  w;
    Position      x, y;
    bit           value;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    FloodLoop(BW, x, y, (value == Invert) ? 
	      !GetBit(BW->bitmap.image, x, y) : value);
}

Boolean BWQueryMarked();
#define QueryHotInMark(BW)\
    ((BW->bitmap.hot.x == max(BW->bitmap.mark.from_x,\
			      min(BW->bitmap.hot.x, BW->bitmap.mark.to_x)))\
     &&\
     (BW->bitmap.hot.y == max(BW->bitmap.mark.from_y,\
			      min(BW->bitmap.hot.y, BW->bitmap.mark.to_y))))

void BWUp(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    bit first, up, down;
    Position from_x, from_y, to_x, to_y;

    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }

    if ((to_y - from_y) == 0)
	return;
    
    for(x = from_x; x <= to_x; x++) {
	first = up = GetBit(BW->bitmap.image, x, to_y);
	for(y = to_y - 1; y >= from_y; y--) {
	    down = GetBit(BW->bitmap.image, x, y);
	    if (up != down) 
		InvertPoint(BW, x, y);
	    up =down;
	}
	if(first != down)
	    InvertPoint(BW, x, to_y);
    }

    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w))
	BWSetHotSpot(w,
		     BW->bitmap.hot.x,
		     (BW->bitmap.hot.y - 1 + BW->bitmap.image->height) % 
		     BW->bitmap.image->height);

}

void BWDown(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    bit first, down, up;
    Position from_x, from_y, to_x, to_y;

    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }

    if ((to_y - from_y) == 0)
	return;

    for(x = from_x; x <= to_x; x++) {
	first = down = GetBit(BW->bitmap.image, x, from_y);
	for(y = from_y + 1; y <= to_y; y++) {
	    up = GetBit(BW->bitmap.image, x, y);
	    if (down != up)
		InvertPoint(BW, x, y);
	    down = up;
	}
	if(first != up) 
	    InvertPoint(BW, x, from_y);
    }
    
    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w))
	BWSetHotSpot(w,
		     BW->bitmap.hot.x,
		     (BW->bitmap.hot.y + 1) % BW->bitmap.image->height);
}

void BWLeft(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    bit first, left, right;
    Position from_x, from_y, to_x, to_y;
    
    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }

    if ((to_x - from_x) == 0)
	return;
    
    for(y = from_y; y <= to_y; y++) {
	first = left = GetBit(BW->bitmap.image, to_x, y);
	for(x = to_x - 1; x >= from_x; x--) {
	    right = GetBit(BW->bitmap.image, x, y);
	    if (left != right)
		InvertPoint(BW, x, y);
	    left = right;
	}
	if(first != right)
	    InvertPoint(BW, to_x, y);
    }
    
    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w))
	BWSetHotSpot(w,
		     (BW->bitmap.hot.x - 1 + BW->bitmap.image->width) % 
		     BW->bitmap.image->width,
		     BW->bitmap.hot.y);
}

void BWRight(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    bit first, right, left;
    Position from_x, from_y, to_x, to_y;
    
    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }
    
    if ((to_x - from_x) == 0)
	return;
    
    for(y = from_y; y <= to_y; y++) {
	first = right = GetBit(BW->bitmap.image, from_x, y);
	for(x = from_x + 1; x <= to_x; x++) {
	    left = GetBit(BW->bitmap.image, x, y);
	    if (right != left)
		InvertPoint(BW, x, y);
	    right = left;
	}
	if(first != left)
	    InvertPoint(BW, from_x, y);
    }

    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w))
	BWSetHotSpot(w,
		     (BW->bitmap.hot.x + 1) % BW->bitmap.image->width,
		     BW->bitmap.hot.y);
}

void TransferImageData();

void BWFold(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    Position x, y, new_x, new_y;
    Dimension horiz, vert;
    char *storage_data;
    XImage *storage;
	
    storage_data = CreateCleanData(Length(BW->bitmap.image->width, 
					  BW->bitmap.image->height));

    storage = CreateBitmapImage(BW, storage_data, 
				BW->bitmap.image->width, 
				BW->bitmap.image->height);

    TransferImageData(BW->bitmap.image, storage);

    BW->bitmap.fold ^= True;
    horiz = (BW->bitmap.image->width + BW->bitmap.fold) / 2;
    vert = (BW->bitmap.image->height + BW->bitmap.fold) / 2;
    
    for (x = 0; x < BW->bitmap.image->width; x++)
	for (y = 0; y < BW->bitmap.image->height; y++) {
	    new_x = (x + horiz) % BW->bitmap.image->width;
	    new_y = (y + vert) % BW->bitmap.image->height;
	    if(GetBit(BW->bitmap.image, new_x, new_y) != 
	       GetBit(storage, x, y))
		InvertPoint(BW, new_x, new_y);
	}
    
    DestroyBitmapImage(&storage);

    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y))
	BWSetHotSpot(w, 
		     (Position) 
		     ((BW->bitmap.hot.x + horiz) % BW->bitmap.image->width),
		     (Position)
		     ((BW->bitmap.hot.y + vert) % BW->bitmap.image->height));
}


void BWClear(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    int i, length;

    length = Length(BW->bitmap.image->width, BW->bitmap.image->height);

    for (x = 0; x < BW->bitmap.image->width; x++)
	for (y = 0; y < BW->bitmap.image->height; y++)
	    if (GetBit(BW->bitmap.image, x, y))
		DrawSquare(BW, x, y);
    
    for (i = 0; i < length; i++)
	BW->bitmap.image->data[i] = 0;

}

void BWSet(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    int i, length;
    
    length = Length(BW->bitmap.image->width, BW->bitmap.image->height);
    
    for (x = 0; x < BW->bitmap.image->width; x++)
	for (y = 0; y < BW->bitmap.image->height; y++)
	    if (!GetBit(BW->bitmap.image, x, y))
		DrawSquare(BW, x, y);
    
    for (i = 0; i < length; i++)
	BW->bitmap.image->data[i] = 255;

}
 
void BWRedraw(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    XClearArea(XtDisplay(BW), XtWindow(BW),
	       0, 0, BW->core.width, BW->core.height,
	       True);
}

void BWInvert(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    int i, length;

    length = Length(BW->bitmap.image->width, BW->bitmap.image->height);

    XFillRectangle(XtDisplay(BW), XtWindow(BW),
		   BW->bitmap.drawing_gc,
		   InWindowX(BW, 0), InWindowY(BW, 0),
		   InWindowX(BW, BW->bitmap.image->width) - InWindowX(BW, 0),
		   InWindowY(BW, BW->bitmap.image->height) - InWindowY(BW, 0));
    
    for (i = 0; i < length; i++)
	BW->bitmap.image->data[i] ^= 255;
}

void BWFlipHoriz(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    Position from_x, from_y, to_x, to_y;
    float half;
    
    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }
    half = (float) (to_y - from_y) / 2.0 + 0.5;
    
    if (half == 0.0)
	return;
    
    for (x = from_x; x <= to_x; x++) 
	for (y = 0; y <  half; y++)
	    if (GetBit(BW->bitmap.image, x, from_y + y) != 
		GetBit(BW->bitmap.image, x, to_y - y)) {
		InvertPoint(BW, x, from_y + y);
		InvertPoint(BW, x, to_y - y);
	    }
    
    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w))
	BWSetHotSpot(w,
		     BW->bitmap.hot.x,
		     BW->bitmap.image->height - 1 - BW->bitmap.hot.y);
}

void BWFlipVert(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    register Position x, y;
    Position from_x, from_y, to_x, to_y;
    float half;
    
    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }
    half = (float) (to_x - from_x) / 2.0 + 0.5;

    if (half == 0)
	return;

    for (y = from_y; y <= to_y; y++)
	for (x = 0; x < half; x++)
	    if (GetBit(BW->bitmap.image, from_x + x, y) != 
		GetBit(BW->bitmap.image, to_x - x, y)) {
		InvertPoint(BW, from_x + x, y);
		InvertPoint(BW, to_x - x, y);
	    }
    
    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w))
	BWSetHotSpot(w,
		     BW->bitmap.image->width - 1 - BW->bitmap.hot.x,
		     BW->bitmap.hot.y);
}


void BWRotateRight(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    Position x, y, delta, shift, tmp;
    Position half_width, half_height;
    XPoint hot;
    bit quad1, quad2, quad3, quad4;
    Position from_x, from_y, to_x, to_y;
    
    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }

    half_width = floor((to_x - from_x) / 2.0 + 0.5);
    half_height = floor((to_y - from_y ) / 2.0 + 0.5);
    shift = min((Position)(to_x - from_x), (Position)(to_y - from_y )) % 2;
    delta = min((Position) half_width, (Position) half_height) - shift;
    
    for (x = 0; x <= delta; x++) {
	for (y = 1 - shift; y <= delta; y++) {
	    quad1 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width + x, 
			   from_y + (Position)half_height + y);
	    quad2 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width + y, 
			   from_y + (Position)half_height - shift - x);
	    quad3 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width - shift - x, 
			   from_y + (Position)half_height - shift - y);
	    quad4 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width - shift - y, 
			   from_y + (Position)half_height + x);

	    if (quad1 != quad2)
		InvertPoint(BW, 
			    from_x + (Position)half_width + x, 
			    from_y + (Position)half_height + y);
	    if (quad2 != quad3)
		InvertPoint(BW, 
			    from_x + (Position)half_width + y, 
			    from_y + (Position)half_height - shift - x);
	    if (quad3 != quad4)
		InvertPoint(BW, 
			    from_x + (Position)half_width - shift - x,
			    from_y + (Position)half_height - shift - y);
	    if (quad4 != quad1)
		InvertPoint(BW, 
			    from_x + (Position)half_width - shift - y, 
			    from_y + (Position)half_height + x);
	}
    }
    
    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w)) {
	hot.x = BW->bitmap.hot.x - half_width;
	hot.y = BW->bitmap.hot.y - half_height;
	if (hot.x >= 0) hot.x += shift;
	if (hot.y >= 0) hot.y += shift;
	tmp = hot.x;
	hot.x = - hot.y;
	hot.y = tmp;
	if (hot.x > 0) hot.x -= shift;
	if (hot.y > 0) hot.y -= shift;
	hot.x += half_width;
	hot.y += half_height;
	if (QueryInBitmap(BW, hot.x, hot.y))
	    BWSetHotSpot(w, hot.x, hot.y);
    }
    
}

void BWRotateLeft(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    Position x, y,delta, shift, tmp;
    Position half_width, half_height;
    XPoint hot;
    bit quad1, quad2, quad3, quad4;
    Position from_x, from_y, to_x, to_y;
    
    if (BWQueryMarked(w)) {
	from_x = BW->bitmap.mark.from_x;
	from_y = BW->bitmap.mark.from_y;
	to_x = BW->bitmap.mark.to_x;
	to_y = BW->bitmap.mark.to_y;
    }
    else {
	from_x = 0;
	from_y = 0;
	to_x = BW->bitmap.width - 1;
	to_y = BW->bitmap.height - 1;
    }

    half_width = floor((to_x - from_x) / 2.0 + 0.5);
    half_height = floor((to_y - from_y ) / 2.0 + 0.5);
    shift = min((Position)(to_x - from_x), (Position)(to_y - from_y )) % 2;
    delta = min((Position) half_width, (Position) half_height) - shift;
    
    for (x = 0; x <= delta; x++) {
	for (y = 1 - shift; y <= delta; y++) {
	    quad1 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width + x, 
			   from_y + (Position)half_height + y);
	    quad2 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width + y, 
			   from_y + (Position)half_height - shift - x);
	    quad3 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width - shift - x, 
			   from_y + (Position)half_height - shift - y);
	    quad4 = GetBit(BW->bitmap.image, 
			   from_x + (Position)half_width - shift - y, 
			   from_y + (Position)half_height + x);

	    if (quad1 != quad4)
		InvertPoint(BW, 
			    from_x + (Position)half_width + x, 
			    from_y + (Position)half_height + y);
	    if (quad2 != quad1)
		InvertPoint(BW, 
			    from_x + (Position)half_width + y, 
			    from_y + (Position)half_height - shift - x);
	    if (quad3 != quad2)
		InvertPoint(BW, 
			    from_x + (Position)half_width - shift - x,
			    from_y + (Position)half_height - shift - y);
	    if (quad4 != quad3)
		InvertPoint(BW, 
			    from_x + (Position)half_width - shift - y, 
			    from_y + (Position)half_height + x);
	}
    }
    
    if (QuerySet(BW->bitmap.hot.x, BW->bitmap.hot.y)
	&&
	!BWQueryMarked(w)) {
	hot.x = BW->bitmap.hot.x - half_width;
	hot.y = BW->bitmap.hot.y - half_height;
	if (hot.x >= 0) hot.x += shift;
	if (hot.y >= 0) hot.y += shift;
	tmp = hot.x;
	hot.x = hot.y;
	hot.y = - tmp;
	if (hot.x > 0) hot.x -= shift;
	if (hot.y > 0) hot.y -= shift;
	hot.x += half_width;
	hot.y += half_height;
	if (QueryInBitmap(BW, hot.x, hot.y))
	    BWSetHotSpot(w, hot.x, hot.y);
    }
}


void CopyImageData(source, destination, from_x, from_y, to_x, to_y, at_x, at_y)
    XImage *source, *destination;
    Position  from_x, from_y, to_x, to_y, at_x, at_y;
{
    Position x, y, delta_x, delta_y;
    
    delta_x = to_x - from_x + 1;
    delta_y = to_y - from_y + 1;
    
    for (x = 0; x < delta_x; x++)
	for (y = 0; y < delta_y; y++)
	    if (GetBit(source, from_x + x, from_y + y))
		SetBit(destination, at_x + x, at_y + y);
	    else
		ClearBit(destination, at_x + x, at_y + y);
}
      
XImage *ConvertToBitmapImage(BW, image)
    BitmapWidget BW;
    XImage *image;
{
    XImage *bitmap_image;
    char   *data;
    Position x, y;
    
    data = CreateCleanData(Length(image->width, image->height));
    bitmap_image = CreateBitmapImage(BW, data, image->width, image->height);

    for (x = 0; x < min(image->width, bitmap_image->width); x++)
	for (y = 0; y < min(image->height, bitmap_image->height); y++)
	    if ((XGetPixel(image, x, y) != 0) != GetBit(bitmap_image, x, y))
		InvertBit(bitmap_image, x, y);

    return bitmap_image;
}

void TransferImageData(source, destination)
    XImage *source, *destination;
{
    Position x, y;
    
    for (x = 0; x < min(source->width, destination->width); x++)
	for (y = 0; y < min(source->height, destination->height); y++)
	    if (GetBit(source, x, y) != GetBit(destination, x, y))
		InvertBit(destination, x, y);
}

void BWStore(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    Dimension width, height;
    char *storage_data;

    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y)) {

	DestroyBitmapImage(&BW->bitmap.storage);

	width = BW->bitmap.mark.to_x - BW->bitmap.mark.from_x + 1;
	height = BW->bitmap.mark.to_y - BW->bitmap.mark.from_y + 1;
	
	storage_data = CreateCleanData(Length(width, height));

	BW->bitmap.storage = CreateBitmapImage(BW,
					       storage_data,
					       width, height);

	CopyImageData(BW->bitmap.image, BW->bitmap.storage,
		      BW->bitmap.mark.from_x,  BW->bitmap.mark.from_y,
		      BW->bitmap.mark.to_x,  BW->bitmap.mark.to_y,
		      0, 0);
    }
}

void BWClearMarked(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y))
	BWDrawFilledRectangle(w,
			      BW->bitmap.mark.from_x,
			      BW->bitmap.mark.from_y,
			      BW->bitmap.mark.to_x,
			      BW->bitmap.mark.to_y,
			      Clear);
}


void BWDragMarked(w, at_x, at_y)
    Widget w;
    Position     at_x, at_y;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y))
	BWDrawRectangle(w, 
			at_x, at_y, 
			at_x + BW->bitmap.mark.to_x - BW->bitmap.mark.from_x,
			at_y + BW->bitmap.mark.to_y - BW->bitmap.mark.from_y,
			Highlight);
}

void BWDragStored(w, at_x, at_y)
    Widget w;
    Position     at_x, at_y;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.storage)
	BWDrawRectangle(w, 
			at_x, at_y,
			at_x + BW->bitmap.storage->width - 1,
			at_y + BW->bitmap.storage->height - 1,
			Highlight);
}

void DrawImageData(BW, image, at_x, at_y, value)
    BitmapWidget BW;
    XImage *image;
    Position at_x, at_y;
    bit value;
{
    Position x, y;
    Boolean  C, S, I, H;
    bit      A, B;

    C = value == Clear;
    S = value == Set;
    I = value == Invert;
    H = value == Highlight;

    for (x = 0; x < image->width; x++) 
	for (y = 0; y < image->height; y++) {
	    A = GetBit(image, x, y);
	    B = GetBit(BW->bitmap.image, at_x + x, at_y + y);
	    if (A & C | (A | B) & S | (A ^ B) & I | (A | B) & H)
		value = (A & H) ? Highlight : Set;
	    else
		value = Clear;
	    BWDrawPoint((Widget) BW, 
			 at_x + x, at_y + y, 
			 value);
	}
}

void BWRestore(w, at_x, at_y, value)
    Widget w;
    Position     at_x, at_y;
    int          value;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (BW->bitmap.storage)
	DrawImageData(BW, BW->bitmap.storage, at_x, at_y, value);
}

void BWCopy(w, at_x, at_y, value)
    Widget w;
    Position     at_x, at_y;
    int          value;
{
    BitmapWidget BW = (BitmapWidget) w;
    XImage *storage;
    char *storage_data;
    Dimension width, height;

    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y)) {

	width = BW->bitmap.mark.to_x - BW->bitmap.mark.from_x + 1;
	height = BW->bitmap.mark.to_y - BW->bitmap.mark.from_y + 1;
	
	storage_data = CreateCleanData(Length(width, height));

	storage = CreateBitmapImage(BW, storage_data, width, height);

	CopyImageData(BW->bitmap.image, storage,
		      BW->bitmap.mark.from_x,  BW->bitmap.mark.from_y,
		      BW->bitmap.mark.to_x,  BW->bitmap.mark.to_y,
		      0, 0);

	DrawImageData(BW, storage, at_x, at_y, value);

	DestroyBitmapImage(&storage);
    }
}

void BWMark();

void BWMove(w, at_x, at_y, value)
    Widget   w;
    Position at_x, at_y;
    int      value;
{
    BitmapWidget BW = (BitmapWidget) w;
    XImage *storage;
    char *storage_data;
    Dimension width, height;

    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y)) {

	width = BW->bitmap.mark.to_x - BW->bitmap.mark.from_x + 1;
	height = BW->bitmap.mark.to_y - BW->bitmap.mark.from_y + 1;
	
	storage_data = CreateCleanData(Length(width, height));

	storage = CreateBitmapImage(BW, storage_data, width, height);

	CopyImageData(BW->bitmap.image, storage,
		      BW->bitmap.mark.from_x,  BW->bitmap.mark.from_y,
		      BW->bitmap.mark.to_x,  BW->bitmap.mark.to_y,
		      0, 0);

	BWDrawFilledRectangle(w,
			      BW->bitmap.mark.from_x, BW->bitmap.mark.from_y,
			      BW->bitmap.mark.to_x, BW->bitmap.mark.to_y,
			      Clear);

	DrawImageData(BW, storage, at_x, at_y, value);

	BWMark(w, at_x, at_y,
	     at_x + BW->bitmap.mark.to_x - BW->bitmap.mark.from_x,
	     at_y + BW->bitmap.mark.to_y - BW->bitmap.mark.from_y);

	DestroyBitmapImage(&storage);
    }
}

void BWRedrawMark(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y)) 
	XFillRectangle(XtDisplay(BW), XtWindow(BW), BW->bitmap.highlighting_gc,
		       InWindowX(BW, BW->bitmap.mark.from_x), 
		       InWindowY(BW, BW->bitmap.mark.from_y), 
		       InWindowX(BW, BW->bitmap.mark.to_x + 1) - 
		       InWindowX(BW, BW->bitmap.mark.from_x),
		       InWindowY(BW, BW->bitmap.mark.to_y + 1) -
		       InWindowY(BW, BW->bitmap.mark.from_y));
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


void BWStoreToBuffer(w)
     Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    bcopy(BW->bitmap.image->data, BW->bitmap.buffer->data,
	  Length(BW->bitmap.image->width, BW->bitmap.image->height));

    BW->bitmap.buffer_hot = BW->bitmap.hot;
    BW->bitmap.buffer_mark = BW->bitmap.mark;
}

void BWUnmark(w)
     Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    BW->bitmap.buffer_mark = BW->bitmap.mark;

    if (QuerySet(BW->bitmap.mark.from_x, BW->bitmap.mark.from_y)) {
	XFillRectangle(XtDisplay(BW), XtWindow(BW), BW->bitmap.highlighting_gc,
		       InWindowX(BW, BW->bitmap.mark.from_x), 
		       InWindowY(BW, BW->bitmap.mark.from_y), 
		       InWindowX(BW, BW->bitmap.mark.to_x + 1) - 
		       InWindowX(BW, BW->bitmap.mark.from_x),
		       InWindowY(BW, BW->bitmap.mark.to_y + 1) -
		       InWindowY(BW, BW->bitmap.mark.from_y));
    
	BW->bitmap.mark.from_x = BW->bitmap.mark.from_y = NotSet;
	BW->bitmap.mark.to_x = BW->bitmap.mark.to_y = NotSet;
    }
}


void BWMark(w, from_x, from_y, to_x, to_y)
    Widget w;
    Position from_x, from_y,
	     to_x, to_y;
{
    BitmapWidget BW = (BitmapWidget) w;

    BWUnmark(w);
    
    if (QuerySet(from_x, from_y)) {
	if ((from_x == to_x) && (from_y == to_y)) {
	    /*
	      BW->bitmap.mark.from_x = 0;
	      BW->bitmap.mark.from_y = 0;
	      BW->bitmap.mark.to_x = BW->bitmap.image->width - 1;
	      BW->bitmap.mark.to_y = BW->bitmap.image->height - 1;
	      */
	    return;
	}
	else {
	    QuerySwap(from_x, to_x);
	    QuerySwap(from_y, to_y);
	    from_x = max(0, from_x);
	    from_y = max(0, from_y);
	    to_x = min(BW->bitmap.image->width - 1, to_x);
	    to_y = min(BW->bitmap.image->height - 1, to_y);
	    
	    BW->bitmap.mark.from_x = from_x;
	    BW->bitmap.mark.from_y = from_y;
	    BW->bitmap.mark.to_x = to_x;
	    BW->bitmap.mark.to_y = to_y;
	}
	
	XFillRectangle(XtDisplay(BW), XtWindow(BW), BW->bitmap.highlighting_gc,
		       InWindowX(BW, BW->bitmap.mark.from_x),
		       InWindowY(BW, BW->bitmap.mark.from_y), 
		       InWindowX(BW, BW->bitmap.mark.to_x + 1) -
		       InWindowX(BW, BW->bitmap.mark.from_x),
		       InWindowY(BW, BW->bitmap.mark.to_y +1) - 
		       InWindowY(BW, BW->bitmap.mark.from_y));
    }
}

void BWSelect(w, from_x, from_y, to_x, to_y, time)
    Widget w;
    Position from_x, from_y,
	     to_x, to_y;
    Time time;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    BWMark(w, from_x, from_y, to_x, to_y);

    BWGrabSelection(w, time);
}

void BWUndo(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;
    Position x, y;
    char *tmp_data;
    XPoint tmp_hot;
    BWArea tmp_mark;
    
    tmp_data = BW->bitmap.image->data;
    BW->bitmap.image->data = BW->bitmap.buffer->data;
    BW->bitmap.buffer->data = tmp_data;

    tmp_hot = BW->bitmap.hot;
    tmp_mark = BW->bitmap.mark;
    
    for (x = 0; x < BW->bitmap.image->width; x++)
	for (y = 0; y < BW->bitmap.image->height; y++)
	 if (GetBit(BW->bitmap.image, x, y) != GetBit(BW->bitmap.buffer, x, y))
	     DrawSquare(BW, x, y);

    BWSetHotSpot(w, BW->bitmap.buffer_hot.x, BW->bitmap.buffer_hot.y);
/*    BWMark(w, BW->bitmap.buffer_mark.from_x, BW->bitmap.buffer_mark.from_y,
	   BW->bitmap.buffer_mark.to_x, BW->bitmap.buffer_mark.to_y);
*/
    BW->bitmap.buffer_hot = tmp_hot;
    BW->bitmap.buffer_mark= tmp_mark;

}

void BWHighlightAxes(w)
    Widget w;
{
    BitmapWidget BW = (BitmapWidget) w;

    XDrawLine(XtDisplay(BW), XtWindow(BW),
	      BW->bitmap.axes_gc,
	      InWindowX(BW, 0), 
	      InWindowY(BW, 0),
	      InWindowX(BW, BW->bitmap.width),
	      InWindowY(BW, BW->bitmap.height));
    XDrawLine(XtDisplay(BW), XtWindow(BW),
	      BW->bitmap.axes_gc,
	      InWindowX(BW, BW->bitmap.width),
	      InWindowY(BW, 0), 
	      InWindowX(BW, 0),
	      InWindowY(BW, BW->bitmap.height));
    XDrawLine(XtDisplay(BW), XtWindow(BW),
	      BW->bitmap.axes_gc,
	      InWindowX(BW, 0),
	      InWindowY(BW, BW->bitmap.height / 2.0),
	      InWindowX(BW, BW->bitmap.width),
	      InWindowY(BW, BW->bitmap.height / 2.0));
    XDrawLine(XtDisplay(BW), XtWindow(BW),
	      BW->bitmap.axes_gc,
	      InWindowX(BW, BW->bitmap.width / 2.0),
	      InWindowY(BW, 0),
	      InWindowX(BW, BW->bitmap.width / 2.0),
	      InWindowY(BW, BW->bitmap.height));
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


/*****************************************************************************
 *                                  Handlers                                 *
 *****************************************************************************/

#define QueryInSquare(BW, x, y, square_x, square_y)\
    ((InBitmapX(BW, x) == (square_x)) &&\
     (InBitmapY(BW, y) == (square_y)))


void DragOnePointHandler(w, status, event)
     Widget       w;
     BWStatus    *status;
     XEvent      *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "D1PH ");

    switch (event->type) {
    
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->at_x, status->at_y)) {
	    BWStoreToBuffer(w);
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    status->success = (Boolean) status->draw;
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, status->value);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->at_x, status->at_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    status->success = (Boolean) status->draw;
      
	    BWTerminateRequest(w, TRUE);
	}
	break;

    case MotionNotify:
	if (QuerySet(status->at_x, status->at_y)) {
	    if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
			       status->at_x, status->at_y)) {
		status->at_x = InBitmapX(BW, event->xmotion.x);
		status->at_y = InBitmapY(BW, event->xmotion.y);
		if (status->draw)
		    (*status->draw)(w,
				    status->at_x, status->at_y, status->value);
	    }
	}
	break;

    }
}

void DragOnePointEngage(w, status, draw, state)
    Widget      w;
    BWStatus   *status;
    void      (*draw)();
    int        *state;
{
    
    status->at_x = NotSet;
    status->at_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;
    
    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		      FALSE, DragOnePointHandler, status);
}

void DragOnePointTerminate(w, status, client_data)
    Widget     w;
    BWStatus  *status;
    caddr_t    client_data;
{
    
    if (status->success) {
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }
    
    XtRemoveEventHandler(w,
		 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		 FALSE, DragOnePointHandler, status);
    
}

void OnePointHandler(w, status, event)
    Widget       w;
    BWStatus    *status;
    XEvent      *event;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    if (DEBUG)
	fprintf(stderr, "1PH ");

    switch (event->type) {
	
    case Expose:
	if (QuerySet(status->at_x, status->at_y)) {
	    BWClip(w, 
		   InBitmapX(BW, event->xexpose.x), 
		   InBitmapY(BW, event->xexpose.y),
		   InBitmapX(BW, event->xexpose.x + event->xexpose.width),
		   InBitmapY(BW, event->xexpose.y + event->xexpose.height));
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
	    
	    BWUnclip(w);
	}
	break;
	
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->at_x, status->at_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->at_x, status->at_y)) {
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
	    
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;
	    status->at_x = InBitmapX(BW, event->xbutton.x);
	    status->at_y = InBitmapY(BW, event->xbutton.y);
	    status->success = True;
	    
	    BWTerminateRequest(w, TRUE);
	}
	break;
	
    case MotionNotify:
	if (QuerySet(status->at_x, status->at_y)) {
	    if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
			       status->at_x, status->at_y)) {
		if (status->draw)
		    (*status->draw)(w,
				    status->at_x, status->at_y, Highlight);
		status->at_x = InBitmapX(BW, event->xmotion.x);
		status->at_y = InBitmapY(BW, event->xmotion.y);
		if (status->draw)
		    (*status->draw)(w,
				    status->at_x, status->at_y, Highlight);
	    }
	}      
	break;
    }
}

void OnePointEngage(w, status, draw, state)
    Widget      w;
    BWStatus   *status;
    void      (*draw)();
    int        *state;
{
    status->at_x = NotSet;
    status->at_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;

    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | 
		      ExposureMask | PointerMotionMask,
		      FALSE, OnePointHandler, status);
}

void OnePointImmediateEngage(w, status, draw, state)
    Widget      w;
    BWStatus   *status;
    void      (*draw)();
    int        *state;
{
    status->at_x = 0;
    status->at_y = 0;
    status->draw = draw;
    status->success = False;
    status->state = *state;
    
    if (status->draw)
	(*status->draw)(w,
			status->at_x, status->at_y, Highlight);
    
    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | 
		      ExposureMask | PointerMotionMask,
		      FALSE, OnePointHandler, status);
}

void OnePointTerminate(w, status, draw)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
{
    
    if (status->success && draw) {
	BWStoreToBuffer(w);
	(*draw)(w,
		status->at_x, status->at_y,
		status->value);
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }    
    else
	if (QuerySet(status->at_x, status->at_y))
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, OnePointHandler, status);
}

void OnePointTerminateTransparent(w, status, draw)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
{
    
    if (status->success && draw)
	(*draw)(w,
		status->at_x, status->at_y,
		status->value);
    else
	if (QuerySet(status->at_x, status->at_y))
	    if (status->draw)
		(*status->draw)(w,
				status->at_x, status->at_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, OnePointHandler, status);
    
}


void TwoPointsHandler(w, status, event)
    Widget      w;
    BWStatus   *status;
    XEvent     *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "2PH ");
    
    switch (event->type) {
	
    case Expose:
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y)) {
	    BWClip(w, 
		   InBitmapX(BW, event->xexpose.x), 
		   InBitmapY(BW, event->xexpose.y),
		   InBitmapX(BW, event->xexpose.x + event->xexpose.width),
		   InBitmapY(BW, event->xexpose.y + event->xexpose.height));
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
	    BWUnclip(w);
	}
	break;
	
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->from_x, status->from_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;
	    status->from_x = InBitmapX(BW, event->xbutton.x);
	    status->from_y = InBitmapY(BW, event->xbutton.y);
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->from_x, status->from_y)) {
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;	    
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    status->success = True;
	    
	    BWTerminateRequest(w, TRUE);
	}
	break;
	
    case MotionNotify:
	if (QuerySet(status->from_x, status->from_y)) {
	    if (QuerySet(status->to_x, status->to_y)) {
		if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
				   status->to_x, status->to_y)) {
		    if (status->draw)
			(*status->draw)(w,
					status->from_x, status->from_y, 
					status->to_x, status->to_y, Highlight);
		    status->to_x = InBitmapX(BW, event->xmotion.x);
		    status->to_y = InBitmapY(BW, event->xmotion.y);
		    if (status->draw)
			(*status->draw)(w,
					status->from_x, status->from_y, 
					status->to_x, status->to_y, Highlight);
		}
	    }
	    else {
		status->to_x = InBitmapX(BW, event->xmotion.x);
		status->to_y = InBitmapY(BW, event->xmotion.y);
		if (status->draw)
		    (*status->draw)(w,
				    status->from_x, status->from_y, 
				    status->to_x, status->to_y, Highlight);
	    }
	}
	break;
    }
}

void TwoPointsEngage(w, status, draw, state)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
    int       *state;
{
    
    status->from_x = NotSet;
    status->from_y = NotSet;
    status->to_x = NotSet;
    status->to_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;

    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | 
		      ExposureMask | PointerMotionMask,
		      FALSE, TwoPointsHandler, status);
}

void TwoPointsTerminate(w, status, draw)
    Widget    w;
    BWStatus *status;
    void    (*draw)();
{
    
    if (status->success && draw) {
	BWStoreToBuffer(w);
	(*draw)(w,
		status->from_x, status->from_y,
		status->to_x, status->to_y,
		status->value);
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }
    else
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y))
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, TwoPointsHandler, status);
}

void TwoPointsTerminateTransparent(w, status, draw)
    Widget    w;
    BWStatus *status;
    void    (*draw)();
{
    
    if (status->success && draw)
	(*draw)(w,
		status->from_x, status->from_y,
		status->to_x, status->to_y,
		status->value);
    else
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y))
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, TwoPointsHandler, status);
}

void TwoPointsTerminateTimed(w, status, draw)
    Widget    w;
    BWStatus *status;
    void    (*draw)();
{
    
    if (status->success && draw)
	(*draw)(w,
		status->from_x, status->from_y,
		status->to_x, status->to_y,
		status->time);
    else
	if (QuerySet(status->from_x, status->from_y) && 
	    QuerySet(status->to_x, status->to_y))
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, Highlight);
    
    XtRemoveEventHandler(w,
			 ButtonPressMask | ButtonReleaseMask | 
			 ExposureMask | PointerMotionMask,
			 FALSE, TwoPointsHandler, status);
}

void Interface(w, status, action)
    Widget     w;
    caddr_t    status;
    void     (*action)();
{
 	(*action)(w);
}

void Paste(w, at_x, at_y, value)
    Widget    w;
    Position  at_x, at_y;
    int       value;
{
    BitmapWidget    BW = (BitmapWidget) w;
    BWStatus       *my_status;
    BWRequest       request;

    my_status = (BWStatus *) 
	BW->bitmap.request_stack[BW->bitmap.current].status;

    my_status->draw = NULL;

   request = (BWRequest)
   BW->bitmap.request_stack[BW->bitmap.current].request->terminate_client_data;
	
    BWTerminateRequest(w, FALSE);
    
    if ((at_x == max(BW->bitmap.mark.from_x, min(at_x, BW->bitmap.mark.to_x)))
	&&
      (at_y == max(BW->bitmap.mark.from_y, min(at_y, BW->bitmap.mark.to_y)))) {
	
	BWStatus *status;
	
	if (DEBUG)
	    fprintf(stderr, "Prepaste request: %s\n", request);
	
	BWEngageRequest(w, request, False, &(my_status->state), sizeof(int));
	
	status = (BWStatus *) 
	    BW->bitmap.request_stack[BW->bitmap.current].status;
	
	status->at_x = at_x;
	status->at_y = at_y;
	status->value = value;
	(*status->draw) (w, at_x, at_y, Highlight);	
    }
    else {

	BWStatus *status;
	
      BWEngageRequest(w, MarkRequest, False, &(my_status->state), sizeof(int));
	
	status = (BWStatus *) 
	    BW->bitmap.request_stack[BW->bitmap.current].status;
	
	status->from_x = status->to_x = at_x;
	status->from_y = status->to_y = at_y;
	status->value = value;
	(*status->draw) (w, at_x, at_y, at_x, at_y, Highlight);
    }
}


void DragTwoPointsHandler(w, status, event)
    Widget      w;
    BWStatus   *status;
    XEvent     *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "D2PH ");

    switch (event->type) {
	
    case ButtonPress:
	if (event->xbutton.state != status->state) return;
	if (!QuerySet(status->from_x, status->from_y)) {
	    BWStoreToBuffer(w);
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;
	    status->from_x = InBitmapX(BW, event->xbutton.x);
	    status->from_y = InBitmapY(BW, event->xbutton.y);
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    status->success = (Boolean) status->draw;
	    if (status->draw)
		(*status->draw)(w,
				status->from_x, status->from_y, 
				status->to_x, status->to_y, status->value);
	}
	break;
	
    case ButtonRelease:
	if (QuerySet(status->from_x, status->from_y)) {
	    status->value = Value(BW, event->xbutton.button);
	    status->time = event->xbutton.time;	    
	    status->from_x = status->to_x;
	    status->from_y = status->to_y;
	    status->to_x = InBitmapX(BW, event->xbutton.x);
	    status->to_y = InBitmapY(BW, event->xbutton.y);
	    status->success = True;
	    
	    BWTerminateRequest(w, TRUE);
	}
	break;
	
    case MotionNotify:
	if (QuerySet(status->from_x, status->from_y)) {
	    if (QuerySet(status->to_x, status->to_y)) {
		if (!QueryInSquare(BW, event->xmotion.x, event->xmotion.y,
				   status->to_x, status->to_y)) {
		    status->from_x = status->to_x;
		    status->from_y = status->to_y;
		    status->to_x = InBitmapX(BW, event->xmotion.x);
		    status->to_y = InBitmapY(BW, event->xmotion.y);
		    if (status->draw)
			(*status->draw)(w,
					status->from_x, status->from_y, 
					status->to_x, status->to_y, status->value);
		}
	    }
	}
	break;
    }
}

void DragTwoPointsEngage(w, status, draw, state)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
    int       *state;
{
    
    status->from_x = NotSet;
    status->from_y = NotSet;
    status->to_x = NotSet;
    status->to_y = NotSet;
    status->draw = draw;
    status->success = False;
    status->state = *state;

    XtAddEventHandler(w,
		      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
		      FALSE, DragTwoPointsHandler, status);
}

void DragTwoPointsTerminate(w, status, draw)
    Widget     w;
    BWStatus  *status;
    void     (*draw)();
{
    
    if (status->success && draw) {
	if ((status->from_x != status->to_x) 
	    || 
	    (status->from_y != status->to_y))
	    (*draw)(w,
		    status->from_x, status->from_y,
		    status->to_x, status->to_y,
		    status->value);
	BWChangeNotify(w, NULL, NULL);
	BWSetChanged(w);
    }
    
    XtRemoveEventHandler(w,
		         ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			 FALSE, DragTwoPointsHandler, status);
}


void BWTPaste(w, event)
    Widget  w;
    XEvent *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (!BW->bitmap.selection.own)
	BWRequestSelection(w, event->xbutton.time, TRUE);
    else 
	BWStore(w);

    if (!BWQueryStored(w))
	return;
    
    BWEngageRequest(w, RestoreRequest, False, 
		    &(event->xbutton.state), sizeof(int));
    
    OnePointHandler(w,
	       (BWStatus*) BW->bitmap.request_stack[BW->bitmap.current].status,
	       event);
}

void BWTMark(w, event)
    Widget  w;
    XEvent *event;
{
    BitmapWidget BW = (BitmapWidget) w;

    BWEngageRequest(w, MarkRequest, False,
		    &(event->xbutton.state), sizeof(int));
    TwoPointsHandler(w,
            (BWStatus*) BW->bitmap.request_stack[BW->bitmap.current].status,
	     event);
}

void BWTUnmark(w)
    Widget w;
{
    BWUnmark(w);
}


/*****************************************************************************/

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
    unsigned int width, height;
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

/*****************************************************************************
 *                               Cut and Paste                               *
 *****************************************************************************/


Boolean ConvertSelection(w, selection, target, type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    caddr_t *value;
    unsigned long *length;
    int *format;
{
    BitmapWidget BW = (BitmapWidget) w;
    Pixmap *pixmap;
    static str;
    char *data;
    XImage *image;
    Dimension width, height;
 
    switch (*target) {
/*
    case XA_TARGETS:
	*type = XA_ATOM;
	*value = (caddr_t) bitmapClassRec.bitmap_class.targets;
	*length = bitmapClassRec.bitmap_class.num_targets;
	*format = 32;
	return True;
*/
    case XA_BITMAP:
    case XA_PIXMAP:
	if (!BWQueryMarked(w)) return False;
	width = BW->bitmap.mark.to_x - BW->bitmap.mark.from_x + 1;
	height = BW->bitmap.mark.to_y - BW->bitmap.mark.from_y + 1;
	data = CreateCleanData(Length(width, height));
	image = CreateBitmapImage(BW, data, width, height);
	CopyImageData(BW->bitmap.image, image, 
		      BW->bitmap.mark.from_x, BW->bitmap.mark.from_y,
		      BW->bitmap.mark.to_x, BW->bitmap.mark.to_y, 0, 0);
	pixmap = (Pixmap *) XtMalloc(sizeof(Pixmap));
	*pixmap = GetPixmap(BW, image);
	DestroyBitmapImage(&image);
	*type = XA_PIXMAP;
	*value = (caddr_t) pixmap;
	*length = 1;
	*format = 32;
	return True;

    case XA_STRING:
	*type = XA_STRING;
	*value = "Hello world!\n";
	*length = XtStrlen(*value);
	*format = 8;
	return True;

    default:
	return False;
    }
}

void LoseSelection(w, selection)
    Widget w;
    Atom *selection;
{
    BitmapWidget BW = (BitmapWidget) w;

    if (DEBUG)
	fprintf(stderr, "Lost Selection\n");
    BW->bitmap.selection.own = False;
    BWUnmark(w);
}

void SelectionDone(w, selection, target)
    Widget w;
    Atom *selection, *target;
{
    BitmapWidget BW = (BitmapWidget) w;
/*  
    if (*target != XA_TARGETS)
	XtFree(BW->bitmap.value);
*/
}

void BWGrabSelection(w, time)
    Widget w;
    Time time;
{
    BitmapWidget BW = (BitmapWidget) w;

    BW->bitmap.selection.own = XtOwnSelection(w, XA_PRIMARY, time,
					      ConvertSelection, 
					      LoseSelection, 
					      SelectionDone);
	if (DEBUG && BW->bitmap.selection.own)
	    fprintf(stderr, "Own the selection\n");
}

void SelectionCallback(w, client_data, selection, type, value, length, format)
    Widget w;
    caddr_t client_data;
    Atom *selection, *type;
    caddr_t value;
    unsigned long *length;
    int *format;
{
    BitmapWidget BW = (BitmapWidget) w;
    Pixmap *pixmap;

   switch (*type) {
	
    case XA_BITMAP:
    case XA_PIXMAP:
	DestroyBitmapImage(&BW->bitmap.storage);
	pixmap = (Pixmap *) value;
	BW->bitmap.storage = GetImage(BW, *pixmap);
	XFreePixmap(XtDisplay(w), *pixmap);
	break;
	
    case XA_STRING:
	if (DEBUG)
	    fprintf(stderr, "Received:%s\n", value);
	break;

    default:
	XtWarning(" selection request failed.  BitmapWidget");
	break;
    }

    BW->bitmap.selection.limbo = FALSE;
}

void BWRequestSelection(w, time, wait)
    Widget w;
    Time time;
    Boolean wait;
{
    BitmapWidget BW = (BitmapWidget) w;
    
    XtGetSelectionValue(w, XA_PRIMARY, XA_PIXMAP,
			SelectionCallback, NULL, time);

    BW->bitmap.selection.limbo = TRUE;

    if (wait)
	while (BW->bitmap.selection.limbo) {
	    XEvent event;
	    XtNextEvent(&event);
	    XtDispatchEvent(&event);
	}
}

/*****************************************************************************/

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
	    XtFree(new->bitmap.basename);
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
    String filename, *basename;
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

int BWWriteFile(w, filename, basename)
    Widget w;
    String filename, basename;
{
    BitmapWidget BW = (BitmapWidget) w;
    unsigned char *data;
    XImage *image;
    XPoint hot;
    int status;
    
    if (BW->bitmap.zooming) {
	bcopy(BW->bitmap.zoom.image->data, data,
	      Length(BW->bitmap.zoom.image->width, 
		     BW->bitmap.zoom.image->height));
	image = CreateBitmapImage(BW, data,
				  BW->bitmap.zoom.image->width,
				  BW->bitmap.zoom.image->height);
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
    char *data;
    int i;

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
		 BW->bitmap.width - 1, BW->bitmap.height - 1);

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
		 BW->bitmap.width - 1, BW->bitmap.height - 1);
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


