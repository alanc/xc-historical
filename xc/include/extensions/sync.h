/*
 * @DEC_COPYRIGHT@
 */
/*
 * HISTORY
 * Changes to C64Low32 and C64High32
 * $Log: sync.h,v $
 * Revision 1.1.1.3  1993/05/06  13:13:03  Pete_Snider
 * 	remove the typedef of CARD64, typedef in Xmd.h
 *
 * Revision 1.1.1.2  1993/05/05  17:29:01  Pete_Snider
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

#ifndef _SYNC_H_
#define _SYNC_H_

#define SYNCNAME "SYNC"

#define SYNC_MAJOR_VERSION	2
#define SYNC_MINOR_VERSION	0

#define X_SyncInfo			0
#define X_SyncCreateCounter		1
#define X_SyncSetCounter		2
#define X_SyncChangeCounter		3
#define X_SyncQueryCounter              4
#define X_SyncDestroyCounter		5
#define X_SyncAwait			6
#define X_SyncCreateAlarm               7
#define X_SyncChangeAlarm	        8
#define X_SyncQueryAlarm	        9
#define X_SyncDestroyAlarm	       10
#define X_SyncSetPriority   	       11
#define X_SyncGetPriority   	       12

#define CounterNotify                   0
#define AlarmNotify			1
#define AlarmNotifyMask 		(1L << AlarmNotify)

#define SyncNumberEvents		2L

#define BadCounter			0L
#define BadAlarm			1L
#define SyncNumberErrors		(BadAlarm + 1)

/*
 * Flags for Alarm Attributes
 */
#define CACounter			(1L<<0)
#define CAValueType			(1L<<1)
#define CAValue				(1L<<2)
#define CATestType			(1L<<3)
#define CADelta				(1L<<4)
#define CAEvents			(1L<<5)

/*
 * Constants for the value_type argument of various requests
 */
#define SyncAbsolute		1
#define SyncRelative		2

/*
 * Alarm Test types
 */
#define PositiveTransition	1
#define NegativeTransition	2
#define PositiveComparison	3
#define NegativeComparison	4

/*
 * Alarm state constants
 */
#define AlarmActive			1
#define AlarmInactive			2
#define AlarmDestroyed			3

typedef CARD8  ALARMSTATE;
typedef CARD32 COUNTER;
typedef CARD32 VALUETYPE;
typedef CARD32 TESTTYPE;
typedef XID    ALARM;


#if LONG_BIT == 64
/*
 * XXX: This code is untested, for obvious reasons!
 */

/* defined in Xmd.h
typedef long CARD64;
*/

#define cvtLongToC64(c64, i)	(c64 = (i))
#define cvtC64ToLong(c64)	(c64)
#define cvtINT32sToC64(c64, l, h) (c64) = ((long)(h)<<32) | (0x0ffffffff & (l))
#define C64GreaterThan(a, b)	((a)>(b))
#define C64LessThan(a, b)	((a)<(b))
#define C64GreaterOrEqual(a, b)	((a)>=(b))
#define C64LessOrEqual(a, b)	((a)<=(b))
#define C64Equal(a, b)		((a)==(b))
#define C64IsZero(a)		((a)==0)
#define C64Low32(c64)		((unsigned int)(c64))
#define C64High32(c64)		((unsigned long)(c64>>32))
#define C64Add(res,a,b)		((long)(res) = (long)(a) + (long)(b))
#define C64Subtract(res,a,b)	((long)(res) = (long)(a) - (long)(b))
#define MAXCARD64(v)		((v)=0xFFFFFFFFFFFFFFFF);

#else /* LongIs64Bits */

#ifndef CARD64
typedef struct CARD64Struct {
    unsigned long lo;
    unsigned long hi;
} CARD64;
#endif

#define cvtLongToC64(c64, i)	((c64).hi=0,(c64).lo=(i))
#define cvtC64ToLong(c64)	((c64).lo)
#define cvtINT32sToC64(c64, l, h) ((c64).lo = (l), (c64).hi = (h))
#define C64GreaterThan(a, b)\
    ((a).hi>(b).hi || ((a).hi==(b).hi && (a).lo>(b).lo))
#define C64LessThan(a, b)\
    ((a).hi<(b).hi || ((a).hi==(b).hi && (a).lo<(b).lo))
