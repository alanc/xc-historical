/* $XConsortium: difsutils.c,v 1.8 93/08/24 18:49:40 gildea Exp $ */
/*
 * misc utility routines
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#define	XK_LATIN1
#include	<stdio.h>
#include	<ctype.h>
#include	"misc.h"
#include	"globals.h"
#include	"clientstr.h"
#include	"accstr.h"
#include	"fontstruct.h"
#include	<X11/keysymdef.h>

#include	"authstr.h"
#include	"auth.h"
#include	"client.h"

extern ClientPtr currentClient;
static FontResolutionPtr default_resolutions;
static int  num_resolutions;
static int  default_point_size;

AuthContextPtr
GetClientAuthorization()
{
    return currentClient->auth;
}

void
SetDefaultPointSize(ps)
    int         ps;
{
    int         i;

    default_point_size = ps;
    for (i = 0; i < num_resolutions; i++)
	default_resolutions[i].point_size = ps;
}

int
SetDefaultResolutions(str)
    char       *str;
{
    int         num,
                numr = 0,
                n;
    char       *s;
    FontResolutionPtr new,
                nr;
    int         state;

    s = str;
    while (*s) {		/* count commas */
	if (*s == ',')
	    numr++;
	s++;
    }

    if ((numr % 2) != 1) {	/* must be multiple of 2 + 1 */
	return FSBadResolution;
    }
    numr = (numr + 1) / 2;
    nr = new = (FontResolutionPtr) fsalloc(sizeof(FontResolutionRec)
					   * numr);
    if (!new)
	return FSBadAlloc;
    s = str;
    num = 0;
    state = 0;
    while (*s) {
	if (*s == ',') {
	    if (state == 0) {
		nr->x_resolution = num;
		state++;
	    } else {
		state = 0;
		nr->y_resolution = num;
		nr->point_size = default_point_size;
		nr++;
	    }
	    num = 0;
	    s++;
	    continue;
	}
	if (!isdigit(*s)) {
	    fsfree((char *) new);
	    return FSBadResolution;
	}
	n = *s - '0';
	num = num * 10 + n;
	s++;
    }

    /* do the last one */
    assert(state == 1);
    nr->y_resolution = num;
    nr->point_size = default_point_size;

    if (default_resolutions) {
	fsfree((char *) default_resolutions);
    }
    default_resolutions = new;
    num_resolutions = numr;
    return FSSuccess;
}

FontResolutionPtr
GetClientResolutions(num)
    int        *num;
{
    /* return the client's if it has them, otherwise the default values */
    if (currentClient->num_resolutions) {
	*num = currentClient->num_resolutions;
	return (FontResolutionPtr) currentClient->resolutions;
    } else {
	*num = num_resolutions;
	return default_resolutions;
    }
}

int
GetDefaultPointSize()
{
    FontResolutionPtr res;
    int         num;

    res = GetClientResolutions(&num);
    if (res)
	return res->point_size;
    else
	return 120;
}

