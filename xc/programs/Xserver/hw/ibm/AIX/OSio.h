/*
 * $XConsortium: OSio.h,v 1.4 93/09/05 15:37:36 rws Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef OSIO_H
#define OSIO_H

#define	CURRENT_X()	(AIXCurrentX)
#define	CURRENT_Y()	(AIXCurrentY)

extern	int	AIXDefaultDisplay;
extern	int	AIXCurrentX;
extern	int	AIXCurrentY;
extern	void	AIXBlockHandler();
extern	void	AIXWakeupHandler();
extern	void	AIXInitEventHandlers();
extern	int	AIXProcessArgument();
extern	void	hftTermQueue();

#define	setCursorPosition(x,y)	{ AIXCurrentX= (x); AIXCurrentY= (y); }

#define	OS_BlockHandler			AIXBlockHandler
#define	OS_WakeupHandler		AIXWakeupHandler
#define	OS_MouseProc			AIXMouseProc
#define	OS_KeybdProc			AIXKeybdProc

/*
#define OS_CapsLockFeedback(dir)	NoopDDA()
*/

#define	OS_PreScreenInit()		AIXMachineDependentInit()
/*
#define	OS_PostScreenInit()		NoopDDA()
#define	OS_ScreenStateChange(e)		NoopDDA()
*/

/*
#define OS_AddAndRegisterOtherDevices()	NoopDDA()
*/
#define	OS_GetDefaultScreens()		AIXGetDefaultScreens()

#define	OS_InitInput()			AIXInitEventHandlers()
/*
#define	OS_SaveState()			NoopDDA()
#define	OS_RestoreState()		NoopDDA()
*/

#define	OS_GiveUp()			hftTermQueue()
#define	OS_Abort()			hftTermQueue()

#define	OS_ProcessArgument		AIXProcessArgument

#endif /* OSIO_H */
