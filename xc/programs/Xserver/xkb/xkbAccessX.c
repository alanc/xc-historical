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
#include "inputstr.h"
#include "XKBsrv.h"

#include <sys/time.h>

void
AccessXInit(keybd)
    DeviceIntPtr keybd;
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    xkbInfo->shiftKeyCount= 0;
    xkbInfo->mouseKeysCounter= 0;
    xkbInfo->inactiveKey= 0;
    xkbInfo->slowKey= 0;
    xkbInfo->repeatKey= 0;
    xkbInfo->krgTimerActive= _OFF_TIMER;
    xkbInfo->accessXFlags= 0;
    xkbInfo->mouseKeyTimer= NULL;
    xkbInfo->slowKeysTimer= NULL;
    xkbInfo->bounceKeysTimer= NULL;
    xkbInfo->repeatKeyTimer= NULL;
    xkbInfo->krgTimer= NULL;
    xkbInfo->accessXFlags= 0;
    ctrls->repeatDelay = 660;
    ctrls->repeatInterval = 40;
    ctrls->debounceDelay = 300;
    ctrls->slowKeysDelay = 300;
    ctrls->mouseKeysDelay = 160;
    ctrls->mouseKeysInterval = 40;
    ctrls->mouseKeysTimeToMax = 30;
    ctrls->mouseKeysMaxSpeed = 30;
    ctrls->mouseKeysCurve = 500;
    ctrls->mouseKeysDfltBtn = 1;
    ctrls->accessXTimeout = 120;
    ctrls->accessXTimeoutMask = KRG_MASK|XKBStickyKeysMask ;
    xkbInfo->mouseKeysCurve= 1.0+(((double)ctrls->mouseKeysCurve)*0.001);
    xkbInfo->mouseKeysCurveFactor= ( ((double)ctrls->mouseKeysMaxSpeed)/
		pow((double)ctrls->mouseKeysTimeToMax,xkbInfo->mouseKeysCurve));
    return;
}

/************************************************************************/
/*									*/
/* AccessXKeyboardEvent							*/
/*									*/
/*	Generate a synthetic keyboard event.				*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
static void AccessXKeyboardEvent(DeviceIntPtr	keybd,
				 BYTE		type,
				 BYTE		keyCode)
#else
static void AccessXKeyboardEvent(keybd,type,keyCode)
    DeviceIntPtr keybd;
    BYTE	 type;
    BYTE	 keyCode;
#endif
{
    xEvent	xE;
    
    xE.u.u.type = type;
    xE.u.u.detail = keyCode;
    xE.u.keyButtonPointer.time = GetTimeInMillis();	    

    XKBProcessKeyboardEvent(&xE,keybd,1L);

    return;
    
} /* AccessXKeyboardEvent */

/************************************************************************/
/*									*/
/* AccessXKRGTurnOn							*/
/*									*/
/*	Turn the keyboard response group on.				*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
static void AccessXKRGTurnOn(DeviceIntPtr	 keybd,
			     CARD16 	  	 KRGControl,
			     xKBControlsNotify	*pCN)
#else
static void AccessXKRGTurnOn(keybd,KRGControl,pCN)
    DeviceIntPtr	keybd;
    CARD16 		KRGControl;
    xKBControlsNotify	*pCN;
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    XKBControlsRec	old= *ctrls;

    ctrls->enabledControls |= (KRGControl&KRG_MASK);
    if (XKBComputeControlsNotify(keybd,&old,ctrls,pCN))
	XKBSendControlsNotify(keybd,pCN);
    if (xkbInfo->iAccel.usesControls)
	XKBUpdateIndicators(keybd,xkbInfo->iAccel.usesControls,NULL);
    return;
    
} /* AccessXKRGTurnOn */

/************************************************************************/
/*									*/
/* AccessXKRGTurnOff							*/
/*									*/
/*	Turn the keyboard response group off.				*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
static void AccessXKRGTurnOff(DeviceIntPtr keybd,xKBControlsNotify *pCN)
#else
static void AccessXKRGTurnOff(keybd,pCN)
    DeviceIntPtr keybd;
    xKBControlsNotify *pCN;
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    XKBControlsRec	 old = *ctrls;

    ctrls->enabledControls &= ~KRG_MASK;
    if (XKBComputeControlsNotify(keybd,&old,ctrls,pCN))
	XKBSendControlsNotify(keybd,pCN);
    if (xkbInfo->iAccel.usesControls)
	XKBUpdateIndicators(keybd,xkbInfo->iAccel.usesControls,NULL);
    return;
    
} /* AccessXKRGTurnOff */

