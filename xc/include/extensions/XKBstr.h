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

#ifndef _XKBSTR_H_
#define	_XKBSTR_H_

#include <X11/extensions/XKBproto.h>

	/*
	 * Common data structures and access macros
	 */

typedef struct _XKBStateRec {
	CARD8	group;
	CARD8	baseGroup;
	CARD8	latchedGroup;
	CARD8   lockedGroup;
	CARD8	mods;
	CARD8	baseMods;
	CARD8	latchedMods;
	CARD8	lockedMods;
	CARD8	compatState;
	CARD8	unlockedMods;
	CARD8	groupsUnlocked;
	CARD8	pad;
} XKBStateRec,*XKBStatePtr;
#define	XKBModLocks(s)	 ((s)->lockedMods&(~(s)->unlockedMods))
#define	XKBStateMods(s)	 ((s)->baseMods|(s)->latchedMods|XKBModLocks(s))
#define	XKBGroupLock(s)	 ((s)->groupsUnlocked?0:(s)->lockedGroup)
#define	XKBStateGroup(s) ((s)->baseGroup+(s)->latchedGroup+XKBGroupLock(s))

typedef struct _XKBKeyType {
	CARD8		 flags;
	CARD8		 mask;
	CARD8		 groupWidth;
	CARD8		 pad;
	CARD8		*map;
	CARD8		*preserve;
} XKBKeyTypeRec, *XKBKeyTypePtr;
#define	XKB_KT_FOR_SYMS			(1<<0)
#define	XKB_KT_FOR_ACTIONS		(1<<1)
#define	XKB_KT_DONT_FREE_MAP		(1<<5)
#define	XKB_KT_DONT_FREE_PRESERVE	(1<<6)
#define	XKB_KT_DONT_FREE_STRUCT		(1<<7)
#define	XKBKTIndex(cm,s)	((cm)->map[(cm)->mask&(s)])
#define	XKBKTModsToClear(cm,s)	((cm)->preserve?\
				 ((cm->mask)&(~(cm)->preserve[(cm)->mask&(s)]))\
				 ((cm)->mask))
#define	XKBKTResultMods(cm,s)	((s)&(~XKBColModsToClear(cm,s)))
#define	XKBKTMapWidth(cm)		(((cm)->mask&0xff)+1)

#define	XKBNumGroups(g)		((g)&0x1f)
#define	XKBGroupsWrap(g)	(((g)&0x80)!=0)
#define	XKBSetGroupInfo(g,w)	(((w)?0x80:0x00)|((g)&0x1f))
#define	XKBSetNumGroups(g,n)	(((g)&0x80)|((n)&0x1f))
#define	XKBSetGroupsWrap(g,w)	(((w)?0x80:0x00)|((g)&0x1f))

	/*
	 * Structures and access macros used primarily by the server
	 */

typedef	struct _XKBAction {
	CARD16	type;
	CARD8	flags;
	CARD8	data;
} XKBAction;
#define	XKBActionType(a)		((a).type)
#define	XKBActionData(a)		((((a).flags)<<8)|((a).data))
#define	XKBActionDataLow(a)		((a).data)
#define	XKBActionDataHigh(a)		((a).flags)
#define	XKBActionSetType(a,t)		((a).type=(t))
#define	XKBActionSetData(a,d)		(((a).flags=(((d)>>8)&0xff)),(a).data=((d)&0xff))
#define	XKBActionSetDataLow(a,d)	((a).data=(d))
#define	XKBActionSetDataHigh(a,d)	((a).flags=(d))
#define	XKBNewAction(a,t,d)		(((a).type=(t)),XKBActionSetData(a,d))
#define	XKBNewAction2(a,t,d1,d2)	(((a).type=(t)),((a).flags=(d1)),((a).data=(d2)))

typedef	struct _XKBControls {
	CARD8		mouseKeysDfltBtn;
	CARD8		numGroups;
	CARD16		internalMods;
	CARD16		ignoreLockMods;
	CARD32		enabledControls;
	CARD16		repeatDelay;
	CARD16		repeatInterval;
	CARD16		slowKeysDelay;
	CARD16		debounceDelay;
	CARD16		mouseKeysDelay;
	CARD16		mouseKeysInterval;
	CARD16		mouseKeysTimeToMax;
	CARD16		mouseKeysMaxSpeed;
	CARD16		mouseKeysCurve;
	CARD16		accessXTimeout;
	CARD32		accessXTimeoutMask;
} XKBControlsRec, *XKBControlsPtr;

