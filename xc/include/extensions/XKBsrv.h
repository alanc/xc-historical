/* $XConsortium: XKBsrv.h,v 1.6 93/09/28 19:48:09 rws Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifndef _XKBSRV_H_
#define	_XKBSRV_H_

#include <X11/extensions/XKBstr.h>

typedef struct _XkbInterestRec {
	struct _XkbSrvInfoRec	*kbd;
	ClientRec		*client;
	CARD16			 stateNotifyMask;
	CARD16			 namesNotifyMask;
	CARD32 			 controlsNotifyMask;
	CARD8			 compatNotifyMask;
	BOOL			 bellNotifyWanted;
	CARD8			 slowKeyNotifyMask;
	CARD32			 iStateNotifyMask;
	CARD32			 iMapNotifyMask;
	CARD16			 altSymsNotifyMask;
	XID			 resource;
	struct _XkbInterestRec	*next;
} XkbInterestRec,*XkbInterestPtr;

typedef struct _XkbRadioGroup {
	CARD8		flags;
	CARD8		nMembers;
	CARD8		dfltDown;
	CARD8		currentDown;
	CARD8		members[XkbRGMaxMembers];
} XkbRadioGroupRec, *XkbRadioGroupPtr;

#define	_OFF_TIMER	0
#define	_KRG_TIMER	1
#define	_TIMEOUT_TIMER	2

typedef struct _XkbSrvInfoRec {
	XkbStateRec	 state;
	XkbControlsRec	 Controls;
	XkbServerMapRec	 Server;
	XkbClientMapRec	 Map;
	XkbIndicatorRec	 Indicators;
	XkbNamesRec	 Names;
	XkbCompatRec	 Compat;
	XkbDescRec	 desc;

	DeviceIntRec	*device;

	XkbRadioGroupRec *radioGroups;
	CARD8		 nRadioGroups;
	CARD8		 clearMods;
	CARD8		 setMods;
	INT16		 groupChange;

	CARD16		 dfltPtrDelta;

	double		 mouseKeysCurve;
	double		 mouseKeysCurveFactor;
	INT16		 mouseKeysDX;
	INT16		 mouseKeysDY;
	CARD8		 mouseKeysAccel;
	CARD8		 mouseKeysCounter;

	CARD8		 lockedPtrButtons;
	CARD8		 shiftKeyCount;
	KeyCode		 mouseKey;
	KeyCode		 inactiveKey;
	KeyCode		 slowKey;
	KeyCode		 repeatKey;
	CARD8		 krgTimerActive;

	CARD32		 accessXFlags;
	CARD32		 lastPtrEventTime;
	OsTimerPtr	 mouseKeyTimer;
	OsTimerPtr	 slowKeysTimer;
	OsTimerPtr	 bounceKeysTimer;
	OsTimerPtr	 repeatKeyTimer;
	OsTimerPtr	 krgTimer;

	CARD8		 dfltNumLockMod;
	CARD8		 compatGroup2Mod;

	CARD8		 compatLookupState;
	CARD8		 compatGrabState;
	CARD16		 lookupState;
	CARD16		 grabState;

 	struct {
	    CARD32		 usesBase;
	    CARD32		 usesLatched;
	    CARD32		 usesLocked;
	    CARD32		 usesEffective;
	    CARD32		 usesCompat;
	    CARD32		 usesControls;
	    CARD32		 usedComponents;
	    CARD32		 haveMap;
	} iAccel;
	CARD32		 iState;

	XkbInterestRec	*interest;
} XkbSrvInfoRec, *XkbSrvInfoPtr;

/************************************************************************
 *
 * Masks for setting/determining the accessx control state.
 */
#define ALLOW_ACCESSX_MASK              (1 << 0)
#define MOUSE_KEYS_MASK                 (1 << 2)
#define TOGGLE_KEYS_MASK                (1 << 3)
#define TWO_KEYS_MASK                   (1 << 6)
#define TIME_OUT_MASK                   (1 << 7)
#define NO_LOCK_ON_TWO_MASK             (1 << 8)
#define STICKY_ONOFF_SOUND_MASK         (1 << 9)
#define STICKY_MOD_SOUND_MASK           (1 << 10)
#define MOUSE_ONOFF_SOUND_MASK          (1 << 11)
#define TOGGLE_ONOFF_SOUND_MASK         (1 << 12)
#define KRG_ONOFF_SOUND_MASK            (1 << 13)
#define KRG_PRESS_SOUND_MASK            (1 << 14)
#define KRG_ACCEPT_SOUND_MASK           (1 << 15)

#define KRG_MASK                (XkbSlowKeysMask|XkbBounceKeysMask)

#define	ALL_FILTERED_MASK	(XkbRepeatKeysMask|KRG_MASK)
#define ANY_OPTIONS_MASK        (XkbStickyKeysMask|XkbMouseKeysMask|KRG_MASK)

/************************************************************************
 *
 * Masks for setting/determining the accessx status.
 */
#define BLOCK_AND_WAKEUP_MASK   (1 << 0)
#define HOLDING_MOUSE_KEY_MASK  (1 << 1)
#define MOVING_MOUSE_MASK       (1 << 2)
#define MOVING_STEADY_MASK      (1 << 3)
#define HOLDING_KRG_KEY_MASK    (1 << 4)
#define HOLDING_SLOW_KEY_MASK   (1 << 5)
#define ISSUED_WARNING_MASK     (1 << 6)
#define REPEATING_MASK          (1 << 7)
#define CALLED_TWO_KEYS_MASK    (1 << 8)
#define WAITING_TO_TIMEOUT_MASK (1 << 9)

