/* $XConsortium: imDefLkup.c,v 1.1 93/09/17 13:25:59 rws Exp $ */
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
#define  NEED_EVENTS
#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"

Public Xic
#if NeedFunctionPrototypes
_XimICOfXICID(
    Xim		  im,
    XICID	  icid)
#else
_XimICOfXICID(im, icid)
    Xim		  im;
    XICID	  icid;
#endif /* NeedFunctionPrototypes */
{
    Xic		  pic;

    for (pic = (Xic)im->core.ic_chain; pic; pic = (Xic)pic->core.next) {
	if (pic->private.proto.icid == icid)
	    return pic;
    }
    return (Xic)0;
}

Private void
_XimProcSetEventMask(ic, buf)
    Xic		 ic;
    XPointer	 buf;
{
    EVENTMASK	*buf_l = (EVENTMASK *)buf;

    ic->private.proto.forward_event_mask     = buf_l[0];
    ic->private.proto.synchronous_event_mask = buf_l[1];
    return;
}

Public Bool
#if NeedFunctionPrototypes
_XimSetEventMaskCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimSetEventMaskCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID        imid = buf_s[0];
    XICID        icid = buf_s[1];
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if ((imid == im->private.proto.imid)
     && (ic = _XimICOfXICID(im, icid))) {
	_XimProcSetEventMask(ic, (XPointer)&buf_s[2]);
	Xfree(data);
	return True;
    }
    return False;
}

Private Bool
_XimSyncCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
{
    Xic		 ic  = (Xic)arg;
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];

    if ((major_opcode == XIM_SYNC_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid)
     && (icid == ic->private.proto.icid))
	return True;
    return False;
}

Public Bool
_XimSync(im, ic)
    Xim		 im;
    Xic		 ic;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_SYNC, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply,
					_XimSyncCheck, (XPointer)ic)))
	return False;
    Xfree(reply);
    return True;
}

Public Bool
_XimProcSyncReply(im, ic)
    Xim		 im;
    Xic		 ic;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */

    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16);			/* sizeof icid */

    _XimSetHeader((XPointer)buf, XIM_SYNC_REPLY, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);
    return True;
}

Public Bool
_XimRespSyncReply(ic, mode)
    Xic		 ic;
    BITMASK16	 mode;
{
    if (mode & XimSYNCHRONUS) /* SYNC Request */
	MARK_NEED_SYNC_REPLY(ic);
    return True;
}

Public Bool
#if NeedFunctionPrototypes
_XimSyncCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimSyncCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID        imid = buf_s[0];
    XICID        icid = buf_s[1];
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if ((imid == im->private.proto.imid)
     && (ic = _XimICOfXICID(im, icid))) {
	(void)_XimProcSyncReply(im, ic);
	Xfree(data);
	return True;
    }
    return False;
}

Private INT16
_XimSetEventToWire(ev, event)
    XEvent	*ev;
    xEvent	*event;
{
    if (!(_XimProtoEventToWire(ev, event, False)))
	return 0;
    event->u.u.sequenceNumber =
		((XAnyEvent *)ev)->serial & (unsigned long)0xffff;
    return sz_xEvent;
}

Private Bool
_XimForwardEventCore(ic, ev, sync)
    Xic		 ic;
    XEvent	*ev;
    Bool	 sync;
{
    Xim		 im = (Xim)ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    XPointer	 reply;
    INT16	 len;

    if (!(len = _XimSetEventToWire(ev, (xEvent *)&buf_s[4])))
	return False;				/* X event */

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */
    buf_s[2] = sync ? XimSYNCHRONUS : 0;	/* flag */
    buf_s[3] =
        (CARD16)((((XAnyEvent *)ev)->serial & ~((unsigned long)0xffff)) >> 16);
						/* serial number */

    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(CARD16)			/* sizeof icid */
	 + sizeof(BITMASK16)			/* sizeof flag */
	 + sizeof(CARD16);			/* sizeof serila number */

    _XimSetHeader((XPointer)buf, XIM_FORWARD_EVENT, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);

    if (sync) {
	if (!(im->private.proto.recv(im, &len, &reply,
					_XimSyncCheck, (XPointer)ic)))
	    return False;
	Xfree(reply);
    }
    return True;
}

