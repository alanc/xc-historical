/* $XConsortium: XExtInt.c,v 1.5 89/09/25 16:20:07 gms Exp $ */

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

/***********************************************************************
 *
 * Extension Xlib functions.
 *
 */

#define NEED_EVENTS
#define NEED_REPLIES
#include <stdio.h>
#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"

#define ENQUEUE_EVENT	True
#define DONT_ENQUEUE	False

extern	long	_xmask[];

XExtCodes *connections[MSKCNT << 5];

int	BadDevice = 0;
int	BadEvent = 1;
int	BadMode = 2;
int	BadClass = 4;

int	IReqCode = 0;
int	ExtWireToEvent();
Status	_XExtEventToWire();
int	_devicevaluator;
int	_devicekeyPress;
int	_devicekeyRelease;
int	_devicebuttonPress;
int	_devicebuttonRelease;
int	_devicemotionNotify;
int	_devicefocusIn;
int	_devicefocusOut;
int	_proximityin;
int	_proximityout;
int	_devicestateNotify;
int	_devicemappingNotify;
int	_changedeviceNotify;
int	_devicekeystateNotify;
int	_devicebuttonstateNotify;
XEvent	save_event, emptyevent;

char *XExtErrorList[] = {
	/* BadDevice	*/	"BadDevice, invalid or uninitialized input device",
	/* BadEvent	*/	"BadEvent, invalid event type",
	/* BadMode	*/	"BadMode, invalid mode parameter",
	/* BadClass	*/	"BadClass, invalid event class",
};
int XExtErrorListSize = sizeof(XExtErrorList);

/***********************************************************************
 *
 * Check to see if the input extension is installed in the server.
 * Also check to see if the version is >= the requested version.
 *
 */

CheckExtInit(dpy, major_version)
    register	Display *dpy;
    register	int	major_version;
    {
    int			i;
    XExtensionVersion 	*ext;
    XExtCodes		*XInitExtension(), *ret;
    extern Bool 	_XUnknownWireEvent();
    char		*XExtError ();
    int			XExtClose ();

    if (connections[dpy->fd] == 0) 
	{
	ret = XInitExtension( dpy, INAME);
	if (ret == NULL)
	    {
    	    UnlockDisplay(dpy);
	    return (-1);
	    }
	connections[dpy->fd] = ret;
	IReqCode = ret->major_opcode;

	BadDevice += ret->first_error;
	BadEvent += ret->first_error;
	BadMode += ret->first_error;
	BadClass += ret->first_error;
	_devicevaluator  += ret->first_event;
	_devicekeyPress = _devicevaluator+1;
	_devicekeyRelease = _devicekeyPress+1;
	_devicebuttonPress = _devicekeyRelease+1;
	_devicebuttonRelease = _devicebuttonPress+1;
	_devicemotionNotify = _devicebuttonRelease+1;
	_devicefocusIn = _devicemotionNotify+1;
	_devicefocusOut = _devicefocusIn+1;
	_proximityin = _devicefocusOut+1;
	_proximityout = _proximityin+1;
	_devicestateNotify = _proximityout+1;
	_devicemappingNotify = _devicestateNotify+1;
	_changedeviceNotify = _devicemappingNotify+1;
	_devicekeystateNotify = _changedeviceNotify+1;
	_devicebuttonstateNotify = _devicekeystateNotify+1;
	XESetErrorString (dpy, ret->extension, XExtError);
	XESetCloseDisplay (dpy, ret->extension, XExtClose);
	}
    else
	IReqCode = connections[dpy->fd]->major_opcode;

    if (dpy->event_vec[_devicevaluator] == _XUnknownWireEvent)
	for (i=0; i<IEVENTS; i++)
	    {
	    XESetWireToEvent (dpy, _devicevaluator+i, ExtWireToEvent);
	    XESetEventToWire (dpy, _devicevaluator+i, _XExtEventToWire);
	    }

    if (major_version > Dont_Check)
	{
	ext = XGetExtensionVersion (dpy, "XInputExtension");
	if (ext->major_version < major_version)
	    {
	    XFree (ext);
    	    UnlockDisplay(dpy);
	    return (-1);
	    }
	XFree (ext);
	}
    return (0);
    }

