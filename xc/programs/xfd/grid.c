/*
 * $XConsortium: fontgrid.c,v 1.2 89/06/02 20:08:38 jim Exp $
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

#define CellWidth(fgw) (((fgw)->fontgrid.text_font->max_bounds.width) + \
			((fgw)->fontgrid.internal_pad * 2))
#define CellHeight(fgw) ((fgw)->fontgrid.text_font->ascent + \
			 (fgw)->fontgrid.text_font->descent + \
			 ((fgw)->fontgrid.internal_pad * 2))

#define Bell(w) XBell(XtDisplay(w), 50)


static void Initialize(), Realize(), Redisplay(), Notify();
static void paint_grid();
static Boolean SetValues();


static XtResource resources[] = {
#define offset(field) XtOffset(FontGridWidget, fontgrid.field)
    { XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	offset(text_font), XtRString, (caddr_t) NULL },
    { XtNfirstChar, XtCFirstChar, XtRInt, sizeof(int),
	offset(start_char), XtRString, (caddr_t) "0" },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(foreground_pixel), XtRString, (caddr_t) "XtDefaultForeground" },
    { XtNcenterChars, XtCCenterChars, XtRBoolean, sizeof(Boolean),
	offset(center_chars), XtRString, (caddr_t) "FALSE" },
    { XtNboxChars, XtCBoxChars, XtRBoolean, sizeof(Boolean),
	offset(box_chars), XtRString, (caddr_t) "FALSE" },
    { XtNboxColor, XtCForeground, XtRPixel, sizeof(Pixel),
	offset(box_pixel), XtRString, (caddr_t) "XtDefaultForeground" },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t),
	offset(callbacks), XtRCallback, (caddr_t) NULL },
    { XtNinternalPad, XtCInternalPad, XtRInt, sizeof(int),
	offset(internal_pad), XtRString, (caddr_t) "4" },
    { XtNgridWidth, XtCGridWidth, XtRInt, sizeof(int),
	offset(grid_width), XtRString, (caddr_t) "1" },
#undef offset
};


static char defaultTranslations[] = 
  "<ButtonPress>:  notify()";

static XtActionsRec actions_list[] = {
    { "notify",		Notify },
};

FontGridClassRec fontgridClassRec = {
  { /* core fields */
    /* superclass               */      (WidgetClass) &simpleClassRec,
    /* class_name               */      "FontGrid",
    /* widget_size              */      sizeof(FontGridRec),
    /* class_initialize         */      NULL,
    /* class_part_initialize    */      NULL,
    /* class_inited             */      FALSE,
    /* initialize               */      Initialize,
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
    FontGridWidget fgw;
    Pixel fore;
{
    XtGCMask mask;
    XGCValues gcv;

    mask = (GCForeground | GCBackground | GCFunction | GCFont);
    gcv.foreground = fore;
    gcv.background = fgw->core.background_pixel;
    gcv.function = GXcopy;
    gcv.font = fgw->fontgrid.text_font->fid;
    if (fgw->fontgrid.grid_width > 0) {
	mask |= GCLineWidth;
	gcv.line_width = ((fgw->fontgrid.grid_width < 2) ? 0 : 
			  fgw->fontgrid.grid_width);
    }
    return (XtGetGC ((Widget) fgw, mask, &gcv));
}


static void Initialize (request, new)
    Widget request, new;
{
    FontGridWidget newfg = (FontGridWidget) new;

    newfg->fontgrid.cell_width = CellWidth (newfg);
    newfg->fontgrid.cell_height = CellHeight (newfg);

    if (newfg->core.width == 0)
      newfg->core.width = (newfg->fontgrid.cell_width * 16 +
			   newfg->fontgrid.grid_width);
    if (newfg->core.height == 0)
      newfg->core.height = (newfg->fontgrid.cell_height * 16 +
			    newfg->fontgrid.grid_width);

    return;
}

static void Realize (gw, valueMask, attributes)
    Widget gw;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    FontGridWidget fgw = (FontGridWidget) gw;
    FontGridPart *p = &fgw->fontgrid;

    p->text_gc = get_gc (fgw, p->foreground_pixel);
    p->box_gc = get_gc (fgw, p->box_pixel);

    (*(XtSuperclass(gw)->core_class.realize)) (gw, valueMask, attributes);
    return;
}



