/*
 * $XConsortium: BitmapP.h,v 1.2 90/03/30 15:26:21 dmatic Exp $
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
 * Author:  Davor Matic, MIT X Consortium
 */



#ifndef _BitmapP_h
#define _BitmapP_h

#include "Bitmap.h"
#include <X11/Xaw/SimpleP.h>

typedef struct {
    Atom           *targets;
    Cardinal        num_targets;
    BWRequestRec   *requests;
    Cardinal        num_requests;
    BWRequestRec   *request[100];
  
} BitmapClassPart;

/* Full class record declaration */
typedef struct _BitmapClassRec {
  CoreClassPart          core_class;
  SimpleClassPart        simple_class;
  BitmapClassPart        bitmap_class;
} BitmapClassRec;

extern BitmapClassRec bitmapClassRec;

/**********/
typedef struct _BWRequestRec {
  char       *name;
  int         status_size;
  void      (*engage)();
  caddr_t     engage_client_data;
  void      (*terminate)();
  caddr_t     terminate_client_data;
  void      (*remove)();
  caddr_t     remove_client_data;
};

typedef struct {
  Position from_x, from_y,
           to_x, to_y;
} BWArea;

typedef struct {
    BWRequestRec *request;
    caddr_t       status;
    Boolean       trap;
    caddr_t       call_data;
} BWRequestStack;

typedef struct {
    XImage   *image, *buffer;
    XPoint    hot;
    Position  at_x, at_y;
    Boolean   fold;
    Boolean   changed;
} BWZoom;

typedef struct {
    Boolean   own;
    Boolean   limbo;
} BWSelection;

/* New fields for the Bitmap widget record */
typedef struct {
  /* resources */
  Pixel            foreground_pixel;
  Pixel            highlight_pixel;
  Pixel            framing_pixel;
  Pixmap           stipple;
  Boolean          stippled;
  Boolean          proportional;
  Boolean          grid;
  Dimension        grid_tolerance;
  Pixmap           dashes;
  Boolean          dashed;
  Boolean          axes;
  Boolean          resize;
  Dimension        distance, squareW, squareH, width, height;
  XPoint           hot;
  int              button_action[5];
  String           filename, basename;
  /* private state */
  Position         horizOffset, vertOffset;
  void           (*notify)();
  BWRequestStack  *request_stack;
  Cardinal         cardinal, current;
  /*Boolean          trapping;*/
  XImage          *image, *buffer, *storage;
  XPoint           buffer_hot;
  BWArea           mark, buffer_mark;
  GC               drawing_gc;
  GC               highlighting_gc;
  GC               framing_gc;
  GC               axes_gc;
  Boolean          changed;
  Boolean          fold;
  Boolean          zooming;
  BWZoom           zoom;
  caddr_t         *value;
  char             status[80];
  BWSelection      selection;
} BitmapPart;

/* Full instance record declaration */
typedef struct _BitmapRec {
  CorePart      core;
  SimplePart    simple;
  BitmapPart    bitmap;
} BitmapRec;

#endif /* _BitmapP_h */




