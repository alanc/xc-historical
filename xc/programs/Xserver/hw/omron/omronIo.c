/*
 * $XConsortium: omronIo.c,v 1.2 95/04/05 19:58:42 kaleb Exp $
 *
 * Copyright 1991 by OMRON Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OMRON
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "omron.h"
#include "omronKbd.h"
#include "omronMouse.h"

int lastEventTime = 0;

static void (* omronIoHandler)(); 

void
omronSetIoHandler( ioHandler )
void (* ioHandler)(); 
{
	omronIoHandler = ioHandler;
}


#ifdef UNUSE_SIGIO_SIGNAL
void
omronWakeupProc(blockData, result, pReadmask)
pointer blockData;
int result;
pointer pReadmask;
{
	fd_set* LastSelectMask = (fd_set*)pReadmask;
	fd_set devicesReadable;

	if(result <= 0) return;

	MASKANDSETBITS(&devicesReadable, LastSelectMask, &EnabledDevices);

	if(XFD_ANYSET(&devicesReadable)) {
		(* omronIoHandler)();
	}
}
#else
void
omronSigIOHandler(sig, code, scp)
int     code;
int     sig;
struct sigcontext *scp;
{
	(* omronIoHandler)();
}
#endif

#ifndef UNUSE_DRV_TIME
static struct _omronEventPrvRec {
		DevicePtr   pMouse;
		DevicePtr   pKeyboard;
		Bool		mouseHasTime;
		Bool		keyHasTime;
}	omronEventPrv;

void
omronInitEventPrvRec()
{
	omronEventPrv.pMouse 	   = NULL;
	omronEventPrv.pKeyboard    = NULL;
	omronEventPrv.mouseHasTime = FALSE;
	omronEventPrv.keyHasTime   = FALSE;
}

void
omronSetDriverTimeMode(pMouse, pKeyboard)
DevicePtr   pMouse;
DevicePtr   pKeyboard;
{
	omronKeyPrvPtr kPrv;
	omronMousePrvPtr  mPrv;

	if(pMouse) {
		omronEventPrv.pMouse = pMouse;
		mPrv = (omronMousePrvPtr)(pMouse->devicePrivate);
		if(ioctl(mPrv->fd, MSTIME,1) < 0) {
			if ( errno != EINVAL ) {
				Error("mouse ioctl MSTIME fault.");
			}
			if(omronEventPrv.pKeyboard != NULL) {
				kPrv = (omronKeyPrvPtr)(omronEventPrv.pKeyboard->devicePrivate);
				if(ioctl(kPrv->fd, KBTIME,0) < 0) {
					if ( errno != EINVAL ) {
						Error("kbd ioctl KBTIME fault.");
					}
				}
			}
			omronEventPrv.keyHasTime = FALSE;
		} else {
			omronEventPrv.mouseHasTime = TRUE;
		}
	} else if(pKeyboard) {
		omronEventPrv.pKeyboard = pKeyboard;
		kPrv = (omronKeyPrvPtr)(pKeyboard->devicePrivate);
		if(ioctl(kPrv->fd, KBTIME,1) < 0) {
			if ( errno != EINVAL ) {
				Error("mouse ioctl KBTIME fault.");
			}
			if(omronEventPrv.pMouse != NULL) {
				mPrv = (omronMousePrvPtr)(omronEventPrv.pMouse->devicePrivate);
				if(ioctl(mPrv->fd, MSTIME,0) < 0) {
					if ( errno == EINVAL ) {
						Error("kbd ioctl MSTIME fault.");
					}
				}
			}
			omronEventPrv.mouseHasTime = FALSE;
		} else {
			omronEventPrv.keyHasTime = TRUE;
		}
	}

	if((omronEventPrv.keyHasTime == TRUE) &&
	   (omronEventPrv.mouseHasTime == TRUE)) {
		omronSetIoHandler(omronEnqueueTEvents); 
	} else {
		omronSetIoHandler(omronEnqueueEvents); 
	}
}
#endif


void
ProcessInputEvents()
{
	mieqProcessInputEvents();
	miPointerUpdate();
}


int
TimeSinceLastInputEvent()
{
	long now;
	
	now = GetTimeInMillis();

	if (lastEventTime == 0) {
		lastEventTime = now;
	}
	return(now - lastEventTime);
}


void
omronSetLastEventTime()
{
	lastEventTime = GetTimeInMillis();
}


void
omronEnqueueEvents()
{
	DevicePtr     	pPtr;
	DevicePtr     	pKbd;
	struct msdata 	*ptrEvents;	
	unsigned char 	*KbdEvents;
	int	 	nk, np;
	Bool	 	ptrRetry, kbdRetry;			
	
	pPtr = LookupPointerDevice();
	pKbd = LookupKeyboardDevice();

	if (!pPtr->on || !pKbd->on)
		return;

	kbdRetry = TRUE;
	while( kbdRetry ) {
		KbdEvents = omronKbdGetEvents(pKbd, &nk, &kbdRetry); 	
		while(nk--) {
			omronKbdEnqueueEvent(pKbd, KbdEvents++);
		}
	}

	ptrRetry = TRUE;
	while( ptrRetry ) {
		ptrEvents = omronMouseGetEvents(pPtr, &np, &ptrRetry); 	
		while(np--) {
			omronMouseEnqueueEvent(pPtr, ptrEvents++);
		}
	}
}


#ifndef UNUSE_DRV_TIME
void
omronEnqueueTEvents()
{
	DevicePtr       pPtr;
	DevicePtr       pKbd;
	struct msdatat 	*ptrEvents = (struct msdatat *) NULL;	
	key_event 	*kbdEvents = (key_event *) NULL;
	register int	nPtrEvent, nKbdEvent;
	int		np, nk;
	Bool		ptrRetry, kbdRetry;			

	pKbd = LookupKeyboardDevice();
	pPtr = LookupPointerDevice();
	if(!pPtr->on || !pKbd->on)
		return;

	nPtrEvent = nKbdEvent = 0;
	ptrRetry = kbdRetry = TRUE;

	while (1) {
		if((nPtrEvent == 0) && ptrRetry) {
			ptrEvents = omronMouseGetTEvents(pPtr, &np, &ptrRetry);
			nPtrEvent = np;
		}
		if((nKbdEvent == 0) && kbdRetry) {
			kbdEvents = omronKbdGetTEvents(pKbd, &nk, &kbdRetry); 	
			nKbdEvent = nk;
		}
		if((nKbdEvent == 0) && (nPtrEvent == 0))
			break;
		if(nPtrEvent && nKbdEvent) {
			if ( ptrEvents->time < kbdEvents->time )  {
				omronMouseEnqueueTEvent(pPtr, ptrEvents);
				lastEventTime = ptrEvents++->time;
				nPtrEvent--;
			} else {
				omronKbdEnqueueTEvent(pKbd, kbdEvents);
				lastEventTime = kbdEvents++->time;
				nKbdEvent--;
			}
		} else if(nPtrEvent) {
			omronMouseEnqueueTEvent(pPtr, ptrEvents);
			lastEventTime = ptrEvents++->time;
			nPtrEvent--;
		} else {
			omronKbdEnqueueTEvent(pKbd, kbdEvents);
			lastEventTime = kbdEvents++->time;
			nKbdEvent--;
		}
	}
}
#endif