Public Bool
_XimForwardEvent(ic, ev, sync)
    Xic		 ic;
    XEvent	*ev;
    Bool	 sync;
{
    Xim		 im = (Xim)ic->core.im;

    MARK_PREVIOUS_FORWARDEVENT(im);

#ifndef NOT_EXT_FORWARD
    if (((ev->type == KeyPress) || (ev->type == KeyRelease)))
	if (_XimExtForwardKeyEvent(ic, (XKeyEvent *)ev, sync))
	    return True;
#endif
    return _XimForwardEventCore(ic, ev, sync);
}

Private void
_XimSetWireToEvent(d, event, ev, serial)
    Display	*d;
    xEvent	*event;
    XEvent	*ev;
    INT16	 serial;
{
    _XimProtoWireToEvent(ev, event, False);
    ev->xany.serial |= serial << 16;
    ev->xany.send_event = False;
    ev->xany.display = d;
    return;
}

Private Bool
_XimProcKey(d, ic, kev, buf)
    Display		*d;
    Xic			 ic;
    XKeyEvent		*kev;
    CARD16		*buf;
{
    INT16	 serial = buf[0];
    xEvent	*ev = (xEvent *)&buf[1];

    _XimSetWireToEvent(d, ev, (XEvent *)kev, serial);
    MARK_FABLICATED(ic);
    return True;
}

Private Bool
_XimForwardEventRecv(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    CARD16	*buf_s = (CARD16 *)buf;
    Display	*d = im->core.display;
    XEvent	 ev;

    /* XXX  Only KeyEvent !!!  FIX ME !!! */
    if (!(_XimProcKey(d, ic, (XKeyEvent *)&ev, &buf_s[1])))
	return False;

    (void)_XimRespSyncReply(ic, buf_s[0]);

    XPutBackEvent(d, &ev);
    return True;
}

Public Bool
#if NeedFunctionPrototypes
_XimForwardEventCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimForwardEventCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID        imid = buf_s[0];
    XICID        icid = buf_s[1];
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if ((imid == im->private.proto.imid)
     && (ic = _XimICOfXICID(im, icid))) {
	(void)_XimForwardEventRecv(im, ic, (XPointer)&buf_s[2]);
	Xfree(data);
	return True;
    }
    return False;
}

Private Bool
_XimRegisterTriggerkey(im, buf)
    Xim			 im;
    XPointer		 buf;
{
    CARD32		*buf_l = (CARD32 *)buf;
    CARD32		 len;
    CARD32 		*key;

    if (IS_DYNAMIC_EVENT_FLOW(im))	/* already Dynamic event flow mode */
	return True;

    /*
     *  register onkeylist
     */

    len = buf_l[0];				/* length of on-keys */
    len += sizeof(INT32);			/* sizeof length of on-keys */

    if (!(key = (CARD32 *)Xmalloc(len)))
	return False;
    bcopy(buf, key, len);
    im->private.proto.im_onkeylist	     = key;

    MARK_DYNAMIC_EVENT_FLOW(im);

    /*
     *  register offkeylist
     */

    buf_l = (CARD32 *)((char *)buf + len);
    len = buf_l[0];				/* length of off-keys */
    len += sizeof(INT32);			/* sizeof length of off-keys */

    if (!(key = (CARD32 *)Xmalloc(len)))
	return False;

    bcopy(buf, key, len);
    im->private.proto.im_offkeylist = key;

    return True;
}

Public Bool
#if NeedFunctionPrototypes
_XimRegisterTriggerKeysCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimRegisterTriggerKeysCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID        imid = buf_s[0];
    Xim		 im = (Xim)call_data;

    if (imid == im->private.proto.imid) {
	(void )_XimRegisterTriggerkey(im, (XPointer)&buf_s[2]);
	Xfree(data);
	return True;
    }
    return False;
}

Public EVENTMASK
_XimGetWindowEventmask(ic)
    Xic		 ic;
{
    Xim			im = (Xim )ic->core.im;
    XWindowAttributes	atr;

    if (!XGetWindowAttributes(im->core.display, ic->core.focus_window, &atr))
	return 0;
    return (EVENTMASK)atr.your_event_mask;
}


