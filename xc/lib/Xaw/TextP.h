/*
* $Header: TextP.h,v 1.7 88/01/06 08:57:13 swick Locked $
*/

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#ifndef _XtTextPrivate_h
#define _XtTextPrivate_h


/****************************************************************
 *
 * Text widget private
 *
 ****************************************************************/
#define MAXCUT	30000	/* Maximum number of characters that can be cut. */

#define LF	0x0a
#define CR	0x0d
#define TAB	0x09
#define BS	0x08
#define SP	0x20
#define DEL	0x7f
#define BSLASH	'\\'

#define EditDone 0
#define EditError 1
#define PositionError 2

#define DEFAULTVALUE -9999

typedef enum {XtsdLeft, XtsdRight} XtTextScanDirection;
typedef enum {XtstPositions, XtstWhiteSpace, XtstEOL, XtstParagraph, XtstAll}
    XtTextScanType;

typedef struct {
    int  firstPos;
    int  length;
    char *ptr;
    Atom format;
    } XtTextBlock, *XtTextBlockPtr;

/* the data field is really a pointer to source info, see disk and 
   stream sources in TextKinds.c */

typedef struct _XtTextSource {
    int			(*Read)();
    int			(*Replace)();
    XtTextPosition	(*GetLastPos)();
    int			(*SetLastPos)();
    XtTextPosition	(*Scan)();
    void		(*AddWidget)( /* source, widget */ );
    void		(*RemoveWidget)( /* source, widget */ );
    void		(*SetSelection)( /* source, left, right */);
    Boolean		(*GetSelection)( /* source, left, right */);
    XtTextEditType	edit_mode;
    caddr_t		data;	    
    };

typedef struct _XtTextSink {
    XFontStruct	*font;
    int foreground;
    int (*Display)();
    int (*InsertCursor)();
    int (*ClearToBackground)();
    int (*FindPosition)();
    int (*FindDistance)();
    int (*Resolve)();
    int (*MaxLines)();
    int (*MaxHeight)();
    caddr_t data;
    };

/* displayable text management data structures */

typedef struct {
    XtTextPosition position;
    Position x, y, endX;
    } XtTextLineTableEntry, *XtTextLineTableEntryPtr;

/* Line Tables are n+1 long - last position displayed is in last lt entry */
typedef struct {
    XtTextPosition	 top;	/* Top of the displayed text.		*/
    int			 lines;	/* How many lines in this table.	*/
    XtTextLineTableEntry *info;	/* A dynamic array, one entry per line  */
    } XtTextLineTable, *XtTextLineTablePtr;

typedef enum {XtisOn, XtisOff} XtTextInsertState;

typedef enum {XtsmTextSelect, XtsmTextExtend} XtTextSelectionMode;

typedef enum {XtactionStart, XtactionAdjust, XtactionEnd}
    XtTextSelectionAction;

typedef struct {
    XtTextPosition left, right;
    XtTextSelectType type;
} XtTextSelection;

#define IsPositionVisible(ctx, pos) \
		(pos >= ctx->text.lt.info[0].position && \
		 pos <= ctx->text.lt.info[ctx->text.lt.lines].position)

/* Private Text Definitions */

typedef int (*ActionProc)();

typedef XtTextSelectType XtTextSelectionArray[20];

/* New fields for the Text widget class record */

typedef struct {int foo;} TextClassPart;

/* Full class record declaration */
typedef struct _TextClassRec {
    CoreClassPart	core_class;
    TextClassPart	text_class;
} TextClassRec;

extern TextClassRec textClassRec;

/* New fields for the Text widget record */
typedef struct {
    XtTextSource	source;
    XtTextSink		sink;
    XtTextLineTable	lt;
    XtTextPosition	insertPos;
    XtTextSelection	s;
    XtTextScanDirection extendDir;
    XtTextSelection	origSel;    /* the selection being modified */
    XtTextSelectionArray sarray;    /* Array to cycle for selections. */
    Dimension	    leftmargin;	    /* Width of left margin. */
    int		    options;	    /* wordbreak, scroll, etc. */
    Time	    lasttime;	    /* timestamp of last processed action */
    Time	    time;	    /* time of last key or button action */ 
    Position	    ev_x, ev_y;	    /* x, y coords for key or button action */
    Widget	    sbar;	    /* The vertical scroll bar (none = 0).  */
    Widget	    outer;	    /* Parent of scrollbar & text (if any) */
    XtTextPosition  *updateFrom;    /* Array of start positions for update. */
    XtTextPosition  *updateTo;	    /* Array of end positions for update. */
    int		    numranges;	    /* How many update ranges there are. */
    int		    maxranges;	    /* How many ranges we have space for */
    Boolean	    showposition;   /* True if we need to show the position. */
    XtTextPosition  lastPos;	    /* Last position of source. */
    Widget	    dialog;	    /* Window containing dialog, if any. */
    GC              gc;
    Boolean         hasfocus;       /* TRUE if we currently have input focus.*/
} TextPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _TextRec {
    CorePart	core;
    TextPart	text;
} TextRec;


#endif _XtTextPrivate_h
/* DON'T ADD STUFF AFTER THIS #endif */
