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
    Boolean		box_chars;	/* put box around logical width */
    Pixel		box_pixel;	/* for box_chars */
    XtCallbackList	callbacks;	/* for notifying caller */
    int			internal_pad;	/* extra padding inside grid */
    int			start_char;	/* first character of grid */
    /* private data */
    GC			text_gc;	/* printing text */
    GC			box_gc;		/* for box_chars */
    int			cell_width, cell_height;  /* size of cell */
} FontGridPart;

typedef struct _FontGridRec {
    CorePart		core;
    SimplePart		simple;
    FontGridPart	fontgrid;
} FontGridRec, *FontGridPtr;

#endif /* _FontGridP_h_ */