/************************************************************************/
/*									*/
/* AccessXStickyKeysTurnOn						*/
/*									*/
/*	Turn StickyKeys on.						*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
static void AccessXStickyKeysTurnOn(DeviceIntPtr keybd,xKBControlsNotify *pCN)
#else
static void AccessXStickyKeysTurnOn(keybd,pCN)
    DeviceIntPtr	 keybd;
    xKBControlsNotify	*pCN;
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    XKBControlsRec	 old = *ctrls;
    
    ctrls->enabledControls |= XKBStickyKeysMask;
    xkbInfo->shiftKeyCount = 0;
    if (XKBComputeControlsNotify(keybd,&old,ctrls,pCN)) 
	XKBSendControlsNotify(keybd,pCN);
    if (xkbInfo->iAccel.usesControls)
	XKBUpdateIndicators(keybd,xkbInfo->iAccel.usesControls,NULL);
    return;
    
} /* AccessXStickyKeysTurnOn */

/************************************************************************/
/*									*/
/* AccessXStickyKeysTurnOff						*/
/*									*/
/*	Turn StickyKeys off.						*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
static void AccessXStickyKeysTurnOff(DeviceIntPtr keybd,xKBControlsNotify *pCN)
#else
static void AccessXStickyKeysTurnOff(keybd,pCN)
    DeviceIntPtr	 keybd;
    xKBControlsNotify	*pCN;
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    XKBControlsRec	 old = *ctrls;
    unsigned ledUpdate;

    ctrls->enabledControls &= ~XKBStickyKeysMask;
    xkbInfo->shiftKeyCount = 0;
    if (XKBComputeControlsNotify(keybd,&old,ctrls,pCN))
	XKBSendControlsNotify(keybd,pCN);

    if (xkbInfo->state.lockedMods || xkbInfo->state.lockedGroup) {
	xkbInfo->state.lockedMods= 0;
	xkbInfo->state.lockedGroup= 0;
	ledUpdate= XKBIndicatorsToUpdate(keybd,
					(XKBModifierLockMask|XKBGroupLockMask));
	ledUpdate|= xkbInfo->iAccel.usesControls;
    }
    else ledUpdate= xkbInfo->iAccel.usesControls;
    if (ledUpdate)
	XKBUpdateIndicators(keybd,ledUpdate,NULL);
    return;
    
} /* AccessXStickyKeysTurnOff */

/************************************************************************/
/*									*/
/* AccessXTwoKeysDown							*/
/*									*/
/*	Turn StickyKeys Off if the user pressed a modifier key and	*/
/*	another key at the same time.					*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
void AccessXTwoKeysDown(DeviceIntPtr keybd,xKBControlsNotify *pCN)
#else
void AccessXTwoKeysDown(keybd,pCN)
    DeviceIntPtr	 keybd;
    xKBControlsNotify	*pCN;
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    
    if (xkbInfo->accessXFlags & TWO_KEYS_MASK) {
	/* unlatch and unlock locked modifiers if this option is called. */
	AccessXStickyKeysTurnOff(keybd,pCN);
    }
    
} /* AccessXTwoKeysDown */

static CARD32
AccessXKRGExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
XKBSrvInfoRec	*xkbInfo= ((DeviceIntPtr)arg)->key->xkbInfo;
xKBControlsNotify	cn;

    cn.keycode = 0;
    cn.eventType = 0;
    cn.requestMajor = 0;
    cn.requestMinor = 0;
    if (xkbInfo->desc.controls->enabledControls&XKBSlowKeysMask)
	 AccessXKRGTurnOff((DeviceIntPtr)arg,&cn);
    else AccessXKRGTurnOn((DeviceIntPtr)arg,XKBSlowKeysMask,&cn);
    xkbInfo->krgTimerActive= _OFF_TIMER;
    return 0;
}

static CARD32
AccessXSlowKeyExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
XKBSrvInfoRec	*xkbInfo= ((DeviceIntPtr)arg)->key->xkbInfo;

    if (xkbInfo->slowKey!=0) {
	xKBSlowKeyNotify ev;
	ev.slowKeyType= XKBSKAccept;
	ev.keycode= xkbInfo->slowKey;
	ev.delay= xkbInfo->desc.controls->slowKeysDelay;
	XKBSendSlowKeyNotify((DeviceIntPtr)arg,&ev);
	AccessXKeyboardEvent((DeviceIntPtr)arg,KeyPress,xkbInfo->slowKey);
    }
    return 0;
}