typedef struct _XKBServerMapRec {
	CARD16			 nActions;
	CARD16			 szActions;
	XKBAction		*actions;

	XKBAction		*keyBehaviors;
	CARD16			*keyActions;
} XKBServerMapRec, *XKServerMapRec;

#define	XKBSMKeyActionsPtr(m,k) (&(m)->actions[(m)->keyActions[k]])

	/*
	 * Structures and access macros used primarily by clients
	 */

typedef	struct _XKBSymMapRec {
	CARD8		 ktIndex;
	CARD8		 groupInfo;
	CARD16		 offset;
} XKBSymMapRec;

typedef struct _XKBClientMapRec {
	CARD8			 nKeyTypes;
	XKBKeyTypeRec		*keyTypes;

	CARD16			 szSyms;
	CARD16			 nSyms;
	KeySym			*syms;
	XKBSymMapRec		*keySymMap;
} XKBClientMapRec, *XKBClientMapPtr;

#define	XKBCMKeyGroupsWrap(m,k) (XKBGroupsWrap((m)->keySymMap[k].groupInfo))
#define	XKBCMKeyNumGroups(m,k)	(XKBNumGroups((m)->keySymMap[k].groupInfo))
#define	XKBCMKeyGroupWidth(m,k)	(XKBCMKeyType(m,k)->groupWidth)
#define	XKBCMKeyTypeIndex(m,k)	((m)->keySymMap[k].ktIndex)
#define	XKBCMKeyType(m,k)	(&(m)->keyTypes[XKBCMKeyTypeIndex(m,k)])
#define	XKBCMKeyNumSyms(m,k)	(XKBCMKeyGroupWidth(m,k)*XKBCMKeyNumGroups(m,k))
#define	XKBCMKeySymsOffset(m,k)	((m)->keySymMap[k].offset)
#define	XKBCMKeySymsPtr(m,k)	(&(m)->syms[XKBCMKeySymsOffset(m,k)])

	/*
	 * Compatibility structures and access macros
	 */

typedef struct _XKBModCompatRec {
	CARD8	mods;
	CARD8	groups;
} XKBModCompatRec,*XKBModCompatPtr;

typedef struct _XKBSymInterpretRec {
	KeySym		sym;
	CARD8		flags;
	CARD8		match;
	CARD8		mods;
	CARD8		indicator;
	XKBAction	action;
	XKBAction	behavior;
} XKBSymInterpretRec,*XKBSymInterpretPtr;

typedef struct _XKBCompatRec {
	CARD16			 nSymInterpret;
	XKBSymInterpretRec	*symInterpret;
	XKBModCompatRec		 modCompat[8];
} XKBCompatRec, *XKBCompatPtr;

typedef struct _XKBIndicatorMapRec {
	CARD8		whichMods;
	CARD8		mods;
	CARD8		whichGroups;
	CARD8		groups;
	CARD32		controls;
} XKBIndicatorMapRec, *XKBIndicatorMapPtr;

typedef struct _XKBIndicatorRec {
	CARD8		  	nRealIndicators;
	CARD8		  	pad1;
	CARD16		  	pad2;
	XKBIndicatorMapRec	maps[XKB_NUM_INDICATORS];
} XKBIndicatorRec,*XKBIndicatorPtr;

	/*
	 * Names for everything 
	 */
typedef struct _XKBNamesRec {
	Atom		  keycodes;
	Atom		  geometry;
	Atom		  symbols;
	Atom		  modifiers[8];
	Atom		 *keyTypes;
	Atom		**levels;
	Atom		 *radioGroups;
	Atom		  indicators[32];
	CARD16		  nCharSets;
	CARD16		  nRadioGroups;
	Atom		 *charSets;
} XKBNamesRec,*XKBNamesPtr;

	/*
	 * Alternate Symbol Sets
	 */
