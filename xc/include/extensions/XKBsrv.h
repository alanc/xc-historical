/* $XConsortium$ */
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

typedef struct _XKBInterestRec {
	struct _XKBSrvInfoRec	*kbd;
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
	struct _XKBInterestRec	*next;
} XKBInterestRec,*XKBInterestPtr;

typedef struct _XKBRadioGroup {
	CARD8		flags;
	CARD8		nMembers;
	CARD8		dfltDown;
	CARD8		currentDown;
	CARD8		members[XKB_RG_MAX_MEMBERS];
} XKBRadioGroupRec, *XKBRadioGroupPtr;

#define	_OFF_TIMER	0
#define	_KRG_TIMER	1
#define	_TIMEOUT_TIMER	2

typedef struct _XKBSrvInfoRec {
	XKBStateRec	 state;
	XKBControlsRec	 Controls;
	XKBServerMapRec	 Server;
	XKBClientMapRec	 Map;
	XKBIndicatorRec	 Indicators;
	XKBNamesRec	 Names;
	XKBCompatRec	 Compat;
	XKBDescRec	 desc;

	DeviceIntRec	*device;

	XKBRadioGroupRec *radioGroups;
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
	struct _XTimer	*mouseKeyTimer;
	struct _XTimer	*slowKeysTimer;
	struct _XTimer	*bounceKeysTimer;
	struct _XTimer	*repeatKeyTimer;
	struct _XTimer	*krgTimer;

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
	} iAccel;
	CARD32		 iState;

	XKBInterestRec	*interest;
} XKBSrvInfoRec, *XKBSrvInfoPtr;

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

#define KRG_MASK                (XKBSlowKeysMask|XKBBounceKeysMask)

#define	ALL_FILTERED_MASK	(XKBRepeatKeysMask|KRG_MASK)
#define ANY_OPTIONS_MASK        (XKBStickyKeysMask|XKBMouseKeysMask|KRG_MASK)

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

extern int XKBEventBase;

_XFUNCPROTOBEGIN

extern KeySym *_XKBNewSymsForKey(
#if NeedFunctionPrototypes
    XKBDescRec *	/* xkb */,
    int 		/* key */,
    int 		/* needed */
#endif
);

extern XKBAction *_XKBNewActionsForKey(
#if NeedFunctionPrototypes
    XKBDescRec *	/* xkb */,
    int 		/* key */,
    int 		/* needed */
#endif
);

extern void XKBUpdateKeyTypesFromCore(
#if NeedFunctionPrototypes
    DeviceIntRec *	/* pXDev */,
    KeyCode 		/* first */,
    CARD8 		/* num */,
    xKBMapNotify *	/* pChanges */
#endif
);

void
XKBUpdateActions(
#if NeedFunctionPrototypes
    DeviceIntRec *	/* pXDev */,
    KeyCode 		/* first */,
    CARD8 		/* num */,
    xKBMapNotify *	/* pChanges */
#endif
);

extern void XKBApplyMappingChange(
#if NeedFunctionPrototypes
    DeviceIntRec *	/* pXDev */,
    CARD8 		/* request */,
    KeyCode 		/* firstKey */,
    CARD8 		/* num */
#endif
);

extern void XKBUpdateIndicators(
#if NeedFunctionPrototypes
    DeviceIntRec *		/* keybd */,
    CARD32		 	/* changed */,
    XKBIndicatorChangesRec *	/* pChanges */
#endif
);

extern void XKBComputeDerivedState(
#if NeedFunctionPrototypes
    XKBSrvInfoRec *		/* xkb */
#endif
);

extern unsigned XKBStateChangedFlags(
#if NeedFunctionPrototypes
    XKBStateRec *	/* old */,
    XKBStateRec *	/* new */
#endif
);

extern	void XKBSendStateNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *	/* kbd */,
       xKBStateNotify *	/* pSN */
#endif
);

extern	void XKBSendMapNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *	/* kbd */,
       xKBMapNotify *	/* ev */
#endif
);

extern	int  XKBComputeControlsNotify(
#if NeedFunctionPrototypes
	DeviceIntRec *		/* kbd */,
	XKBControlsRec *	/* old */,
	XKBControlsRec *	/* new */,
	xKBControlsNotify *	/* pCN */
#endif
);

extern	void XKBSendControlsNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *		/* kbd */,
       xKBControlsNotify *	/* ev */
#endif
);

extern	void XKBSendIndicatorNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *		/* kbd */,
       xKBIndicatorNotify *	/* ev */
#endif
);

extern	void XKBHandleBell(
#if NeedFunctionPrototypes
       DeviceIntRec *	/* kbd */,
       CARD8		/* percent */,
       pointer 		/* control */,
       CARD8		/* class */,
       Atom		/* name */
#endif
);

extern	void XKBSendSlowKeyNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *		/* kbd */,
       xKBSlowKeyNotify *	/* pEv */
#endif
);

extern	void XKBSendNamesNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *	/* kbd */,
       xKBNamesNotify *	/* ev */
#endif
);

extern	void XKBSendCompatNotify(
#if NeedFunctionPrototypes
       DeviceIntRec *		/* kbd */,
       xKBCompatMapNotify *	/* ev */
#endif
);

extern void XKBSendNotification(
#if NeedFunctionPrototypes
    DeviceIntRec *		/* kbd */,
    XKBChangesRec *		/* pChanges */
#endif
);

extern void XKBProcessKeyboardEvent(
#if NeedFunctionPrototypes
    xEvent * 			/* xE */,
    DeviceIntRec *		/* keybd */,
    int 			/* count */
#endif
);

extern	XKBInterestRec *XKBFindClientResource(
#if NeedFunctionPrototypes
       DeviceRec *	/* inDev */,
       ClientRec *	/* client */
#endif
);

extern	XKBInterestRec *XKBAddClientResource(
#if NeedFunctionPrototypes
       DeviceRec *	/* inDev */,
       ClientRec *	/* client */,
       XID		/* id */
#endif
);

extern	int XKBRemoveClient(
#if NeedFunctionPrototypes
       DeviceRec *	/* inDev */,
       ClientRec *	/* client */
#endif
);

extern	int XKBRemoveResourceClient(
#if NeedFunctionPrototypes
       DeviceRec *	/* inDev */,
       XID		/* id */
#endif
);

extern	void DDXUpdateIndicators(
#if NeedFunctionPrototypes
	DeviceRec *	/* keybd */,
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
#define	XKB_IMMEDIATE	(1<<1)

#endif /* _XKBSRV_H_ */
