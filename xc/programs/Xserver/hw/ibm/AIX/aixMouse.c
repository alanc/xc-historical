/*
 * $XConsortium: aixMouse.c,v 1.1 91/05/10 08:28:42 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#include <stdio.h>
#include <sys/hft.h>

#include "X.h"
#include "Xproto.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "input.h"
#include "cursorstr.h"

#include "ibmIO.h"
#include "ibmMouse.h"
#include "ibmScreen.h"

#include "ibmTrace.h"
#include "hftQueue.h"

#include "AIX.h"
#include "AIXext.h"

extern  char    *getenv();

/***================================================================***/

static int
rtGetMotionEvents(buff, start, stop, pScr)
    CARD32 start, stop;
    xTimecoord *buff;
    ScreenPtr pScr;
{
    TRACE(("rtGetMotionEvents( buff= 0x%x, start= %d, stop= %d )\n",
	                                                buff,start,stop));
    return 0;
}

/***================================================================***/

static unsigned always0= 0;
#define BUTTONMAPSIZE   5


#ifdef AIXV3
extern int hftQFD;
#endif

int
AIXMouseProc(pDev, onoff)
    DevicePtr   pDev;
    int onoff;
{
    BYTE map[BUTTONMAPSIZE];

#ifdef AIXV3
    struct hfchgloc hf_info;
#endif

    TRACE(("AIXMouseProc( pDev= 0x%x, onoff= 0x%x )\n",pDev, onoff ));

    switch (onoff)
    {
    case DEVICE_INIT:
	    ibmPtr = pDev;
	    map[1] = Button1;
	    map[2] = Button2;
	    map[3] = Button3;
	    map[4] = Button4;
	    InitPointerDeviceStruct(
	        ibmPtr, map, BUTTONMAPSIZE, rtGetMotionEvents, ibmChangePointerControl, 0);
	    SetInputCheck( &hftPending, &always0 );

#ifdef AIXV3
	    /* Set mouse sample rate, resolution, threshhold, and scale.
	       Ideally, the default values should be okay, but they're not,
	       so this is neccessary to make the mouse smooth. -- EWu 8/17/89 */

	    hf_info.hf_cmd = HFMRATE;
	    hf_info.loc_value1 = 60;
	    if (ioctl(hftQFD,HFCHGLOC,&hf_info)<0) perror("mouse ioctl rate");

	    hf_info.hf_cmd = HFMRES;
	    hf_info.loc_value1 = 8;
	    if (ioctl(hftQFD,HFCHGLOC,&hf_info)<0) perror("mouse ioctl resolution");

	    hf_info.hf_cmd = HFMTHRESH;
	    hf_info.loc_value1 = 1;
	    hf_info.loc_value2 = 1;
	    if (ioctl(hftQFD,HFCHGLOC,&hf_info)<0) perror("mouse ioctl threshhold");

	    hf_info.hf_cmd = HFMSCALE;
	    hf_info.loc_value1 = 1;
	    if (ioctl(hftQFD,HFCHGLOC,&hf_info)<0) perror("mouse ioctl scale");

#endif
	    break;
    case DEVICE_ON:
	    pDev->on = TRUE;
	    break;
    case DEVICE_OFF:
	    pDev->on = FALSE;
	    break;
    case DEVICE_CLOSE:
	    break;
    }
    return Success;
}
