/* $XConsortium$ */
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

#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include "Xlibint.h"
#include "Xlibnet.h"
#include <X11/Xtrans.h>
#include "Xlcint.h"
#include "Ximint.h"
#include "XimTrans.h"


#ifndef XIM_CONNECTION_RETRIES
#define XIM_CONNECTION_RETRIES 5
#endif


Private Bool
_XimTransConnect(im)
    Xim			 im;
{
    TransSpecRec	*spec = (TransSpecRec *)im->private.proto.spec;
    int			connect_stat, retry;
    Window		window;

    for (retry = XIM_CONNECTION_RETRIES; retry >= 0; retry--)
    {
	if ((spec->trans_conn = _XIMTransOpenCOTSClient (
	    spec->address)) == NULL)
	{
	    break;
	}

	if ((connect_stat = _XIMTransConnect (
	    spec->trans_conn, spec->address)) < 0)
	{
	    _XIMTransClose (spec->trans_conn);
	    spec->trans_conn = NULL;

	    if (connect_stat == TRANS_TRY_CONNECT_AGAIN)
	    {
		sleep(1);
		continue;
	    }
	    else
		break;
	}
	else
	    break;
    }

    if (spec->trans_conn == NULL)
	return False;

    spec->fd = _XIMTransGetConnectionNumber (spec->trans_conn);

    if (!(window = XCreateSimpleWindow(im->core.display,
		DefaultRootWindow(im->core.display), 0, 0, 1, 1, 1, 0, 0)))
	return False;
    spec->window = window;

    _XRegisterFilterByType(im->core.display, window, KeyPress, KeyPress,
				_XimTransFilterWaitEvent, (XPointer)im);

    return _XRegisterInternalConnection(im->core.display, spec->fd, 
			(_XInternalConnectionProc)_XimTransInternalConnection, 
			(XPointer)im);
}


Private Bool
_XimTransShutdown(im)
    Xim im;
{
    TransSpecRec *spec = (TransSpecRec *)im->private.proto.spec;

    _XIMTransDisconnect(spec->trans_conn);
    (void)_XIMTransClose(spec->trans_conn);
    _XimFreeTransIntrCallback(im);
    _XUnregisterInternalConnection(im->core.display, spec->fd);
    _XUnregisterFilter(im->core.display, spec->window,
				_XimTransFilterWaitEvent, (XPointer)im);
    XDestroyWindow(im->core.display, spec->window);
    Xfree(spec->address);
    Xfree(spec);
    return True;
}



Public Bool
#if NeedFunctionPrototypes
_XimTransIntrCallback(
    Xim				 im,
    Bool			 (*callback)(
#if NeedNestedPrototypes
					     Xim, INT16, XPointer, XPointer
#endif
					     ),
    XPointer			 call_data)
#else
_XimTransIntrCallback(im, callback, call_data)
    Xim				 im;
    Bool			 (*callback)();
    XPointer			 call_data;
#endif
{
    TransSpecRec		*spec = (TransSpecRec *)im->private.proto.spec;
    TransIntrCallbackPtr	 rec;

    if (!(rec = (TransIntrCallbackPtr)Xmalloc(sizeof(TransIntrCallbackRec))))
        return False;

    rec->func       = callback;
    rec->call_data  = call_data;
    rec->next       = spec->intr_cb;
    spec->intr_cb   = rec;
    return True;
}


Public void
_XimFreeTransIntrCallback(im)
    Xim				 im;
{
    TransSpecRec		*spec = (TransSpecRec *)im->private.proto.spec;
    register TransIntrCallbackPtr	 rec, next;

    for (rec = spec->intr_cb; rec;) {
	next = rec->next;
	Xfree(rec);
	rec = next;
    }
    return;
}


Public Bool
#if NeedFunctionPrototypes
_XimTransIntrCallbackCheck(Xim im, INT16 len, XPointer data)
#else
_XimTransIntrCallbackCheck(im, len, data)
    Xim				 im;
    INT16			 len;
    XPointer			 data;
