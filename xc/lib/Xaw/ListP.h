/*
 * $XConsortium: ListP.h,v 1.12 89/12/11 15:09:04 kit Exp $
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
 * Author:  Chris D. Peterson, MIT X Consortium
 */


/* 
 * ListP.h - Private definitions for List widget
 * 
 * This is a List widget.  It allows the user to select an item in a list and
 * notifies the application through a callback function.
 *
 *	Created: 	8/13/88
 *	By:		Chris D. Peterson
 *                      MIT - Project Athena
 */

#ifndef _XawListP_h
#define _XawListP_h

/***********************************************************************
 *
 * List Widget Private Data
 *
 ***********************************************************************/

#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/List.h>

#define NO_HIGHLIGHT            XAW_LIST_NONE
#define OUT_OF_RANGE            -1
#define OKAY                     0

/* New fields for the List widget class record */

typedef struct {int foo;} ListClassPart;

/* Full class record declaration */
typedef struct _ListClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    ListClassPart	list_class;
} ListClassRec;

extern ListClassRec listClassRec;

/* New fields for the List widget record */
typedef struct {
    /* resources */
    Pixel	foreground;
    Dimension	internal_width, /* if not 3d, user sets directly. */
        	internal_height,
                column_space,	/* half of *_space is add on top/bot/left of*/
                row_space;	/* each item's text bounding box. half added to longest for right */
    int         default_cols;
    Boolean     force_cols,
                paste,
                vertical_cols;
    int         longest;	/* in pixels */
    int         nitems;		/* number of items in the list. */
    XFontStruct	*font;
    XFontSet 	fontset;	/* Sheeran, Omron KK, 93/03/05 */
    String *    list;		/* for i18n, always in multibyte format */
    XtCallbackList callback;

    /* private state */
    int         is_highlighted,	/* set to the item currently highlighted. */
                highlight,	/* set to the item that should be highlighted.*/
                col_width,	/* width of each column. */
                row_height,	/* height of each row. */
                nrows,		/* number of rows in the list. */
                ncols;		/* number of columns in the list. */
    GC		normgc,		/* a couple of GC's. */
                revgc,
                graygc;		/* used when inactive. */

    int         freedoms;       /* flags for resizing height and width */

} ListPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _ListRec {
    CorePart	core;
    SimplePart	simple;
    ListPart	list;
} ListRec;

#endif /* _XawListP_h */
