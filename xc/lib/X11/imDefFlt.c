/* $XConsortium$ */
/******************************************************************

           Copyright 1992, 1993 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of FUJITSU LIMITED
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.
FUJITSU LIMITED makes no representations about the suitability of
this software for any purpose. 
It is provided "as is" without express or implied warranty.

FUJITSU LIMITED DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL FUJITSU LIMITED BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

  Author: Takashi Fujiwara     FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include <X11/Xatom.h>
#define NEED_EVENTS
#include "Xlibint.h"
#include "Xutil.h"
#include "Xlcint.h"
#include "Ximint.h"

Private long
_XimTriggerCheck(im, ev, len, keylist)
    Xim			 im;
    XKeyEvent		*ev;
    INT32		 len;
    CARD32		*keylist;
{
    register long	 i;
    KeySym	 	 keysym;
    char	 	 buf[BUFSIZE];
    int			 modifier;
    int			 modifier_mask;
    CARD32		 min_len = sizeof(CARD32)   /* sizeof keysym */
				 + sizeof(CARD32)   /* sizeof modifier */
				 + sizeof(CARD32);  /* sizeof modifier mask */

    XLookupString(ev, buf, BUFSIZE, &keysym, NULL);
    if (!keysym)
	return -1;

    for (i = 0; len > min_len; i++, len -= min_len) {
	modifier      = keylist[i + 1];
	modifier_mask = keylist[i + 2];
	if (((KeySym)keylist[i] == keysym)
	 && ((ev->state & modifier_mask) == modifier))
	    return i;
    }
    return -1;
}

Private long
_XimTriggerOnCheck(im, ev)
    Xim		 im;
    XKeyEvent	*ev;
{
    return _XimTriggerCheck(im, ev, (INT32)im->private.proto.im_onkeylist[0],
					&im->private.proto.im_onkeylist[1]);
}

Private long
_XimTriggerOffCheck(im, ev)
    Xim		 im;
    XKeyEvent	*ev;
{
    return _XimTriggerCheck(im, ev, (INT32)im->private.proto.im_offkeylist[0],
					&im->private.proto.im_offkeylist[1]);
}

Private Bool
_XimOnKeysCheck(ic, ev)
    Xic		 ic;
    XKeyEvent	*ev;
{
    Xim		 im = (Xim)ic->core.im;
    long	 idx;

    if (IS_DYNAMIC_EVENT_FLOW(ic->core.im) &&
	    im->private.proto.im_onkeylist &&
	    im->private.proto.im_onkeylist[0]) {
	if ((idx = _XimTriggerOnCheck(im, ev)) >= 0) {
	    (void)_XimTriggerNotify(im, ic, 0, (CARD32)idx); /* Trigger on */
	    return True;
	}
    }
    return False;
}

Private Bool
_XimOffKeysCheck(ic, ev)
    Xic		 ic;
    XKeyEvent	*ev;
{
    Xim		 im = (Xim)ic->core.im;
    long	 idx;

    if (IS_DYNAMIC_EVENT_FLOW(ic->core.im) &&
	    im->private.proto.im_offkeylist &&
	    im->private.proto.im_offkeylist[0]) {
	if ((idx = _XimTriggerOffCheck(im, ev)) >= 0) {
	    _XimTriggerNotify(im, ic, 1, (CARD32)idx); /* Trigger off */
	    return True;
	}
    }
    return False;
}

Public Bool
_XimRegPendingProc(im, ic, arg, proc)
    Xim		   im;
    Xic		   ic;
    XIMArg	  *arg;
    void	   (*proc)();
{
    XimPendingRec *rec;

    if (!(rec = (XimPendingRec *)Xmalloc(sizeof(XimPendingRec))))
        return False;

    rec->func	   = proc;
    rec->ic	   = (XIC)ic;
    rec->arg	   = arg;
    rec->next	   = im->private.proto.pending;
    im->private.proto.pending = rec;
    return True;
}

Public void
_XimFreePendingProc(im)
    Xim		 	    im;
{
    register XimPendingRec *rec, *next;

    for (rec = im->private.proto.pending; rec;) {
	next = rec->next;
	Xfree(rec);
	rec = next;
    }
    im->private.proto.pending = NULL;
    return;
}

Private Bool
_XimCallPendingProc(im)
    Xim			    im;
{
    register XimPendingRec *rec = im->private.proto.pending;

    for (; rec; rec = rec->next) {
	if (rec->arg) {
	    (*rec->func)(rec->ic, rec->arg);	/* SetICValues */
	    _XimFreeRemakeArg(rec->arg);
	} else
	    (*rec->func)(rec->ic);		/* SetFocus or  UnsetFocus */
    }
    _XimFreePendingProc(im);
    return True;
}

Public void
_XimPendingFilter(ic)
    Xic	 	 ic;
{
    Xim          im = (Xim)ic->core.im;

    if (IS_NEED_SYNC_REPLY(ic)) {
	(void)_XimProcSyncReply(im, ic);
	UNMARK_NEED_SYNC_REPLY(ic);
    }
    if (IS_NEED_PENDING_CALL(im)) {
	(void)_XimCallPendingProc(im);
	UNMARK_NEED_PENDING_CALL(im);
    }
    return;
}

