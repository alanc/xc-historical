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

#ifndef _XKB_H_
#define	_XKB_H_

#define	X_KBUseExtension		 0
#define	X_KBSelectEvents	 	 1
#define	X_KBSendEvent			 2
#define	X_KBBell			 3
#define	X_KBGetState			 4
#define	X_KBLatchLockState		 5
#define	X_KBGetControls			 6
#define	X_KBSetControls			 7
#define	X_KBGetMap			 8
#define	X_KBSetMap			 9
#define	X_KBGetCompatMap		10
#define	X_KBSetCompatMap		11
#define	X_KBGetIndicatorState		12
#define	X_KBGetIndicatorMap		13
#define	X_KBSetIndicatorMap		14
#define	X_KBGetNames			15
#define	X_KBSetNames			16
#define	X_KBListAlternateSyms		17
#define	X_KBGetAlternateSyms		18
#define	X_KBSetAlternateSyms		19
#define	X_KBGetGeometry			20
#define	X_KBSetGeometry			21
#define	X_KBSetDebuggingFlags		101

#define	XKBEventCode			0
#define	XKBNumberEvents			(XKBEventCode+1)

#define	XKBStateNotify			0
#define XKBMapNotify			1
#define XKBControlsNotify		2
#define	XKBIndicatorNotify		3
#define	XKBBellNotify			4
#define	XKBSlowKeyNotify		5
#define	XKBNamesNotify			6
#define XKBCompatMapNotify		7
#define	XKBAlternateSymsNotify		8

#define	XKBStateNotifyMask		(1L << 0)
#define XKBMapNotifyMask		(1L << 1)
#define XKBControlsNotifyMask		(1L << 2)
#define	XKBIndicatorNotifyMask		(1L << 3)
#define	XKBBellNotifyMask		(1L << 4)
#define	XKBSlowKeyNotifyMask		(1L << 5)
#define	XKBNamesNotifyMask		(1L << 6)
#define XKBCompatMapNotifyMask		(1L << 7)
#define XKBAlternateSymsNotifyMask	(1L << 8)
#define	XKBAllEventsMask		(0x01FF)

#define	XKBSKPress			0
#define	XKBSKAccept			1
#define	XKBSKReject			2
#define	XKBSKRelease			3

#define	XKBSKPressMask			(1L << 0)
#define	XKBSKAcceptMask			(1L << 1)
#define	XKBSKRejectMask			(1L << 2)
#define	XKBSKReleaseMask		(1L << 3)
#define	XKBSKAllEventsMask		(0xf)

#define	XKB_Keyboard			0
#define	XKBNumberErrors			1

#define	XKBModifierStateMask		(1L << 0)
#define	XKBModifierBaseMask		(1L << 1)
#define	XKBModifierLatchMask		(1L << 2)
#define	XKBModifierLockMask		(1L << 3)
#define	XKBGroupStateMask		(1L << 4)
#define	XKBGroupBaseMask		(1L << 5)
#define	XKBGroupLatchMask		(1L << 6)
#define XKBGroupLockMask		(1L << 7)
#define	XKBCompatStateMask		(1L << 8)
#define	XKBModifierUnlockMask		(1L << 9)
#define	XKBGroupUnlockMask		(1L << 10)
#define	XKBAllStateComponentsMask	(0x7ff)

#define	XKBRepeatKeysMask	 (1L << 0)
#define	XKBSlowKeysMask		 (1L << 1)
#define	XKBBounceKeysMask	 (1L << 2)
#define	XKBStickyKeysMask	 (1L << 3)
#define	XKBMouseKeysMask	 (1L << 4)
#define	XKBMouseKeysAccelMask	 (1L << 5)
#define	XKBAccessXKeysMask	 (1L << 6)
#define	XKBAccessXTimeoutMask	 (1L << 7)
#define	XKBGroupsWrapMask	 (1L << 8)
#define	XKBAudibleBellMask	 (1L << 9)
#define	XKBAutoAutorepeatMask	 (1L << 10)

#define XKBKeyboardGroupsMask	 (1L << 12)
#define	XKBInternalModsMask	 (1L << 13)
#define	XKBIgnoreLockModsMask	 (1L << 14)
#define	XKBControlsEnabledMask	 (1L << 15)

#define	XKBAllAccessXMask	 (0x00FF)
#define	XKBAllControlsMask	 (0xF7FF)

#define	XKB_USE_CORE_KBD	0x0100
#define	XKB_PRIVATE		0x4000

#define	XKB_SA_CLEAR_LOCKS	(1L << 0)
#define	XKB_SA_LATCH_TO_LOCK	(1L << 1)
#define	XKB_SA_GROUP_ABSOLUTE	(1L << 2)
#define	XKB_SA_USE_DFLT_BUTTON	0