#define C64GreaterOrEqual(a, b)\
    ((a).hi>(b).hi || ((a).hi==(b).hi && (a).lo>=(b).lo))
#define C64LessOrEqual(a, b)\
    ((a).hi<(b).hi || ((a).hi==(b).hi && (a).lo<=(b).lo))
#define C64Equal(a, b)		((a).lo==(b).lo && (a).hi==(b).hi)
#define C64IsZero(a)		((a).lo==0 && (a).hi==0)
#define C64Low32(c64)		((c64).lo)
#define C64High32(c64)		((c64).hi)
#define C64Add(res,a,b) {\
	int t = (a).lo;\
	((res).lo = (a).lo + (b).lo);\
	((res).hi = (a).hi + (b).hi);\
	if (t>(res).lo) (res).hi++;\
     }
#define C64Subtract(res,a,b) {\
	int t = (a).lo;\
	((res).lo = (a).lo - (b).lo);\
	((res).hi = (a).hi - (b).hi);\
	if (t>(res).lo) (res).hi--;\
     }
#define MAXCARD64(v)		((v).lo=0xFFFFFFFF,(v).hi=0xFFFFFFFF);

#endif /* LongIs64Bits */

typedef struct _SystemCounterRec {
    char *name;
    XID counter;
    CARD64 resolution;
} SystemCounterRec, *SystemCounterList;


typedef struct {
    int counter;  /* PDS check out */
    int wait_type;
    CARD64 wait_value;
    int test_type;
} XWaitCondition, *XWaitConditionList;

typedef struct {
    COUNTER	counter B32;
    VALUETYPE	wait_type B32;
    CARD64	wait_value;
    TESTTYPE	test_type B32;
} TRIGGER;

typedef struct {
	TRIGGER   trigger;
	CARD64	  event_threshold B32;
} WAITCONDITION;


#ifndef _SYNC_SERVER

typedef int Counter; /* PDS Check */
typedef XID Alarm;  /* PDS Check */

typedef struct {
    TRIGGER trigger;
    CARD64  delta;
    int events;
    int state;
} XSyncAlarmAttributes;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    COUNTER counter;
    CARD64 wait_value;
    CARD64 counter_value;
    Time time;
    CARD16 count;
    BOOL destroyed;
    BYTE pad;
} XCounterNotifyEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Alarm alarm;
    CARD64 counter_value;
    CARD64 alarm_value;
    Time time;
    ALARMSTATE state;
    BYTE pad0;
    BYTE pad1;
    BYTE pad2;
} XAlarmNotifyEvent;

extern Bool XSyncQueryExtension(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    unsigned long*	/* event_return */,
    unsigned long*	/* error_return */
#endif
);

extern Status XSyncInfo(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    int*		/* major_return */,
    int*		/* minor_return */,
    SystemCounterRec**	/* list_return */,
    unsigned long*     	/* n_counters_return */
#endif
);

extern Counter XSyncCreateCounter(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Counter*            /* counter_return */,
    CARD64	        /* initial_value */
#endif
);

extern Status XSyncSetCounter(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Counter             /* counter */,
    CARD64	        /* value */
#endif
);

extern Status XSyncChangeCounter(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Counter             /* counter */,
    CARD64	        /* value */
#endif
);

extern Status XSyncDestroyCounter(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Counter		/* counter */
#endif
);

extern Status XSyncQueryCounter(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Counter		/* counter */,
    CARD64*	        /* value_return */
#endif
);

extern Status XSyncAwait(
#if NeedFunctionPrototypes
    Display *           /* dpy */,
    XWaitCondition *  /* wait_list */,
    unsigned long       /* n_conditions */
#endif
);

extern Alarm XSyncCreateAlarm(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Alarm*              /* alarm_return */,
    unsigned long       /* values_mask */,
    XSyncAlarmAttributes* /* values */
#endif
);

extern Status XSyncDestroyAlarm(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Alarm		/* alarm*/
#endif
);

extern Status XSyncQueryAlarm(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Alarm		/* alarm */,
    XSyncAlarmAttributes* /* values_return */
#endif
);

extern Status XSyncChangeAlarm(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Alarm		/* alarm */,
    unsigned long	/* values_mask */,
    XSyncAlarmAttributes* /* values */
#endif
);

#endif /* _SYNC_SERVER */

#endif /* _SYNC_H_ */
