/*
* $XConsortium: TextP.h,v 1.34 89/07/16 16:24:10 kit Exp $
*/


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XawTextP_h
#define _XawTextP_h

#include <X11/Xaw/Text.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/TextSrcP.h>

/****************************************************************
 *
 * Text widget private
 *
 ****************************************************************/
#define MAXCUT	30000	/* Maximum number of characters that can be cut. */

extern XtActionsRec textActionsTable[];
extern Cardinal textActionsTableCount;

#define LF	0x0a
#define CR	0x0d
#define TAB	0x09
#define BS	0x08
#define SP	0x20
#define DEL	0x7f
#define BSLASH	'\\'

/* constants that subclasses may want to know */
#define DEFAULT_TEXT_HEIGHT ((Dimension)~0)
#define  yMargin 2

/* displayable text management data structures */

typedef struct {
    XawTextPosition position;
    Position x, y, endX;
    } XawTextLineTableEntry, *XawTextLineTableEntryPtr;

/* Line Tables are n+1 long - last position displayed is in last lt entry */
typedef struct {
    XawTextPosition	 top;	/* Top of the displayed text.		*/
    int			 lines;	/* How many lines in this table.	*/
    XawTextLineTableEntry *info;/* A dynamic array, one entry per line  */
} XawTextLineTable, *XawTextLineTablePtr;

#define IsPositionVisible(ctx, pos) \
		(pos >= ctx->text.lt.info[0].position && \
		 pos < ctx->text.lt.info[ctx->text.lt.lines].position)

/*
 * Search & Replace data structure.
 */

struct SearchAndReplace {
  Boolean selection_changed;	/* flag so that the selection cannot be
				   changed out from underneath query-replace.*/
  Widget search_popup;		/* The poppup widget that allows searches.*/
  Widget label1;		/* The label widgets for the search window. */
  Widget label2;
  Widget left_toggle;		/* The left search toggle radioGroup. */
  Widget right_toggle;		/* The right search toggle radioGroup. */
  Widget rep_label;		/* The Replace label string. */
  Widget rep_text;		/* The Replace text field. */
  Widget search_text;		/* The Search text field. */
  Widget rep_one;		/* The Replace one button. */
  Widget rep_all;		/* The Replace all button. */
};
    
/* Private Text Definitions */

typedef int (*ActionProc)();

/* New fields for the Text widget class record */

typedef struct {int empty;} TextClassPart;

/* Full class record declaration */
typedef struct _TextClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    TextClassPart	text_class;
} TextClassRec;

extern TextClassRec textClassRec;

/* New fields for the Text widget record */
typedef struct _TextPart {
    /* resources */
    XawTextSource	source;
    XawTextSink		sink;
    XawTextPosition	insertPos;
    XawTextSelection	s;
    XawTextSelectType	*sarray;	   /* Array to cycle for selections. */
    Dimension		client_leftmargin;   /* client-visible resource */
    int			options;	     /* wordbreak, scroll, etc. */
    int			dialog_horiz_offset; /* position for popup dialog */
    int			dialog_vert_offset;  /* position for popup dialog */
    Boolean		display_caret;	     /* insertion pt visible iff T */
    /* private state */
    XawTextLineTable	lt;
    XawTextScanDirection extendDir;
    XawTextSelection	origSel;    /* the selection being modified */
    Dimension	    leftmargin;	    /* Width of left margin. */
    Time	    lasttime;	    /* timestamp of last processed action */
    Time	    time;	    /* time of last key or button action */ 
    Position	    ev_x, ev_y;	    /* x, y coords for key or button action */
    Widget	    sbar;	    /* The vertical scroll bar (none = 0).  */
    Widget	    outer;	    /* Parent of scrollbar & text (if any) */
    struct SearchAndReplace * search;/* Search and replace structure. */
    Widget          file_insert;    /* The file insert popup widget. */
    XawTextPosition  *updateFrom;   /* Array of start positions for update. */
    XawTextPosition  *updateTo;	    /* Array of end positions for update. */
    int		    numranges;	    /* How many update ranges there are. */
    int		    maxranges;	    /* How many ranges we have space for */
    Boolean	    showposition;   /* True if we need to show the position. */
    XawTextPosition  lastPos;	    /* Last position of source. */
    GC              gc;
    Boolean         hasfocus;       /* TRUE if we currently have input focus.*/
    Boolean	    update_disabled; /* TRUE if display updating turned off */
    XawTextPosition  old_insert;      /* Last insertPos for batched updates */
    short           mult;	      /* Multiplier. */
} TextPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _TextRec {
    CorePart	core;
    SimplePart	simple;
    TextPart	text;
} TextRec;

#ifdef XAW_BC
/*************************************************************
 * For Compatibility only.                                   */

#define XtTextLineTable                 XawTextLineTable
#define XtTextLineTablePtr              XawTextLineTablePtr
#define XtTextLineTableEntry            XawTextLineTableEntry
#define XtTextLineTableEntryPtr         XawTextLineTableEntryPtr

/*************************************************************/
#endif /* XAW_BC */

#endif /* _XawTextP_h */