#define	XKB_SA_ISO_DFLT_IS_GROUP 	(1L << 7)
#define	XKB_SA_ISO_NO_AFFECT_MODS	(1L << 6)
#define	XKB_SA_ISO_NO_AFFECT_GROUP	(1L << 5)
#define	XKB_SA_ISO_NO_AFFECT_PTR	(1L << 4)
#define	XKB_SA_ISO_NO_AFFECT_CTRLS	(1L << 3)

	/* flags values for XKB_SA_SET_PTR_DFLT */
#define	XKB_SA_SET_DFLT_BTN		1
#define	XKB_SA_INCR_DFLT_BTN		2

#define	XKB_SA_NO_ACTION		0x0000
#define	XKB_SA_SET_MODS			0x0001
#define	XKB_SA_LATCH_MODS		0x0002
#define	XKB_SA_LOCK_MODS		0x0003
#define	XKB_SA_SET_GROUP		0x0004
#define	XKB_SA_LATCH_GROUP		0x0005
#define	XKB_SA_LOCK_GROUP		0x0006
#define	XKB_SA_MOVE_PTR			0x0007
#define	XKB_SA_ACCEL_PTR		0x0008
#define	XKB_SA_PTR_BTN			0x0009
#define	XKB_SA_CLICK_PTR_BTN		0x000a
#define	XKB_SA_LOCK_PTR_BTN		0x000b
#define	XKB_SA_SET_PTR_DFLT		0x000c
#define	XKB_SA_ISO_LOCK			0x000d
#define	XKB_SA_TERMINATE		0x000e
#define	XKB_SA_SWITCH_SCREEN		0x000f
#define	XKB_SA_SET_CONTROLS		0x0010
#define	XKB_SA_LOCK_CONTROLS		0x0011

#define	XKB_KB_DEFAULT		0x0000
#define	XKB_KB_LOCK		0x0001
#define	XKB_KB_RADIO_GROUP	0x0002

#define	XKB_NUM_INDICATORS	32
#define	XKB_NUM_KBD_GROUPS	8
#define	XKB_RG_MAX_MEMBERS	12

#define	XKBNAME "XKEYBOARD"
#define	XKB_MAJOR_VERSION	0
#define	XKB_MINOR_VERSION	21

#define	XKBKeyTypesMask		(1<<0)
#define	XKBKeySymsMask		(1<<1)
#define XKBKeyActionsMask	(1<<8)
#define	XKBKeyBehaviorsMask	(1<<9)

#define	XKBFullClientInfoMask	(XKBKeyTypesMask|XKBKeySymsMask)
#define	XKBFullServerInfoMask	(XKBKeyActionsMask|XKBKeyBehaviorsMask)
#define	XKBAllMapComponentsMask	(XKBFullClientInfoMask|XKBFullServerInfoMask)
#define	XKBResizableInfoMask	(XKBKeyTypesMask)

#define	XKB_SI_NoIndicator	0xff
#define	XKB_SI_Autorepeat	(1<<0)
#define	XKB_SI_UpdateGroup	(1<<1)
#define	XKB_SI_UpdateKeypad	(1<<2)
#define	XKB_SI_UseModMapMods	(1<<3)
#define	XKB_SI_UpdateInternal	(1<<4)
#define	XKB_SI_UpdateIgnoreLocks (1<<5)

#define	XKB_SI_NoneOf		(0)
#define	XKB_SI_AnyOfOrNone	(1)
#define	XKB_SI_AnyOf		(2)
#define	XKB_SI_AllOf		(3)
#define	XKB_SI_Exactly		(4)

#define	XKB_IMUseBase		(1L << 0)
#define	XKB_IMUseLatched	(1L << 1)
#define	XKB_IMUseLocked		(1L << 2)
#define	XKB_IMUseEffectiveLocked (1L << 3)
#define	XKB_IMUseEffective	(1L << 4)
#define	XKB_IMUseCompat		(1L << 5)
#define	XKB_IMUseAnyState	(0xF)

#define	XKBKeycodesNameMask	(1<<0)
#define	XKBGeometryNameMask	(1<<1)
#define	XKBSymbolsNameMask	(1<<2)
#define	XKBKeyTypeNamesMask	(1<<3)
#define	XKBKTLevelNamesMask	(1<<4)
#define	XKBRGNamesMask		(1<<5)
#define	XKBIndicatorNamesMask	(1<<6)
#define	XKBModifierNamesMask	(1<<7)
#define	XKBCharSetsMask		(1<<8)
#define	XKBAllNamesMask		(0x1ff)

#endif /* _XKB_H_ */
