/* $XConsortium: imDefIm.c,v 1.3 93/09/24 10:29:54 rws Exp $ */
/******************************************************************

         Copyright 1990, 1991, 1992 by Sun Microsystems, Inc.
         Copyright 1992, 1993 by FUJITSU LIMITED
         Copyright 1993 by Sony Corporation

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
          Makoto Wakamatsu     Sony Corporation
                               makoto@sm.sony.co.jp

******************************************************************/

#include <X11/Xatom.h>
#define NEED_EVENTS
#include "Xlibint.h"
#include "Xlcint.h"
#include "XlcPublic.h"
#include "XimTrInt.h"
#include "Ximint.h"

Public int
_XimCheckDataSize(buf, len)
    XPointer	 buf;
    int		 len;
{
    CARD16	*buf_s = (CARD16 *)buf;

    if(len < XIM_HEADER_SIZE)
	return -1;
    return  buf_s[1];
}

Public void
#if NeedFunctionPrototypes
_XimSetHeader(
    XPointer	 buf,
    CARD8	 major_opcode,
    CARD8	 minor_opcode,
    INT16	*len
)
#else
_XimSetHeader(buf, major_opcode, minor_opcode, len)
    XPointer	 buf;
    CARD8	 major_opcode;
    CARD8	 minor_opcode;
    INT16	*len;
#endif /* NeedFunctionPrototypes */
{
    CARD8	*buf_b = (CARD8 *)buf;
    CARD16	*buf_s = (CARD16 *)buf;

    buf_b[0] = major_opcode;
    buf_b[1] = minor_opcode;
    buf_s[1] = *len;
    *len += XIM_HEADER_SIZE;
    return;
}

Private char
_XimGetMyEndian()
{
    CARD16	 test_card = 1;

    if(*((char *)&test_card))
	return LITTLEENDIAN;
    else
	return BIGENDIAN;
}

Private Bool
_XimCheckServerName(im, str)
    Xim		   im;
    char	  *str;
{
    char	  *server_name = im->core.im_name;
    int		   len;
    int		   str_len;
    int		   category_len = strlen(XIM_SERVER_CATEGORY);
    char	  *pp;
    register char *p;

    if(server_name && *server_name)
	len = strlen(server_name);
    else
	return True;

    if((int)strlen(str) < category_len)
	return False;

    if(strncmp(str, XIM_SERVER_CATEGORY, category_len))
	return False;
 
    pp = &str[category_len];

    for(;;) {
	for(p = pp; (*p != ',') && (*p); p++);
	str_len = (int)(p - pp);

	if((len == str_len) && (!strncmp(pp, server_name, len)))
	    break;
	if(!(*p))
	    return False;
	pp = p + 1;
    }
    return True;
}

Private char *
_XimCheckLocaleName(im, address, address_len, locale_name, len)
    Xim		   im;
    char	  *address;
    int		   address_len;
    char	  *locale_name[];
    int		   len;
{
    int		   category_len;
    char	  *pp;
    register char *p;
    register int   n;

    category_len = strlen(XIM_LOCAL_CATEGORY);
    if(address_len < category_len)
	return False;

    if(strncmp(address, XIM_LOCAL_CATEGORY, category_len))
	return False;
 
    pp = &address[category_len];

    for(;;) {
	for (p = pp; (*p != ',') && (*p); p++);
	*p = '\0';
	address_len = (int)(p - pp);

	for( n = 0; n < len; n++ )
	    if( !strcmp( pp, locale_name[n] ) )
		return locale_name[n];
	if (!(*p))
	    break;
	pp = p + 1;
    }
    return (char *)NULL;
}

Private Bool
_XimCheckTransport(address, address_len, transport, len, trans_addr)
    char	  *address;
    int		   address_len;
    char	  *transport;
    int		   len;
    char	 **trans_addr;
{
    int		   category_len = strlen(XIM_TRANSPORT_CATEGORY);
    char	  *pp;
    register char *p;

    if(address_len < category_len)
	return False;

    if(strncmp(address, XIM_TRANSPORT_CATEGORY, category_len))
	return False;
 
    pp = &address[category_len];

    for(;;) {
	for(p = pp; (*p != '/') && (*p != ',') && (*p); p++);
	if(*p == ',') {
	    pp = p + 1;
	    continue;
	}
	if(!(*p))
	    return False;

	address_len = (int)(p - pp);

	if((len == address_len) && (!strncmp(pp, transport, len)))
	    break;
	pp = p + 1;
    }
    pp = p + 1;
    for(p = pp; (*p != ',') && (*p); p++);
    *p = '\0';
    *trans_addr = pp;
    return True;
}