/*****************************************************************************
 *
 * Error handling routine.
 *
 */

char *
XExtError (dpy, code, codes, buffer, nbytes)
    Display	*dpy;
    int		code;
    XExtCodes	*codes;
    char	*buffer;
    int		nbytes;
    {
    int	 error = code - codes->first_error;
    char buf[32];
    char *defaultp = NULL;

    if (error <= XExtErrorListSize/ sizeof (char *)) 
	{
	defaultp =  XExtErrorList[error];
	XGetErrorDatabaseText(dpy, "XRequestMajor", buf, defaultp, buffer, 
		nbytes);
	}
    }

/*****************************************************************************
 *
 * Close display routine.
 *
 */

XExtClose (dpy, codes)
    Display	*dpy;
    XExtCodes	*codes;
    {
    connections[dpy->fd] = NULL;
    }

/*****************************************************************************
 *
 *
 * We should be using the corresponding routine in XlibInt.c, but they
 * declared it static.  We also need to check for DeviceStateNotify 
 * rather than KeymapNotify.
 *
 */
static unsigned long
_ExtSetLastRequestRead(dpy, rep)
    register Display *dpy;
    register xGenericReply *rep;
{
    register unsigned long	newseq, lastseq;

    newseq = (dpy->last_request_read & ~((unsigned long)0xffff)) |
	     rep->sequenceNumber;
    lastseq = dpy->last_request_read;
    while (newseq < lastseq) {
	newseq += 0x10000;
	if (newseq > dpy->request) {
	    (void) fprintf(stderr, "sequence lost!\n");
	    newseq -= 0x10000;
	    break;
	}
    }

    dpy->last_request_read = newseq;
    return(newseq);
}

int
Ones(mask)  
    Mask mask;
{
    register Mask y;

    y = (mask >> 1) &033333333333;
    y = mask - y - ((y >>1) & 033333333333);
    return (((y + (y >> 3)) & 030707070707) % 077);
}

/***********************************************************************
 *
 * Handle Input extension events.
 * Reformat a wire event into an XEvent structure of the right type.
 *
 */

