/* $XConsortium: imTrX.c,v 1.1 93/09/17 13:29:16 rws Exp $ */
/******************************************************************

           Copyright 1992 by Sun Microsystems, Inc.
           Copyright 1992, 1993 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and FUJITSU LIMITED not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and FUJITSU LIMITED makes no representations about
the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Sun Microsystems Inc. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL Sun Microsystems, Inc. AND FUJITSU LIMITED
BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
          Takashi Fujiwara     FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include <string.h>
#include <X11/Xatom.h>
#define NEED_EVENTS
#include "Xlibint.h"
#include "Xlcint.h"
#include "Ximint.h"
#include "XimTrInt.h"
#include "XimTrX.h"

Private Bool
_GetReadData(im, event, len, data, prop)
    Xim			  im;
    XEvent		 *event;
    INT16		 *len;
    XPointer		 *data;
    Atom		 *prop;
{
    unsigned long	  reply_length;
    unsigned char	 *buf;
    XPointer		  reply_data;
    int			  return_code;
    Atom		  type_ret;
    int			  format_ret;
    unsigned long	  bytes_after_ret;
    XSpecRec		 *spec = (XSpecRec *)im->private.proto.spec;
    unsigned long	  length;

    if (event->xclient.format == 8) {
	reply_length = XIM_CM_DATA_SIZE; /* XXX */
	*prop = (Atom)0;
	if (!(reply_data = (XPointer)Xmalloc(reply_length)))
	    return False;
	bcopy(event->xclient.data.b, reply_data, reply_length);
    } else {
	length = (unsigned long)event->xclient.data.l[0];
	*prop = (Atom)event->xclient.data.l[1];
	return_code = XGetWindowProperty(im->core.display,
		spec->lib_connect_wid, *prop, 0L,
		length, False, AnyPropertyType, &type_ret,
		&format_ret, &reply_length, &bytes_after_ret, &buf);
	if (return_code != Success || format_ret == 0 || reply_length == 0) {
	    if (return_code == Success)
		XFree(buf);
	    return False;
	}
	if ((int)reply_length < (int)length)
	    return False;

	if (!(reply_data = (XPointer)Xmalloc(reply_length))) {
	    XFree(buf);
	    return False;
	}
	bcopy(buf, reply_data, reply_length);
	XFree(buf);
    }

    *len  = (INT16)reply_length;
    *data = reply_data;
    return True;
}

Private Bool
_IntrCallbackCheck(im, len, data)
    Xim			 	 im;
    INT16			 len;
    XPointer			 data;
{
    register XIntrCallbackRec	*rec;
    XSpecRec		*spec = (XSpecRec *)im->private.proto.spec;

    for (rec = spec->intr_cb; rec; rec = rec->next) {
	if ((*rec->func)(im, len, data, rec->call_data))
	    return True;
    }
    return False;
}

Private Bool
_FilterWaitEvent(d, w, ev, xim)
    Display	*d;
    Window	 w;
    XEvent	*ev;
    XPointer	 xim;
{
    INT16	len;
    XPointer	data;
    Atom	 prop;
    Xim		 im = (Xim)xim;
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    if (!(_GetReadData(im, ev, &len, &data, &prop)))
	return False;
    if (_IntrCallbackCheck(im, len, data)) {
	if (prop)
	    XDeleteProperty(im->core.display, spec->lib_connect_wid, prop);
	return True;
    }
    Xfree(data);
    return False;
}

Private Bool
_CheckCMEvent(display, event, xim)
    Display	*display;
    XEvent	*event;
    XPointer	 xim;
{
    Xim		 im = (Xim)xim;
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    if ((event->type == ClientMessage)
     && (event->xclient.message_type == spec->improtocolid))
	return True;
    return False;
}

Private Bool
_XimXRecv(im, len, data, predicate, arg)
    Xim		 im;
    INT16	*len;
    XPointer	*data;
    Bool	(*predicate)();
    XPointer	 arg;
{
    XEvent	 event;
    Atom	 prop;
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    for (;;) {
	XIfEvent(im->core.display, &event, _CheckCMEvent, (XPointer)im);

	if (!(_GetReadData(im, &event, len, data, &prop)))
	    return False;

	if ((*predicate)(im, *len, *data, arg)) {
	    if (prop)
		XDeleteProperty(im->core.display, spec->lib_connect_wid, prop);
	    return True;
	}

	if (_IntrCallbackCheck(im, *len, *data)) {
	    if (prop)
		XDeleteProperty(im->core.display, spec->lib_connect_wid, prop);
	    continue;
	}

	XPutBackEvent(im->core.display, &event);
	Xfree(*data);
    }
}

Private char *
_NewAtom(atomName)
    char	*atomName;
{
    static int	 sequence = 0;

    (void)sprintf(atomName, "_client%d", sequence);
    sequence = ((sequence < 20) ? sequence + 1 : 0);
    return atomName;
}