Private Bool
_CheckSNEvent(display, xevent, arg)
    Display		*display;
    XEvent		*xevent;
    XPointer		 arg;
{
    XSelectionEvent	*event = (XSelectionEvent *)xevent;
    Window		 window = *(Window*)arg;

    if((event->type == SelectionNotify) && (window == event->requestor))
	return True;
    return False;
}

Private Bool
_XimGetSelectionNotify(display, window, target, ret_address)
    Display		 *display;
    Window		  window;
    Atom		  target;
    char		**ret_address;
{
    XEvent		  event;
    XSelectionEvent	 *ev = (XSelectionEvent *)&event;
    Atom		  actual_type;
    int			  actual_format;
    unsigned long	  nitems, bytes_after;

    for(;;) {
	XIfEvent(display, &event, _CheckSNEvent, (XPointer)&window);
	if((ev->type == SelectionNotify) && (window == ev->requestor))
	    break;
    }

    if(ev->property == (Atom)None)
	return False;
    if( XGetWindowProperty( display, window, target, 0L, 1000000L,
			    True, target, &actual_type, &actual_format,
			    &nitems, &bytes_after,
			    (unsigned char **)&*ret_address ) != Success )
	return False;
    return True;
}

Private Bool
_XimPreConnectionIM(im, selection)
    Xim			 im;
    Atom		 selection;
{
    Display		*display = im->core.display; 
    Atom		 locales, transport;
    char		*address;
    XLCd		 lcd;
    char		*language;
    char		*territory;
    char		*codeset;
    char		*trans_addr;
    char		*locale_name[4], *locale;
    int			 llen, tlen, clen;
    register int	 i;
    Window		 window;
    char		*str;

    if(!(lcd = im->core.lcd))
	return False;

    /* requestor window */
    if(!(window = XCreateSimpleWindow(display, DefaultRootWindow(display),
			 				0, 0, 1, 1, 1, 0, 0)))
	return False;

    /* server name check */
    if( !(str = XGetAtomName( display, selection )) )
	return False;
    if(!_XimCheckServerName(im, str)) {
	XFree( (XPointer)str );
	goto Error;
    }
    XFree( (XPointer)str );

    /* locale name check */
    _XGetLCValues(lcd, XlcNLanguage, &language, XlcNTerritory, &territory,
                    XlcNCodeset, &codeset, NULL);
    llen = strlen( language );
    tlen = strlen( territory );
    clen = strlen( codeset );

    for( i = 0; i < 4; i++ )
	locale_name[i] = NULL;
    if( tlen != 0  &&  clen != 0 ) {
	if( (locale_name[0] = Xmalloc(llen+tlen+clen+3)) != NULL )
	    sprintf( locale_name[0], "%s_%s.%s", language, territory, codeset );
    }
    if( clen != 0 ) {
	if( (locale_name[1] = Xmalloc(llen+clen+2)) != NULL )
	    sprintf( locale_name[1], "%s.%s", language, codeset );
	else
	    goto Error;
    }
    if( tlen != 0 ) {
	if( (locale_name[2] = Xmalloc(llen+tlen+2)) != NULL )
	    sprintf( locale_name[2], "%s_%s", language, territory );
	else
	    goto Error;
    }
    if( (locale_name[3] = Xmalloc(llen+1)) != NULL )
	strcpy( locale_name[3], language );
    else
	goto Error;
    if((locales = XInternAtom(display, XIM_LOCALES, True)) == (Atom)None)
	goto Error;

    XConvertSelection(display, selection, locales, locales, window,
		      CurrentTime);
    if(!(_XimGetSelectionNotify(display, window, locales, &address)))
	goto Error;

    if((locale = _XimCheckLocaleName(im, address, strlen(address), locale_name,
				     4)) == NULL) {
	XFree((XPointer)address);
	goto Error;
    }
    im->private.proto.locale_name = locale;
    for( i = 0; i < 4; i++ ) {
	if( locale_name[i] != NULL  &&  locale_name[i] != locale ) {
	    XFree( locale_name[i] );
	    locale_name[i] = NULL;
	}
    }
    XFree((XPointer)address);

    /* transport check */
    if((transport = XInternAtom(display, XIM_TRANSPORT, True)) == (Atom)None)
	goto Error;

    XConvertSelection(display, selection, transport, transport, window,
		      CurrentTime);
    if(!_XimGetSelectionNotify(display, window, transport, &address))
	goto Error;

    for(i = 0; _XimTransportRec[i].transportname ; i++) {
	if( _XimCheckTransport(address, strlen(address),
				_XimTransportRec[i].transportname,
				_XimTransportRec[i].namelen, &trans_addr)) {
	    if( _XimTransportRec[i].config(im, trans_addr) ) {
		XFree((XPointer)address);
		XDestroyWindow(display, window);
		return True;
	    }
	}
    }

    XFree((XPointer)address);
Error:
    for( i = 0; i < 4; i++ )
	if( locale_name[i] != NULL )
	    XFree( locale_name[i] );
    XDestroyWindow(display, window);
    return False; 
}

