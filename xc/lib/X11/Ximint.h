/* $XConsortium: Ximint.h,v 1.2 93/09/17 17:50:19 rws Exp $ */
/******************************************************************

                Copyright 1992 by FUJITSU LIMITED
                Copyright 1993 by Sony Corporation

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

  Author:   Takashi Fujiwara   FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp
  Motifier: Makoto Wakamatsu   Sony Corporation
			       makoto@sm.sony.co.jp

******************************************************************/

#ifndef _XIMINT_H
#define _XIMINT_H

#include <X11/Xutil.h>

#define Public /**/
#define Private static

typedef struct _Xim	*Xim;
typedef struct _Xic	*Xic;

/*
 * Input Method data
 */
#include "XimintL.h"
#include "XimintP.h"

/*
 * XIM dependent data
 */
typedef union _XIMPrivateRec {
    XimLocalPrivateRec   local;
    XimProtoPrivateRec   proto;
} XIMPrivateRec;

/*
 * IM struct
 */
typedef struct _Xim {
    XIMMethods		methods;
    XIMCoreRec		core;
    XIMPrivateRec	private;
} XimRec;

/*
 * IC deprndent data
 */
typedef union _XICPrivateRec {
    XicLocalPrivateRec   local;
    XicProtoPrivateRec   proto;
} XICPrivateRec;

/*
 * IC struct
 */
typedef struct _Xic {
	XICMethods	methods;
	XICCoreRec	core;
	XICPrivateRec	private;
} XicRec;

#define FILTERD         True
#define NOTFILTERD      False

#define XIMMODIFIER		"@im="

/*
 * Global symbols
 */

extern Bool	_XimDispatchInit(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern Bool	 _XimGetAttributeID(
#if NeedFunctionPrototypes
    Xim		 im,
    CARD16	*buf
#endif
);

extern Bool	 _XimExtension(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern void	_XimDestroyIMStructureList(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern char *	_XimEncodeAttrIDList(
#if NeedFunctionPrototypes
    XIMResourceList	 res_list,
    unsigned int	 res_num,
    XIMArg		*arg,
    CARD16		*idList,
    INT16		*num
#endif
);

extern char *	_XimDecodeATTRIBUTE(
#if NeedFunctionPrototypes
    Xic			 ic,
    XIMResourceList	 res_list,
    unsigned int	 res_num,
    CARD16		*buf,
    INT16		 buf_len,
    XIMArg		*arg,
    BITMASK32		 mode
#endif
);

extern Bool	_XimRegProtoIntrCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    CARD16	 major_code,
    CARD16	 minor_code,
    Bool	 (*proc)(
#if NeedNestedPrototypes
			Xim, INT16, XPointer, XPointer
#endif
			),
    XPointer	 call_data
#endif
);

extern Bool	_XimErrorCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data
#endif
);

extern Bool	_XimRegisterTriggerKeysCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data
#endif
);

extern Bool	_XimSetEventMaskCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data
#endif
);

extern Bool	_XimForwardEventCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data
#endif
);

extern Bool	_XimCommitCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data
#endif
);

extern Bool	_XimSyncCallback(
#if NeedFunctionPrototypes
    Xim		 im,
    INT16	 len,
    XPointer	 data,
    XPointer	 call_data
#endif
);

