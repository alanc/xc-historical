/* $XConsortium: imExten.c,v 1.1 93/09/17 13:26:27 rws Exp $ */
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
#include "Xlcint.h"
#include "Ximint.h"

/*
 * index of extensions
 */

#define	XIM_EXT_SET_EVENT_MASK_IDX	0
#ifndef NOT_EXT_FORWARD
#define	XIM_EXT_FORWARD_KEYEVENT_IDX	1
#endif
#ifndef NOT_EXT_MOVE
#define	XIM_EXT_MOVE_IDX		2
#endif

typedef struct	_XIM_QueryExtRec {
    Bool	 is_support;
    char	*name;
    INT16	 name_len;
    CARD16	 major_opcode;
    CARD16	 minor_opcode;
    int		 idx;
} XIM_QueryExtRec;

Private XIM_QueryExtRec	extensions[] = {
	{False, "XIM_EXT_SET_EVENT_MASK", 0, 0, 0,
					XIM_EXT_SET_EVENT_MASK_IDX}, 
#ifndef NOT_EXT_FORWARD
	{False, "XIM_EXT_FORWARD_KEYEVENT", 0, 0, 0,
					XIM_EXT_FORWARD_KEYEVENT_IDX},
#endif
#ifndef NOT_EXT_MOVE
	{False, "XIM_EXT_MOVE", 0, 0, 0, XIM_EXT_MOVE_IDX},
#endif
	{False, NULL, 0, 0, 0, 0}
};

Private int
_XimIsSupportExt(idx)
    int		 idx;
{
    register int i;

    for (i = 0; extensions[i].name; i++) {
	if (extensions[i].idx == idx)
	    if (extensions[idx].is_support)
		return i;
	    else
		break;
    }
    return -1;
}

Private Bool
_XimProcExtSetEventMask(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    EVENTMASK	*buf_l = (EVENTMASK *)buf;
    EVENTMASK	 mask = _XimGetWindowEventmask(ic);
    EVENTMASK	 select_mask = mask;
    EVENTMASK	 target_mask = XIM_FORWARD_EVENT_MASKS; /* XXX */

    ic->private.proto.filter_event_mask      = buf_l[0];
    ic->private.proto.intercept_event_mask   = buf_l[1];
    ic->private.proto.select_event_mask      = buf_l[2];
    ic->private.proto.forward_event_mask     = buf_l[3];
    ic->private.proto.synchronous_event_mask = buf_l[4];

    select_mask &= ~ic->private.proto.intercept_event_mask;
						/* deselected event mask */
    select_mask |= ic->private.proto.select_event_mask;
						/* selected event mask */
    mask = (mask & ~target_mask) | (select_mask & target_mask);

    XSelectInput(im->core.display, ic->core.focus_window, mask);
    if (!(_XimProcSyncReply(im, ic)))
	return False;
    return True;
}

Private Bool
#if NeedFunctionPrototypes
_XimExtSetEventMaskCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimExtSetEventMaskCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if ((imid == im->private.proto.imid)
     && (ic = _XimICOfXICID(im, icid))) {
	(void)_XimProcExtSetEventMask(im, ic, (XPointer)&buf_s[2]);
	Xfree(data);
	return True;
    }
    return False;
}

#ifndef NOT_EXT_FORWARD
Private Bool
_XimProcExtForwardKeyEvent(im, ic, buf)
    Xim		 im;
    Xic		 ic;
    XPointer	 buf;
{
    CARD8	*buf_b = (CARD8 *)buf;
    CARD16	*buf_s = (CARD16 *)buf;
    CARD32	*buf_l = (CARD32 *)buf;
    XEvent	 ev;
    XKeyEvent	*kev = (XKeyEvent *)&ev;

    bzero(&ev, sizeof(XEvent));
    kev->send_event	= False;
    kev->display	= im->core.display;
    kev->serial		= buf_s[1];		/* sequence number */
    kev->type		= buf_b[4] & 0x7f;	/* xEvent.u.u.type */
    kev->keycode	= buf_b[5];		/* Keycode */
    kev->state		= buf_s[3];		/* state */
    kev->time		= buf_l[2];		/* time */
    kev->window		= buf_l[3];		/* window XXX */

    XPutBackEvent(im->core.display, &ev);
    MARK_NEED_SYNC_REPLY(ic);

    MARK_FABLICATED(ic);

    return True;
}

Private Bool
#if NeedFunctionPrototypes
_XimExtForwardKeyEventCallback(
    Xim		 xim,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data)
#else
_XimExtForwardKeyEventCallback(xim, len, data, call_data)
    Xim		 xim;
    INT16	 len;
    XPointer	 data;
    XPointer	 call_data;