Private Bool
_XimPreConnect(im)
    Xim		    im;
{
    Display	   *display = im->core.display; 
    Atom	    imserver;
    Atom	    actual_type;
    int		    actual_format;
    unsigned long   nitems;
    unsigned long   bytes_after;
    unsigned char  *prop_return;
    Atom	   *atoms;
    Window	    im_window;
    register int    i;

    if((imserver = XInternAtom(display, XIM_SERVERS, True)) == (Atom)None)
	return False;

    if(XGetWindowProperty(display, RootWindow(display, DefaultScreen(display)),
			imserver, 0L, 1000000L, False, XA_ATOM, &actual_type, 
			&actual_format, &nitems, &bytes_after,
			&prop_return) != Success)
	return False;

    if( (actual_type != XA_ATOM) || (actual_format != 32) ) {
	if( nitems )
	    XFree((XPointer)prop_return);
	return False;
    }

    atoms = (Atom *)prop_return;
    for(i = 0; i < nitems; i++) {
	if((im_window = XGetSelectionOwner(display, atoms[i])) == (Window)None)
	    continue;

	if(_XimPreConnectionIM(im, atoms[i]))
	    break;
    }

    XFree((XPointer)prop_return);
    if(i >= nitems)
	return False;

    im->private.proto.im_window = im_window;
    return True;
}

Private Bool
_XimGetAuthProtocolNames(im, buf, num, len)
    Xim		 im;
    CARD16	*buf;
    CARD8	*num;
    INT16	*len;
{
    if (!IS_USE_AUTHORIZATION_FUNC(im)) {
	*num = 0;
	*len = 0;
	return True;
    }
    /*
     * Not yet
     */
    return True;
}

Private Bool
_XimSetAuthReplyData(im, buf, len)
    Xim		 im;
    XPointer	 buf;
    INT16	*len;
{
    /*
    * Not yet
    */
    *len = 0;
    return True;
}

Private Bool
_XimSetAuthNextData(im, buf, len)
    Xim		 im;
    XPointer	 buf;
    INT16	*len;
{
    /*
    * Not yet
    */
    *len = 0;
    return True;
}

Private Bool
_XimSetAuthRequiredData(im, buf, len)
    Xim		 im;
    XPointer	 buf;
    INT16	*len;
{
    /*
    * Not yet
    */
    *len = 0;
    return True;
}

Private Bool
_XimCheckAuthSetupData(im, buf)
    Xim		 im;
    XPointer	 buf;
{
    /*
    * Not yet
    */
    return True;
}

Private Bool
_XimCheckAuthNextData(im, buf)
    Xim		 im;
    XPointer	 buf;
{
    /*
    * Not yet
    */
    return True;
}

#define	NO_MORE_AUTH	2
#define	GOOD_AUTH	1
#define	BAD_AUTH	0

Private int
_XimClientAuthCheck(im, buf)
    Xim		 im;
    XPointer	 buf;
{
    /*
    * Not yet
    */
    return NO_MORE_AUTH;
}

Private void
_XimAuthNG(im)
    Xim		 im;
{
    CARD8	 buf[BUFSIZE];
    INT16	 len = 0;

    _XimSetHeader((XPointer)buf, XIM_AUTH_NG, 0, &len);
    (void)_XimSend(im, len, (XPointer)buf);
    _XimFlush(im);
    return;
}

Private	Bool
#if NeedFunctionPrototypes
_XimAllRecv(
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 arg)
#else
_XimAllRecv(im, len, data, arg)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
    XPointer	 arg;
#endif
{
    return True;
}

#define	CLIENT_WAIT1		1
#define	CLIENT_WAIT2		2

