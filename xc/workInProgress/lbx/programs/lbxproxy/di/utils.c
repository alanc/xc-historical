/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
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
/* $XConsortium: utils.c,v 1.11 95/05/17 18:26:41 dpw Exp mor $ */
/* $NCDId: @(#)utils.c,v 1.8 1994/11/16 02:27:25 lemke Exp $ */

#include "lbx.h"
#include <stdio.h>
#ifdef X_POSIX_C_SOURCE
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <signal.h>
#undef _POSIX_C_SOURCE
#else
#if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <signal.h>
#else
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#endif
#endif
#ifndef SYSV
#include <sys/resource.h>
#endif

/* lifted from Xt/VarargsI.h */
#if NeedVarargsPrototypes
#include <stdarg.h>
#endif

#if NeedVarargsPrototypes
static void VErrorF(char*, va_list);
#endif

#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

#include "util.h"
#include "proxyopts.h"

extern char *display_name;
extern char *display;

extern Bool PartialNetwork;
extern Bool defeatAccessControl;

Bool CoreDump;
int auditTrailLevel = 1;

#ifdef DEBUG
#ifndef SPECIAL_MALLOC
#define MEMBUG
#endif
#endif

#ifdef MEMBUG
#define MEM_FAIL_SCALE 100000
long Memory_fail = 0;

#endif

Bool Must_have_memory = FALSE;

char *dev_tty_from_init = NULL;		/* since we need to parse it anyway */

OsSigHandlerPtr
OsSignal(sig, handler)
    int sig;
    OsSigHandlerPtr handler;
{
#ifdef X_NOT_POSIX
    return signal(sig, handler);
#else
    struct sigaction act, oact;

    sigemptyset(&act.sa_mask);
    if (handler != SIG_IGN)
	sigaddset(&act.sa_mask, sig);
    act.sa_flags = 0;
    act.sa_handler = handler;
    sigaction(sig, &act, &oact);
    return oact.sa_handler;
#endif
}

/* Force connections to close on SIGHUP from init */

SIGVAL
AutoResetServer (sig)
    int	sig;
{
    dispatchException |= DE_RESET;
    isItTimeToYield = TRUE;
#ifdef GPROF
    chdir ("/tmp");
    exit (0);
#endif
#ifdef SYSV
    signal (SIGHUP, AutoResetServer);
#endif
}

/* Force connections to close and then exit on SIGTERM, SIGINT */

SIGVAL
GiveUp(sig)
    int	sig;
{
    dispatchException |= DE_TERMINATE;
    isItTimeToYield = TRUE;
}


static void
AbortServer()
{
    fflush(stderr);
    if (CoreDump)
	abort();
    exit (1);
}

void
Error(str)
    char *str;
{
    perror(str);
}