#endif
{
    TransSpecRec		*spec = (TransSpecRec *)im->private.proto.spec;
    TransIntrCallbackRec	*rec;

    for (rec = spec->intr_cb; rec; rec = rec->next) {
	if ((*rec->func)(im, len, data, rec->call_data))
	    return True;
    }
    return False;
}


Public Bool
_XimTransFilterWaitEvent(d, w, ev, arg)
    Display		*d;
    Window		 w;
    XEvent		*ev;
    XPointer		 arg;
{
    Xim			 im = (Xim)arg;
    TransSpecRec	*spec = (TransSpecRec *)im->private.proto.spec;

    spec->is_putback  = False;
    return _XimFilterWaitEvent(im, (XPointer)NULL);
}


Public void
_XimTransInternalConnection(d, fd, arg)
    Display		*d;
    int			 fd;
    XPointer		 arg;
{
    Xim			 im = (Xim)arg;
    XEvent		 ev;
    XKeyEvent		*kev;
    TransSpecRec	*spec = (TransSpecRec *)im->private.proto.spec;

    if (spec->is_putback)
	return;
    kev = (XKeyEvent *)&ev;
    kev->type = KeyPress;
    kev->send_event = False;
    kev->display = im->core.display;
    kev->window = spec->window;
    kev->keycode = 0;
    XPutBackEvent(im->core.display, &ev);
    XFlush(im->core.display);
    spec->is_putback = True;
    return;
}


Public Bool
#if NeedFunctionPrototypes
_XimTransSend(Xim im, INT16 len, XPointer data)
#else
_XimTransSend(im, len, data)
    Xim			 im;
    INT16		 len;
    XPointer		 data;
#endif
{
    TransSpecRec	*spec	= (TransSpecRec *)im->private.proto.spec;
    char		*buf = (char *)data;
    register int	 nbyte;

    while (len > 0) {
	if ((nbyte = _XIMTransWrite(spec->trans_conn, buf, len)) <= 0)
	    return False;
	len -= nbyte;
	buf += nbyte;
    }
    return True;
}


Public Bool
_XimTransRecv(im, recv_buf, recv_point, min_len, buf_len, ret_len, arg)
    Xim			 im;
    XPointer		 recv_buf;
    int			 recv_point;
    int			 min_len;
    int			 buf_len;
    int			*ret_len;
    XPointer		 arg;
{
    TransSpecRec	*spec = (TransSpecRec *)im->private.proto.spec;
    int			 len;

    if ((min_len > buf_len) ||  (recv_point > buf_len))
	return False;

    while(recv_point < min_len) {
	if ((len = _XIMTransRead(spec->trans_conn, &recv_buf[recv_point],
						(buf_len - recv_point))) <= 0)
	    return False;
	recv_point += len;
    }
    *ret_len = recv_point;
    return True;
}


Public void
_XimTransFlush(im)
    Xim		 im;
{
    return;
}



Public Bool
_XimTransConf(im, address)
    Xim		   	 im;
    char	 	*address;
{
    char		*p;
    char		*paddr;
    TransSpecRec	*spec;

    if (!(paddr = (char *)Xmalloc(strlen(address) + 1)))
	return False;

    if (!(spec = (TransSpecRec *) Xmalloc(sizeof(TransSpecRec)))) {
	Xfree(paddr);
	return False;
    }

    bzero(spec, sizeof(TransSpecRec));

    (void)strcpy(paddr, address);
    spec->address   = paddr;

    im->private.proto.spec     = (XPointer)spec;
    im->private.proto.connect  = _XimTransConnect;
    im->private.proto.shutdown = _XimTransShutdown;
    im->private.proto.send     = _XimTransSend;
    im->private.proto.recv     = _XimTransRecv;
    im->private.proto.flush    = _XimTransFlush;
    im->private.proto.intr_cb  = _XimTransIntrCallback;
    im->private.proto.check_cb = _XimTransIntrCallbackCheck;

    return True;
}