Private Bool
_XimConnection(im)
    Xim		 im;
{
    CARD8	 buf[BUFSIZE];
    CARD8	*buf_b = &buf[XIM_HEADER_SIZE];
    CARD16	*buf_s = (CARD16 *)((XPointer)buf_b);
    INT16	 len;
    CARD8	 num;
    XPointer	 reply;
    CARD8	 major_opcode;
    int		 wait_mode;
    int		 ret;

    if(!(_XimConnect(im)))	/* Transport Connect */
	return False;

    if(!_XimDispatchInit(im))
	return False;

    _XimRegProtoIntrCallback(im, XIM_ERROR, 0, _XimErrorCallback, (XPointer)im);

    if(!_XimGetAuthProtocolNames(im, &buf_s[4], &num, &len))
	return False;

    buf_b[0] = _XimGetMyEndian();	/* byte order */
    buf_b[1] = 0;			/* unused */
    buf_s[1] = PROTOCOLMAJORVERSION;	/* major-version */
    buf_s[2] = PROTOCOLMINORVERSION;	/* minor-version */
    buf_s[3] = num;			/* number of auth-protocol-names */
    len  += sizeof(CARD8)		/* sizeof byte order */
          + sizeof(CARD8)		/* sizeof unused */
          + sizeof(CARD16)		/* sizeof major-version */
          + sizeof(CARD16)		/* sizeof minor-version */
          + sizeof(CARD16);		/* sizeof number of auth-names */

    major_opcode = XIM_CONNECT;
    wait_mode = (IS_USE_AUTHORIZATION_FUNC(im)) ? CLIENT_WAIT1 : CLIENT_WAIT2;

    for(;;) {
	_XimSetHeader((XPointer)buf, major_opcode, 0, &len);
	if (!(_XimSend(im, len, (XPointer)buf)))
	    return False;
	_XimFlush(im);
	if (!(_XimRecv(im, &len, &reply, _XimAllRecv, 0)))
	    return False;

	major_opcode = *((CARD8 *)reply);
	buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);

	if (wait_mode == CLIENT_WAIT1) {
	    if (major_opcode == XIM_AUTH_REQUIRED) {
		ret = _XimClientAuthCheck(im, (XPointer)buf_s);
		Xfree(reply);
		if (ret == NO_MORE_AUTH) {
		    if (!(_XimSetAuthReplyData(im,
				(XPointer)&buf[XIM_HEADER_SIZE], &len))) {
			_XimAuthNG(im);
			return False;
		    }
		    major_opcode = XIM_AUTH_REPLY;
		    wait_mode = CLIENT_WAIT2;
		} else if (ret == GOOD_AUTH) {
		    if (!(_XimSetAuthNextData(im,
				(XPointer)&buf[XIM_HEADER_SIZE], &len))) {
			_XimAuthNG(im);
			return False;
		    }
		    major_opcode = XIM_AUTH_NEXT;
		} else {	/* BAD_AUTH */
		    _XimAuthNG(im);
		    return False;
		}
	    } else {
		Xfree(reply);
		_XimAuthNG(im);
		return False;
	    }
	} else {	/* CLIENT_WAIT2 */
	    if (major_opcode == XIM_CONNECT_REPLY) {
		break;
	    } else if (major_opcode == XIM_AUTH_SETUP) {
		if (!(_XimCheckAuthSetupData(im, (XPointer)buf_s))) {
		    _XimAuthNG(im);
		    return False;
		}
		Xfree(reply);
		if (!(_XimSetAuthRequiredData(im,
				(XPointer)&buf[XIM_HEADER_SIZE], &len))) {
		    _XimAuthNG(im);
		    return False;
		}
		major_opcode = XIM_AUTH_REQUIRED;
	    } else if (major_opcode == XIM_AUTH_NEXT) {
		if (!(_XimCheckAuthNextData(im, (XPointer)buf_s))) {
		    _XimAuthNG(im);
		    return False;
		}
		Xfree(reply);
		if (!(_XimSetAuthRequiredData(im,
				(XPointer)&buf[XIM_HEADER_SIZE], &len))) {
		    _XimAuthNG(im);
		    return False;
		}
		major_opcode = XIM_AUTH_REQUIRED;
	    } else if (major_opcode == XIM_AUTH_NG) {
		Xfree(reply);
		return False;
	    } else {
		_XimAuthNG(im);
		Xfree(reply);
		return False;
	    }
	}
    }

    if (!( buf_s[0] == PROTOCOLMAJORVERSION 
        && buf_s[1] == PROTOCOLMINORVERSION)) {
	Xfree(reply);
	return False;
    }
    Xfree(reply);
    MARK_SERVER_CONNECTED(im);

    _XimRegProtoIntrCallback(im, XIM_REGISTER_TRIGGERKEYS, 0,
				 _XimRegisterTriggerKeysCallback, (XPointer)im);
    return True;
}

