/* $XConsortium$ */
/******************************************************************

           Copyright 1991, 1992 by Sun Microsystems, Inc.
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

#ifndef _XIMINTP_H
#define _XIMINTP_H

#include "XimProto.h"

/*
 * for protocol layer callback function
 */
typedef struct _XimProtoIntrRec {
    Bool			 (*func)(
#if NeedFunctionPrototypes
	Xim, INT16, void *, XPointer
#endif
    );
    CARD16			 major_code;
    CARD16			 minor_code;
    XPointer			 call_data;
    struct _XimProtoIntrRec	*next;
} XimProtoIntrRec;

/*
 * for pending function
 */
typedef struct _XimPendingRec {
    void			 (*func)();
    XIC				 ic;
    XIMArg			*arg;
    struct _XimPendingRec	*next;
} XimPendingRec;

/*
 * private part of IM
 */
typedef struct _XimProtoPrivateRec {
    Window 		 im_window;		/* XIM_SERVERS ATOM  */
    XIMID 		 imid;
    XIMStyles		*default_styles; 	/* for delay binding */
    XIMStyles 		*im_styles;		/* IMS's input style */
    CARD32		*im_onkeylist;		/* LISTofTriggerKey  */
    CARD32		*im_offkeylist;		/* LISTofTriggerKey  */
    BITMASK32 		 flag;			/* IMS Mode          */
    unsigned long	 filter_events;		/* default filter event */
    BITMASK32		 register_filter_event; /* default register  */
    char		*im_attribute_name;
    char		*ic_attribute_name;
    XimProtoIntrRec	*intrproto;
    XimPendingRec	*pending;

    /*
     * transport specific
     */
    Bool	         (*connect)();
    Bool	         (*shutdown)();
    Bool	         (*send)();
    Bool	         (*recv)();
    Bool	         (*intr_cb)();
    void	         (*flush)();
    XPointer	         spec;
} XimProtoPrivateRec;


/*
 * bit mask for the flag of XIMPrivateRec
 */
#define DELAYBINDABLE		(1L)
#define RECONNECTABLE		(1L<<1)
#define RESTARTABLE		(1L<<2)
#define SERVER_CONNECTED	(1L<<3)
#define	DYNAMIC_EVENT_FLOW	(1L<<4)
#define	PREVIOUS_FORWARDEVENT	(1L<<5)
#define	NEED_PENDING_CALL	(1L<<6)
#define	USE_AUTHORIZATION_FUNC	(1L<<7)

/*
 * macro for the flag of XIMPrivateRec
 */
#define IS_DELAYBINDABLE(im) \
		(((Xim)im)->private.proto.flag & DELAYBINDABLE)
#define MARK_DELAYBINDABLE(im) \
		(((Xim)im)->private.proto.flag |= DELAYBINDABLE)

#define IS_RECONNECTABLE(im) \
		(((Xim)im)->private.proto.flag & RECONNECTABLE)
#define MARK_RECONNECTABLE(im) \
		(((Xim)im)->private.proto.flag |= RECONNECTABLE)

#define IS_RESTARTABLE(im) \
		(((Xim)im)->private.proto.flag & RESTARTABLE)
#define MARK_RESTARTABLE(im) \
		(((Xim)im)->private.proto.flag |= RESTARTABLE)

#define IS_CONNECTABLE(im) \
    (((Xim)im)->private.proto.flag & (DELAYBINDABLE|RECONNECTABLE|RESTARTABLE))
#define MAKE_CONNECTABLE(im) \
    (((Xim)im)->private.proto.flag |= (DELAYBINDABLE|RECONNECTABLE|RESTARTABLE))

#define IS_UNCONNECTABLE(im) (!(IS_CONNECTABLE(im)))
#define MAKE_UNCONNECTABLE(im) \
(((Xim)im)->private.proto.flag &=  ~(DELAYBINDABLE|RECONNECTABLE|RESTARTABLE))

#define IS_SERVER_CONNECTED(im) \
		((((Xim)im))->private.proto.flag & SERVER_CONNECTED)
#define MARK_SERVER_CONNECTED(im) \
		((((Xim)im))->private.proto.flag |= SERVER_CONNECTED)

#define	IS_DYNAMIC_EVENT_FLOW(im) \
		(((Xim)im)->private.proto.flag & DYNAMIC_EVENT_FLOW)
#define	MARK_DYNAMIC_EVENT_FLOW(im) \
		(((Xim)im)->private.proto.flag |= DYNAMIC_EVENT_FLOW)

#define	IS_PREVIOUS_FORWARDEVENT(im) \
		(((Xim)im)->private.proto.flag & PREVIOUS_FORWARDEVENT)
#define	MARK_PREVIOUS_FORWARDEVENT(im) \
		(((Xim)im)->private.proto.flag |= PREVIOUS_FORWARDEVENT)
#define	UNMARK_PREVIOUS_FORWARDEVENT(im) \
		(((Xim)im)->private.proto.flag &= ~PREVIOUS_FORWARDEVENT)

