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

static	int	XKBComputeAutoRepeat = 1;
#define	CREATE_ATOM(s)	MakeAtom(s,sizeof(s)-1,1)

#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

/***====================================================================***/

XKBAction *
_XKBNewActionsForKey(xkb,key,needed)
    XKBDescRec *xkb;
    int key;
    int needed;
{
register int i,nActs;
XKBAction	*newActs;

    if ((xkb->server->keyActions[key]!=0)&&(XKBKeyNumSyms(xkb,key)>=needed)) {
	return XKBKeyActionsPtr(xkb,key);
    }
    if (xkb->server->szActions-xkb->server->nActions>=needed) {
	xkb->server->keyActions[key]= xkb->server->nActions;
	xkb->server->nActions+= needed;
	return &xkb->server->actions[xkb->server->keyActions[key]];
    }
    xkb->server->szActions+= 32;
    newActs = (XKBAction *)Xcalloc(xkb->server->szActions*sizeof(XKBAction));
    newActs[0].type = XKB_SA_NO_ACTION;
    nActs = 1;
    for (i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	if (xkb->server->keyActions[i]!=0) {
	    memcpy(&newActs[nActs],XKBKeyActionsPtr(xkb,i),
				XKBKeyNumActions(xkb,i)*sizeof(XKBAction));
	    xkb->server->keyActions[i]= nActs;
	    if (i!=key)	nActs+= XKBKeyNumActions(xkb,i);
	    else	nActs+= needed;
	}
    }
    free(xkb->server->actions);
    xkb->server->actions = newActs;
    xkb->server->nActions = nActs;
    return &xkb->server->actions[xkb->server->keyActions[key]];
}

KeySym *
_XKBNewSymsForKey(xkb,key,needed)
    XKBDescRec *xkb;
    int key;
    int needed;
{
register int i,nSyms;
KeySym	*newSyms;

    if (XKBKeyNumSyms(xkb,key)>=needed) {
	return XKBKeySymsPtr(xkb,key);
    }
    if (xkb->server->keyActions[key]!=0)
	_XKBNewActionsForKey(xkb,key,needed);

    if (xkb->map->szSyms-xkb->map->nSyms>=needed) {
	xkb->map->keySymMap[key].offset = xkb->map->nSyms;
	xkb->map->nSyms+= needed;
	return &xkb->map->syms[xkb->map->keySymMap[key].offset];
    }
    xkb->map->szSyms+= 128;
    newSyms = (KeySym *)Xcalloc(xkb->map->szSyms*sizeof(KeySym));
    nSyms = 1;
    for (i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	memcpy(&newSyms[nSyms],XKBKeySymsPtr(xkb,i),
					XKBKeyNumSyms(xkb,i)*sizeof(KeySym));
	xkb->map->keySymMap[i].offset = nSyms;
	if (i!=key)	nSyms+= XKBKeyNumSyms(xkb,i);
	else		nSyms+= needed;
    }
    free(xkb->map->syms);
    xkb->map->syms = newSyms;
    xkb->map->nSyms = nSyms;
    return &xkb->map->syms[xkb->map->keySymMap[key].offset];
}

