/*
 * @DEC_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: syncstr.h,v $
 * Revision 1.1.1.2  1993/05/05  17:31:47  Pete_Snider
 * 	initial rcs submit
 *
 * $EndLog$
 */

/***********************************************************
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
the Massachusetts Institute of Technology, Cambridge, Massachusetts,
and Olivetti Research Limited, Cambridge, England.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital, Olivetti or MIT
not be used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL AND OLIVETTI DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL THEY BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#include "sync.h"

#ifndef _SYNC_OS

/*
 * Sync Info
 */
typedef struct _xSyncInfoReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
} xSyncInfoReq;
#define sz_xSyncInfoReq		4

typedef struct {
    BYTE	type;
    CARD8	unused;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	majorVersion;
    CARD8	minorVersion;
    CARD16	pad0 B16;
    CARD32	nCounters;		/* followed by system counters */
    CARD32  	pad1 B32;
    CARD32  	pad2 B32;
    CARD32  	pad3 B32;
    CARD32  	pad4 B32;
} xSyncInfoReply;
#define sz_xSyncInfoReply	32

/*
 * Create Counter
 */
typedef struct _xSyncCreateCounterReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	cid B32;
    CARD32	initial_value_lo B32;
    CARD32      initial_value_hi B32;
} xSyncCreateCounterReq;
#define sz_xSyncCreateCounterReq	16

/*
 * Change Counter
 */
typedef struct _xSyncChangeCounterReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	cid B32;
    CARD32	value_lo B32;
    CARD32      value_hi B32;
} xSyncChangeCounterReq;
#define sz_xSyncChangeCounterReq	16

/*
 * Set Counter
 */
typedef struct _xSyncSetCounterReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	cid B32;
    CARD32	value_lo B32;
    CARD32      value_hi B32;
} xSyncSetCounterReq;
#define sz_xSyncSetCounterReq	16

/*
 * Destroy Counter
 */
typedef struct _xSyncDestroyCounterReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	counter B32;
} xSyncDestroyCounterReq;
#define sz_xSyncDestroyCounterReq	8

/*
 * Query Counter
 */
typedef struct _xSyncQueryCounterReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	counter B32;
} xSyncQueryCounterReq;
#define sz_xSyncQueryCounterReq		8


typedef struct {
    BYTE	type;
    CARD8	unused;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	value_lo B32;
    CARD32	value_hi B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
} xSyncQueryCounterReply;
#define sz_xSyncQueryCounterReply	32

/*
 * Await
 */
typedef struct _xSyncAwaitReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
} xSyncAwaitReq;
#define sz_xSyncAwaitReq		4

/*
 * Create Alarm
 */
typedef struct _xSyncCreateAlarmReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	id B32;
    CARD32      valueMask;
} xSyncCreateAlarmReq;
#define sz_xSyncCreateAlarmReq		12

/*
 * Destroy Alarm
 */
typedef struct _xSyncDestroyAlarmReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	alarm B32;
} xSyncDestroyAlarmReq;
#define sz_xSyncDestroyAlarmReq		8

typedef struct {
    CARD32	counter B32;
    CARD32	wait_type B32;
    CARD32	wait_value_lo B32;
    CARD32	wait_value_hi B32;
    CARD32	TestType      B32;
} xTrigger;

typedef struct {
    CARD32	counter B32;
    CARD32	wait_type B32;
    CARD64	wait_value;
    CARD32	test_type      B32;
} Trigger;

/*
 * Query Alarm
 */
typedef struct _xSyncQueryAlarmReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	alarm B32;
} xSyncQueryAlarmReq;
#define sz_xSyncQueryAlarmReq		8

typedef struct {
    BYTE	type;
    CARD8	unused;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	counter B32;
    CARD32	wait_type B32;
    CARD32	wait_value_lo B32;
    CARD32	wait_value_hi B32;
    CARD32	test_type      B32;
    CARD32	delta_lo B32;
    CARD32	delta_hi B32;
    BYTE        events;
    BYTE        state;
    BYTE	pad0;
    BYTE	pad1;
} xSyncQueryAlarmReply;
#define sz_xSyncQueryAlarmReply		40

/*
 * Change Alarm
 */
typedef struct _xSyncChangeAlarmReq {
    CARD8	reqType;
    CARD8	syncReqType;
    CARD16	length B16;
    CARD32	alarm B32;
    CARD32	valueMask B32;
} xSyncChangeAlarmReq;
#define sz_xSyncChangeAlarmReq		12

/*
 * SetPriority
 */
typedef struct _xSyncSetPriority{
    CARD8   	reqType;
    CARD8   	syncReqType;
    CARD16  	length;
    CARD32  	id;
    CARD32  	priority;
} xSyncSetPriorityReq;

