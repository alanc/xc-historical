/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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
 * Author:  Jim Fulton, MIT X Consortium
 *          rewritten from an Xlib version by Mark Lillibridge
 */


#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/SimpleP.h>
#include "fontgridP.h"

#define CellWidth(fs) ((fs) ? (fs)->max_bounds.width : 0)
#define CellHeight(fs) ((fs) ? ((fs)->ascent + (fs)->descent) : 0)

#define Bell(w) XBell(XtDisplay(w), 50)



static char defaultTranslations[] = 
  "<ButtonPress>:  notify()";

static void Notify();

static XtActionsRec actions_list[] = {
    { "notify",		Notify },
};

static XtResource resources[] = {
#define offset(field) XtOffset(FontGridPtr, fontgrid.field)
    { XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(text_font), XtRString, (caddr_t) NULL },
    { XtNfirstChar, XtCFirstChar, XtRInt, sizeof(int),
	offset(start_char), XtRString, (caddr_t) "0" },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(foreground_pixel), XtRString, (caddr_t) "XtDefaultForeground" },
    { XtNboxCharacters, XtCBoxCharacters, XtRBoolean, sizeof(Boolean),
	offset(box_chars), XtRString, (caddr_t) "FALSE" },
    { XtNboxColor, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(box_pixel), XtRString, (caddr_t) "XtDefaultForeground" },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t),
	offset(callbacks), XtRCallback, (caddr_t) NULL },
    { XtNinternalPad, XtCInternalPad, XtRInt, sizeof(int),
	offset(internal_pad), XtRString, (caddr_t) "2" },
#undef offset
};


static void Realize(), Redisplay();
static Boolean SetValues();

FontGridClassRec fontgridClassRec = {
  { /* core fields */
    /* superclass               */      (WidgetClass) &simpleClassRec,
    /* class_name               */      "FontGrid",
    /* widget_size              */      sizeof(FontGridRec),
    /* class_initialize         */      NULL,
    /* class_part_initialize    */      NULL,
    /* class_inited             */      FALSE,
    /* initialize               */      NULL,
    /* initialize_hook          */      NULL,
    /* realize                  */      Realize,
    /* actions                  */      actions_list,
    /* num_actions              */      XtNumber(actions_list),
    /* resources                */      resources,
    /* num_resources            */      XtNumber(resources),
    /* xrm_class                */      NULLQUARK,
    /* compress_motion          */      TRUE,
    /* compress_exposure        */      TRUE,
    /* compress_enterleave      */      TRUE,
    /* visible_interest         */      FALSE,
    /* destroy                  */      NULL,
    /* resize                   */      NULL,
    /* expose                   */      Redisplay,
    /* set_values               */      SetValues,
    /* set_values_hook          */      NULL,
    /* set_values_almost        */      XtInheritSetValuesAlmost,
    /* get_values_hook          */      NULL,
    /* accept_focus             */      NULL,
    /* version                  */      XtVersion,
    /* callback_private         */      NULL,
    /* tm_table                 */      defaultTranslations,
    /* query_geometry           */      XtInheritQueryGeometry,
    /* display_accelerator      */      XtInheritDisplayAccelerator,
    /* extension                */      NULL
  }
};

WidgetClass fontgridWidgetClass = (WidgetClass) &fontgridClassRec;


static GC get_gc (fgw, fore)
    FontGridPtr fgw;
    Pixel fore;
{
    XtGCMask mask;
    XGCValues gcv;

    mask = (GCForeground | GCBackground | GCFunction);
    gcv.foreground = fore;
    gcv.background = fgw->core.background_pixel;
    gcv.function = GXcopy;
    return (XtGetGC ((Widget) fgw, mask, &gcv));
}


static void Realize (gw, valueMask, attributes)
    Widget gw;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    FontGridPtr fgw = (FontGridPtr) gw;
    FontGridPart *p = &fgw->fontgrid;

    p->text_gc = get_gc (fgw, p->foreground_pixel);
    p->box_gc = get_gc (fgw, p->box_pixel);
    p->cell_width = CellWidth (p->text_font);
    p->cell_height = CellHeight (p->text_font);

    (*(XtSuperclass(gw)->core_class.realize)) (gw, valueMask, attributes);
    return;
}


