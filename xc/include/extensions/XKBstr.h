/* $XConsortium: XKBstr.h,v 1.4 93/09/28 20:16:45 rws Exp $ */
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

typedef struct _XkbBehavior {
	CARD8	type;
	CARD8	data;
} XkbBehavior;

typedef	struct _XkbAnyAction {
	CARD8	type;
	CARD8	data[7];
} XkbAnyAction;

typedef struct _XkbModAction {
	CARD8		type;
	CARD8		flags;
	CARD8		mods;
	CARD8		suppressLocks;
	CARD8		pad[4];
} XkbModAction;

typedef struct _XkbGroupAction {
	CARD8		type;
	CARD8		flags;
	INT8		group;
	CARD8		suppressLocks;
	CARD8		pad[4];
} XkbGroupAction;

typedef struct _XkbISOAction {
	CARD8		type;
	CARD8		flags;
	CARD8		mods;
	INT8		group;
	CARD8		affect;
	CARD8		pad[3];
} XkbISOAction;

typedef struct _XkbPtrAction {
	CARD8		type;
	CARD8		flags;
	CARD8		highX;
	CARD8		lowX;
	CARD8		highY;
	CARD8		lowY;
	CARD8		pad[2];
} XkbPtrAction;
#define	XkbPtrActionX(a)      ((INT16)(((a)->highX<<8)|((a)->lowX)))
#define	XkbPtrActionY(a)      ((INT16)(((a)->highY<<8)|((a)->lowY)))
#define	XkbSetPtrActionX(a,x) (((a)->highX=(((x)>>8)&0xff)),(a)->lowX=((x)&0xff))
#define	XkbSetPtrActionY(a,y) (((a)->highY=(((y)>>8)&0xff)),(y)->lowX=((y)&0xff))

typedef struct _XkbPtrBtnAction {
	CARD8		type;
	CARD8		flags;
	CARD8		count;
	CARD8		button;
	CARD8		pad[4];
} XkbPtrBtnAction;

typedef struct _XkbPtrDfltAction {
	CARD8		type;
	CARD8		flags;
	CARD8		value;
	CARD8		pad[5];
} XkbPtrDfltAction;

typedef struct _XkbSwitchScreenAction {
	CARD8		type;
	CARD8		flags;
	CARD8		screen;
	CARD8		pad[5];
} XkbSwitchScreenAction;

typedef struct _XkbCtrlsAction {
	CARD8		type;
	CARD8		flags;
	CARD8		ctrls3;
	CARD8		ctrls2;
	CARD8		ctrls1;
	CARD8		ctrls0;
	CARD8		pad[2];
} XkbCtrlsAction;
#define	XkbActionSetCtrls(a,c)	(((a)->ctrls3=(((c)>>24)&0xff)),\
					((a)->ctrls2=(((c)>>16)&0xff)),\
					((a)->ctrls1=(((c)>>8)&0xff)),\
					((a)->ctrls0=((c)&0xff)))
#define	XkbActionCtrls(a)	((CARD32)(((a)->ctrls3<<24)|((a)->ctrls2<<16)|\
					  ((a)->ctrls1<<8)|((a)->ctrls0)))

typedef	union _XkbAction {
	XkbAnyAction		any;
	XkbModAction		mods;
	XkbGroupAction		group;
	XkbISOAction		iso;
	XkbPtrAction		ptr;
	XkbPtrBtnAction		btn;
	XkbPtrDfltAction	dflt;
	XkbSwitchScreenAction	screen;
	XkbCtrlsAction		ctrls;
	CARD8	 		type;
} XkbAction;

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

	XkbBehavior		*keyBehaviors;
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