extern void	_XimFreeProtoIntrCallback(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern void	_XimFreePendingProc(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern XIC	 _XimProtoCreateIC(
#if NeedFunctionPrototypes
    XIM		 im,
    XIMArg	*arg
#endif
);

extern void	_XimRegisterServerFilter(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern void	_XimUnregisterServerFilter(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern Bool	_XimEncodingNegitiation(
#if NeedFunctionPrototypes
    Xim		 im
#endif
);

extern Bool	_XimTriggerNotify(
#if NeedFunctionPrototypes
    Xim		 im,
    Xic		 ic,
    int		 mode,
    CARD32	 idx
#endif
);

extern Bool	_XimProcSyncReply(
#if NeedFunctionPrototypes
    Xim		 im,
    Xic		 ic
#endif
);

extern void	_XimSendSetFocus(
#if NeedFunctionPrototypes
    Xim		 im,
    Xic		 ic
#endif
);

extern Bool	_XimForwardEvent(
#if NeedFunctionPrototypes
    Xic		 ic,
    XEvent	*ev,
    Bool	 sync
#endif
);

extern void	_XimFreeRemakeArg(
#if NeedFunctionPrototypes
    XIMArg	*arg
#endif
);

extern void	_XimServerDestroy(
#if NeedFunctionPrototypes
    void
#endif
);

extern char *	_XimEncodeATTRIBUTE(
#if NeedFunctionPrototypes
    Xic			 ic,
    XIMResourceList	 res_list,
    unsigned int	 res_num,
    CARD16		*buf,
    INT16		 buf_size,
    XIMArg		*arg,
    INT16		*total_length,
    BITMASK32		*flag,
    int			*is_window,
    BITMASK32		 mode
#endif
);

#ifndef NOT_EXT_MOVE
extern Bool	_XimExtenMove(
#if NeedFunctionPrototypes
    Xim		 im,
    Xic		 ic,
    CARD32	 flag,
    CARD16	*buf,
    INT16	 length
#endif
);
#endif

extern int	_Xlcctstombs(
#if NeedFunctionPrototypes
    XLCd	 lcd,
    char	*to,
    char	*from,
    int		 to_len
#endif
);

extern int	_Xlcctstowcs(
#if NeedFunctionPrototypes
    XLCd	 lcd,
    wchar_t	*to,
    char	*from,
    int		 to_len
#endif
);

extern void	_XimSetHeader(
#if NeedFunctionPrototypes
    XPointer	 buf,
    CARD8	 major_opcode,
    CARD8	 minor_opcode,
    INT16	*len
#endif
);

extern Bool	_XimSync(
#if NeedFunctionPrototypes
    Xim		 im,
    Xic		 ic
#endif
);

extern int	_XimProtoMbLookupString(
#if NeedFunctionPrototypes
    XIC		 xic,
    XKeyEvent	*ev,
    char	*buffer,
    int		 bytes,
    KeySym	*keysym,
    Status	*status
#endif
);

extern int	_XimProtoWcLookupString(
#if NeedFunctionPrototypes
    XIC		 xic,
    XKeyEvent	*ev,
    wchar_t	*buffer,
    int		 bytes,
    KeySym	*keysym,
    Status	*status
#endif
);

extern int	_XimRegPendingProc(
#if NeedFunctionPrototypes
    Xim		 im,
    Xic		 ic,
    XIMArg	*arg,
    void	 (*proc)()
#endif
);

extern void	_XimUnregisterKeyFilter(
#if NeedFunctionPrototypes
    Xic		 ic
#endif
);

extern Status	_XimProtoEventToWire(
#if NeedFunctionPrototypes
    XEvent	*re,
    xEvent	*event,
    Bool	sw
#endif
);

extern Bool	_XimProtoWireToEvent(
#if NeedFunctionPrototypes
    XEvent	*re,
    xEvent	*event,
    Bool	 sw
#endif
);

#ifndef NOT_EXT_FORWARD
extern Bool	_XimExtForwardKeyEvent(
#if NeedFunctionPrototypes
    Xic		 ic,
    XKeyEvent	*ev,
    Bool	 sync
#endif
);
#endif

extern int	_XimLookupMBText(
#if NeedFunctionPrototypes
    Xic			 ic,
    XKeyEvent		*event,
    unsigned char	*buffer,
    int			 nbytes,
    KeySym		*keysym,
    XComposeStatus	*status
#endif
);

extern int	_XimLookupWCText(
#if NeedFunctionPrototypes
    Xic			 ic,
    XKeyEvent		*event,
    wchar_t		*buffer,
    int			 nbytes,
    KeySym		*keysym,
    XComposeStatus	*status
#endif
);

extern void	_XimPendingFilter(
#if NeedFunctionPrototypes
    Xic			 ic
#endif
);

extern EVENTMASK	_XimGetWindowEventmask(
#if NeedFunctionPrototypes
    Xic		 ic
#endif
);

extern Xic	_XimICOfXICID(
#if NeedFunctionPrototypes
    Xim		im,
    XICID	icid
#endif
);

extern void	_XimRegisterKeyFilter(
#if NeedFunctionPrototypes
    Xic		 ic
#endif
);

extern void	_XimResetIMInstantiateCallback(
#if NeedFunctionPrototypes
    Xim         xim
#endif
);

#ifndef NOT_EXT_MOVE
extern CARD32	_XimExtenArgCheck(
#if NeedFunctionProtoTypes
    XIMArg	*arg
#endif
);
#endif

#endif /* _XIMINT_H */
