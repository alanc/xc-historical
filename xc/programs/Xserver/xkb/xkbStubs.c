/* $XConsortium: xkbStubs.c,v 1.1 93/09/26 21:12:52 rws Exp $ */
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
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "inputstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "XKBsrv.h"

#ifdef sgi
#define	NEED_UPDATE_INDICATORS		0
#endif

#ifndef NEED_UPDATE_INDICATORS
#define	NEED_UPDATE_INDICATORS		1
#endif
#ifndef NEED_FAKE_POINTER_MOTION
#define	NEED_FAKE_POINTER_MOTION	1
#endif
#ifndef NEED_FAKE_POINTER_BUTTON
#define	NEED_FAKE_POINTER_BUTTON	1
#endif
#ifndef NEED_CHANGE_XKB_CONTROLS
#define	NEED_CHANGE_XKB_CONTROLS	1
#endif
#ifndef NEED_TERMINATE_SERVER
#define NEED_TERMINATE_SERVER		1
#endif
#ifndef NEED_SWITCH_VIRTUAL_SCREEN
#define NEED_SWITCH_VIRTUAL_SCREEN	1
#endif

#if NEED_UPDATE_INDICATORS
void
DDXUpdateIndicators(pXDev,old,new)
    DeviceIntPtr  pXDev;
    CARD32 old;
    CARD32 new;
{
    if (pXDev->kbdfeed) {
	pXDev->kbdfeed->ctrl.leds= new;
	if (pXDev->kbdfeed->CtrlProc)
	    (*pXDev->kbdfeed->CtrlProc)(pXDev,&pXDev->kbdfeed->ctrl);
    }
    return;
}
#endif

#if NEED_FAKE_POINTER_MOTION
void
DDXFakePointerMotion(dx,dy)
    int dx;
    int dy;
{
extern	WindowPtr GetSpriteWindow();
ScreenPtr	   pScreen;
int 		   oldX,oldY;
xEvent	ev;

    pScreen= GetSpriteWindow()->drawable.pScreen;
    GetSpritePosition(&oldX,&oldY);
    oldX+= dx;
    oldY+= dy;
    if (oldX<0)				oldX= 0;
    else if (oldX>=pScreen->width)	oldX= pScreen->width-1;
    if (oldY<0)				oldY= 0;
    else if (oldY>=pScreen->height)	oldY= pScreen->height-1;

    if (pScreen->SetCursorPosition)
	(*pScreen->SetCursorPosition)(pScreen, oldX, oldY, TRUE);
    return;
}
#endif

#if NEED_FAKE_POINTER_BUTTON
void
DDXFakePointerButton(event,button)
    int event;
    int button;
{
xEvent	ev;
int	x,y;
DevicePtr ptr;

    if ((ptr = LookupPointerDevice())==NULL)
	return;
    GetSpritePosition(&x,&y);
    ev.u.u.type = event;
    ev.u.u.detail = button;
    ev.u.keyButtonPointer.time = GetTimeInMillis();
    ev.u.keyButtonPointer.rootX = x;
    ev.u.keyButtonPointer.rootY = y;
    (*ptr->processInputProc)( &ev, ptr, 1 );
    return;
}
#endif

#if NEED_CHANGE_XKB_CONTROLS
void
DDXChangeXKBControls(dev,old,new)
    DeviceIntPtr    dev;
    XKBControlsRec *old;
    XKBControlsRec *new;
{
    /* 8/11/93 (ef) -- XXX! IMPLEMENT THIS */
}
#endif

#if NEED_TERMINATE_SERVER
int
DDXTerminateServer(dev,key,action)
    DeviceIntPtr  dev;
    KeyCode	  key;
    XKBAction	 *action;
{
    GiveUp(1);
    return 0;
}
#endif

#if NEED_SWITCH_VIRTUAL_SCREEN
int
DDXSwitchScreen(dev,key,action)
    DeviceIntPtr  dev;
    KeyCode	  key;
    XKBAction	 *action;
{
    return 1;
}
#endif