#endif /* NeedFunctionPrototypes */
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    XIMID	 imid = buf_s[0];
    XICID	 icid = buf_s[1];
    Xim		 im = (Xim)call_data;
    Xic		 ic;

    if ((imid == im->private.proto.imid)
     && (ic = _XimICOfXICID(im, icid))) {
	(void)_XimProcExtForwardKeyEvent(im, ic, (XPointer)&buf_s[2]);
	Xfree(data);
	return True;
    }
    return False;
}

Private Bool
_XimExtForwardKeyEventCheck(im, len, data, arg)
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
_XimExtForwardKeyEvent(ic, ev, sync)
    Xic		 ic;
    XKeyEvent	*ev;
    Bool	 sync;
{
    Xim		 im = (Xim) ic->core.im;
    CARD8	 buf[BUFSIZE];
    CARD8	*buf_b = &buf[XIM_HEADER_SIZE];	
    CARD16	*buf_s = (CARD16 *)buf_b;
    CARD32	*buf_l = (CARD32 *)buf_b;
    XPointer	reply;
    INT16	len;
    int		idx;

    if ((idx = _XimIsSupportExt(XIM_EXT_FORWARD_KEYEVENT_IDX)) < 0)
	return False;

    buf_s[0] = im->private.proto.imid;		/* imid */
    buf_s[1] = ic->private.proto.icid;		/* icid */
    buf_s[2] = sync ? XimSYNCHRONUS : 0;	/* flag */
    buf_s[3] = (CARD16)(((XAnyEvent *)ev)->serial & ((unsigned long) 0xffff));
						/* sequence number */
    buf_b[8] = ev->type;			/* xEvent.u.u.type */
    buf_b[9] = ev->keycode;			/* keycode */
    buf_s[5] = ev->state;			/* state */
    buf_l[3] = ev->time;			/* time */
    buf_l[4] = ev->window;			/* window */
    len = sizeof(CARD16)			/* sizeof imid */
	+ sizeof(CARD16)			/* sizeof icid */
	+ sizeof(BITMASK16)			/* sizeof flag */
	+ sizeof(CARD16)			/* sizeof sequence number */
	+ sizeof(BYTE)				/* sizeof xEvent.u.u.type */
	+ sizeof(BYTE)				/* sizeof keycode */
	+ sizeof(CARD16)			/* sizeof state */
	+ sizeof(CARD32)			/* sizeof time */
	+ sizeof(CARD32);			/* sizeof window */

    _XimSetHeader((XPointer)buf,
		extensions[idx].major_opcode,
		extensions[idx].minor_opcode, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);
    if (sync) {
	if (!(im->private.proto.recv(im, &len, &reply,
				    _XimExtForwardKeyEventCheck, (XPointer)ic)))
	    return False;
	Xfree(reply);
    }
    return True;
}
#endif /* NOT_EXT_FORWARD */

Private INT16
_XimSetExtensionList(buf)
    CARD8	*buf;
{
    register int i;
    BYTE	 len;
    INT16	 total = 0;

    for (i = 0; extensions[i].name; i++) {
	len = (BYTE)strlen(extensions[i].name);
	extensions[i].name_len = len;
	buf[0] = len;
	(void)strcpy((char *)&buf[1], extensions[i].name);
	len += sizeof(BYTE);
	total += len;
	buf += len;
    }
    return total;
}

Private void
_XimParseExtensionList(im, data)
    Xim		     im;
    CARD16	    *data;
{
    INT16	     len;
    INT16	     str_len;
    register CARD16 *buf;
    register int     i;

    if (!(len = data[0]))		/* length of extensions */
	return;

    buf = &data[1];;
    while (len > 0) {
	for (i = 0; extensions[i].name; i++) {
	    str_len = buf[2];
	    if ((str_len == extensions[i].name_len)
	     && (!strncmp((char *)&buf[3],
				extensions[i].name, str_len))) {
		extensions[i].major_opcode = (CARD8)buf[0];
		extensions[i].minor_opcode = (CARD8)buf[1];
		extensions[i].is_support   = True;
		break;
	    }
	}
	str_len += sizeof(CARD16)	/* sizeof major_opcode */
		 + sizeof(CARD16)	/* sizeof minor_opcode */
		 + sizeof(INT16)	/* sizeof length */
		 + XIM_PAD(str_len + 2);/* sizeof pad */
	len -= str_len;
	buf = (CARD16 *)((char *)buf + str_len);
    }
    return;
}

Private Bool
_XimQueryExtensionCheck(im, len, data, arg)
    Xim          im;
    INT16       *len;
    XPointer	 data;
    XPointer     arg;
{
    CARD16	*buf_s = (CARD16 *)((CARD8 *)data + XIM_HEADER_SIZE);
    CARD8	 major_opcode = *((CARD8 *)data);
    CARD8	 minor_opcode = *((CARD8 *)data + 1);
    XIMID	 imid = buf_s[0];

    if ((major_opcode == XIM_QUERY_EXTENSION_REPLY)
     && (minor_opcode == 0)
     && (imid == im->private.proto.imid))
	return True;
    return False;
}