Private Bool
_XimTriggerNotifyCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
{
    Xic		 ic  = (Xic)arg;
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];

    if ((major_opcode == XIM_TRIGGER_NOTIFY_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid)
     && (icid == ic->private.proto.icid))
	return True;
    return False;
}

Public Bool
_XimTriggerNotify(im, ic, mode, idx)
    Xim		 im;
    Xic		 ic;
    int		 mode;
    CARD32	 idx;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    CARD32	*buf_l = (CARD32 *)&buf[XIM_HEADER_SIZE];
    XPointer	 reply;
    INT16	 len;
    EVENTMASK	 mask = _XimGetWindowEventmask(ic);

    buf_s[0] = im->private.proto.imid;	/* imid */
    buf_s[1] = ic->private.proto.icid;	/* icid */
    buf_l[1] = mode;			/* flag */
    buf_l[2] = idx;			/* index of keys list */
    buf_l[3] = mask;			/* select-event-mask */

    len = sizeof(CARD16)		/* sizeof imid */
	+ sizeof(CARD16)		/* sizeof icid */
	+ sizeof(CARD32)		/* sizeof flag */
	+ sizeof(CARD32)		/* sizeof index of key list */
	+ sizeof(EVENTMASK);		/* sizeof select-event-mask */

    _XimSetHeader((XPointer)buf, XIM_TRIGGER_NOTIFY, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply,
				_XimTriggerNotifyCheck, (XPointer)ic)))
	return False;
    Xfree(reply);
    return True;
}

Private Bool
_XimProcCommit(d, ic, ev, buf)
    Display	*d;
    Xic		 ic;
    XKeyEvent	*ev;
    CARD16	*buf;
{
    int		 len = buf[0];
    char	*commit;

    if (ic->private.proto.xim_commit)
	return False;

    len += sizeof(INT16);		/* sizeof length */

    if (!(commit = Xmalloc(len + 1)))
	return False;

    bcopy(&buf[1], commit, len);
    commit[len] = 0;
    ic->private.proto.xim_commit = commit;
    MARK_FABLICATED(ic);

    ev->type = KeyPress;
    ev->send_event = False;
    ev->display = d;
    ev->window = ic->core.focus_window;
    ev->keycode = 0;
    return True;
}

Private Bool
_XimCommitRecv(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    CARD16	*buf_s = (CARD16 *)buf;
    Display	*d = im->core.display;
    XEvent	 ev;

    if (!(_XimProcCommit(d, ic, (XKeyEvent *)&ev, &buf_s[1])))
	return False;

    (void)_XimRespSyncReply(ic, buf_s[0]);

    XPutBackEvent(d, &ev);
    return True;
}

Public Bool
#if NeedFunctionPrototypes
_XimCommitCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimCommitCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID        imid = buf_s[0];
    XICID        icid = buf_s[1];
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if ((imid == im->private.proto.imid)
     && (ic = _XimICOfXICID(im, icid))) {
	(void)_XimCommitRecv(im, ic, (XPointer)&buf_s[2]);
	Xfree(data);
	return True;
    }
    return False;
}

Private Bool
_XimProcError(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    /*
     * Not yet
     */
    return True;
}

Public Bool
#if NeedFunctionPrototypes
_XimErrorCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimErrorCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    BITMASK16	 flag = buf_s[2];
    XIMID        imid;
    XICID        icid;
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if (flag & XIM_IMID_VALID) {
	imid = buf_s[0];
	if (imid != im->private.proto.imid)
	    return False;
    }
    if (flag & XIM_ICID_VALID) {
	icid = buf_s[1];
	if (!(ic = _XimICOfXICID(im, icid)))
	    return False;
    }
    (void)_XimProcError(im, ic, (XPointer)&buf_s[3]);
    Xfree(data);

    return True;
}

