/* $XConsortium: xkbInit.c,v 1.7 93/09/29 20:53:34 rws Exp $ */
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

#define	CREATE_ATOM(s)	MakeAtom(s,sizeof(s)-1,1)

#define IsKeypadKey(keysym) \
  (((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#ifdef sgi
#define LED_CAPS	5
#define	LED_NUM		6
#define	LED_SCROLL	7
#else
#define	LED_CAPS	1
#define	LED_NUM		2
#define	LED_SCROLL	3
#endif

/***====================================================================***/

#define	KT_1LEVEL_KEY		0
#define	KT_2LEVEL_KEY		1
#define	KT_KEYPAD_KEY		2
#define	KT_3LEVEL_KEY		3
#define	KT_PC_BREAK_KEY		4
#define	KT_PC_SYSRQ_KEY		5
#define	KT_COUNT		6

#define	NUM_LOCK_VMOD_INDEX	0
#define	MODE_SWITCH_VMOD_INDEX	1
#define	LEVEL_THREE_VMOD_INDEX	2
#define	SCROLL_LOCK_VMOD_INDEX	3

#define	NUM_LOCK_VMOD_MASK	(1<<NUM_LOCK_VMOD_INDEX)
#define	MODE_SWITCH_VMOD_MASK	(1<<MODE_SWITCH_VMOD_INDEX)
#define	LEVEL_THREE_VMOD_MASK	(1<<LEVEL_THREE_VMOD_INDEX)
#define	SCROLL_LOCK_VMOD_MASK	(1<<SCROLL_LOCK_VMOD_INDEX)

#define	NUM_LOCK_MOD_INDEX	Mod4MapIndex
#define	MODE_SWITCH_MOD_INDEX	Mod3MapIndex
#define	LEVEL_THREE_MOD_INDEX	Mod3MapIndex
#define	SCROLL_LOCK_MOD_INDEX	XkbNoModifier

#define	NUM_LOCK_MOD_MASK	Mod4Mask
#define	MODE_SWITCH_MOD_MASK	Mod3Mask
#define	LEVEL_THREE_MOD_MASK	Mod3Mask
#define	SCROLL_LOCK_MOD_MASK	XkbNoModifierMask

char	*XkbBaseDirectory=	NULL;
char	*XkbInitialMap=		NULL;

static void
XkbInitKeyTypes(xkb)
    XkbSrvInfoRec *xkb;
{
XkbKeyTypeRec	*kt;
XkbDescRec	*desc = &xkb->desc;

    if (desc->map->num_types>0)
	return;

    desc->map->types=(XkbKeyTypeRec*)Xcalloc(KT_COUNT*sizeof(XkbKeyTypeRec));
    if (!desc->map->types) {
	desc->map->num_types = 0;
	return;
    }

    desc->map->num_types = KT_COUNT;
    kt = &desc->map->types[KT_1LEVEL_KEY];
    kt->mask = 0;
    kt->group_width = 1;
    kt->real_mods = 0;
    kt->vmods = 0;
    kt->map_count = 0;
    kt->map = NULL;
    kt->preserve= NULL;
    kt->free = 0;

    kt = &desc->map->types[KT_2LEVEL_KEY];
    kt->mask = ShiftMask;
    kt->real_mods = ShiftMask;
    kt->group_width = 2;
    kt->vmods= 0;
    kt->map_count= 1;
    kt->map=(XkbKTMapEntryPtr)Xcalloc(kt->map_count*sizeof(XkbKTMapEntryRec));
    kt->map[0].active= 	    1;
    kt->map[0].mask= 	    ShiftMask;
    kt->map[0].level=       1;
    kt->map[0].vmods= 0;
    kt->map[0].real_mods=    ShiftMask;
    kt->preserve= NULL;
    kt->free = XkbNoFreeKTStruct;

    kt = &desc->map->types[KT_KEYPAD_KEY];
    kt->mask = ShiftMask;
    kt->real_mods = ShiftMask;
    kt->group_width = 2;
    kt->vmods = NUM_LOCK_VMOD_MASK;
    kt->map_count = 2;
    kt->map=(XkbKTMapEntryPtr)Xcalloc(kt->map_count*sizeof(XkbKTMapEntryRec));
    kt->map[0].active= 	    1;
    kt->map[0].mask= 	    ShiftMask;
    kt->map[0].level=       1;
    kt->map[0].vmods= 0;
    kt->map[0].real_mods=    ShiftMask;
    kt->map[1].active= 	    1;
    kt->map[1].mask=        NUM_LOCK_MOD_MASK;
    kt->map[1].level=       1;
    kt->map[1].vmods= NUM_LOCK_VMOD_MASK;
    kt->map[1].real_mods=    0;
    kt->preserve= NULL;
    kt->free = XkbNoFreeKTStruct;

    kt = &desc->map->types[KT_3LEVEL_KEY];
    kt->mask = ShiftMask;
    kt->real_mods = ShiftMask;
    kt->group_width = 3;
    kt->vmods = LEVEL_THREE_VMOD_MASK;
    kt->map_count = 3;
    kt->map=(XkbKTMapEntryPtr)Xcalloc(kt->map_count*sizeof(XkbKTMapEntryRec));
    kt->map[0].active= 	    1;
    kt->map[0].mask=        ShiftMask;
    kt->map[0].level=       1;
    kt->map[0].vmods= 0;
    kt->map[0].real_mods=    ShiftMask;
    kt->map[1].active= 	    1;
    kt->map[1].mask=        LEVEL_THREE_MOD_MASK;
    kt->map[1].level=       2;
    kt->map[1].vmods= LEVEL_THREE_VMOD_MASK;
    kt->map[1].real_mods=    0;
    kt->map[2].active= 	    1;
    kt->map[2].mask=        ShiftMask|LEVEL_THREE_VMOD_MASK;
    kt->map[2].vmods= LEVEL_THREE_VMOD_MASK;
    kt->map[2].level=       2;
    kt->map[2].real_mods=    ShiftMask;
    kt->preserve= NULL;
    kt->free = XkbNoFreeKTStruct;

    kt = &desc->map->types[KT_PC_BREAK_KEY];
    kt->mask = ControlMask;
    kt->real_mods = ControlMask;
    kt->group_width = 2;
    kt->vmods = 0;
    kt->map_count= 1;
    kt->map=(XkbKTMapEntryPtr)Xcalloc(kt->map_count*sizeof(XkbKTMapEntryRec));
    kt->map[0].active= 	    1;
    kt->map[0].mask=        ControlMask;
    kt->map[0].vmods= 0;
    kt->map[0].real_mods=    ControlMask;
    kt->map[0].level=       1;
    kt->preserve= NULL;
    kt->free = XkbNoFreeKTStruct;

    kt = &desc->map->types[KT_PC_SYSRQ_KEY];
    kt->mask = Mod1Mask;
    kt->real_mods = Mod1Mask;
    kt->group_width = 2;
    kt->vmods= 0;
    kt->map_count= 1;
    kt->map=(XkbKTMapEntryPtr)Xcalloc(kt->map_count*sizeof(XkbKTMapEntryRec));
    kt->map[0].active= 	    1;
    kt->map[0].mask=        Mod1Mask;
    kt->map[0].vmods= 0;
    kt->map[0].real_mods=    Mod1Mask;
    kt->map[0].level=       1;
    kt->preserve= NULL;
    kt->free = XkbNoFreeKTStruct;
    return;
}

static void
XkbInitKeySyms(xkb)
    XkbSrvInfoRec *xkb;
{
XkbClientMapRec	*map= xkb->desc.map;
int	nKeys= xkb->desc.max_key_code-xkb->desc.min_key_code+1;

    map->num_syms= 1;
    map->size_syms= (nKeys*15)/10;
    map->syms=(KeySym *)Xcalloc(map->size_syms*sizeof(KeySym));
    if (map->syms) {
	register int i;
	XkbSymMapRec *symMap= &map->key_sym_map[xkb->desc.min_key_code];
	map->syms[0]= NoSymbol;
	for (i=0;i<nKeys;symMap++,i++) {
	    symMap->kt_index = KT_1LEVEL_KEY;
	    symMap->group_info = XkbSetGroupInfo(0,FALSE);
	    symMap->offset = 0;
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
int	nKeys= xkb->desc.max_key_code-xkb->desc.min_key_code+1;

    map->num_acts= 1;
    map->size_acts= (nKeys/2)+1;
    map->acts=(XkbAction *)Xcalloc(map->size_acts*sizeof(XkbAction));
    if (map->acts) {
	register int i;
	map->acts[0].type = XkbSA_NoAction;
	for (i=0;i<nKeys;i++) {
	    map->key_acts[i+xkb->desc.min_key_code]= 0;
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
    if ( xkb->num_rg>0 )
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

#if defined(luna)
#define	XKB_DDX_PERMANENT_LOCK	1
#endif
#include "xkbDflts.h"

static void
XkbInitCompatStructs(xkb)
    XkbSrvInfoRec *xkb;
{
register int i;
XkbCompatRec	*compat;

    compat = xkb->desc.compat;
    for (i=0;i<XkbNumModifiers;i++) {
	compat->real_mod_compat[i].mods = (1<<i);
	compat->real_mod_compat[i].groups = 0;
	compat->mod_compat[i]= &compat->real_mod_compat[i];
    }
    for (i=0;i<XkbNumVirtualMods;i++) {
	compat->vmod_compat[i].mods = 0;
	compat->vmod_compat[i].groups = 0;
    }
    compat->vmod_compat[MODE_SWITCH_VMOD_INDEX].groups= 0xfe;

    compat->num_si= 0;
    compat->sym_interpret = (XkbSymInterpretRec *)Xcalloc(sizeof(dfltSI));
    if (compat->sym_interpret) {
	compat->num_si = num_dfltSI;
	memcpy((char *)compat->sym_interpret,(char *)dfltSI,sizeof(dfltSI));
    }
    return;
}

/***====================================================================***/

static void
XkbInitNames(xkb)
    XkbSrvInfoPtr xkb;
{
XkbClientMapPtr	map;
XkbNamesPtr	names;
register int	i;

    map = xkb->desc.map;
    names = xkb->desc.names;
    bzero(names,sizeof(XkbNamesRec));
    names->keycodes= CREATE_ATOM("SGI_IRIS");
    names->phys_geometry= names->geometry= CREATE_ATOM("SGI_102");
    names->phys_symbols= names->symbols= CREATE_ATOM("ascii");
    names->semantics= CREATE_ATOM("DEFAULT");
    names->mods[0] = CREATE_ATOM("Shift");
    names->mods[1] = CREATE_ATOM("Caps");
    names->mods[2] = CREATE_ATOM("Control");
    names->mods[3] = CREATE_ATOM("Alt");
    names->mods[4] = CREATE_ATOM("Mod2");
    names->mods[5] = CREATE_ATOM("Mod3");
    names->mods[6] = CREATE_ATOM("Mod4");
    names->mods[7] = CREATE_ATOM("Mod5");
    names->vmods[NUM_LOCK_VMOD_INDEX]= CREATE_ATOM("Num Lock");
    names->vmods[MODE_SWITCH_VMOD_INDEX]= CREATE_ATOM("Mode Switch");
    names->vmods[LEVEL_THREE_VMOD_INDEX]= CREATE_ATOM("Level Three");
    names->vmods[SCROLL_LOCK_VMOD_INDEX]= CREATE_ATOM("Scroll Lock");
    names->indicators[4] = CREATE_ATOM("Caps Lock");
    names->indicators[5] = CREATE_ATOM("Num Lock");
    names->indicators[6] = CREATE_ATOM("Scroll Lock");
    if (map->types) {
	Atom *levelNames;
	map->types[KT_1LEVEL_KEY].name= CREATE_ATOM("ONE_LEVEL");
	map->types[KT_2LEVEL_KEY].name= CREATE_ATOM("TWO_LEVEL");
	map->types[KT_KEYPAD_KEY].name= CREATE_ATOM("KEYPAD");
	map->types[KT_3LEVEL_KEY].name= CREATE_ATOM("THREE_LEVEL");
	map->types[KT_PC_BREAK_KEY].name= CREATE_ATOM("PC_BREAK");
	map->types[KT_PC_SYSRQ_KEY].name= CREATE_ATOM("PC_SYS_RQ");

	levelNames= (Atom *)Xcalloc(sizeof(Atom));
	if (levelNames!=NULL) {
	    levelNames[0]= CREATE_ATOM("Any");
	}
	map->types[KT_1LEVEL_KEY].lvl_names= levelNames;

	levelNames= (Atom *)Xcalloc(2*sizeof(Atom));
	if (levelNames!=NULL) {
	    levelNames[0]= CREATE_ATOM("Base");
	    levelNames[1]= CREATE_ATOM("Shift");
	}
	map->types[KT_2LEVEL_KEY].lvl_names= levelNames;

	levelNames= (Atom *)Xcalloc(2*sizeof(Atom));
	if (levelNames) {
	    levelNames[0]= CREATE_ATOM("Base");
	    levelNames[1]= CREATE_ATOM("Num Lock");
	}
	map->types[KT_KEYPAD_KEY].lvl_names= levelNames;

	levelNames= (Atom *)Xcalloc(3*sizeof(Atom));
	if (levelNames) {
	    levelNames[0]= CREATE_ATOM("Base");
	    levelNames[1]= CREATE_ATOM("Shift");
	    levelNames[2]= CREATE_ATOM("Level 3");
	}
	map->types[KT_3LEVEL_KEY].lvl_names= levelNames;

	levelNames= (Atom *)Xcalloc(2*sizeof(Atom));
	if (levelNames) {
	    levelNames[0]= CREATE_ATOM("Base");
	    levelNames[1]= CREATE_ATOM("Control");
	}
	map->types[KT_PC_BREAK_KEY].lvl_names= levelNames;

	levelNames= (Atom *)Xcalloc(2*sizeof(Atom));
	if (levelNames) {
	    levelNames[0]= CREATE_ATOM("Base");
	    levelNames[1]= CREATE_ATOM("Alt");
	}
	map->types[KT_PC_SYSRQ_KEY].lvl_names= levelNames;
    }
#ifdef DEBUG_RADIO_GROUPS
    names->radio_groups= (Atom *)Xcalloc(RG_COUNT*sizeof(Atom));
    if (names->radio_groups) {
	names->num_rg = RG_COUNT;
	names->radio_groups[RG_BOGUS_FUNCTION_GROUP]= CREATE_ATOM("BOGUS");
    }
#endif
    names->num_char_sets= 1;
    names->char_sets= (Atom *)Xcalloc(sizeof(Atom));
    names->char_sets[0]= CREATE_ATOM("iso8859-1");

#ifdef DEBUG_KEY_NAMES
    i= xkb->desc.max_key_code+1;
    names->keys= (XkbKeyNamePtr)Xcalloc(i*sizeof(XkbKeyNameRec));
    if (names->keys) {
	for (i=xkb->desc.min_key_code;i<=xkb->desc.max_key_code;i++) {
	    sprintf(names->keys[i].name,"K%02X",i);
	}
    }
#endif
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
	alt->num_char_sets= 1;
	alt->char_sets= (Atom *)Xcalloc(sizeof(Atom));
	alt->char_sets[0]= CREATE_ATOM("iso8859-1");
	alt->first_key= 27;
	alt->num_keys= 13;
	alt->num_syms= 4;
	alt->syms= (KeySym *)Xcalloc(alt->num_syms*sizeof(KeySym));
	alt->syms[0]= XK_y;
	alt->syms[1]= XK_Y;
	alt->syms[2]= XK_z;
	alt->syms[3]= XK_Z;
	alt->maps= (XkbSymMapRec *)Xcalloc(alt->num_keys*sizeof(XkbSymMapRec));
	alt->maps[0].kt_index= KT_2LEVEL_KEY;
	alt->maps[0].group_info= XkbSetGroupInfo(1,0);
	alt->maps[0].offset= 0;
	alt->maps[12].kt_index= KT_2LEVEL_KEY;
	alt->maps[12].group_info= XkbSetGroupInfo(1,0);
	alt->maps[12].offset= 2;
	alt->next = NULL;
   }
   xkb->desc.alt_syms= alt;
   return;
}

static void
XkbInitIndicatorMap(xkb)
    XkbSrvInfoRec *xkb;
{
XkbIndicatorRec	*map = xkb->desc.indicators;
register int i;

    map->num_phys_indicators = 7;
    xkb->iStateAuto= 0;
    xkb->iStateExplicit= 0;
    xkb->iStateEffective= 0;
    bzero(map->maps,XkbNumIndicators*sizeof(XkbIndicatorMapRec));
    map->maps[LED_CAPS-1].flags= XkbIM_NoExplicit;
    map->maps[LED_CAPS-1].which_mods= XkbIM_UseLocked;
    map->maps[LED_CAPS-1].mask= LockMask;
    map->maps[LED_CAPS-1].real_mods= LockMask;
    xkb->iAccel.usesLocked|= (1<<(LED_CAPS-1));
    xkb->iAccel.haveMap|= (1<<(LED_CAPS-1));

    map->maps[LED_NUM-1].flags= XkbIM_NoExplicit;
    map->maps[LED_NUM-1].which_mods= XkbIM_UseLocked;
    map->maps[LED_NUM-1].mask= 0;
    map->maps[LED_NUM-1].real_mods= 0;
    map->maps[LED_NUM-1].vmods= NUM_LOCK_VMOD_MASK;
    xkb->iAccel.haveMap|= (1<<(LED_NUM-1));

    map->maps[LED_SCROLL-1].which_mods= XkbIM_UseLocked;
    map->maps[LED_SCROLL-1].mask= 0;
    map->maps[LED_SCROLL-1].real_mods= 0;
    map->maps[LED_SCROLL-1].vmods= SCROLL_LOCK_VMOD_MASK;
    xkb->iAccel.haveMap|= (1<<(LED_SCROLL-1));

    xkb->iAccel.usedComponents|= XkbModifierLockMask;
    return;
}

void
XkbInitDevice(pXDev)
    DeviceIntPtr pXDev;
{
int		nKeys,i;
XkbSrvInfoPtr	xkb;
XkbChangesRec	changes;

    /* determine size of fixed-length arrays */
    i= sizeof(XkbBehavior)+sizeof(XkbSymMapRec);
    i+= sizeof(unsigned short);	/* key acts */
    i+= sizeof(unsigned char);	/* explicit components */
    i*= pXDev->key->curKeySyms.maxKeyCode+1;

    pXDev->key->xkbInfo= xkb=(XkbSrvInfoRec *)Xcalloc(sizeof(XkbSrvInfoRec)+i);
    if ( xkb ) {
	xkb->desc.ctrls = &xkb->Controls;
	xkb->desc.map = &xkb->Map;
	xkb->desc.server = &xkb->Server;
	xkb->desc.indicators = &xkb->Indicators;
	xkb->desc.compat = &xkb->Compat;
	xkb->desc.names = &xkb->Names;
	xkb->desc.device_spec = pXDev->id;
	xkb->desc.flags = 1;
	xkb->desc.min_key_code = pXDev->key->curKeySyms.minKeyCode;
	xkb->desc.max_key_code = pXDev->key->curKeySyms.maxKeyCode;

	nKeys = xkb->desc.max_key_code+1;	/* size of fixed arrays */
	xkb->Map.key_sym_map= (XkbSymMapRec *)&xkb[1];
	xkb->Server.behaviors = (XkbBehavior *)&xkb->Map.key_sym_map[nKeys];
       xkb->Server.key_acts=(unsigned short*)&xkb->Server.behaviors[nKeys];
	xkb->Server.explicit=(unsigned char *)&xkb->Server.key_acts[nKeys];

	xkb->dfltPtrDelta=1;
	xkb->interest = NULL;
	xkb->device = pXDev;

	for (i=0;i<XkbNumVirtualMods;i++) { 
	   xkb->desc.server->vmods[i]= XkbNoModifier;
	}
	XkbInitKeyTypes(xkb);
	XkbInitNames(xkb);
	XkbInitKeySyms(xkb);
	XkbInitKeyActions(xkb);
	XkbInitRadioGroups(xkb);
	XkbInitCompatStructs(xkb);

	xkb->state.group = 0;
	xkb->state.latched_group = 0;
	xkb->state.mods = 0;
	xkb->state.latched_mods = 0;
	xkb->state.locked_mods = 0;
	xkb->state.compat_state = 0;

	XkbInitIndicatorMap(xkb);

	xkb->desc.ctrls->num_groups = 1;
	xkb->desc.ctrls->internal_mask = 0;
	xkb->desc.ctrls->internal_real_mods = 0;
	xkb->desc.ctrls->internal_vmods = 0;
	xkb->desc.ctrls->ignore_lock_mask = 0;
	xkb->desc.ctrls->ignore_lock_real_mods = 0;
	xkb->desc.ctrls->ignore_lock_vmods = NUM_LOCK_VMOD_MASK;
	/*
	 * 11/18/93 (ef) -- XXX Don't enable AccessXKeys until we
	 *    have a watchdog program and an easy way to turn it
	 *    off.
	 */
#ifdef NOTYET
	xkb->desc.ctrls->enabled_ctrls = XkbAccessXKeysMask|
					 	XkbAccessXTimeoutMask|
						XkbRepeatKeysMask|
						XkbGroupsWrapMask|
						XkbMouseKeysAccelMask|
						XkbAudibleBellMask|
						XkbAutoAutorepeatMask;
#else
	xkb->desc.ctrls->enabled_ctrls = XkbAccessXTimeoutMask|
						XkbGroupsWrapMask|
						XkbMouseKeysAccelMask|
						XkbAudibleBellMask|
						XkbAutoAutorepeatMask;
#endif
	AccessXInit(pXDev);
	DDXInitXkbDevice(pXDev);

	XkbUpdateKeyTypesFromCore(pXDev,xkb->desc.min_key_code,
					XkbNumKeys(&xkb->desc),&changes);
	XkbUpdateActions(pXDev,xkb->desc.min_key_code,
					XkbNumKeys(&xkb->desc),&changes);
    }
}

	/*
	 * InitKeyClassDeviceStruct initializes the key class before it
	 * initializes the keyboard feedback class for a device. UpdateActions
	 * can't set up the correct autorepeat for keyboard initialization
	 * because the keyboard feedback isn't created yet.   Instead, 
	 * UpdateActions notes the "correct" autorepeat in the SrvInfo
	 * structure and InitKbdFeedbackClass calls UpdateAutorepeat to
	 * apply the computed autorepeat once the feedback class exists.
	 *
	 * DIX will apply the changed autorepeat, so there's no need to
	 * do so here.   This function returns True if both RepeatKeys and
	 * the core protocol autorepeat ctrls are set (i.e. should use 
	 * software autorepeat), false otherwise.
	 */
int
XkbUpdateAutorepeat(pXDev)
    DeviceIntPtr pXDev;
{
XkbSrvInfoRec *xkb;
int	       softRepeat;

    if (pXDev && pXDev->key && pXDev->key->xkbInfo) {
	xkb= pXDev->key->xkbInfo;
	softRepeat= (xkb->desc.ctrls->enabled_ctrls&XkbRepeatKeysMask)!=0;
	if (XkbComputeAutoRepeat && pXDev->kbdfeed) {
	    memcpy((char *)pXDev->kbdfeed->ctrl.autoRepeats,
						(char *)xkb->repeat,32);
	    softRepeat= softRepeat&&pXDev->kbdfeed->ctrl.autoRepeat;
	}
    }
    else softRepeat= 0;
    return softRepeat;
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
    if (xkb->desc.ctrls)
	xkb->desc.ctrls= NULL;
    if (xkb->desc.server) {
	if (xkb->desc.server->acts)
	    Xfree(xkb->desc.server->acts);
	xkb->desc.server->num_acts= xkb->desc.server->size_acts= 0;
	xkb->desc.server->acts= NULL;
	if (xkb->desc.server->behaviors)
	    xkb->desc.server->behaviors= NULL;
	if (xkb->desc.server->key_acts)
	    xkb->desc.server->key_acts= NULL;
	xkb->desc.server= NULL;
    }
    if (xkb->desc.names) {
	if (xkb->desc.names->radio_groups) {
	    Xfree(xkb->desc.names->radio_groups);
	    xkb->desc.names->radio_groups= NULL;
	}
	if (xkb->desc.names->char_sets) {
	    Xfree(xkb->desc.names->char_sets);
	    xkb->desc.names->char_sets= NULL;
	}
	xkb->desc.names= NULL;
    }
    if (xkb->desc.map) {
	if (xkb->desc.map->types) {
	    register unsigned i;
	    XkbKeyTypeRec	*type= xkb->desc.map->types;
	    for (i=0;i<xkb->desc.map->num_types;i++,type++) {
		if (type->map&&(!(type->free&XkbNoFreeKTMap)))
		    Xfree(type->map);
		type->map= NULL;
		if (type->preserve&&(!(type->free&XkbNoFreeKTPreserve)))
		    Xfree(type->preserve);
		type->preserve= NULL;
	    }
	    type= xkb->desc.map->types;
	    if (!(type->free&XkbNoFreeKTStruct)) {
		Xfree(type);
		xkb->desc.map->types= NULL;
	    }
	}
	if (xkb->desc.map->syms) {
	    Xfree(xkb->desc.map->syms);
	    xkb->desc.map->syms= NULL;
	}
	if (xkb->desc.map->key_sym_map) {
	    xkb->desc.map->key_sym_map= NULL;
	}
	xkb->desc.map= NULL;
    }
    if (xkb->desc.indicators) {
	xkb->desc.indicators= NULL;
    }
    if (xkb->desc.compat) {
	if (xkb->desc.compat->sym_interpret)
	    Xfree(xkb->desc.compat->sym_interpret);
	xkb->desc.compat->num_si= 0;
	xkb->desc.compat->sym_interpret= NULL;
	xkb->desc.compat= NULL;
    }
    if (xkb->desc.alt_syms) {
	XkbAlternateSymsRec *this,*next;
	this= xkb->desc.alt_syms;
	next= this->next;
	while (this) {
	    if (this->char_sets)
		Xfree(this->char_sets);
	    this->num_char_sets= 0;
	    this->char_sets= NULL;
	    if (this->syms)
		Xfree(this->syms);
	    this->num_syms= 0;
	    this->syms= NULL;
	    if (this->maps)
		Xfree(this->maps);
	    this->num_keys= 0;
	    this->maps= NULL;
	    Xfree(this);
	    this= next;
	}
	xkb->desc.alt_syms= NULL;
    }
    Xfree(xkb);
    return;
}
