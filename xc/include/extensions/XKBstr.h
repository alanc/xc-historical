/* $XConsortium: XKBstr.h,v 1.3 93/09/28 19:48:12 rws Exp $ */
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

typedef struct _XkbStateRec {
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
} XkbStateRec,*XkbStatePtr;
#define	XkbModLocks(s)	 ((s)->lockedMods&(~(s)->unlockedMods))
#define	XkbStateMods(s)	 ((s)->baseMods|(s)->latchedMods|XkbModLocks(s))
#define	XkbGroupLock(s)	 ((s)->groupsUnlocked?0:(s)->lockedGroup)
#define	XkbStateGroup(s) ((s)->baseGroup+(s)->latchedGroup+XkbGroupLock(s))

typedef struct _XkbKeyType {
	CARD8		 flags;
	CARD8		 mask;
	CARD8		 groupWidth;
	CARD8		 pad;
	CARD8		*map;
	CARD8		*preserve;
} XkbKeyTypeRec, *XkbKeyTypePtr;
#define	XkbNoFreeKTMap		(1<<5)
#define	XkbNoFreeKTPreserve	(1<<6)
#define	XkbNoFreeKTStruct		(1<<7)
#define	XkbKTIndex(cm,s)	((cm)->map[(cm)->mask&(s)])
#define	XkbKTModsToClear(cm,s)	((cm)->preserve?\
				 ((cm->mask)&(~(cm)->preserve[(cm)->mask&(s)]))\
				 ((cm)->mask))
#define	XkbKTResultMods(cm,s)	((s)&(~XkbTModsToClear(cm,s)))
#define	XkbKTMapWidth(cm)		(((cm)->mask&0xff)+1)

#define	XkbNumGroups(g)		((g)&0x1f)
#define	XkbGroupsWrap(g)	(((g)&0x80)!=0)
#define	XkbSetGroupInfo(g,w)	(((w)?0x80:0x00)|((g)&0x1f))
#define	XkbSetNumGroups(g,n)	(((g)&0x80)|((n)&0x1f))
#define	XkbSetGroupsWrap(g,w)	(((w)?0x80:0x00)|((g)&0x1f))

	/*
	 * Structures and access macros used primarily by the server
	 */

typedef	struct _XkbAction {
	CARD16	type;
	CARD8	flags;
	CARD8	data;
} XkbAction;
#define	XkbActionType(a)		((a).type)
#define	XkbActionData(a)		((((a).flags)<<8)|((a).data))
#define	XkbActionDataLow(a)		((a).data)
#define	XkbActionDataHigh(a)		((a).flags)
#define	XkbActionSetType(a,t)		((a).type=(t))
#define	XkbActionSetData(a,d)		(((a).flags=(((d)>>8)&0xff)),(a).data=((d)&0xff))
#define	XkbActionSetDataLow(a,d)	((a).data=(d))
#define	XkbActionSetDataHigh(a,d)	((a).flags=(d))
#define	XkbNewAction(a,t,d)		(((a).type=(t)),XkbActionSetData(a,d))
#define	XkbNewAction2(a,t,d1,d2)	(((a).type=(t)),((a).flags=(d1)),((a).data=(d2)))

typedef	struct _XkbControls {
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
} XkbControlsRec, *XkbControlsPtr;

typedef struct _XkbServerMapRec {
	CARD16			 nActions;
	CARD16			 szActions;
	XkbAction		*actions;

	XkbAction		*keyBehaviors;
	CARD16			*keyActions;
} XkbServerMapRec, *XkbServerMapPtr;

#define	XkbSMKeyActionsPtr(m,k) (&(m)->actions[(m)->keyActions[k]])

	/*
	 * Structures and access macros used primarily by clients
	 */

typedef	struct _XkbSymMapRec {
	CARD8		 ktIndex;
	CARD8		 groupInfo;
	CARD16		 offset;
} XkbSymMapRec;

typedef struct _XkbClientMapRec {
	CARD8			 nKeyTypes;
	XkbKeyTypeRec		*keyTypes;

	CARD16			 szSyms;
	CARD16			 nSyms;
	KeySym			*syms;
	XkbSymMapRec		*keySymMap;
} XkbClientMapRec, *XkbClientMapPtr;

#define	XkbCMKeyGroupsWrap(m,k) (XkbGroupsWrap((m)->keySymMap[k].groupInfo))
#define	XkbCMKeyNumGroups(m,k)	(XkbNumGroups((m)->keySymMap[k].groupInfo))
#define	XkbCMKeyGroupWidth(m,k)	(XkbCMKeyType(m,k)->groupWidth)
#define	XkbCMKeyTypeIndex(m,k)	((m)->keySymMap[k].ktIndex)
#define	XkbCMKeyType(m,k)	(&(m)->keyTypes[XkbCMKeyTypeIndex(m,k)])
#define	XkbCMKeyNumSyms(m,k)	(XkbCMKeyGroupWidth(m,k)*XkbCMKeyNumGroups(m,k))
#define	XkbCMKeySymsOffset(m,k)	((m)->keySymMap[k].offset)
#define	XkbCMKeySymsPtr(m,k)	(&(m)->syms[XkbCMKeySymsOffset(m,k)])

	/*
	 * Compatibility structures and access macros
	 */