static CARD32
AccessXRepeatKeyExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
XKBSrvInfoRec	*xkbInfo= ((DeviceIntPtr)arg)->key->xkbInfo;
    if (xkbInfo->repeatKey!=0) {
	AccessXKeyboardEvent((DeviceIntPtr)arg,KeyRelease,xkbInfo->repeatKey);
	AccessXKeyboardEvent((DeviceIntPtr)arg,KeyPress,xkbInfo->repeatKey);
	return xkbInfo->desc.controls->repeatInterval;
    }
    return 0;
}

static CARD32
AccessXBounceKeyExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
XKBSrvInfoRec	*xkbInfo= ((DeviceIntPtr)arg)->key->xkbInfo;

    xkbInfo->inactiveKey= 0;
    return 0;
}

static CARD32
AccessXTimeoutExpire(timer,now,arg)
    OsTimerPtr	 timer;
    CARD32	 now;
    pointer	 arg;
{
DeviceIntPtr	 keybd = (DeviceIntPtr)arg;
XKBSrvInfoRec	*xkbInfo= keybd->key->xkbInfo;
XKBControlsRec	*ctrls= xkbInfo->desc.controls;
XKBControlsRec	 old= *ctrls;
xKBControlsNotify	cn;

    if (xkbInfo->lastPtrEventTime) {
	unsigned timeLeft;
	timeLeft= (xkbInfo->desc.controls->accessXTimeout*1000);
	timeLeft-= (now-xkbInfo->lastPtrEventTime);
	return timeLeft;
    }
    ctrls->enabledControls&= ~(xkbInfo->desc.controls->accessXTimeoutMask);
    if (XKBComputeControlsNotify(keybd,&old,ctrls,&cn)) {
	cn.keycode = 0;
	cn.eventType = 0;
	cn.requestMajor = 0;
	cn.requestMinor = 0;
	XKBSendControlsNotify(keybd,&cn);
    }
    if (xkbInfo->iAccel.usesControls)
	XKBUpdateIndicators(keybd,xkbInfo->iAccel.usesControls,NULL);
    xkbInfo->krgTimerActive= _OFF_TIMER;
    return 0;
}


/************************************************************************/
/*									*/
/* AccessXFilterPressEvent						*/
/*									*/
/* Filter events before they get any further if SlowKeys is turned on.	*/
/* In addition, this routine handles the ever so popular magic key	*/
/* actions for turning various accessibility features on/off.		*/
/*									*/
/* Returns TRUE if this routine has discarded the event.		*/
/* Returns FALSE if the event needs further processing.			*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
Bool AccessXFilterPressEvent(register xEvent 		*xE, 
				    register DeviceIntPtr	keybd, 
				    int				count)
#else
Bool AccessXFilterPressEvent(xE,keybd,count)
    register xEvent 		*xE;
    register DeviceIntPtr	keybd;
    int				count;    
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    Bool		 ignoreKeyEvent = FALSE;
    KeyCode		 key = xE->u.u.detail;
    KeySym		*sym = XKBKeySymsPtr(&xkbInfo->desc,key);

    if (ctrls->enabledControls&XKBAccessXKeysMask) {
	/* check for magic sequences */
	if (sym[0]==XK_Shift_R) {
	    xkbInfo->krgTimerActive = _KRG_TIMER;
	    xkbInfo->krgTimer= TimerSet(xkbInfo->krgTimer, 0,4000,
					AccessXKRGExpire, (pointer)keybd);
	    xkbInfo->shiftKeyCount++;
	}
	else {
	    if (xkbInfo->krgTimerActive) {
		xkbInfo->krgTimer= TimerSet(xkbInfo->krgTimer,0, 0, NULL, NULL);
		xkbInfo->krgTimerActive= _OFF_TIMER;
	    }
	    if (sym[0]==XK_Shift_L)
		xkbInfo->shiftKeyCount++;
	}
    }
	
    /* Don't transmit the KeyPress if SlowKeys is turned on;
     * The wakeup handler will synthesize one for us if the user
     * has held the key long enough.
     */
    if (ctrls->enabledControls & XKBSlowKeysMask) {
	xKBSlowKeyNotify	ev;
	ev.slowKeyType= XKBSKPress;
	ev.keycode= key;
	ev.delay= xkbInfo->desc.controls->slowKeysDelay;
	XKBSendSlowKeyNotify(keybd,&ev);
	xkbInfo->slowKey= key;
	xkbInfo->slowKeysTimer = TimerSet(xkbInfo->slowKeysTimer,
				 0, xkbInfo->desc.controls->slowKeysDelay,
				 AccessXSlowKeyExpire, (pointer)keybd);
	ignoreKeyEvent = TRUE;
    }

    /* Don't transmit the KeyPress if BounceKeys is turned on
     * and the user pressed the same key within a given time period
     * from the last release.
     */
    else if ((ctrls->enabledControls & XKBBounceKeysMask) && 
					(key == xkbInfo->inactiveKey)) {
	ignoreKeyEvent = TRUE;
    }

    /* Start repeating if necessary.  Stop autorepeating if the user
     * presses a non-modifier key that doesn't autorepeat.
     */
    if ((keybd->kbdfeed->ctrl.autoRepeat) &&
	((ctrls->enabledControls&(XKBSlowKeysMask|XKBRepeatKeysMask))==
							XKBRepeatKeysMask)) 
    {
#ifndef AIXV3
	if (BitIsOn(keybd->kbdfeed->ctrl.autoRepeats,key))
#endif
	{
	    xkbInfo->repeatKey = key;
	    xkbInfo->repeatKeyTimer= TimerSet(xkbInfo->repeatKeyTimer,
					0, xkbInfo->desc.controls->repeatDelay,
					AccessXRepeatKeyExpire, (pointer)keybd);
	}
    }
    
    /* Check for two keys being pressed at the same time.  This section
     * essentially says the following:
     *
     *	If StickyKeys is on, and a modifier is currently being held down,
     *  and one of the following is true:  the current key is not a modifier
     *  or the currentKey is a modifier, but not the only modifier being
     *  held down, turn StickyKeys off if the TwoKeys off control is set.
     */
    if ((ctrls->enabledControls & XKBStickyKeysMask) && 
	(xkbInfo->state.baseMods!=0)) {
	xKBControlsNotify cn;
	cn.keycode = key;
	cn.eventType = KeyPress;
	cn.requestMajor = 0;
	cn.requestMinor = 0;
	AccessXTwoKeysDown(keybd,&cn);
    }
    
    if (!ignoreKeyEvent)
	XKBProcessKeyboardEvent(xE,keybd,count);
    return ignoreKeyEvent;
} /* AccessXFilterPressEvent */

