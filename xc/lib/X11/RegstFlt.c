/*
 * $Header: XRegstFlt.c,v 1.6 91/04/02 17:37:01 rws Exp $
 * $Id: XRegstFlt.c,v 1.3 91/02/01 08:09:30 proj Exp Locker: proj $
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

#include "Xlibint.h"
#include "Xi18nint.h"

static void
_XFreeIMFilters(display)
    Display *display;
{
    register XFilterEventList fl;

    while (fl = display->im_filters) {
        display->im_filters = fl->next;
        Xfree((char *)fl);
    }
}

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
    XPointer client_data;
{
    XFilterEventRec		*rec;

    rec = (XFilterEventList)Xmalloc(sizeof(XFilterEventRec));
    if (!rec)
	return;
    rec->window = window;
    rec->event_mask = event_mask;
    rec->nonmaskable = nonmaskable;
    rec->filter = filter;
    rec->client_data = client_data;
    LockDisplay(display);
    rec->next = display->im_filters;
    display->im_filters = rec;
    display->free_funcs->im_filters = _XFreeIMFilters;
    UnlockDisplay(display);
}

void
XUnregisterFilter(display, window, filter, client_data)
    Display *display;
    Window window;
    Bool (*filter)();
    XPointer client_data;
{
    register XFilterEventList	*prev, fl;

    for (prev = &display->im_filters; fl = *prev; prev = &fl->next) {
	if (fl->window == window &&
	    fl->filter == filter && fl->client_data == client_data) {
	    *prev = fl->next;
	    Xfree((char *)fl);
	    break;
	}
    }
}
