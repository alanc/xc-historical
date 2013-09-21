/* $XConsortium: os.h,v 1.1 95/05/16 20:35:49 dpw Exp $ */

/*

Copyright (c) 1995  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
#ifndef OS_H
#define OS_H


#define DEFAULT_ACCESS_CONTROL TRUE

/* WaitFor.c */

extern int WaitForSomething(
#if NeedFunctionPrototypes
    int * /*pClientsReady*/
#endif
);

/* access.c */

extern void EnableLocalHost(
#if NeedFunctionPrototypes
    void
#endif
);

extern void DisableLocalHost(
#if NeedFunctionPrototypes
    void
#endif
);

extern void AccessUsingXdmcp(
#if NeedFunctionPrototypes
    void
#endif
);

extern void DefineSelf(
#if NeedFunctionPrototypes
    int /*fd*/
#endif
);

extern void AddLocalHosts(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ResetHosts(
#if NeedFunctionPrototypes
    char * /*display*/
#endif
);

extern int AddHost(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*family*/,
    unsigned /*length*/,
    pointer /*pAddr*/
#endif
);

extern Bool ForEachHostInFamily(
#if NeedFunctionPrototypes
    int /*family*/,
    Bool (* /*func*/)(),
    pointer /*closure*/
#endif
);

extern int RemoveHost(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*family*/,
    unsigned /*length*/,
    pointer /*pAddr*/
#endif
);

extern int GetHosts(
#if NeedFunctionPrototypes
    pointer * /*data*/,
    int * /*pnHosts*/,
    int * /*pLen*/,
    BOOL * /*pEnabled*/
#endif
);

typedef  struct sockaddr *sockaddrptr;

extern int InvalidHost(
#if NeedFunctionPrototypes
    sockaddrptr  /*saddr*/,
    int /*len*/
#endif
);

extern int ChangeAccessControl(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*fEnabled*/
#endif
);

/* connection.c */

extern void CreateWellKnownSockets(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ResetWellKnownSockets(
#if NeedFunctionPrototypes
    void
#endif
);

extern char *ClientAuthorized(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    unsigned int /*proto_n*/,
    char * /*auth_proto*/,
    unsigned int /*string_n*/,
    char * /*auth_string*/
#endif
);

extern int ClientConnectionNumber(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void AvailableClientInput(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern ClientPtr AllocNewConnection(
#if 0 /* NeedFunctionPrototypes */
    int /*fd*/,
    int (* /*Read*/)(),
    int (* /*Writev*/)(),
    void (* /*Close*/)()
#endif
);

extern void SwitchConnectionFuncs(
#if 0 /* NeedFunctionPrototypes */
    ClientPtr /*client*/,
    int (* /*Read*/)(),
    int (* /*Writev*/)(),
    void (* /*Close*/)()
#endif
);

extern void StartOutputCompression(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    void (* /*CompressOn*/)(),
    void (* /*CompressOff*/)()
#endif
);

typedef struct iovec *iovecptr;
extern int Writev(
#if NeedFunctionPrototypes
    int /*fd*/,
    iovecptr /*iov*/,
    int /*iovcnt*/
#endif
);

extern Bool EstablishNewConnections(
#if NeedFunctionPrototypes
    ClientPtr /*clientUnused*/,
    pointer /*closure*/
#endif
);

extern void CloseDownFileDescriptor(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void CheckConnections(
#if NeedFunctionPrototypes
    void
#endif
);

extern void CloseDownConnection(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void OnlyListenToOneClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void ListenToAllClients(
#if NeedFunctionPrototypes
    void
#endif
);

extern void IgnoreClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void AttendClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void MakeClientGrabImpervious(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void MakeClientGrabPervious(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

/* io.c */

extern unsigned long StandardRequestLength(
#if NeedFunctionPrototypes
    xReq * /*req*/,
    ClientPtr /*client*/,
    int /*got*/,
    Bool * /*partp*/
#endif
);

extern int StandardReadRequestFromClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int PendingClientOutput(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int CheckPendingClientInput(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void MarkConnectionWriteBlocked(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int BytesInClientBuffer(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void SkipInClientBuffer(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*nbytes*/,
    int /*lenLastReq*/
#endif
);

extern Bool InsertFakeRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/,
    int /*count*/
#endif
);

extern void ResetCurrentRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void FlushAllOutput(
#if NeedFunctionPrototypes
    void
#endif
);
extern int StandardWriteToClient(
#if NeedFunctionPrototypes
    ClientPtr /*who*/,
    int /*count*/,
    char * /*buf*/
#endif
);

extern int UncompressWriteToClient(
#if NeedFunctionPrototypes
    ClientPtr /*who*/,
    int /*count*/,
    char * /*buf*/
#endif
);

extern void ResetOsBuffers(
#if NeedFunctionPrototypes
    void
#endif
);

/* osinit.c */

extern void OsInit(
#if NeedFunctionPrototypes
    void
#endif
);

#endif