void
CopyISOLatin1Lowered(dest, source, length)
    register unsigned char *dest,
               *source;
    int         length;
{
    register int i;

    for (i = 0; i < length; i++, source++, dest++) {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}

int
strncmpnocase(first, second, n)
    char       *first,
               *second;
    int         n;
{
    register unsigned char *ap,
               *bp;

    for (ap = (unsigned char *) first,
	    bp = (unsigned char *) second;
    /* SUPPRESS 112 */
	    n > 0 && *ap && *bp; n--, ap++, bp++) {
	register unsigned char a,
	            b;

	/* SUPPRESS 112 */
	if ((a = *ap) != (b = *bp)) {
	    /* try lowercasing and try again */

	    if ((a >= XK_A) && (a <= XK_Z))
		a += (XK_a - XK_A);
	    else if ((a >= XK_Agrave) && (a <= XK_Odiaeresis))
		a += (XK_agrave - XK_Agrave);
	    else if ((a >= XK_Ooblique) && (a <= XK_Thorn))
		a += (XK_oslash - XK_Ooblique);

	    if ((b >= XK_A) && (b <= XK_Z))
		b += (XK_a - XK_A);
	    else if ((b >= XK_Agrave) && (b <= XK_Odiaeresis))
		b += (XK_agrave - XK_Agrave);
	    else if ((b >= XK_Ooblique) && (b <= XK_Thorn))
		b += (XK_oslash - XK_Ooblique);

	    if (a != b)
		break;
	}
    }
    /* SUPPRESS 112 */
    return (n ? (((int) *ap) - ((int) *bp)) : 0);
}

void
NoopDDA()
{
}

/* host list manipulation */
int
AddHost(list, addr)
    HostList   *list;
    HostAddress *addr;
{
    HostAddress *new;

    new = (HostAddress *) fsalloc(sizeof(HostAddress));
    if (!new)
	return FSBadAlloc;
    new->address = (pointer) fsalloc(addr->addr_len);
    if (!new->address) {
	fsfree((char *) addr);
	return FSBadAlloc;
    }
    new->type = addr->type;
    new->addr_len = addr->addr_len;
    memmove( (char *) new->address, (char *) addr->address, new->addr_len);

    new->next = *list;
    *list = new;
    return FSSuccess;
}

int
RemoveHost(list, addr)
    HostList   *list;
    HostAddress *addr;
{
    HostAddress *t,
               *last;

    last = (HostAddress *) 0;
    t = *list;
    while (t) {
	if (t->type == addr->type &&
		t->addr_len == addr->addr_len &&
		memcmp((char *) t->address, (char *) addr->address,
		     min(t->addr_len, addr->addr_len)) == 0) {
	    if (last) {
		last->next = t->next;
	    } else {
		*list = t->next;
	    }
	    fsfree((char *) t->address);
	    fsfree((char *) t);
	    return FSSuccess;
	}
	last = t;
	t = t->next;
    }
    return FSBadName;		/* bad host name */
}

Bool
ValidHost(list, addr)
    HostList    list;
    HostAddress *addr;
{
    HostAddress *t;

    t = list;
    while (t) {
	if (t->type == addr->type &&
		t->addr_len == addr->addr_len &&
		memcmp((char *) t->address, (char *) addr->address,
		     min(t->addr_len, addr->addr_len)) == 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

/* block & wakeup handlers */

typedef struct _BlockHandler {
    void        (*BlockHandler) ();
    void        (*WakeupHandler) ();
    pointer     blockData;
    Bool        deleted;
}           BlockHandlerRec, *BlockHandlerPtr;

static BlockHandlerPtr handlers;
static int  numHandlers;
static int  sizeHandlers;
static Bool inHandler;
static Bool handlerDeleted;

/* called from the OS layer */
BlockHandler(pTimeout, pReadmask)
    pointer     pTimeout;	/* DIX doesn't want to know how OS represents
				 * time */
    pointer     pReadmask;	/* nor how it represents the set of
				 * descriptors */
{
    register int i,
                j;

    ++inHandler;
    for (i = 0; i < numHandlers; i++)
	(*handlers[i].BlockHandler) (handlers[i].blockData,
				     pTimeout, pReadmask);
    if (handlerDeleted) {
	for (i = 0; i < numHandlers;)
	    if (handlers[i].deleted) {
		for (j = i; j < numHandlers - 1; j++)
		    handlers[j] = handlers[j + 1];
		numHandlers--;
	    } else
		i++;
    }
    --inHandler;
}


WakeupHandler(result, pReadmask)
    unsigned long result;	/* 32 bits of undefined result from the wait */
    pointer     pReadmask;	/* the resulting descriptor mask */
{
    register int i,
                j;

    ++inHandler;
    for (i = numHandlers - 1; i >= 0; i--)
	(*handlers[i].WakeupHandler) (handlers[i].blockData,
				      result, pReadmask);
    if (handlerDeleted) {
	for (i = 0; i < numHandlers;)
	    if (handlers[i].deleted) {
		for (j = i; j < numHandlers - 1; j++)
		    handlers[j] = handlers[j + 1];
		numHandlers--;
	    } else
		i++;
    }
    --inHandler;
}

/* Reentrant with BlockHandler and WakeupHandler, except wakeup won't
 * get called until next time
 */

Bool
RegisterBlockAndWakeupHandlers(blockHandler, wakeupHandler, blockData)
    void        (*blockHandler) ();
    void        (*wakeupHandler) ();
    pointer     blockData;
{
    BlockHandlerPtr new;

    if (numHandlers >= sizeHandlers) {
	new = (BlockHandlerPtr) fsrealloc(handlers, (numHandlers + 1) *
					  sizeof(BlockHandlerRec));
	if (!new)
	    return FALSE;
	handlers = new;
	sizeHandlers = numHandlers + 1;
    }
    handlers[numHandlers].BlockHandler = blockHandler;
    handlers[numHandlers].WakeupHandler = wakeupHandler;
    handlers[numHandlers].blockData = blockData;
    numHandlers = numHandlers + 1;
    return TRUE;
}

void
RemoveBlockAndWakeupHandlers(blockHandler, wakeupHandler, blockData)
    void        (*blockHandler) ();
    void        (*wakeupHandler) ();
    pointer     blockData;
{
    int         i;

    for (i = 0; i < numHandlers; i++)
	if (handlers[i].BlockHandler == blockHandler &&
		handlers[i].WakeupHandler == wakeupHandler &&
		handlers[i].blockData == blockData) {
	    if (inHandler) {
		handlerDeleted = TRUE;
		handlers[i].deleted = TRUE;
	    } else {
		for (; i < numHandlers - 1; i++)
		    handlers[i] = handlers[i + 1];
		numHandlers--;
	    }
	    break;
	}
}

InitBlockAndWakeupHandlers()
{
    fsfree(handlers);
    handlers = (BlockHandlerPtr) 0;
    numHandlers = 0;
    sizeHandlers = 0;
}

/*
 * A general work queue.  Perform some task before the server
 * sleeps for input.
 */

WorkQueuePtr workQueue;
static WorkQueuePtr *workQueueLast = &workQueue;

/* ARGSUSED */
void
ProcessWorkQueue()
{
    WorkQueuePtr q,
                n,
                p;

    p = NULL;
    /*
     * Scan the work queue once, calling each function.  Those which return
     * TRUE are removed from the queue, otherwise they will be called again.
     * This must be reentrant with QueueWorkProc, hence the crufty usage of
     * variables.
     */
    for (q = workQueue; q; q = n) {
	if ((*q->function) (q->client, q->closure)) {
	    /* remove q from the list */
	    n = q->next;	/* don't fetch until after func called */
	    if (p)
		p->next = n;
	    else
		workQueue = n;
	    fsfree(q);
	} else {
	    n = q->next;	/* don't fetch until after func called */
	    p = q;
	}
    }
    if (p)
	workQueueLast = &p->next;
    else {
	workQueueLast = &workQueue;
    }
}

Bool
QueueWorkProc(function, client, data)
    Bool        (*function) ();
    ClientPtr   client;
    pointer     data;
{
    WorkQueuePtr q;

    q = (WorkQueuePtr) fsalloc(sizeof *q);
    if (!q)
	return FALSE;
    q->function = function;
    q->client = client;
    q->closure = data;
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
    struct _SleepQueue *next;
    ClientPtr   client;
    Bool        (*function) ();
    pointer     closure;
}           SleepQueueRec, *SleepQueuePtr;

static SleepQueuePtr sleepQueue = NULL;

Bool
ClientSleep(client, function, data)
    ClientPtr   client;
    Bool        (*function) ();
    pointer     data;
{
    SleepQueuePtr q;

    q = (SleepQueuePtr) fsalloc(sizeof *q);
    if (!q)
	return FALSE;

    IgnoreClient(client);
    q->next = sleepQueue;
    q->client = client;
    q->function = function;
    q->closure = data;
    sleepQueue = q;
    return TRUE;
}

Bool
ClientSignal(client)
    ClientPtr   client;
{
    SleepQueuePtr q;

    for (q = sleepQueue; q; q = q->next)
	if (q->client == client) {
	    return QueueWorkProc(q->function, q->client, q->closure);
	}
    return FALSE;
}

ClientWakeup(client)
    ClientPtr   client;
{
    SleepQueuePtr q,
               *prev;

    prev = &sleepQueue;
    while ((q = *prev) != (SleepQueuePtr) 0) {
	if (q->client == client) {
	    *prev = q->next;
	    fsfree(q);
	    if (client->clientGone == CLIENT_GONE)
		CloseDownClient(client);
	    else
		AttendClient(client);
	    break;
	}
	prev = &q->next;
    }
}

Bool
ClientIsAsleep(client)
    ClientPtr   client;
{
    SleepQueuePtr q;

    for (q = sleepQueue; q; q = q->next)
	if (q->client == client)
	    return TRUE;
    return FALSE;
}

unsigned long *
Xalloc(m)
    unsigned long m;
{
    return fsalloc(m);
}

unsigned long *
Xrealloc(n, m)
    unsigned long *n,
                m;
{
    return fsrealloc(n, m);
}

void
Xfree(n)
    unsigned long *n;
{
    fsfree(n);
}

int
set_font_authorizations(authorizations, authlen, client)
char **authorizations;
int *authlen;
ClientPtr client;
{
#define AUTH1_NAME "hp-hostname-1"
#define AUTH2_NAME "hp-printername-1"
    static char result[1024];
    char *p;
    AuthContextPtr acp = client->auth;
    int len1, len2;

    if (acp != NULL && acp->authname != NULL && acp->authdata != NULL &&
	(!strcmp(AUTH1_NAME, acp->authname) ||
	 !strcmp(AUTH2_NAME, acp->authname)) &&
	(len1 = strlen(acp->authname) + 1) +
	(len2 = strlen(acp->authdata) + 1) + 2 * sizeof(short) <= 1024)
    {
	p = result;
	*p++ = len1 >> 8;
	*p++ = len1 &0xff;
	*p++ = len2 >> 8;
	*p++ = len2 & 0xff;
	memmove( p, acp->authname, len1);
	p += len1;
	memmove( p, acp->authdata, len2);
	p += len2;
	*authlen = p - result;
	*authorizations = result;
	return 1;
    }

    *authlen = 0;
    return 0;
}

int
client_auth_generation(client)
ClientPtr client;
{
    return client->auth_generation;
}