#define X_AccessXStickyKeysOn           0
#define X_AccessXStickyKeysOff          1
#define X_AccessXMouseKeysOn            2
#define X_AccessXMouseKeysOff           3
#define X_AccessXKRGWarning             4
#define X_AccessXKRGOn                  5
#define X_AccessXKRGOff                 6
#define X_AccessXToggleKeysOn           7
#define X_AccessXToggleKeysOff          8
#define X_AccessXToggleKeyDown          9
#define X_AccessXToggleKeyUp            10
#define X_AccessXModifierLatch          11
#define X_AccessXModifierUnlatch        12
#define X_AccessXModifierLock           13
#define X_AccessXModifierUnlock         14
#define X_AccessXPressSlowKey           15
#define X_AccessXAcceptSlowKey          16
#define X_AccessXChangeCurrentButton    17
#define X_AccessXPressButton            18
#define X_AccessXReleaseButton          19

extern int XkbEventBase;

_XFUNCPROTOBEGIN

extern KeySym *_XkbNewSymsForKey(
#if NeedFunctionPrototypes
    XkbDescRec *	/* xkb */,
    int 		/* key */,
    int 		/* needed */
#endif
);

extern XkbAction *_XkbNewActionsForKey(
#if NeedFunctionPrototypes
    XkbDescRec *	/* xkb */,
    int 		/* key */,
    int 		/* needed */
#endif
);

extern void XkbUpdateKeyTypesFromCore(
#if NeedFunctionPrototypes
    DeviceIntPtr	/* pXDev */,
    KeyCode 		/* first */,
    CARD8 		/* num */,
    xkbMapNotify *	/* pChanges */
#endif
);

void
XkbUpdateActions(
#if NeedFunctionPrototypes
    DeviceIntPtr	/* pXDev */,
    KeyCode 		/* first */,
    CARD8 		/* num */,
    xkbMapNotify *	/* pChanges */
#endif
);

extern void XkbApplyMappingChange(
#if NeedFunctionPrototypes
    DeviceIntPtr	/* pXDev */,
    CARD8 		/* request */,
    KeyCode 		/* firstKey */,
    CARD8 		/* num */
#endif
);

extern void XkbUpdateIndicators(
#if NeedFunctionPrototypes
    DeviceIntPtr		/* keybd */,
    CARD32		 	/* changed */,
    XkbIndicatorChangesRec *	/* pChanges */
#endif
);

extern void XkbComputeDerivedState(
#if NeedFunctionPrototypes
    XkbSrvInfoRec *		/* xkb */
#endif
);

extern unsigned XkbStateChangedFlags(
#if NeedFunctionPrototypes
    XkbStateRec *	/* old */,
    XkbStateRec *	/* new */
#endif
);

extern	void XkbSendStateNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr	/* kbd */,
       xkbStateNotify *	/* pSN */
#endif
);

extern	void XkbSendMapNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr	/* kbd */,
       xkbMapNotify *	/* ev */
#endif
);

extern	int  XkbComputeControlsNotify(
#if NeedFunctionPrototypes
	DeviceIntPtr		/* kbd */,
	XkbControlsRec *	/* old */,
	XkbControlsRec *	/* new */,
	xkbControlsNotify *	/* pCN */
#endif
);

extern	void XkbSendControlsNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr		/* kbd */,
       xkbControlsNotify *	/* ev */
#endif
);

extern	void XkbSendIndicatorNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr		/* kbd */,
       xkbIndicatorNotify *	/* ev */
#endif
);

extern	void XkbHandleBell(
#if NeedFunctionPrototypes
       DeviceIntPtr	/* kbd */,
       CARD8		/* percent */,
       pointer 		/* control */,
       CARD8		/* class */,
       Atom		/* name */
#endif
);

extern	void XkbSendSlowKeyNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr		/* kbd */,
       xkbSlowKeyNotify *	/* pEv */
#endif
);

extern	void XkbSendNamesNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr	/* kbd */,
       xkbNamesNotify *	/* ev */
#endif
);

extern	void XkbSendCompatNotify(
#if NeedFunctionPrototypes
       DeviceIntPtr		/* kbd */,
       xkbCompatMapNotify *	/* ev */
#endif
);

extern void XkbSendNotification(
#if NeedFunctionPrototypes
    DeviceIntPtr		/* kbd */,
    XkbChangesRec *		/* pChanges */
#endif
);

extern void XkbProcessKeyboardEvent(
#if NeedFunctionPrototypes
    xEvent * 			/* xE */,
    DeviceIntPtr		/* keybd */,
    int 			/* count */
#endif
);

extern	XkbInterestRec *XkbFindClientResource(
#if NeedFunctionPrototypes
       DevicePtr	/* inDev */,
       ClientPtr	/* client */
#endif
);

extern	XkbInterestRec *XkbAddClientResource(
#if NeedFunctionPrototypes
       DevicePtr	/* inDev */,
       ClientPtr	/* client */,
       XID		/* id */
#endif
);

extern	int XkbRemoveClient(
#if NeedFunctionPrototypes
       DevicePtr	/* inDev */,
       ClientPtr	/* client */
#endif
);

extern	int XkbRemoveResourceClient(
#if NeedFunctionPrototypes
       DevicePtr	/* inDev */,
       XID		/* id */
#endif
);

extern	void DDXUpdateIndicators(
#if NeedFunctionPrototypes
	DeviceIntPtr	/* keybd */,
	CARD32		/* oldState */,
	CARD32		/* newState */
#endif
);

_XFUNCPROTOEND

	/*
	 * bits in xkbClientFlags field of 
	 * ClientRec structure
	 */
#define	XKB_INITIALIZED	(1<<0)
#define	XkbIMMEDIATE	(1<<1)

#endif /* _XKBSRV_H_ */