/************************************************************************/
/*									*/
/* AccessXFilterReleaseEvent						*/
/*									*/
/* Filter events before they get any further if SlowKeys is turned on.	*/
/* In addition, this routine handles the ever so popular magic key	*/
/* actions for turning various accessibility features on/off.		*/
/*									*/
/* Returns TRUE if this routine has discarded the event.		*/
/* Returns FALSE if the event needs further processing.			*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
Bool AccessXFilterReleaseEvent(register xEvent 		*xE, 
				      register DeviceIntPtr	keybd, 
				      int			count)
#else
Bool AccessXFilterReleaseEvent(xE,keybd,count)
    register xEvent 		*xE;
    register DeviceIntPtr	keybd;
    int				count;    
#endif
{
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;
    XKBControlsRec	*ctrls = xkbInfo->desc.controls;
    KeyCode		 key = xE->u.u.detail;
    Bool		 ignoreKeyEvent = FALSE;
    
    /* Don't transmit the KeyRelease if BounceKeys is on and
     * this is the release of a key that was ignored due to 
     * BounceKeys.
     */
    if (ctrls->enabledControls & XKBBounceKeysMask) {
	if (!BitIsOn(keybd->key->down,key))
	    ignoreKeyEvent = TRUE;
	xkbInfo->inactiveKey= key;
	xkbInfo->bounceKeysTimer= TimerSet(xkbInfo->bounceKeysTimer, 0,
					xkbInfo->desc.controls->debounceDelay,
					AccessXBounceKeyExpire, (pointer)keybd);
    }

    /* Don't transmit the KeyRelease if SlowKeys is turned on and
     * the user didn't hold the key long enough.  We know we passed
     * the key if the down bit was set by CoreProcessKeyboadEvent.
     */
    if (ctrls->enabledControls & XKBSlowKeysMask) {
	xKBSlowKeyNotify	ev;
	ev.keycode= key;
	ev.delay= xkbInfo->desc.controls->slowKeysDelay;
	if (BitIsOn(keybd->key->down,key)) {
	    ev.slowKeyType= XKBSKRelease;
	}
	else {
	    ev.slowKeyType= XKBSKReject;
	    ignoreKeyEvent = TRUE;
	}
	XKBSendSlowKeyNotify(keybd,&ev);
	if (xkbInfo->slowKey==key)
	    xkbInfo->slowKey= 0;
    }

    /* Stop Repeating if the user releases the key that is currently
     * repeating.
     */
    if ((ctrls->enabledControls&XKBRepeatKeysMask)&&(xkbInfo->repeatKey==key)) {
	xkbInfo->repeatKey= 0;
    }

    if ((ctrls->enabledControls&XKBAccessXTimeoutMask)&&
		(ctrls->accessXTimeout>0)&&
		(ctrls->enabledControls&ctrls->accessXTimeoutMask)) {
	xkbInfo->lastPtrEventTime= 0;
	xkbInfo->krgTimer= TimerSet(xkbInfo->krgTimer, 0, 
					ctrls->accessXTimeout*1000,
					AccessXTimeoutExpire, (pointer)keybd);
	xkbInfo->krgTimerActive= _TIMEOUT_TIMER;
    }
    else if (xkbInfo->krgTimerActive) {
	xkbInfo->krgTimer= TimerSet(xkbInfo->krgTimer, 0, 0, NULL, NULL);
	xkbInfo->krgTimerActive= _OFF_TIMER;
    }
	
    /* Keep track of how many times the Shift key has been pressed.
     * If it has been pressed and released 5 times in a row, toggle
     * the state of StickyKeys.
     */
    if ((!ignoreKeyEvent)&&(xkbInfo->shiftKeyCount)) {
	KeySym *pSym= XKBKeySymsPtr(&xkbInfo->desc,key);
	if ((pSym[0]!=XK_Shift_L)&&(pSym[0]!=XK_Shift_R)) {
	    xkbInfo->shiftKeyCount= 0;
	}
	else if (xkbInfo->shiftKeyCount>=5) {
	     xKBControlsNotify cn;
	     cn.keycode = key;
	     cn.eventType = KeyPress;
	     cn.requestMajor = 0;
	     cn.requestMinor = 0;
	     if (ctrls->enabledControls & XKBStickyKeysMask)
		AccessXStickyKeysTurnOff(keybd,&cn);
	     else
		AccessXStickyKeysTurnOn(keybd,&cn);
	     xkbInfo->shiftKeyCount= 0;
	}
    }
    
    if (!ignoreKeyEvent)
	XKBProcessKeyboardEvent(xE,keybd,count);
    return ignoreKeyEvent;
    
} /* AccessXFilterReleaseEvent */

