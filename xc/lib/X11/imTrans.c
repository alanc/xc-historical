/* $XConsortium: imTrans.c,v 1.3 94/03/18 11:29:37 mor Exp $ */
/******************************************************************

           Copyright 1992 by Sun Microsystems, Inc.
           Copyright 1992, 1993, 1994 by FUJITSU LIMITED

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
#define NEED_EVENTS
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
	if ((spec->trans_conn = _XimXTransOpenCOTSClient (
	    spec->address)) == NULL)
	{
	    break;
	}

	if ((connect_stat = _XimXTransConnect (
	    spec->trans_conn, spec->address)) < 0)
	{
	    _XimXTransClose (spec->trans_conn);
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

    spec->fd = _XimXTransGetConnectionNumber (spec->trans_conn);

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

    _XimXTransDisconnect(spec->trans_conn);
    (void)_XimXTransClose(spec->trans_conn);
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
_XimTransRegisterDispatcher(
    Xim				 im,
    Bool			 (*callback)(
#if NeedNestedPrototypes
					     Xim, INT16, XPointer, XPointer
#endif
					     ),
    XPointer			 call_data)
#else
_XimTransRegisterDispatcher(im, callback, call_data)
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
_XimTransCallDispatcher(Xim im, INT16 len, XPointer data)
#else
_XimTransCallDispatcher(im, len, data)
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
    return _XimFilterWaitEvent(im);
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
_XimTransWrite(Xim im, INT16 len, XPointer data)
#else
_XimTransWrite(im, len, data)
    Xim			 im;
    INT16		 len;
    XPointer		 data;
#endif
{
    TransSpecRec	*spec	= (TransSpecRec *)im->private.proto.spec;
    char		*buf = (char *)data;
    register int	 nbyte;

    while (len > 0) {
	if ((nbyte = _XimXTransWrite(spec->trans_conn, buf, len)) <= 0)
	    return False;
	len -= nbyte;
	buf += nbyte;
    }
    return True;
}


Public Bool
_XimTransRead(im, recv_buf, buf_len, ret_len)
    Xim			 im;
    XPointer		 recv_buf;
    int			 buf_len;
    int			*ret_len;
{
    TransSpecRec	*spec = (TransSpecRec *)im->private.proto.spec;
    int			 len;

    if ((len = _XimXTransRead(spec->trans_conn, recv_buf, buf_len)) <= 0)
	return False;
    *ret_len = len;
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
    im->private.proto.write    = _XimTransWrite;
    im->private.proto.read     = _XimTransRead;
    im->private.proto.flush    = _XimTransFlush;
    im->private.proto.register_dispatcher = _XimTransRegisterDispatcher;
    im->private.proto.call_dispatcher = _XimTransCallDispatcher;

    return True;
}
