/* $XConsortium: XKB.h,v 1.3 93/09/27 23:58:32 rws Exp $ */
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

#define	X_kbUseExtension		 0
#define	X_kbSelectEvents	 	 1
#define	X_kbSendEvent			 2
#define	X_kbBell			 3
#define	X_kbGetState			 4
#define	X_kbLatchLockState		 5
#define	X_kbGetControls			 6
#define	X_kbSetControls			 7
#define	X_kbGetMap			 8
#define	X_kbSetMap			 9
#define	X_kbGetCompatMap		10
#define	X_kbSetCompatMap		11
#define	X_kbGetIndicatorState		12
#define	X_kbGetIndicatorMap		13
#define	X_kbSetIndicatorMap		14
#define	X_kbGetNames			15
#define	X_kbSetNames			16
#define	X_kbListAlternateSyms		17
#define	X_kbGetAlternateSyms		18
#define	X_kbSetAlternateSyms		19
#define	X_kbGetGeometry			20
#define	X_kbSetGeometry			21
#define	X_kbSetDebuggingFlags		101

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

#define	XkbKeyboard			0
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

#define	XkbUseCoreKbd	0x0100
#define	XkbPrivate		0x4000

#define	XkbSAClearLocks	(1L << 0)
#define	XkbSALatchToLock	(1L << 1)
#define	XkbSAGroupAbsolute	(1L << 2)
#define	XkbSAUseDfltButton	0

#define	XkbSAISODfltIsGroup 	(1L << 7)
#define	XkbSAISONoAffectMods	(1L << 6)
#define	XkbSAISONoAffectGroup	(1L << 5)
#define	XkbSAISONoAffectPtr	(1L << 4)
#define	XkbSAISONoAffectCtrls	(1L << 3)

	/* flags values for XkbSASetPtrDflt */
#define	XkbSASetDfltBtn		1
#define	XkbSAIncrDfltBtn		2

#define	XkbSANoAction		0x0000
#define	XkbSASetMods			0x0001
#define	XkbSALatchMods		0x0002
#define	XkbSALockMods		0x0003
#define	XkbSASetGroup		0x0004
#define	XkbSALatchGroup		0x0005
#define	XkbSALockGroup		0x0006
#define	XkbSAMovePtrBtn			0x0007
#define	XkbSAAccelPtr		0x0008
#define	XkbSAPtrBtn			0x0009
#define	XkbSAClickPtrBtn		0x000a
#define	XkbSALockPtrBtn		0x000b
#define	XkbSASetPtrDflt		0x000c
#define	XkbSAISOLock			0x000d
#define	XkbSATerminate		0x000e
#define	XkbSASwitchScreen		0x000f
#define	XkbSASetControls		0x0010
#define	XkbSALockControls		0x0011

#define	XkbDefaultKB		0x0000
#define	XkbLockKB		0x0001
#define	XkbRadioGroupKB	0x0002

#define	XkbNumIndicators	32
#define	XkbNumKbdGroups	8
#define	XkbRGMaxMembers	12

#define	XkbName "XKEYBOARD"
#define	XkbMajorVersion	0
#define	XkbMinorVersion	21

#define	XkbKeyTypesMask		(1<<0)
#define	XkbKeySymsMask		(1<<1)
#define XkbKeyActionsMask	(1<<8)
#define	XkbKeyBehaviorsMask	(1<<9)

#define	XkbFullClientInfoMask	(XkbKeyTypesMask|XkbKeySymsMask)
#define	XkbFullServerInfoMask	(XkbKeyActionsMask|XkbKeyBehaviorsMask)
#define	XkbAllMapComponentsMask	(XkbFullClientInfoMask|XkbFullServerInfoMask)
#define	XkbResizableInfoMask	(XkbKeyTypesMask)

#define	XkbSI_NoIndicator	0xff
#define	XkbSI_Autorepeat	(1<<0)
#define	XkbSI_UpdateGroup	(1<<1)
#define	XkbSI_UpdateKeypad	(1<<2)
#define	XkbSI_UseModMapMods	(1<<3)
#define	XkbSI_UpdateInternal	(1<<4)
#define	XkbSI_UpdateIgnoreLocks (1<<5)

#define	XkbSI_NoneOf		(0)
#define	XkbSI_AnyOfOrNone	(1)
#define	XkbSI_AnyOf		(2)
#define	XkbSI_AllOf		(3)
#define	XkbSI_Exactly		(4)

#define	XkbIMUseBase		(1L << 0)
#define	XkbIMUseLatched	(1L << 1)
#define	XkbIMUseLocked		(1L << 2)
#define	XkbIMUseEffectiveLocked (1L << 3)
#define	XkbIMUseEffective	(1L << 4)
#define	XkbIMUseCompat		(1L << 5)
#define	XkbIMUseAnyState	(0xF)

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