/************************************************************************/
/*									*/
/* ProcessPointerEvent							*/
/*									*/
/* This routine merely sets the shiftKeyCount and clears the keyboard   */
/* response group timer (if necessary) on a mouse event.  This is so	*/
/* multiple shifts with just the mouse and shift-drags with the mouse	*/
/* don't accidentally turn on StickyKeys or the Keyboard Response Group.*/
/*									*/
/************************************************************************/
#if NeedFunctionPrototypes
void ProcessPointerEvent(register xEvent 	*xE, 
			 register DeviceIntPtr	mouse, 
			 int		        count)
#else
void ProcessPointerEvent(xE,mouse,count)
    register xEvent 		*xE;
    register DeviceIntPtr	mouse;
    int		        	count;
#endif
{
    DeviceIntPtr ptr = (DeviceIntPtr)LookupPointerDevice();
    DeviceIntPtr keybd = (DeviceIntPtr)LookupKeyboardDevice();
    XKBSrvInfoRec	*xkbInfo = keybd->key->xkbInfo;

    xkbInfo->shiftKeyCount = 0;
    if (xkbInfo->krgTimerActive==_KRG_TIMER) {
	xkbInfo->krgTimer= TimerSet(xkbInfo->krgTimer, 0,
				xkbInfo->desc.controls->accessXTimeout*1000,
				AccessXTimeoutExpire, (pointer)keybd);
	xkbInfo->krgTimerActive== _OFF_TIMER;
    }
    else xkbInfo->lastPtrEventTime= GetTimeInMillis();
    if (xE->u.u.type==ButtonPress) {
	if (ptr->button->down[xE->u.u.detail>>3]&(1<<(xE->u.u.detail&0x7)))
	    return;
    }
    else if (xE->u.u.type==ButtonRelease) {
	if ((ptr->button->down[xE->u.u.detail>>3]&(1<<(xE->u.u.detail&0x7)))==0)
	    return;
	xkbInfo->lockedPtrButtons&= ~(1<<(xE->u.u.detail&0x7));
    }
    CoreProcessPointerEvent(xE,mouse,count);

} /* ProcessPointerEvent */