#define sz_xSyncSetPriorityReq	    	12

/*
 * Get Priority
 */
typedef struct _xSyncGetPriority{
    CARD8   	reqType;
    CARD8   	syncReqType;
    CARD16  	length;
    CARD32  	id;
} xSyncGetPriorityReq;

#define sz_xSyncGetPriorityReq	    	 8

typedef struct {
    BYTE	type;
    CARD8	unused;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32  	priority B32;
    CARD32  	pad0 B32;
    CARD32  	pad1 B32;
    CARD32  	pad2 B32;
    CARD32  	pad3 B32;
    CARD32  	pad4 B32;
} xSyncGetPriorityReply;

#define sz_xSyncGetPriorityReply	32

/*
 * Events
 */

typedef struct _xCounterNotifyEvent {
    BYTE	type;
    BYTE	kind;
    CARD16	sequenceNumber B16;
    CARD32	counter B32;
    CARD32	wait_value_lo B32;
    CARD32	wait_value_hi B32;
    CARD32	counter_value_lo B32;
    CARD32	counter_value_hi B32;
    CARD32	time B32;
    CARD16	count B16;
    BOOL	destroyed;
    BYTE        pad0;
} xCounterNotifyEvent;

typedef struct _xAlarmNotifyEvent {
    BYTE	type;
    BYTE	kind;
    CARD16	sequenceNumber B16;
    CARD32	alarm B32;
    CARD32	counter_value_lo B32;
    CARD32	counter_value_hi B32;
    CARD32	alarm_value_lo B32;
    CARD32	alarm_value_hi B32;
    CARD32	time B32;
    BYTE        state;
    BYTE        pad0;
    BYTE        pad1;
    BYTE        pad2;
} xAlarmNotifyEvent;

#endif /* _SYNC_OS */

#ifdef _SYNC_SERVER

/*
 * The System Counter interface
 * XXX: This is rather broken at the moment
 */

struct _WaitCondition {
int	counter;
int	wait_type;
CARD64	wait_value;
int	test_type;
CARD64	event_threshold;
} WaitCondition;

typedef struct _SyncCounter {
    ClientPtr		owner;		/* Owning client. 0 for system counters */
    CARD32		id;		/* resource ID */
    CARD64		value;		/* counter value */
    struct _WaitList	*waitlist;	/* sorted list of clients */
    void	(*Query)(
#if NeedFunctionPrototypes
			 struct _SyncCounter  *,
			 CARD64 *
#endif
			 );
    void	(*NewWait)(
#if NeedFunctionPrototypes
			   struct _SyncCounter  *,
			   CARD64,
			   int,
			   CARD64 *
#endif
			   );
    void	(*NoWait)(
#if NeedFunctionPrototypes
			  struct _SyncCounter  *,
#endif
			  );
} SyncCounter;

typedef struct _AlarmClient {
    ClientPtr	client;
    CARD32		id;
    int	    	events;
    struct _AlarmClient *next;
} AlarmClient;

/* Warning: atleast the first 3 items MUST be the same between SyncAlarm &
   SyncAwait,  items 3 - 7 probably should be in a trigger structure  */

typedef struct _SyncAlarm {
    ClientPtr	client;
    CARD32 	id;
    SyncCounter *counter;
    CARD64	value;		/* wait value */
    int		value_type;     /* Absolute or Relative */
    int		test_type;	/* transition or Comparision type */
    CARD64	event_value;	/* trigger event threshold value */
    CARD64	delta;
    int		events;
    int		state;
    AlarmClient	*clients;
} SyncAlarm;

typedef struct _SyncAwait {
    ClientPtr	client;
    CARD32 	id;
    SyncCounter *counter;
    CARD64	value;		/* wait value */
    int		value_type;	/* Absolute or Relative */
    int		test_type;	/* transition or Comparision type */
    CARD64	event_value;	/* trigger event threshold value */
    struct _SyncAwait *next;	/* next await for the client */
} SyncAwait;

typedef struct _WaitList {
    struct _WaitList	*next;
    SyncAlarm	*pAlarm;
    int		is_alarm;
} WaitList;

typedef struct _SysCounter {
    char	*name;
    SyncCounter	*counter;
    CARD64	resolution;
} SysCounter;

extern pointer SyncCreateSystemCounter(
#if NeedFunctionPrototypes
    Atom		/* name */,
    CARD64  	/* inital_value */,
    CARD64  	/* resolution */,
    void	(*query)(),
    void	(*newwait)()
#endif
);

extern void SyncChangeCounter(
#if NeedFunctionPrototypes
    pointer		/* pCounter*/,
    CARD64  		/* new_value */
#endif
);

extern void InitServertime();

#endif /* _SYNC_SERVER */
