/***********************************************************

 "$XConsortium: Label.c,v 1.64 89/03/16 12:58:17 swick Exp $";

Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

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

/*
 * SimpleMenuP.h - Private Header file for SimpleMenu widget.
 *
 * Date:    April 3, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium / Project Athena.
 *          kit@athena.mit.edu
 */

#ifndef _SimpleMenuP_h
#define _SimpleMenuP_h

#include <X11/SimpleMenu.h>
#include <X11/ShellP.h>

#define MAGIC_DIMENSION 54321
#define NO_ENTRY -1

typedef struct {
    caddr_t extension;		/* For future needs. */
} SimpleMenuClassPart;

typedef struct _SimpleMenuClassRec {
  CoreClassPart	          core_class;
  CompositeClassPart      composite_class;
  ShellClassPart          shell_class;
  OverrideShellClassPart  override_shell_class;
  SimpleMenuClassPart	  simpleMenu_class;
} SimpleMenuClassRec;

extern SimpleMenuClassRec simpleMenuClassRec;

typedef enum {
  XawErefreshAll, XawErefreshEntry, XawErefreshLabel
} XawRefreshTypes;

typedef struct _MenuEntry {
  struct _MenuEntry * next;	 /* next menu entry. */
  String            name;        /* name of this menu item. */
  String            label;       /* label displayed for this menu item. */
  Boolean           sensitive;	 /* is this item sensitive. */
  Pixmap            left_bitmap; /* Bitmap to display on the left and right. */
  Pixmap            right_bitmap;
  XawMenuEntryType  type;	 /* Type of item to display. */
  XtCallbackList callbacks;	 /* The callback list. */

  /* private state */
  
  unsigned int         left_bitmap_width; /* sizes of bitmaps */
  unsigned int         left_bitmap_height;
  unsigned int         right_bitmap_width;
  unsigned int         right_bitmap_height;
} MenuEntry;

typedef struct _SimpleMenuPart {

  /* resources */

  Cursor       cursor;		/* The menu's cursor. */
  String       label;		/* label the menu will display. */
  Pixel	       foreground;	/* foreground color for label. */
  XFontStruct  *font;		/* font to display all text. */
  Dimension    left_margin;	/* margins on each side of menu entries. */
  Dimension    right_margin;
  Dimension    top_margin;	
  Dimension    bottom_margin;
  Dimension    vertical_space;	/* Space between entries ( in pixels ). */
  Dimension    row_height;	/* height of each row - 
				   excluding vertical space */
  Dimension    column_width;    /* width of column, excluding margins. */
  Boolean      auto_resize;	/* row and column height are automatically
				   calculated of this is TRUE. */

  /* private state */

  GC           norm_gc;		/* GC to use when drawing normally. */
  GC           rev_gc;		/* GC to use when drawing in reverse vid. */
  GC           norm_grey_gc;	/* GC for insensitive entries. */
  GC           invert_gc;	/* GC to use when inverting items. */

  MenuEntry * entries;		/* a pointer to list of entries. */
  Cardinal num_entries;		/* number of current menu
				   entries + menu label. */
  Boolean recursive_set_values;	/* contain a possible infinite loop. */
  int          entry_set;	/* The entry that is currently set or
				   highlighted. */
} SimpleMenuPart;

typedef struct _SimpleMenuRec {
  CorePart		core;
  CompositePart 	composite;
  ShellPart 	        shell;
  OverrideShellPart     override;
  SimpleMenuPart	simple_menu;
} SimpleMenuRec;

#endif  _SimpleMenuP_h
