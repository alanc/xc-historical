/* $XConsortium: XKBproto.h,v 1.4 93/09/28 19:48:05 rws Exp $ */
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

#ifndef _XKBPROTO_H_
#define	_XKBPROTO_H_

#include <X11/extensions/XKB.h>
#include <X11/Xmd.h>

#define	XkbModAction(a)	(((a)->type>=XkbSASetMods)&&((a)->type<=XkbSALockMods))
#define	XkbGroupAction(a)	(((a)->type>=XkbSASetGroup)&&((a)->type<=XkbSALockGroup))
#define	XkbPtrAction(a)	(((a)->type>=XkbSAMovePtrBtn)&&((a)->type<=XkbSASetPtrDflt))

#define	XkbError2(a,b)		((((unsigned)(a))<<24)|(b))
#define	XkbError3(a,b,c)	XkbError2(a,(((unsigned)(b))<<16)|(c))
#define	XkbError4(a,b,c,d)	XkbError3(a,b,((((unsigned)(c))<<8)|(d)))

typedef struct _UseExtension {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBUseExtension */
    CARD16	length B16;
    CARD16	wantedMajor B16;
    CARD16	wantedMinor B16;
} xkbUseExtensionReq;
#define	sz_xkbUseExtensionReq	8

typedef struct _UseExtensionReply {
    BYTE	type;		/* X_Reply */
    BOOL	supported;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	serverMajor B16;
    CARD16	serverMinor B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xkbUseExtensionReply;
#define	sz_xkbUseExtensionReply	32

typedef	struct _SelectEvents {
    CARD8	reqType;
    CARD8	xkbReqType;	/* X_KBSelectEvents */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	affectState B16;
    CARD16	state B16;
    CARD16	affectMap B16;
    CARD16	map B16;
    CARD16	pad1 B16;
    CARD32	affectControls B32;
    CARD32	controls B32;
    CARD16	affectNames B16;
    CARD16	names B16;
    CARD16	affectCompatMap B16;
    CARD16	compatMap B16;
    BOOL	affectBell;
    BOOL	bell;
    CARD8	affectSlowKey;
    CARD8	slowKey;
    CARD32	affectIndicatorState B32;
    CARD32	indicatorState B32;
    CARD32	affectIndicatorMap B32;
    CARD32	indicatorMap B32;
    CARD16	affectAlternateSyms B16;
    CARD16	alternateSyms B16;
} xkbSelectEventsReq;
#define	sz_xkbSelectEventsReq	56

#ifdef NEED_EVENTS

typedef struct _SendEvent {
    CARD8	reqType;
    CARD8	xkbReqType;	/* X_KBSendEvent */
    CARD16	length B16;
    BOOL	propagate;
    BOOL	synthesizeClick;
    CARD16	pad B16;
    Window	destination;
    CARD32	eventMask B32;
    xEvent	event;
} xkbSendEventReq;
#define	sz_xkbSendEventReq	48

#endif /* NEED_EVENTS */

typedef struct _XkbBell {
    CARD8	reqType;
    CARD8	xkbReqType;	/* X_KBBell */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD8	bellClass;
    CARD8	bellID;
    INT8	percent;
    CARD8	pad1;
    CARD16	pad2 B16;
    Atom	name;
} xkbBellReq;
#define	sz_xkbBellReq		16

typedef struct _XkbGetState {
	CARD8		reqType;
	CARD8		xkbReqType;	/* always X_KBGetState */
	CARD16		length B16;
	CARD16		deviceSpec B16;
	CARD16		pad B16;
} xkbGetStateReq;
#define	sz_xkbGetStateReq	8

typedef	struct _XkbGetStateReply {
    BYTE	type;
    BYTE	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	pad1 B32;
    CARD8	mods;
    CARD8	baseMods;
    CARD8	latchedMods;
    CARD8	lockedMods;
    CARD8	unlockedMods;
    CARD8	group;
    CARD8	baseGroup;
    CARD8	latchedGroup;
    CARD8	lockedGroup;
    CARD8	compatState;
    CARD8	groupsUnlocked;
    CARD8	pad2;
    CARD32	pad3[2] B32;
} xkbGetStateReply;
#define	sz_xkbGetStateReply	32

typedef struct _LatchLockState {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBLatchLockState */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD8	affectModLocks;
    CARD8	modLocks;
    BOOL	lockGroup;
    CARD8	groupLock;
    CARD8	affectModLatches;
    CARD8	modLatches;
    BOOL	latchGroup;
    CARD8	groupLatch;
    CARD16	pad B16;
} xkbLatchLockStateReq;
#define	sz_xkbLatchLockStateReq		16

typedef struct _GetControls {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetControls */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	pad B16;
} xkbGetControlsReq;
#define	sz_xkbGetControlsReq	8

#define	xkb_KTHasPreserve	1
typedef struct _KeyTypeWireDesc {
    CARD8	flags;
    CARD8	mask;
    CARD8	groupWidth;
    CARD8	mapWidth;
} xkbKeyTypeWireDesc;

typedef struct _SymMapWireDesc {
    CARD8	ktIndex;
    CARD8	groupInfo;
    CARD16	nSyms B16;
} xkbSymMapWireDesc;

typedef struct _GetControlsReply {
    BYTE	type;		/* X_Reply */
    CARD8	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	mouseKeysDfltBtn;
    CARD8	numGroups;
    CARD16	internalMods B16;
    CARD16	ignoreLockMods B16;
    CARD16	pad1 B16;
    CARD32	enabledControls B32;
    CARD16	repeatDelay B16;
    CARD16	repeatInterval B16;
    CARD16	slowKeysDelay B16;
    CARD16	debounceDelay B16;
    CARD16	mouseKeysDelay B16;
    CARD16	mouseKeysInterval B16;
    CARD16	mouseKeysTimeToMax B16;
    CARD16	mouseKeysMaxSpeed B16;
    CARD16	mouseKeysCurve B16;
    CARD16	accessXTimeout B16;
    CARD32	accessXTimeoutMask B32;
} xkbGetControlsReply;
#define	sz_xkbGetControlsReply	44

typedef struct _SetControls {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetControls */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	affectInternalMods B16;
    CARD16	internalMods B16;
    CARD16	affectIgnoreLockMods B16;
    CARD16	ignoreLockMods B16;
    CARD8	mouseKeysDfltBtn;
    CARD8	pad1;
    CARD32	affectEnabledControls B32;
    CARD32	enabledControls B32;
    CARD32	changeControls B32;
    CARD16	repeatDelay B16;
    CARD16	repeatInterval B16;
    CARD16	slowKeysDelay B16;
    CARD16	debounceDelay B16;
    CARD16	mouseKeysDelay B16;
    CARD16	mouseKeysInterval B16;
    CARD16	mouseKeysTimeToMax B16;
    CARD16	mouseKeysMaxSpeed B16;
    CARD16	mouseKeysCurve B16;
    CARD16	accessXTimeout B16;
    CARD32	accessXTimeoutMask B32;
} xkbSetControlsReq;
#define	sz_xkbSetControlsReq	52

typedef struct _GetMap {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetMap */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	full B16;
    CARD16	partial B16;
    CARD8	firstKeyType;
    CARD8	nKeyTypes;
    CARD8	firstKeySym;
    CARD8	nKeySyms;
    CARD8	firstKeyAction;
    CARD8	nKeyActions;
    CARD8	firstKeyBehavior;
    CARD8	nKeyBehaviors;
    CARD16	pad B16;
} xkbGetMapReq;
#define	sz_xkbGetMapReq	20

typedef struct _GetMapReply {
    CARD8	type;		/* always X_Reply */
    CARD8	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	minKeyCode;
    CARD8	maxKeyCode;
    CARD16	present B16;
    CARD8	firstKeyType;
    CARD8	nKeyTypes;
    CARD8	totalKeyTypes;
    CARD8	pad1;
    CARD8	firstKeySym;
    CARD8	nKeySyms;
    CARD16	totalSyms B16;
    CARD8	firstKeyAction;
    CARD8	nKeyActions;
    CARD16	totalActions B16;
    CARD8	firstKeyBehavior;
    CARD8	nKeyBehaviors;
    CARD16	pad2 B16;
    CARD32	pad3 B32;
} xkbGetMapReply;
#define	sz_xkbGetMapReply		32

typedef struct _XkbSetMap {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetMap */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	present B16;
    CARD16	resize B16;
    CARD8	firstKeyType;
    CARD8	nKeyTypes;
    CARD8	firstKeySym;
    CARD8	nKeySyms;
    CARD16	totalSyms B16;
    CARD8	firstKeyAction;
    CARD8	nKeyActions;
    CARD16	totalActions B16;
    CARD8	firstKeyBehavior;
    CARD8	nKeyBehaviors;
    CARD16	pad B16;
} xkbSetMapReq;
#define	sz_xkbSetMapReq	24

#define	XkbModCompatMask	(1<<0)
#define	XkbSymInterpMask	(1<<1)
#define	XkbAllCompatMask	(0x3)

typedef struct _GetCompatMap {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetCompatMap */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    BOOL	getAllSyms;
    CARD8	mods;
    CARD16	firstSym B16;
    CARD16	nSyms B16;
} xkbGetCompatMapReq;
#define	sz_xkbGetCompatMapReq	16

typedef struct _GetCompatMapReply {
    CARD8	type;		/* always X_Reply */
    CARD8	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	pad;
    CARD8	mods;
    CARD16	firstSym B16;
    CARD16	nSyms B16;
    CARD16	nTotalSyms B16;
    CARD32	pad2[3] B32;
} xkbGetCompatMapReply;
#define	sz_xkbGetCompatMapReply		32

typedef struct _XkbSetCompatMap {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetCompatMap */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    BOOL	recomputeActions;
    BOOL	truncateSyms;
    CARD8	pad1;
    CARD8	mods;
    CARD16	firstSym B16;
    CARD16	nSyms B16;
    CARD16	pad2 B16;
} xkbSetCompatMapReq;
#define	sz_xkbSetCompatMapReq	16

typedef struct _GetIndicatorState {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetIndicatorState */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	pad B16;
} xkbGetIndicatorStateReq;
#define	sz_xkbGetIndicatorStateReq	8

typedef struct _GetIndicatorStateReply {
    CARD8	type;		/* always X_Reply */
    CARD8	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	state B32;
    CARD32	pad[5] B32;
} xkbGetIndicatorStateReply;
#define	sz_xkbGetIndicatorStateReply	32

typedef struct _GetIndicatorMap {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetIndicatorMap */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	pad B16;
    CARD32	which B32;
} xkbGetIndicatorMapReq;
#define	sz_xkbGetIndicatorMapReq	12

typedef struct _GetIndicatorMapReply {
    CARD8	type;		/* always X_Reply */
    CARD8	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	which B32;
    CARD8	nRealIndicators;
    CARD8	nIndicators;
    CARD16	pad2 B16;
    CARD32	pad3[4] B32;
} xkbGetIndicatorMapReply;
#define	sz_xkbGetIndicatorMapReply	32

typedef struct _IndicatorMapWireDesc {
    CARD8	whichMods;
    CARD8	mods;
    CARD8	whichGroups;
    CARD8	groups;
    CARD32	controls B32;
} xkbIndicatorMapWireDesc;
#define	sz_xkbIndicatorMapWireDesc	8

typedef struct _SetIndicatorMap {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetIndicatorMap */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	pad1 B16;
    CARD32	which B32;
} xkbSetIndicatorMapReq;
#define	sz_xkbSetIndicatorMapReq	12

typedef struct _GetNames {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetNames */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	which B16;
} xkbGetNamesReq;
#define	sz_xkbGetNamesReq		8

typedef	struct _XkbGetNamesReply {
    BYTE	type;
    BYTE	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	pad1 B16;
    CARD16	which B16;
    Atom	keycodes;
    Atom	geometry;
    Atom	symbols;
    CARD8	nKeyTypes;
    CARD8	nRadioGroups;
    CARD8	nCharSets;
    CARD8	pad2;
    CARD32	pad3 B32;
} xkbGetNamesReply;
#define	sz_xkbGetNamesReply	32

typedef struct _SetNames {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetNames */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	which B16;
    Atom	keycodes;
    Atom	geometry;
    Atom	symbols;
    CARD8	firstKeyType;
    CARD8	nKeyTypes;
    CARD8	firstKTLevel;
    CARD8	nKTLevels;
    CARD32	indicators B32;
    CARD8	modifiers;
    CARD8	nRadioGroups;
    CARD8	nCharSets;
    CARD8	pad1;
    CARD16	resize B16;
    CARD16	pad2 B16;
} xkbSetNamesReq;
#define	sz_xkbSetNamesReq		36

typedef struct _ListAlternateSyms {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBListAlternateSyms */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	pad B16;
    Atom	name;
    Atom	charset;
} xkbListAlternateSymsReq;
#define	sz_xkbListAlternateSymsReq	16

typedef struct _ListAlternateSymsReply {
    BYTE	type;		/* X_Reply */
    CARD8	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD8	nAlternateSyms;
    CARD8	pad1;
    CARD16	pad2 B16;
    CARD8	indices[20];
} xkbListAlternateSymsReply;
#define sz_xkbListAlternateSymsReply	32

typedef struct _GetAlternateSyms {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetAlternateSyms */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD8	index;
    CARD8	firstKey;
    CARD8	nKeys;
    CARD8	pad1;
    CARD16	pad2 B16;
} xkbGetAlternateSymsReq;
#define	sz_xkbGetAlternateSymsReq		12

typedef	struct _XkbGetAlternateSymsReply {
    BYTE	type;
    BYTE	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    Atom	name;
    CARD8	index;
    CARD8	nCharSets;
    CARD8	firstKey;
    CARD8	nKeys;
    CARD16	totalSyms B16;
    CARD16	pad2 B16;
    CARD32	pad3[3] B32;
} xkbGetAlternateSymsReply;
#define	sz_xkbGetAlternateSymsReply	32

typedef struct _SetAlternateSyms {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetAlternateSyms */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    BOOL	create;
    CARD8	replace;
    CARD16	present B16;
    CARD16	pad1 B16;
    CARD32	name B32;
    CARD8	nCharSets;
    CARD8	firstKey;
    CARD8	nKeys;
    CARD8	pad;
} xkbSetAlternateSymsReq;
#define	sz_xkbSetAlternateSymsReq		20

typedef struct _GetGeometry {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBGetGeometry */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD16	pad B16;
} xkbGetGeometryReq;
#define	sz_xkbGetGeometryReq	8

typedef struct _KeyShape {
    CARD16	width B16;
    CARD16	height B16;
} xkbKeyShapeWireDesc;

#define	XkbMaxSectionNameLen	16
typedef struct _SectionWireDesc {
    CARD8	name[XkbMaxSectionNameLen];
    CARD16	angle B16;
    CARD16	x B16;
    CARD16	y B16;
    CARD16	width B16;
    CARD16	height B16;
    CARD8	nRows;
    CARD8	pad;
} xkbSectionWireDesc;
#define	sz_xkbSectionWireDesc	28

typedef struct _RowWireDesc {
    CARD16	x B16;
    CARD16	y B16;
    CARD16	width B16;
    CARD16	height B16;
    CARD8	rows;
    CARD8	pad1;
    CARD16	pad2 B16;
} xkbRowWireDesc;
#define sz_xkbRowWireDesc	12

typedef struct _KeyGeomWireDesc {
    CARD8	keycode;
    CARD8	shape;
    CARD16	offset B16;
} xkbGeomWireDesc;
#define sz_xkbGeomWireDesc	4

typedef struct _GetGeometryReply {
    BYTE	type;
    BYTE	deviceID;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    Atom	name;
    BOOL	supported;
    CARD8	nSections;
    CARD8	nShapes;
    CARD8	minKeyCode;
    CARD8	maxKeyCode;
    CARD8	pad1;
    CARD16	widthMM B16;
    CARD16	heightMM B16;
    CARD16	pad2 B16;
    CARD32	pad3[2] B32;
} xkbGetGeometryReply;
#define sz_xkbGetGeometryReply	32

#define	XkbGMByName		1
#define	XkbGMDescription	2
typedef struct _SetGeometry {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetGeometry */
    CARD16	length B16;
    CARD16	deviceSpec B16;
    CARD8	nShapes;
    CARD8	nSections;
    Atom	name;
    CARD16	widthMM B16;
    CARD16	heightMM B16;
} xkbSetGeometryReq;
#define	sz_xkbSetGeometryReq	16

typedef struct _SetDebuggingFlags {
    CARD8	reqType;
    CARD8	xkbReqType;	/* always X_KBSetDebuggingFlags */
    CARD16	length B16;
    CARD32	flags B32;
} xkbSetDebuggingFlagsReq;
#define	sz_xkbSetDebuggingFlagsReq	8

	/*
	 * X KEYBOARD EXTENSION EVENT STRUCTURES
	 */

typedef struct _xkbAnyEvent {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    Time	time;
    CARD8	deviceID;
    CARD8	pad1;
    CARD16	pad2 B16;
    CARD32	pad3[4] B32;
} xkbAnyEvent;
#define	sz_xkbAnyEvent;

typedef	struct _xkbStateNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	mods;
    CARD8	baseMods;
    CARD8	lockedMods;
    CARD8	latchedMods;
    CARD8	group;
    CARD8	baseGroup;
    CARD8	latchedGroup;
    CARD8	lockedGroup;
    CARD8	compatState;
    CARD8	unlockedMods;
    CARD8	groupsUnlocked;
    CARD8	keycode;
    CARD8	eventType;
    CARD8	requestMajor;
    CARD8	requestMinor;
    CARD16	changed B16;
    CARD16	pad1 B16;
    CARD32	pad2 B32;
} xkbStateNotify;
#define	sz_xkbStateNotify	32