void
_XKBResizeKeyType(xkb,ndx,nLevels)
    XKBDescRec	*xkb;
    int		 ndx;
    int		 nLevels;
{
XKBKeyTypeRec	*type= &xkb->map->keyTypes[ndx];
int 		 nTotal;
KeySym		*newSyms;
register int	 i,nSyms;

    if (type->groupWidth==nLevels)
	return;
    for (nTotal=0,i=xkb->minKeyCode;i<=xkb->maxKeyCode;i++) {
	if (xkb->map->keySymMap[i].ktIndex==ndx) 
	     nTotal+= XKBKeyNumGroups(xkb,i)*nLevels;
	else nTotal+= XKBKeyNumSyms(xkb,i);
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
	    nGroups= XKBKeyNumGroups(xkb,i);
	    for (g=0;g<nGroups;g++) {
		KeySym *pOld;
		pOld= XKBKeySymsPtr(xkb,i)+(g*XKBKeyGroupWidth(xkb,i));
		memcpy(&newSyms[nSyms+(nLevels*g)],pOld,nLevels*sizeof(KeySym));
	    }
	    xkb->map->keySymMap[i].offset = nSyms;
	    nSyms+= nGroups*nLevels;
	}
	else {
	    memcpy(&newSyms[nSyms],XKBKeySymsPtr(xkb,i),
					XKBKeyNumSyms(xkb,i)*sizeof(KeySym));
	    xkb->map->keySymMap[i].offset = nSyms;
	    nSyms+= XKBKeyNumSyms(xkb,i);
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
XKBInitKeyTypes(xkb)
    XKBSrvInfoRec *xkb;
{
XKBKeyTypeRec	*kt;
XKBDescRec	*desc = &xkb->desc;

    if (desc->map->nKeyTypes>0)
	return;

    desc->map->keyTypes=(XKBKeyTypeRec*)Xcalloc(KT_COUNT*sizeof(XKBKeyTypeRec));
    if (!desc->map->keyTypes) {
	desc->map->nKeyTypes = 0;
	return;
    }

    desc->map->nKeyTypes = KT_COUNT;
    kt = &desc->map->keyTypes[KT_1LEVEL_KEY];
    kt->mask = 0;
    kt->groupWidth = 1;
    kt->map = (CARD8 *)Xcalloc(XKBKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->preserve = NULL;
    kt->flags = 0;

    kt = &desc->map->keyTypes[KT_2LEVEL_KEY];
    kt->mask = ShiftMask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XKBKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ShiftMask] = 1;
    kt->preserve = NULL;
    kt->flags = XKB_KT_DONT_FREE_STRUCT;

    kt = &desc->map->keyTypes[KT_KEYPAD_KEY];
    kt->mask = ShiftMask|Mod4Mask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XKBKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ShiftMask|Mod4Mask] = 0;	/* shift negates num lock */
    kt->map[ShiftMask] = 1;
    kt->map[Mod4Mask] = 1;
    kt->preserve = NULL;
    kt->flags = XKB_KT_DONT_FREE_STRUCT;

    kt = &desc->map->keyTypes[KT_3LEVEL_KEY];
    kt->mask = ShiftMask|Mod3Mask;
    kt->groupWidth = 3;
    kt->map = (CARD8 *)Xcalloc(XKBKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ShiftMask] = 1;
    kt->map[Mod3Mask] = 2;
    kt->map[ShiftMask|Mod3Mask] = 2;
    kt->preserve = NULL;
    kt->flags = XKB_KT_DONT_FREE_STRUCT;

    kt = &desc->map->keyTypes[KT_PC_BREAK_KEY];
    kt->mask = ControlMask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XKBKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[ControlMask] = 1;
    kt->preserve = NULL;
    kt->flags = XKB_KT_DONT_FREE_STRUCT;

    kt = &desc->map->keyTypes[KT_PC_SYSRQ_KEY];
    kt->mask = Mod1Mask;
    kt->groupWidth = 2;
    kt->map = (CARD8 *)Xcalloc(XKBKTMapWidth(kt)*sizeof(CARD8));
    kt->map[0] = 0;
    kt->map[Mod1Mask] = 1;
    kt->preserve = NULL;
    kt->flags = XKB_KT_DONT_FREE_STRUCT;
    return;
}

static void
XKBInitKeySyms(xkb)
    XKBSrvInfoRec *xkb;
{
XKBClientMapRec	*map= xkb->desc.map;
int	nKeys= xkb->desc.maxKeyCode-xkb->desc.minKeyCode+1;

    map->nSyms= 1;
    map->szSyms= (nKeys*15)/10;
    map->syms=(KeySym *)Xcalloc(map->szSyms*sizeof(KeySym));
    if (map->syms) {
	register int i;
	XKBSymMapRec *symMap= &map->keySymMap[xkb->desc.minKeyCode];
	for (i=0;i<nKeys;symMap++,i++) {
	    symMap->ktIndex = KT_1LEVEL_KEY;
	    symMap->groupInfo = XKBSetGroupInfo(1,FALSE);
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
XKBInitKeyActions(xkb)
    XKBSrvInfoRec *xkb;
{
XKBServerMapRec	*map= xkb->desc.server;
int	nKeys= xkb->desc.maxKeyCode-xkb->desc.minKeyCode+1;

    map->nActions= 1;
    map->szActions= (nKeys/2)+1;
    map->actions=(XKBAction *)Xcalloc(map->szActions*sizeof(XKBAction));
    if (map->actions) {
	register int i;
	map->actions[0].type = XKB_SA_NO_ACTION;
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
XKBInitRadioGroups(xkb)
    XKBSrvInfoRec *xkb;
{
XKBRadioGroupRec	*grp;
 
#ifndef DEBUG_RADIO_GROUPS
    xkb->nRadioGroups = 0;
    xkb->radioGroups = NULL;
    return;
#else
    if ( xkb->nRadioGroups>0 )
	return;

    xkb->radioGroups=(XKBRadioGroupRec *)Xcalloc(RG_COUNT*
						sizeof(XKBRadioGroupRec));
    if (!xkb->radioGroups)
	return;
    xkb->nRadioGroups = RG_COUNT;

    grp = &xkb->radioGroups[RG_BOGUS_FUNCTION_GROUP];
    grp->flags = 0;
    grp->nMembers = 0;
    grp->dfltDown = 0;
#endif
    return;
}

#define	SI_LF	(XKB_SA_CLEAR_LOCKS|XKB_SA_LATCH_TO_LOCK)
static	XKBSymInterpretRec	dfltSI[] = {
    { XK_Caps_Lock,    0, XKB_SI_Exactly, LockMask, 4,
					{ XKB_SA_LOCK_MODS, SI_LF, LockMask },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Shift_Lock,   0, XKB_SI_AnyOf,   0xff, XKB_SI_NoIndicator,
					{ XKB_SA_LOCK_MODS, SI_LF, ShiftMask },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_ISO_Lock,     0, XKB_SI_Exactly, LockMask, 4,
					{ XKB_SA_ISO_LOCK, SI_LF, LockMask },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_ISO_Level2_Latch, 0, XKB_SI_Exactly, ShiftMask, XKB_SI_NoIndicator,
					{ XKB_SA_LATCH_MODS, SI_LF, ShiftMask },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_ISO_Level3_Latch, XKB_SI_UseModMapMods, XKB_SI_AnyOf, 0xff, 
					XKB_SI_NoIndicator,
					{ XKB_SA_LATCH_MODS, SI_LF, 0 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Num_Lock, XKB_SI_UseModMapMods|XKB_SI_UpdateKeypad, 
					XKB_SI_AnyOf, 0xff, 5,
					{ XKB_SA_LOCK_MODS, SI_LF, 0 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Scroll_Lock, XKB_SI_UseModMapMods, XKB_SI_AnyOf, 0xff, 6, 
					{ XKB_SA_LOCK_MODS, SI_LF, 0 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Mode_switch, XKB_SI_UpdateGroup, XKB_SI_AnyOf, 0xff,XKB_SI_NoIndicator,
					{ XKB_SA_SET_GROUP, SI_LF, 1 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_ISO_Group_Latch, XKB_SI_UpdateGroup, XKB_SI_AnyOfOrNone, 0xff,
					XKB_SI_NoIndicator,
					{ XKB_SA_LATCH_GROUP, SI_LF, 1 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_ISO_Next_Group, XKB_SI_UpdateGroup, XKB_SI_AnyOfOrNone, 0xff,
					XKB_SI_NoIndicator,
					{ XKB_SA_LOCK_GROUP, SI_LF, 1 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_ISO_Prev_Group, XKB_SI_UpdateGroup, XKB_SI_AnyOfOrNone, 0xff, 
					XKB_SI_NoIndicator,
					{ XKB_SA_LOCK_GROUP, SI_LF, (CARD8)-1 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_KP_Left, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
					{ XKB_SA_MOVE_PTR, (CARD8)-1, 0 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_KP_Right, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
					{ XKB_SA_MOVE_PTR, 1, 0 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_KP_Up, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
					{ XKB_SA_MOVE_PTR, 0, (CARD8)-1 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_KP_Down, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
					{ XKB_SA_MOVE_PTR, 0, 1 },
					{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_KP_Begin, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, XKB_SA_USE_DFLT_BUTTON },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Button_Dflt, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, XKB_SA_USE_DFLT_BUTTON },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Button1, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, 1 },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Button2, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, 2 },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Button3, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, 3 },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Button4, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, 4 },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Button5, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_PTR_BTN, 1, 5 },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_EnableKeys, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_LOCK_CONTROLS, 0, XKBMouseKeysMask },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { XK_Pointer_Accelerate, 0, XKB_SI_AnyOfOrNone, 0xff, XKB_SI_NoIndicator,
			{ XKB_SA_LOCK_CONTROLS, 0, XKBMouseKeysAccelMask },
			{ XKB_KB_DEFAULT, 0, 0 } },
    { NoSymbol, XKB_SI_UseModMapMods, XKB_SI_AnyOf, 0xff, XKB_SI_NoIndicator,
					{ XKB_SA_SET_MODS, SI_LF, 0 },
					{ XKB_KB_DEFAULT, 0, 0 } }
};

static void
XKBInitCompatStructs(xkb)
    XKBSrvInfoRec *xkb;
{
register int i;
XKBCompatRec	*compat;

    compat = xkb->desc.compat;
    for (i=0;i<8;i++) {
	compat->modCompat[i].mods = (1<<i);
	compat->modCompat[i].groups = 0;
    }

    compat->nSymInterpret= 0;
    compat->symInterpret = (XKBSymInterpretRec *)Xcalloc(sizeof(dfltSI));
    if (compat->symInterpret) {
	compat->nSymInterpret = sizeof(dfltSI)/sizeof(XKBSymInterpretRec);
	memcpy(compat->symInterpret,dfltSI,sizeof(dfltSI));
    }
    return;
}

/***====================================================================***/

static void
XKBInitNames(xkb)
    XKBSrvInfoRec *xkb;
{
XKBNamesRec	*names;

    names = xkb->desc.names;
    bzero(names,sizeof(XKBNamesRec));
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
XKBInitAlternateSyms(xkb)
    XKBSrvInfoRec *xkb;
{
XKBAlternateSymsRec *alt;

   alt= (XKBAlternateSymsRec *)Xcalloc(sizeof(XKBAlternateSymsRec));
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
	alt->maps= (XKBSymMapRec *)Xcalloc(alt->nKeys*sizeof(XKBSymMapRec));
	bzero(alt->maps,alt->nKeys*sizeof(XKBSymMapRec));
	alt->maps[0].ktIndex= KT_2LEVEL_KEY;
	alt->maps[0].groupInfo= XKBSetGroupInfo(1,0);
	alt->maps[0].offset= 0;
	alt->maps[12].ktIndex= KT_2LEVEL_KEY;
	alt->maps[12].groupInfo= XKBSetGroupInfo(1,0);
	alt->maps[12].offset= 2;
	alt->next = NULL;
   }
   xkb->desc.altSymSets= alt;
   return;
}

unsigned
XKBIndicatorsToUpdate(keybd,modsChanged)
    DeviceIntRec		*keybd;
    unsigned long	 	 modsChanged;
{
register unsigned update = 0;
XKBSrvInfoRec	*xkb = keybd->key->xkbInfo;

    if (modsChanged&(XKBModifierStateMask|XKBGroupStateMask))
	update|= xkb->iAccel.usesEffective;
    if (modsChanged&(XKBModifierBaseMask|XKBGroupBaseMask))
	update|= xkb->iAccel.usesBase;
    if (modsChanged&(XKBModifierLatchMask|XKBGroupLatchMask))
	update|= xkb->iAccel.usesLatched;
    if (modsChanged&(XKBModifierLockMask|XKBGroupLockMask))
	update|= xkb->iAccel.usesLocked;
    if (modsChanged&XKBCompatStateMask)
	update|= xkb->iAccel.usesCompat;
    return update;
}
    
void
XKBUpdateIndicators(keybd,update,pChanges)
    DeviceIntRec		*keybd;
    register CARD32		 update;
    XKBIndicatorChangesRec	*pChanges;
{
register int	i,bit;
XKBSrvInfoRec	*xkb = keybd->key->xkbInfo;
XKBIndicatorRec	*map = xkb->desc.indicators;
XKBStateRec	*kbdState = &xkb->state;
CARD32		 oldState;


    oldState= xkb->iState;
    for (i=0,bit=1;update;i++,bit<<=1) {
	if (update&bit) {
	    int on;
	    CARD8 mods,group;
	    XKBIndicatorMapRec *map= &xkb->desc.indicators->maps[i];
	    on= mods= group= 0;
	    if (map->whichMods&XKB_IMUseBase)
		mods|= kbdState->baseMods;
	    if (map->whichGroups&XKB_IMUseBase)
		group|= (1L << kbdState->baseGroup);

	    if (map->whichMods&XKB_IMUseLatched)
		mods|= kbdState->latchedMods;
	    if (map->whichGroups&XKB_IMUseLatched)
		group|= (1L << kbdState->latchedGroup);

	    if (map->whichMods&XKB_IMUseLocked)
		mods|= kbdState->lockedMods;
	    if (map->whichGroups&XKB_IMUseLocked)
		group|= (1L << kbdState->lockedGroup);

	    if (map->whichMods&XKB_IMUseEffectiveLocked)
		mods|= kbdState->lockedMods&(~kbdState->unlockedMods);
	    if (map->whichGroups&XKB_IMUseEffectiveLocked)
		group= (1L<<(kbdState->groupsUnlocked?0:kbdState->lockedGroup));

	    if (map->whichMods&XKB_IMUseEffective)
		mods|= kbdState->mods;
	    if (map->whichGroups&XKB_IMUseEffective)
		group|= (1L << kbdState->group);

	    if (map->whichMods&XKB_IMUseCompat)
		mods|= kbdState->compatState;

	    if ((map->whichMods|map->whichGroups)&XKB_IMUseAnyState) {
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
	DDXUpdateIndicators((DevicePtr)keybd,oldState,xkb->iState);
    }
    if (pChanges)
	pChanges->stateChanges|= (xkb->iState^oldState);
    else if ((xkb->iState^oldState)!=0) {
	xKBIndicatorNotify	in;
	in.stateChanged= (xkb->iState^oldState);
	in.state = xkb->iState;
	in.mapChanged = 0;
	XKBSendIndicatorNotify(keybd,&in);
    }
    return;
}


#if NeedFunctionPrototypes
void
XKBUpdateKeyTypesFromCore(	DeviceIntRec	*pXDev,
				KeyCode	 	first,
				CARD8	 	num,
				xKBMapNotify	*pMN)
#else
void
XKBUpdateKeyTypesFromCore(pXDev,first,num,pMN)
    DeviceIntPtr pXDev;
    KeyCode first;
    CARD8 num;
    xKBMapNotify *pMN;
#endif
{
XKBDescRec *xkb;
XKBSymMapRec	*map;
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
	newSyms= _XKBNewSymsForKey(xkb,r+first,nGroups*nLevels);
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
	   XKBControlsRec *controls= xkb->controls;
	   controls->numGroups= nGroups;
	   /* 8/19/93 (ef) -- XXX! generate controls notify here */
	}
/* 8/10/93 (ef) -- XXX! Look up correct index here */
	if (nLevels==1)
	     map->ktIndex= KT_1LEVEL_KEY;
	else if (IsKeypadKey(newSyms[0])&&IsKeypadKey(newSyms[1]))
	     map->ktIndex= KT_KEYPAD_KEY;
	else map->ktIndex= KT_2LEVEL_KEY;
	map->groupInfo= XKBSetGroupInfo(map->groupInfo,nGroups);
    }
    if (pMN->changed&XKBKeySymsMask) {
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
	pMN->changed|= XKBKeySymsMask;
	pMN->firstKeySym = first;
	pMN->nKeySyms = num;
    }
    return;
}

#if NeedFunctionPrototypes
void
XKBUpdateActions(	DeviceIntRec	*pXDev,
			KeyCode		 first,
			CARD8		 num,
			xKBMapNotify	*pMN)
#else
void
XKBUpdateActions(pXDev,first,num,pMN)
    DeviceIntPtr pXDev;
    KeyCode first;
    CARD8 num;
    xKBMapNotify *pMN;
#endif
{
XKBDescRec *xkb;
register int	 i,r,n;
int		 nSyms,found;
KeySym		*pSym;
XKBSymInterpretRec  *interps[8];
CARD8		 mods,repeat[32];

    xkb= &pXDev->key->xkbInfo->desc;
    if (XKBComputeAutoRepeat && pXDev->kbdfeed)
	memcpy(repeat,pXDev->kbdfeed->ctrl.autoRepeats,32);

    for (r=0; r<num; r++) {
	mods= pXDev->key->modifierMap[r+first];
	pSym= XKBKeySymsPtr(xkb,r+first);
	nSyms= XKBKeyNumSyms(xkb,r+first);
	found= 0;
	for (n=0;n<nSyms;n++,pSym++) {
	    interps[n]= NULL;
	    if (*pSym!=NoSymbol) {
		XKBSymInterpretRec *interp;
		interp= xkb->compat->symInterpret;
		for (i=0;i<xkb->compat->nSymInterpret;i++,interp++) {
		    if ((interp->sym==NoSymbol)||(*pSym==interp->sym)) {
			int match;
			switch (interp->match) {
			    case XKB_SI_NoneOf:
				match= ((interp->mods&mods)==0);
				break;
			    case XKB_SI_AnyOfOrNone:
				match= ((mods==0)||((interp->mods&mods)!=0));
				break;
			    case XKB_SI_AnyOf:
				match= ((interp->mods&mods)!=0);
				break;
			    case XKB_SI_AllOf:
				match= ((interp->mods&mods)==interp->mods);
				break;
			    case XKB_SI_Exactly:
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
		if (interps[n]&&interps[n]->action.type!=XKB_SA_NO_ACTION)
		    found++;
	    }
	}
	if (!found)
	    xkb->server->keyActions[r+first]= 0;
	else {
	    XKBAction *pActs= _XKBNewActionsForKey(xkb,r+first,nSyms);
	    for (i=0;i<nSyms;i++) {
		if (interps[i]) {
		    pActs[i]= interps[i]->action;
		    if (interps[i]->flags&XKB_SI_UseModMapMods)
			pActs[i].data= mods;
		}
		else pActs[i].type= XKB_SA_NO_ACTION;
	    }
	    if (interps[0]) {
		xkb->server->keyBehaviors[r+first]= interps[0]->behavior;
		if (XKBComputeAutoRepeat) {
		    if (interps[0]->flags&XKB_SI_Autorepeat)
			repeat[(r+first)/8]|= (1<<((r+first)%8));
		    else {
			repeat[(r+first)/8]&= ~(1<<((r+first)%8));
		    }
		}
		if (interps[0]->flags&XKB_SI_UpdateKeypad) {
		    CARD8 oldMod= pXDev->key->xkbInfo->dfltNumLockMod;
		    if (oldMod!=mods) {
			XKBKeyTypeRec *keypad;
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
		if (interps[0]->flags&XKB_SI_UpdateGroup) {
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
		if (interps[0]->flags&XKB_SI_UpdateInternal) {
		    xkb->controls->internalMods|= mods;
		    /* 9/20/93 (ef) -- XXX! Generate ControlsNotify? */
		}
		if (interps[0]->flags&XKB_SI_UpdateIgnoreLocks) {
		    xkb->controls->ignoreLockMods|= mods;
		    /* 9/20/93 (ef) -- XXX! Generate ControlsNotify? */
		}
		if (interps[0]->indicator!=XKB_SI_NoIndicator) {
		    XKBIndicatorMapRec *map;
		    map= &xkb->indicators->maps[interps[0]->indicator];
		    if (interps[0]->flags&XKB_SI_UpdateGroup) {
			map->whichGroups|= XKB_IMUseLocked;
			map->mods= 0xff;
			map->groups= 0xfe;
		    }
		    else {
			map->whichMods|= XKB_IMUseLocked;
			map->mods= mods;
			map->groups= 0;
		    }
		    pXDev->key->xkbInfo->iAccel.usesLocked|= 
						(1<<interps[0]->indicator);
		    pXDev->key->xkbInfo->iAccel.usedComponents|=XKB_IMUseLocked;
		    XKBUpdateIndicators(pXDev,(1<<interps[0]->indicator),NULL);
		}
	    }
	}
	if ((XKBComputeAutoRepeat)&&(!found)||(interps[0]==NULL))
	    repeat[(r+first)/8]|= (1<<((r+first)%8));
    }

    if (XKBComputeAutoRepeat && pXDev->kbdfeed) {
        memcpy(pXDev->kbdfeed->ctrl.autoRepeats,repeat, 32);
        if (pXDev->kbdfeed->ctrl.autoRepeat)
            (*pXDev->kbdfeed->CtrlProc)(pXDev, &pXDev->kbdfeed->ctrl);
    }
    if (pMN->changed&XKBKeyActionsMask) {
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
	pMN->changed|= XKBKeyActionsMask;
	pMN->firstKeyAction = first;
	pMN->nKeyActions = num;
    }
    return;
}

void
XKBUpdateCoreDescription(keybd)
    DeviceIntPtr keybd;
{
    /* 8/3/93 (ef) -- XXX! Not implemented yet */
}

static void
XKBInitIndicatorMap(xkb)
    XKBSrvInfoRec *xkb;
{
XKBIndicatorRec	*map = xkb->desc.indicators;
register int i;

    map->nRealIndicators = 7;
    xkb->iState= 0;
    bzero(map->maps,XKB_NUM_INDICATORS*sizeof(XKBIndicatorMapRec));
    map->maps[4].whichMods= XKB_IMUseLocked;
    map->maps[4].mods= LockMask;
    xkb->iAccel.usesLocked= (1<<4);
    xkb->iAccel.usedComponents= XKBModifierLockMask;
    return;
}

void
XKBInitDevice(pXDev)
    DeviceIntPtr pXDev;
{
int	nKeys,i;
XKBSrvInfoRec *xkb;
xKBMapNotify	mn;

    /* determine size of fixed-length arrays */
    i= sizeof(XKBAction)+sizeof(XKBSymMapRec)+sizeof(CARD16);
    i*= pXDev->key->curKeySyms.maxKeyCode+1;

    pXDev->key->xkbInfo= xkb=(XKBSrvInfoRec *)Xcalloc(sizeof(XKBSrvInfoRec)+i);
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
	xkb->Map.keySymMap= (XKBSymMapRec *)&xkb[1];
	xkb->Server.keyBehaviors = (XKBAction *)&xkb->Map.keySymMap[nKeys];
	xkb->Server.keyActions=(CARD16*)&xkb->Server.keyBehaviors[nKeys];

	xkb->dfltNumLockMod = Mod4Mask;
	xkb->compatGroup2Mod = Mod3Mask;
	xkb->dfltPtrDelta=1;
	xkb->interest = NULL;
	xkb->device = pXDev;

	XKBInitNames(xkb);
	XKBInitKeyTypes(xkb);
	XKBInitKeySyms(xkb);
	XKBInitKeyActions(xkb);
	XKBInitRadioGroups(xkb);
	XKBInitCompatStructs(xkb);

	xkb->state.group = 0;
	xkb->state.latchedGroup = 0;
	xkb->state.mods = 0;
	xkb->state.latchedMods = 0;
	xkb->state.lockedMods = 0;
	xkb->state.compatState = 0;
	xkb->state.unlockedMods = 0;
	xkb->state.groupsUnlocked = 0;

	XKBInitIndicatorMap(xkb);

	xkb->desc.controls->numGroups = 1;
	xkb->desc.controls->internalMods = 0x00;
	xkb->desc.controls->ignoreLockMods = 0xe040;
	xkb->desc.controls->enabledControls = XKBAccessXKeysMask|
					 	XKBAccessXTimeoutMask|
						XKBRepeatKeysMask|
						XKBGroupsWrapMask|
						XKBMouseKeysAccelMask|
						XKBAudibleBellMask|
						XKBAutoAutorepeatMask;
	AccessXInit(pXDev);

	XKBUpdateKeyTypesFromCore(pXDev,xkb->desc.minKeyCode,
						XKBNumKeys(&xkb->desc),&mn);
	XKBUpdateActions(pXDev,xkb->desc.minKeyCode,XKBNumKeys(&xkb->desc),&mn);
    }
}

	/*
	 * Be very careful about what does and doesn't get freed by this 
	 * function.  To reduce fragmentation, XKBInitDevice allocates a 
	 * single huge block per device and divides it up into most of the 
	 * fixed-size structures for the device.   Don't free anything that
	 * is part of this larger block.
	 */
void
XKBFreeInfo(xkb)
    XKBSrvInfoRec *xkb;
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
	    XKBKeyTypeRec	*type= xkb->desc.map->keyTypes;
	    for (i=0;i<xkb->desc.map->nKeyTypes;i++,type++) {
		if (type->preserve&&(!(type->flags&XKB_KT_DONT_FREE_PRESERVE))){
		    Xfree(type->preserve);
		    type->preserve= NULL;
		}
		if (type->map&&(!(type->flags&XKB_KT_DONT_FREE_MAP))) {
		    Xfree(type->map);
		    type->map= NULL;
	        }
	    }
	    type= xkb->desc.map->keyTypes;
	    if (!(type->flags&XKB_KT_DONT_FREE_STRUCT)) {
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
	XKBAlternateSymsRec *this,*next;
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
XKBApplyMappingChange(	DeviceIntRec	*kbd,
			CARD8		 request,
			KeyCode		 firstKey,
			CARD8		 num)
#else
void
XKBApplyMappingChange(kbd,request,firstKey,num)
    DeviceIntPtr kbd;
    CARD8 request;
    KeyCode firstKey;
    CARD8 num;
#endif
{
xKBMapNotify mn;

    if (kbd->key->xkbInfo==NULL)
	XKBInitDevice(kbd);
    bzero(&mn,sizeof(xKBMapNotify));
    if (request==MappingKeyboard) {
	XKBUpdateKeyTypesFromCore(kbd,firstKey,num,&mn);
	XKBUpdateActions(kbd,firstKey,num,&mn);
    }
    else if (request==MappingModifier) {
	XKBDescRec *desc= &kbd->key->xkbInfo->desc;
	num = desc->maxKeyCode-desc->minKeyCode+1;
	XKBUpdateActions(kbd,desc->minKeyCode,num,&mn);
    }
    if (mn.changed)
	XKBSendMapNotify(kbd,&mn);
    return;
}

void
XKBDisableComputedAutoRepeats()
{
     XKBComputeAutoRepeat = 0;
}

unsigned
XKBStateChangedFlags(old,new)
    XKBStateRec *old;
    XKBStateRec *new;
{
int		groupUnlatch,changed;

    changed=(old->group!=new->group?XKBGroupStateMask:0);
    changed|=(old->baseGroup!=new->baseGroup?XKBGroupBaseMask:0);
    changed|=(old->latchedGroup!=new->latchedGroup?XKBGroupLatchMask:0);
    changed|=(old->lockedGroup!=new->lockedGroup?XKBGroupLockMask:0);
    changed|=(old->mods!=new->mods?XKBModifierStateMask:0);
    changed|=(old->baseMods!=new->baseMods?XKBModifierBaseMask:0);
    changed|=(old->latchedMods!=new->latchedMods?XKBModifierLatchMask:0);
    changed|=(old->lockedMods!=new->lockedMods?XKBModifierLockMask:0);
    changed|=(old->compatState!=new->compatState?XKBCompatStateMask:0);
    changed|=(old->unlockedMods!=new->unlockedMods?XKBModifierUnlockMask:0);
    changed|=(old->groupsUnlocked!=new->groupsUnlocked?XKBGroupUnlockMask:0);
    return changed;
}

/***====================================================================***/

static XKBAction
XKBKeyAction(xkb,xkbState,keycode)
    XKBDescRec	*xkb;
    XKBStateRec *xkbState;
    CARD8 	 keycode;
{
int		n;
int		col;
XKBKeyTypeRec	*map;
XKBAction	*pActs;
static XKBAction fake;

    n= XKBKeyNumActions(xkb,keycode);
    pActs= XKBKeyActionsPtr(xkb,keycode);
    if (n==1)
	col = 0;
    else {
	map= XKBKeyKeyType(xkb,keycode);
	col= map->map[map->mask&xkbState->mods];
	if ((xkbState->group!=0)&&(XKBKeyNumGroups(xkb,keycode)>1)) {
	    int effectiveGroup = xkbState->group;
	    if (effectiveGroup>=XKBKeyNumGroups(xkb,keycode)) {
		if ( XKBKeyGroupsWrap(xkb,keycode) ) {
		     effectiveGroup %= XKBKeyNumGroups(xkb,keycode);
		}
		else effectiveGroup  = XKBKeyNumGroups(xkb,keycode)-1;
	    }
	    col+= (effectiveGroup*map->groupWidth);
	}
    }
    if (xkb->controls->enabledControls&XKBStickyKeysMask) {
	if (pActs[col].type==XKB_SA_SET_MODS) {
	    fake.type = XKB_SA_LATCH_MODS;
	    fake.flags= XKB_SA_CLEAR_LOCKS|XKB_SA_LATCH_TO_LOCK;
	    fake.data= pActs[col].data;
	    return fake;
	}
	if (pActs[col].type==XKB_SA_SET_GROUP) {
	    fake.type = XKB_SA_LATCH_GROUP;
	    fake.flags= XKB_SA_CLEAR_LOCKS|XKB_SA_LATCH_TO_LOCK;
	    fake.data= pActs[col].data;
	    return fake;
	}
    }
    else if (XKBPtrAction(&pActs[col])&&
	     ((xkb->controls->enabledControls&XKBMouseKeysMask)==0)) {
	fake.type = XKB_SA_NO_ACTION;
	fake.flags= 0;
	fake.data= 0;
	return fake;
    }
    return pActs[col];
}

static void
_XKBAdjustGroup(pGroup,nGroups,groupsWrap)
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
XKBComputeCompatState(xkb)
    XKBSrvInfoRec	*xkb;
{
    register int i,bit;
    CARD8 grabGroup,lookupGroup;
    XKBStateRec	*state= &xkb->state;
    XKBModCompatRec *map;

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
XKBComputeDerivedState(xkb)
    XKBSrvInfoRec	*xkb;
{
XKBStateRec	*state= &xkb->state;
XKBControlsRec	*controls= xkb->desc.controls;
Bool		wrap= ((controls->enabledControls&XKBGroupsWrapMask)!=0);

    state->mods= (state->baseMods|state->latchedMods);
    state->mods|= (state->lockedMods&(~state->unlockedMods));
    xkb->lookupState=state->mods&(~controls->internalMods);
    xkb->grabState= xkb->lookupState&(~controls->ignoreLockMods);
    xkb->grabState|= (state->baseMods&controls->ignoreLockMods);

    _XKBAdjustGroup(&state->baseGroup,controls->numGroups,wrap);
    _XKBAdjustGroup(&state->latchedGroup,controls->numGroups,wrap);
    _XKBAdjustGroup(&state->lockedGroup,controls->numGroups,wrap);

    xkb->state.group = state->baseGroup+state->latchedGroup;
    if (!xkb->state.groupsUnlocked)
	xkb->state.group+= state->lockedGroup;
    _XKBAdjustGroup(&state->group,controls->numGroups,wrap);

    if ((controls->internalMods&0xE000)==0) {
	xkb->lookupState|= ((state->group&0x7)<<13);
	if ((controls->ignoreLockMods&0xE000)==0)
	     xkb->grabState|= ((state->group&0x7)<<13);
	else xkb->grabState|= ((state->baseGroup&0x7)<<13);
    }

    XKBComputeCompatState(xkb);
    return;
}


/***====================================================================***/

#define	SYNTHETIC_KEYCODE	1

typedef struct _XKBFilter {
	CARD8			  keycode;
	CARD8			  active;
	CARD8			  filterOthers;
	CARD32			  priv;
	XKBAction		  upAction;
	int			(*filter)();
	struct _XKBFilter	 *next;
} XKBFilter;

static int
_XKBFilterSetState(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{

    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = ((pAction->flags&XKB_SA_CLEAR_LOCKS)!=0);
	filter->priv = 0;
	filter->filter = _XKBFilterSetState;
	if (XKBActionType(*pAction)==XKB_SA_SET_MODS) {
	    filter->upAction = *pAction;
	    xkb->setMods= XKBActionDataLow(*pAction);
	}
	else {
	    xkb->groupChange = XKBActionDataLow(*pAction);
	    if (pAction->flags&XKB_SA_GROUP_ABSOLUTE)
		 xkb->groupChange-= xkb->state.baseGroup;
	    filter->upAction= *pAction;
	    XKBActionSetDataLow(filter->upAction,xkb->groupChange);
	}
    }
    else if (filter->keycode==keycode) {
	if (XKBActionType(filter->upAction)==XKB_SA_SET_MODS) {
	    if (filter->upAction.flags&XKB_SA_CLEAR_LOCKS) {
		xkb->state.lockedMods&= ~XKBActionDataLow(filter->upAction);
	    }
	    xkb->clearMods = XKBActionDataLow(filter->upAction);
	}
	else {
	    if (filter->upAction.flags&XKB_SA_CLEAR_LOCKS) {
		xkb->state.lockedGroup = 0;
	    }
	    xkb->groupChange = -XKBActionDataLow(filter->upAction);
	}
	filter->active = 0;
    }
    else {
	filter->upAction.flags&= ~XKB_SA_CLEAR_LOCKS;
	filter->filterOthers = 0;
    }
    return 1;
}

#define	LATCH_KEY_DOWN	1
#define	LATCH_PENDING	2
#define	NO_LATCH	3

static int
_XKBFilterLatchState(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{

    if (filter->keycode==0) {			/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 1;
	filter->priv = LATCH_KEY_DOWN;
	filter->filter = _XKBFilterLatchState;
	if (XKBActionType(*pAction)==XKB_SA_LATCH_MODS) {
	    filter->upAction = *pAction;
	    xkb->setMods = XKBActionDataLow(*pAction);
	}
	else {
	    xkb->groupChange = XKBActionDataLow(*pAction);
	    if (pAction->flags&XKB_SA_GROUP_ABSOLUTE)
		 xkb->groupChange-= xkb->state.baseGroup;
	    filter->upAction= *pAction;
	    XKBActionSetDataLow(filter->upAction,xkb->groupChange);
	}
    }
    else if ( pAction && (filter->priv==LATCH_PENDING) ) {
	if (pAction->type==XKB_SA_NO_ACTION) {
	    filter->active = 0;
	    if (XKBActionType(filter->upAction)==XKB_SA_LATCH_MODS)
		 xkb->state.latchedMods&= ~XKBActionDataLow(filter->upAction);
	    else xkb->state.latchedGroup-= XKBActionDataLow(filter->upAction);
	}
	else if ((pAction->type==filter->upAction.type)&&
		 (pAction->data==filter->upAction.data)&&
		 (pAction->flags==filter->upAction.flags)) {
	    if (filter->upAction.flags&XKB_SA_LATCH_TO_LOCK) {
		if (XKBActionType(filter->upAction)==XKB_SA_LATCH_MODS)
		     XKBActionSetType(*pAction,XKB_SA_LOCK_MODS);
		else XKBActionSetType(*pAction,XKB_SA_LOCK_GROUP);
	    }
	    else {
		if (XKBActionType(filter->upAction)==XKB_SA_LATCH_MODS)
		     XKBActionSetType(*pAction,XKB_SA_SET_MODS);
		else XKBActionSetType(*pAction,XKB_SA_SET_GROUP);
	    }
	    if (XKBActionType(filter->upAction)==XKB_SA_LATCH_MODS)
		 xkb->state.latchedMods&= ~XKBActionDataLow(filter->upAction);
	    else xkb->state.latchedGroup-= XKBActionDataLow(filter->upAction);
	    filter->active = 0;
	}
    }
    else if (filter->keycode==keycode) {	/* release */
	if (XKBActionType(filter->upAction)==XKB_SA_LATCH_MODS) {
	     xkb->clearMods = XKBActionDataLow(filter->upAction);
	     if ((filter->upAction.flags&XKB_SA_CLEAR_LOCKS)&&
		 (xkb->clearMods&xkb->state.lockedMods)==xkb->clearMods) {
		xkb->state.lockedMods&= ~xkb->clearMods;
		filter->priv= NO_LATCH;
	     }
	}
	else {
	    xkb->groupChange = -XKBActionDataLow(filter->upAction);
	    if ((filter->upAction.flags&XKB_SA_CLEAR_LOCKS)&&
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
	    if (XKBActionType(filter->upAction)==XKB_SA_LATCH_MODS)
		 xkb->state.latchedMods |= XKBActionDataLow(filter->upAction);
	    else xkb->state.latchedGroup+= XKBActionDataLow(filter->upAction);
	}
    }
    else if (filter->priv==LATCH_KEY_DOWN) {
	filter->priv= NO_LATCH;
	filter->filterOthers = 0;
    }
    return 1;
}

static int
_XKBFilterLockState(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{

    if (pAction&&(XKBActionType(*pAction)==XKB_SA_LOCK_GROUP)) {
	if (pAction->flags&XKB_SA_GROUP_ABSOLUTE)
	     xkb->state.lockedGroup= XKBActionDataLow(*pAction);
	else xkb->state.lockedGroup+= XKBActionDataLow(*pAction);
	return 1;
    }
    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv = 0;
	filter->filter = _XKBFilterLockState;
	filter->upAction = *pAction;
	xkb->state.lockedMods^= XKBActionDataLow(*pAction);
	xkb->setMods = XKBActionDataLow(*pAction);
    }
    else if (filter->keycode==keycode) {
	filter->active = 0;
	xkb->clearMods = XKBActionDataLow(filter->upAction);
    }
    return 1;
}

#define	ISO_KEY_DOWN		0
#define	NO_ISO_LOCK		1

static int
_XKBFilterISOLock(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{

    if (filter->keycode==0) {		/* initial press */
	CARD8	flags = XKBActionDataHigh(*pAction);
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 1;
	filter->priv = ISO_KEY_DOWN;
	filter->upAction = *pAction;
	filter->filter = _XKBFilterISOLock;
	if (flags&XKB_SA_ISO_DFLT_IS_GROUP) {
	    xkb->groupChange = XKBActionDataLow(*pAction);
	    xkb->setMods = 0;
	}
	else {
	    xkb->setMods = XKBActionDataLow(*pAction);
	    xkb->groupChange = 0;
	}
	if ((!(flags&XKB_SA_ISO_NO_AFFECT_MODS))&&(xkb->state.baseMods)) {
	    filter->priv= NO_ISO_LOCK;
	    xkb->state.lockedMods^= xkb->state.baseMods;
	}
	if ((!(flags&XKB_SA_ISO_NO_AFFECT_GROUP))&&(xkb->state.baseGroup)) {
/* 6/22/93 (ef) -- lock groups if group key is down first */
	}
	if (!(flags&XKB_SA_ISO_NO_AFFECT_PTR)) {
/* 6/22/93 (ef) -- lock mouse buttons if they're down */
	}
    }
    else if (filter->keycode==keycode) {
	CARD8	flags = XKBActionDataHigh(filter->upAction);
	if (flags&XKB_SA_ISO_DFLT_IS_GROUP) {
	    xkb->groupChange = -XKBActionDataHigh(filter->upAction);
	    xkb->clearMods = 0;
	    if (filter->priv==ISO_KEY_DOWN)
		xkb->state.lockedGroup+= XKBActionDataHigh(filter->upAction);
	}
	else {
	    xkb->clearMods= XKBActionDataLow(filter->upAction);
	    xkb->groupChange= 0;
	    if (filter->priv==ISO_KEY_DOWN)
		xkb->state.lockedMods^= XKBActionDataLow(filter->upAction);
	}
	filter->active = 0;
    }
    else if (pAction) {
	CARD8	flags = XKBActionDataHigh(filter->upAction);
	switch (XKBActionType(*pAction)) {
	    case XKB_SA_SET_MODS: case XKB_SA_LATCH_MODS:
		if (!(flags&XKB_SA_ISO_NO_AFFECT_MODS)) {
		    XKBActionSetType(*pAction,XKB_SA_LOCK_MODS);
		    filter->priv= NO_ISO_LOCK;
		}
		break;
	    case XKB_SA_SET_GROUP: case XKB_SA_LATCH_GROUP:
		if (!(flags&XKB_SA_ISO_NO_AFFECT_GROUP)) {
		    XKBActionSetType(*pAction,XKB_SA_LOCK_GROUP);
		    filter->priv= NO_ISO_LOCK;
		}
		break;
	    case XKB_SA_PTR_BTN:
		if (!(flags&XKB_SA_ISO_NO_AFFECT_PTR)) {
		     XKBActionSetType(*pAction,XKB_SA_LOCK_PTR_BTN);
		     filter->priv= NO_ISO_LOCK;
		}
		break;
	    case XKB_SA_SET_CONTROLS:
		if (!(flags&XKB_SA_ISO_NO_AFFECT_CTRLS)) {
		    XKBActionSetType(*pAction,XKB_SA_LOCK_CONTROLS);
		    filter->priv= NO_ISO_LOCK;
		}
		break;
	}
    }
    return 1;
}


static CARD32
_XKBPtrAccelExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
XKBSrvInfoRec	*xkbInfo= (XKBSrvInfoRec *)arg;
XKBControlsRec	*ctrls= xkbInfo->desc.controls;
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
_XKBFilterPointerMove(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{
    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv=0;
	filter->filter = _XKBFilterPointerMove;
	filter->upAction= *pAction;
	xkb->mouseKeysCounter= 0;
	xkb->mouseKey= keycode;
	DDXFakePointerMotion((INT8)XKBActionDataHigh(*pAction),
					  (INT8)XKBActionDataLow(*pAction));
	xkb->mouseKeysAccel= 
		((XKBActionType(*pAction)==XKB_SA_ACCEL_PTR)||
		((XKBActionType(*pAction)==XKB_SA_MOVE_PTR)&&
		 (xkb->desc.controls->enabledControls&XKBMouseKeysAccelMask)));
	xkb->mouseKeysDX= (INT8)XKBActionDataHigh(*pAction);
	xkb->mouseKeysDY= (INT8)XKBActionDataLow(*pAction);
	xkb->mouseKeyTimer= TimerSet(xkb->mouseKeyTimer, 0,
				xkb->desc.controls->mouseKeysDelay,
				_XKBPtrAccelExpire,(pointer)xkb);
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
_XKBFilterPointerBtn(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{
    if (filter->keycode==0) {		/* initial press */
	int button = XKBActionDataLow(*pAction);
	if (button==XKB_SA_USE_DFLT_BUTTON)
	    button = xkb->desc.controls->mouseKeysDfltBtn;

	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv=0;
	filter->filter = _XKBFilterPointerBtn;
	filter->upAction= *pAction;
	XKBActionSetDataLow(filter->upAction,button);
	switch (XKBActionType(*pAction)) {
	    case XKB_SA_LOCK_PTR_BTN:
		if ((xkb->lockedPtrButtons&(1<<button))==0) {
		    xkb->lockedPtrButtons|= (1<<button);
		    DDXFakePointerButton(ButtonPress,button);
		    filter->upAction.type= XKB_SA_NO_ACTION;
		}
		break;
	    case XKB_SA_PTR_BTN:
		DDXFakePointerButton(ButtonPress,button);
		break;
	    case XKB_SA_CLICK_PTR_BTN:
		{
		    register int i,nClicks;
		    nClicks= (INT8)XKBActionDataHigh(*pAction);
		    for (i=0;i<nClicks;i++) {
			DDXFakePointerButton(ButtonPress,button);
			DDXFakePointerButton(ButtonRelease,button);
		    }
		    filter->upAction.type= XKB_SA_NO_ACTION;
		}
		break;
	    case XKB_SA_SET_PTR_DFLT:
		{
		    XKBControlsRec	*ctrls= xkb->desc.controls;
		    XKBControlsRec	old= *ctrls;
		    xKBControlsNotify	cn;
		    switch (XKBActionDataHigh(*pAction)) {
			case XKB_SA_SET_DFLT_BTN:
			    ctrls->mouseKeysDfltBtn= XKBActionDataLow(*pAction);
			    break;
			case XKB_SA_INCR_DFLT_BTN:
			    ctrls->mouseKeysDfltBtn+=XKBActionDataLow(*pAction);
			    if (ctrls->mouseKeysDfltBtn>5)
				ctrls->mouseKeysDfltBtn= 5;
			    else if (ctrls->mouseKeysDfltBtn<1)
				ctrls->mouseKeysDfltBtn= 1;
			    break;
		    }
		    if (XKBComputeControlsNotify(xkb->device,
						&old,xkb->desc.controls,
						&cn)) {
			cn.keycode = keycode;
			cn.eventType = KeyPress;
			cn.requestMajor = 0;
			cn.requestMinor = 0;
			XKBSendControlsNotify(xkb->device,&cn);
		    }
		}
		break;
	}
    }
    else if (filter->keycode==keycode) {
	int button= (INT16)XKBActionDataLow(filter->upAction);
	switch (XKBActionType(filter->upAction)) {
	    case XKB_SA_LOCK_PTR_BTN:
		xkb->lockedPtrButtons&= ~(1<<button);
	    case XKB_SA_PTR_BTN:
		DDXFakePointerButton(ButtonRelease,button);
		break;
	}
	filter->active = 0;
    }
    return 0;
}

static int
_XKBFilterControls(xkb,filter,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    XKBFilter		*filter;
    CARD8		 keycode;
    XKBAction		*pAction;
{
XKBControlsRec	old= *xkb->desc.controls;
CARD16		change;

    if (filter->keycode==0) {		/* initial press */
	filter->keycode = keycode;
	filter->active = 1;
	filter->filterOthers = 0;
	filter->priv = change= XKBActionData(*pAction);
	filter->filter = _XKBFilterControls;
	filter->upAction = *pAction;

	if (XKBActionType(*pAction)==XKB_SA_LOCK_CONTROLS) {
	    filter->priv= (xkb->desc.controls->enabledControls&change);
	    change&= ~xkb->desc.controls->enabledControls;
	}

	if (change) {
	    xKBControlsNotify cn;
	    xkb->desc.controls->enabledControls|= change;
	    if (XKBComputeControlsNotify(xkb->device,&old,xkb->desc.controls,
									&cn)) {
		cn.keycode = keycode;
		cn.eventType = KeyPress;
		cn.requestMajor = 0;
		cn.requestMinor = 0;
		XKBSendControlsNotify(xkb->device,&cn);
	    }
	    if (xkb->iAccel.usesControls)
		XKBUpdateIndicators(xkb->device,xkb->iAccel.usesControls,NULL);
	}
    }
    else if (filter->keycode==keycode) {
	change= filter->priv;
	if (change) {
	    xKBControlsNotify cn;
	    xkb->desc.controls->enabledControls&= ~change;
	    if (XKBComputeControlsNotify(xkb->device,&old,xkb->desc.controls,
									&cn)) {
		cn.keycode = keycode;
		cn.eventType = KeyRelease;
		cn.requestMajor = 0;
		cn.requestMinor = 0;
		XKBSendControlsNotify(xkb->device,&cn);
	    }
	    if (xkb->iAccel.usesControls)
		XKBUpdateIndicators(xkb->device,xkb->iAccel.usesControls,NULL);
	}
	filter->keycode= 0;
	filter->active= 0;
    }
    return 0;
}

static	int		 szFilters = 0;
static	XKBFilter	*filters = NULL;

static XKBFilter *
_XKBNextFreeFilter()
{
register int i;

    if (szFilters==0) {
	szFilters = 4;
	filters = (XKBFilter *)Xcalloc(szFilters*sizeof(XKBFilter));
	/* 6/21/93 (ef) -- XXX! deal with allocation failure */
    }
    for (i=0;i<szFilters;i++) {
	if (!filters[i].active) {
	    filters[i].keycode = 0;
	    return &filters[i];
	}
    }
    szFilters*=2;
    filters= (XKBFilter *)Xrealloc(filters,szFilters*sizeof(XKBFilter));
    /* 6/21/93 (ef) -- XXX! deal with allocation failure */
    bzero(&filters[szFilters/2],(szFilters/2)*sizeof(XKBFilter));
    return &filters[szFilters/2];
}

static int
_XKBApplyFilters(xkb,keycode,pAction)
    XKBSrvInfoRec	*xkb;
    CARD8		 keycode;
    XKBAction		*pAction;
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
XKBHandleActions(xE,keybd,count)
    xEvent	 *xE;
    DeviceIntPtr  keybd;
    int		  count;
{
int		key,bit,i;
register CARD8	realMods;
XKBSrvInfoRec	*xkb;
KeyClassRec	*keyc = keybd->key;
int		 changed,sendEvent;
CARD16		 data;
XKBStateRec	 oldState;
XKBAction	 action;
XKBFilter	*filter;

    key= xE->u.u.detail;
    realMods = keyc->modifierMap[key];
    keyc->modifierMap[key]= 0;
    xkb= keyc->xkbInfo;
    oldState= xkb->state;

    xkb->clearMods = xkb->setMods = 0;
    xkb->groupChange = 0;

    sendEvent = 1;

    if ( xE->u.u.type == KeyPress ) {
	action = XKBKeyAction(&xkb->desc,&xkb->state,key);
	sendEvent = _XKBApplyFilters(xkb,key,&action);
	if (sendEvent) {
	    data = XKBActionData(action);

	    switch (XKBActionType(action)) {
		case XKB_SA_SET_MODS:
		case XKB_SA_SET_GROUP:
		    filter = _XKBNextFreeFilter();
		    sendEvent = _XKBFilterSetState(xkb,filter,key,&action);
		    break;
		case XKB_SA_LATCH_MODS:
		case XKB_SA_LATCH_GROUP:
		    filter = _XKBNextFreeFilter();
		    sendEvent=_XKBFilterLatchState(xkb,filter,key,&action);
		    break;
		case XKB_SA_LOCK_MODS:
		case XKB_SA_LOCK_GROUP:
		    filter = _XKBNextFreeFilter();
		    sendEvent=_XKBFilterLockState(xkb,filter,key,&action);
		    break;
		case XKB_SA_ISO_LOCK:
		    filter = _XKBNextFreeFilter();
		    sendEvent=_XKBFilterISOLock(xkb,filter,key,&action);
		    break;
		case XKB_SA_ACCEL_PTR:
		case XKB_SA_MOVE_PTR:
		    filter = _XKBNextFreeFilter();
		    sendEvent= _XKBFilterPointerMove(xkb,filter,key,&action);
		    break;
		case XKB_SA_PTR_BTN:
		case XKB_SA_CLICK_PTR_BTN:
		case XKB_SA_LOCK_PTR_BTN:
		case XKB_SA_SET_PTR_DFLT:
		    filter = _XKBNextFreeFilter();
		    sendEvent= _XKBFilterPointerBtn(xkb,filter,key,&action);
		    break;
		case XKB_SA_TERMINATE:
		    sendEvent= DDXTerminateServer(keybd,key,&action);
		    break;
		case XKB_SA_SWITCH_SCREEN:
		    sendEvent= DDXSwitchScreen(keybd,key,&action);
		    break;
		case XKB_SA_SET_CONTROLS:
		case XKB_SA_LOCK_CONTROLS:
		    filter = _XKBNextFreeFilter();
		    sendEvent=_XKBFilterControls(xkb,filter,key,&action);
		    break;
	    }
	}
    }
    else {
	sendEvent = _XKBApplyFilters(xkb,key,NULL);
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
    XKBComputeDerivedState(xkb);
    keyc->state= xkb->lookupState&0xE0FF;

    changed = XKBStateChangedFlags(&oldState,&xkb->state);
    if (changed) {
	xKBStateNotify	sn;
	sn.keycode= key;
	sn.eventType= xE->u.u.type;
	sn.requestMajor = sn.requestMinor = 0;
	sn.changed= changed;
	XKBSendStateNotify(keybd,&sn);
    }
    if (changed&xkb->iAccel.usedComponents) {
	changed= XKBIndicatorsToUpdate(keybd,changed);
	if (changed)
	    XKBUpdateIndicators(keybd,changed,NULL);
    }

    return;
}

void
XKBProcessKeyboardEvent(xE,keybd,count)
    xEvent *xE;
    DeviceIntPtr keybd;
    int count;
{
KeyClassRec	*keyc = keybd->key;
XKBSrvInfoRec	*xkb;
int		 key;
XKBAction	 action;

    xkb= keyc->xkbInfo;
    key= xE->u.u.detail;

    action= xkb->desc.server->keyBehaviors[key];
    switch (XKBActionType(action)) {
	case XKB_KB_DEFAULT:
	    if (( xE->u.u.type == KeyPress ) && 
		(keyc->down[key>>3] & (1<<(key&7)))) {
		xE->u.u.type = KeyRelease;
		XKBHandleActions(xE,keybd,count);
		xE->u.u.type = KeyPress;
		XKBHandleActions(xE,keybd,count);
		return;
	    }
	    else if ((xE->u.u.type==KeyRelease) &&
			(!(keyc->down[key>>3]&(1<<(key&7))))) {
		xE->u.u.type = KeyPress;
		XKBHandleActions(xE,keybd,count);
		xE->u.u.type = KeyRelease;
		XKBHandleActions(xE,keybd,count);
		return;
	    }
	    break;
	case XKB_KB_LOCK:
	    if ( xE->u.u.type == KeyRelease )
		return;
	    else {
		int	bit= 1<<(key&7);
		if ( keyc->down[key>>3]&bit )
		    xE->u.u.type= KeyRelease;
	    }
	    break;
	case XKB_KB_RADIO_GROUP:
	    if ( xE->u.u.type == KeyRelease )
		return;
	    else {
		int		ndx = XKBActionDataLow(action);
		if ( ndx<xkb->nRadioGroups ) {
		    XKBRadioGroupRec	*rg;
		    register int i;
		    rg = &xkb->radioGroups[ndx];
		    if ( rg->currentDown == xE->u.u.detail ) {
			xE->u.u.type = KeyRelease;
			XKBHandleActions(xE,keybd,count);
			rg->currentDown= 0;
			return;
		    }
		    if ( rg->currentDown!=0 ) {
			int key = xE->u.u.detail;
			xE->u.u.type= KeyRelease;
			xE->u.u.detail= rg->currentDown;
			XKBHandleActions(xE,keybd,count);
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
    XKBHandleActions(xE,keybd,count);
    return;
}

void
ProcessKeyboardEvent(xE,keybd,count)
    xEvent *xE;
    DeviceIntPtr keybd;
    int	count;
{
KeyClassRec	*keyc = keybd->key;
XKBSrvInfoRec	*xkb;
int		 key;
XKBAction	 action;

    xkb= keyc->xkbInfo;
    key= xE->u.u.detail;

    if (!(xkb->desc.controls->enabledControls&ALL_FILTERED_MASK))
	XKBProcessKeyboardEvent(xE,keybd,count);
    else if (xE->u.u.type==KeyPress)
	AccessXFilterPressEvent(xE,keybd,count);
    else if (xE->u.u.type==KeyRelease)
	AccessXFilterReleaseEvent(xE,keybd,count);
    return;
}

/***====================================================================***/

int
XKBLatchModifiers(pXDev,mask,latches)
    DeviceIntPtr pXDev;
    CARD8 mask;
    CARD8 latches;
{
XKBSrvInfoRec	*xkb;
XKBFilter	*filter;
XKBAction	 action;
unsigned	 clear;

    if ( pXDev && pXDev->key && pXDev->key->xkbInfo ) {
	xkb = pXDev->key->xkbInfo;
	clear= (mask&(~latches));
	xkb->state.latchedMods&= ~clear;
	action.type = XKB_SA_LATCH_MODS;
	action.flags = 0;
	action.data = mask&latches;
	filter = _XKBNextFreeFilter();
	_XKBFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,&action);
	_XKBFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,NULL);
	return Success;
    }
    return BadValue;
}

int
XKBLatchGroup(pXDev,group)
    DeviceIntPtr  pXDev;
    int		  group;
{
XKBSrvInfoRec	*xkb;
XKBFilter	*filter;
XKBAction	 action;

    if ( pXDev && pXDev->key && pXDev->key->xkbInfo ) {
	xkb = pXDev->key->xkbInfo;
	action.type = XKB_SA_LATCH_GROUP;
	action.flags = 0;
	action.data = group;
	filter = _XKBNextFreeFilter();
	_XKBFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,&action);
	_XKBFilterLatchState(xkb,filter,SYNTHETIC_KEYCODE,NULL);
	return Success;
    }
    return BadValue;
}