static void paint_grid (fgw, col, row, ncols, nrows)
    FontGridPtr fgw;			/* widget in which to draw */
    int col, row;			/* where to start */
    int ncols, nrows;			/* number of cells */
{
    FontGridPart *p = &fgw->fontgrid;
    int i, j;
    Display *dpy = XtDisplay(fgw);
    Window wind = XtWindow(fgw);
    int cw = p->cell_width;
    int ch = p->cell_height;
    int trows = fgw->core.width / cw;
    int tcols = fgw->core.height / ch;
    GC gc = p->text_gc;
    int x1, y1, x2, y2, x, y;

    /*
     * paint the grid lines for the indicated rows 
     */
    x1 = col * cw;
    y1 = row * ch;
    x2 = x1 + ncols * cw;
    y2 = y1 + nrows * ch;
    for (i = 0, x = x1; i <= ncols; i++, x += cw) {
	XDrawLine (dpy, wind, gc, x, y1, x, y2);
    }
    for (i = 0, y = y1; i <= nrows; i++, y += ch) {
	XDrawLine (dpy, wind, gc, x1, y, x2, y);
    }

	
    /*
     * Draw a character in every box; treat all fonts as if they were 16bit
     * fonts.  Store the high eight bits in byte1 and the low eight bits in 
     * byte2.
     */
    for (j = 0, y = row * ch; j < nrows; j++, y += ch) {
	for (i = 0, x = col * cw; i < ncols; i++, x += cw) {
	    unsigned n = (p->start_char + (i + col) +
			  (row + j) * tcols);
	    XChar2b thechar;
	    thechar.byte1 = (n >> 8);	/* high eight bits */
	    thechar.byte2 = (n & 255);	/* low eight bits */
	    XDrawImageString16 (dpy, wind, gc,
				x + p->internal_pad,
				y + p->internal_pad + p->text_font->ascent,
				&thechar, 1);
	    if (p->box_chars) {
		/*
		 * draw a rectangle around the ink boundary
		 */
	    }
	}
    }

    return;
}


/* ARGSUSED */
static void Redisplay (gw, event, region)
    Widget gw;
    XEvent *event;
    Region region;
{
    FontGridPtr fgw = (FontGridPtr) gw;
    XRectangle rect;			/* bounding rect for region */
    int cx, cy, cw, ch;			/* cell location and size */
    int ncols, nrows;			/* number of cells to repaint */

    if (!fgw->fontgrid.text_font) {
	Bell (gw);
	return;
    }

    XClipBox (region, &rect);
    cw = fgw->fontgrid.cell_width;
    ch = fgw->fontgrid.cell_height;
    if ((cx = rect.x / cw) < 0) cx = 0;
    if ((cy = rect.y / ch) < 0) cy = 0;
    ncols = (int) (((unsigned) rect.width) / (unsigned) cw);
    nrows = (int) (((unsigned) rect.height) / (unsigned) ch);

    paint_grid (fgw, cx, cy, ncols, nrows);
}


static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    FontGridPtr curfg = (FontGridPtr) current;
    FontGridPtr reqfg = (FontGridPtr) request;
    FontGridPtr newfg = (FontGridPtr) new;
    Boolean redisplay = FALSE;

    if (curfg->fontgrid.text_font != newfg->fontgrid.text_font ||
	curfg->fontgrid.internal_pad != newfg->fontgrid.internal_pad) {
	newfg->fontgrid.cell_width = CellWidth (newfg->fontgrid.text_font);
	newfg->fontgrid.cell_height = CellHeight (newfg->fontgrid.text_font);
	redisplay = TRUE;
    }

    if (curfg->fontgrid.foreground_pixel != newfg->fontgrid.foreground_pixel) {
	XtReleaseGC (new, curfg->fontgrid.text_gc);
	newfg->fontgrid.text_gc = get_gc (newfg,
					  newfg->fontgrid.foreground_pixel);
	redisplay = TRUE;
    }

    if (curfg->fontgrid.box_pixel != newfg->fontgrid.box_pixel) {
	XtReleaseGC (new, curfg->fontgrid.text_gc);
	newfg->fontgrid.box_gc = get_gc (newfg, newfg->fontgrid.box_pixel);
	redisplay = TRUE;
    }

    return redisplay;
}


/* ARGSUSED */
static void Notify (gw, event, params, nparams)
    Widget gw;
    XEvent *event;
    String *params;
    Cardinal *nparams;
{
    FontGridPtr fgw = (FontGridPtr) gw;
    int x, y;				/* where the event happened */
    FontGridCharRec rec;		/* callback data */

    switch (event->type) {
      case KeyPress:
      case KeyRelease:
	x = event->xkey.x;
	y = event->xkey.y;
	break;
      case ButtonPress:
      case ButtonRelease:
	x = event->xbutton.x;
	y = event->xbutton.y;
	break;
      case MotionNotify:
	x = event->xmotion.x;
	y = event->xmotion.y;
	break;
      default:
	Bell (gw);
	return;
    }

    {
	int cw = fgw->fontgrid.cell_width, ch = fgw->fontgrid.cell_height;
        int tcols = fgw->core.height / ch;
	register unsigned n = (fgw->fontgrid.start_char +
			       ((y / ch) * tcols) + (x / cw));

	rec.thefont = fgw->fontgrid.text_font;
	rec.thechar.byte1 = (n >> 8);
	rec.thechar.byte2 = (n & 255);
    }

    XtCallCallbacks (gw, XtNcallback, (caddr_t) &rec);
}
