/* $XConsortium: xkbUtils.c,v 1.2 93/09/28 00:00:31 rws Exp $ */
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

#include <stdio.h>
#include <math.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "misc.h"
#include "inputstr.h"
#include "XKBsrv.h"
#include <ctype.h>

static	int	XkbComputeAutoRepeat = 1;
#define	CREATE_ATOM(s)	MakeAtom(s,sizeof(s)-1,1)

#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

/***====================================================================***/

XkbAction *
_XkbNewActionsForKey(xkb,key,needed)
    XkbDescRec *xkb;
    int key;
    int needed;
{
register int i,nActs;
XkbAction	*newActs;

    if ((xkb->server->keyActions[key]!=0)&&(XkbKeyNumSyms(xkb,key)>=needed)) {
	return XkbKeyActionsPtr(xkb,key);
    }
    if (xkb->server->szActions-xkb->server->nActions>=needed) {
	xkb->server->keyActions[key]= xkb->server->nActions;
	xkb->server->nActions+= needed;
	return &xkb->server->actions[xkb->server->keyActions[key]];
    }
    xkb->server->szActions+= 32;
    newActs = (XkbAction *)Xcalloc(xkb->server->szActions*sizeof(XkbAction));
    newActs[0].type = XkbSANoAction;
    nActs = 1;
    for (i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	if (xkb->server->keyActions[i]!=0) {
	    memcpy(&newActs[nActs],XkbKeyActionsPtr(xkb,i),
				XkbKeyNumActions(xkb,i)*sizeof(XkbAction));
	    xkb->server->keyActions[i]= nActs;
	    if (i!=key)	nActs+= XkbKeyNumActions(xkb,i);
	    else	nActs+= needed;
	}
    }
    free(xkb->server->actions);
    xkb->server->actions = newActs;
    xkb->server->nActions = nActs;
    return &xkb->server->actions[xkb->server->keyActions[key]];
}

KeySym *
_XkbNewSymsForKey(xkb,key,needed)
    XkbDescRec *xkb;
    int key;
    int needed;
{
register int i,nSyms;
KeySym	*newSyms;

    if (XkbKeyNumSyms(xkb,key)>=needed) {
	return XkbKeySymsPtr(xkb,key);
    }
    if (xkb->server->keyActions[key]!=0)
	_XkbNewActionsForKey(xkb,key,needed);

    if (xkb->map->szSyms-xkb->map->nSyms>=needed) {
	xkb->map->keySymMap[key].offset = xkb->map->nSyms;
	xkb->map->nSyms+= needed;
	return &xkb->map->syms[xkb->map->keySymMap[key].offset];
    }
    xkb->map->szSyms+= 128;
    newSyms = (KeySym *)Xcalloc(xkb->map->szSyms*sizeof(KeySym));
    nSyms = 1;
    for (i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	memcpy(&newSyms[nSyms],XkbKeySymsPtr(xkb,i),
					XkbKeyNumSyms(xkb,i)*sizeof(KeySym));
	xkb->map->keySymMap[i].offset = nSyms;
	if (i!=key)	nSyms+= XkbKeyNumSyms(xkb,i);
	else		nSyms+= needed;
    }
    free(xkb->map->syms);
    xkb->map->syms = newSyms;
    xkb->map->nSyms = nSyms;
    return &xkb->map->syms[xkb->map->keySymMap[key].offset];
}

void
_XkbResizeKeyType(xkb,ndx,nLevels)
    XkbDescRec	*xkb;
    int		 ndx;
    int		 nLevels;
{
XkbKeyTypeRec	*type= &xkb->map->keyTypes[ndx];
int 		 nTotal;
KeySym		*newSyms;
register int	 i,nSyms;

    if (type->groupWidth==nLevels)
	return;
    for (nTotal=0,i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	if (xkb->map->keySymMap[i].ktIndex==ndx) 
	     nTotal+= XkbKeyNumGroups(xkb,i)*nLevels;
	else nTotal+= XkbKeyNumSyms(xkb,i);
    }
    xkb->map->szSyms= (nTotal*12)/10;
    newSyms = (KeySym *)Xcalloc(xkb->map->szSyms*sizeof(KeySym));
    /* 9/3/93 (ef) -- XXX! deal with allocation failure */
    nSyms= 1;
    for (i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	if (xkb->map->keySymMap[i].ktIndex==ndx) {
	    register int g;
	    int lastLevel,nGroups;

	    if (type->groupWidth>nLevels)	lastLevel= nLevels;
	    else				lastLevel= type->groupWidth;
	    nGroups= XkbKeyNumGroups(xkb,i);
	    for (g=0;g<nGroups;g++) {
		KeySym *pOld;
		pOld= XkbKeySymsPtr(xkb,i)+(g*XkbKeyGroupWidth(xkb,i));
		memcpy(&newSyms[nSyms+(nLevels*g)],pOld,nLevels*sizeof(KeySym));
	    }
	    xkb->map->keySymMap[i].offset = nSyms;
	    nSyms+= nGroups*nLevels;
	}
	else {
	    memcpy(&newSyms[nSyms],XkbKeySymsPtr(xkb,i),
					XkbKeyNumSyms(xkb,i)*sizeof(KeySym));
	    xkb->map->keySymMap[i].offset = nSyms;
	    nSyms+= XkbKeyNumSyms(xkb,i);
	}
    }
    free(xkb->map->syms);
    xkb->map->syms = newSyms;
    xkb->map->nSyms = nSyms;
    return;
}

/***====================================================================***/

#define	KT_1LEVEL_KEY		0
#define	KT_2LEVEL_KEY		1
#define	KT_KEYPAD_KEY		2
#define	KT_3LEVEL_KEY		3
#define	KT_PC_BREAK_KEY		4
#define	KT_PC_SYSRQ_KEY		5
#define	KT_COUNT		6

static void
XkbInitKeyTypes(xkb)
    XkbSrvInfoRec *xkb;
{
XkbKeyTypeRec	*kt;
XkbDescRec	*desc = &xkb->desc;

    if (desc->map->nKeyTypes>0)
	return;

    desc->map->keyTypes=(XkbKeyTypeRec*)Xcalloc(KT_COUNT*sizeof(XkbKeyTypeRec));
    if (!desc->map->keyTypes) {
	desc->map->nKeyTypes = 0;
	return;
    }

    desc->map->nKeyTypes = KT_COUNT;
    kt = &desc->map->keyTypes[KT_1LEVEL_KEY];
    kt->mask = 0;
    kt->groupWidth = 1;
    kt->map = (CARD8 *)Xcalloc(XkbKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->preserve = NULL;
    kt->flags = 0;

    kt = &desc->map->keyTypes[KT_2LEVEL_KEY];
    kt->mask = ShiftMask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XkbKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ShiftMask] = 1;
    kt->preserve = NULL;
    kt->flags = XkbNoFreeKTStruct;

    kt = &desc->map->keyTypes[KT_KEYPAD_KEY];
    kt->mask = ShiftMask|Mod4Mask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XkbKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ShiftMask|Mod4Mask] = 0;	/* shift negates num lock */
    kt->map[ShiftMask] = 1;
    kt->map[Mod4Mask] = 1;
    kt->preserve = NULL;
    kt->flags = XkbNoFreeKTStruct;

    kt = &desc->map->keyTypes[KT_3LEVEL_KEY];
    kt->mask = ShiftMask|Mod3Mask;
    kt->groupWidth = 3;
    kt->map = (CARD8 *)Xcalloc(XkbKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ShiftMask] = 1;
    kt->map[Mod3Mask] = 2;
    kt->map[ShiftMask|Mod3Mask] = 2;
    kt->preserve = NULL;
    kt->flags = XkbNoFreeKTStruct;

    kt = &desc->map->keyTypes[KT_PC_BREAK_KEY];
    kt->mask = ControlMask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XkbKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ControlMask] = 1;
    kt->preserve = NULL;
    kt->flags = XkbNoFreeKTStruct;

    kt = &desc->map->keyTypes[KT_PC_SYSRQ_KEY];
    kt->mask = Mod1Mask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XkbKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[Mod1Mask] = 1;
    kt->preserve = NULL;
    kt->flags = XkbNoFreeKTStruct;
    return;
}

static void
XkbInitKeySyms(xkb)
    XkbSrvInfoRec *xkb;
{
XkbClientMapRec	*map= xkb->desc.map;
int	nKeys= xkb->desc.maxKeyCode-xkb->desc.minKeyCode+1;

    map->nSyms= 1;
    map->szSyms= (nKeys*15)/10;
    map->syms=(KeySym *)Xcalloc(map->szSyms*sizeof(KeySym));
    if (map->syms) {
	register int i;
	XkbSymMapRec *symMap= &map->keySymMap[xkb->desc.minKeyCode];
	for (i=0;i<nKeys;symMap++,i++) {
	    symMap->ktIndex = KT_1LEVEL_KEY;
	    symMap->groupInfo = XkbSetGroupInfo(1,FALSE);
	    symMap->offset = map->nSyms;
	    map->syms[map->nSyms++]= NoSymbol;
	}
    }
    else {
	ErrorF("Fatal Error! Couldn't allocate keysyms array\n");
	GiveUp(1);
    }
    return;
}

static void
XkbInitKeyActions(xkb)
    XkbSrvInfoRec *xkb;
{
XkbServerMapRec	*map= xkb->desc.server;
int	nKeys= xkb->desc.maxKeyCode-xkb->desc.minKeyCode+1;

    map->nActions= 1;
    map->szActions= (nKeys/2)+1;
    map->actions=(XkbAction *)Xcalloc(map->szActions*sizeof(XkbAction));
    if (map->actions) {
	register int i;
	map->actions[0].type = XkbSANoAction;
	for (i=0;i<nKeys;i++) {
	    map->keyActions[i+xkb->desc.minKeyCode]= 0;
	}
    }
    else {
	ErrorF("Fatal Error! Couldn't allocate keysyms array\n");
	GiveUp(1);
    }
    return;
}

#define	RG_BOGUS_FUNCTION_GROUP	0
#define	RG_COUNT		1

