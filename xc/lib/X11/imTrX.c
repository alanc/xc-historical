/* $XConsortium: imTrX.c,v 1.4 93/09/18 13:16:59 rws Exp $ */
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
_XimXIntrCallback(im, callback, call_data)
    Xim			 im;
    Bool		 (*callback)(
#if NeedNestedPrototypes
				     Xim, INT16, XPointer, XPointer
#endif
				     );
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
_XimXFreeIntrCallback(im)
    Xim			 im;
{
    XSpecRec		*spec = (XSpecRec *)im->private.proto.spec;
    register XIntrCallbackPtr rec, next;

    for (rec = spec->intr_cb; rec;) {
	next = rec->next;
	Xfree(rec);
	rec = next;
    }
    return;
}

Private Bool
#if NeedFunctionPrototypes
_XimXIntrCallbackCheck(Xim im, INT16 len, XPointer data)
#else
_XimXIntrCallbackCheck(im, len, data)
    Xim			 	 im;
    INT16			 len;
    XPointer			 data;
#endif
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
_XimXFilterWaitEvent(d, w, ev, arg)
    Display	*d;
    Window	 w;
    XEvent	*ev;
    XPointer	 arg;
{
    Xim		 im = (Xim)arg;

    return _XimFilterWaitEvent(im, (XPointer)ev);
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
#if NeedFunctionPrototypes
_XimXConnect(Xim im)
#else
_XimXConnect(im)
    Xim im;
#endif
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
			ClientMessage, ClientMessage,
			 _XimXFilterWaitEvent, (XPointer)im);
    return True;
}

Private Bool
#if NeedFunctionPrototypes
_XimXShutdown(Xim im)
#else
_XimXShutdown(im)
    Xim		 im;
#endif
{
    XSpecRec	*spec = (XSpecRec *)im->private.proto.spec;

    /* ClientMessage Event Filter */
    _XUnregisterFilter(im->core.display,
	    ((XSpecRec *)im->private.proto.spec)->lib_connect_wid,
	    _XimXFilterWaitEvent, (XPointer)im);
    XDestroyWindow(im->core.display,
	    ((XSpecRec *)im->private.proto.spec)->lib_connect_wid);
    _XimXFreeIntrCallback(im);
    Xfree(spec);
    return True;
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
#if NeedFunctionPrototypes
_XimXSend(Xim im, INT16 len, XPointer data)    
#else
_XimXSend(im, len, data)    
    Xim		 im;
    INT16	 len;
    XPointer	 data;
#endif
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
	memcpy((char *)p, data, len);
    }

    XSendEvent(im->core.display, spec->ims_connect_wid,
					False, NoEventMask, &event);
    return True;
}

Private Bool
_XimXGetReadData(im, buf, buf_len, ret_len, event)
    Xim			  im;
    char		 *buf;
    int			  buf_len;
    int			 *ret_len;
    XEvent		 *event;
{
    char		 *data;
    int			  len;

    char		  tmp_buf[XIM_CM_DATA_SIZE];
    XSpecRec		 *spec = (XSpecRec *)im->private.proto.spec;
    unsigned long	  length;
    Atom		  prop;
    int			  return_code;
    Atom		  type_ret;
    int			  format_ret;
    unsigned long	  nitems;
    unsigned long	  bytes_after_ret;
    unsigned char	 *prop_ret;

    if (event->xclient.format == 8) {
	data = event->xclient.data.b;
	if (buf_len >= XIM_CM_DATA_SIZE) {
	    (void)memcpy(buf, data, XIM_CM_DATA_SIZE);
	    *ret_len = XIM_CM_DATA_SIZE;
	} else {
	    (void)memcpy(buf, data, buf_len);
	    len = XIM_CM_DATA_SIZE - buf_len;
	    (void)memcpy(tmp_buf, &data[buf_len], len);
	    bzero(data, XIM_CM_DATA_SIZE);
	    (void)memcpy(data, tmp_buf, len);
	    XPutBackEvent(im->core.display, event);
	    *ret_len = buf_len;
	}
    } else {
	length = (unsigned long)event->xclient.data.l[0];
	prop = (Atom)event->xclient.data.l[1];
	return_code = XGetWindowProperty(im->core.display,
		spec->lib_connect_wid, prop, 0L,
		((length + 3)/ 4), True, AnyPropertyType,
		&type_ret, &format_ret, &nitems, &bytes_after_ret, &prop_ret);
	if (return_code != Success || format_ret == 0 || nitems == 0) {
	    if (return_code == Success)
		XFree(prop_ret);
	    return False;
	}
	if (buf_len >= length) {
	    (void)memcpy(buf, prop_ret, (int)nitems);
	    *ret_len  = (int)nitems;
	} else {
	    (void)memcpy(buf, prop_ret, buf_len);
	    *ret_len  = buf_len;
	    len = nitems - buf_len;
	    prop = XInternAtom(im->core.display, "_XimXSaveProp", False);	
	    XChangeProperty(im->core.display, spec->ims_connect_wid, prop,
		XA_STRING, 8, PropModeReplace, &prop_ret[buf_len], len); 
	    event->xclient.data.l[0] = (long)len;
	    event->xclient.data.l[1] = (long)prop;
	    XPutBackEvent(im->core.display, event);
	}
	XFree(prop_ret);

	if (bytes_after_ret > 0) {
	    XGetWindowProperty(im->core.display,
		spec->lib_connect_wid, prop, 0L,
		((length + bytes_after_ret + 3)/ 4), True, AnyPropertyType,
		&type_ret, &format_ret, &nitems, &bytes_after_ret, &prop_ret);
	    XChangeProperty(im->core.display, spec->ims_connect_wid, prop,
		XA_STRING, 8, PropModeReplace, &prop_ret[length],
		(nitems - length)); 
	    XFree(prop_ret);
	}
    }
    return True;
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
#if NeedFunctionPrototypes
_XimXRecv(Xim im, XPointer recv_buf, int recv_point, int min_len, int buf_len, int *ret_len, XPointer arg)
#else
_XimXRecv(im, recv_buf, recv_point, min_len, buf_len, ret_len, arg)
    Xim		 im;
    XPointer	 recv_buf;
    int		 recv_point;
    int		 min_len;
    int		 buf_len;
    int		*ret_len;
    XPointer	 arg;
#endif
{
    XEvent	*ev;
    XEvent	 event;
    Atom	 prop;
    int		 len;

    if ((min_len > buf_len) || (recv_point > buf_len))
	return False;

   if (!arg) {
	ev = &event;
	while (recv_point < min_len) {
	    XIfEvent(im->core.display, ev, _CheckCMEvent, (XPointer)im);
	    if (!(_XimXGetReadData(im, &recv_buf[recv_point],
			 		(buf_len - recv_point), &len, ev)))
		return False;
	    recv_point += len;
	}
	*ret_len = recv_point;
    } else {
	ev = (XEvent *)arg;
	if (!(_XimXGetReadData(im, &recv_buf[recv_point],
			 		(buf_len - recv_point), &len, ev)))
	    return False;
	*ret_len = recv_point + len;
    }
    return True;
}

Private void
#if NeedFunctionPrototypes
_XimXFlush(Xim im)
#else
_XimXFlush(im)
    Xim		 im;
#endif
{
    XFlush(im->core.display);
    return;
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
    im->private.proto.recv     = _XimXRecv;
    im->private.proto.flush    = _XimXFlush;
    im->private.proto.intr_cb  = _XimXIntrCallback;
    im->private.proto.check_cb = _XimXIntrCallbackCheck;

    return True;
}