Private	Bool
#if NeedFunctionPrototypes
_XimDisconnectCheck(
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 arg)
#else
_XimDisconnectCheck(im, len, data, arg)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
    XPointer	 arg;
#endif
{
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);

    if ((major_opcode == XIM_DISCONNECT_REPLY)
     && (minor_opcode == 0))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0))
	return True;
    return False;
}

Private Bool
_XimDisconnect(im)
    Xim		 im;
{
    CARD8	 buf[BUFSIZE];
    INT16	 len = 0;
    XPointer	 reply;

    if (IS_SERVER_CONNECTED(im)) {
	_XimSetHeader((XPointer)buf, XIM_DISCONNECT, 0, &len);
	if (!(_XimSend(im, len, (XPointer)buf)))
	    return False;
	_XimFlush(im);
	if (!(_XimRecv(im, &len, &reply, _XimDisconnectCheck, 0)))
	    return False;
	Xfree(reply);
    }

    if (!(_XimShutdown(im)))	/* Transport shutdown */
	return False;

    return True;
}

Private	Bool
#if NeedFunctionPrototypes
_XimOpenCheck(
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 arg)
#else
_XimOpenCheck(im, len, data, arg)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
    XPointer	 arg;
#endif
{
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);

    if ((major_opcode == XIM_OPEN_REPLY)
     && (minor_opcode == 0))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0))
	return True;
    return False;
}

Private Bool
_XimOpen(im)
    Xim		 im;
{
    CARD8	 buf[BUFSIZE];
    CARD8	*buf_b = &buf[XIM_HEADER_SIZE];
    CARD16	*buf_s;
    INT16	 len;
    XPointer	 reply;
    char	*language;
    char	*territory;
    char	*codeset;
    char	*locale_name;

    locale_name = im->private.proto.locale_name;
    len = strlen(locale_name);
    buf_b[0] = (BYTE)len;			   /* length of locale name */
    (void)strcpy((char *)&buf_b[1], locale_name);  /* locale name */
    len += sizeof(BYTE);			   /* sizeof length */
    XIM_SET_PAD(buf_b, len);			   /* pad */

    _XimSetHeader((XPointer)buf, XIM_OPEN, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf)))
	return False;
    _XimFlush(im);
    if (!(_XimRecv(im, &len, &reply, _XimOpenCheck, 0)))
	return False;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return False;
    }

    im->private.proto.imid = buf_s[0];		/* imid */

    if (!(_XimGetAttributeID(im, &buf_s[1]))) {
	Xfree(reply);
	return False;
    }
    Xfree(reply);

    if (!(_XimSetInnerIMResourceList(&(im->private.proto.im_inner_resources),
				&(im->private.proto.im_num_inner_resources))))
	return False;

    if (!(_XimSetInnerICResourceList(&(im->private.proto.ic_inner_resources),
				&(im->private.proto.ic_num_inner_resources))))
	return False;

    _XimSetIMMode(im->core.im_resources, im->core.im_num_resources);
    _XimSetIMMode(im->private.proto.im_inner_resources,
				im->private.proto.im_num_inner_resources);

    /* Transport Callbak */
    _XimRegProtoIntrCallback(im, XIM_SET_EVENT_MASK, 0,
				 _XimSetEventMaskCallback, (XPointer)im);
    _XimRegProtoIntrCallback(im, XIM_FORWARD_EVENT, 0,
				 _XimForwardEventCallback, (XPointer)im);
    _XimRegProtoIntrCallback(im, XIM_COMMIT, 0,
				 _XimCommitCallback, (XPointer)im);
    _XimRegProtoIntrCallback(im, XIM_SYNC, 0,
				 _XimSyncCallback, (XPointer)im);

    if(!_XimExtension(im))
	return False;

    /* register a hook for callback protocols */
    _XimIntrCallback(im, _XimCbDispatch, (XPointer)im);

    return True;
}

Private	Bool
#if NeedFunctionPrototypes
_XimCloseCheck(
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 arg)
#else
_XimCloseCheck(im, len, data, arg)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
    XPointer	 arg;