typedef struct _XKBAlternateSymsRec {
	Atom		 name;
	CARD8		 index;
	CARD8		 nCharSets;
	Atom		*charSets;
	CARD8		 firstKey;
	CARD8		 nKeys;
	CARD16		 nSyms;
	KeySym		*syms;
	XKBSymMapRec	*maps;
	struct _XKBAlternateSymsRec	*next;
} XKBAlternateSymsRec, *XKBAlternateSymsPtr;

	/*
	 * Tie it all together into one big keyboard description
	 */
typedef	struct _XKBDesc {
	CARD16		 	 flags;
	CARD16			 deviceSpec;
	KeyCode			 minKeyCode;
	KeyCode			 maxKeyCode;

	XKBControlsRec		*controls;
	XKBServerMapRec		*server;
	XKBClientMapRec		*map;
	XKBIndicatorRec		*indicators;
	XKBNamesRec		*names;
	XKBCompatRec		*compat;
	XKBAlternateSymsRec	*altSymSets;
} XKBDescRec, *XKBDescPtr;
#define	XKBKeyKeyType(d,k)	(XKBCMKeyType((d)->map,k))
#define	XKBKeyGroupWidth(d,k)	(XKBKeyKeyType(d,k)->groupWidth)
#define	XKBKeyGroupsWrap(d,k)	(XKBCMKeyGroupsWrap((d)->map,(k)))
#define	XKBKeyNumGroups(d,k)	(XKBCMKeyNumGroups((d)->map,(k)))
#define	XKBKeyNumSyms(d,k)	(XKBCMKeyNumSyms((d)->map,(k)))
#define	XKBKeySymsPtr(d,k)	(XKBCMKeySymsPtr((d)->map,(k)))
#define	XKBKeySym(d,k,n)	(XKBKeySymsPtr(d,k)[n])

#define	XKBKeyNumActions(d,k)	(((d)->server->keyActions[k]==0)?1:\
							XKBKeyNumSyms(d,k))
#define	XKBKeyActionsPtr(d,k)	(XKBSMKeyActionsPtr((d)->server,k))
#define	XKBNumKeys(d)		((d)->maxKeyCode-(d)->minKeyCode+1)

typedef struct _XKBMapChanges {
	CARD16			 changed;
	CARD8			 firstKeyType;
	CARD8			 nKeyTypes;
	CARD8			 firstKeySym;
	CARD8			 nKeySyms;
	CARD8			 firstKeyAction;
	CARD8			 nKeyActions;
	CARD8			 firstKeyBehavior;
	CARD8			 nKeyBehaviors;
} XKBMapChangesRec;

typedef struct _XKBControlsChanges {
	CARD32			 changedControls;
	CARD32			 enabledControlsChanges;
} XKBControlsChangesRec;

typedef struct _XKBIndicatorChanges {
	CARD32			 stateChanges;
	CARD32			 mapChanges;
} XKBIndicatorChangesRec;

typedef struct _XKBNameChanges {
	CARD16			changed;
	CARD8			firstKeyType;
	CARD8			nKeyTypes;
	CARD8			firstLevel;
	CARD8			nLevels;
	CARD8			firstRadioGroup;
	CARD8			nRadioGroups;
	CARD32			changedIndicators;
	CARD8			changedMods;
	CARD8			pad1;
	CARD16			pad2;
} XKBNameChangesRec;

typedef struct _XKBCompatChanges {
	CARD8			changedMods;
	CARD8			pad1;
	CARD16			pad2;
	CARD16			firstSym;
	CARD16			nSyms;
} XKBCompatChangesRec;

typedef struct _XKBAlternateSymChanges {
	CARD8			id;
	CARD8			firstKey;
	CARD8			nKeys;
	CARD8			pad1;
} XKBAlternateSymChanges;

typedef struct _XKBChanges {
	CARD16			 deviceSpec;
	CARD16			 stateChanges;
	XKBMapChangesRec	 map;
	XKBControlsChangesRec	 controls;
	XKBIndicatorChangesRec	 indicators;
	XKBNameChangesRec	 names;
	XKBCompatChangesRec	 compat;
} XKBChangesRec, *XKBChangesPtr;

#endif /* _XKBSTR_H_ */