ExtWireToEvent (dpy, re, event)
    Display	*dpy;
    XEvent	*re;
    xEvent	*event;
    {
    XEvent	*tmp = &save_event;
    xEvent	nevent;
    unsigned	int	type;
    unsigned	int	ret = 1;
    unsigned	int	i;

    type = event->u.u.type & 0x7f;

    if (type != _devicevaluator && 
	type != _devicekeystateNotify &&
	type != _devicebuttonstateNotify)
	{
	*tmp = emptyevent;
        tmp->type = type;
        ((XAnyEvent *)tmp)->serial = _ExtSetLastRequestRead(dpy,
	    (xGenericReply *)event);
        ((XAnyEvent *)tmp)->send_event = ((event->u.u.type & 0x80) != 0);
        ((XAnyEvent *)tmp)->display = dpy;
	}
	
    if (type == _devicemotionNotify)
	    {
	    register XDeviceMotionEvent *ev = (XDeviceMotionEvent*) &save_event;
	    deviceKeyButtonPointer *ev2 = (deviceKeyButtonPointer *) event;

	    ev->root 		= ev2->root;
	    ev->window 		= ev2->event;
	    ev->subwindow 	= ev2->child;
	    ev->time 		= ev2->time;
	    ev->x_root 		= ev2->root_x;
	    ev->y_root 		= ev2->root_y;
	    ev->x 		= ev2->event_x;
	    ev->y 		= ev2->event_y;
	    ev->state		= ev2->state;
	    ev->same_screen	= ev2->same_screen;
	    ev->is_hint 	= ev2->detail;
	    ev->deviceid	= ev2->deviceid & DEVICE_BITS;
    	    return (DONT_ENQUEUE);
	    }
    else if (type == _devicekeyPress || type == _devicekeyRelease)
	    {
	    register XDeviceKeyEvent *ev = (XDeviceKeyEvent*) &save_event;
	    deviceKeyButtonPointer *ev2 = (deviceKeyButtonPointer *) event;

	    ev->root 		= ev2->root;
	    ev->window 		= ev2->event;
	    ev->subwindow 	= ev2->child;
	    ev->time 		= ev2->time;
	    ev->x_root 		= ev2->root_x;
	    ev->y_root 		= ev2->root_y;
	    ev->x 		= ev2->event_x;
	    ev->y 		= ev2->event_y;
	    ev->state		= ev2->state;
	    ev->same_screen	= ev2->same_screen;
	    ev->keycode 	= ev2->detail;
	    ev->deviceid	= ev2->deviceid & DEVICE_BITS;
	    if (ev2->deviceid & MORE_EVENTS)
		return (DONT_ENQUEUE);
	    else
		{
		*re = save_event;
		return (ENQUEUE_EVENT);
		}
	    }
    else if (type == _devicebuttonPress || type == _devicebuttonRelease)
	    {
	    register XDeviceButtonEvent *ev = (XDeviceButtonEvent*) &save_event;
	    deviceKeyButtonPointer *ev2 = (deviceKeyButtonPointer *) event;

	    ev->root 		= ev2->root;
	    ev->window 		= ev2->event;
	    ev->subwindow 	= ev2->child;
	    ev->time 		= ev2->time;
	    ev->x_root 		= ev2->root_x;
	    ev->y_root 		= ev2->root_y;
	    ev->x 		= ev2->event_x;
	    ev->y 		= ev2->event_y;
	    ev->state		= ev2->state;
	    ev->same_screen	= ev2->same_screen;
	    ev->button  	= ev2->detail;
	    ev->deviceid	= ev2->deviceid & DEVICE_BITS;
	    if (ev2->deviceid & MORE_EVENTS)
		return (DONT_ENQUEUE);
	    else
		{
		*re = save_event;
		return (ENQUEUE_EVENT);
		}
	    }
    else if (type == _proximityin || type == _proximityout)
	    {
	    register XProximityNotifyEvent *ev = 
		(XProximityNotifyEvent *) &save_event;
	    deviceKeyButtonPointer *ev2 = (deviceKeyButtonPointer *) event;

	    ev->root 		= ev2->root;
	    ev->window 		= ev2->event;
	    ev->subwindow 	= ev2->child;
	    ev->time 		= ev2->time;
	    ev->x_root 		= ev2->root_x;
	    ev->y_root 		= ev2->root_y;
	    ev->x 		= ev2->event_x;
	    ev->y 		= ev2->event_y;
	    ev->state		= ev2->state;
	    ev->same_screen	= ev2->same_screen;
	    ev->deviceid	= ev2->deviceid & DEVICE_BITS;
	    if (ev2->deviceid & MORE_EVENTS)
		return (DONT_ENQUEUE);
	    else
		{
		*re = save_event;
		return (ENQUEUE_EVENT);
		}
	}
    else if (type == _devicevaluator)
	    {
	    deviceValuator *xev = (deviceValuator *) event;

	    if (save_event.type == _devicekeyPress ||
	        save_event.type == _devicekeyRelease)
		{
	        XDeviceKeyEvent *kev = (XDeviceKeyEvent*) &save_event;
		kev->device_state = xev->device_state;
		if (kev->axes_count == 0)
		    {
		    kev->axes_count = xev->num_valuators;
		    kev->first_axis = xev->first_valuator;
		    }
		else if (kev->axes_count + xev->num_valuators <= 8)
		    kev->axes_count += xev->num_valuators;
		for (i=0; i<xev->num_valuators; i++)
		    kev->axis_data[xev->first_valuator+i] = xev->valuators[i];
		}
	    else if (save_event.type == _devicebuttonPress ||
	        save_event.type == _devicebuttonRelease)
		{
	        XDeviceButtonEvent *bev = (XDeviceButtonEvent*) &save_event;
		bev->device_state = xev->device_state;
		if (bev->axes_count == 0)
		    {
		    bev->axes_count = xev->num_valuators;
		    bev->first_axis = xev->first_valuator;
		    }
		else if (bev->axes_count + xev->num_valuators <= 8)
		    bev->axes_count += xev->num_valuators;
		for (i=0; i<xev->num_valuators; i++)
		    bev->axis_data[xev->first_valuator+i] = xev->valuators[i];
		for (i=0; i<xev->num_valuators; i++)
		    bev->axis_data[xev->first_valuator+i] = xev->valuators[i];
		}
	    else if (save_event.type == _devicemotionNotify) 
		{
	        XDeviceMotionEvent *mev = (XDeviceMotionEvent*) &save_event;
		mev->device_state = xev->device_state;
		if (mev->axes_count == 0)
		    {
		    mev->axes_count = xev->num_valuators;
		    mev->first_axis = xev->first_valuator;
		    }
		else if (mev->axes_count + xev->num_valuators <= 8)
		    mev->axes_count += xev->num_valuators;
		for (i=0; i<xev->num_valuators; i++)
		    mev->axis_data[xev->first_valuator+i] = xev->valuators[i];
		for (i=0; i<xev->num_valuators; i++)
		    mev->axis_data[xev->first_valuator+i] = xev->valuators[i];
		}
	    else if (save_event.type == _proximityin ||
	        save_event.type == _proximityout)
		{
	        XProximityNotifyEvent *pev = 
			(XProximityNotifyEvent*) &save_event;
		pev->device_state = xev->device_state;
		if (pev->axes_count == 0)
		    {
		    pev->axes_count = xev->num_valuators;
		    pev->first_axis = xev->first_valuator;
		    }
		else if (pev->axes_count + xev->num_valuators <= 8)
		    pev->axes_count += xev->num_valuators;
		for (i=0; i<xev->num_valuators; i++)
		    pev->axis_data[xev->first_valuator+i] = xev->valuators[i];
		}
	    else if (save_event.type == _devicestateNotify)
		{
	        XDeviceStateNotifyEvent *sev = 
			(XDeviceStateNotifyEvent*) &save_event;
		XInputClass *any = (XInputClass *) &sev->data[0];
		XValuatorStatus *v;

		for (i=0; i<sev->num_classes; i++)
		    if (any->class != ValuatorClass)
			any = (XInputClass *) ((char *) any + any->length);
		v = (XValuatorStatus *) any;
		if (v->num_valuators + xev->num_valuators <= 6)
		    v->num_valuators += xev->num_valuators;
		for (i=0; i<xev->num_valuators; i++)
		    v->valuators[xev->first_valuator+i] = xev->valuators[i];

		}
	    if (xev->deviceid & MORE_EVENTS)
		return (DONT_ENQUEUE);
	    else
		{
		*re = save_event;
		return (ENQUEUE_EVENT);
		}
	    }
    else if (type == _devicefocusIn || type == _devicefocusOut)
	{
	register XDeviceFocusChangeEvent *ev = (XDeviceFocusChangeEvent *) re;
	deviceFocus *fev = (deviceFocus *) event;

	*ev			= *((XDeviceFocusChangeEvent *) &save_event);
	ev->window 		= fev->window;
	ev->time   		= fev->time;
	ev->mode		= fev->mode;
	ev->detail		= fev->detail;
	ev->deviceid 		= fev->deviceid & DEVICE_BITS;
    	return (ENQUEUE_EVENT);
	}
    else if (type == _devicestateNotify)
	{
	XDeviceStateNotifyEvent *stev = (XDeviceStateNotifyEvent *) &save_event;
	deviceStateNotify *sev = (deviceStateNotify *) event;
	XKeyStatus *kstev;
	XButtonStatus *bev;
	XValuatorStatus *vev;
	char *data;

	stev->window 		= dpy->current;
	stev->deviceid 		= sev->deviceid & DEVICE_BITS;
	stev->time     		= sev->time;
	stev->num_classes	 	= Ones (sev->classes_reported);
 	data = (char *) &stev->data[0];
	if (sev->classes_reported & (1 << KeyClass))
	    {
	    register XKeyStatus *kstev = (XKeyStatus *) data;
	    kstev->class = KeyClass;
	    kstev->length = sizeof (XKeyStatus);
	    kstev->num_keys = 32;
	    bcopy ((char *) &sev->keys[0], (char *) &kstev->keys[0], 4);
	    data += sizeof (XKeyStatus);
	    }
	if (sev->classes_reported & (1 << ButtonClass))
	    {
	    register XButtonStatus *bev = (XButtonStatus *) data;
	    bev->class = ButtonClass;
	    bev->length = sizeof (XButtonStatus);
	    bev->num_buttons = 32;
	    bcopy ((char *) sev->buttons, (char *) bev->buttons, 4);
	    data += sizeof (XButtonStatus);
	    }
	if (sev->classes_reported & (1 << ValuatorClass))
	    {
	    register XValuatorStatus *vev = (XValuatorStatus *) data;
	    vev->class = ValuatorClass;
	    vev->length = sizeof (XValuatorStatus);
	    vev->num_valuators = 3;
	    for (i=0; i<3; i++)
		vev->valuators[i] = sev->valuators[i];
	    data += sizeof (XValuatorStatus);
	    }
    	if (sev->deviceid & MORE_EVENTS)
	    return (DONT_ENQUEUE);
	else
	    {
	    *re = save_event;
	    stev = (XDeviceStateNotifyEvent *) re;
	    return (ENQUEUE_EVENT);
	    }
	}
    else if (type == _devicekeystateNotify)
	{
	int i;
	XInputClass *anyclass;
	register XKeyStatus *kv;
	deviceKeyStateNotify *ksev = (deviceKeyStateNotify *) event;
	XDeviceStateNotifyEvent *kstev = 
		(XDeviceStateNotifyEvent *) &save_event;

	anyclass = (XInputClass *) &kstev->data[0];
	for (i=0; i<kstev->num_classes; i++)
	    if (anyclass->class == KeyClass)
		break;
	    else 
		anyclass = (XInputClass *) ((char *) anyclass + 
			anyclass->length);
	
	kv = (XKeyStatus *) anyclass;
	kv->num_keys = 256;
	bcopy ((char *) ksev->keys, (char *) &kv->keys[4], 28);
    	if (ksev->deviceid & MORE_EVENTS)
	    return (DONT_ENQUEUE);
	else
	    {
	    *re = save_event;
	    kstev = (XDeviceStateNotifyEvent *) re;
	    return (ENQUEUE_EVENT);
	    }
	}
    else if (type == _devicebuttonstateNotify)
	{
	int i;
	XInputClass *anyclass;
	register XButtonStatus *bv;
	deviceButtonStateNotify *bsev = (deviceButtonStateNotify *) event;
	XDeviceStateNotifyEvent *bstev = 
		(XDeviceStateNotifyEvent *) &save_event;


	anyclass = (XInputClass *) &bstev->data[0];
	for (i=0; i<bstev->num_classes; i++)
	    if (anyclass->class == ButtonClass)
		break;
	    else 
		anyclass = (XInputClass *) ((char *) anyclass + 
			anyclass->length);
	
	bv = (XButtonStatus *) anyclass;
	bv->num_buttons = 256;
	bcopy ((char *) bsev->buttons, (char *) &bv->buttons[4], 28);
    	if (bsev->deviceid & MORE_EVENTS)
	    return (DONT_ENQUEUE);
	else
	    {
	    *re = save_event;
	    bstev = (XDeviceStateNotifyEvent *) re;
	    return (ENQUEUE_EVENT);
	    }
	}
    else if (type == _devicemappingNotify)
	{
	register XDeviceMappingEvent *ev = (XDeviceMappingEvent *) re;
	deviceMappingNotify *ev2 = (deviceMappingNotify *) event;

	*ev			= *((XDeviceMappingEvent *) &save_event);
	ev->first_keycode 	= ev2->firstKeyCode;
	ev->request 		= ev2->request;
	ev->count 		= ev2->count;
	ev->time  		= ev2->time;
	ev->deviceid 		= ev2->deviceid & DEVICE_BITS;
    	return (ENQUEUE_EVENT);
	}
    else if (type == _changedeviceNotify)
	{
	register XChangeDeviceNotifyEvent *ev = (XChangeDeviceNotifyEvent *) re;
	changeDeviceNotify *ev2 = (changeDeviceNotify *) event;

	*ev			= *((XChangeDeviceNotifyEvent *) &save_event);
	ev->request 		= ev2->request;
	ev->time  		= ev2->time;
	ev->deviceid 		= ev2->deviceid & DEVICE_BITS;
    	return (ENQUEUE_EVENT);
	}
    else
	printf ("ExtWireToEvent: UNKNOWN WIRE EVENT! type=%d\n",type);

    return (DONT_ENQUEUE);
    }