#endif
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_CLOSE_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Private Bool
_XimClose(im)
    Xim		 im;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;

    if (!IS_SERVER_CONNECTED(im))
	return False;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = 0;				/* unused */
    len = sizeof(CARD16)			/* sizeof imid */
        + sizeof(CARD16);			/* sizeof unused */
  
    _XimSetHeader((XPointer)buf, XIM_CLOSE, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf)))
	return False;
    _XimFlush(im);
    if (!(_XimRecv(im, &len, &reply, _XimCloseCheck, 0)))
	return False;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return False;
    }

    Xfree(reply);
    return True;
}

Public void
_XimProtoIMFree(im)
    Xim		  im;
{
    /* XIMPrivateRec */
    if(im->private.proto.im_onkeylist)
	Xfree(im->private.proto.im_onkeylist);
    if(im->private.proto.im_offkeylist)
	Xfree(im->private.proto.im_offkeylist);
    if(im->private.proto.default_styles)
	Xfree(im->private.proto.default_styles);
    if(im->private.proto.im_attribute_name)
	Xfree(im->private.proto.im_attribute_name);
    if(im->private.proto.intrproto)
	_XimFreeProtoIntrCallback(im);
    if(im->private.proto.im_inner_resources)
	Xfree(im->private.proto.im_inner_resources);
    if(im->private.proto.ic_inner_resources)
	Xfree(im->private.proto.ic_inner_resources);
    if(im->private.proto.ctom_conv)
	_XlcCloseConverter(im->private.proto.ctom_conv);
    if(im->private.proto.ctow_conv)
	_XlcCloseConverter(im->private.proto.ctow_conv);
    if(im->private.proto.locale_name)
	Xfree(im->private.proto.locale_name);

    /* core */
    if(im->core.res_name)
        Xfree(im->core.res_name);
    if(im->core.res_class)
        Xfree(im->core.res_class);
    if(im->core.extensions)
        Xfree(im->core.extensions);
    if(im->core.options)
        Xfree(im->core.options);
    if(im->core.icattributes)
        Xfree(im->core.icattributes);
    if(im->core.im_name)
	Xfree(im->core.im_name);
    if(im->core.styles)
	Xfree(im->core.styles);
    if(im->core.im_resources)
        Xfree(im->core.im_resources);
    if(im->core.ic_resources)
        Xfree(im->core.ic_resources);

    return;
}

Private Status
_XimProtoCloseIM(xim)
    XIM		 xim;
{
    Xim		 im = (Xim)xim;

    while (im->core.ic_chain)
	XDestroyIC(im->core.ic_chain);
    _XimResetIMInstantiateCallback(im);
    (void)_XimClose(im);
    (void)_XimDisconnect(im);
    _XimUnregisterServerFilter(im);
    _XimProtoIMFree(im);
    _XimDestroyIMStructureList(im);
    return 0;
}

Private Bool
#if NeedFunctionPrototypes
_XimSetIMValuesCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimSetIMValuesCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_SET_IM_VALUES_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Private char *
_XimProtoSetIMValues(xim, arg)
    XIM			 xim;
    XIMArg		*arg;
{
    Xim			 im = (Xim)xim;
    char		*attr_buf;
    int			 buf_len;
    CARD8		*buf;
    CARD16		*buf_s;
    INT16		 len;
    XPointer		 reply;
    char		*name;
    
    name = _XimEncodeIMATTRIBUTE(im, im->core.im_resources,
				im->core.im_num_resources, arg,
				&attr_buf, &len, XIM_SETIMVALUES);

    if (!len)
	return name;

    buf_len = XIM_HEADER_SIZE
	    + sizeof(CARD16)
	    + sizeof(INT16)
	    + len;

    if (!(buf = (CARD8 *)Xmalloc(buf_len))) {
	Xfree(attr_buf);
	return arg->name;
    }
    buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = len;				/* length of im-attr-id */
    (void)memcpy((char *)&buf_s[2], attr_buf, len);
    Xfree(attr_buf);
    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(INT16);			/* sizeof length of attr */

    _XimSetHeader((XPointer)buf, XIM_SET_IM_VALUES, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf))) {
	Xfree(buf);
	return arg->name;
    }
    _XimFlush(im);
    Xfree(buf);
    if (!(_XimRecv(im, &len, &reply, _XimSetIMValuesCheck, 0)))
	return arg->name;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return arg->name;
    }
    Xfree(reply);

    return name;
}

Private Bool
#if NeedFunctionPrototypes
_XimGetIMValuesCheck(
    Xim          im,
    INT16        len,
    XPointer	 data,
    XPointer     arg)
