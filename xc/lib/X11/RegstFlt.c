/*
 * $XConsortium: XRegstFlt.c,v 1.2 91/02/14 16:27:03 rws Exp $
 */

 /*
  * Copyright 1990, 1991 by OMRON Corporation
  * Copyright 1991 by the Massachusetts Institute of Technology
  *
  * Permission to use, copy, modify, distribute, and sell this software and its
  * documentation for any purpose is hereby granted without fee, provided that
  * the above copyright notice appear in all copies and that both that
  * copyright notice and this permission notice appear in supporting
  * documentation, and that the names of OMRON and MIT not be used in
  * advertising or publicity pertaining to distribution of the software without
  * specific, written prior permission.  OMRON and MIT make no representations
  * about the suitability of this software for any purpose.  It is provided
  * "as is" without express or implied warranty.
  *
  * OMRON AND MIT DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
  * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
  * EVENT SHALL OMRON OR MIT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
  * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
  * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
  * TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  * PERFORMANCE OF THIS SOFTWARE. 
  *
  *	Author:	Seiji Kuwari	OMRON Corporation
  *				kuwa@omron.co.jp
  *				kuwa%omron.co.jp@uunet.uu.net
  */				

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/XIMlib.h>
#include "XIMlibint.h"
#include "XFilter.h"

/*
 * Register a filter with the filter machinery.
 */
void
XRegisterFilter(display, window, event_mask, nonmaskable, filter, client_data)
Display *display;
Window window;
unsigned long event_mask;
Bool nonmaskable;
Bool (*filter)();
XIMValue client_data;
{
    register XFilterEventList	prev, fl;
    XFilterEventRec		*rec;

    for (prev = NULL, fl = filter_list; fl != NULL; prev = fl, fl = fl->next);
    if ((rec = (XFilterEventList)XIMMalloc(sizeof(XFilterEventRec))) == NULL) {
	return;
    }
    if (prev == NULL) {
	filter_list = rec;
    } else {
	prev->next = rec;
    }
    rec->display = display;
    rec->window = window;
    rec->event_mask = event_mask;
    rec->nonmaskable = nonmaskable;
    rec->filter = filter;
    rec->client_data = client_data;
    rec->next = NULL;
    return;
}

void
XUnregisterFilter(display, window, filter, client_data)
Display *display;
Window window;
Bool (*filter)();
XIMValue client_data;
{
    register XFilterEventList	prev, fl;
    XFilterEventRec		*rec;

    for (prev = NULL, fl = filter_list; fl != NULL; prev = fl, fl = fl->next) {
	if (fl->display == display && fl->window == window
	    && fl->filter == filter && fl->client_data == client_data) {
	    if (prev == NULL) {
		filter_list = NULL;
	    } else {
		prev->next = fl->next;
	    }
	    XIMFree((char *)fl);
	    return;
	}
    }
    return;
}
