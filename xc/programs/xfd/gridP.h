#ifndef _FontGridP_h_
#define _FontGridP_h_

#include "fontgrid.h"

typedef struct _FontGridClassPart { int dummy; } FontGridClassPart;

typedef struct _FontGridClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    FontGridClassPart	grid_class;
} FontGridClassRec;
extern FontGridClassRec fontgridClassRec;

typedef struct _FontGridPart {
    XFontStruct *	text_font;		/* font to display */
    Pixel		foreground_pixel;	/* color of text */
    Boolean		center_chars;	/* center characters in grid */
    Boolean		box_chars;	/* put box around logical width */
    Pixel		box_pixel;	/* for box_chars */
    XtCallbackList	callbacks;	/* for notifying caller */
    int			internal_pad;	/* extra padding inside grid */
    long		start_char;	/* first character of grid */
    int			grid_width;	/* width of grid lines */
    /* private data */
    GC			text_gc;	/* printing text */
    GC			box_gc;		/* for box_chars */
    int			cell_width, cell_height;  /* size of cell */
    int			cell_cols, cell_rows;  /* number of cells */
} FontGridPart;

typedef struct _FontGridRec {
    CorePart		core;
    SimplePart		simple;
    FontGridPart	fontgrid;
} FontGridRec;

#define CellWidth(fgw) (((fgw)->fontgrid.text_font->max_bounds.width) + \
			((fgw)->fontgrid.internal_pad * 2))
#define CellHeight(fgw) ((fgw)->fontgrid.text_font->ascent + \
			 (fgw)->fontgrid.text_font->descent + \
			 ((fgw)->fontgrid.internal_pad * 2))

#define CellColumns(fgw) (((fgw)->core.width + (fgw)->fontgrid.grid_width) \
			  / (fgw)->fontgrid.cell_width)
#define CellRows(fgw) (((fgw)->core.height + (fgw)->fontgrid.grid_width) \
		       / (fgw)->fontgrid.cell_height)

#endif /* _FontGridP_h_ */