Public Bool
_XimError(im, ic, error_code, detail_length, type, detail)
    Xim		 im;
    Xic		 ic;
    CARD16	 error_code;
    INT16	 detail_length;
    CARD16	 type;
    char	*detail;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len = 0;

    buf_s[0] = im->private.proto.imid;	/* imid */
    buf_s[2] = XIM_IMID_VALID;		/* flag */
    if (ic) {
    	buf_s[1] = ic->private.proto.icid;	/* icid */
	buf_s[2] |= XIM_ICID_VALID;		/* flag */
    }
    buf_s[3] = error_code;			/* Error Code */
    buf_s[4] = detail_length;			/* length of error detail */
    buf_s[5] = type;				/* type of error detail */

    if (detail_length && detail) {
	len = detail_length;
	bcopy(detail, &buf_s[6], len);
	XIM_SET_PAD(&buf_s[6], len);
    }

    len += sizeof(CARD16)		/* sizeof imid */
	 + sizeof(CARD16)		/* sizeof icid */
	 + sizeof(BITMASK16)		/* sizeof flag */
	 + sizeof(CARD16)		/* sizeof error_code */
	 + sizeof(INT16)		/* sizeof length of detail */
	 + sizeof(CARD16);		/* sizeof type */

    _XimSetHeader((XPointer)buf, XIM_ERROR, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);
    return True;
}

Public int
_XimProtoMbLookupString(xic, ev, buffer, bytes, keysym, status)
    XIC		 xic;
    XKeyEvent	*ev;
    char	*buffer;
    int		 bytes;
    KeySym	*keysym;
    Status	*status;
{
    Xic		 ic = (Xic)xic;
    Xim		 im = (Xim)ic->core.im;
    int		 ret;
    Status	 tmp_status;

    if (!status)
	status = &tmp_status;

    if (IS_FABLICATED(ic)) {
	_XimPendingFilter(ic);
	UNMARK_FABLICATED(ic);
    }
    if ((ev->type == KeyPress) && (ev->keycode == 0)) { /* Filter function */
	if (!ic->private.proto.xim_commit) {
	    *status = XLookupNone;
	    return 0;
	}

	ret = _Xlcctstombs(ic->core.im->core.lcd, buffer,
					ic->private.proto.xim_commit, bytes);
	if (ret > 0)
	    *status = XLookupChars;
	else
	    *status = XLookupNone;

	Xfree(ic->private.proto.xim_commit);
	ic->private.proto.xim_commit = NULL;
	return ret;
    } else  if (ev->type == KeyPress) {
	ret = _XimLookupMBText(ic, ev, (unsigned char *)buffer,
							bytes, keysym, NULL);
	if (ret > 0) {
	    if (keysym && *keysym != NoSymbol)
		*status = XLookupBoth;
	    else
		*status = XLookupChars;
	} else {
	    if (keysym && *keysym != NoSymbol)
		*status = XLookupKeySym;
	    else
		*status = XLookupNone;
	}
    } else
	*status = XLookupNone;

    return ret;
}

Public int
_XimProtoWcLookupString(xic, ev, buffer, bytes, keysym, status)
    XIC		 xic;
    XKeyEvent	*ev;
    wchar_t	*buffer;
    int		 bytes;
    KeySym	*keysym;
    Status	*status;
{
    Xic		 ic = (Xic)xic;
    Xim		 im = (Xim)ic->core.im;
    int		 ret;
    Status	 tmp_status;

    if (!status)
	status = &tmp_status;

    if (IS_FABLICATED(ic)) {
	_XimPendingFilter(ic);
	UNMARK_FABLICATED(ic);
    }
    if (ev->type == KeyPress && ev->keycode == 0) { /* Filter function */
	if (!ic->private.proto.xim_commit) {
	    *status = XLookupNone;
	    return 0;
	}

	ret = _Xlcctstowcs(ic->core.im->core.lcd, buffer,
					ic->private.proto.xim_commit, bytes);
	if (ret > 0)
	    *status = XLookupChars;
	else
	    *status = XLookupNone;

	Xfree(ic->private.proto.xim_commit);
	ic->private.proto.xim_commit = NULL;
	return ret;
    } else if (ev->type == KeyPress) {
	ret = _XimLookupWCText(ic, ev, buffer, bytes, keysym, NULL);
	if (ret > 0) {
	    if (keysym && *keysym != NoSymbol)
		*status = XLookupBoth;
	    else
		*status = XLookupChars;
	} else {
	    if (keysym && *keysym != NoSymbol)
		*status = XLookupKeySym;
	    else
		*status = XLookupNone;
	}
    } else
	*status = XLookupNone;

    return ret;
}

Public Bool
_XimEncodingNegitiation(im)
    Xim		 im;
{
    /*
     * Not yet
     */
    return True;
}
