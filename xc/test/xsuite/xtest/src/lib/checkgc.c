/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include	"pixval.h"

int
checkgccomponent(disp, gc, valuemask, values)
Display *disp;
GC gc;
unsigned int valuemask;  
XGCValues *values;
{
	XGCValues rvalues;

	if(XGetGCValues(disp, gc, valuemask, &rvalues) != True)
	  return(0);

	switch(valuemask) {
	      case GCBackground :
		return(rvalues.background == values->background);
	      case GCLineWidth :
		return(rvalues.line_width == values->line_width);
	      case GCLineStyle :
		return(rvalues.line_style == values->line_style);
	      case GCCapStyle :
		return(rvalues.cap_style == values->cap_style);
	      case GCJoinStyle :
		return(rvalues.join_style == values->join_style);
	      case  GCFillStyle :
		return(rvalues.fill_style == values->fill_style);
	      case GCFillRule :
		return(rvalues.fill_rule == values->fill_rule);
	      case GCTile :
		return(rvalues.tile == values->tile);
	      case GCStipple :
		return(rvalues.stipple == values->stipple);
	      case GCTileStipXOrigin :
		return(rvalues.ts_x_origin == values->ts_x_origin);
	      case  GCTileStipYOrigin :
		return(rvalues.ts_y_origin == values->ts_y_origin);
	      case GCFont :
		return(rvalues.font == values->font);
	      case GCSubwindowMode :
		return(rvalues.subwindow_mode == values->subwindow_mode);
	      case GCGraphicsExposures :
		return(rvalues.graphics_exposures == values->graphics_exposures);
	      case GCClipXOrigin :
		return(rvalues.clip_x_origin == values->clip_y_origin);
	      case GCClipYOrigin :
		return(rvalues.clip_y_origin == values->clip_y_origin);
	      case GCDashOffset :
		return(rvalues.dash_offset == values->dash_offset);
	      case GCArcMode :
		return(rvalues.arc_mode == values->arc_mode);
              default :
		  return(0);
	}
}

