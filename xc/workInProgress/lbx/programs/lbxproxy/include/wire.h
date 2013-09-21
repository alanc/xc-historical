/* $XConsortium: wire.h,v 1.5 95/05/17 18:23:24 dpw Exp $ */
/*
 * $NCDOr: wire.h,v 1.1 1993/11/16 17:58:49 keithp Exp keithp $
 * $NCDId: @(#)wire.h,v 1.7 1994/09/15 22:43:04 dct Exp $
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */
#include "misc.h"
#include "lbx.h"
#include "lbxdeltastr.h"

typedef struct _XServer {
    int			index;
    int			fd;
    int			lbxReq;
    int			lbxEvent;
    int			lbxError;
    Bool		initialized;
    ClientPtr		send, recv;
    ClientPtr		serverClient;
    int			recv_expect;
    int			motion_allowed;
    pointer		compHandle;
    LBXDeltasRec	indeltas;
    LBXDeltasRec	outdeltas;
} XServerRec;

#define MAX_SERVERS 128

extern XServerPtr   servers[];

extern void WriteReqToServer(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*len*/,
    char * /*buf*/
#endif
);

extern void WriteToServer(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*len*/,
    char * /*buf*/
#endif
);

extern void WriteToServerUncompressed(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*len*/,
    char * /*buf*/
#endif
);

extern Bool NewClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*setuplen*/
#endif
);

extern void CloseClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void ModifySequence(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*num*/
#endif
);

extern void AllowMotion(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*num*/
#endif
);

extern void SendIncrementPixel(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*cmap*/,
    unsigned long /*pixel*/
#endif
);

extern void SendGetModifierMapping(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void SendGetKeyboardMapping(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void SendQueryFont(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*fid*/
#endif
);

extern void SendChangeProperty(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Window /*win*/,
    Atom /*prop*/,
    Atom /*type*/,
    int /*format*/,
    int /*mode*/,
    unsigned long /*num*/
#endif
);

extern void SendGetProperty(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Window /*win*/,
    Atom /*prop*/,
    Atom /*type*/,
    Bool /*delete*/,
    unsigned long /*off*/,
    unsigned long /*len*/
#endif
);

extern void SendQueryTag(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*tag*/
#endif
);

extern void SendInvalidateTag(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*tag*/
#endif
);

extern void SendTagData(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*tag*/,
    unsigned long /*len*/,
    pointer /*data*/
#endif
);

extern void SendGetImage(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    Drawable /*drawable*/,
    int /*x*/,
    int /*y*/,
    unsigned int /*width*/,
    unsigned int /*height*/,
    unsigned long /*planeMask*/,
    int /*format*/
#endif
);

extern void ServerReply(
#if NeedFunctionPrototypes
    XServerPtr /*server*/,
    xReply * /*rep*/
#endif
);

extern void ExpectServerReply(
#if NeedFunctionPrototypes
    XServerPtr /*server*/,
    void (* /*func*/)()
#endif
);

extern unsigned long ServerRequestLength(
#if NeedFunctionPrototypes
    xReq * /*req*/,
    ClientPtr /*sc*/,
    int /*gotnow*/,
    Bool * /*partp*/
#endif
);

extern int ServerProcStandardEvent(
#if NeedFunctionPrototypes
    ClientPtr /*sc*/
#endif
);

extern void LbxIgnoreAllClients(
#if NeedFunctionPrototypes
    XServerPtr /*server*/
#endif
);

extern void LbxAttendAllClients(
#if NeedFunctionPrototypes
    XServerPtr /*server*/
#endif
);

extern void LbxOnlyListenToOneClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void LbxListenToAllClients(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool ProxyWorkProc(
#if NeedFunctionPrototypes
    pointer /*dummy*/,
    int /*strchr*/
#endif
);

extern void CloseServer(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void StartProxyReply(
#if NeedFunctionPrototypes
    XServerPtr /*server*/,
    xLbxStartReply * /*rep*/
#endif
);

extern void StartProxy(
#if NeedFunctionPrototypes
    XServerPtr /*server*/
#endif
);

extern Bool ConnectToServer(
#if NeedFunctionPrototypes
    char * /*dpy_name*/
#endif
);
