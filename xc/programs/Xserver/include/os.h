/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: os.h,v 1.51 93/09/24 12:17:42 rws Exp $ */

#ifndef OS_H
#define OS_H
#include "misc.h"

#ifdef INCLUDE_ALLOCA_H
#include <alloca.h>
#endif

#define NullFID ((FID) 0)

#define SCREEN_SAVER_ON   0
#define SCREEN_SAVER_OFF  1
#define SCREEN_SAVER_FORCER 2
#define SCREEN_SAVER_CYCLE  3

#ifndef MAX_REQUEST_SIZE
#define MAX_REQUEST_SIZE 65535
#endif
#ifndef MAX_BIG_REQUEST_SIZE
#define MAX_BIG_REQUEST_SIZE 1048575
#endif

typedef pointer	FID;
typedef struct _FontPathRec *FontPathPtr;
typedef struct _NewClientRec *NewClientPtr;

#ifndef NO_ALLOCA
/*
 * os-dependent definition of local allocation and deallocation
 * If you want something other than Xalloc/Xfree for ALLOCATE/DEALLOCATE
 * LOCAL then you add that in here.
 */
#ifdef __HIGHC__

extern char *alloca();

#if HCVERSION < 21003
#define ALLOCATE_LOCAL(size)	alloca((int)(size))
pragma on(alloca);
#else /* HCVERSION >= 21003 */
#define	ALLOCATE_LOCAL(size)	_Alloca((int)(size))
#endif /* HCVERSION < 21003 */

#define DEALLOCATE_LOCAL(ptr)  /* as nothing */

#endif /* defined(__HIGHC__) */


#ifdef __GNUC__
#define alloca __builtin_alloca
#endif

/*
 * warning: old mips alloca (pre 2.10) is unusable, new one is builtin
 * Test is easy, the new one is named __builtin_alloca and comes
 * from alloca.h which #defines alloca.
 */
#if defined(vax) || defined(sun) || defined(apollo) || defined(stellar) || defined(alloca)
/*
 * Some System V boxes extract alloca.o from /lib/libPW.a; if you
 * decide that you don't want to use alloca, you might want to fix 
 * ../os/4.2bsd/Imakefile
 */
#ifndef alloca
char *alloca();
#endif
#define ALLOCATE_LOCAL(size) alloca((int)(size))
#define DEALLOCATE_LOCAL(ptr)  /* as nothing */
#endif /* who does alloca */

#endif /* NO_ALLOCA */

#ifndef ALLOCATE_LOCAL
#define ALLOCATE_LOCAL(size) Xalloc((unsigned long)(size))
#define DEALLOCATE_LOCAL(ptr) Xfree((pointer)(ptr))
#endif /* ALLOCATE_LOCAL */


#define xalloc(size) Xalloc((unsigned long)(size))
#define xrealloc(ptr, size) Xrealloc((pointer)(ptr), (unsigned long)(size))
#define xfree(ptr) Xfree((pointer)(ptr))

#ifndef X_NOT_STDC_ENV
#include <string.h>
#else
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif
#endif

/* have to put $(SIGNAL_DEFINES) in DEFINES in Imakefile to get this right */
#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

extern int WaitForSomething(
#if NeedFunctionPrototypes
    int* /*pClientsReady*/
#endif
);

extern int ReadRequestFromClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern Bool InsertFakeRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char* /*data*/,
    int /*count*/
#endif
);

extern int ResetCurrentRequest(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void FlushAllOutput(
#if NeedFunctionPrototypes
    void
#endif
);

extern void FlushIfCriticalOutputPending(
#if NeedFunctionPrototypes
    void
#endif
);

extern void SetCriticalOutputPending(
#if NeedFunctionPrototypes
    void
#endif
);

extern int WriteToClient(
#if NeedFunctionPrototypes
    ClientPtr /*who*/,
    int /*count*/,
    char* /*buf*/
#endif
);

extern void ResetOsBuffers(
#if NeedFunctionPrototypes
    void
#endif
);

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
    char* /*auth_proto*/,
    unsigned int /*string_n*/,
    char* /*auth_string*/
#endif
);

extern Bool EstablishNewConnections(
#if NeedFunctionPrototypes
    ClientPtr /*clientUnused*/,
    pointer /*closure*/
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

extern int AddEnabledDevice(
#if NeedFunctionPrototypes
    int /*fd*/
#endif
);

extern int RemoveEnabledDevice(
#if NeedFunctionPrototypes
    int /*fd*/
#endif
);

extern int OnlyListenToOneClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int ListenToAllClients(
#if NeedFunctionPrototypes
    void
#endif
);

extern int IgnoreClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int AttendClient(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int MakeClientGrabImpervious(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int MakeClientGrabPervious(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void Error(
#if NeedFunctionPrototypes
    char* /*str*/
#endif
);

extern CARD32 GetTimeInMillis(
#if NeedFunctionPrototypes
    void
#endif
);

extern int AdjustWaitForDelay(
#if NeedFunctionPrototypes
    pointer /*waitTime*/,
    unsigned long /*newdelay*/
#endif
);

extern SIGVAL AutoResetServer(
#if NeedFunctionPrototypes
    int /*sig*/
#endif
);

extern SIGVAL GiveUp(
#if NeedFunctionPrototypes
    int /*sig*/
#endif
);

extern void UseMsg(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ProcessCommandLine(
#if NeedFunctionPrototypes
    int /*argc*/,
    char* /*argv*/[]
#endif
);

extern unsigned long *Xalloc(
#if NeedFunctionPrototypes
    unsigned long /*amount*/
#endif
);

extern unsigned long *Xcalloc(
#if NeedFunctionPrototypes
    unsigned long /*amount*/
#endif
);

extern unsigned long *Xrealloc(
#if NeedFunctionPrototypes
    pointer /*ptr*/,
    unsigned long /*amount*/
#endif
);

extern void Xfree(
#if NeedFunctionPrototypes
    pointer /*ptr*/
#endif
);

extern int OsInitAllocator(
#if NeedFunctionPrototypes
    void
#endif
);

typedef SIGVAL (*OsSigHandlerPtr)(
#if NeedFunctionPrototypes
    int /* sig */
#endif
);

extern OsSigHandlerPtr OsSignal(
#if NeedFunctionPrototypes
    int /* sig */,
    OsSigHandlerPtr /* handler */
#endif
);

extern void AuditF(
#if NeedVarargsPrototypes
    char* /*f*/,
    ...
#endif
);

extern void FatalError(
#if NeedVarargsPrototypes
    char* /*f*/,
    ...
#endif
);

extern void ErrorF(
#if NeedVarargsPrototypes
    char* /*f*/,
    ...
#endif
);

#endif /* OS_H */