typedef	struct _xkbMapNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	pad1;
    CARD8	deviceID;
    CARD16	changed B16;
    CARD8	firstKeyType;
    CARD8	nKeyTypes;
    CARD8	firstKeySym;
    CARD8	nKeySyms;
    CARD8	firstKeyAction;
    CARD8	nKeyActions;
    CARD8	firstKeyBehavior;
    CARD8	nKeyBehaviors;
    CARD32	pad2[3] B32;
} xkbMapNotify;
#define	sz_xkbMapNotify	32

typedef struct _xkbControlsNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	pad1;
    CARD16	pad2 B16;
    CARD32	changedControls B32;
    CARD32	enabledControls B32;
    CARD32	enabledControlChanges B32;
    CARD8	keycode;
    CARD8	eventType;
    CARD8	requestMajor;
    CARD8	requestMinor;
    CARD32	pad3 B32;
} xkbControlsNotify;
#define	sz_xkbControlsNotify	32

typedef struct _xkbIndicatorNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	pad1;
    CARD16	pad2 B16;
    CARD32	stateChanged B32;
    CARD32	state B32;
    CARD32	mapChanged B32;
    CARD32	pad3[2] B32;
} xkbIndicatorNotify;
#define	sz_xkbIndicatorNotify	32

typedef struct _xkbBellNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	bellClass;
    CARD8	bellID;
    CARD8	percent;
    CARD16	pitch B16;
    CARD16	duration B16;
    Atom	name;
    CARD32	pad1[3] B32;
} xkbBellNotify;
#define	sz_xkbBellNotify	32

