/* $XConsortium: imTransR.c,v 1.2 93/09/17 17:04:19 rws Exp $ */
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

#include "Xlibint.h"
#include "Xlcint.h"
#include "XimTrInt.h"
#include "Ximint.h"

Public TransportSW _XimTransportRec[] = {
    "X",          1, _XimXConf,  /* 1st entry must be X. 
					This will be a fallback */
#ifdef TCPCONN
    "tcp",        3, _XimInetConf,
#endif /* TCPCONN */
#ifdef UNIXCONN
    "local",      4, _XimInternalConf,
#endif /* UNIXCONN */
#ifdef DNETCONN
    "decnet",     6, _XimDecnetConf,
#endif /* DNETCONN */
#ifdef STREAMSCONN
    "streams",    6, _XimStreamsConf,
#endif /* STREAMSCONN */
    (char *)NULL, 0, (Bool (*)())NULL,
};

Public Bool
_XimConnect(im)
    Xim		 im;
{
    return im->private.proto.connect(im);
}

Public Bool
_XimShutdown(im)
    Xim		 im;
{
    return im->private.proto.shutdown(im);
}

Public Bool
#if NeedFunctionPrototypes
_XimSend(Xim im, INT16 len, XPointer data)
#else
_XimSend(im, len, data)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
#endif
{
    return im->private.proto.send(im, len, data);
}

Private Bool
_CheckProtocolData(im, recv_buf, recv_len, packet, packet_size)
    Xim		  im;
    char	 *recv_buf;
    int		  recv_len;
    char	**packet;
    int		 *packet_size;
{
    INT16	 data_len;
    char	*buf;
    int		 len;

    data_len = *((CARD16 *)recv_buf + 1) + XIM_HEADER_SIZE;

    if (recv_len < data_len) {
	*packet = (char *)NULL;
	*packet_size = (int)data_len;
	return True;
    }
    if (!(buf = (char *)Xmalloc(data_len)))
	return False;
    (void)memcpy(buf, recv_buf, data_len);
    *packet = buf;
    *packet_size = data_len;
    for (;data_len < recv_len; data_len++) {
	if (recv_buf[data_len])
	    break;
    }
    len = recv_len - data_len;
    if (len > 0) {
	if (!(buf = (char *)Xmalloc(len)))
	    return False;
	(void)memcpy(buf, &recv_buf[data_len], len);
	im->private.proto.hold_data = buf;
	im->private.proto.hold_data_len = len;
    }
    return True;
}