#define	IS_NEED_PENDING_CALL(im) \
		(((Xim)im)->private.proto.flag & NEED_PENDING_CALL)
#define	MARK_NEED_PENDING_CALL(im) \
		(((Xim)im)->private.proto.flag |= NEED_PENDING_CALL)
#define	UNMARK_NEED_PENDING_CALL(im) \
		(((Xim)im)->private.proto.flag &= ~NEED_PENDING_CALL)

#define	IS_USE_AUTHORIZATION_FUNC(im) \
		(((Xim)im)->private.proto.flag & USE_AUTHORIZATION_FUNC)
#define	MARK_USE_AUTHORIZATION_FUNC(im) \
		(((Xim)im)->private.proto.flag |= USE_AUTHORIZATION_FUNC)

/*
 * bit mask for the register_filter_event of XIMPrivateRec/XICPrivateRec
 */
#define	KEYPRESS_MASK		(1L)
#define	KEYRELEASE_MASK		(1L<<1)
#define	DESTROYNOTIFY_MASK	(1L<<2)

/*
 * private part of IC
 */
typedef struct _XicProtoPrivateRec {
    XICID        icid;				/* ICID		*/
    BITMASK32	 flag;				/* Input Mode	*/
    BITMASK32	 register_filter_event;
    EVENTMASK	 filter_event_mask;
    EVENTMASK	 intercept_event_mask;
    EVENTMASK	 select_event_mask;
    EVENTMASK	 forward_event_mask;
    EVENTMASK	 synchronous_event_mask;

    BITMASK32	 mandatory_mask;		/* API check 	*/
    char	*preedit_font;			/* Base font name list */
    int		 preedit_font_length;		/* length of base font name */
    char	*status_font;			/* Base font name list */
    int		 status_font_length;		/* length of base font name */
    char	*xim_commit;
} XicProtoPrivateRec ;

/*
 * macro for the icid of XimProtoPrivateRec
 */
#define IS_IC_CONNECTED(ic)       (((Xic)ic)->private.proto.icid)

/*
 * bit mask for the flag of XICPrivateRec
 */
#define BEING_PREEDITED    (1L)
#define FABLICATED         (1L<<1)
#define	NEED_SYNC_REPLY    (1L<<2)

/*
 * macro for the flag of XICPrivateRec
 */
#define IS_BEING_PREEDITED(ic) \
		(((Xic)ic)->private.proto.flag & BEING_PREEDITED)
#define MARK_BEING_PREEDITED(ic) \
		(((Xic)ic)->private.proto.flag |= BEING_PREEDITED)
#define UNMARK_BEING_PREEDITED(ic) \
		(((Xic)ic)->private.proto.flag &= ~BEING_PREEDITED)

#define IS_FABLICATED(ic) \
		(((Xic)ic)->private.proto.flag & FABLICATED)
#define MARK_FABLICATED(ic) \
		(((Xic)ic)->private.proto.flag |= FABLICATED)
#define UNMARK_FABLICATED(ic) \
		(((Xic)ic)->private.proto.flag &= ~FABLICATED)

#define	IS_NEED_SYNC_REPLY(ic) \
		(((Xic)ic)->private.proto.flag & NEED_SYNC_REPLY)
#define	MARK_NEED_SYNC_REPLY(ic) \
		(((Xic)ic)->private.proto.flag |= NEED_SYNC_REPLY)
#define	UNMARK_NEED_SYNC_REPLY(ic) \
		(((Xic)ic)->private.proto.flag &= ~NEED_SYNC_REPLY)

/*
 * XIM Library Support FORWARD Events mask
 */
#define	XIM_FORWARD_EVENT_MASKS (KeyPressMask|KeyReleaseMask)

/*
 * macro for the filter_event_mask of XICPrivateRec
 */
#define	IS_NEGLECT_EVENT(ic, mask) \
		(((Xic)ic)->private.proto.filter_event_mask & (mask))

/*
 * macro for the forward_event_mask of XICPrivateRec
 */
#define	IS_FORWARD_EVENT(ic, mask) \
		(((Xic)ic)->private.proto.forward_event_mask & (mask))

/*
 * macro for the synchronous_event_mask of XICPrivateRec
 */
#define	IS_SYNCHRONOUS_EVENT(ic, mask) \
		(((Xic)ic)->private.proto.synchronous_event_mask & (mask))

/*
 * Check bit mask for API (XCreateIC)
 */
#define _XIM_INPUT_STYLE_MASK            (1L <<  0)
#define _XIM_CLIENT_WIN_MASK             (1L <<  1)
#define _XIM_FOCUS_WIN_MASK              (1L <<  2)

/*
 * mode of _XimEncodeATTRIBUTE
 */
#define	_XIM_TOP_ATTR			(1L << 0)
#define	_XIM_PREEDIT_ATTR		(1L << 1)
#define	_XIM_STATUS_ATTR		(1L << 2)

#define XIM_MAXIMNAMELEN 64
#define XIM_MAXLCNAMELEN 64

#endif /* _XIMINTP_H */