static void
XkbInitRadioGroups(xkb)
    XkbSrvInfoRec *xkb;
{
XkbRadioGroupRec	*grp;
 
#ifndef DEBUG_RADIO_GROUPS
    xkb->nRadioGroups = 0;
    xkb->radioGroups = NULL;
    return;
#else
    if ( xkb->nRadioGroups>0 )
	return;

    xkb->radioGroups=(XkbRadioGroupRec *)Xcalloc(RG_COUNT*
						sizeof(XkbRadioGroupRec));
    if (!xkb->radioGroups)
	return;
    xkb->nRadioGroups = RG_COUNT;

    grp = &xkb->radioGroups[RG_BOGUS_FUNCTION_GROUP];
    grp->flags = 0;
    grp->nMembers = 0;
    grp->dfltDown = 0;
#endif
}

#define	SI_LF	(XkbSAClearLocks|XkbSALatchToLock)
static	XkbSymInterpretRec	dfltSI[] = {
    { XK_Caps_Lock,    0, XkbSI_Exactly, LockMask, 4,
					{ XkbSALockMods, SI_LF, LockMask },
					{ XkbDefaultKB, 0, 0 } },
    { XK_Shift_Lock,   0, XkbSI_AnyOf,   0xff, XkbSI_NoIndicator,
					{ XkbSALockMods, SI_LF, ShiftMask },
					{ XkbDefaultKB, 0, 0 } },
    { XK_ISO_Lock,     0, XkbSI_Exactly, LockMask, 4,
					{ XkbSAISOLock, SI_LF, LockMask },
					{ XkbDefaultKB, 0, 0 } },
    { XK_ISO_Level2_Latch, 0, XkbSI_Exactly, ShiftMask, XkbSI_NoIndicator,
					{ XkbSALatchMods, SI_LF, ShiftMask },
					{ XkbDefaultKB, 0, 0 } },
    { XK_ISO_Level3_Latch, XkbSI_UseModMapMods, XkbSI_AnyOf, 0xff, 
					XkbSI_NoIndicator,
					{ XkbSALatchMods, SI_LF, 0 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_Num_Lock, XkbSI_UseModMapMods|XkbSI_UpdateKeypad, 
					XkbSI_AnyOf, 0xff, 5,
					{ XkbSALockMods, SI_LF, 0 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_Scroll_Lock, XkbSI_UseModMapMods, XkbSI_AnyOf, 0xff, 6, 
					{ XkbSALockMods, SI_LF, 0 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_Mode_switch, XkbSI_UpdateGroup, XkbSI_AnyOf, 0xff,XkbSI_NoIndicator,
					{ XkbSASetGroup, SI_LF, 1 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_ISO_Group_Latch, XkbSI_UpdateGroup, XkbSI_AnyOfOrNone, 0xff,
					XkbSI_NoIndicator,
					{ XkbSALatchGroup, SI_LF, 1 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_ISO_Next_Group, XkbSI_UpdateGroup, XkbSI_AnyOfOrNone, 0xff,
					XkbSI_NoIndicator,
					{ XkbSALockGroup, SI_LF, 1 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_ISO_Prev_Group, XkbSI_UpdateGroup, XkbSI_AnyOfOrNone, 0xff, 
					XkbSI_NoIndicator,
					{ XkbSALockGroup, SI_LF, (CARD8)-1 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_KP_Left, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
					{ XkbSAMovePtrBtn, (CARD8)-1, 0 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_KP_Right, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
					{ XkbSAMovePtrBtn, 1, 0 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_KP_Up, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
					{ XkbSAMovePtrBtn, 0, (CARD8)-1 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_KP_Down, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
					{ XkbSAMovePtrBtn, 0, 1 },
					{ XkbDefaultKB, 0, 0 } },
    { XK_KP_Begin, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, XkbSAUseDfltButton },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Button_Dflt, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, XkbSAUseDfltButton },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Button1, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, 1 },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Button2, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, 2 },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Button3, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, 3 },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Button4, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, 4 },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Button5, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSAPtrBtn, 1, 5 },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_EnableKeys, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSALockControls, 0, XkbMouseKeysMask },
			{ XkbDefaultKB, 0, 0 } },
    { XK_Pointer_Accelerate, 0, XkbSI_AnyOfOrNone, 0xff, XkbSI_NoIndicator,
			{ XkbSALockControls, 0, XkbMouseKeysAccelMask },
			{ XkbDefaultKB, 0, 0 } },
    { NoSymbol, XkbSI_UseModMapMods, XkbSI_AnyOf, 0xff, XkbSI_NoIndicator,
					{ XkbSASetMods, SI_LF, 0 },
					{ XkbDefaultKB, 0, 0 } }
};

static void
XkbInitCompatStructs(xkb)
    XkbSrvInfoRec *xkb;
{
register int i;
XkbCompatRec	*compat;

    compat = xkb->desc.compat;
    for (i=0;i<8;i++) {
	compat->modCompat[i].mods = (1<<i);
	compat->modCompat[i].groups = 0;
    }

    compat->nSymInterpret= 0;
    compat->symInterpret = (XkbSymInterpretRec *)Xcalloc(sizeof(dfltSI));
    if (compat->symInterpret) {
	compat->nSymInterpret = sizeof(dfltSI)/sizeof(XkbSymInterpretRec);
	memcpy(compat->symInterpret,dfltSI,sizeof(dfltSI));
    }
    return;
}

/***====================================================================***/

static void
XkbInitNames(xkb)
    XkbSrvInfoRec *xkb;
{
XkbNamesRec	*names;

    names = xkb->desc.names;
    bzero(names,sizeof(XkbNamesRec));
    names->keycodes= CREATE_ATOM("SGI_IRIS");
    names->geometry= CREATE_ATOM("SGI_102");
    names->symbols= CREATE_ATOM("ASCII");
    names->modifiers[0] = CREATE_ATOM("Shift");
    names->modifiers[1] = CREATE_ATOM("Caps");
    names->modifiers[2] = CREATE_ATOM("Control");
    names->modifiers[3] = CREATE_ATOM("Alt");
    names->modifiers[4] = CREATE_ATOM("Mod2");
    names->modifiers[5] = CREATE_ATOM("Mod3");
    names->modifiers[6] = CREATE_ATOM("Mod4");
    names->modifiers[7] = CREATE_ATOM("Mod5");
    names->indicators[4] = CREATE_ATOM("Caps Lock");
    names->indicators[5] = CREATE_ATOM("Num Lock");
    names->indicators[6] = CREATE_ATOM("Scroll Lock");
    names->keyTypes= (Atom *)Xcalloc(KT_COUNT*sizeof(Atom));
    if (names->keyTypes) {
	names->keyTypes[KT_1LEVEL_KEY]= CREATE_ATOM("ONE_LEVEL");
	names->keyTypes[KT_2LEVEL_KEY]= CREATE_ATOM("TWO_LEVEL");
	names->keyTypes[KT_3LEVEL_KEY]= CREATE_ATOM("THREE_LEVEL");
	names->keyTypes[KT_KEYPAD_KEY]= CREATE_ATOM("KEYPAD");
	names->keyTypes[KT_PC_BREAK_KEY]= CREATE_ATOM("PC_BREAK");
	names->keyTypes[KT_PC_SYSRQ_KEY]= CREATE_ATOM("PC_SYS_RQ");
    }
    names->levels= (Atom **)Xcalloc(KT_COUNT*sizeof(Atom *));
    if (names->levels) {
	names->levels[KT_1LEVEL_KEY]= (Atom *)Xcalloc(sizeof(Atom));
	names->levels[KT_1LEVEL_KEY][0]= CREATE_ATOM("Any");
	names->levels[KT_2LEVEL_KEY]= (Atom *)Xcalloc(2*sizeof(Atom));
	names->levels[KT_2LEVEL_KEY][0]= CREATE_ATOM("Base");
	names->levels[KT_2LEVEL_KEY][1]= CREATE_ATOM("Shift");
	names->levels[KT_KEYPAD_KEY]= (Atom *)Xcalloc(2*sizeof(Atom));
	names->levels[KT_KEYPAD_KEY][0]= CREATE_ATOM("Base");
	names->levels[KT_KEYPAD_KEY][1]= CREATE_ATOM("Num Lock");
	names->levels[KT_3LEVEL_KEY]= (Atom *)Xcalloc(3*sizeof(Atom));
	names->levels[KT_3LEVEL_KEY][0]= CREATE_ATOM("Base");
	names->levels[KT_3LEVEL_KEY][1]= CREATE_ATOM("Shift");
	names->levels[KT_3LEVEL_KEY][2]= CREATE_ATOM("Level 3");
	names->levels[KT_PC_BREAK_KEY]= (Atom *)Xcalloc(2*sizeof(Atom));
	names->levels[KT_PC_BREAK_KEY][0]= CREATE_ATOM("Base");
	names->levels[KT_PC_BREAK_KEY][1]= CREATE_ATOM("Control");
	names->levels[KT_PC_SYSRQ_KEY]= (Atom *)Xcalloc(2*sizeof(Atom));
	names->levels[KT_PC_SYSRQ_KEY][0]= CREATE_ATOM("Base");
	names->levels[KT_PC_SYSRQ_KEY][1]= CREATE_ATOM("Alt");
    }
    if (xkb->nRadioGroups>0) {
	names->radioGroups= (Atom *)Xcalloc(xkb->nRadioGroups*sizeof(Atom));
	names->radioGroups[RG_BOGUS_FUNCTION_GROUP]= CREATE_ATOM("BOGUS");
    }
    names->nCharSets= 1;
    names->charSets= (Atom *)Xcalloc(sizeof(Atom));
    names->charSets[0]= CREATE_ATOM("iso8859-1");
    return;
}

static void
XkbInitAlternateSyms(xkb)
    XkbSrvInfoRec *xkb;
{
XkbAlternateSymsRec *alt;

   alt= (XkbAlternateSymsRec *)Xcalloc(sizeof(XkbAlternateSymsRec));
   if (alt) {
	alt->name= CREATE_ATOM("BOGUS_DE");
	alt->index= 0;
	alt->nCharSets= 1;
	alt->charSets= (Atom *)Xcalloc(sizeof(Atom));
	alt->charSets[0]= CREATE_ATOM("iso8859-1");
	alt->firstKey= 27;
	alt->nKeys= 13;
	alt->nSyms= 4;
	alt->syms= (KeySym *)Xcalloc(alt->nSyms*sizeof(KeySym));
	alt->syms[0]= XK_y;
	alt->syms[1]= XK_Y;
	alt->syms[2]= XK_z;
	alt->syms[3]= XK_Z;
	alt->maps= (XkbSymMapRec *)Xcalloc(alt->nKeys*sizeof(XkbSymMapRec));
	bzero(alt->maps,alt->nKeys*sizeof(XkbSymMapRec));
	alt->maps[0].ktIndex= KT_2LEVEL_KEY;
	alt->maps[0].groupInfo= XkbSetGroupInfo(1,0);
	alt->maps[0].offset= 0;
	alt->maps[12].ktIndex= KT_2LEVEL_KEY;
	alt->maps[12].groupInfo= XkbSetGroupInfo(1,0);
	alt->maps[12].offset= 2;
	alt->next = NULL;
   }
   xkb->desc.altSymSets= alt;
   return;
}

unsigned
XkbIndicatorsToUpdate(keybd,modsChanged)
    DeviceIntRec		*keybd;
    unsigned long	 	 modsChanged;
{
register unsigned update = 0;
XkbSrvInfoRec	*xkb = keybd->key->xkbInfo;

    if (modsChanged&(XkbModifierStateMask|XkbGroupStateMask))
	update|= xkb->iAccel.usesEffective;
    if (modsChanged&(XkbModifierBaseMask|XkbGroupBaseMask))
	update|= xkb->iAccel.usesBase;
    if (modsChanged&(XkbModifierLatchMask|XkbGroupLatchMask))
	update|= xkb->iAccel.usesLatched;
    if (modsChanged&(XkbModifierLockMask|XkbGroupLockMask))
	update|= xkb->iAccel.usesLocked;
    if (modsChanged&XkbCompatStateMask)
	update|= xkb->iAccel.usesCompat;
    return update;
}
    
void
XkbUpdateIndicators(keybd,update,pChanges)
    DeviceIntRec		*keybd;
    register CARD32		 update;
    XkbIndicatorChangesRec	*pChanges;
{
register int	i,bit;
XkbSrvInfoRec	*xkb = keybd->key->xkbInfo;
XkbIndicatorRec	*map = xkb->desc.indicators;
XkbStateRec	*kbdState = &xkb->state;
CARD32		 oldState;


    oldState= xkb->iState;
    for (i=0,bit=1;update;i++,bit<<=1) {
	if (update&bit) {
	    int on;
	    CARD8 mods,group;
	    XkbIndicatorMapRec *map= &xkb->desc.indicators->maps[i];
	    on= mods= group= 0;
	    if (map->whichMods&XkbIMUseBase)
		mods|= kbdState->baseMods;
	    if (map->whichGroups&XkbIMUseBase)
		group|= (1L << kbdState->baseGroup);

	    if (map->whichMods&XkbIMUseLatched)
		mods|= kbdState->latchedMods;
	    if (map->whichGroups&XkbIMUseLatched)
		group|= (1L << kbdState->latchedGroup);

	    if (map->whichMods&XkbIMUseLocked)
		mods|= kbdState->lockedMods;
	    if (map->whichGroups&XkbIMUseLocked)
		group|= (1L << kbdState->lockedGroup);

	    if (map->whichMods&XkbIMUseEffectiveLocked)
		mods|= kbdState->lockedMods&(~kbdState->unlockedMods);
	    if (map->whichGroups&XkbIMUseEffectiveLocked)
		group= (1L<<(kbdState->groupsUnlocked?0:kbdState->lockedGroup));

	    if (map->whichMods&XkbIMUseEffective)
		mods|= kbdState->mods;
	    if (map->whichGroups&XkbIMUseEffective)
		group|= (1L << kbdState->group);

	    if (map->whichMods&XkbIMUseCompat)
		mods|= kbdState->compatState;

	    if ((map->whichMods|map->whichGroups)&XkbIMUseAnyState) {
		on = (((map->mods&mods)!=0)||(map->mods==mods));
		on = on && (((map->groups&group)!=0)||(map->groups==0));
	    }
	    if (map->controls)
		on = on || (xkb->desc.controls->enabledControls&map->controls);

	    if ((on)&&(!(xkb->iState&(1<<i)))) {
		xkb->iState|= (1<<i);
	    }
	    else if ((!on)&&(xkb->iState&(1<<i))) {
		xkb->iState&= ~(1<<i);
	    }
	    update&= ~bit;
	}
    }
    if (xkb->iState!=oldState) {
	DDXUpdateIndicators(keybd,oldState,xkb->iState);
    }
    if (pChanges)
	pChanges->stateChanges|= (xkb->iState^oldState);
    else if ((xkb->iState^oldState)!=0) {
	xkbIndicatorNotify	in;
	in.stateChanged= (xkb->iState^oldState);
	in.state = xkb->iState;
	in.mapChanged = 0;
	XkbSendIndicatorNotify(keybd,&in);
    }
    return;
}


#if NeedFunctionPrototypes
void
XkbUpdateKeyTypesFromCore(	DeviceIntRec	*pXDev,
				KeyCode	 	first,
				CARD8	 	num,
				xkbMapNotify	*pMN)
#else
void
XkbUpdateKeyTypesFromCore(pXDev,first,num,pMN)
    DeviceIntPtr pXDev;
    KeyCode first;
    CARD8 num;
    xkbMapNotify *pMN;
#endif
{
XkbDescRec *xkb;
XkbSymMapRec	*map;
int	r,mapWidth,nLevels,nGroups;
KeySym	*pSym,*newSyms;
KeySym	core[4];

    xkb= &pXDev->key->xkbInfo->desc;
    xkb->minKeyCode = pXDev->key->curKeySyms.minKeyCode;
    xkb->maxKeyCode = pXDev->key->curKeySyms.maxKeyCode;

    mapWidth= pXDev->key->curKeySyms.mapWidth;
    map= &xkb->map->keySymMap[first];
    pSym= &pXDev->key->curKeySyms.map[(first-xkb->minKeyCode)*mapWidth];
    for (r=0; r<num; r++,map++,pSym+= mapWidth) {
	core[0]= pSym[0];
	if (mapWidth>1)	core[1]= pSym[1];
	else		core[1]= NoSymbol;
	if (mapWidth>2)	core[2]= pSym[2];
	else		core[2]= NoSymbol;
	if (mapWidth>3)	core[3]= pSym[3];
	else		core[3]= NoSymbol;

	if (((core[0]&0xffffff00)==0)&&(core[1]==NoSymbol)) {
	    if (isupper(core[0])) {
		core[1]= core[0];
		core[0]= tolower(core[0]);
	    }
	}
	if (((core[2]&0xffffff00)==0)&&(core[3]==NoSymbol)) {
	    if (isupper(core[2])) {
		core[3]= core[2];
		core[2]= tolower(core[2]);
	    }
	}
	if ((core[1]==NoSymbol)&&(core[3]==NoSymbol))
	     nLevels= 1;
	else nLevels= 2;
	if ((core[2]==NoSymbol)&&(core[3]==NoSymbol))
	     nGroups= 1;
	else nGroups= 2;
	newSyms= _XkbNewSymsForKey(xkb,r+first,nGroups*nLevels);
	if (nLevels==1) {
	    newSyms[0]= core[0];
	    if (nGroups>1)
		newSyms[1]= core[2];
	}
	else {
	    newSyms[0]= core[0];
	    newSyms[1]= core[1];
	    if (nGroups>1) {
		newSyms[2]= core[2];
		newSyms[3]= core[3];
	    }
	}
	if ((nGroups>1)&&(xkb->controls->numGroups<2)) {
	   XkbControlsRec *controls= xkb->controls;
	   controls->numGroups= nGroups;
	   /* 8/19/93 (ef) -- XXX! generate controls notify here */
	}
/* 8/10/93 (ef) -- XXX! Look up correct index here */
	if (nLevels==1)
	     map->ktIndex= KT_1LEVEL_KEY;
	else if (IsKeypadKey(newSyms[0])&&IsKeypadKey(newSyms[1]))
	     map->ktIndex= KT_KEYPAD_KEY;
	else map->ktIndex= KT_2LEVEL_KEY;
	map->groupInfo= XkbSetGroupInfo(map->groupInfo,nGroups);
    }
    if (pMN->changed&XkbKeySymsMask) {
	CARD8 oldLast,newLast;
	oldLast = pMN->firstKeySym+pMN->nKeySyms-1;
	newLast = first+num-1;

	if (first<pMN->firstKeySym)
	    pMN->firstKeySym = first;
	if (newLast>oldLast)
	    newLast= oldLast;
	pMN->nKeySyms = newLast-pMN->firstKeySym+1;
    }
    else {
	pMN->changed|= XkbKeySymsMask;
	pMN->firstKeySym = first;
	pMN->nKeySyms = num;
    }
    return;
}

#if NeedFunctionPrototypes
void
XkbUpdateActions(	DeviceIntRec	*pXDev,
			KeyCode		 first,
			CARD8		 num,
			xkbMapNotify	*pMN)
#else
void
XkbUpdateActions(pXDev,first,num,pMN)
    DeviceIntPtr pXDev;
    KeyCode first;
    CARD8 num;
    xkbMapNotify *pMN;
#endif
{
XkbDescRec *xkb;
register int	 i,r,n;
int		 nSyms,found;
KeySym		*pSym;
XkbSymInterpretRec  *interps[8];
CARD8		 mods,repeat[32];

    xkb= &pXDev->key->xkbInfo->desc;
    if (XkbComputeAutoRepeat && pXDev->kbdfeed)
	memcpy(repeat,pXDev->kbdfeed->ctrl.autoRepeats,32);

    for (r=0; r<num; r++) {
	mods= pXDev->key->modifierMap[r+first];
	pSym= XkbKeySymsPtr(xkb,r+first);
	nSyms= XkbKeyNumSyms(xkb,r+first);
	found= 0;
	for (n=0;n<nSyms;n++,pSym++) {
	    interps[n]= NULL;
	    if (*pSym!=NoSymbol) {
		XkbSymInterpretRec *interp;
		interp= xkb->compat->symInterpret;
		for (i=0;i<xkb->compat->nSymInterpret;i++,interp++) {
		    if ((interp->sym==NoSymbol)||(*pSym==interp->sym)) {
			int match;
			switch (interp->match) {
			    case XkbSI_NoneOf:
				match= ((interp->mods&mods)==0);
				break;
			    case XkbSI_AnyOfOrNone:
				match= ((mods==0)||((interp->mods&mods)!=0));
				break;
			    case XkbSI_AnyOf:
				match= ((interp->mods&mods)!=0);
				break;
			    case XkbSI_AllOf:
				match= ((interp->mods&mods)==interp->mods);
				break;
			    case XkbSI_Exactly:
				match= (interp->mods==mods);
				break;
			    default:
				ErrorF("Illegal match in UpdateActions\n");
				match= 0;
				break;
			}
			if (match) {
			    if (interp->sym!=NoSymbol) {
				interps[n]= interp;
				break;
			    }
			    else if (interps[n]==NULL) {
				interps[n]= interp;
			    }
			}
		    }
		}
		if (interps[n]&&interps[n]->action.type!=XkbSANoAction)
		    found++;
	    }
	}
	if (!found)
	    xkb->server->keyActions[r+first]= 0;
	else {
	    XkbAction *pActs= _XkbNewActionsForKey(xkb,r+first,nSyms);
	    for (i=0;i<nSyms;i++) {
		if (interps[i]) {
		    pActs[i]= interps[i]->action;
		    if (interps[i]->flags&XkbSI_UseModMapMods)
			pActs[i].data= mods;
		}
		else pActs[i].type= XkbSANoAction;
	    }
	    if (interps[0]) {
		xkb->server->keyBehaviors[r+first]= interps[0]->behavior;
		if (XkbComputeAutoRepeat) {
		    if (interps[0]->flags&XkbSI_Autorepeat)
			repeat[(r+first)/8]|= (1<<((r+first)%8));
		    else {
			repeat[(r+first)/8]&= ~(1<<((r+first)%8));
		    }
		}
		if (interps[0]->flags&XkbSI_UpdateKeypad) {
		    CARD8 oldMod= pXDev->key->xkbInfo->dfltNumLockMod;
		    if (oldMod!=mods) {
			XkbKeyTypeRec *keypad;
			CARD8	tmpMod,tmpBoth;
			keypad = &xkb->map->keyTypes[KT_KEYPAD_KEY];
			tmpMod= keypad->map[oldMod&keypad->mask];
			tmpBoth= keypad->map[(oldMod|ShiftMask)&keypad->mask];

			pXDev->key->xkbInfo->dfltNumLockMod= mods;
			keypad->mask= ShiftMask|mods;
			keypad->map[mods&keypad->mask]= tmpMod;
			keypad->map[(mods|ShiftMask)&keypad->mask]= tmpBoth;
			/* 8/12/93 (ef) -- XXX! What about preserve?? */
			/* 8/12/93 (ef) -- XXX! Generate MapNotify */
		    }
		}
		if (interps[0]->flags&XkbSI_UpdateGroup) {
		    CARD8 oldMod = pXDev->key->xkbInfo->compatGroup2Mod;
		    if (oldMod!=mods) {
			register int i,bit;
			for (i=0,bit=1;i<8;i++,bit<<=1) {
			    if (mods&bit)
				xkb->compat->modCompat[i].groups= 0xfe;
			    else if (oldMod&bit)
				xkb->compat->modCompat[i].groups= 0;
			}
			pXDev->key->xkbInfo->compatGroup2Mod= mods;
			/* 8/12/93 (ef) -- XXX! Generate CompatMapNotify */
		    }
		}
		if (interps[0]->flags&XkbSI_UpdateInternal) {
		    xkb->controls->internalMods|= mods;
		    /* 9/20/93 (ef) -- XXX! Generate ControlsNotify? */
		}
		if (interps[0]->flags&XkbSI_UpdateIgnoreLocks) {
		    xkb->controls->ignoreLockMods|= mods;
		    /* 9/20/93 (ef) -- XXX! Generate ControlsNotify? */
		}
		if (interps[0]->indicator!=XkbSI_NoIndicator) {
		    XkbIndicatorMapRec *map;
		    map= &xkb->indicators->maps[interps[0]->indicator];
		    if (interps[0]->flags&XkbSI_UpdateGroup) {
			map->whichGroups|= XkbIMUseLocked;
			map->mods= 0xff;
			map->groups= 0xfe;
		    }
		    else {
			map->whichMods|= XkbIMUseLocked;
			map->mods= mods;
			map->groups= 0;
		    }
		    pXDev->key->xkbInfo->iAccel.usesLocked|= 
						(1<<interps[0]->indicator);
		    pXDev->key->xkbInfo->iAccel.usedComponents|=XkbIMUseLocked;
		    XkbUpdateIndicators(pXDev,(1<<interps[0]->indicator),NULL);
		}
	    }
	}
	if ((XkbComputeAutoRepeat)&&(!found)||(interps[0]==NULL))
	    repeat[(r+first)/8]|= (1<<((r+first)%8));
    }

    if (XkbComputeAutoRepeat && pXDev->kbdfeed) {
        memcpy(pXDev->kbdfeed->ctrl.autoRepeats,repeat, 32);
        if (pXDev->kbdfeed->ctrl.autoRepeat)
            (*pXDev->kbdfeed->CtrlProc)(pXDev, &pXDev->kbdfeed->ctrl);
    }
    if (pMN->changed&XkbKeyActionsMask) {
	CARD8 oldLast,newLast;
	oldLast = pMN->firstKeyAction+pMN->nKeyActions-1;
	newLast = first+num-1;

	if (first<pMN->firstKeyAction)
	    pMN->firstKeyAction = first;
	if (newLast>oldLast)
	    newLast= oldLast;
	pMN->nKeyActions = newLast-pMN->firstKeyAction+1;
    }
    else {
	pMN->changed|= XkbKeyActionsMask;
	pMN->firstKeyAction = first;
	pMN->nKeyActions = num;
    }
    return;
}

void
XkbUpdateCoreDescription(keybd)
    DeviceIntPtr keybd;
{
    /* 8/3/93 (ef) -- XXX! Not implemented yet */
}

static void
XkbInitIndicatorMap(xkb)
    XkbSrvInfoRec *xkb;
{
XkbIndicatorRec	*map = xkb->desc.indicators;
register int i;

    map->nRealIndicators = 7;
    xkb->iState= 0;
    bzero(map->maps,XkbNumIndicators*sizeof(XkbIndicatorMapRec));
    map->maps[4].whichMods= XkbIMUseLocked;
    map->maps[4].mods= LockMask;
    xkb->iAccel.usesLocked= (1<<4);
    xkb->iAccel.usedComponents= XkbModifierLockMask;
    return;
}

void
XkbInitDevice(pXDev)
    DeviceIntPtr pXDev;
{
int	nKeys,i;
XkbSrvInfoRec *xkb;
xkbMapNotify	mn;

    /* determine size of fixed-length arrays */
    i= sizeof(XkbAction)+sizeof(XkbSymMapRec)+sizeof(CARD16);
    i*= pXDev->key->curKeySyms.maxKeyCode+1;

    pXDev->key->xkbInfo= xkb=(XkbSrvInfoRec *)Xcalloc(sizeof(XkbSrvInfoRec)+i);
    if ( xkb ) {
	xkb->desc.controls = &xkb->Controls;
	xkb->desc.map = &xkb->Map;
	xkb->desc.server = &xkb->Server;
	xkb->desc.indicators = &xkb->Indicators;
	xkb->desc.compat = &xkb->Compat;
	xkb->desc.names = &xkb->Names;
	xkb->desc.deviceSpec = pXDev->id;
	xkb->desc.flags = 1;
	xkb->desc.minKeyCode = pXDev->key->curKeySyms.minKeyCode;
	xkb->desc.maxKeyCode = pXDev->key->curKeySyms.maxKeyCode;

	nKeys = xkb->desc.maxKeyCode+1;	/* size of fixed arrays */
	xkb->Map.keySymMap= (XkbSymMapRec *)&xkb[1];
	xkb->Server.keyBehaviors = (XkbAction *)&xkb->Map.keySymMap[nKeys];
	xkb->Server.keyActions=(CARD16*)&xkb->Server.keyBehaviors[nKeys];

	xkb->dfltNumLockMod = Mod4Mask;
	xkb->compatGroup2Mod = Mod3Mask;
	xkb->dfltPtrDelta=1;
	xkb->interest = NULL;
	xkb->device = pXDev;

	XkbInitNames(xkb);
	XkbInitKeyTypes(xkb);
	XkbInitKeySyms(xkb);
	XkbInitKeyActions(xkb);
	XkbInitRadioGroups(xkb);
	XkbInitCompatStructs(xkb);

	xkb->state.group = 0;
	xkb->state.latchedGroup = 0;
	xkb->state.mods = 0;
	xkb->state.latchedMods = 0;
	xkb->state.lockedMods = 0;
	xkb->state.compatState = 0;
	xkb->state.unlockedMods = 0;
	xkb->state.groupsUnlocked = 0;

	XkbInitIndicatorMap(xkb);

	xkb->desc.controls->numGroups = 1;
	xkb->desc.controls->internalMods = 0x00;
	xkb->desc.controls->ignoreLockMods = 0xe040;
	xkb->desc.controls->enabledControls = XkbAccessXKeysMask|
					 	XkbAccessXTimeoutMask|
						XkbRepeatKeysMask|
						XkbGroupsWrapMask|
						XkbMouseKeysAccelMask|
						XkbAudibleBellMask|
						XkbAutoAutorepeatMask;
	AccessXInit(pXDev);

	XkbUpdateKeyTypesFromCore(pXDev,xkb->desc.minKeyCode,
						XkbNumKeys(&xkb->desc),&mn);
	XkbUpdateActions(pXDev,xkb->desc.minKeyCode,XkbNumKeys(&xkb->desc),&mn);
    }
}

	/*
	 * Be very careful about what does and doesn't get freed by this 
	 * function.  To reduce fragmentation, XkbInitDevice allocates a 
	 * single huge block per device and divides it up into most of the 
	 * fixed-size structures for the device.   Don't free anything that
	 * is part of this larger block.
	 */
void
XkbFreeInfo(xkb)
    XkbSrvInfoRec *xkb;
{
    if (xkb->radioGroups) {
	Xfree(xkb->radioGroups);
	xkb->radioGroups= NULL;
    }
    if (xkb->mouseKeyTimer) {
	TimerFree(xkb->mouseKeyTimer);
	xkb->mouseKeyTimer= NULL;
    }
    if (xkb->slowKeysTimer) {
	TimerFree(xkb->slowKeysTimer);
	xkb->slowKeysTimer= NULL;
    }
    if (xkb->bounceKeysTimer) {
	TimerFree(xkb->bounceKeysTimer);
	xkb->bounceKeysTimer= NULL;
    }
    if (xkb->repeatKeyTimer) {
	TimerFree(xkb->repeatKeyTimer);
	xkb->repeatKeyTimer= NULL;
    }
    if (xkb->krgTimer) {
	TimerFree(xkb->krgTimer);
	xkb->krgTimer= NULL;
    }
    if (xkb->desc.controls)
	xkb->desc.controls= NULL;
    if (xkb->desc.server) {
	if (xkb->desc.server->actions)
	    Xfree(xkb->desc.server->actions);
	xkb->desc.server->nActions= xkb->desc.server->szActions= 0;
	xkb->desc.server->actions= NULL;
	if (xkb->desc.server->keyBehaviors)
	    xkb->desc.server->keyBehaviors= NULL;
	if (xkb->desc.server->keyActions)
	    xkb->desc.server->keyActions= NULL;
	xkb->desc.server= NULL;
    }
    if (xkb->desc.names) {
	if (xkb->desc.names->keyTypes)
	    Xfree(xkb->desc.names->keyTypes);
	xkb->desc.names->keyTypes= NULL;
	if (xkb->desc.names->levels) {
	    register int i;
	    for (i=0;i<xkb->desc.map->nKeyTypes;i++) {
		if (xkb->desc.names->levels[i]) {
		    Xfree(xkb->desc.names->levels[i]);
		    xkb->desc.names->levels[i]= NULL;
		}
	    }
	    Xfree(xkb->desc.names->levels);
	    xkb->desc.names->levels= NULL;
	}
	if (xkb->desc.names->radioGroups) {
	    Xfree(xkb->desc.names->radioGroups);
	    xkb->desc.names->radioGroups= NULL;
	}
	if (xkb->desc.names->charSets) {
	    Xfree(xkb->desc.names->charSets);
	    xkb->desc.names->charSets= NULL;
	}
	xkb->desc.names= NULL;
    }
    if (xkb->desc.map) {
	if (xkb->desc.map->keyTypes) {
	    register int i;
	    XkbKeyTypeRec	*type= xkb->desc.map->keyTypes;
	    for (i=0;i<xkb->desc.map->nKeyTypes;i++,type++) {
		if (type->preserve&&(!(type->flags&XkbNoFreeKTPreserve))){
		    Xfree(type->preserve);
		    type->preserve= NULL;
		}
		if (type->map&&(!(type->flags&XkbNoFreeKTMap))) {
		    Xfree(type->map);
		    type->map= NULL;
	        }
	    }
	    type= xkb->desc.map->keyTypes;
	    if (!(type->flags&XkbNoFreeKTStruct)) {
		Xfree(type);
		xkb->desc.map->keyTypes= NULL;
	    }
	}
	if (xkb->desc.map->syms) {
	    Xfree(xkb->desc.map->syms);
	    xkb->desc.map->syms= NULL;
	}
	if (xkb->desc.map->keySymMap) {
	    xkb->desc.map->keySymMap= NULL;
	}
	xkb->desc.map= NULL;
    }
    if (xkb->desc.indicators) {
	xkb->desc.indicators= NULL;
    }
    if (xkb->desc.compat) {
	if (xkb->desc.compat->symInterpret)
	    Xfree(xkb->desc.compat->symInterpret);
	xkb->desc.compat->nSymInterpret= 0;
	xkb->desc.compat->symInterpret= NULL;
	xkb->desc.compat= NULL;
    }
    if (xkb->desc.altSymSets) {
	XkbAlternateSymsRec *this,*next;
	this= xkb->desc.altSymSets;
	next= this->next;
	while (this) {
	    if (this->charSets)
		Xfree(this->charSets);
	    this->nCharSets= 0;
	    this->charSets= NULL;
	    if (this->syms)
		Xfree(this->syms);
	    this->nSyms= 0;
	    this->syms= NULL;
	    if (this->maps)
		Xfree(this->maps);
	    this->nKeys= 0;
	    this->maps= NULL;
	    Xfree(this);
	    this= next;
	}
	xkb->desc.altSymSets= NULL;
    }
    Xfree(xkb);
    return;
}

#if NeedFunctionPrototypes
void
XkbApplyMappingChange(	DeviceIntRec	*kbd,
			CARD8		 request,
			KeyCode		 firstKey,
			CARD8		 num)
#else
void
XkbApplyMappingChange(kbd,request,firstKey,num)
    DeviceIntPtr kbd;
    CARD8 request;
    KeyCode firstKey;
    CARD8 num;
#endif
{
xkbMapNotify mn;

    if (kbd->key->xkbInfo==NULL)
	XkbInitDevice(kbd);
    bzero(&mn,sizeof(xkbMapNotify));
    if (request==MappingKeyboard) {
	XkbUpdateKeyTypesFromCore(kbd,firstKey,num,&mn);
	XkbUpdateActions(kbd,firstKey,num,&mn);
    }
    else if (request==MappingModifier) {
	XkbDescRec *desc= &kbd->key->xkbInfo->desc;
	num = desc->maxKeyCode-desc->minKeyCode+1;
	XkbUpdateActions(kbd,desc->minKeyCode,num,&mn);
    }
    if (mn.changed)
	XkbSendMapNotify(kbd,&mn);
    return;
}

void
XkbDisableComputedAutoRepeats()
{
     XkbComputeAutoRepeat = 0;
}

unsigned
XkbStateChangedFlags(old,new)
    XkbStateRec *old;
    XkbStateRec *new;
{
int		groupUnlatch,changed;

    changed=(old->group!=new->group?XkbGroupStateMask:0);
    changed|=(old->baseGroup!=new->baseGroup?XkbGroupBaseMask:0);
    changed|=(old->latchedGroup!=new->latchedGroup?XkbGroupLatchMask:0);
    changed|=(old->lockedGroup!=new->lockedGroup?XkbGroupLockMask:0);
    changed|=(old->mods!=new->mods?XkbModifierStateMask:0);
    changed|=(old->baseMods!=new->baseMods?XkbModifierBaseMask:0);
    changed|=(old->latchedMods!=new->latchedMods?XkbModifierLatchMask:0);
    changed|=(old->lockedMods!=new->lockedMods?XkbModifierLockMask:0);
    changed|=(old->compatState!=new->compatState?XkbCompatStateMask:0);
    changed|=(old->unlockedMods!=new->unlockedMods?XkbModifierUnlockMask:0);
    changed|=(old->groupsUnlocked!=new->groupsUnlocked?XkbGroupUnlockMask:0);
    return changed;
}

/***====================================================================***/

static XkbAction
XkbKeyAction(xkb,xkbState,keycode)
    XkbDescRec	*xkb;
    XkbStateRec *xkbState;
    CARD8 	 keycode;
{
int		n;
int		col;
XkbKeyTypeRec	*map;
XkbAction	*pActs;
static XkbAction fake;

    n= XkbKeyNumActions(xkb,keycode);
    pActs= XkbKeyActionsPtr(xkb,keycode);
    if (n==1)
	col = 0;
    else {
	map= XkbKeyKeyType(xkb,keycode);
	col= map->map[map->mask&xkbState->mods];
	if ((xkbState->group!=0)&&(XkbKeyNumGroups(xkb,keycode)>1)) {
	    int effectiveGroup = xkbState->group;
	    if (effectiveGroup>=XkbKeyNumGroups(xkb,keycode)) {
		if ( XkbKeyGroupsWrap(xkb,keycode) ) {
		     effectiveGroup %= XkbKeyNumGroups(xkb,keycode);
		}
		else effectiveGroup  = XkbKeyNumGroups(xkb,keycode)-1;
	    }
	    col+= (effectiveGroup*map->groupWidth);
	}
    }
    if (xkb->controls->enabledControls&XkbStickyKeysMask) {
	if (pActs[col].type==XkbSASetMods) {
	    fake.type = XkbSALatchMods;
	    fake.flags= XkbSAClearLocks|XkbSALatchToLock;
	    fake.data= pActs[col].data;
	    return fake;
	}
	if (pActs[col].type==XkbSASetGroup) {
	    fake.type = XkbSALatchGroup;
	    fake.flags= XkbSAClearLocks|XkbSALatchToLock;
	    fake.data= pActs[col].data;
	    return fake;
	}
    }
    else if (XkbPtrAction(&pActs[col])&&
	     ((xkb->controls->enabledControls&XkbMouseKeysMask)==0)) {
	fake.type = XkbSANoAction;
	fake.flags= 0;
	fake.data= 0;
	return fake;
    }
    return pActs[col];
}

static void
_XkbAdjustGroup(pGroup,nGroups,groupsWrap)
    CARD8 *pGroup;
    CARD8  nGroups;
    Bool   groupsWrap;
{
register int group = *(INT8 *)pGroup;

    if (group>=nGroups) {
	if ( groupsWrap )	group%= nGroups;
	else 			group= nGroups-1;
    }
    else if (group<0) {
	while ( group < 0 )  {
	    if ( groupsWrap )	group+= nGroups;
	    else	    	group= 0;
	}
    }
    *pGroup= group;
    return;
}

void
XkbComputeCompatState(xkb)
    XkbSrvInfoRec	*xkb;
{
    register int i,bit;
    CARD8 grabGroup,lookupGroup;
    XkbStateRec	*state= &xkb->state;
    XkbModCompatRec *map;

    lookupGroup= (xkb->lookupState>>13)&0x7;
    grabGroup= (xkb->grabState>>13)&0x7;
    state->compatState = 0;
    xkb->compatLookupState= 0;
    xkb->compatGrabState= 0;
    map = &xkb->desc.compat->modCompat[0];
    for (i=0,bit=1;i<8;i++,bit<<=1,map++) {
	if ((map->mods&state->mods)||(map->groups&(1<<state->group)))
	    state->compatState|= bit;

	if ((map->mods&xkb->lookupState)||(map->groups&(1<<lookupGroup)))
	    xkb->compatLookupState|= bit;

	if ((map->mods&xkb->grabState)||(map->groups&(1<<grabGroup)))
	    xkb->compatGrabState|= bit;
    }
}

void
XkbComputeDerivedState(xkb)
    XkbSrvInfoRec	*xkb;
{
XkbStateRec	*state= &xkb->state;
XkbControlsRec	*controls= xkb->desc.controls;
Bool		wrap= ((controls->enabledControls&XkbGroupsWrapMask)!=0);

    state->mods= (state->baseMods|state->latchedMods);
    state->mods|= (state->lockedMods&(~state->unlockedMods));
    xkb->lookupState=state->mods&(~controls->internalMods);
    xkb->grabState= xkb->lookupState&(~controls->ignoreLockMods);
    xkb->grabState|= (state->baseMods&controls->ignoreLockMods);

    _XkbAdjustGroup(&state->baseGroup,controls->numGroups,wrap);
    _XkbAdjustGroup(&state->latchedGroup,controls->numGroups,wrap);
    _XkbAdjustGroup(&state->lockedGroup,controls->numGroups,wrap);

    xkb->state.group = state->baseGroup+state->latchedGroup;
    if (!xkb->state.groupsUnlocked)
	xkb->state.group+= state->lockedGroup;
    _XkbAdjustGroup(&state->group,controls->numGroups,wrap);

    if ((controls->internalMods&0xE000)==0) {
	xkb->lookupState|= ((state->group&0x7)<<13);
	if ((controls->ignoreLockMods&0xE000)==0)
	     xkb->grabState|= ((state->group&0x7)<<13);
	else xkb->grabState|= ((state->baseGroup&0x7)<<13);
    }

    XkbComputeCompatState(xkb);
    return;
}


/***====================================================================***/

#define	SYNTHETIC_KEYCODE	1

typedef struct _XkbFilter {
	CARD8			  keycode;
	CARD8			  active;
	CARD8			  filterOthers;
	CARD32			  priv;
	XkbAction		  upAction;
	int			(*filter)();
	struct _XkbFilter	 *next;
} XkbFilter;

static int
_XkbFilterSetState(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{

    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = ((pAction->flags&XkbSAClearLocks)!=0);
	filter->priv = 0;
	filter->filter = _XkbFilterSetState;
	if (XkbActionType(*pAction)==XkbSASetMods) {
	    filter->upAction = *pAction;
	    xkb->setMods= XkbActionDataLow(*pAction);
	}
	else {
	    xkb->groupChange = XkbActionDataLow(*pAction);
	    if (pAction->flags&XkbSAGroupAbsolute)
		 xkb->groupChange-= xkb->state.baseGroup;
	    filter->upAction= *pAction;
	    XkbActionSetDataLow(filter->upAction,xkb->groupChange);
	}
    }
    else if (filter->keycode==keycode) {
	if (XkbActionType(filter->upAction)==XkbSASetMods) {
	    if (filter->upAction.flags&XkbSAClearLocks) {
		xkb->state.lockedMods&= ~XkbActionDataLow(filter->upAction);
	    }
	    xkb->clearMods = XkbActionDataLow(filter->upAction);
	}
	else {
	    if (filter->upAction.flags&XkbSAClearLocks) {
		xkb->state.lockedGroup = 0;
	    }
	    xkb->groupChange = -XkbActionDataLow(filter->upAction);
	}
	filter->active = 0;
    }
    else {
	filter->upAction.flags&= ~XkbSAClearLocks;
	filter->filterOthers = 0;
    }
    return 1;
}

#define	LATCH_KEY_DOWN	1
#define	LATCH_PENDING	2
#define	NO_LATCH	3

static int
_XkbFilterLatchState(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{

    if (filter->keycode==0) {			/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 1;
	filter->priv = LATCH_KEY_DOWN;
	filter->filter = _XkbFilterLatchState;
	if (XkbActionType(*pAction)==XkbSALatchMods) {
	    filter->upAction = *pAction;
	    xkb->setMods = XkbActionDataLow(*pAction);
	}
	else {
	    xkb->groupChange = XkbActionDataLow(*pAction);
	    if (pAction->flags&XkbSAGroupAbsolute)
		 xkb->groupChange-= xkb->state.baseGroup;
	    filter->upAction= *pAction;
	    XkbActionSetDataLow(filter->upAction,xkb->groupChange);
	}
    }
    else if ( pAction && (filter->priv==LATCH_PENDING) ) {
	if (pAction->type==XkbSANoAction) {
	    filter->active = 0;
	    if (XkbActionType(filter->upAction)==XkbSALatchMods)
		 xkb->state.latchedMods&= ~XkbActionDataLow(filter->upAction);
	    else xkb->state.latchedGroup-= XkbActionDataLow(filter->upAction);
	}
	else if ((pAction->type==filter->upAction.type)&&
		 (pAction->data==filter->upAction.data)&&
		 (pAction->flags==filter->upAction.flags)) {
	    if (filter->upAction.flags&XkbSALatchToLock) {
		if (XkbActionType(filter->upAction)==XkbSALatchMods)
		     XkbActionSetType(*pAction,XkbSALockMods);
		else XkbActionSetType(*pAction,XkbSALockGroup);
	    }
	    else {
		if (XkbActionType(filter->upAction)==XkbSALatchMods)
		     XkbActionSetType(*pAction,XkbSASetMods);
		else XkbActionSetType(*pAction,XkbSASetGroup);
	    }
	    if (XkbActionType(filter->upAction)==XkbSALatchMods)
		 xkb->state.latchedMods&= ~XkbActionDataLow(filter->upAction);
	    else xkb->state.latchedGroup-= XkbActionDataLow(filter->upAction);
	    filter->active = 0;
	}
    }
    else if (filter->keycode==keycode) {	/* release */
	if (XkbActionType(filter->upAction)==XkbSALatchMods) {
	     xkb->clearMods = XkbActionDataLow(filter->upAction);
	     if ((filter->upAction.flags&XkbSAClearLocks)&&
		 (xkb->clearMods&xkb->state.lockedMods)==xkb->clearMods) {
		xkb->state.lockedMods&= ~xkb->clearMods;
		filter->priv= NO_LATCH;
	     }
	}
	else {
	    xkb->groupChange = -XkbActionDataLow(filter->upAction);
	    if ((filter->upAction.flags&XkbSAClearLocks)&&
						(xkb->state.lockedGroup)) {
		xkb->state.lockedGroup = 0;
		filter->priv = NO_LATCH;
	    }
	}
	if (filter->priv==NO_LATCH) {
	    filter->active= 0;
	}
	else {
	    filter->priv= LATCH_PENDING;
	    if (XkbActionType(filter->upAction)==XkbSALatchMods)
		 xkb->state.latchedMods |= XkbActionDataLow(filter->upAction);
	    else xkb->state.latchedGroup+= XkbActionDataLow(filter->upAction);
	}
    }
    else if (filter->priv==LATCH_KEY_DOWN) {
	filter->priv= NO_LATCH;
	filter->filterOthers = 0;
    }
    return 1;
}

static int
_XkbFilterLockState(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{

    if (pAction&&(XkbActionType(*pAction)==XkbSALockGroup)) {
	if (pAction->flags&XkbSAGroupAbsolute)
	     xkb->state.lockedGroup= XkbActionDataLow(*pAction);
	else xkb->state.lockedGroup+= XkbActionDataLow(*pAction);
	return 1;
    }
    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv = 0;
	filter->filter = _XkbFilterLockState;
	filter->upAction = *pAction;
	xkb->state.lockedMods^= XkbActionDataLow(*pAction);
	xkb->setMods = XkbActionDataLow(*pAction);
    }
    else if (filter->keycode==keycode) {
	filter->active = 0;
	xkb->clearMods = XkbActionDataLow(filter->upAction);
    }
    return 1;
}

#define	ISO_KEY_DOWN		0
#define	NO_ISO_LOCK		1

static int
_XkbFilterISOLock(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{

    if (filter->keycode==0) {		/* initial press */
	CARD8	flags = XkbActionDataHigh(*pAction);
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 1;
	filter->priv = ISO_KEY_DOWN;
	filter->upAction = *pAction;
	filter->filter = _XkbFilterISOLock;
	if (flags&XkbSAISODfltIsGroup) {
	    xkb->groupChange = XkbActionDataLow(*pAction);
	    xkb->setMods = 0;
	}
	else {
	    xkb->setMods = XkbActionDataLow(*pAction);
	    xkb->groupChange = 0;
	}
	if ((!(flags&XkbSAISONoAffectMods))&&(xkb->state.baseMods)) {
	    filter->priv= NO_ISO_LOCK;
	    xkb->state.lockedMods^= xkb->state.baseMods;
	}
	if ((!(flags&XkbSAISONoAffectGroup))&&(xkb->state.baseGroup)) {
/* 6/22/93 (ef) -- lock groups if group key is down first */
	}
	if (!(flags&XkbSAISONoAffectPtr)) {
/* 6/22/93 (ef) -- lock mouse buttons if they're down */
	}
    }
    else if (filter->keycode==keycode) {
	CARD8	flags = XkbActionDataHigh(filter->upAction);
	if (flags&XkbSAISODfltIsGroup) {
	    xkb->groupChange = -XkbActionDataHigh(filter->upAction);
	    xkb->clearMods = 0;
	    if (filter->priv==ISO_KEY_DOWN)
		xkb->state.lockedGroup+= XkbActionDataHigh(filter->upAction);
	}
	else {
	    xkb->clearMods= XkbActionDataLow(filter->upAction);
	    xkb->groupChange= 0;
	    if (filter->priv==ISO_KEY_DOWN)
		xkb->state.lockedMods^= XkbActionDataLow(filter->upAction);
	}
	filter->active = 0;
    }
    else if (pAction) {
	CARD8	flags = XkbActionDataHigh(filter->upAction);
	switch (XkbActionType(*pAction)) {
	    case XkbSASetMods: case XkbSALatchMods:
		if (!(flags&XkbSAISONoAffectMods)) {
		    XkbActionSetType(*pAction,XkbSALockMods);
		    filter->priv= NO_ISO_LOCK;
		}
		break;
	    case XkbSASetGroup: case XkbSALatchGroup:
		if (!(flags&XkbSAISONoAffectGroup)) {
		    XkbActionSetType(*pAction,XkbSALockGroup);
		    filter->priv= NO_ISO_LOCK;
		}
		break;
	    case XkbSAPtrBtn:
		if (!(flags&XkbSAISONoAffectPtr)) {
		     XkbActionSetType(*pAction,XkbSALockPtrBtn);
		     filter->priv= NO_ISO_LOCK;
		}
		break;
	    case XkbSASetControls:
		if (!(flags&XkbSAISONoAffectCtrls)) {
		    XkbActionSetType(*pAction,XkbSALockControls);
		    filter->priv= NO_ISO_LOCK;
		}
		break;
	}
    }
    return 1;
}


static CARD32
_XkbPtrAccelExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
XkbSrvInfoRec	*xkbInfo= (XkbSrvInfoRec *)arg;
XkbControlsRec	*ctrls= xkbInfo->desc.controls;
int	dx,dy;

    if (xkbInfo->mouseKey==0)
	return 0;

    if (xkbInfo->mouseKeysAccel) {
	if ((xkbInfo->mouseKeysCounter)<ctrls->mouseKeysTimeToMax) {
	    double step;
	    xkbInfo->mouseKeysCounter++;
	    step= xkbInfo->mouseKeysCurveFactor*
		 pow((double)xkbInfo->mouseKeysCounter,xkbInfo->mouseKeysCurve);
	    dx= ceil( ((double)xkbInfo->mouseKeysDX)*step );
	    dy= ceil( ((double)xkbInfo->mouseKeysDY)*step );
	}
	else {
	    dx= xkbInfo->mouseKeysDX*ctrls->mouseKeysMaxSpeed;
	    dy= xkbInfo->mouseKeysDY*ctrls->mouseKeysMaxSpeed;
	}
    }
    else {
	dx= xkbInfo->mouseKeysDX;
	dy= xkbInfo->mouseKeysDY;
    }
    DDXFakePointerMotion(dx,dy);
    return xkbInfo->desc.controls->mouseKeysInterval;
}

static int
_XkbFilterPointerMove(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{
    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv=0;
	filter->filter = _XkbFilterPointerMove;
	filter->upAction= *pAction;
	xkb->mouseKeysCounter= 0;
	xkb->mouseKey= keycode;
	DDXFakePointerMotion((INT8)XkbActionDataHigh(*pAction),
					  (INT8)XkbActionDataLow(*pAction));
	xkb->mouseKeysAccel= 
		((XkbActionType(*pAction)==XkbSAAccelPtr)||
		((XkbActionType(*pAction)==XkbSAMovePtrBtn)&&
		 (xkb->desc.controls->enabledControls&XkbMouseKeysAccelMask)));
	xkb->mouseKeysDX= (INT8)XkbActionDataHigh(*pAction);
	xkb->mouseKeysDY= (INT8)XkbActionDataLow(*pAction);
	xkb->mouseKeyTimer= TimerSet(xkb->mouseKeyTimer, 0,
				xkb->desc.controls->mouseKeysDelay,
				_XkbPtrAccelExpire,(pointer)xkb);
    }
    else if (filter->keycode==keycode) {
	filter->active = 0;
	if (xkb->mouseKey==keycode) {
	    xkb->mouseKey= 0;
	    xkb->mouseKeyTimer= TimerSet(xkb->mouseKeyTimer, 0, 0,
							NULL, NULL);
	}
    }
    return 0;
}

static int
_XkbFilterPointerBtn(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{
    if (filter->keycode==0) {		/* initial press */
	int button = XkbActionDataLow(*pAction);
	if (button==XkbSAUseDfltButton)
	    button = xkb->desc.controls->mouseKeysDfltBtn;

	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv=0;
	filter->filter = _XkbFilterPointerBtn;
	filter->upAction= *pAction;
	XkbActionSetDataLow(filter->upAction,button);
	switch (XkbActionType(*pAction)) {
	    case XkbSALockPtrBtn:
		if ((xkb->lockedPtrButtons&(1<<button))==0) {
		    xkb->lockedPtrButtons|= (1<<button);
		    DDXFakePointerButton(ButtonPress,button);
		    filter->upAction.type= XkbSANoAction;
		}
		break;
	    case XkbSAPtrBtn:
		DDXFakePointerButton(ButtonPress,button);
		break;
	    case XkbSAClickPtrBtn:
		{
		    register int i,nClicks;
		    nClicks= (INT8)XkbActionDataHigh(*pAction);
		    for (i=0;i<nClicks;i++) {
			DDXFakePointerButton(ButtonPress,button);
			DDXFakePointerButton(ButtonRelease,button);
		    }
		    filter->upAction.type= XkbSANoAction;
		}
		break;
	    case XkbSASetPtrDflt:
		{
		    XkbControlsRec	*ctrls= xkb->desc.controls;
		    XkbControlsRec	old= *ctrls;
		    xkbControlsNotify	cn;
		    switch (XkbActionDataHigh(*pAction)) {
			case XkbSASetDfltBtn:
			    ctrls->mouseKeysDfltBtn= XkbActionDataLow(*pAction);
			    break;
			case XkbSAIncrDfltBtn:
			    ctrls->mouseKeysDfltBtn+=XkbActionDataLow(*pAction);
			    if (ctrls->mouseKeysDfltBtn>5)
				ctrls->mouseKeysDfltBtn= 5;
			    else if (ctrls->mouseKeysDfltBtn<1)
				ctrls->mouseKeysDfltBtn= 1;
			    break;
		    }
		    if (XkbComputeControlsNotify(xkb->device,
						&old,xkb->desc.controls,
						&cn)) {
			cn.keycode = keycode;
			cn.eventType = KeyPress;
			cn.requestMajor = 0;
			cn.requestMinor = 0;
			XkbSendControlsNotify(xkb->device,&cn);
		    }
		}
		break;
	}
    }
    else if (filter->keycode==keycode) {
	int button= (INT16)XkbActionDataLow(filter->upAction);
	switch (XkbActionType(filter->upAction)) {
	    case XkbSALockPtrBtn:
		xkb->lockedPtrButtons&= ~(1<<button);
	    case XkbSAPtrBtn:
		DDXFakePointerButton(ButtonRelease,button);
		break;
	}
	filter->active = 0;
    }
    return 0;
}

static int
_XkbFilterControls(xkb,filter,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    XkbFilter		*filter;
    CARD8		 keycode;
    XkbAction		*pAction;
{
XkbControlsRec	old= *xkb->desc.controls;
CARD16		change;

    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv = change= XkbActionData(*pAction);
	filter->filter = _XkbFilterControls;
	filter->upAction = *pAction;

	if (XkbActionType(*pAction)==XkbSALockControls) {
	    filter->priv= (xkb->desc.controls->enabledControls&change);
	    change&= ~xkb->desc.controls->enabledControls;
	}

	if (change) {
	    xkbControlsNotify cn;
	    xkb->desc.controls->enabledControls|= change;
	    if (XkbComputeControlsNotify(xkb->device,&old,xkb->desc.controls,
									&cn)) {
		cn.keycode = keycode;
		cn.eventType = KeyPress;
		cn.requestMajor = 0;
		cn.requestMinor = 0;
		XkbSendControlsNotify(xkb->device,&cn);
	    }
	    if (xkb->iAccel.usesControls)
		XkbUpdateIndicators(xkb->device,xkb->iAccel.usesControls,NULL);
	}
    }
    else if (filter->keycode==keycode) {
	change= filter->priv;
	if (change) {
	    xkbControlsNotify cn;
	    xkb->desc.controls->enabledControls&= ~change;
	    if (XkbComputeControlsNotify(xkb->device,&old,xkb->desc.controls,
									&cn)) {
		cn.keycode = keycode;
		cn.eventType = KeyRelease;
		cn.requestMajor = 0;
		cn.requestMinor = 0;
		XkbSendControlsNotify(xkb->device,&cn);
	    }
	    if (xkb->iAccel.usesControls)
		XkbUpdateIndicators(xkb->device,xkb->iAccel.usesControls,NULL);
	}
	filter->keycode= 0;
	filter->active= 0;
    }
    return 0;
}

static	int		 szFilters = 0;
static	XkbFilter	*filters = NULL;

static XkbFilter *
_XkbNextFreeFilter()
{
register int i;

    if (szFilters==0) {
	szFilters = 4;
	filters = (XkbFilter *)Xcalloc(szFilters*sizeof(XkbFilter));
	/* 6/21/93 (ef) -- XXX! deal with allocation failure */
    }
    for (i=0;i<szFilters;i++) {
	if (!filters[i].active) {
	    filters[i].keycode = 0;
	    return &filters[i];
	}
    }
    szFilters*=2;
    filters= (XkbFilter *)Xrealloc(filters,szFilters*sizeof(XkbFilter));
    /* 6/21/93 (ef) -- XXX! deal with allocation failure */
    bzero(&filters[szFilters/2],(szFilters/2)*sizeof(XkbFilter));
    return &filters[szFilters/2];
}

static int
_XkbApplyFilters(xkb,keycode,pAction)
    XkbSrvInfoRec	*xkb;
    CARD8		 keycode;
    XkbAction		*pAction;
{
register int i,send;

    send= 1;
    for (i=0;i<szFilters;i++) {
	if ((filters[i].active)&&(filters[i].filter))
	    send= ((*filters[i].filter)(xkb,&filters[i],keycode,pAction)&&send);
    }
    return send;
}

void
XkbHandleActions(xE,keybd,count)
    xEvent	 *xE;
    DeviceIntPtr  keybd;
    int		  count;
{
int		key,bit,i;
register CARD8	realMods;
XkbSrvInfoRec	*xkb;
KeyClassRec	*keyc = keybd->key;
int		 changed,sendEvent;
CARD16		 data;
XkbStateRec	 oldState;
XkbAction	 action;
XkbFilter	*filter;

    key= xE->u.u.detail;
    realMods = keyc->modifierMap[key];
    keyc->modifierMap[key]= 0;
    xkb= keyc->xkbInfo;
    oldState= xkb->state;

    xkb->clearMods = xkb->setMods = 0;
    xkb->groupChange = 0;

    sendEvent = 1;

    if ( xE->u.u.type == KeyPress ) {
	action = XkbKeyAction(&xkb->desc,&xkb->state,key);
	sendEvent = _XkbApplyFilters(xkb,key,&action);
	if (sendEvent) {
	    data = XkbActionData(action);

	    switch (XkbActionType(action)) {
		case XkbSASetMods:
		case XkbSASetGroup:
		    filter = _XkbNextFreeFilter();
		    sendEvent = _XkbFilterSetState(xkb,filter,key,&action);
		    break;
		case XkbSALatchMods:
		case XkbSALatchGroup:
		    filter = _XkbNextFreeFilter();
		    sendEvent=_XkbFilterLatchState(xkb,filter,key,&action);
		    break;
		case XkbSALockMods:
		case XkbSALockGroup:
		    filter = _XkbNextFreeFilter();
		    sendEvent=_XkbFilterLockState(xkb,filter,key,&action);
		    break;
		case XkbSAISOLock:
		    filter = _XkbNextFreeFilter();
		    sendEvent=_XkbFilterISOLock(xkb,filter,key,&action);
		    break;
		case XkbSAAccelPtr:
		case XkbSAMovePtrBtn:
		    filter = _XkbNextFreeFilter();
		    sendEvent= _XkbFilterPointerMove(xkb,filter,key,&action);
		    break;
		case XkbSAPtrBtn:
		case XkbSAClickPtrBtn:
		case XkbSALockPtrBtn:
		case XkbSASetPtrDflt:
		    filter = _XkbNextFreeFilter();
		    sendEvent= _XkbFilterPointerBtn(xkb,filter,key,&action);
		    break;
		case XkbSATerminate:
		    sendEvent= DDXTerminateServer(keybd,key,&action);
		    break;
		case XkbSASwitchScreen:
		    sendEvent= DDXSwitchScreen(keybd,key,&action);
		    break;
		case XkbSASetControls:
		case XkbSALockControls:
		    filter = _XkbNextFreeFilter();
		    sendEvent=_XkbFilterControls(xkb,filter,key,&action);
		    break;
	    }
	}
    }
    else {
	sendEvent = _XkbApplyFilters(xkb,key,NULL);
    }

    xkb->state.baseGroup+= xkb->groupChange;
    if (xkb->setMods) {
	for (i=0,bit=1; xkb->setMods; i++,bit<<=1 ) {
	    if (xkb->setMods&bit) {
		keyc->modifierKeyCount[i]++;
		xkb->state.baseMods|= bit;
		xkb->setMods&= ~bit;
	    }
	}
    }
    if (xkb->clearMods) {
	for (i=0,bit=1; xkb->clearMods; i++,bit<<=1 ) {
	    if (xkb->clearMods&bit) {
		keyc->modifierKeyCount[i]--;
		if (keyc->modifierKeyCount[i]<=0) {
		    xkb->state.baseMods&= ~bit;
		    keyc->modifierKeyCount[i] = 0;
		}
		xkb->clearMods&= ~bit;
	    }
	}
    }

    keyc->modifierMap[key] = 0;
    if (sendEvent)
	CoreProcessKeyboardEvent(xE,keybd,count);
    keyc->modifierMap[key] = realMods;
    XkbComputeDerivedState(xkb);
    keyc->state= xkb->lookupState&0xE0FF;

    changed = XkbStateChangedFlags(&oldState,&xkb->state);
    if (changed) {
	xkbStateNotify	sn;
	sn.keycode= key;
	sn.eventType= xE->u.u.type;
	sn.requestMajor = sn.requestMinor = 0;
	sn.changed= changed;
	XkbSendStateNotify(keybd,&sn);
    }
    if (changed&xkb->iAccel.usedComponents) {
	changed= XkbIndicatorsToUpdate(keybd,changed);
	if (changed)
	    XkbUpdateIndicators(keybd,changed,NULL);
    }

    return;
}

void
XkbProcessKeyboardEvent(xE,keybd,count)
    xEvent *xE;
    DeviceIntPtr keybd;
    int count;
{
KeyClassRec	*keyc = keybd->key;
XkbSrvInfoRec	*xkb;
int		 key;
XkbAction	 action;

    xkb= keyc->xkbInfo;
    key= xE->u.u.detail;

    action= xkb->desc.server->keyBehaviors[key];
    switch (XkbActionType(action)) {
	case XkbDefaultKB:
	    if (( xE->u.u.type == KeyPress ) && 
		(keyc->down[key>>3] & (1<<(key&7)))) {
		xE->u.u.type = KeyRelease;
		XkbHandleActions(xE,keybd,count);
		xE->u.u.type = KeyPress;
		XkbHandleActions(xE,keybd,count);
		return;
	    }
	    else if ((xE->u.u.type==KeyRelease) &&
			(!(keyc->down[key>>3]&(1<<(key&7))))) {
		xE->u.u.type = KeyPress;
		XkbHandleActions(xE,keybd,count);
		xE->u.u.type = KeyRelease;
		XkbHandleActions(xE,keybd,count);
		return;
	    }
	    break;
	case XkbLockKB:
	    if ( xE->u.u.type == KeyRelease )
		return;
	    else {
		int	bit= 1<<(key&7);
		if ( keyc->down[key>>3]&bit )
		    xE->u.u.type= KeyRelease;
	    }
	    break;
	case XkbRadioGroupKB:
	    if ( xE->u.u.type == KeyRelease )
		return;
	    else {
		int		ndx = XkbActionDataLow(action);
		if ( ndx<xkb->nRadioGroups ) {
		    XkbRadioGroupRec	*rg;
		    register int i;
		    rg = &xkb->radioGroups[ndx];
		    if ( rg->currentDown == xE->u.u.detail ) {
			xE->u.u.type = KeyRelease;
			XkbHandleActions(xE,keybd,count);
			rg->currentDown= 0;
			return;
		    }
		    if ( rg->currentDown!=0 ) {
			int key = xE->u.u.detail;
			xE->u.u.type= KeyRelease;
			xE->u.u.detail= rg->currentDown;
			XkbHandleActions(xE,keybd,count);
			xE->u.u.type= KeyPress;
			xE->u.u.detail= key;
		    }
		    rg->currentDown= key;
		}
		else ErrorF("InternalError! Illegal radio group %d\n",ndx);
	    }
	    break;
	default:
	    ErrorF("unknown key behavior 0x%04x\n",action);
	    break;
    }
    XkbHandleActions(xE,keybd,count);
    return;
}

void
ProcessKeyboardEvent(xE,keybd,count)
    xEvent *xE;
    DeviceIntPtr keybd;
    int	count;
{
KeyClassRec	*keyc = keybd->key;
XkbSrvInfoRec	*xkb;
int		 key;
XkbAction	 action;

    xkb= keyc->xkbInfo;
    key= xE->u.u.detail;

    if (!(xkb->desc.controls->enabledControls&ALL_FILTERED_MASK))
	XkbProcessKeyboardEvent(xE,keybd,count);
    else if (xE->u.u.type==KeyPress)
	AccessXFilterPressEvent(xE,keybd,count);
    else if (xE->u.u.type==KeyRelease)
	AccessXFilterReleaseEvent(xE,keybd,count);
    return;
}

/***====================================================================***/

int
XkbLatchModifiers(pXDev,mask,latches)
    DeviceIntPtr pXDev;
    CARD8 mask;
    CARD8 latches;
{
XkbSrvInfoRec	*xkb;
XkbFilter	*filter;
XkbAction	 action;
unsigned	 clear;

    if ( pXDev && pXDev->key && pXDev->key->xkbInfo ) {
	xkb = pXDev->key->xkbInfo;
	clear= (mask&(~latches));
	xkb->state.latchedMods&= ~clear;
	action.type = XkbSALatchMods;
	action.flags = 0;
	action.data = mask&latches;
	filter = _XkbNextFreeFilter();
	_XkbFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,&action);
	_XkbFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,NULL);
	return Success;
    }
    return BadValue;
}

int
XkbLatchGroup(pXDev,group)
    DeviceIntPtr  pXDev;
    int		  group;
{
XkbSrvInfoRec	*xkb;
XkbFilter	*filter;
XkbAction	 action;

    if ( pXDev && pXDev->key && pXDev->key->xkbInfo ) {
	xkb = pXDev->key->xkbInfo;
	action.type = XkbSALatchGroup;
	action.flags = 0;
	action.data = group;
	filter = _XkbNextFreeFilter();
	_XkbFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,&action);
	_XkbFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,NULL);
	return Success;
    }
    return BadValue;
}

