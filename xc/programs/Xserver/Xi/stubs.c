/* $XConsortium: xstubs.c,v 1.2 89/11/10 11:58:20 rws Exp $ */

/************************************************************
Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, California, and the 
Massachusetts Institute of Technology, Cambridge, Massachusetts.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Hewlett-Packard or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#define	 NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "XI.h"
#include "XIproto.h"

/***********************************************************************
 *
 * This procedure does the implementation-dependent portion of the work
 * needed to change the keyboard device.
 *
 */

int
ChangeKeyboardDevice (old_dev, new_dev)
    DeviceIntPtr	old_dev;
    DeviceIntPtr	new_dev;
    {
    return Success;
    }


/***********************************************************************
 *
 * This procedure does the implementation-dependent portion of the work
 * needed to change the pointer device.
 *
 */

int
ChangePointerDevice (old_dev, new_dev, xaxis, yaxis)
    DeviceIntPtr	old_dev;
    DeviceIntPtr	new_dev;
    unsigned char	xaxis, yaxis;
    {
    return Success;
    }

/***********************************************************************
 *
 * Take care of implementation-dependent details of closing a device.
 * Some implementations may actually close the device, others may just
 * remove this clients interest in that device.
 *
 */

void
CloseInputDevice (d, client)
    DeviceIntPtr d;
    ClientPtr client;
    {
    }

/***********************************************************************
 *
 * This procedure should be used by implementations that do not initialize
 * all input devices at server startup.  It should do device-dependent
 * initialization for any devices not previously initialized, and call
 * AddInputDevice for each of those devices so that a DeviceIntRec will be 
 * created for them.
 *
 */

void
AddOtherInputDevices ()
    {
    DeviceIntPtr dev;
    DeviceProc deviceProc;
    pointer private;

    /*  for each uninitialized device, do something like: */

    dev = (DeviceIntPtr) AddInputDevice(deviceProc, TRUE);
    dev->public.devicePrivate = private;
    RegisterOtherDevice(dev);
    dev->inited = ((*dev->deviceProc)(dev, DEVICE_INIT) == Success);

    }

/***********************************************************************
 *
 * Implementation-dependent routine to open an input device.
 * If the physical device is not already open, do it here.
 *
 */

void
OpenInputDevice (dev, client, status)
    DeviceIntPtr dev;
    ClientPtr client;
    int *status;
    {
    }

/****************************************************************************
 *
 * Change the mode of an extension device.
 *
 */

int
SetDeviceMode (client, id, mode)
    register	ClientPtr	client;
    int		id;
    int		mode;
    {
    return BadMatch;
    }