/* ARGSUSED */
static void Redisplay (gw, event, region)
    Widget gw;
    XEvent *event;
    Region region;
{
    FontGridWidget fgw = (FontGridWidget) gw;
    XRectangle rect;			/* bounding rect for region */
    int left, right, top, bottom;	/* which cells were damaged */
    int cw, ch;				/* cell size */

    if (!fgw->fontgrid.text_font) {
	Bell (gw);
	return;
    }

    /*
     * compute the left, right, top, and bottom cells that were damaged
     */
    XClipBox (region, &rect);
    cw = fgw->fontgrid.cell_width;
    ch = fgw->fontgrid.cell_height;
    if ((left = (((int) rect.x) / cw)) < 0) left = 0;
    right = (((int) (rect.x + rect.width - 1)) / cw);
    if ((top = (((int) rect.y) / ch)) < 0) top = 0;
    bottom = (((int) (rect.y + rect.height - 1)) / ch);

    paint_grid (fgw, left, top, right - left + 1, bottom - top + 1);
}


static void paint_grid (fgw, col, row, ncols, nrows)
    FontGridWidget fgw;			/* widget in which to draw */
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
    int x1, y1, x2, y2, x, y;
    unsigned maxn = ((p->text_font->max_byte1 << 8) |
		     p->text_font->max_char_or_byte2);
    unsigned n, prevn;
    int startx;

    if (col + ncols >= tcols) {
	ncols = tcols - col;
	if (ncols < 1) return;
    }

    if (row + nrows >= trows) {
	nrows = trows - row;
	if (nrows < 1) return;
    }

    /*
     * paint the grid lines for the indicated rows 
     */
    if (p->grid_width > 0) {
	x1 = col * cw;
	y1 = row * ch;
	x2 = x1 + ncols * cw;
	y2 = y1 + nrows * ch;
	for (i = 0, x = x1; i <= ncols; i++, x += cw) {
	    XDrawLine (dpy, wind, p->box_gc, x, y1, x, y2);
	}
	for (i = 0, y = y1; i <= nrows; i++, y += ch) {
	    XDrawLine (dpy, wind, p->box_gc, x1, y, x2, y);
	}
    }

	
    /*
     * Draw a character in every box; treat all fonts as if they were 16bit
     * fonts.  Store the high eight bits in byte1 and the low eight bits in 
     * byte2.
     */
    prevn = p->start_char + col + row * tcols;
    startx = col * cw + p->internal_pad;
    for (j = 0, y = row * ch + p->internal_pad + p->text_font->ascent;
	 j < nrows; j++, y += ch) {
	n = prevn;
	for (i = 0, x = startx; i < ncols; i++, x += cw) {
	    XChar2b thechar;
	    int xoff = 0, yoff = 0;

	    if (n > maxn) goto done;	/* no break out of nested */

	    thechar.byte1 = (n >> 8);	/* high eight bits */
	    thechar.byte2 = (n & 255);	/* low eight bits */
	    if (p->box_chars || p->center_chars) {
		XCharStruct metrics;
		int direction, fontascent, fontdescent;

		XTextExtents16 (p->text_font, &thechar, 1, &direction,
				&fontascent, &fontdescent, &metrics);

		if (p->box_chars) {
		    XDrawRectangle (dpy, wind, p->box_gc,
				    x, y - p->text_font->ascent, 
				    metrics.width - 1,
				    fontascent + fontdescent - 1);
		}
		if (p->center_chars) {
		    /*
		     * We want to move the origin by enough to center the ink
		     * within the cell.  The left edge will then be at 
		     * (cell_width - (rbearing - lbearing)) / 2; so we subtract
		     * the lbearing to find the origin.  Ditto for vertical.
		     */
		    xoff = (((p->cell_width -
			      (metrics.rbearing - metrics.lbearing)) / 2) -
			    p->internal_pad - metrics.lbearing);
		    yoff = (((p->cell_height - 
			      (metrics.descent + metrics.ascent)) / 2) -
			    p->internal_pad -
			    p->text_font->ascent + metrics.ascent);
		}
	    }
	    XDrawString16 (dpy, wind, p->text_gc, x + xoff, y + yoff,
			   &thechar, 1);
	    n++;
	}
	prevn += tcols;
    }

  done:
    return;
}


static Boolean SetValues (current, request, new)
    Widget current, request, new;
{
    FontGridWidget curfg = (FontGridWidget) current;
    FontGridWidget reqfg = (FontGridWidget) request;
    FontGridWidget newfg = (FontGridWidget) new;
    Boolean redisplay = FALSE;

    if (curfg->fontgrid.text_font != newfg->fontgrid.text_font ||
	curfg->fontgrid.internal_pad != newfg->fontgrid.internal_pad) {
	newfg->fontgrid.cell_width = CellWidth (newfg);
	newfg->fontgrid.cell_height = CellHeight (newfg);
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
    FontGridWidget fgw = (FontGridWidget) gw;
    int x, y;				/* where the event happened */
    FontGridCharRec rec;		/* callback data */

    /*
     * only allow events with (x,y)
     */
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

    /*
     * compute the callback data
     */
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
