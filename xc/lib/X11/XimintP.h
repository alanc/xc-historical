/* $XConsortium: XimintP.h,v 1.3 93/09/24 10:48:06 rws Exp $ */
/******************************************************************

           Copyright 1991, 1992 by Sun Microsystems, Inc.
           Copyright 1992, 1993 by FUJITSU LIMITED
           Copyright 1993       by Sony Corporation

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and FUJITSU LIMITED not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. FUJITSU LIMITED and Sony Corporation makes no
representations about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Sun Microsystems Inc. ,FUJITSU LIMITED AND SONY CORPORATION DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL Sun Microsystems, Inc.,
FUJITSU LIMITED, SONY CORPORATIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
          Takashi Fujiwara     FUJITSU LIMITED 
                                 fujiwara@a80.tech.yk.fujitsu.co.jp
	  Makoto Wakamatsu     Sony Corporation
                                 makoto@sm.sony.co.jp
	  Hiroyuki Miyamoto    Digital Equipment Corporation
                                 miyamoto@jrd.dec.com

******************************************************************/

#ifndef _XIMINTP_H
#define _XIMINTP_H

#include "XimProto.h"
#include "XlcPublic.h"

/*
 * for protocol layer callback function
 */
typedef Bool (*XimProtoIntrProc)(
#if NeedFunctionPrototypes
	Xim, INT16, XPointer, XPointer
#endif
);
typedef struct _XimProtoIntrRec {
    XimProtoIntrProc		 func;
    CARD16			 major_code;
    CARD16			 minor_code;
    XPointer			 call_data;
    struct _XimProtoIntrRec	*next;
} XimProtoIntrRec;

/*
 * for transport layer methods
 */
typedef Bool (*XimTransConnectProc)(
#if NeedFunctionPrototypes
				 Xim
#endif
);
typedef Bool (*XimTransShutdownProc)(
#if NeedFunctionPrototypes
				  Xim
#endif
);
typedef Bool (*XimTransSendProc)(
#if NeedFunctionPrototypes
			      Xim, INT16, XPointer
#endif
);
typedef Bool (*XimTransRecvProc)(
#if NeedFunctionPrototypes
			      Xim, XPointer, int, int, int, int *, XPointer
#endif
);
typedef void (*XimTransFlushProc)(
#if NeedFunctionPrototypes
				  Xim
#endif
);
typedef Bool (*XimTransIntrProc)(
#if NeedFunctionPrototypes
		      Xim, Bool (*)(Xim, INT16, XPointer, XPointer), XPointer
#endif
);
typedef Bool (*XimTransIntrCheckProc)(
#if NeedFunctionPrototypes
				   Xim, INT16, XPointer
#endif
);

/*
 * private part of IM
 */
typedef struct _XimProtoPrivateRec {
    Window 		 im_window;		/* XIM_SERVERS ATOM  */
    XIMID 		 imid;
    XIMStyles		*default_styles; 	/* for delay binding */
    CARD32		*im_onkeylist;		/* LISTofTriggerKey  */
    CARD32		*im_offkeylist;		/* LISTofTriggerKey  */
    BITMASK32 		 flag;			/* IMS Mode          */

    BITMASK32		 registed_filter_event; /* registed filter mask */
    EVENTMASK		 forward_event_mask;	/* default forward event */
    EVENTMASK		 synchronous_event_mask;/* default sync event */

    char		*im_attribute_name;
    XimProtoIntrRec	*intrproto;
    XIMResourceList	 im_inner_resources;
    unsigned int	 im_num_inner_resources;
    XIMResourceList	 ic_inner_resources;
    unsigned int	 ic_num_inner_resources;
    char		*hold_data;
    int			 hold_data_len;
    char		*locale_name;

    XlcConv		 ctom_conv;
    XlcConv		 ctow_conv;

    /*
     * transport specific
     */
    XimTransConnectProc	 connect;
    XimTransShutdownProc shutdown;
    XimTransSendProc	 send;
    XimTransRecvProc	 recv;
    XimTransFlushProc	 flush;
    XimTransIntrProc	 intr_cb;
    XimTransIntrCheckProc check_cb;
    XPointer	         spec;
} XimProtoPrivateRec;

/*
 * bit mask for the flag of XIMPrivateRec
 */
#define DELAYBINDABLE		(1L << 0)
#define RECONNECTABLE		(1L << 1)
#define RESTARTABLE		(1L << 2)
#define SERVER_CONNECTED	(1L << 3)
#define	DYNAMIC_EVENT_FLOW	(1L << 4)
#define	USE_AUTHORIZATION_FUNC	(1L << 5)

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
#define UNMARK_SERVER_CONNECTED(im) \
		((((Xim)im))->private.proto.flag &= ~SERVER_CONNECTED)

#define	IS_DYNAMIC_EVENT_FLOW(im) \
		(((Xim)im)->private.proto.flag & DYNAMIC_EVENT_FLOW)
#define	MARK_DYNAMIC_EVENT_FLOW(im) \
		(((Xim)im)->private.proto.flag |= DYNAMIC_EVENT_FLOW)

#define	IS_USE_AUTHORIZATION_FUNC(im) \
		(((Xim)im)->private.proto.flag & USE_AUTHORIZATION_FUNC)
#define	MARK_USE_AUTHORIZATION_FUNC(im) \
		(((Xim)im)->private.proto.flag |= USE_AUTHORIZATION_FUNC)

/*
 * bit mask for the register_filter_event of XIMPrivateRec/XICPrivateRec
 */
#define KEYPRESS_MASK		(1L)
#define KEYRELEASE_MASK		(1L << 1)
#define DESTROYNOTIFY_MASK	(1L << 2)

typedef struct _XimCommitInfoRec {
    struct _XimCommitInfoRec	*next;
    char			*string;
    int				 string_len;
    KeySym			*keysym;
    int				 keysym_len;
} XimCommitInfoRec, *XimCommitInfo;

typedef struct _XimPendingCallback {
    int				 major_opcode;
    Xim				 im;
    Xic				 ic;
    char			*proto;
    int				 proto_len;
    struct _XimPendingCallback	*next;
} XimPendingCallbackRec, *XimPendingCallback;


/*
 * private part of IC
 */
typedef struct _XicProtoPrivateRec {
    XICID	         icid;			/* ICID		*/
    BITMASK32		 flag;			/* Input Mode	*/

    BITMASK32		 registed_filter_event; /* registed filter mask */
    EVENTMASK		 forward_event_mask;	/* forward event mask */
    EVENTMASK		 synchronous_event_mask;/* sync event mask */
    EVENTMASK		 filter_event_mask;	/* negrect event mask */
    EVENTMASK		 intercept_event_mask;	/* deselect event mask */
    EVENTMASK		 select_event_mask;	/* select event mask */

    char		*preedit_font;		/* Base font name list */
    int			 preedit_font_length;	/* length of base font name */
    char		*status_font;		/* Base font name list */
    int			 status_font_length;	/* length of base font name */

    XimCommitInfo	 commit_info;
    XIMResourceList	 ic_resources;
    unsigned int	 ic_num_resources;
    XIMResourceList	 ic_inner_resources;
    unsigned int	 ic_num_inner_resources;
    XimPendingCallback	 pend_cb_que;
    Bool		 waitCallback;
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
   ((((Xic)ic)->private.proto.synchronous_event_mask & (mask)) ? True: False)

#define XIM_MAXIMNAMELEN 64
#define XIM_MAXLCNAMELEN 64

#endif /* _XIMINTP_H */
