/*
 * $XConsortium: Exp $
 */

/* 
 * DviP.h - Private definitions for Dvi widget
 */

#ifndef _XtDviP_h
#define _XtDviP_h

#include "Dvi.h"
#include "DviChar.h"

/***********************************************************************
 *
 * Dvi Widget Private Data
 *
 ***********************************************************************/

/************************************
 *
 *  Class structure
 *
 ***********************************/

/*
 * New fields for the Dvi widget class record
 */

typedef struct _DviClass {
	int		makes_compiler_happy;  /* not used */
} DviClassPart;

/*
 * Full class record declaration
 */

typedef struct _DviClassRec {
	CoreClassPart	core_class;
	DviClassPart	command_class;
} DviClassRec;

extern DviClassRec dviClassRec;

/***************************************
 *
 *  Instance (widget) structure 
 *
 **************************************/

/*
 * a list of fonts we've used for this widget
 */

typedef struct _dviFontSizeList {
	struct _dviFontSizeList	*next;
	int			size;
	char			*x_name;
	XFontStruct		*font;
	int			doesnt_exist;
} DviFontSizeList;

typedef struct _dviFontList {
	struct _dviFontList	*next;
	char			*dvi_name;
	char			*x_name;
	int			dvi_number;
	DviFontSizeList		*sizes;
	DviCharNameMap		*char_map;
} DviFontList;

typedef struct _dviFontMap {
	struct _dviFontMap	*next;
	char			*dvi_name;
	char			*x_name;
} DviFontMap;

#define DVI_TEXT_CACHE_SIZE	256
#define DVI_CHAR_CACHE_SIZE	1024

typedef struct _dviCharCache {
	XTextItem	cache[DVI_TEXT_CACHE_SIZE];
	char		char_cache[DVI_CHAR_CACHE_SIZE];
	int		index;
	int		char_index;
	int		font_size;
	int		font_number;
	XFontStruct	*font;
	int		start_x, start_y;
	int		x, y;
} DviCharCache;

typedef struct _dviState {
	struct _dviState	*next;
	int			font_size;
	int			font_number;
	int			line_style;
	int			line_width;
	int			x;
	int			y;
} DviState;

typedef struct _dviFileMap {
	struct _dviFileMap	*next;
	long			position;
	int			page_number;
} DviFileMap;

/*
 * New fields for the Dvi widget record
 */

typedef struct {
	/*
	 * resource specifiable items
	 */
	char		*font_map_string;
	unsigned long	foreground;
	char		*file_name;
	int		requested_page;
	int		last_page;
	/*
 	 * private state
 	 */
	FILE		*file;
	GC		normal_GC;
	DviFileMap	*file_map;
	DviFontList	*fonts;
	DviFontMap	*font_map;
	int		current_page;
	int		font_size;
	int		font_number;
	int		line_width;
	int		line_style;
	int		backing_store;
	XFontStruct	*font;
	XFontStruct	*default_font;
	int		display_enable;
	struct ExposedExtents {
	    int x1, y1, x2, y2;
	}		extents;
	DviState	*state;
	DviCharCache	cache;
} DviPart;

/*
 * Full widget declaration
 */

typedef struct _DviRec {
	CorePart	core;
	DviPart		dvi;
} DviRec;

extern XFontStruct	*QueryFont ();

extern DviCharNameMap	*QueryFontMap ();
#endif _XtDviP_h


