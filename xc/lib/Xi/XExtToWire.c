/* $XConsortium: XExtToWire.c,v 1.2 89/09/25 16:20:11 gms Exp $ */

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

/****************************************************************
 *
 *	XExtToWire.c - reformat an XEvent into a wire event.
 */
#define NEED_EVENTS
#define NEED_REPLIES

#include "Xlibint.h"
#include "XInput.h"
#include "XIproto.h"

extern	int	_devicevaluator;
extern	int	_devicekeyPress;
extern	int	_devicekeyRelease;
extern	int	_devicebuttonPress;
extern	int	_devicebuttonRelease;
extern	int	_devicemotionNotify;
extern	int	_devicefocusIn;
extern	int	_devicefocusOut;
extern	int	_proximityin;
extern	int	_proximityout;
extern	int	_devicestateNotify;
extern	int	_devicemappingNotify;
extern	int	_changedeviceNotify;
extern	int	_devicekeystateNotify;
extern	int	_devicebuttonstateNotify;

Status
_XExtEventToWire(dpy, re, event, count)
    register Display *dpy;	/* pointer to display structure */
    register XEvent *re;	/* pointer to where event should be reformatted */
    register xEvent **event;	/* wire protocol event */
    register int *count;
    {
    int	i;

    if (re->type == _devicekeyPress ||
        re->type == _devicekeyRelease)
	    {
	    register XDeviceKeyEvent *ev = (XDeviceKeyEvent*) re;
	    register deviceKeyButtonPointer *kev;
	    register deviceValuator *vev;

	    if (ev->axes_count > 6)
		*count = 3;
	    else if (ev->axes_count > 0)
		*count = 2;
	    else
		*count = 1;

	    kev = (deviceKeyButtonPointer *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) kev;

	    kev->type		= ev->type;
	    kev->root		= ev->root;
	    kev->event		= ev->window;
	    kev->child  	= ev->subwindow;
	    kev->time		= ev->time;
	    kev->event_x 	= ev->x ;
	    kev->event_y	= ev->y ;
	    kev->root_x		= ev->x_root;
	    kev->root_y 	= ev->y_root;
	    kev->state  	= ev->state;
	    kev->same_screen 	= ev->same_screen;
	    kev->detail 	= ev->keycode;
	    kev->deviceid	= ev->deviceid;

	    if (ev->axes_count != 0)
		{
		kev->deviceid |= MORE_EVENTS;
		vev = (deviceValuator *) ++kev;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 0;
		vev->num_valuators = ev->axes_count < 6 ? ev->axes_count : 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[i];
		}
	    if (ev->axes_count > 6)
		{
		(vev++)->deviceid |= MORE_EVENTS;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 6;
		vev->num_valuators = ev->axes_count - 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[6+i];
		}
	    }
    else if (re->type == _proximityin ||
	     re->type == _proximityout)
	    {
	    register XProximityNotifyEvent *ev =  
		(XProximityNotifyEvent *) re;
	    register deviceKeyButtonPointer *pev;
	    register deviceValuator *vev;

	    if (ev->axes_count > 6)
		*count = 3;
	    else if (ev->axes_count > 0)
		*count = 2;
	    else
		*count = 1;

	    pev = (deviceKeyButtonPointer *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) pev;

	    pev->type		= ev->type;
	    pev->root		= ev->root;
	    pev->event		= ev->window;
	    pev->child  	= ev->subwindow;
	    pev->time		= ev->time;
	    pev->event_x 	= ev->x ;
	    pev->event_y	= ev->y ;
	    pev->root_x		= ev->x_root;
	    pev->root_y 	= ev->y_root;
	    pev->state  	= ev->state;
	    pev->same_screen 	= ev->same_screen;
	    pev->deviceid	= ev->deviceid;

	    if (ev->axes_count != 0)
		{
		pev->deviceid |= MORE_EVENTS;
		vev = (deviceValuator *) ++pev;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 0;
		vev->num_valuators = ev->axes_count < 6 ? ev->axes_count : 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[i];
		}
	    if (ev->axes_count > 6)
		{
		(vev++)->deviceid |= MORE_EVENTS;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 6;
		vev->num_valuators = ev->axes_count - 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[6+i];
		}
	    }
    else if (re->type == _devicebuttonPress ||
	     re->type == _devicebuttonRelease)
	    {
	    register XDeviceButtonEvent *ev =  
		(XDeviceButtonEvent *) re;
	    register deviceKeyButtonPointer *bev;
	    register deviceValuator *vev;

	    if (ev->axes_count > 6)
		*count = 3;
	    else if (ev->axes_count > 0)
		*count = 2;
	    else
		*count = 1;

	    bev = (deviceKeyButtonPointer *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) bev;

	    bev->type		= ev->type;
	    bev->root		= ev->root;
	    bev->event		= ev->window;
	    bev->child  	= ev->subwindow;
	    bev->time		= ev->time;
	    bev->event_x 	= ev->x ;
	    bev->event_y	= ev->y ;
	    bev->root_x		= ev->x_root;
	    bev->root_y 	= ev->y_root;
	    bev->state  	= ev->state;
	    bev->same_screen 	= ev->same_screen;
	    bev->detail 	= ev->button;
	    bev->deviceid	= ev->deviceid;

	    if (ev->axes_count != 0)
		{
		bev->deviceid |= MORE_EVENTS;
		vev = (deviceValuator *) ++bev;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 0;
		vev->num_valuators = ev->axes_count < 6 ? ev->axes_count : 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[i];
		}
	    if (ev->axes_count > 6)
		{
		(vev++)->deviceid |= MORE_EVENTS;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 6;
		vev->num_valuators = ev->axes_count - 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[6+i];
		}
	    }
    else if (re->type == _devicemotionNotify)
	    {
	    register XDeviceMotionEvent *ev =   
		(XDeviceMotionEvent *)re;
	    register deviceKeyButtonPointer *mev;
	    register deviceValuator *vev;

	    if (ev->axes_count > 6)
		*count = 3;
	    else if (ev->axes_count > 0)
		*count = 2;
	    else
		*count = 1;

	    mev = (deviceKeyButtonPointer *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) mev;

	    mev->type		= ev->type;
	    mev->root		= ev->root;
	    mev->event		= ev->window;
	    mev->child  	= ev->subwindow;
	    mev->time		= ev->time;
	    mev->event_x 	= ev->x ;
	    mev->event_y	= ev->y ;
	    mev->root_x		= ev->x_root;
	    mev->root_y 	= ev->y_root;
	    mev->state  	= ev->state;
	    mev->same_screen 	= ev->same_screen;
	    mev->detail 	= ev->is_hint;
	    mev->deviceid	= ev->deviceid;

	    if (ev->axes_count != 0)
		{
		mev->deviceid |= MORE_EVENTS;
		vev = (deviceValuator *) ++mev;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 0;
		vev->num_valuators = ev->axes_count < 6 ? ev->axes_count : 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[i];
		}
	    if (ev->axes_count > 6)
		{
		(vev++)->deviceid |= MORE_EVENTS;
		vev->type = _devicevaluator;
		vev->deviceid = ev->deviceid;
		vev->first_valuator = 6;
		vev->num_valuators = ev->axes_count - 6;
		for (i=0; i<vev->num_valuators; i++)
		    vev->valuators[i] = ev->axis_data[6+i];
		}
	    }
    else if (re->type == _devicefocusIn ||
	     re->type == _devicefocusOut)
	    {
	    register XDeviceFocusChangeEvent *ev = 
		(XDeviceFocusChangeEvent *) re;
	    register deviceFocus *fev;

	    *count = 1;
	    fev = (deviceFocus *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) fev;

	    fev->type		= ev->type;
	    fev->window		= ev->window;
	    fev->mode		= ev->mode;
	    fev->detail		= ev->detail;
	    fev->time		= ev->time;
	    fev->deviceid	= ev->deviceid;
	    }
    else if (re->type == _devicemappingNotify)
	    {
	    register XDeviceMappingEvent *ev = (XDeviceMappingEvent *) re;
	    register deviceMappingNotify *mev;

	    *count = 1;
	    mev = (deviceMappingNotify *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) mev;

	    mev->type		= ev->type;
	    mev->firstKeyCode	= ev->first_keycode;
	    mev->request 	= ev->request;
	    mev->count		= ev->count;
	    mev->time		= ev->time;
	    mev->deviceid	= ev->deviceid;
	    }
    else if (re->type == _devicestateNotify)
	    {
	    register XDeviceStateNotifyEvent *ev = 
		(XDeviceStateNotifyEvent *) re;
	    register deviceStateNotify *sev;
	    register xEvent *tev;
	    XInputClass	*any = (XInputClass *) &ev->data[0];
	    unsigned char *sav_id;

	    *count = 1;

	    for (i=0; i<ev->num_classes; i++)
		{
		if (any->class == KeyClass)
		    {
		    XKeyStatus *k = (XKeyStatus *) any;
		    if (k->num_keys > 32)
			(*count)++;
		    }
		else if (any->class == ButtonClass)
		    {
		    XButtonStatus *b = (XButtonStatus *) any;
		    if (b->num_buttons > 32)
			(*count)++;
		    }
		else if (any->class == ValuatorClass)
		    {
		    XValuatorStatus *v = (XValuatorStatus *) any;
		    if (v->num_valuators > 3)
			(*count)++;
		    }
		any = (XInputClass *) ((char *) any + any->length);
		}

	    sev = (deviceStateNotify *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) sev;
	    tev = (xEvent *) (sev+1);

	    sev->type			= ev->type;
	    sev->deviceid		= ev->deviceid;
	    sav_id 			= &(sev->deviceid);
	    sev->time			= ev->time;
	    sev->classes_reported 	= 0;

	    any = (XInputClass *) &ev->data[0];
	    for (i=0; i<ev->num_classes; i++)
		{
		if (any->class == KeyClass)
		    {
		    XKeyStatus *k = (XKeyStatus *) any;
		    register deviceKeyStateNotify *kev;

		    sev->classes_reported |= (1 << KeyClass);
		    bcopy ( (char *) (k->keys) , (char *) (sev->keys), 4);
		    if (k->num_keys > 32)
			{
		        kev = (deviceKeyStateNotify *) tev++;
		        kev->type = _devicekeystateNotify;
		        kev->deviceid = ev->deviceid;
		        *sav_id |= MORE_EVENTS;
			sav_id = &(kev->deviceid);
			bcopy ( (char *) (&k->keys[4]) , (char *) (kev->keys), 
				28);
			}
		    }
		else if (any->class == ButtonClass)
		    {
		    XButtonStatus *b = (XButtonStatus *) any;
		    register deviceButtonStateNotify *bev;

		    sev->classes_reported |= (1 << ButtonClass);
		    bcopy ( (char *) (b->buttons) , (char *) (sev->buttons), 4);
		    if (b->num_buttons > 32)
			{
		        bev = (deviceButtonStateNotify *) tev++;
		        bev->type = _devicebuttonstateNotify;
		        bev->deviceid = ev->deviceid;
		        *sav_id |= MORE_EVENTS;
			sav_id = &(bev->deviceid);
			bcopy ((char *)(&b->buttons[4]), (char *)(bev->buttons),
				28);
			}
		    }
		else if (any->class == ValuatorClass)
		    {
		    int	j;
		    XValuatorStatus *val = (XValuatorStatus *) any;
		    register deviceValuator *vev;

		    sev->classes_reported |= (1 << ValuatorClass);
		    for (j=0; j<val->num_valuators && j<3; j++)
			sev->valuators[j] = val->valuators[j];
		    if (val->num_valuators > 3)
			{
		        vev = (deviceValuator *) tev++;
		        vev->type = _devicevaluator;
		        vev->deviceid = ev->deviceid;
		        vev->first_valuator = 3;
		        vev->num_valuators = val->num_valuators - 3;
		        *sav_id |= MORE_EVENTS;
			sav_id = &(vev->deviceid);
			for (j=3; j<val->num_valuators; j++)
			    vev->valuators[j-3] = val->valuators[j];
		        }
		    }
		any = (XInputClass *) ((char *) any + any->length);
		}
	    }
    else if (re->type == _changedeviceNotify)
	    {
	    register XChangeDeviceNotifyEvent *ev = 
		(XChangeDeviceNotifyEvent *) re;
	    register changeDeviceNotify *cev;

	    *count = 1;
	    cev = (changeDeviceNotify *) Xmalloc (*count * sizeof (xEvent));
	    *event = (xEvent *) cev;

	    cev->type		= ev->type;
	    cev->request 	= ev->request;
	    cev->time		= ev->time;
	    cev->deviceid	= ev->deviceid;
	    }
    else
	return(_XUnknownNativeEvent(dpy, re, event));
    return(1);
    }
