/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xmu/DisplayQue.h>

static int _DQCloseDisplay();

#define DoCallback(q,e) (void) (*((q)->closefunc)) ((q), (e))

/*
 * XmuDQCreate - create a display queue
 */
DisplayQueue *XmuDQCreate (closefunc, data)
    int (*closefunc)();
    caddr_t data;
{
    DisplayQueue *q = (DisplayQueue *) malloc (sizeof (DisplayQueue));
    if (q) {
	q->nentries = 0;
	q->head = q->tail = NULL;
	q->closefunc = closefunc;
	q->data = data;
    }
    return q;
}


/*
 * XmuDQDestroy - free all storage associated with this display queue, 
 * optionally invoking the close callbacks.
 */

Bool XmuDQDestroy (q, docallbacks)
    DisplayQueue *q;
    Bool docallbacks;
{
    DisplayQueueEntry *e = q->head;

    while (e) {
	DisplayQueueEntry *nexte = e->next;
	if (docallbacks && q->closefunc) DoCallback (q, e);
	free ((char *) e);
	e = nexte;
    }
    free ((char *) q);
    return True;
}


/*
 * XmuDQLookupDisplay - finds the indicated display on the given queue
 */
DisplayQueueEntry *XmuDQLookupDisplay (q, dpy)
    DisplayQueue *q;
    Display *dpy;
{
    DisplayQueueEntry *e;

    for (e = q->head; e; e = e->next) {
	if (e->display == dpy) return e;
    }
    return NULL;
}


/*
 * XmuDQAddDisplay - add the specified display to the queue; set data as a
 * convenience.  Does not ensure that dpy hasn't already been added.
 */
DisplayQueueEntry *XmuDQAddDisplay (q, dpy, data)
    DisplayQueue *q;
    Display *dpy;
    caddr_t data;
{
    DisplayQueueEntry **ep = (q->tail ? &(q->tail->next) : &(q->tail));
    DisplayQueueEntry *e;

    if (!(e = (DisplayQueueEntry *) malloc (sizeof (DisplayQueueEntry)))) {
	return NULL;
    }
    if (!(e->closehook = XmuAddCloseDisplayHook (dpy, _DQCloseDisplay,
						 (caddr_t) q))) {
	free ((char *) e);
	return NULL;
    }

    e->display = dpy;
    e->next = NULL;
    e->data = data;

    if (q->tail) {
	q->tail->next = e;
	e->prev = q->tail;
    } else {
	q->head = e;
	e->prev = NULL;
    }
    q->tail = e;
    q->nentries++;
    return e;
}


/*
 * XmuDQRemoveDisplay - remove the specified display from the queue
 */
Bool XmuDQRemoveDisplay (q, dpy)
    DisplayQueue *q;
    Display *dpy;
{
    DisplayQueueEntry *e;

    for (e = q->head; e; e = e->next) {
	if (e->display == dpy) {
	    if (q->head == e)
	      q->head = e->next;	/* if at head, then bump head */
	    else
	      e->prev->next = e->next;	/* else splice out */
	    if (q->tail == e)
	      q->tail = e->prev;	/* if at tail, then bump tail */
	    else
	      e->next->prev = e->prev;	/* else splice out */
	    (void) XmuRemoveCloseDisplayHook (dpy, e->closehook,
					      _DQCloseDisplay, (caddr_t) q);
	    free ((char *) e);
	    q->nentries--;
	    return True;
	}
    }
    return False;
}


/*****************************************************************************
 *			       private functions                             *
 *****************************************************************************/

/*
 * _DQCloseDisplay - upcalled from CloseHook to notify this queue; remove the
 * display when finished
 */
static int _DQCloseDisplay (dpy, arg)
    Display *dpy;
    caddr_t arg;
{
    DisplayQueue *q = (DisplayQueue *) arg;
    DisplayQueueEntry *e;

    for (e = q->head; e; e = e->next) {
	if (e->display == dpy) {
	    if (q->closefunc) DoCallback (q, e);
	    (void) XmuDQRemoveDisplay (q, dpy);
	    return 1;
	}
    }

    return 0;
}