#else
_XimGetIMValuesCheck(im, len, data, arg)
    Xim          im;
    INT16        len;
    XPointer	 data;
    XPointer     arg;
#endif
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_GET_IM_VALUES_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Private char *
_XimProtoGetIMValues(xim, arg)
    XIM			 xim;
    XIMArg		*arg;
{
    Xim			 im = (Xim)xim;
    register XIMArg	*p;
    register int	 n;
    int			 buf_len;
    CARD8		*buf;
    CARD16		*buf_s;
    INT16		 len;
    XPointer		 reply;
    char		*encode_name;
    char		*decode_name;

    for (n = 0, p = arg; p->name; p++)
	n++;

    if (!n)
	return (char *)NULL;

    buf_len =  sizeof(CARD16) * n;
    buf_len += XIM_HEADER_SIZE
	     + sizeof(CARD16)
	     + sizeof(INT16)
	     + XIM_PAD(buf_len);

    if (!(buf = (CARD8 *)Xmalloc(buf_len)))
	return arg->name;
    buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];

    encode_name = _XimEncodeIMAttrIDList(im, im->core.im_resources,
				im->core.im_num_resources, arg,
				&buf_s[2], &len, XIM_GETIMVALUES);

    if (!len)
	return encode_name;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = len;				/* length of im-attr-id */
    XIM_SET_PAD(&buf_s[2], len);		/* pad */
    len += sizeof(CARD16)			/* sizeof imid */
	 + sizeof(INT16);			/* sizeof length of attr */

    _XimSetHeader((XPointer)buf, XIM_GET_IM_VALUES, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf))) {
	Xfree(buf);
	return arg->name;
    }
    _XimFlush(im);
    Xfree(buf);
    if (!(_XimRecv(im, &len, &reply, _XimGetIMValuesCheck, 0)))
	return arg->name;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return arg->name;
    }

    decode_name = _XimDecodeIMATTRIBUTE(im, im->core.im_resources,
			im->core.im_num_resources, &buf_s[2], buf_s[1],
			arg, XIM_GETIMVALUES);
    Xfree(reply);

    if (decode_name)
	return decode_name;
    else
	return encode_name;
}

Private XIMMethodsRec     im_methods = {
    _XimProtoCloseIM,           /* close */
    _XimProtoSetIMValues,       /* set_values */
    _XimProtoGetIMValues,       /* get_values */
    _XimProtoCreateIC,          /* create_ic */
};

Private Bool
_XimSetEncodingByName(im, buf, len)
    Xim		  im;
    char	**buf;
    int		 *len;
{
    char	*encoding = (char *)NULL;
    int		 encoding_len;
    int		 compound_len;
    BYTE	*ret;

    _XGetLCValues(im->core.lcd, XlcNCodeset, &encoding, NULL);
    if (!encoding) {
	*buf = (char *)NULL;
	*len = 0;
	return True;
    }
    encoding_len = strlen(encoding);
    compound_len = strlen("COMPOUND_TEXT");
    *len = encoding_len + sizeof(BYTE) + compound_len + sizeof(BYTE);
    if (!(ret = (BYTE *)Xmalloc(*len))) {
	return False;
    }
    *buf = (char *)ret;

    ret[0] = (BYTE)encoding_len;
    (void)strncpy((char *)&ret[1], encoding, encoding_len);
    ret += (encoding_len + sizeof(BYTE));
    ret[0] = (BYTE)compound_len;
    (void)strncpy((char *)&ret[1], "COMPOUND_TEXT", compound_len);
    return True;
}

Private Bool
_XimSetEncodingByDetail(im, buf, len)
    Xim		 im;
    char	**buf;
    int		 *len;
{
    *len = 0;
    *buf = NULL;
    return True;
}