typedef struct _xkbSlowKeyNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	slowKeyType;
    CARD8	keycode;
    CARD8	pad1;
    CARD16	delay B16;
    CARD16	pad2 B16;
    CARD32	pad3[4] B32;
} xkbSlowKeyNotify;
#define	sz_xkbSlowKeyNotify	32

typedef struct _xkbNamesNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	pad1;
    CARD16	changed B16;
    CARD8	firstKeyType;
    CARD8	nKeyTypes;
    CARD8	firstLevelName;
    CARD8	nLevelNames;
    CARD8	firstRadioGroup;
    CARD8	nRadioGroups;
    CARD8	nCharSets;
    CARD8	changedMods;
    CARD32	changedIndicators B32;
    CARD32	pad3[2] B32;
} xkbNamesNotify;
#define	sz_xkbNamesNotify	32

typedef struct _xkbCompatMapNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	changedMods;
    CARD16	firstSym B16;
    CARD16	nSyms B16;
    CARD16	nTotalSyms B16;
    CARD32	pad3[4] B32;
} xkbCompatMapNotify;
#define sz_xkbCompatMapNotify	32

typedef struct _xkbAlternateSymsNotify {
    BYTE	type;
    BYTE	xkbType;
    CARD16	sequenceNumber B16;
    Time	time;
    CARD8	deviceID;
    CARD8	altSymsID;
    CARD8	firstKey;
    CARD8	nKeys;
    CARD32	pad2[5] B32;
} xkbAlternateSymsNotify;
#define	sz_xkbAlternateSymsNotify	32

typedef struct _xkbEvent {
    union {
	xkbAnyEvent		any;
	xkbStateNotify		state;
	xkbMapNotify		map;
	xkbControlsNotify	controls;
	xkbIndicatorNotify	indicators;
	xkbBellNotify		bell;
	xkbSlowKeyNotify	slowkey;
	xkbNamesNotify		names;
	xkbCompatMapNotify	compat;
	xkbAlternateSymsNotify	altSyms;
    } u;
} xkbEvent;
#define sz_xkbEvent	32

#endif /* _XKBPROTO_H_ */