#ifndef DDXTIME
CARD32
GetTimeInMillis()
{
    struct timeval  tp;

    X_GETTIMEOFDAY(&tp);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
#endif

void UseMsg()
{
#if !defined(AIXrt) && !defined(AIX386)
    ErrorF("use: lbxproxy [:<display>] [option]\n");
    ErrorF("-ac                    disable access control restrictions\n");
#ifdef MEMBUG
    ErrorF("-alloc int             chance alloc should fail\n");
#endif
/*
    ErrorF("-auth string           select authorization file\n");	
*/
    ErrorF("-help                  prints message with these options\n");
    ErrorF("-I                     ignore all remaining arguments\n");
    ErrorF("-to #                  connection time out\n");
#ifdef XDMCP
    XdmcpUseMsg();
#endif
#endif /* !AIXrt && ! AIX386 */
}

extern int  lbxTagCacheSize;
extern Bool lbxUseTags;
extern Bool lbxUseLbx;
extern Bool lbxDoSquishing;
extern Bool lbxCompressImages;
extern Bool lbxDoShortCircuiting;
extern Bool lbxDoLbxGfx;

static int
proxyProcessArgument (argc, argv, i)
    int argc;
    char    **argv;
    int i;
{
    if (strcmp (argv[i], "-debug") == 0)
    {
	if (++i < argc)
	    lbxDebug = atoi(argv[i]);
	else
	    UseMsg ();
	return 2;
    }
    if (strcmp (argv[i], "-nogfx") == 0)
    {
	lbxDoLbxGfx = 0;
	return 1;
    }
    if (strcmp (argv[i], "-nosc") == 0)
    {
	lbxDoShortCircuiting = 0;
	return 1;
    }
    if (strcmp (argv[i], "-nolzw") == 0)
    {
	LbxNoComp();
	return 1;
    }
    if (strcmp (argv[i], "-nocomp") == 0)
    {
	LbxNoComp();
	return 1;
    }
    if (strcmp (argv[i], "-nodelta") == 0)
    {
	LbxNoDelta();
	return 1;
    }
    if (strcmp (argv[i], "-notags") == 0)
    {
	lbxUseTags = 0;
	return 1;
    }
    if (strcmp (argv[i], "-nolbx") == 0)
    {
	lbxUseLbx = 0;
	return 1;
    }
    if (strcmp (argv[i], "-noimage") == 0)
    {
	lbxCompressImages = 0;
	return 1;
    }
    if (strcmp (argv[i], "-nosquish") == 0)
    {
	LbxNoSquish();
	return 1;
    }
    if (strcmp (argv[i], "-tagcachesize") == 0)
    {
	if (++i < argc)
	    lbxTagCacheSize = atoi(argv[i]);
	else
	    UseMsg ();
	return 2;
    }
    return 0;
}

/*
 * This function parses the command line. Handles device-independent fields
 * and allows ddx to handle additional fields.  It is not allowed to modify
 * argc or any of the strings pointed to by argv.
 */
void
ProcessCommandLine ( argc, argv )
int	argc;
char	*argv[];

{
    int i, skip;

    for ( i = 1; i < argc; i++ )
    {
	/* do proxy-specific stuff first */
        if(skip = proxyProcessArgument(argc, argv, i))
	{
	    i += (skip - 1);
	}
	else if(argv[i][0] ==  ':')  
	{
	    /* initialize display */
	    display = argv[i];
	    display++;
	}
	else if ( strcmp( argv[i], "-ac") == 0)
	{
	    defeatAccessControl = TRUE;
	}
#ifdef MEMBUG
	else if ( strcmp( argv[i], "-alloc") == 0)
	{
	    if(++i < argc)
	        Memory_fail = atoi(argv[i]);
	    else
		UseMsg();
	}
#endif
#ifdef old
	else if ( strcmp( argv[i], "-auth") == 0)
	{
	    if(++i < argc)
	        InitAuthorization (argv[i]);
	    else
		UseMsg();
	}
#endif
	else if ( strcmp( argv[i], "-display") == 0)
	{
	    if(++i < argc)
	        display_name = argv[i];
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-core") == 0)
	    CoreDump = TRUE;
	else if ( strcmp( argv[i], "-help") == 0)
	{
	    UseMsg();
	    exit(0);
	}
	else if ( strcmp( argv[i], "-pn") == 0)
	    PartialNetwork = TRUE;
	else if ( strcmp( argv[i], "-to") == 0)
	{
	    if(++i < argc)
		TimeOutValue = ((long)atoi(argv[i])) * MILLI_PER_SECOND;
	    else
		UseMsg();
	}
	else if ( strcmp( argv[i], "-terminate") == 0)
	{
	    extern Bool terminateAtReset;
	    
	    terminateAtReset = TRUE;
	}
	else if ( strcmp( argv[i], "-I") == 0)
	{
	    /* ignore all remaining arguments */
	    break;
	}
	else if (strncmp (argv[i], "tty", 3) == 0)
	{
	    /* just in case any body is interested */
	    dev_tty_from_init = argv[i];
	}
#ifdef XDMCP
	else if ((skip = XdmcpOptions(argc, argv, i)) != i)
	{
	    i = skip - 1;
	}
#endif
 	else
 	{
	    UseMsg();
	    exit (1);
        }
    }
}

/* XALLOC -- X's internal memory allocator.  Why does it return unsigned
 * int * instead of the more common char *?  Well, if you read K&R you'll
 * see they say that alloc must return a pointer "suitable for conversion"
 * to whatever type you really want.  In a full-blown generic allocator
 * there's no way to solve the alignment problems without potentially
 * wasting lots of space.  But we have a more limited problem. We know
 * we're only ever returning pointers to structures which will have to
 * be long word aligned.  So we are making a stronger guarantee.  It might
 * have made sense to make Xalloc return char * to conform with people's
 * expectations of malloc, but this makes lint happier.
 */

unsigned long * 
Xalloc (amount)
    unsigned long amount;
{
    char		*malloc();
    register pointer  ptr;
	
    if ((long)amount <= 0)
	return (unsigned long *)NULL;
    /* aligned extra on long word boundary */
    amount = (amount + 3) & ~3;
#ifdef MEMBUG
    if (!Must_have_memory && Memory_fail &&
	((random() % MEM_FAIL_SCALE) < Memory_fail))
	return (unsigned long *)NULL;
#endif
    if (ptr = (pointer)malloc(amount))
	return (unsigned long *)ptr;
    if (Must_have_memory)
	FatalError("Out of memory");
    return (unsigned long *)NULL;
}

/*****************
 * Xcalloc
 *****************/

unsigned long *
Xcalloc (amount)
    unsigned long   amount;
{
    unsigned long   *ret;

    ret = Xalloc (amount);
    if (ret)
	bzero ((char *) ret, (int) amount);
    return ret;
}

/*****************
 * Xrealloc
 *****************/

unsigned long *
Xrealloc (ptr, amount)
    register pointer ptr;
    unsigned long amount;
{
    char *malloc();
    char *realloc();

#ifdef MEMBUG
    if (!Must_have_memory && Memory_fail &&
	((random() % MEM_FAIL_SCALE) < Memory_fail))
	return (unsigned long *)NULL;
#endif
    if ((long)amount <= 0)
    {
	if (ptr && !amount)
	    free(ptr);
	return (unsigned long *)NULL;
    }
    amount = (amount + 3) & ~3;
    if (ptr)
        ptr = (pointer)realloc((char *)ptr, amount);
    else
	ptr = (pointer)malloc(amount);
    if (ptr)
        return (unsigned long *)ptr;
    if (Must_have_memory)
	FatalError("Out of memory");
    return (unsigned long *)NULL;
}
                    
/*****************
 *  Xfree
 *    calls free 
 *****************/    

void
Xfree(ptr)
    register pointer ptr;
{
    if (ptr)
	free((char *)ptr); 
}

void
OsInitAllocator ()
{
#ifdef MEMBUG
    static int	been_here;

    /* Check the memory system after each generation */
    if (been_here)
	CheckMemory ();
    else
	been_here = 1;
#endif
}

/*VARARGS1*/
void
AuditF(
#if NeedVarargsPrototypes
    char * f, ...)
#else
 f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
#endif
{
#ifdef notyet		/* ever ? */
#ifdef X_NOT_STDC_ENV
    long tm;
#else
    time_t tm;
#endif
    char *autime, *s;
#if NeedVarargsPrototypes
    va_list args;
#endif

    if (*f != ' ')
    {
	time(&tm);
	autime = ctime(&tm);
	if (s = strchr(autime, '\n'))
	    *s = '\0';
	if (s = strrchr(argvGlobal[0], '/'))
	    s++;
	else
	    s = argvGlobal[0];
	ErrorF("AUDIT: %s: %d %s: ", autime, getpid(), s);
    }
#if NeedVarargsPrototypes
    va_start(args, f);
    VErrorF(f, args);
    va_end(args);
#else
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
#endif
#endif
}

/*VARARGS1*/
void
FatalError(
#if NeedVarargsPrototypes
    char *f, ...)
#else
f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
#endif
{
#if NeedVarargsPrototypes
    va_list args;
#endif
    ErrorF("\nFatal proxy error:\n");
#if NeedVarargsPrototypes
    va_start(args, f);
    VErrorF(f, args);
    va_end(args);
#else
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
#endif
    ErrorF("\n");
    AbortServer();
    /*NOTREACHED*/
}

#if NeedVarargsPrototypes
static void
VErrorF(f, args)
    char *f;
    va_list args;
{
    vfprintf(stderr, f, args);
}
#endif

/*VARARGS1*/
void
ErrorF(
#if NeedVarargsPrototypes
    char * f, ...)
#else
 f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
#endif
{
#if NeedVarargsPrototypes
    va_list args;
    va_start(args, f);
    VErrorF(f, args);
    va_end(args);
#else
    fprintf( stderr, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
#endif
}

char *
strnalloc(str, len)
    char       *str;
    int 	len;
{
    char       *t;

    t = (char *) Xalloc(len);
    if (!t)
	return (char *) 0;
    memcpy(t, str, len);
    return t;
}

/*
 * A general work queue.  Perform some task before the server
 * sleeps for input.
 */

typedef struct _WorkQueue {
    struct _WorkQueue *next;
    Bool        (*function) (
#if NeedNestedPrototypes
		ClientPtr	/* pClient */,
		pointer		/* closure */
#endif
);
    ClientPtr   client;
    pointer     closure;
}           WorkQueueRec;

WorkQueuePtr		workQueue;
static WorkQueuePtr	*workQueueLast = &workQueue;

/* ARGSUSED */
void
ProcessWorkQueue()
{
    WorkQueuePtr    q, n, p;

    p = NULL;
    /*
     * Scan the work queue once, calling each function.  Those
     * which return TRUE are removed from the queue, otherwise
     * they will be called again.  This must be reentrant with
     * QueueWorkProc, hence the crufty usage of variables.
     */
    for (q = workQueue; q; q = n)
    {
	if ((*q->function) (q->client, q->closure))
	{
	    /* remove q from the list */
	    n = q->next;    /* don't fetch until after func called */
	    if (p)
		p->next = n;
	    else
		workQueue = n;
	    xfree (q);
	}
	else
	{
	    n = q->next;    /* don't fetch until after func called */
	    p = q;
	}
    }
    if (p)
	workQueueLast = &p->next;
    else
    {
	workQueueLast = &workQueue;
    }
}

Bool
QueueWorkProc (function, client, closure)
    Bool	(*function)();
    ClientPtr	client;
    pointer	closure;
{
    WorkQueuePtr    q;

    q = (WorkQueuePtr) xalloc (sizeof *q);
    if (!q)
	return FALSE;
    q->function = function;
    q->client = client;
    q->closure = closure;
    q->next = NULL;
    *workQueueLast = q;
    workQueueLast = &q->next;
    return TRUE;
}

/*
 * Manage a queue of sleeping clients, awakening them
 * when requested, by using the OS functions IgnoreClient
 * and AttendClient.  Note that this *ignores* the troubles
 * with request data interleaving itself with events, but
 * we'll leave that until a later time.
 */

typedef struct _SleepQueue {
    struct _SleepQueue	*next;
    ClientPtr		client;
    Bool		(*function)();
    pointer		closure;
} SleepQueueRec, *SleepQueuePtr;

static SleepQueuePtr	sleepQueue = NULL;

Bool
ClientSleep (client, function, closure)
    ClientPtr	client;
    Bool	(*function)();
    pointer	closure;
{
    SleepQueuePtr   q;

    q = (SleepQueuePtr) xalloc (sizeof *q);
    if (!q)
	return FALSE;

    IgnoreClient (client);
    q->next = sleepQueue;
    q->client = client;
    q->function = function;
    q->closure = closure;
    sleepQueue = q;
    return TRUE;
}

Bool
ClientSignal (client)
    ClientPtr	client;
{
    SleepQueuePtr   q;

    for (q = sleepQueue; q; q = q->next)
	if (q->client == client)
	{
	    return QueueWorkProc (q->function, q->client, q->closure);
	}
    return FALSE;
}

void
ClientWakeup (client)
    ClientPtr	client;
{
    SleepQueuePtr   q, *prev;

    prev = &sleepQueue;
    while (q = *prev)
    {
	if (q->client == client)
	{
	    *prev = q->next;
	    xfree (q);
	    if (!client->clientGone)
		AttendClient (client);
	    break;
	}
	prev = &q->next;
    }
}

Bool
ClientIsAsleep (client)
    ClientPtr	client;
{
    SleepQueuePtr   q;

    for (q = sleepQueue; q; q = q->next)
	if (q->client == client)
	    return TRUE;
    return FALSE;
}