Private Bool
_XimGetEncoding(im, buf, name, name_len, detail, detail_len)
    Xim		 im;
    CARD16	*buf;
    char	*name;
    int		 name_len;
    char	*detail;
    int		 detail_len;
{
    XLCd	 lcd = im->core.lcd;
    CARD16	 category = buf[0];
    CARD16	 idx = buf[1];
    int		 len;
    XlcConv	 ctom_conv;
    XlcConv	 ctow_conv;

    if (idx == (CARD16)XIM_Default_Encoding_IDX) { /* XXX */
	if (!(ctom_conv = _XlcOpenConverter(lcd,
				 XlcNCompoundText, lcd, XlcNMultiByte)))
	    return False;
	if (!(ctow_conv = _XlcOpenConverter(lcd,
				 XlcNCompoundText, lcd, XlcNWideChar)))
	    return False;
    }

    if (category == XIM_Encoding_NameCategory) {
	while (name_len > 0) {
	    len = (int)name[0];
	    if (!strncmp(&name[1], "COMPOUND_TEXT", len)) {
		if (!(ctom_conv = _XlcOpenConverter(lcd,
				 XlcNCompoundText, lcd, XlcNMultiByte)))
		    return False;
		if (!(ctow_conv = _XlcOpenConverter(lcd,
				 XlcNCompoundText, lcd, XlcNWideChar)))
		    return False;
		break;
	    } else {
		/*
		 * Not yet
		 */
	    }
	    len += sizeof(BYTE);
	    name_len -= len;
	    name += len;
	}
    } else if (category == XIM_Encoding_DetailCategory) {
	/*
	 * Not yet
	 */
    } else {
	return False;
    }
    im->private.proto.ctom_conv = ctom_conv;
    im->private.proto.ctow_conv = ctow_conv;
    return True;
}

Private	Bool
#if NeedFunctionPrototypes
_XimEncodingNegoCheck(
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 arg)
#else
_XimEncodingNegoCheck(im, len, data, arg)
    Xim		 im;
    INT16	 len;
    XPointer	 data;
    XPointer	 arg;
#endif
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_ENCODING_NEGOTIATION_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    if ((major_opcode == XIM_ERROR)
     && (minor_opcode == 0)
     && (buf_s[2] & XIM_IMID_VALID)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Private Bool
_XimEncodingNegitiation(im)
    Xim		 im;
{
    char	*name_ptr = 0;
    int		 name_len = 0;
    char	*detail_ptr = 0;
    int		 detail_len = 0;
    CARD8	*buf;
    CARD16	*buf_s;
    INT16	 len;
    XPointer	 reply;

    if (!(_XimSetEncodingByName(im, &name_ptr, &name_len)))
	return False;

    if (!(_XimSetEncodingByDetail(im, &detail_ptr, &detail_len))) {
	if (name_ptr)
	    Xfree(name_ptr);
	return False;
    }

    len = sizeof(CARD16)
	+ sizeof(INT16)
	+ name_len
	+ XIM_PAD(name_len)
	+ sizeof(INT16)
	+ sizeof(CARD16)
	+ detail_len;

    if (!(buf = (CARD8 *)Xmalloc(XIM_HEADER_SIZE + len))) {
	if (name_ptr)
	    Xfree(name_ptr);
	if (detail_ptr)
	    Xfree(detail_ptr);
	return False;
    }
    buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];

    buf_s[0] = im->private.proto.imid;
    buf_s[1] = (INT16)name_len;
    if (name_ptr)
	(void)memcpy((char *)&buf_s[2], name_ptr, name_len);
    XIM_SET_PAD(&buf_s[2], name_len);
    buf_s = (CARD16 *)((char *)&buf_s[2] + name_len);
    buf_s[0] = detail_len;
    buf_s[1] = 0;
    if (detail_ptr)
	(void)memcpy((char *)&buf_s[2], detail_ptr, detail_len);

    _XimSetHeader((XPointer)buf, XIM_ENCODING_NEGOTIATION, 0, &len);
    if (!(_XimSend(im, len, (XPointer)buf))) {
	Xfree(buf);
	return False;
    }
    _XimFlush(im);
    Xfree(buf);
    if (!(_XimRecv(im, &len, &reply, _XimEncodingNegoCheck, 0)))
	return False;
    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    if (*((CARD8 *)reply) == XIM_ERROR) {
	_XimProcError(im, 0, (XPointer)&buf_s[3]);
	Xfree(reply);
	return False;
    }

    if (!(_XimGetEncoding(im, &buf_s[1], name_ptr, name_len,
						detail_ptr, detail_len))) {
	Xfree(reply);
	return False;
    }
    if (name_ptr)
	Xfree(name_ptr);
    if (detail_ptr)
	Xfree(detail_ptr);

    Xfree(reply);

    return True;
}

Public Bool
_XimProtoOpenIM(im)
    Xim		 im;
{
    _XimInitialResourceInfo();

    im->methods = &im_methods;

    if (_XimPreConnect(im)
     && _XimConnection(im)
     && _XimOpen(im)
     && _XimEncodingNegitiation(im)) {
	_XimRegisterServerFilter(im);
	return True;
    } else if (IS_DELAYBINDABLE(im)) {
	/* Not yet
	 * return True;
	 */
	return False;
    }
    return False;
}