Public Bool
_XimExtension(im)
    Xim		 im;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    XPointer	 reply;
    int		 idx;

    len = _XimSetExtensionList((CARD8 *)&buf_s[2]);
					/* extensions supported */
    if (!len)
	return True;

    buf_s[0] = im->private.proto.imid;	/* imid */
    buf_s[1] = len;			/* length of Extensions */
    XIM_SET_PAD(&buf_s[2], len);	/* pad */
    len += sizeof(CARD16)		/* sizeof imid */
	 + sizeof(INT16);		/* sizeof length of extensions */

   _XimSetHeader((XPointer)buf, XIM_QUERY_EXTENSION, 0, &len);
    if (!(im->private.proto.send(im, len, (XPointer)buf)))
	return False;
    im->private.proto.flush(im);
    if (!(im->private.proto.recv(im, &len, &reply, _XimQueryExtensionCheck, 0)))
	return False;

    buf_s = (CARD16 *)((char *)reply + XIM_HEADER_SIZE);
    _XimParseExtensionList(im, &buf_s[1]);
    Xfree(reply);

    if ((idx = _XimIsSupportExt(XIM_EXT_SET_EVENT_MASK_IDX)) >= 0)
	_XimRegProtoIntrCallback(im,
	 	extensions[idx].major_opcode,
	 	extensions[idx].minor_opcode,
		_XimExtSetEventMaskCallback, (XPointer)im);
#ifndef NOT_EXT_FORWARD
    if ((idx = _XimIsSupportExt(XIM_EXT_FORWARD_KEYEVENT_IDX)) >= 0)
	_XimRegProtoIntrCallback(im,
		extensions[idx].major_opcode,
		extensions[idx].minor_opcode,
		_XimExtForwardKeyEventCallback, (XPointer)im);
#endif

    return True;
}

#ifndef NOT_EXT_MOVE
/* flag of ExtenArgCheck */
#define	EXT_XNSPOTLOCATION	(1L<<0)

/* macro for ExtenArgCheck */
#define SET_EXT_XNSPOTLOCATION(flag) (flag |= EXT_XNSPOTLOCATION)
#define IS_EXT_XNSPOTLOCATION(flag)  (flag & EXT_XNSPOTLOCATION)

/* length of XPoint attribute */
#define	XIM_Xpoint_length	12

Private Bool
_XimExtMove(im, ic, x, y)
    Xim		 im;
    Xic		 ic;
    CARD16	 x;
    CARD16	 y;
{
    CARD8	 buf[BUFSIZE];
    CARD16	*buf_s = (CARD16 *)&buf[XIM_HEADER_SIZE];
    INT16	 len;
    int		idx;

    if ((idx = _XimIsSupportExt(XIM_EXT_MOVE_IDX)) < 0)
	return False;

    buf_s[0] = im->private.proto.imid;	/* imid */
    buf_s[1] = ic->private.proto.icid;	/* icid */
    buf_s[2] = x;			/* X */
    buf_s[3] = y;			/* Y */
    len = sizeof(CARD16)		/* sizeof imid */
	+ sizeof(CARD16)		/* sizeof icid */
	+ sizeof(INT16)			/* sizeof X */
	+ sizeof(INT16);		/* sizeof Y */

    _XimSetHeader((XPointer)buf, extensions[idx].major_opcode,
			extensions[idx].minor_opcode, &len);
    if (!im->private.proto.send(im, len, (XPointer)buf))
	return False;
    im->private.proto.flush(im);
    return True;
}

Public BITMASK32
_XimExtenArgCheck(arg)
    XIMArg	*arg;
{
    CARD32	flag = 0L;
    if (!strcmp(arg->name, XNSpotLocation))
	SET_EXT_XNSPOTLOCATION(flag);
    return flag;
}

Public Bool
#if NeedFunctionPrototypes
_XimExtenMove(
    Xim		 im,
    Xic		 ic,
    CARD32	 flag,
    CARD16	*buf,
    INT16	 length)
#else
_XimExtenMove(im, ic, flag, buf, length)
    Xim		 im;
    Xic		 ic;
    CARD32	 flag;
    CARD16	*buf;
    INT16	 length;
#endif /* NeedFunctionPrototypes */
{
    if ((IS_EXT_XNSPOTLOCATION(flag)) && (length == XIM_Xpoint_length))
	return _XimExtMove(im, ic, buf[4], buf[5]);
    return False;
}
#endif /* NOT_EXT_MOVE */
