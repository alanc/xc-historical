/* $XConsortium: Xkb.h,v 1.1 93/09/26 21:12:18 rws Exp $ */
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

#define	XkbEventCode			0
#define	XkbNumberEvents			(XkbEventCode+1)

#define	XkbStateNotify			0
#define XkbMapNotify			1
#define XkbControlsNotify		2
#define	XkbIndicatorNotify		3
#define	XkbBellNotify			4
#define	XkbSlowKeyNotify		5
#define	XkbNamesNotify			6
#define XkbCompatMapNotify		7
#define	XkbAlternateSymsNotify		8

#define	XkbStateNotifyMask		(1L << 0)
#define XkbMapNotifyMask		(1L << 1)
#define XkbControlsNotifyMask		(1L << 2)
#define	XkbIndicatorNotifyMask		(1L << 3)
#define	XkbBellNotifyMask		(1L << 4)
#define	XkbSlowKeyNotifyMask		(1L << 5)
#define	XkbNamesNotifyMask		(1L << 6)
#define XkbCompatMapNotifyMask		(1L << 7)
#define XkbAlternateSymsNotifyMask	(1L << 8)
#define	XkbAllEventsMask		(0x01FF)

#define	XkbSKPress			0
#define	XkbSKAccept			1
#define	XkbSKReject			2
#define	XkbSKRelease			3

#define	XkbSKPressMask			(1L << 0)
#define	XkbSKAcceptMask			(1L << 1)
#define	XkbSKRejectMask			(1L << 2)
#define	XkbSKReleaseMask		(1L << 3)
#define	XkbSKAllEventsMask		(0xf)

#define	XKB_Keyboard			0
#define	XkbNumberErrors			1

#define	XkbModifierStateMask		(1L << 0)
#define	XkbModifierBaseMask		(1L << 1)
#define	XkbModifierLatchMask		(1L << 2)
#define	XkbModifierLockMask		(1L << 3)
#define	XkbGroupStateMask		(1L << 4)
#define	XkbGroupBaseMask		(1L << 5)
#define	XkbGroupLatchMask		(1L << 6)
#define XkbGroupLockMask		(1L << 7)
#define	XkbCompatStateMask		(1L << 8)
#define	XkbModifierUnlockMask		(1L << 9)
#define	XkbGroupUnlockMask		(1L << 10)
#define	XkbAllStateComponentsMask	(0x7ff)

#define	XkbRepeatKeysMask	 (1L << 0)
#define	XkbSlowKeysMask		 (1L << 1)
#define	XkbBounceKeysMask	 (1L << 2)
#define	XkbStickyKeysMask	 (1L << 3)
#define	XkbMouseKeysMask	 (1L << 4)
#define	XkbMouseKeysAccelMask	 (1L << 5)
#define	XkbAccessXKeysMask	 (1L << 6)
#define	XkbAccessXTimeoutMask	 (1L << 7)
#define	XkbGroupsWrapMask	 (1L << 8)
#define	XkbAudibleBellMask	 (1L << 9)
#define	XkbAutoAutorepeatMask	 (1L << 10)

#define XkbKeyboardGroupsMask	 (1L << 12)
#define	XkbInternalModsMask	 (1L << 13)
#define	XkbIgnoreLockModsMask	 (1L << 14)
#define	XkbControlsEnabledMask	 (1L << 15)

#define	XkbAllAccessXMask	 (0x00FF)
#define	XkbAllControlsMask	 (0xF7FF)

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

#define	XkbKeyTypesMask		(1<<0)
#define	XkbKeySymsMask		(1<<1)
#define XkbKeyActionsMask	(1<<8)
#define	XkbKeyBehaviorsMask	(1<<9)

#define	XkbFullClientInfoMask	(XkbKeyTypesMask|XkbKeySymsMask)
#define	XkbFullServerInfoMask	(XkbKeyActionsMask|XkbKeyBehaviorsMask)
#define	XkbAllMapComponentsMask	(XkbFullClientInfoMask|XkbFullServerInfoMask)
#define	XkbResizableInfoMask	(XkbKeyTypesMask)

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

#define	XkbKeycodesNameMask	(1<<0)
#define	XkbGeometryNameMask	(1<<1)
#define	XkbSymbolsNameMask	(1<<2)
#define	XkbKeyTypeNamesMask	(1<<3)
#define	XkbKTLevelNamesMask	(1<<4)
#define	XkbRGNamesMask		(1<<5)
#define	XkbIndicatorNamesMask	(1<<6)
#define	XkbModifierNamesMask	(1<<7)
#define	XkbCharSetsMask		(1<<8)
#define	XkbAllNamesMask		(0x1ff)

#endif /* _XKB_H_ */