Private Bool
_XimProtoKeypressFilter(ic, ev)
    Xic		 ic;
    XKeyEvent	*ev;
{
    Xim		im = (Xim)ic->core.im;
    EVENTMASK	target_mask = XIM_FORWARD_EVENT_MASKS; /* XXX */

    if (IS_FABLICATED(ic)) {
	_XimPendingFilter(ic); /* XXX */
	UNMARK_FABLICATED(ic);
	return NOTFILTERD;
    }
    if ((IS_NEGLECT_EVENT(ic, KeyPressMask))
     && (KeyPressMask & target_mask))
	return FILTERD;
    if (IS_IC_CONNECTED(ic)) {
	if (!IS_FORWARD_EVENT(ic, KeyPressMask)) {
	    if (_XimOnKeysCheck(ic, ev))
		return FILTERD;
	    return NOTFILTERD;
	}
	if (_XimOffKeysCheck(ic, ev))
	    return FILTERD;
	if (IS_SYNCHRONOUS_EVENT(ic, KeyPressMask))
	    return _XimForwardEvent(ic, (XEvent *)ev, True);
	else
	    return _XimForwardEvent(ic, (XEvent *)ev, False);

    } else if (IS_RECONNECTABLE(im)) {
	/* 
	 * Not yet
	 */
	return FILTERD;

    }
    return NOTFILTERD;
}

Private Bool
_XimFilterKeypress (d, w, ev, client_data)
    Display		*d;
    Window		 w;
    XEvent		*ev;
    XPointer		 client_data;
{
    return _XimProtoKeypressFilter((Xic)((void *)client_data),
							(XKeyEvent *)ev );
}

Private Bool
_XimProtoKeyreleaseFilter( ic, ev )
    Xic		 ic;
    XKeyEvent	*ev;
{
    Xim		im = (Xim)ic->core.im;
    EVENTMASK	target_mask = XIM_FORWARD_EVENT_MASKS; /* XXX */

    if ((IS_NEGLECT_EVENT(ic, KeyReleaseMask))
     && (KeyReleaseMask & target_mask))
	return FILTERD;
    if (IS_IC_CONNECTED(ic)) {
	if (!IS_FORWARD_EVENT(ic, KeyReleaseMask)) {
	    if (_XimOnKeysCheck(ic, ev))
		return FILTERD;
	    return NOTFILTERD;
	}
	if (_XimOffKeysCheck(ic, ev))
	    return FILTERD;
	_XimPendingFilter(ic); /* XXX */
	if (IS_SYNCHRONOUS_EVENT(ic, KeyReleaseMask))
	    return _XimForwardEvent(ic, (XEvent *)ev, True);
	else
	    return _XimForwardEvent(ic, (XEvent *)ev, False);
    } else if (IS_RECONNECTABLE(im)) {
	/* 
	 * Not yet
	 */
	return FILTERD;

    }
    return NOTFILTERD;
}

Private Bool
_XimFilterKeyrelease (d, w, ev, client_data)
    Display		*d;
    Window		 w;
    XEvent		*ev;
    XPointer		 client_data;
{
    return _XimProtoKeyreleaseFilter((Xic)((void *)client_data),
							(XKeyEvent *)ev);
}

Public void 
_XimRegisterKeyFilter(ic)
    Xic		 ic ;
{
    if (ic->core.focus_window) {
	if (!(ic->private.proto.register_filter_event & KEYPRESS_MASK)) {
	    _XRegisterFilterByType (ic->core.im->core.display,
				    ic->core.focus_window,
				    KeyPress, KeyPress,
				    _XimFilterKeypress,
				    (XPointer)ic);
	    ic->private.proto.register_filter_event |= KEYPRESS_MASK;
	}
	if (!(ic->private.proto.register_filter_event & KEYRELEASE_MASK)) {
	    _XRegisterFilterByType (ic->core.im->core.display,
				    ic->core.focus_window,
				    KeyRelease, KeyRelease,
				    _XimFilterKeyrelease,
				    (XPointer)ic);
	    ic->private.proto.register_filter_event |= KEYRELEASE_MASK;
	}
    }
    return;
}

Public void 
_XimUnregisterKeyFilter(ic)
    Xic		 ic ;
{
    if (ic->core.focus_window) {
	if (ic->private.proto.register_filter_event & KEYPRESS_MASK) {
	    _XUnregisterFilter (ic->core.im->core.display,
				ic->core.focus_window,
				_XimFilterKeypress,
				(XPointer)ic);
	    ic->private.proto.register_filter_event &= ~KEYPRESS_MASK;
	}
	if (ic->private.proto.register_filter_event & KEYRELEASE_MASK) {
	    _XUnregisterFilter (ic->core.im->core.display,
				ic->core.focus_window,
				_XimFilterKeyrelease,
				(XPointer)ic);
	    ic->private.proto.register_filter_event &= ~KEYRELEASE_MASK;
	}
    }
    return;
}

Private Bool
_XimFilterServerDestroy (d, w, ev, client_data)
    Display		*d;
    Window		 w;
    XEvent		*ev;
    XPointer		 client_data;
{
    _XimServerDestroy();
    return False;
}

Public void 
_XimRegisterServerFilter(im)
    Xim		 im ;
{
    if (im->private.proto.im_window) {
	if (!(im->private.proto.register_filter_event & DESTROYNOTIFY_MASK)) {
	    _XRegisterFilterByType(im->core.display,
		    im->private.proto.im_window,
		    DestroyNotify, DestroyNotify,
		    _XimFilterServerDestroy,
		    (XPointer)im);
	    im->private.proto.register_filter_event |= DESTROYNOTIFY_MASK;
	}
    }
    return;
}

Public void 
_XimUnregisterServerFilter(im)
    Xim		 im ;
{
    if (im->private.proto.im_window) {
	if (im->private.proto.register_filter_event & DESTROYNOTIFY_MASK) {
	    _XUnregisterFilter(im->core.display,
		    im->private.proto.im_window,
		    _XimFilterServerDestroy,
		    (XPointer)im);
	    im->private.proto.register_filter_event &= ~DESTROYNOTIFY_MASK;
	}
    }
    return;
}
