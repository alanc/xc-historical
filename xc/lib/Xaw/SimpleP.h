#include <X/copyright.h>

/* $Header: SimpleP.h,v 1.1 87/12/23 16:43:02 swick Exp $ */
/* Copyright	Massachusetts Institute of Technology	1987 */

#ifndef _SimpleP_h
#define _SimpleP_h

#include <X/Simple.h>

typedef struct {
    Boolean	(*change_sensitive)(/* widget */);
} SimpleClassPart;

typedef struct _SimpleClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
} SimpleClassRec;

extern SimpleClassRec simpleClassRec;

typedef struct {
    /* resources */
    Cursor	cursor;
    Pixmap	insensitive_border;

    /* private state */
} SimplePart;

typedef struct _SimpleRec {
    CorePart	core;
    SimplePart	simple;
} SimpleRec;

#endif  _SimpleP_h