Private Bool
_XimReadData(im, len, data, arg)
    Xim		 im;
    INT16	*len;
    XPointer	*data;
    XPointer	 arg;
{
    char	 buf[BUFSIZE];
    char	*recv_buf;
    char	*alloc_buf = (char *)NULL;
    char	*tmp;
    int		 recv_point;
    int		 min_len;
    int		 buf_len;
    int		 ret_len;
    int		 packet_size;
    char	*packet;

    bzero(buf, sizeof(buf));
    recv_buf = buf;
    recv_point = 0;
    min_len = XIM_HEADER_SIZE;
    buf_len = BUFSIZE;

    if (im->private.proto.hold_data) {
	ret_len = im->private.proto.hold_data_len;
	if (ret_len > BUFSIZE) {
	    if (!(alloc_buf = (char *)Xmalloc(ret_len)))
		return False;
	    recv_buf = alloc_buf;
	    bzero(recv_buf, ret_len);
	    buf_len = ret_len;
	}
	recv_point = ret_len;
	(void)memcpy(recv_buf, im->private.proto.hold_data, ret_len);
	Xfree(im->private.proto.hold_data);
	im->private.proto.hold_data = NULL;
	im->private.proto.hold_data_len = 0;
	if (ret_len >= XIM_HEADER_SIZE) {
	    if (!(_CheckProtocolData(im, recv_buf, ret_len,
					 	&packet, &packet_size))) {
		if (alloc_buf)
		    Xfree(alloc_buf);
		return False;
	    }
	    if (packet) {
		*len = (INT16)packet_size;
		*data = (XPointer)packet;
		if (alloc_buf)
		    Xfree(alloc_buf);
	 	return True;
	    }
	    if (packet_size > buf_len) {
		if (!(tmp = (char *)Xmalloc(packet_size))) {
		    if (alloc_buf)
			Xfree(alloc_buf);
		    return False;
		}
		(void)memcpy(tmp, recv_buf, ret_len);
		recv_buf = tmp;
		if (alloc_buf)
		    Xfree(alloc_buf);
		alloc_buf = tmp;
		buf_len = packet_size;
	    }
	    min_len = packet_size;
	}
    }

    for (;;) {
	if (!(im->private.proto.recv(im, (XPointer)recv_buf, recv_point,
					min_len, buf_len, &ret_len, arg))) {
	    if (alloc_buf)
		Xfree(alloc_buf);
	    return False;
	}
	if (!(_CheckProtocolData(im, recv_buf, ret_len,
					 	&packet, &packet_size))) {
	    if (alloc_buf)
		Xfree(alloc_buf);
	    return False;
	}
	if (packet)
	    break;
	if (packet_size > buf_len) {
	    if (!(tmp = (char *)Xmalloc(packet_size))) {
		if (alloc_buf)
		    Xfree(alloc_buf);
		return False;
	    }
	    (void)memcpy(tmp, recv_buf, ret_len);
	    recv_buf = tmp;
	    if (alloc_buf)
		Xfree(alloc_buf);
	    alloc_buf = tmp;
	    buf_len = packet_size;
	}
	recv_point = ret_len;
	min_len = packet_size;
    }
    *len = (INT16)packet_size;
    *data = (XPointer)packet;
    if (alloc_buf)
	Xfree(alloc_buf);
    return True;
}

Private Bool
_XimIntrCallbackCheck(im, len, data)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
{
    return im->private.proto.check_cb(im, len, data);
}

Public Bool
_XimRecv(im, len, data, predicate, arg)
    Xim		 im;
    INT16	*len;
    XPointer	*data;
    Bool	 (*predicate)(
#if NeedNestedPrototypes
			      Xim, INT16, XPointer, XPointer
#endif
			      );
    XPointer	 arg;
{
    INT16	 read_len;
    XPointer	 read_buf;

    for (;;) {
	if (!(_XimReadData(im, &read_len, &read_buf, (XPointer)NULL)))
	    return False;
	if ((*predicate)(im, read_len, read_buf, arg))
	    break;
	if (_XimIntrCallbackCheck(im, read_len, read_buf))
	    continue;
	_XimError(im, 0, XIM_BadProtocol, (INT16)0, (CARD16)0, (char *)NULL);
	Xfree(read_buf);
    }
    *len = read_len;
    *data = read_buf;
    return True;
}

Public Bool
_XimIntrCallback(im, callback, call_data)
    Xim		 im;
    Bool	 (*callback)(
#if NeedNestedPrototypes
			     Xim, INT16, XPointer, XPointer
#endif
			     );
    XPointer	 call_data;
{
    return im->private.proto.intr_cb(im, callback, call_data);
}

Public void
_XimFlush(im)
    Xim		 im;
{
    im->private.proto.flush(im);
    return;
}

Public Bool
_XimFilterWaitEvent(im, arg)
    Xim		 im;
    XPointer	 arg;
{
    INT16	 read_len;
    XPointer	 read_buf;

    if (!(_XimReadData(im, &read_len, &read_buf, arg)))
	return False;
    if (_XimIntrCallbackCheck(im, read_len, read_buf))
	return True;
    _XimError(im, 0, XIM_BadProtocol, (INT16)0, (CARD16)0, (char *)NULL);
    Xfree(read_buf);
    return True;
}
