/*
 * $XConsortium$
 *
 * CloseDisplayHook package - provide callback on XCloseDisplay
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

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xmu.h>


/*
 * There is a list of display queues, each of which has a list of callbacks.
 */

typedef struct _CallbackHook {
    struct _CallbackHook *next;		/* next link in chain */
    int (*func)();			/* function to call */
    caddr_t arg;			/* argument to pass with function */
} CallbackHook;


typedef struct _DisplayQueue {
    struct _DisplayQueue *next;		/* next link in chain */
    Display *dpy;			/* the display this represents */
    int extension;			/* from XAddExtension */
    struct _CallbackHook *start, *end;	/* linked list of callbacks */
} DisplayQueue;


static DisplayQueue *topq = NULL;
static Bool _Initialize();


/*
 * Find the given display
 */
static DisplayQueue *_FindDisplayQueue (dpy, prevp)
    register Display *dpy;
    DisplayQueue **prevp;
{
    register DisplayQueue *d, *prev;

    for (d = topq, prev = NULL; d; d = d->next) {
	if (d->dpy == dpy) {
	    if (prevp) *prevp = prev;
	    return d;
	}
	prev = d;
    }
    return NULL;
}


/*
 * Add - add a callback for the given display
 */
caddr_t XmuAddCloseDisplayHook (dpy, func, arg)
    Display *dpy;
    int (*func)();			/* function to call on close display */
    caddr_t arg;			/* arg to pass */
{
    DisplayQueue *dq;
    CallbackHook *hook;

    /* allocate ahead of time */
    hook = (CallbackHook *) malloc (sizeof (CallbackHook));
    if (!hook) return ((caddr_t) NULL);

    dq = _FindDisplayQueue (dpy, NULL);
    if (!dq) {
	if ((dq = (DisplayQueue *) malloc (sizeof (DisplayQueue))) == NULL ||
	    !_Initialize(dpy, &dq->extension)) {
	    free ((char *) hook);
	    if (dq) free ((char *) dq);
	    return ((caddr_t) NULL);
	}
	dq->next = NULL;
	dq->dpy = dpy;
	dq->start = dq->end = NULL;
	dq->next = topq;
	topq = dq;
    }

    /* add to end of list of hooks */
    hook->func = func;
    hook->arg = arg;
    hook->next = NULL;
    if (dq->end) {
	dq->end->next = hook;
    } else {
	dq->start = hook;
    }
    dq->end = hook;

    return ((caddr_t) hook);
}


/*
 * Remove - get rid of a callback
 */
Bool XmuRemoveCloseDisplayHook (dpy, hook, func, arg)
    Display *dpy;
    caddr_t hook;			/* value from XmuAddCloseDisplayHook */
    int (*func)();			/* function to call on close display */
    caddr_t arg;			/* arg to pass */
{
    DisplayQueue *dq = _FindDisplayQueue (dpy, NULL);
    register CallbackHook *h, *prev;

    if (!dq) return False;

    /* look for hook or function/argument pair */
    for (h = dq->start, prev = NULL; h; h = h->next) {
	if (hook) {
	    if (h == (CallbackHook *) hook) break;
	} else {
	    if (h->func == func && h->arg == arg) break;
	}
	prev = h;
    }
    if (!h) return False;


    /* remove from list, watch head */
    if (dq->start == h) {
	dq->start = h->next;
    } else {
	prev->next = h->next;
    }
    if (dq->end == h) dq->end = prev;
    free ((char *) h);
    return True;
}


/*
 * Lookup - see whether or not a hook has been installed
 */
Bool XmuLookupCloseDisplayHook (dpy, hook, func, arg)
    Display *dpy;
    caddr_t hook;			/* value from XmuAddCloseDisplayHook */
    int (*func)();			/* function to call on close display */
    caddr_t arg;			/* arg to pass */
{
    DisplayQueue *dq = _FindDisplayQueue (dpy, NULL);
    register CallbackHook *h;

    if (!dq) return False;

    for (h = dq->start; h; h = h->next) {
	if (hook) {
	    if (h == (CallbackHook *) hook) break;
	} else {
	    if (h->func == func && h->arg == arg) break;
	}
    }
    return (h ? True : False);
}


/*
 * internal routines
 */

static int _close_display (dpy, codes)
    Display *dpy;
    XExtCodes *codes;
{
    DisplayQueue *prev;
    DisplayQueue *dq = _FindDisplayQueue (dpy, &prev);
    CallbackHook *h;

    if (!dq) return 0;

    /* walk the list doing the callbacks and freeing callback record */
    for (h = dq->start; h;) {
	register CallbackHook *oldh = h;
	(*(h->func)) (dpy, h->arg);
	free ((char *) h);
	h = oldh->next;
    }

    /* unlink this display from chain */
    if (topq == dq) {
	topq = dq->next;
    } else {
	prev->next = dq->next;
    }
    free ((char *) dq);
    return 1;
}

static Bool _Initialize (dpy, extensionp)
    Display *dpy;
    int *extensionp;
{
    XExtCodes *codes;

    codes = XAddExtension (dpy);
    if (!codes) return False;

    (void) XESetCloseDisplay (dpy, codes->extension, _close_display);

    *extensionp = codes->extension;
    return True;
}
