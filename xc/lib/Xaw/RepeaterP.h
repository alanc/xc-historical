/*
 * $XConsortium: RepeaterP.h,v 1.13 90/02/28 13:35:36 jim Exp $
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
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifndef _XawRepeaterP_h
#define _XawRepeaterP_h

#include <X11/Xaw/CommandP.h>
#include <X11/Xaw/Repeater.h>

typedef struct {			/* new fields in widget class */
    int dummy;
} RepeaterClassPart;

typedef struct _RepeaterClassRec {	/* Repeater widget class */
    CoreClassPart core_class;
    SimpleClassPart simple_class;
    LabelClassPart label_class;
    CommandClassPart command_class;
    RepeaterClassPart repeater_class;
} RepeaterClassRec;

typedef struct {			/* new fields in widget */
    /* resources... */
    int initial_delay;			/* initialDelay/Delay */
    int repeat_delay;			/* repeatDelay/Delay */
    Boolean flash;			/* flash/Boolean */
    /* private data... */
    XtIntervalId timer;			/* timer that will fire */
} RepeaterPart;

typedef struct _RepeaterRec {
    CorePart core;
    SimplePart simple;
    LabelPart label;
    CommandPart command;
    RepeaterPart repeater;
} RepeaterRec;

#define REP_DEF_INITIAL_DELAY 200	/* milliseconds */
#define REP_DEF_REPEAT_DELAY 50		/* milliseconds */

/*
 * external declarations
 */
extern RepeaterClassRec repeaterClassRec;

#endif /* _XawRepeaterP_h */