typedef struct _XkbModCompatRec {
	CARD8	mods;
	CARD8	groups;
} XkbModCompatRec,*XkbModCompatPtr;

typedef struct _XkbSymInterpretRec {
	KeySym		sym;
	CARD8		flags;
	CARD8		match;
	CARD8		mods;
	CARD8		indicator;
	XkbAction	action;
	XkbAction	behavior;
} XkbSymInterpretRec,*XkbSymInterpretPtr;

typedef struct _XkbCompatRec {
	CARD16			 nSymInterpret;
	XkbSymInterpretRec	*symInterpret;
	XkbModCompatRec		 modCompat[8];
} XkbCompatRec, *XkbCompatPtr;

typedef struct _XkbIndicatorMapRec {
	CARD8		whichMods;
	CARD8		mods;
	CARD8		whichGroups;
	CARD8		groups;
	CARD32		controls;
} XkbIndicatorMapRec, *XkbIndicatorMapPtr;

typedef struct _XkbIndicatorRec {
	CARD8		  	nRealIndicators;
	CARD8		  	pad1;
	CARD16		  	pad2;
	XkbIndicatorMapRec	maps[XkbNumIndicators];
} XkbIndicatorRec,*XkbIndicatorPtr;

	/*
	 * Names for everything 
	 */
typedef struct _XkbNamesRec {
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
} XkbNamesRec,*XkbNamesPtr;

	/*
	 * Alternate Symbol Sets
	 */
typedef struct _XkbAlternateSymsRec {
	Atom		 name;
	CARD8		 index;
	CARD8		 nCharSets;
	Atom		*charSets;
	CARD8		 firstKey;
	CARD8		 nKeys;
	CARD16		 nSyms;
	KeySym		*syms;
	XkbSymMapRec	*maps;
	struct _XkbAlternateSymsRec	*next;
} XkbAlternateSymsRec, *XkbAlternateSymsPtr;

	/*
	 * Tie it all together into one big keyboard description
	 */
typedef	struct _XkbDesc {
	CARD16		 	 flags;
	CARD16			 deviceSpec;
	KeyCode			 minKeyCode;
	KeyCode			 maxKeyCode;

	XkbControlsRec		*controls;
	XkbServerMapRec		*server;
	XkbClientMapRec		*map;
	XkbIndicatorRec		*indicators;
	XkbNamesRec		*names;
	XkbCompatRec		*compat;
	XkbAlternateSymsRec	*altSymSets;
} XkbDescRec, *XkbDescPtr;
#define	XkbKeyKeyType(d,k)	(XkbCMKeyType((d)->map,k))
#define	XkbKeyGroupWidth(d,k)	(XkbKeyKeyType(d,k)->groupWidth)
#define	XkbKeyGroupsWrap(d,k)	(XkbCMKeyGroupsWrap((d)->map,(k)))
#define	XkbKeyNumGroups(d,k)	(XkbCMKeyNumGroups((d)->map,(k)))
#define	XkbKeyNumSyms(d,k)	(XkbCMKeyNumSyms((d)->map,(k)))
#define	XkbKeySymsPtr(d,k)	(XkbCMKeySymsPtr((d)->map,(k)))
#define	XkbKeySym(d,k,n)	(XkbKeySymsPtr(d,k)[n])

#define	XkbKeyNumActions(d,k)	(((d)->server->keyActions[k]==0)?1:\
							XkbKeyNumSyms(d,k))
#define	XkbKeyActionsPtr(d,k)	(XkbSMKeyActionsPtr((d)->server,k))
#define	XkbNumKeys(d)		((d)->maxKeyCode-(d)->minKeyCode+1)

typedef struct _XkbMapChanges {
	CARD16			 changed;
	CARD8			 firstKeyType;
	CARD8			 nKeyTypes;
	CARD8			 firstKeySym;
	CARD8			 nKeySyms;
	CARD8			 firstKeyAction;
	CARD8			 nKeyActions;
	CARD8			 firstKeyBehavior;
	CARD8			 nKeyBehaviors;
} XkbMapChangesRec;

typedef struct _XkbControlsChanges {
	CARD32			 changedControls;
	CARD32			 enabledControlsChanges;
} XkbControlsChangesRec;

typedef struct _XkbIndicatorChanges {
	CARD32			 stateChanges;
	CARD32			 mapChanges;
} XkbIndicatorChangesRec;

typedef struct _XkbNameChanges {
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
} XkbNameChangesRec;

typedef struct _XkbCompatChanges {
	CARD8			changedMods;
	CARD8			pad1;
	CARD16			pad2;
	CARD16			firstSym;
	CARD16			nSyms;
} XkbCompatChangesRec;

typedef struct _XkbAlternateSymChanges {
	CARD8			id;
	CARD8			firstKey;
	CARD8			nKeys;
	CARD8			pad1;
} XkbAlternateSymChanges;

typedef struct _XkbChanges {
	CARD16			 deviceSpec;
	CARD16			 stateChanges;
	XkbMapChangesRec	 map;
	XkbControlsChangesRec	 controls;
	XkbIndicatorChangesRec	 indicators;
	XkbNameChangesRec	 names;
	XkbCompatChangesRec	 compat;
} XkbChangesRec, *XkbChangesPtr;

#endif /* _XKBSTR_H_ */