Private Bool
_XimXSend(im, len, data)    
    Xim		 im;
    INT16	 len;
    XPointer	 data;
{
    Atom	 atom;
    char	 atomName[16];
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;
    XEvent	 event;
    CARD8	 *p;

    bzero(&event,sizeof(XEvent));
    event.xclient.type         = ClientMessage;
    event.xclient.display      = im->core.display;
    event.xclient.window       = spec->ims_connect_wid;
    event.xclient.message_type = spec->improtocolid;

    if (len > XIM_CM_DATA_SIZE) {
	atom = XInternAtom(im->core.display, _NewAtom(atomName), False);
	XChangeProperty(im->core.display, spec->ims_connect_wid,
			atom, XA_STRING, 8, PropModeAppend,
			(unsigned char *)data, len);
	event.xclient.format = 32;
	event.xclient.data.l[0] = (long)len;
	event.xclient.data.l[1] = (long)atom;
    } else {
	event.xclient.format = 8;
	p = (CARD8 *)&event.xclient.data.b[0];
	bcopy(data, p, len);
    }

    XSendEvent(im->core.display, spec->ims_connect_wid,
					False, NoEventMask, &event);
    return True;
}

Private void
_XimXFlush(im)
    Xim		 im;
{
    XFlush(im->core.display);
    return;
}

Private Bool
_XimXIntrCallBack(im, callback, call_data)
    Xim			 im;
    Bool		 (*callback)();
    XPointer		 call_data;
{
    XIntrCallbackPtr	 rec;
    XSpecRec		*spec = (XSpecRec *)im->private.proto.spec;

    if (!(rec = (XIntrCallbackPtr)Xmalloc(sizeof(XIntrCallbackRec))))
        return False;

    rec->func       = callback;
    rec->call_data  = call_data;
    rec->next       = spec->intr_cb;
    spec->intr_cb   = rec;
    return True;
}

Private void
_XimXFreeIntrCallBack(spec)
   XSpecRec	 	      *spec;
{
    register XIntrCallbackPtr rec, next;

    for (rec = spec->intr_cb; rec;) {
	next = rec->next;
	Xfree(rec);
	rec = next;
    }
    return;
}

Private Bool
_CheckConnect(display, event, xim)
    Display	*display;
    XEvent	*event;
    XPointer	 xim;
{
    Xim		 im = (Xim)xim;
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    if ((event->type == ClientMessage)
     && (event->xclient.message_type == spec->imconnectid))
	return True;
    return False;
}

Private Bool
_XimXConnect(im)
    Xim im;
{
    XEvent	 event;
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    if (!(spec->lib_connect_wid = XCreateSimpleWindow(im->core.display,
		DefaultRootWindow(im->core.display), 0, 0, 1, 1, 1, 0, 0)))
	return False;

    event.xclient.type         = ClientMessage;
    event.xclient.display      = im->core.display;
    event.xclient.window       = im->private.proto.im_window;
    event.xclient.message_type = spec->imconnectid;
    event.xclient.format       = 32;
    event.xclient.data.l[0]    = (CARD32)spec->lib_connect_wid;

    XSendEvent(im->core.display, im->private.proto.im_window,
		 False, NoEventMask, &event);
    XFlush(im->core.display);

    for (;;) {
	XIfEvent(im->core.display, &event, _CheckConnect, (XPointer)im);
	if (event.xclient.type != ClientMessage)
	    return False;
	if (event.xclient.message_type == spec->imconnectid)
	    break;
    }

    spec->ims_connect_wid = (Window)event.xclient.data.l[0];

    /* ClientMessage Event Filter */
    _XRegisterFilterByType(im->core.display, spec->lib_connect_wid,
		ClientMessage, ClientMessage, _FilterWaitEvent, (XPointer)im);
    return True;
}

Private Bool
_XimXShutdown(im)
    Xim		 im;
{
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    /* ClientMessage Event Filter */
    _XUnregisterFilter(im->core.display,
	    ((XSpecRec *)im->private.proto.spec)->lib_connect_wid,
	    _FilterWaitEvent, (XPointer)im);
    XDestroyWindow(im->core.display,
	    ((XSpecRec *)im->private.proto.spec)->lib_connect_wid);
    _XimXFreeIntrCallBack(spec);
    Xfree(spec);
    return True;
}

Public Bool
_XimXConf(im, address)
    Xim		 im;
    char	*address;
{
    XSpecRec	*spec;

    if (!(spec = (XSpecRec *) Xmalloc(sizeof(XSpecRec))))
	return False;
    bzero(spec, sizeof(XSpecRec));

    spec->improtocolid = XInternAtom(im->core.display, _XIM_PROTOCOL, False);
    spec->imconnectid  = XInternAtom(im->core.display, _XIM_XCONNECT, False);

    im->private.proto.spec     = (XPointer)spec;
    im->private.proto.connect  = _XimXConnect;
    im->private.proto.shutdown = _XimXShutdown;
    im->private.proto.send     = _XimXSend;
    im->private.proto.flush    = _XimXFlush;
    im->private.proto.recv     = _XimXRecv;
    im->private.proto.intr_cb  = _XimXIntrCallBack;

    return True;
}
