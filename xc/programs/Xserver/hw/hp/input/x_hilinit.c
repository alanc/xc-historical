/* $XConsortium: x_hilinit.c,v 8.201 94/04/17 20:30:17 dpw Exp $ */
/*

Copyright (c) 1988-1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright (c) 1988-1992 by Hewlett-Packard Corporation, Palo Alto,
              California
	      
Permission to use, copy, modify, and distribute this software 
and its documentation for any purpose and without fee is hereby 
granted, provided that the above copyright notice appear in all 
copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the name of 
Hewlett-Packard not be used in advertising or 
publicity pertaining to distribution of the software without specific, 
written prior permission.

HP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HP BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

#define SERIAL_DRIVER_WRITE(f, type, ptr) \
	{ int i; \
	for (i=0; i<num_serial_devices; i++) \
	    if (f==serialprocs[i].fd) { \
		if ((*(serialprocs[i].write)) (f, type, ptr)==WRITE_SUCCESS) \
		    return Success; } }

#ifndef LIBDIR
#if OSMAJORVERSION >= 10
#define LIBDIR "/etc/X11"
#else
#define LIBDIR "/usr/lib/X11"
#endif
#endif
#define DRVRLIBDIR LIBDIR/**/"/extensions/"

#define	 MAXNAMLEN	255
#define  spare  	MAX_LOGICAL_DEVS - 2
#define	 NEED_EVENTS
#include <stdio.h>
#include <errno.h>

#if defined(__hpux) || defined(__hp_osf)
#define BEEPER_DEVICE   "/dev/rhil"

#ifdef __hp_osf
#include <hp/hilioctl.h>
#include <sys/mman.h>
#else
#include <sys/hilioctl.h>
#endif

#include <fcntl.h>
#include <dl.h>
#include <sys/utsname.h>
#include "ps2io.h"
#endif /* __hpux */

#include "X.h"
#include "Xproto.h"
#include "hildef.h"
#include "XHPproto.h"				/* extension constants	*/
#include "x_hilinit.h"
#include "x_hil.h"
#include "x_serialdrv.h"
#include "inputstr.h"
#include "../../../os/osdep.h"

#ifdef XINPUT
#include "XI.h"
#include "XIproto.h"
#else
#define Relative 0
#define Absolute 1
#endif /* XINPUT */

#ifdef __apollo
#include "screenint.h"
#include "../apollo/apollo.h"
#include "../apollo/smd.h"
#endif /* __apollo */

/******************************************************************
 *
 * Externs and global variables that may be referenced by other files.
 *
 */

char	*mflg="TRUE";
int		num_serial_devices;
SerialProcs 	serialprocs[MAX_DEVICES];
HPInputDevice	*hpKeyboard;
HPInputDevice	*hpPointer;
HPInputDevice	*hptablet_extension;

#ifdef __hp_osf
HILQ *hil_qp;
int hil_qd;
#endif /* __hp_osf */

#ifdef __apollo
extern long	*apECV;
extern long	*apLastECV;
static int	fdApollo = 0;
status_$t       status;
#endif /* __apollo */

#ifdef XINPUT
extern	int	BadDevice;
extern	int	BadMode;
extern	int	IReqCode;
extern	int	DeviceKeyPress;
extern	int	DeviceKeyRelease;
extern	int	DeviceButtonPress;
extern	int	DeviceButtonRelease;
extern	int	DeviceMotionNotify; 
XID	hp_device_ids[MAX_DEVICES];
XID		x_device_ids[MAX_DEVICES];
#endif  /* XINPUT */

extern	int	*dpmotionBuf[];
extern	int	*dheadmotionBuf[];
extern  void 	SetBellAttributes();
extern  void 	hpBell();
extern  u_char	identity_map[];
extern  u_char	mv_mods, ptr_mods, rs_mods, bw_mods;
extern  u_char	pointer_amt_bits[];
extern  char   	*display;		/* display number as a string */
extern  int	queue_events_free;
extern  struct	x11EventQueue *events_queue;
extern 	InputInfo 	inputInfo;
extern 	unsigned char lockcode;

int 	axes_changed = FALSE;
int 	keyboard_click;
int	allocated_dev_names = FALSE;
int	x_axis, y_axis;

int  otherndx;
int  beeper_fd = -1;
int  max_input_fd;
char ldigit = '\0';
unsigned char	xhp_kbdid;
unsigned tablet_xlimit;
unsigned tablet_ylimit;
unsigned tablet_xorg;
unsigned tablet_yorg;

HPInputDevice	l_devs[MAX_LOGICAL_DEVS];
DeviceIntPtr	LookupDeviceIntRec ();
DeviceIntPtr	tablet_extension_device;
DeviceIntPtr	screen_change_dev;

int     HPType;
int     device_ndx;

/******************************************************************
 *
 * Variables that are global to this file only.
 *
 */

static Bool din_mouse_present();
static Bool din_kbd_present();
static int init_hil_devs ();
static int get_device_details();
static int get_device_type();
static void SetAutoRepeat ();
static int device_files ();
static DevicePtr hpAddInputDevice();
static void RecordOpenRequest();
static void SetInputDevice();
static void mask_from_kcodes();

static	int	loopnum;
static	struct	utsname uts_name;
static	char	hilpath[MAXNAMLEN+1];

void		ProcessOtherEvent();

static	xHPEvent	events_array[MAX_EVENTS];	/* input event buffer*/
static	struct		x11EventQueue ev_queue;
static	int 		count [NUM_DEV_TYPES];

#if defined(__hpux) || defined(__hp_osf)
static	int prompt[] = {HILP, HILP1, HILP2, HILP3, HILP4, HILP5, HILP6, HILP7};
static	int ack[] = {HILA, HILA1, HILA2, HILA3, HILA4, HILA5, HILA6, HILA7};
#endif /* __hpux */

static char  *dev_names[MAX_LOGICAL_DEVS];

#if defined(__hp9000s300) || defined(__hp9000s700) || defined(__hp_osf)
static char  *default_names[MAX_LOGICAL_DEVS] =
    { 
    "/dev/hil1",
    "/dev/hil2",
    "/dev/hil3",
    "/dev/hil4",
    "/dev/hil5",
    "/dev/hil6",
    "/dev/hil7",
    "",
    "/dev/null"};
#else				/* building for s800 */
char beeper_name[] = "/dev/hilkbd ";
static char  *default_names[MAX_LOGICAL_DEVS] =
    { 
    "/dev/hil_0.1",
    "/dev/hil_0.2",
    "/dev/hil_0.3",
    "/dev/hil_0.4",
    "/dev/hil_0.5",
    "/dev/hil_0.6",
    "/dev/hil_0.7",
    "/dev/hil_1.1",
    "/dev/hil_1.2",
    "/dev/hil_1.3",
    "/dev/hil_1.4",
    "/dev/hil_1.5",
    "/dev/hil_1.6",
    "/dev/hil_1.7",
    "/dev/hil_2.1",
    "/dev/hil_2.2",
    "/dev/hil_2.3",
    "/dev/hil_2.4",
    "/dev/hil_2.5",
    "/dev/hil_2.6",
    "/dev/hil_2.7",
    "/dev/hil_3.1",
    "/dev/hil_3.2",
    "/dev/hil_3.3",
    "/dev/hil_3.4",
    "/dev/hil_3.5",
    "/dev/hil_3.6",
    "/dev/hil_3.7",
    "",
    "/dev/null"};
#endif	/* building on __hp9000s300 or for s700 */


/****************************************************************************
 *
 * Procedures to control Integer and String feedbacks.  
 * These are not used by currently supported devices, but serial input devices
 * could use them.
 */

static hpChangeIntegerControl(pDevice, ctrl)
    DevicePtr pDevice;
    IntegerCtrl *ctrl;
    {
    HPIntegerFeedbackControl	dctrl;
    HPInputDevice *d = GET_HPINPUTDEVICE ((DeviceIntPtr) pDevice);

    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dctrl.class = IntegerFeedbackClass;
	dctrl.resolution = ctrl->resolution;
	dctrl.max_value = ctrl->max_value;
	dctrl.integer_displayed = ctrl->integer_displayed;
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeFeedbackControl, &dctrl);
	}
    }

static StringCtrlProcPtr hpChangeStringControl(pDevice, ctrl)
    DevicePtr pDevice;
    StringCtrl *ctrl;
    {
    HPStringFeedbackControl	dctrl;
    HPInputDevice *d = GET_HPINPUTDEVICE ((DeviceIntPtr) pDevice);

    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dctrl.class = StringFeedbackClass;
	dctrl.max_symbols = ctrl->max_symbols;
	dctrl.num_symbols_supported = ctrl->num_symbols_supported;
	dctrl.num_symbols_displayed = ctrl->num_symbols_displayed;
	dctrl.symbols_supported = (int *) ctrl->symbols_supported;
	dctrl.symbols_displayed = (int *) ctrl->symbols_displayed;
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeFeedbackControl, &dctrl);
	}
    }

static BellCtrlProcPtr hpChangeBellControl(pDevice, ctrl)
    DevicePtr pDevice;
    BellCtrl *ctrl;
    {
    HPBellFeedbackControl	dctrl;
    HPInputDevice *d = GET_HPINPUTDEVICE ((DeviceIntPtr) pDevice);

    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dctrl.class = BellFeedbackClass;
	dctrl.percent = ctrl->percent;
	dctrl.pitch = ctrl->pitch;
	dctrl.duration = ctrl->duration;
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeFeedbackControl, &dctrl);
	}
    }

/****************************************************************************
 *
 * Change acceleration & threshold.
 * The DIX routine that handles the ChangePointerControl request has
 * already validity checked the values and copied them into the
 * DeviceIntRec.  This routine just copies them into fields that are
 * the same no matter what kind of device we're dealing with.
 *
 */

static hpChangePointerControl(pDevice, ctrl)
    DevicePtr pDevice;
    PtrCtrl *ctrl;
    {
    HPInputDevice *d;
    HPPointerFeedbackControl	dctrl;
#ifdef XINPUT
    PtrFeedbackPtr b;

    b = ((DeviceIntPtr) pDevice)->ptrfeed;

    b->ctrl = *ctrl;
#else
    extern int threshold;
    extern float acceleration;

    threshold = ctrl->threshold;
    acceleration = (float) ctrl->num / (float) ctrl->den;
    if (acceleration <= 0)
	acceleration = 1;
#endif /* XINPUT */

    d = GET_HPINPUTDEVICE ((DeviceIntPtr) pDevice);
#ifdef __apollo
    {
    smd_$pos_t pos;
    extern smd_unit_event_data_t olddata;

    if ((DeviceIntPtr) pDevice == inputInfo.pointer)
	{
	pos.column = d->coords[0];
	pos.line = d->coords[1];
	olddata.pos = pos;
	smd_$set_unit_cursor_pos (1, pos, &status);
	}
    }
#endif /* __apollo */
    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dctrl.class = PtrFeedbackClass;
	dctrl.num = ctrl->num;
	dctrl.den = ctrl->den;
	dctrl.threshold = ctrl->threshold;
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeFeedbackControl, &dctrl);
	}
    }

/****************************************************************************
 *
 * Turn LEDs on or off.
 *
 */

static SetLeds (d, leds, mask)
    HPInputDevice *d;
    unsigned int leds, mask;
    {
    int			i, iob;
    char 		ioctl_data[12];

#if defined(__hpux) || defined(__hp_osf)

    if (d->hil_header.iob & HILIOB_PAA)		/* general prompt */
	{
	if (leds & 1)
	    ioctl (d->file_ds, HILP, ioctl_data);
	else
	    ioctl (d->file_ds, HILA, ioctl_data);
	leds >>= 1;
	}

    if (iob = ((u_char) (d->hil_header.iob & HILIOB_NPA) >> 4))/* prompt 1-7 */
	for (i=1; i<=iob; i++)
	    {
	    if (leds & 1)
		ioctl (d->file_ds, prompt[i], ioctl_data);
	    else
		ioctl (d->file_ds, ack[i], ioctl_data);
    	    leds >>= 1;
	    }
#endif /* __hpux */

    }

/****************************************************************************
 *
 * The members of the ledCtrl structure have the following values:
 *
 * mask:	1 bit per LED.
 * value:	if mask set, turn it on or off.
 *
 */

static hpChangeLedControl(pDevice, ctrl)
    DevicePtr pDevice;
    LedCtrl *ctrl;
    {
    HPInputDevice	*d;
    HPLedFeedbackControl	dctrl;

    d = GET_HPINPUTDEVICE ((DeviceIntPtr) pDevice);
    SetLeds(d, ctrl->led_values, ctrl->led_mask);
    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dctrl.class = LedFeedbackClass;
	dctrl.led_values = ctrl->led_values;
	dctrl.led_mask = ctrl->led_mask;
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeFeedbackControl, &dctrl);
	}
    }

/****************************************************************************
 *
 * The members of the keybdCtrl structure have the following values:
 *
 * click:	0(off) - 100 (loud);	-1 => default;
 * bell:	0(off) - 100 (loud); 	-1 => default;
 * bell_pitch:  Pitch of the bell in Hz;-1 => default;
 * bell_duration: in miliseconds;	-1 => default;
 *
 * keyboard_click is checked whenever a key is pressed, in x_hil.c.
 */

static hpChangeKeyboardControl(pDevice, ctrl)
    DevicePtr pDevice;
    KeybdCtrl *ctrl;
    {
    HPInputDevice	*d;
    HPKeyboardFeedbackControl	dctrl;

    if (inputInfo.keyboard &&
        ((DeviceIntPtr) pDevice)->id==inputInfo.keyboard->id)
        keyboard_click = (int)((double)(ctrl->click) * 15.0 / 100.0);

    d = GET_HPINPUTDEVICE ((DeviceIntPtr) pDevice);
    SetAutoRepeat(d, ctrl->autoRepeat);
    SetBellAttributes(d, ctrl);
    SetLeds(d, ctrl->leds, 0xffffffff);
    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	copy_kbd_ctrl_params (&dctrl, ctrl);
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeFeedbackControl, &dctrl);
	}
    }

copy_kbd_ctrl_params (dctrl, ctrl)
    HPKeyboardFeedbackControl	*dctrl;
    KeybdCtrl *ctrl;
    {
    dctrl->class = KbdFeedbackClass;
    dctrl->click = ctrl->click;
    dctrl->bell_percent = ctrl->bell;
    dctrl->bell_pitch = ctrl->bell_pitch;
    dctrl->bell_duration = ctrl->bell_duration;
    dctrl->autoRepeat = ctrl->autoRepeat;
    memmove(dctrl->autoRepeats, ctrl->autoRepeats, 32);
    dctrl->leds = ctrl->leds;
    }

/****************************************************************************
 *
 * hpGetDeviceMotionEvents.
 *
 */

static int hpGetDeviceMotionEvents (dev, coords, start, stop, pScreen)
    DeviceIntPtr  dev;
    CARD32 start, stop;
    xTimecoord  *coords;
    ScreenPtr pScreen;
    {
    HPInputDevice 	*pHPDev = (HPInputDevice *) dev->public.devicePrivate;
    int			i;
    int			evcount = 0;
    int			size = pHPDev->hil_header.ax_num + 1;
    int 		*first, *last, 	*curr;
    int 		*buffp = (int *) coords;
    int 		*pmBuf = dpmotionBuf[dev->id];
    int 		*hmBuf = dheadmotionBuf[dev->id];

    if (pmBuf == hmBuf)
	{
        if (*pmBuf == 0)			/* no events yet           */
	    return 0;
	else
	    last = hmBuf + (99 * size);
	}
    else
	last = pmBuf-size;

    if (*pmBuf == 0)				/* haven't wrapped yet	    */
	first = hmBuf;
    else
	first = pmBuf;

    if (start > *last)				/* start time > last time    */
        return 0;
    else
	{
	curr = first;
	while (*curr < start)
	    {
	    curr += size;
	    if (curr == hmBuf+(100*size))
		curr = hmBuf;
	    if (curr == first)
		return 0;
	    }
	while (*curr <= stop && *curr != 0)
	    {
	    if (dev == inputInfo.pointer)	/*X pointer is 16 bits/axis */
		{
	        *buffp++ = *curr++;		/* copy the time */
	        *buffp++ = *curr << 16 | *(curr+1); /* copy data for 2 axes */
		curr += 2;
		}
	    else				/* other devices are 32 bits */
		for (i=0; i<size; i++)
		    *buffp++ = *curr++;
	    evcount++;
	    if (curr == hmBuf+(100*size))
		curr = hmBuf;
	    if (curr == first)
		break;
	    }
	}
    return (evcount);
    }

/****************************************************************************
 *
 * NOTE: The first parameter passed to this routine is really a DeviceIntPtr.
 *       The declaration used here works because the first element of the    
 *	 structure pointed to by the DeviceIntPtr is a DeviceRec.
 *
 */

static Bool hpDeviceProc(pDev, onoff)
    DevicePtr pDev;
    int onoff;
    {
    char 		*x_basename ();
    int			keyId;
    unsigned int	mask;
    KeySymsRec		*key_syms, keysym_rec;
    CARD8		*the_modmap;
#ifdef __apollo
#define SHORT_STRLEN 4
    char            kbdtypestr[SHORT_STRLEN];
    short           kbdtypestrlen;
    static char     kbdtype, kbdsubtype;
#endif /* __apollo */
    DeviceIntPtr	dev = 		(DeviceIntPtr) pDev;
    HPInputDevice 	*pHPDev = 	(HPInputDevice *) pDev->devicePrivate;
    struct		hil_desc_record	*h = &pHPDev->hil_header;
    int			i;
    int			button_count =	h->v_button_count ?
			                h->v_button_count : 3;
    int			mbufsiz =  (h->ax_num * sizeof(int) + sizeof(Time)) *
				   MOTION_BUFFER_SIZE;
#ifdef XINPUT
    char		*strchr();
#endif /* XINPUT */

    switch (onoff)
        {
	case DEVICE_INIT: 
	    pDev->on = FALSE;
	    pHPDev->pScreen = screenInfo.screens[0];
    	    AssignTypeAndName (pDev, pHPDev->x_atom, x_basename(pHPDev->x_name));

	    if (h->num_keys)
		{
#if defined(__hpux) || defined(__hp_osf)

		if (pHPDev->id_detail & HP_HIL && h->iob & HILIOB_NPA)
		    {
		    keyId = hil_to_kbd_id(h->id + 0x20);
		    pHPDev->id_detail |= PC101_KBD;
		    }
		else
		    keyId = hil_to_kbd_id(h->id);
#endif
#ifdef __apollo
		/*
		 * Detect keyboard type and do initialization accordingly.
		 * Note:
		 *   If the keyboard is "3x" but not one of the known types, its
		 *     probably an ISO keyboard.  The Swedish/Finish keymap is
		 *     a superset of ISO (according to Dan G) so I use that.
		 *   Otherwise, use North American as a default.
		 */

		smd_$inq_kbd_type(SHORT_STRLEN, kbdtypestr, &kbdtypestrlen, &status);
		kbdtype    = (kbdtypestrlen > 0) ? kbdtypestr[0] : '2';
		kbdsubtype = (kbdtypestrlen > 1) ? kbdtypestr[1] : ' ';

 		keyId = 33;	/* assume North American (subtype ' ') */
		if (kbdtype == '3')
		    switch (kbdsubtype)
			{
			case ' ': keyId = 33; break;	/* North American */
			case 'a': keyId = 34; break;	/* German */
			case 'b': keyId = 35; break;	/* French */
			case 'c': keyId = 36; break;	/* Norwegian/Danish */
			case 'd': keyId = 37; break;	/* Swedish/Finish */
			case 'e': keyId = 38; break;	/* UK */
			case 'g': keyId = 39; break;	/* Swiss */
			case 'f': keyId = 40; break;	/* Japanese */
			default:  keyId = 37;		/* unknown - ISO ? */
			}
#endif /* __apollo */

		if (pHPDev->hpflags & IS_SERIAL_DEVICE)
		    {
		    i = pHPDev->sndx;
		    if (!HPKget_kb_info_by_name(serialprocs[i].keymap_file, 
			serialprocs[i].keymap_name, &keysym_rec, &the_modmap))
			FatalError ("Can't find a keymap in the %s/XHPKeymaps file for device %s.\n", LIBDIR, pHPDev->x_name);
		    h->keymap_name = serialprocs[i].keymap_name;
		    }
		else
		    HPKget_maps_by_id(keyId, &keysym_rec, &the_modmap);
		key_syms = &keysym_rec;
		h->min_kcode = key_syms->minKeyCode;
		h->max_kcode = key_syms->maxKeyCode;
		h->num_keys = key_syms->maxKeyCode - key_syms->minKeyCode + 1;
		pHPDev->x_type = KEYBOARD;
		if (dev->id == inputInfo.keyboard->id)
		    {
		    InitKeyboardDeviceStruct(pDev, key_syms, the_modmap, hpBell,
			(KbdCtrlProcPtr) hpChangeKeyboardControl);
		    get_pointerkeys();
		    fix_modifierkeys();
		    }
#ifdef XINPUT
		else
		    {
		    InitKeyClassDeviceStruct (dev, key_syms, the_modmap);
		    InitKbdFeedbackClassDeviceStruct (dev, hpBell,
			(KbdCtrlProcPtr) hpChangeKeyboardControl);
		    InitFocusClassDeviceStruct (dev);
		    }
		}

	    if (h->num_leds && dev->id != inputInfo.pointer->id && 
		dev->id != inputInfo.keyboard->id)
		{
		LedFeedbackPtr led;
		InitLedFeedbackClassDeviceStruct(dev,(LedCtrlProcPtr) hpChangeLedControl);
		mask=0;
		for (i=0; i<h->num_leds; i++)
		    mask |= (1 << i);
		for (led=dev->leds; led; led = led->next)
		    led->ctrl.led_mask = mask;
		}

	    if (pHPDev->hpflags & IS_SERIAL_DEVICE)
		{ 
		int j;
		unsigned char *ptrf;
		HPStrF	*ptrs;

		i = pHPDev->sndx;
		for (j=0, ptrf=serialprocs[i].feedbacks; 
		    j<serialprocs[i].num_fdbk; j++,ptrf++)
		    if (*ptrf == IntegerFeedbackClass)
			InitIntegerFeedbackClassDeviceStruct (
			    dev, (IntegerCtrlProcPtr) hpChangeIntegerControl);
		    else if (*ptrf == BellFeedbackClass)
			InitBellFeedbackClassDeviceStruct (
			    dev, hpBell, (BellCtrlProcPtr) hpChangeBellControl);
		    else if (*ptrf == KbdFeedbackClass)
			InitKbdFeedbackClassDeviceStruct( dev, hpBell, 
			    (KbdCtrlProcPtr) hpChangeKeyboardControl);
		    else if (*ptrf == PtrFeedbackClass)
			InitPtrFeedbackClassDeviceStruct(
			    dev, (PtrCtrlProcPtr) hpChangePointerControl);
		for (j=0, ptrf=serialprocs[i].ledf; 
		     j<serialprocs[i].num_ledf; j++,ptrf++)
		    InitLedFeedbackClassDeviceStruct ( dev, (LedCtrlProcPtr) hpChangeLedControl);
		for (j=0, ptrs=serialprocs[i].strf; 
		     j<serialprocs[i].num_strf; j++, ptrs++)
		    InitStringFeedbackClassDeviceStruct ( dev, 
			(StringCtrlProcPtr) hpChangeStringControl, 
			ptrs->max_symbols,
			ptrs->num_symbols_supported,
			(KeySym *) ptrs->symbols_supported);
		}

#endif /* XINPUT */

	    if (h->ax_num)
		{
		if (dev->id == inputInfo.pointer->id)
		    {
		    if (pHPDev->dev_type == NULL_DEVICE)
			{
	        	pHPDev->coords[0] = pHPDev->pScreen->width;
	        	pHPDev->coords[1] = pHPDev->pScreen->height;
			}
		    else
			{
	        	pHPDev->coords[0] = pHPDev->pScreen->width / 2;
	        	pHPDev->coords[1] = pHPDev->pScreen->height / 2;
			}
#ifdef __apollo
		    smd_$pos_t pos;
		    extern smd_unit_event_data_t olddata;
		
		    pos.column = pHPDev->coords[0];
		    pos.line = pHPDev->coords[1];
		    olddata.pos = pos;
		    smd_$set_unit_cursor_pos (1, pos, &status);
#endif /* __apollo */
		    InitPointerDeviceStruct (pDev, ptr_button_map, button_count,
			hpGetDeviceMotionEvents, (PtrCtrlProcPtr) hpChangePointerControl, 
			    MOTION_BUFFER_SIZE);
		    }
#ifdef XINPUT
		else
		    {
		    InitFocusClassDeviceStruct (dev);
#if defined(__hpux) || defined(__hp_osf)
		    if (h->iob & HILIOB_PIO)
			InitProximityClassDeviceStruct (dev);
#endif /* __hpux */
		    InitValuatorClassDeviceStruct (dev, h->ax_num, 
			hpGetDeviceMotionEvents, 100, 
			(h->flags & HIL_ABSOLUTE)?1:0);
		    InitPtrFeedbackClassDeviceStruct (dev, 
			(PtrCtrlProcPtr) hpChangePointerControl);
		    }
		InitValuatorAxisStruct (dev, 0, 0, (unsigned int) h->size_x, 
		    (unsigned int) h->resx, 0, (unsigned int) h->resx);
		if (h->ax_num > 1)
		    InitValuatorAxisStruct (dev, 1, 0, (unsigned int) h->size_y,
		    (unsigned int) h->resy, 0, (unsigned int) h->resy);
		if (dev->id != inputInfo.pointer->id)
		    for (i=2; i < (u_char) h->ax_num; i++)
			InitValuatorAxisStruct (dev, i, 0, (unsigned int) h->size_x,
			(unsigned int) h->resx, 0, (unsigned int) h->resx);

		dpmotionBuf[dev->id] = (int *) Xalloc (mbufsiz);
		memset (dpmotionBuf[dev->id], 0, mbufsiz);
		dheadmotionBuf[dev->id] = dpmotionBuf[dev->id];
		}

	    if (h->p_button_count && dev->id != inputInfo.pointer->id)
    		InitButtonClassDeviceStruct (dev, button_count, identity_map);
#endif /* XINPUT */
 	    break;
	case DEVICE_ON: 
	    pDev->on = TRUE;
	    if ( pHPDev != NULL) 
		{
#ifndef __hp_osf
        	if (pHPDev->dev_type != NULL_DEVICE)
		    AddEnabledDevice( pHPDev->file_ds );
#endif /* __hp_osf */
		if (h->ax_num)
		    set_scale_and_screen_change (pHPDev);
	        }
	    SetAutoRepeat(pHPDev, TRUE);
	    break;
	case DEVICE_OFF:
	    pDev->on = FALSE;
	    if (dev->button) {
		memset(dev->button->down, 0, DOWN_LENGTH);
		dev->button->state = 0;
		dev->button->buttonsDown = 0;
		pHPDev->button_state = 0;
		pHPDev->ignoremask = 0;
		pHPDev->sent_button = 0;
		pHPDev->savebutton = 0;
	    }
	    if (dev->key) {
		memset(dev->key->down, 0, DOWN_LENGTH);
		dev->key->state = 0;
		dev->key->prev_state = 0;
	    }
	    if (dev->id != inputInfo.pointer->id &&
		pHPDev->file_ds == hpPointer->file_ds)
		break;
	    if (pHPDev != NULL && pHPDev->file_ds >= 0)
		{
#ifndef __hp_osf
		RemoveEnabledDevice(pHPDev->file_ds);
#endif /* __hp_osf */
    	        close_device (pHPDev);
		}
	    break;
	case DEVICE_CLOSE: 
	    if ( pHPDev != NULL && pHPDev->file_ds >= 0)
		{
#ifndef __hp_osf
		RemoveEnabledDevice( pHPDev->file_ds);
#endif /* __hp_osf */
    	        close_device (pHPDev);
		}
#ifdef XINPUT
	    if (dheadmotionBuf[dev->id])
		{
		Xfree (dheadmotionBuf[dev->id]);
		dheadmotionBuf[dev->id] = NULL;
		dpmotionBuf[dev->id] = NULL;
		}
#endif /* XINPUT */
	    if (dev->id == inputInfo.pointer->id)
		close (beeper_fd);
	    break;
	}
    return(Success);
    }

/****************************************************************************
 *
 * InitInput --
 *	Initialize pointer and keyboard devices.
 *
 */

struct	opendevs serial[MAX_DEVICES];
 
void
InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
    {
    int	i, j;
    DeviceIntPtr x_init_device();
    int CheckInput();

    mflg = (char *)getenv("XHPPRINTDEBUGMSG");
    x_axis = 0;
    y_axis = 1;
    axes_changed = FALSE;
    hpPointer = NULL;
    hpKeyboard = NULL;
    hptablet_extension = NULL;
    tablet_width = 0;
    otherndx = 2;
    device_ndx = MAX_POSITIONS - 1;
    for (i=0; i<256; i++)
	identity_map[i]=i;
    for (i=0; i<NUM_DEV_TYPES; i++)
	count[i] = 0;
    if (din_mouse_present())
	count[MOUSE]++;
    if (din_kbd_present())
	count[KEYBOARD]++;
    serial[XPOINTER].name[0]='\0';
    serial[XKEYBOARD].name[0]='\0';
    lockcode = 0;
    uname(&uts_name);

#ifdef __hp_osf
    if (!hil_qp)
    {
    if ((beeper_fd = open(BEEPER_DEVICE,O_RDWR)) < 0)
 	ErrorF ("Unable to open beeper device \"%s\".\n",BEEPER_DEVICE);
#ifdef SPECIAL_68K_OSF
    if ((ioctl (beeper_fd, HILALLOCQ, &hil_qd)) < 0)
	FatalError ("Error allocating HIL event queue.\n");

    if ((int) (hil_qp = (HILQ *) mmap (0, sizeof(HILQ), PROT_READ|PROT_WRITE,
	MAP_FILE|MAP_SHARED, beeper_fd, hil_qd*sizeof(HILQ))) <0)
	FatalError("Unable to map /dev/rhil\n");
#else
    if ((ioctl (beeper_fd, HILALLOCQ, &hil_qp)) < 0)
	FatalError ("Error allocating HIL event queue.\n");
#endif /* SPECIAL_68K_OSF */
    SetInputCheck(&hil_qp->hil_evqueue.head, &hil_qp->hil_evqueue.tail);
    }

    /* discard all the current input events  */
    hil_qp->hil_evqueue.head = hil_qp->hil_evqueue.tail;

    AddEnabledDevice (beeper_fd);
#endif /* __hp_osf */

    RegisterBlockAndWakeupHandlers ((BlockHandlerProcPtr) NoopDDA, 
				    (WakeupHandlerProcPtr) CheckInput, 
				    (pointer) NULL);
    loopnum = atoi(display);
    hilpath[0] = '\0';
    ldigit = '\0';
    get_pointerkeys();
    init_l_devs ();
    init_events_queue ( &ev_queue);
#if defined(__hpux)
    init_beeper();			/* beeper_fd = /dev/rhil */
#endif /* __hpux */

    /*
     * Now initialize the devices as far as X is concerned.
     */

    for (i=0, j=0; i<MAX_DEVICES && j < MAX_LOGICAL_DEVS; j++) 
	{
	if (l_devs[j].hil_header.id == 1 ||		/* inaccessible device*/
	    (l_devs[j].dev_type == NULL_DEVICE && 
	     !(l_devs[j].hpflags & OPEN_THIS_DEVICE)))
		continue;
	if (l_devs[j].file_ds != -1)
	    (void) x_init_device (&l_devs[j], TRUE);
	else
	    (void) x_init_device (&l_devs[j], FALSE);
	i++;
	}
    }

/***********************************************************
 *
 * Perform X initialization for the device.
 *
 */

DeviceIntPtr
x_init_device (dev, start_it)
    HPInputDevice *dev;
    Bool start_it;
    {
    DevicePtr	pXDev;

    pXDev = hpAddInputDevice(hpDeviceProc, start_it, dev);
    if (dev==hpKeyboard)
	{
	RegisterKeyboardDevice(pXDev);
        if (dev->dev_type == KEYBOARD)
	    xhp_kbdid = dev->hil_header.id - 0xA0;
	}
    else if (dev==hpPointer)
	{
	RegisterPointerDevice(pXDev);
#ifdef SPECIAL_68K_OSF
	miRegisterPointerDevice(screenInfo.screens[0], pXDev);
#endif
	if (dev->x_type == KEYBOARD)
	    InitKbdFeedbackClassDeviceStruct ((DeviceIntPtr) pXDev, hpBell,
		(KbdCtrlProcPtr) hpChangeKeyboardControl);
	screen_change_dev = (DeviceIntPtr) pXDev;
	if (screen_change_amt == SCREEN_CHANGE_DEFAULT)
	    if (dev->hil_header.flags & HIL_ABSOLUTE)
		screen_change_amt = 0;
	    else
		screen_change_amt = 30;
	}
#ifdef XINPUT
    else
    	{
	RegisterOtherDevice(pXDev);
	if (tablet_width && dev->file_ds==hpPointer->file_ds)
	    {
	    tablet_extension_device = (DeviceIntPtr) pXDev;
	    hptablet_extension = dev;
	    screen_change_dev = tablet_extension_device;
	    }
	}
#endif /* XINPUT */

    return ((DeviceIntPtr) pXDev);
    }

/*****************************************************************
 *
 * Initialize the l_devs array of structures.
 * There is one per logical input device.
 *
 */
 
init_l_devs()
    {
    int		i;
    int		dev_num = 2;
    int		sdev_num = 2;
    FILE	*fp;
    char	fname[MAXNAMLEN];
    struct	opendevs opendevs [MAX_LOGICAL_DEVS];

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	opendevs[i].type = -1;
	opendevs[i].pos = -1;
	opendevs[i].name[0] = '\0';
	opendevs[i].path[0] = '\0';
	}

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	memset(&l_devs[i], 0, sizeof(HPInputDevice));
        l_devs[i].hil_header.id = 1;
        l_devs[i].hil_header.keymap_name = "";
        l_devs[i].mode = ABSOLUTE;
	l_devs[i].file_ds = -1;
	l_devs[i].id_detail = HP_HIL;
#if defined(__hpux) || defined(__hp_osf)
        l_devs[i].repeat_rate = HILER1;
#endif /* __hpux */
	}

    sprintf(fname, "%s/X%sdevices",LIBDIR,display);
    fp = fopen ( fname, "r");
    if (fp) 
	{
        dev_num = device_files (fp, opendevs, &sdev_num);
	fclose (fp);
	}
    compute_device_names (opendevs, dev_num);

#if defined(__hpux) || defined(__hp_osf)
    init_hil_devs (opendevs, dev_num);
#endif /* __hpux */
#ifdef __apollo
    init_apollo_devs (opendevs, dev_num);
#endif /* __apollo */

    /*
     * Check for any dynamically loaded input device drivers.
     */

    init_dynamic_devs (opendevs, sdev_num);

    if (hpPointer->x_type == KEYBOARD)
	{
	hpPointer->hil_header.v_button_count = 8;
	hpPointer->hil_header.p_button_count = 8;
	hpPointer->hil_header.ax_num = 2;
	}
    }

/********************************************************************
 *
 * Compute the names of the input devices we should use.
 * If a path for the input devices has been specified, use it.
 * Otherwise use /dev/hil.
 * If we have multiple HIL loops (series 800), and the display number
 * is between 0 and 3, use the corresponding loop.  Otherwise, search
 * all loops.
 *
 */

compute_device_names (opendevs, dev_num)
    struct	opendevs opendevs [];
    int		dev_num;
    {
    int		ndx = MAX_POSITIONS - 1;
    int		i;
    int 	hlen = strlen(hilpath);

#if defined(__hp9000s800) && !defined(__hp9000s700)

    if (hlen > 0 && isdigit (hilpath[hlen-1]))	/* hilpath ends in digit */
	{
	ldigit = hilpath[hlen-1];
	hilpath[hlen-1] = '\0';
	}
    else if (loopnum >= 0 && loopnum < 4)	/* X invoked with display # */
	ldigit = display[0];
#endif /* __hp9000s800 */

    if (hlen > 0)
	allocated_dev_names = TRUE;
    else
	allocated_dev_names = FALSE;

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	if (hlen > 0 && i<MAX_POSITIONS)
	    {
	    if (allocated_dev_names == TRUE)
		Xfree (dev_names[i]);
	    dev_names[i] = (char *) Xalloc (strlen (hilpath) + 4);
	    if (ldigit == '\0' || i < 7)				
		{
		strcpy (dev_names[i], hilpath);
		strcat (dev_names[i], suffix[i]);
		}
	    }
	else
	    dev_names[i] = default_names[i];
	}

#if defined(__hp9000s800) && !defined(__hp9000s700)
    if (ldigit != '\0')
        for (i=0; i<MAX_POSITIONS; i++)
	    {
	    if (i < 7)				
		{
		suffix [i][0] = ldigit;
		dev_names[i][9] = ldigit;
		}
	    else
		{
		dev_names[i][0] = '\0';
		suffix [i][0] = '\0';
		}
	    }
#endif /* __hp9000s800 */

    while (--dev_num >= 0)
	{
	if (opendevs[dev_num].path[0] == '\0')
	    continue;
	for (i=0; i<MAX_POSITIONS; i++)
	    if (strcmp (opendevs[dev_num].path, dev_names[i]) == 0)
		break;
	if (i==MAX_POSITIONS)
	    strcpy (dev_names[ndx--], opendevs[dev_num].path);
	   
	}
    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	strcpy (l_devs[i].dev_name,dev_names[i]);
    }

/********************************************************************
 *
 * Find the requested key and pointer devices.
 * If no key or pointer device was named, find a default one.
 *
 */

#define DEFAULT_DEV 	0
#define EXPLICIT_DEV	1

#if defined(__hpux) || defined(__hp_osf)
static int init_hil_devs (opendevs, numdev)
    struct	opendevs opendevs [];
    int		numdev;
    {
    Bool OnlyOpenExplicit = FALSE;
    int	i, j;
    HPInputDevice	*d, *last_mouse=NULL, *last_pointer=NULL, 
			*last_keyboard=NULL, *last_key_device=NULL;

/*****************************************************************************
 *
 * Attempt to open all devices and find out what they are.
 * Find out which will be the default devices.
 * Count them so that we can assign names by position.
 * A device that can't be opened is considered not present.
 *
 */
    if (opendevs[XKEYBOARD].path[0] != '\0' &&
        opendevs[XPOINTER].path[0] != '\0')
	OnlyOpenExplicit = TRUE;

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	{
	d = &l_devs[i];
	if (OnlyOpenExplicit)
	    {
	    for (j=0; j<numdev; j++)
		if (strcmp (opendevs[j].path, d->dev_name) == 0)
		    break;
	    if (j==numdev)
		continue;
	    }
	if (open_device (d) >= 0)
	    {
	    for (j=0; j<numdev; j++)
		{
	        if ((d->dev_type == opendevs[j].type && 
		     count[d->dev_type]==opendevs[j].pos) ||
		    (opendevs[j].type == -1 && 
		     strcmp (opendevs[j].path, d->dev_name) == 0))
		    {
		    d->hpflags |= OPEN_THIS_DEVICE;
		    if (j==XKEYBOARD && hpKeyboard==NULL){
			hpKeyboard = d;
			hpKeyboard->pad2 = EXPLICIT_DEV;
			}
		    else if (j==XPOINTER && hpPointer==NULL){
			hpPointer = d;
			hpPointer->pad2 = EXPLICIT_DEV;
			}
		    else
			{
			d->hpflags |= MERGED_DEVICE;
			d->open_cnt=1;
			}
		    }
		}
	    count[d->dev_type]++;
	    if (d->dev_type == MOUSE)
		last_mouse = d;
	    else if (d->dev_type == KEYBOARD)
		last_keyboard = d;
	    else if (d->x_type == KEYBOARD)
		last_key_device = d;
	    else if (d->x_type == MOUSE)
		last_pointer = d;
	    }
	}

/*****************************************************************************
 *
 * If the user didn't pick a keyboard and pointer, assign a default.
 * If present, defaults are the last keyboard and last mouse.
 *
 */

    if (hpKeyboard==NULL)
	{
        if (last_keyboard != NULL)
	    hpKeyboard = last_keyboard;
	else if (last_key_device != NULL)
	    hpKeyboard = last_key_device;
	else {
	    for (i=0; i<MAX_LOGICAL_DEVS; i++)
		if (l_devs[i].file_ds != -1 &&
		    !(l_devs[i].hpflags & OPEN_THIS_DEVICE))
		    close_device (&l_devs[i]);
	    return;				/* must be serial keyboard */
	    }
	hpKeyboard->hpflags |= OPEN_THIS_DEVICE;
	hpKeyboard->hpflags &= ~MERGED_DEVICE;
	hpKeyboard->pad2 = DEFAULT_DEV;
	}

    if (hpPointer==NULL)
	{
        if (last_mouse != NULL)
	    hpPointer = last_mouse;
	else if (last_pointer != NULL)
	    hpPointer = last_pointer;
	else
	    hpPointer = hpKeyboard;
	hpPointer->hpflags |= OPEN_THIS_DEVICE;
	hpPointer->hpflags &= ~MERGED_DEVICE;
	hpPointer->pad2 = DEFAULT_DEV;
	}

    if (hpPointer == hpKeyboard)
	{
	hpKeyboard->hpflags |= SECOND_LOGICAL_DEVICE;
	l_devs[spare] = *hpKeyboard;
	hpPointer = &l_devs[spare];
	}

/*****************************************************************************
 *
 * If tablet subsetting specified and the pointer is not a tablet,
 * force the last tablet (if there is one) to be the pointer. 
 * The tablet must also be accessible as an extension device.
 *
 */

    if (tablet_width)
	{
	if (hpPointer->dev_type != TABLET)
	    {				   
	    for (i=MAX_LOGICAL_DEVS-1; i>=0; i--)
	        if (l_devs[i].dev_type == TABLET)
		    break;
	    if (i>=0)
		{
	        hpPointer->hpflags &= ~OPEN_THIS_DEVICE;
		hpPointer = &l_devs[i];
	        hpPointer->hpflags |= OPEN_THIS_DEVICE;
		l_devs[spare] = *hpPointer; /* will also be extension device */
	        l_devs[spare].hpflags |= SECOND_LOGICAL_DEVICE;
	        l_devs[spare].open_cnt = 1;
		}
	    }
	else
	    {
	    l_devs[spare] = *hpPointer; /* will also be an extension device */
	    l_devs[spare].hpflags |= SECOND_LOGICAL_DEVICE;
	    l_devs[spare].open_cnt = 1;
	    }
	}

/***********************************************************************
 *
 * Now close all the devices that X was not instructed to use.
 *
 */

    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	if (l_devs[i].file_ds != -1 &&
	    !(l_devs[i].hpflags & OPEN_THIS_DEVICE))
	    close_device (&l_devs[i]);

    }
#endif /* __hpux */

/***********************************************************************
 *
 * Open the beeper device.
 * On s800 machines, this is /dev/hilkbd#, where # is 0...3.
 * On s300 and s700, this is /dev/rhil.
 *
 */

#if defined(__hpux)

init_beeper()
    {

#if defined(__hp9000s300) || defined(__hp9000s700)
    if ((beeper_fd = open(BEEPER_DEVICE,O_RDWR)) < 0 &&
	((beeper_fd = open("/dev/beep",O_RDWR)) < 0) &&
	((strcmp(uts_name.machine, "9000/7LC") != 0) &&
	 (strcmp(uts_name.machine, "9000/712") != 0)))
 	    ErrorF ("Unable to open beeper device \"%s\".\n",BEEPER_DEVICE);
#endif /*__hp9000s300 or __hp9000s700 */

#if defined(__hp9000s800) && !defined(__hp9000s700)
    int		len;

    if (ldigit != '\0')
	beeper_name[11] = ldigit;
    else 
	{
        len = strlen (hpKeyboard->dev_name);
	beeper_name[11] = hpKeyboard->dev_name[len-3];
	}
    if ((beeper_name[11] >= '0' && beeper_name[11] < '4') &&
        (beeper_fd = open(beeper_name,O_RDWR)) < 0)
	ErrorF ("Unable to open beeper device \"%s\".\n",beeper_name);
#endif /*__hp9000s800 && !__hp9000s700 */

    }
#endif /* __hpux */

/***********************************************************************
 *
 * Initialize Domain input devices.
 *
 */

#ifdef __apollo
static int init_apollo_devs (opendevs, numdev)
    struct	opendevs opendevs [];
    int		numdev;
    {
    if (!fdApollo)
        fdApollo = MakeSMDStream();
    strcpy (l_devs[0].dev_name, "Apollo_internal");
    strcpy (l_devs[1].dev_name, "Apollo_internal");

    l_devs[1].x_type = KEYBOARD;
    l_devs[1].dev_type = KEYBOARD;
    l_devs[1].hil_header.id = 0xdf;
    l_devs[0].hil_header.num_keys = 113;
    strcpy(l_devs[1].x_name,"FIRST_KEYBOARD");
    l_devs[1].x_atom = MakeAtom ("KEYBOARD", 8, 0);
    l_devs[1].file_ds = fdApollo;

    l_devs[0].x_type = MOUSE;
    l_devs[0].dev_type = MOUSE;
    l_devs[0].hil_header.id = 0x68;
    l_devs[0].hil_header.ax_num = 2;
    l_devs[0].hil_header.p_button_count = 3;
    l_devs[0].hil_header.v_button_count = 5;
    l_devs[0].hil_header.size_x = screenInfo.screens[0]->width;
    l_devs[0].hil_header.size_y = screenInfo.screens[0]->height;
  
    strcpy(l_devs[0].x_name,"FIRST_MOUSE");
    l_devs[0].x_atom = MakeAtom ("MOUSE", 5, 0);
    l_devs[0].file_ds = fdApollo;

    if (opendevs[XPOINTER].type == KEYBOARD)
	{
	l_devs[1].hpflags |= SECOND_LOGICAL_DEVICE;
	l_devs[MAX_LOGICAL_DEVS-2] = l_devs[1];
	l_devs[0].file_ds = -1;
	hpPointer = &l_devs[MAX_LOGICAL_DEVS-2];
	}
    else if (hpPointer==NULL || open_device(hpPointer) < 0)
	hpPointer = &l_devs[0];
    else
	{
	l_devs[0].hil_header.id = 1;
	l_devs[0].file_ds = -1;
	}

    if (hpKeyboard==NULL || open_device(hpKeyboard) < 0)
	hpKeyboard = &l_devs[1];
    }
#endif /* __apollo */

/****************************************************************************
 *
 * open_device opens one of the input devices.
 * The dev_name is filled in by device_files(), or is the default.
 * If the open fails, it may be because the keyboard and pointer
 * are the same device, and the device is already open.
 *
 */

HPInputDeviceHeader zdhdr;

open_device (d)
    HPInputDevice	*d;
    {
    HPInputDeviceHeader dhdr;
    int		fd;

#ifdef __apollo
    if (!strcmp (d->dev_name, "Apollo_internal"))
	fd = fdApollo;
    else
#endif /* __apollo */
    d->led[NLOCK] = LockMask;
    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dhdr = zdhdr;
	strcpy (dhdr.path, d->dev_name);
	dhdr.file_ds = HP_VENDOR_RELEASE_NUMBER;
	if (button_chording == CHORDING_ON)
	    dhdr.button_chording = 100;
	if ((*(serialprocs[d->sndx].configure)) (&dhdr) != 
	    INIT_SUCCESS)
	    return(-1);
	serialprocs[d->sndx].fd = fd = dhdr.file_ds;
	}
    else
	{
	fd = open (d->dev_name, O_RDWR | O_NDELAY);
	if (fd < 0) 
	    return (fd);

	if (get_device_details (fd, d) < 0)
	    return (-1);
	}

    d->file_ds = fd;
    if (fd > max_input_fd)
	max_input_fd = fd;

#ifdef __hp_osf
#ifdef SPECIAL_68K_OSF
    if (d->dev_type != NULL_DEVICE &&
        ioctl (d->file_ds, HILMAPQ, &hil_qd) < 0)
	FatalError ("HILMAPQ failed for device %s\n",d->dev_name);
#else
    if (d->dev_type != NULL_DEVICE &&
        ioctl (d->file_ds, HILMAPQ, &hil_qp->hil_evqueue.qnum) < 0)
	{
	FatalError ("HILMAPQ failed for device %s, file_ds=%d errno=%d\n",
	    d->dev_name, d->file_ds, errno);
	}
#endif /* SPECIAL_68K_OSF */
#endif /* __hp_osf */

    return (fd);
    }

/****************************************************************************
 *
 * Query the hil device for detailed information.
 *
 */

static int get_device_details(file_ds, input_dev)
    int file_ds;
    HPInputDevice *input_dev;
    {
    int 	i, dev_status;
    u_char	describe[11], iob;
    struct	hil_desc_record		*hd;
    int		hi_resol =0;
    int 	lo_resol = 0;
    int 	support_it = TRUE;
#if defined(__hpux) || defined(__hp_osf)

    for (i=0; i<11; i++)
	describe[i] = 0;
    dev_status = ioctl (file_ds, HILID, &(describe[0]));
    hd = &(input_dev->hil_header);

    if (dev_status >= 0) 
	{
	hd->id  = describe[0];
	if (hd->id >= 0xE0)			/* HP98203C - not supported */
	    {
	    close (file_ds);
	    return (-1);
	    }
	else if (hd->id >= 0xA0 && hd->id < 0xC0) /* compressed keyboard      */
	    {
	    close (file_ds);
	    return (-1);
	    }

	hd->flags = describe[1];
	input_dev->hpflags |= DATA_IS_8_BITS;
	hd->ax_num = (hd->flags & HIL_NUM_AXES);

	/*
	 *
	 * if # of axes indicate it is a positional device
	 * then gather resolution.	
	 * if 16 bits of information are reported, resolution is
	 * in counts/ cm.  In this case, convert to counts/ meter.
	 *
	 */

	if ( hd->ax_num) 
	    {
	    lo_resol =  describe[2];
	    hi_resol =  describe[3];
	    hd->resx = hd->resy = (hi_resol << 8) + lo_resol;
	    if (hd->flags & HIL_16_BITS)
		{
		input_dev->hpflags |= DATA_IS_16_BITS;
		hd->resx = hd->resy =  hd->resx * 100;
		}
	    /* If it is an absolute device, gather size */
	    if (hd->flags & HIL_ABSOLUTE)
		{
		switch ( hd->ax_num) 
		    {
		    case 2:
		        hd->size_y = (int)describe[6]|((int)describe[7] << 8);
		    case 1:
			 hd->size_x = (int)describe[4]|((int)describe[5] << 8);
		    default:
			 break;
		    }
		iob = describe[4 + hd->ax_num * 2];
		}
	    else
		iob = describe[4];
	    }
	else 
	    {
	    iob = describe[2];
	    hd->resx = hd->resy = 0;
	    }
		   
	if (hd->flags & HIL_IOB)
	    hd->iob=iob;
	if (hd->iob & HILIOB_BUTTONS) 
	   {
	   hd->p_button_count = hd->iob & HILIOB_BUTTONS ;
	   hd->v_button_count = hd->iob & HILIOB_BUTTONS ;

	   /*
	    * initialize structures for emulating 3 buttons
	    * where we have 2, or 5 buttons where we have 3.
	    */

	    if (hd->p_button_count == 2)  
	       hd->v_button_count = 3;  
	    else if (hd->p_button_count == 3 || hd->p_button_count == 4)  
	       hd->v_button_count = 5;  
	   }
        if (hd->iob & HAS_LEDS)
	    {
	    hd->num_leds = hd->iob & HILIOB_NPA;
	    if (!hd->num_leds)
	        hd->num_leds=1;
	    }
        get_device_type (input_dev, hd->id);
	}
    else
	{
	hd->size_x = hd->size_y = 1000;
	hd->resx = hd->resy = 1000;
	hd->ax_num = 2;
	hd->p_button_count = 3;
	hd->v_button_count = 3;
	hd->min_kcode = 10;
	hd->max_kcode = 135;
	hd->num_keys = 109;
	input_dev->hil_header.id = 0xd5;
        input_dev->dev_type = NULL_DEVICE;
        input_dev->x_type = XOTHER;
	strcpy (input_dev->x_name,"FIRST_NULL");
	support_it = FALSE;
	}
#endif /* __hpux */
    return ( support_it);
    }

/****************************************************************************
 *
 * This routine determines the type of the input device.
 * dev_type is the actual type, x_type is what X considers it to be
 * (mouse or keyboard).
 * The 9-knob box and quadrature box have the same HIL id.
 * But if it doesn't have 3 axes, it's not a 9-knob box.
 *
 */

static
get_device_type (dev, id)
    HPInputDevice *dev;
    int	id;
    {
    char	*nptr;
    int		i, dev_count;

    for (i=0; devices[i].dev_type != NULL_DEVICE; i++)
	if (id >= devices[i].lowid && id <= devices[i].highid)
	    {
    	    if (id == NINE_KNOB_ID && dev->hil_header.ax_num != 3)
		i = QUAD_INDEX;
	    dev->hil_header.min_kcode = devices[i].min_kcode;
	    dev->hil_header.max_kcode = devices[i].max_kcode;
	    dev->hil_header.num_keys = devices[i].num_keys;
	    dev->dev_type = devices[i].dev_type;
	    dev->x_type    = devices[i].x_type;
	    dev_count = count [dev->dev_type];
	    strcpy (dev->x_name, position[dev_count]);
	    strcat (dev->x_name, "_");
	    if (nptr = strrchr(devices[i].name,'-'))
	        strcat (dev->x_name, (nptr+1));
	    else 
	        strcat (dev->x_name, devices[i].name);
	    dev->x_atom = MakeAtom (devices[i].name, strlen(devices[i].name),0);
	    break;
	    }
    }

/****************************************************************************
 *
 * This routine recalculates the device x_name.
 * The x_name is a string created by concatenating the device type and position.
 * The position may change if a device that was previously inaccessible
 * to X is made accessible.
 *
 */

recalculate_x_name ()
    {
    int	i;
    int	j;

    for (i=0; i<NUM_DEV_TYPES; i++)
	count[i] = 0;
    
    for (i=0; i<MAX_LOGICAL_DEVS; i++)
	for (j=0; j<MAX_LOGICAL_DEVS; j++)
	    if (strcmp (l_devs[j].dev_name,"/dev/null") == 0)
		continue;
	    else if (strcmp (dev_names[i], l_devs[j].dev_name) == 0)
		{
		if (l_devs[j].file_ds != -1)
		    {
		    if (l_devs[j].hpflags & SECOND_LOGICAL_DEVICE)
			continue;
		    get_device_type (&l_devs[j], l_devs[j].hil_header.id);
		    count [l_devs[j].dev_type]++;
		    }
		else if (open_device (&l_devs[j]) > 0)
		    {
		    count [l_devs[j].dev_type]++;
		    close_device(&l_devs[j]);
		    }
		else 
		    l_devs[j].x_name[0] = '\0';
		break;
		}
    }

/****************************************************************************
 *
 * SetAutoRepeat (onoff)
 *  Enable or disable the auto repeat feature of the specified device.
 */

static void SetAutoRepeat (d, onoff)
    HPInputDevice	*d;
    int onoff;
    {
#if defined(__hpux) || defined(__hp_osf)
    char ioctl_data[12];
    int state = HILDKR;
	
    if (d->hpflags & IS_SERIAL_DEVICE)
	return;
    if (onoff)
	state = d->repeat_rate;

    if (d->file_ds != -1)
	{
	ioctl (d->file_ds, state, ioctl_data);
	}
#endif /*__hpux */
    }

/********************************************************************
 *
 * If the file LIBDIR/X[display#]devices exists, this routine 
 * processes it.
 * It translates the strings in the file to a device type and relative
 * position on the HIL.
 *
 */


static int device_files (fd, opendevs, sdev_num)
    FILE	*fd;
    struct	opendevs opendevs [];
    int		*sdev_num;
    {
    char buf[MAXNAMLEN+1];
    char devuse[MAXNAMLEN+1];
    char path[MAXNAMLEN+1];
    char pos[MAXNAMLEN+1];
    char *fgets();
    int	i;
    int	other = XOTHER;
    int	sother = XOTHER;
    int parms;

    while (fgets(buf,MAXNAMLEN+1,fd) != NULL)
	{
	buf[strlen(buf)-1] = '\0';
	if (other == MAX_LOGICAL_DEVS)
	    {
	    ErrorF ("Too many X*devices entries. Ignoring \"%s\".\n",buf);
	    continue;
	    }
	pos[0] = '\0';
	path[0] = '\0';
	devuse[0] = '\0';
	parms = sscanf (buf, "%s%s%s", pos, path, devuse);

	if (pos[0] == '#')	/* comment, skip it */
	    continue;
	else if (path[0] == '#')/* 1 parm           */
	    parms = 1;
	else if (devuse[0] == '#')/* 2 parms        */
	    parms = 2;

	if (parms == EOF)		/* blank line            */
	    continue;
	else if (parms == 1)
	    {
	    for (i=0; i<strlen(pos); i++)
		pos[i] = toupper(pos[i]);
	    if (strcmp (pos,"BEGIN_DEVICE_DESCRIPTION") == 0)
		parse_description (fd, serial, &sother, sdev_num);
	    else
		{
		ErrorF("Invalid X*devices entry: \"%s\" - Entry skipped\n",buf);
		continue;
		}
	    }
	else if (parms == 2)	/* device name specified */
	    parse_2_parms (pos, path, opendevs, &other);
	else if (parms ==3)
	    parse_3_parms (pos, path, devuse, opendevs, &other);

	}
    return (other);
    }

/***********************************************************************
 *
 * This routine is invoked when two parameters are specified.
 * Either they are a device path and intended use, or a device loop path.
 *
 */

parse_2_parms (dev, use, o, ondx)
    char *dev;
    char *use;
    struct	opendevs o[];
    int  *ondx;
    {
    int i;
    int ndx;
    int len = strlen(use);

    for (i=0; i<len; i++)
	use[i] = toupper(use[i]);

    if (strcmp (use,"HIL_PATH") == 0)
	{
	strcpy (hilpath,dev);
	return;
	}
    else if (strcmp (use, "POINTER") == 0)
	ndx = XPOINTER;
    else if (strcmp (use, "KEYBOARD") == 0)
	ndx = XKEYBOARD;
    else if (strcmp (use, "OTHER") == 0)
	ndx = (*ondx)++;
    else
	{
	ErrorF ("Bad device use \"%s\" in X*devices file - Entry skipped.\n",
		use);
	return;
	}

    o[ndx].type = -1;
    o[ndx].pos = -1;
    strcpy (o[ndx].path, dev);
    }

/***********************************************************************
 *
 * This routine is invoked when three parameters are specified.
 * They are a position, a device type, and its intended use.
 *
 */

parse_3_parms (pos, name, use, o, ondx)
    char *pos;
    char *name;
    char *use;
    struct	opendevs o[];
    int  *ondx;
    {
    int i;
    int ndx;

    for (i=0; i<strlen(pos); i++)
	pos[i] = toupper(pos[i]);
    for (i=0; i<strlen(use); i++)
	use[i] = toupper(use[i]);

    if (strcmp (use, "POINTER") == 0)
	ndx = XPOINTER;
    else if (strcmp (use, "KEYBOARD") == 0)
	ndx = XKEYBOARD;
    else if (strcmp (use, "OTHER") == 0)
	ndx = *ondx;
    else
	{
	ErrorF ("Bad device use \"%s\" in X*devices file - Entry skipped.\n",
	    use);
	return;
	}

    for (i=0; i<MAX_POSITIONS; i++)
	if (strcmp (position[i], pos) == 0)
	    {
	    o[ndx].pos = i;
	    break;
	    }

    if (i == MAX_POSITIONS) /* failed, skip to next */
	{
	ErrorF ("Bad ordinal \"%s\" in X*devices file - Entry skipped.\n",
	    pos);
	return;
	}

    for (i=0; i<strlen(name); i++)
	name[i] = toupper(name[i]);

    for (i=0; i<MAX_DEV_TYPES; i++)
	if (strcmp (devices[i].name,name) == 0)
	    {
	    o[ndx].type = devices[i].dev_type;
	    break;
	    }

    if (i == MAX_DEV_TYPES) /* failed, skip to next */
	{
	ErrorF ("Bad device type \"%s\" in X*devices file - Entry skipped.\n",
		name);
	return;
	}
    else if (ndx == *ondx)
	(*ondx)++;
    }

/********************************************************************
 *
 *
 *
 *
 */

parse_description(fd, o, ondx, sdev_num)
    FILE	*fd;
    struct	opendevs o[];
    int  	*ondx;
    int  	*sdev_num;
    {
    int		ndx = -1, i, len;
    char buf[256], ubuf[256], name[256], uname[256], path[256], var[64], 
	use[64], entry[64];
    char *fgets();

    name[0] = '\0';
    entry[0] = '\0';
    while (fgets(buf,256,fd) != NULL)
	{
	len = strlen(buf);
	for (i=0; i<len; i++)
	    ubuf[i] = toupper(buf[i]);
	if (sscanf (ubuf,"PATH %s", var) == 1)
	    sscanf (buf,"%s %s", var, path);
	else if (sscanf (ubuf,"NAME %s", uname) == 1)
	    sscanf (buf,"%s %s", var, name);
	else if (sscanf (ubuf,"ENTRYPOINT %s", var) == 1)
	    sscanf (buf,"%s %s", var, entry);
	else if (sscanf (ubuf,"USE %s",use) == 1)
	    {
	    if (!strcmp(use,"POINTER"))
		{
		ndx = XPOINTER;
		o[ndx].use = XPOINTER;
		}
	    else if (!strcmp(use,"KEYBOARD"))
		{
		ndx = XKEYBOARD;
		o[ndx].use = XKEYBOARD;
		}
	    else if (!strcmp(use,"OTHER"))
		{
		ndx = (*ondx)++;
		o[ndx].use = XOTHER;
		}
	    else	/* assume used as extension device */
		{
		ndx = (*ondx)++;
		o[ndx].use = XOTHER+1;
		}
	     }
	else if (sscanf (ubuf," %s",var) == 1 &&
		 strcmp (var,"END_DEVICE_DESCRIPTION")==0)
	    {
	    if (device_ndx < 0)
		{
		ErrorF("Too many devices in X*devices file - entry skipped.\n");
		return;
		}
	    if (ndx != -1 && path)
		{
		o[ndx].type = 99;
		strcpy (o[ndx].path, path);
		strcpy (o[ndx].name, name);
		if (entry[0])
		    strcpy (o[ndx].entry, entry);
		else
		    {
		    len = strcspn (name,".");
		    strncpy (o[ndx].entry, name, len);
		    o[ndx].entry[len] = '\0';
		    strcat (o[ndx].entry, "_Init");
		    }
		(*sdev_num)++;
		}
	    return;
	    }
	else
	    {
	    ErrorF ("Invalid line in device description - ignored.\n");
	    ErrorF ("line was: %s",buf);
	    }
	}
    ErrorF("No END_DESCRIPTION line in X*devices file - description skipped.\n");
    }

/********************************************************************
 *
 * get_codes()
 * Used to assign codes to keys used to move the pointer.
 * Also to assign numbers to the amount to move the pointer.
 * This routine uses the index into the file to determine the keycode.
 * The down keycode is (index * 2), the up keycode is that plus 1.
 * If the type is NUMBER, the key string is assumed to be an ascii
 * representation of a number.
 * This is used as the increment to move the pointer.
 *
 */

static get_codes (key, code, type, makeupper)
    char	*key;
    int		*code;
    int		type;
    Bool	makeupper;
    {
    int		i;
    KeySym	sym;

    if (makeupper)
	for (i=0; i<strlen(key); i++)
	    *(key+i) = toupper(*(key+i));

    if (type == UCHAR_NUMBER || type == USHORT_NUMBER || type == UINT_NUMBER)
	{
	*code = atoi (key);
	return (0);
	}
    else if (type == STRING)
	for (i=0; i<MAX_STRINGS; i++)
	    if (strcmp (key, strings[i].string) == 0)
		{
		*code = strings[i].value;
		return (0);
		}
    for (i=0; keyset1[i].keystr; i++)
        if (strcmp (key, keyset1[i].keystr) == 0)
	    break;
    if (!inputInfo.keyboard)
	return(0);
    *code = HPKeySymToKeyCode(inputInfo.keyboard, keyset1[i].sym);
    if (*code)
	return(0);
    return (-1);
    }

int
HPKeySymToKeyCode (dev, sym)
    DeviceIntPtr dev;
    KeySym sym;
{
    int i;
    register KeySym *syms;
    KeySymsPtr rec;

    rec = &dev->key->curKeySyms;
    syms = rec->map;
    for (i=0; i<rec->maxKeyCode - rec->minKeyCode + 1; i++)
	{
	if (*syms == sym)
	    return (i + rec->minKeyCode);
	syms += rec->mapWidth;
	}
}

/********************************************************************
 *
 * get_vars()
 * get the address of variables to contain keycodes for pointer functions.
 *
 */

static get_vars (func, codevar, index)
    char	*func;
    u_char	**codevar;
    int		*index;
    {
    int		i;
    
    for (i=0; i<strlen(func); i++)
	*(func+i) = toupper(*(func+i));

    for (i=0; i<MAX_POINTER_FUNCS; i++)
        if (strcmp (func, pointerfunc[i].name) == 0)
	    {
	    *codevar = pointerfunc[i].code;
	    *index = i;
	    return (0);
	    }
    return (-1);
    }

/********************************************************************
 *
 * get_pointerkeys().
 * This routine provides the ability to configure keyboard keys to 
 * move the pointer and act like buttons on the pointer device.
 * The file processed is the X*pointerkeys file, which consists
 * of pairs.  The form is:
 *
 * 	function	key, modifier, or value
 *
 * Look at the pointerfunc table in x_hilinit.h to understand this code.
 * There are 3 types of assignment done:
 * 	1). keys - have both a down and an up code to assign.
 *	2). modifiers - are a bit position in a mask.
 *	3). values - are a single integer number.
 * Possible errors:
 *	1). only 1 of the pair was specified.
 *	2). an invalid function was specified.
 *	3). an invalid key or modifier was specified.
 */

get_pointerkeys()
    {
    char	fname[MAXNAMLEN+1];
    FILE	*fp;
    int 	len;
    int 	cret;
    int 	vret;
    int 	ret2;
    int 	index;
    char 	buf[MAXNAMLEN+1];
    char 	function[MAXNAMLEN+1];
    char 	key[MAXNAMLEN+1];
    char 	*fgets();
    int		code;
    union
	{
	u_char 	*cptr;
	u_short *sptr;
	u_int 	*iptr;
	} codevar;

    if (inputInfo.keyboard) {
	get_codes ("KEYPAD_2", &code, KEY, FALSE);
	cursor_down = (unsigned char) code;
	get_codes ("KEYPAD_5", &code, KEY, FALSE);
	cursor_up = (unsigned char) code;
	get_codes ("KEYPAD_1", &code, KEY, FALSE);
	cursor_left = (unsigned char) code;
	get_codes ("KEYPAD_3", &code, KEY, FALSE);
	cursor_right = (unsigned char) code;
	get_codes ("KEYPAD_*", &code, KEY, FALSE);
	button_1 = (unsigned char) code;
	get_codes ("KEYPAD_/", &code, KEY, FALSE);
	button_2 = (unsigned char) code;
	get_codes ("KEYPAD_+", &code, KEY, FALSE);
	button_3 = (unsigned char) code;
	get_codes ("KEYPAD_-", &code, KEY, FALSE);
	button_4 = (unsigned char) code;
	get_codes ("KEYPAD_7", &code, KEY, FALSE);
	button_5 = (unsigned char) code;
	}
    sprintf(fname, "%s/X%spointerkeys",LIBDIR,display);
    fp = fopen ( fname, "r");
    if (fp == NULL)
	return;

    while (fgets(buf,MAXNAMLEN+1,fp) != NULL)
	{
	ret2 = sscanf (buf,"%s%s",function,key);

	/* comments begin with a '#'.  Skip them. */

	if (function[0] == '#')		/* comment, skip it 	*/
	    continue;

	if (ret2 == 2)			/* error if < 2 items 	*/
	    {
	    vret = get_vars (function, &codevar, &index);
	    if (vret < 0)		/* invalid function     */
		{
		ErrorF ("Bad function \"%s\" skipped in X*pointerkeys file.\n",
		    function);
		continue;		/* error - skip this one*/
		}
	    cret = get_codes (key, &code, pointerfunc[index].type, TRUE);
	    if (cret < 0 &&		/* not a key or modifier*/
	        pointerfunc[index].type == KEY) /* but must be  */
		{
		ErrorF ("Bad key name \"%s\" skipped in X*pointerkeys file.\n",
		    key);
		continue;		/* error - skip this one*/
		}

	    if (pointerfunc[index].type==MODIFIER) /* modifier - compute bit*/
	        *codevar.cptr = code - 8;
	    else if (pointerfunc[index].type==UINT_NUMBER)
	        *codevar.iptr = code;		/* code for 16-bit number */
	    else if (pointerfunc[index].type==USHORT_NUMBER)
	        *codevar.sptr = code;		/* code for 16-bit number */
	    else
	        *codevar.cptr = code;		/* code for 8-bit key */
	    }
	else
	    {
	    len = strlen(buf) - 1;	/* fgets adds a newline */
	    buf[len] = '\0';
	    if (len > 0)
	        ErrorF ("Bad entry \"%s\" skipped in X*pointerkeys file.\n",
		    buf);
	    }
	}
	
    fclose (fp);
    }

/****************************************************************************
 *
 * hpAddInputDevice(deviceProc, autoStart, pHPDev)
 * create an X input device, then assign pHPDev to it's devicePrivate field.
 *
 */

static DevicePtr hpAddInputDevice(deviceProc, autoStart, pHPDev)
    DeviceProc deviceProc;
    Bool autoStart;
    HPInputDevice *pHPDev;
    {
    DevicePtr pXDev;

    if ((pXDev = AddInputDevice(deviceProc, autoStart)) == NULL)
	FatalError ("Too many input devices - X server terminating!\n");
    pHPDev->dev_id = ((DeviceIntPtr) pXDev)->id;
#ifdef XINPUT
    if (pHPDev == hpPointer)
	{
	hp_device_ids[pHPDev->dev_id] = XPOINTER;
	x_device_ids[XPOINTER] = pHPDev->dev_id;
	}
    else if (pHPDev == hpKeyboard)
	{
	hp_device_ids[pHPDev->dev_id] = XKEYBOARD;
	x_device_ids[XKEYBOARD] = pHPDev->dev_id;
	}
    else
	{
	hp_device_ids[pHPDev->dev_id] = otherndx;
	x_device_ids[otherndx++] = pHPDev->dev_id;
	}
#endif /* XINPUT */
    pXDev->devicePrivate = (pointer) pHPDev;
    return  pXDev;
    }

/****************************************************************************
 *
 * We allow any keycode to be specified as a modifer, Even one that can't
 * be generated by our keyboard.
 *
 */

LegalModifier(key, dev)
    unsigned int key;
    DevicePtr dev;
    {
    return TRUE;
    }

/****************************************************************************
 *
 * close_device closes one of the input devices.
 *
 */

close_device(d)
    HPInputDevice	*d;
    {
    int i, tmp_fd = d->file_ds;

#ifdef __apollo
    if (!strcmp (d->dev_name, "Apollo_internal"))
	return;
#endif /* __apollo */
#ifdef __hp_osf
    if (d->file_ds != -1)
	ioctl(d->file_ds, HILUNMAPQ, &hil_qp->hil_evqueue.qnum);
    if (max_input_fd == 0)
	{
	RemoveEnabledDevice (beeper_fd);
	ioctl (beeper_fd, HILFREEQ, &hil_qp->hil_evqueue.qnum);
	close (beeper_fd);
	hil_qp = 0;
	}
#endif /* __hp_osf */
    if (d->hpflags & IS_SERIAL_DEVICE)
	(*(serialprocs[d->sndx].close)) (d->file_ds);
    else
	close (d->file_ds);
    d->file_ds = -1;
    if (tmp_fd == max_input_fd) 
	{
	max_input_fd = 0;
	for (i=0; i<MAX_LOGICAL_DEVS; i++)
	    if (l_devs[i].file_ds > max_input_fd)
		max_input_fd = l_devs[i].file_ds;
	}

    }

/*****************************
 *
 * init_events_queue (queue)
 * 
 */

init_events_queue(queue)
    struct  x11EventQueue	*queue;		
    {
    queue->events = events_array;	
    queue->head = 0;
    queue->tail = 0;
    queue->size = MAX_EVENTS;
    events_queue = queue;
    }

/*****************************************************************
 *
 * allocate_event ()
 *	allocates the next available event to the caller and increments
 *	the tail pointer of the events queue; sets queue_events_free as needed.
 *
 */

xHPEvent  *allocate_event ()
    {
    xHPEvent		*event;

    event = &( (events_queue->events)[events_queue->tail]);

    if ( events_queue->tail == WR_EVENTS)
	events_queue->tail = 0;
    else  (events_queue->tail)++;

    queue_events_free--;
    if (queue_events_free == 0)
	ErrorF ("Server Internal events queue is full!!!\n");
    return (event);
    }

deallocate_event (ev)
    xHPEvent		*ev;
    {
    xHPEvent		*tmp, *tail, *last, *first;

    tail = &( (events_queue->events)[events_queue->tail]);
    last = &( (events_queue->events)[WR_EVENTS]);
    first = &( (events_queue->events)[0]);

    for (tmp=ev; tmp!=tail; tmp++)
	if (tmp==last)
	    {
	    *tmp = *first;
	    tmp = first-1;
	    }
	else
	    *tmp = *(tmp+1);

    if (events_queue->tail == 0)
	events_queue->tail = WR_EVENTS;
    else
	events_queue->tail--;
    queue_events_free++;
    }

extern int apLeave_X, apReenter_X;      /*  in hp/apollo/apInit2.c */

CheckInput (data, result, LastSelectMask)
    pointer data;
    unsigned long result;
    long LastSelectMask[];
    {
    long devicesReadable[mskcnt];
    extern long EnabledDevices[];
    extern Bool	display_borrowed;	/* in x_hil.c */

    if (result <= 0)
	return;
    MASKANDSETBITS(devicesReadable, LastSelectMask, EnabledDevices);
#ifdef __apollo
    if (apReenter_X) apReturnFromDM();
    if (display_borrowed) return;

    while (GetSMDEvent(TRUE, NULL))
	;
    if (apLeave_X)   apReturnToDM();
    BITCLEAR (devicesReadable, fdApollo);
#endif /* __apollo */
#ifdef __hp_osf
    BITCLEAR (devicesReadable, beeper_fd);
#endif /* __hp_osf */
    if (ANYSET(devicesReadable)) 
	store_inputs (devicesReadable);
    }

#ifdef XINPUT
AddOtherInputDevices ()
    {
    int i;
    HPInputDevice *hp, *tmphp;
    DeviceIntPtr dev;
    Bool found;

    for (i=0, hp=l_devs; i<MAX_LOGICAL_DEVS; hp++,i++) 
	{
	found = FALSE;
        for (dev=inputInfo.devices; dev; dev=dev->next)
	    {
	    tmphp = GET_HPINPUTDEVICE (dev);
	    if (hp == tmphp)
		{
		found = TRUE;
		break;
		}
	    }
        for (dev=inputInfo.off_devices; found==FALSE && dev; dev=dev->next)
	    {
	    tmphp = GET_HPINPUTDEVICE (dev);
	    if (hp == tmphp)
		{
		found = TRUE;
		break;
		}
	    }
	if (found == FALSE && hp->x_name[0] != '\0' && 
		(strcmp (hp->dev_name,"/dev/null") != 0))
	    {
	    dev = x_init_device (hp, TRUE);
	    dev->inited = ((*dev->deviceProc)(dev, DEVICE_INIT) == Success);
	    }
	}
    }

ChangeKeyboardDevice (old_dev, new_dev)
    DeviceIntPtr	old_dev;
    DeviceIntPtr	new_dev;
    {
    CARD8		tmp;
    HPInputDevice 	*old = GET_HPINPUTDEVICE (old_dev);
    HPInputDevice 	*new = GET_HPINPUTDEVICE (new_dev);

    if (old->hpflags & OPEN_THIS_DEVICE)
	old->hpflags &= ~OPEN_THIS_DEVICE;
    tmp = hp_device_ids[new_dev->id];
    hp_device_ids[new_dev->id] = XKEYBOARD;
    hp_device_ids[old_dev->id] = tmp;
    x_device_ids[XKEYBOARD] = new_dev->id;
    x_device_ids[tmp] = old->dev_id;
    hpKeyboard = new;
    return (Success);
    }

ChangePointerDevice (old_dev, new_dev, x, y)
    DeviceIntPtr	old_dev;
    DeviceIntPtr	new_dev;
    unsigned char	x,y;
    {
    XID			tmp;
    HPInputDevice 	*old = GET_HPINPUTDEVICE (old_dev);
    HPInputDevice 	*new = GET_HPINPUTDEVICE (new_dev);
    
    if (new_dev == tablet_extension_device)
	return (BadDevice);
    x_axis = x;
    y_axis = y;
    if (x_axis != 0 || y_axis != 1)
	axes_changed = TRUE;
    else
	axes_changed = FALSE;

    new->coords[0] = old->coords[0];
    new->coords[1] = old->coords[1];

    if (old->hpflags & OPEN_THIS_DEVICE)
	old->hpflags &= ~OPEN_THIS_DEVICE;

    screen_change_dev = new_dev;
    tmp = hp_device_ids[new_dev->id];
    hp_device_ids[new_dev->id] = XPOINTER;
    hp_device_ids[old_dev->id] = tmp;
    x_device_ids[XPOINTER] = new_dev->id;
    x_device_ids[tmp] = old->dev_id;
    hpPointer = new;
#ifdef __apollo
    {
    smd_$pos_t pos;

    pos.column = hpPointer->coords[0];
    pos.line = hpPointer->coords[1];
    smd_$set_unit_cursor_pos (1, pos, &status);
    }
#endif /* __apollo */
    InitFocusClassDeviceStruct(old_dev);
    return (Success);
    }

/****************************************************************************
 *
 * Turn on a non-standard device.
 *
 */

OpenInputDevice (dev, client, status)
    DeviceIntPtr dev;
    ClientPtr client;
    int *status;
    {
    int			mode;
    HPInputDevice 	*d; 
    DeviceClientsPtr	tmp; 

    if (*status != Success)		/* kludge - if not Success, */
	mode = (*status >> 8);		/* called from HPSetInputDevice */
    else				/* mode hidden in 2nd byte	*/
	mode = DEVICE_EVENTS | ON;

    *status = Success;

    d = GET_HPINPUTDEVICE (dev);
    if (d->file_ds  == -1)			/* device not yet open   */
        {
        if (open_device (d) < 0)		/* couldn't open device  */
	    {
	    *status = BadDevice;
	    return;
	    }
        recalculate_x_name ();			/* recalculate names	*/
        }
    else
	{
        for (tmp = (DeviceClientsPtr) d->clients; tmp!=NULL; tmp=tmp->next)
    	    if (tmp->mode != mode)
		{
		*status = BadMode;
		return;
		}
	}
    SetInputDevice (d, mode);

    dev->startup = 1;
    RecordOpenRequest (client, d, dev->id, mode);
    }

/***********************************************************************
 *
 * Record a successful request from a client to open an input device.
 *
 */

static void
RecordOpenRequest (client, d, id, token)
    register ClientPtr client;
    HPInputDevice *d;
    CARD8 id;
    int token;
    {
    DeviceClientsPtr tmp;
    DeviceClientsPtr new_client;

    if (d->clients != NULL)
        {
        for (tmp = (DeviceClientsPtr) d->clients; tmp!=NULL; tmp=tmp->next)
    	if (tmp->client == client)
	    {
    	    tmp->count++;
    	    return;
	    }
    	else if (tmp->next == NULL)
    	    break;

        new_client = (DeviceClients *) Xalloc(sizeof(DeviceClients));
        tmp->next = new_client;
        }
    else
        {
        new_client = (DeviceClients *) Xalloc(sizeof(DeviceClients));
        d->clients = new_client;
        }

    memset ((char *) new_client, 0, sizeof (DeviceClients));
    new_client->resource = FakeClientID(client->index);
    new_client->client = client;
    new_client->next = NULL;
    new_client->count = 1;
    new_client->mode = token;

    AddResource((XID) new_client->resource, (RESTYPE) HPType, (pointer) id);
    }


/***********************************************************************
 *
 * Turn off a device because a client died.
 * Also called when a client closes a device.
 *
 */

int HPShutDownDevice (deviceid, clientid)
    CARD8 deviceid;
    int	clientid;
    {
    DeviceIntPtr 	dev = NULL; 
    DeviceClientsPtr	tmp;
    DeviceClientsPtr	save;
    HPInputDevice	*d;

    if (deviceid == inputInfo.pointer->id)
	d = hpPointer;
    else if (deviceid == inputInfo.keyboard->id)
	d = hpKeyboard;
    else
	{
	dev = LookupDeviceIntRec(deviceid);
	if (dev == NULL)
	    return;
	d = GET_HPINPUTDEVICE (dev);
	}
    
    if (d->clients != NULL)
        {
        tmp = (DeviceClientsPtr) d->clients;
        if (tmp->resource == clientid)
    	    {
	    d->clients = tmp->next;
	    Xfree (tmp);
	    }
        else
           for (save=tmp,tmp=tmp->next; tmp!=NULL; save=tmp, tmp=tmp->next)
               {
               if (tmp->resource == clientid)
        	   {
        	   save->next = tmp->next;
    		   Xfree (tmp);
    		   }
    	       }
        if (dev && d->clients == NULL)
    	    {
    	    if (d->open_cnt == 0)
    	        DisableDevice(dev);
	    else
		{
		d->mode = ABSOLUTE;
		d->hpflags |= MERGED_DEVICE;
		}
	    }
	}
    }

/****************************************************************************
 *
 * Turn off an extension device.
 * This code does not allow the keyboard or pointer to be turned off.
 *
 */

CloseInputDevice (dev, client)
    DeviceIntPtr	dev;
    ClientPtr		client;
    {
    HPInputDevice 	*d;
    DeviceClientsPtr	tmp;

    d = GET_HPINPUTDEVICE (dev);

    for (tmp= (DeviceClientsPtr) d->clients; tmp!=NULL; tmp=tmp->next)
    	if (tmp->client == client)
	    {
	    tmp->count--;
	    if (tmp->count == 0)
		{
    	        FreeResource(tmp->resource, RT_NONE);
		return;
		}
	    }
    }

/****************************************************************************
 *
 * Change the state of a non-standard device.
 * Modes are:
 *    ON - turn the device on.
 *    OFF - turn the device off.
 *    SYSTEM_EVENTS - report the standard input events.
 *    DEVICE_EVENTS - report the extension input events.
 *
 */

static void
SetInputDevice (d, mode)
    HPInputDevice	*d;
    int			mode;
    {

    if ((mode & DEVICE_EVENTS) == DEVICE_EVENTS)
	{
	d->mode = RELATIVE;
	d->hpflags &= ~MERGED_DEVICE;
	}
    else
	{
	mode |= ABSOLUTE;
        d->mode = ABSOLUTE;
        d->hpflags |= MERGED_DEVICE;
	}

    if ((mode & ABSOLUTE) == ABSOLUTE)
	{
	d->coords[0] = hpPointer->coords[0];
	d->coords[1] = hpPointer->coords[1];
	d->mode = ABSOLUTE;
	}
    else
	{
#if defined(__hpux) || defined(__hp_osf)
	d->coords[0] = 0;
	d->coords[1] = 0;
#endif /* __hpux */
	d->mode = RELATIVE;
	}
    }

/****************************************************************************
 *
 * Change the mode of an extension device.
 * This is for devices such as graphics tablets that can report either
 * relative or absolute motion.
 * We currently do not support this.
 *
 */

SetDeviceMode (client, dev, mode)
    register	ClientPtr	client;
    DeviceIntPtr dev;
    int		mode;
    {
    int i;
    HPInputDevice *d;

    d = GET_HPINPUTDEVICE (dev);
    if (d->dev_type == NULL_DEVICE)
	return Success;
    if (d->hpflags & IS_SERIAL_DEVICE)
	SERIAL_DRIVER_WRITE(d->file_ds, _XSetDeviceMode, &mode);
    return BadMatch;
    }

/****************************************************************************
 *
 * Set the value of valuators on an extension device.
 * This is needed for some devices that can report both
 * relative and absolute motion.  Some may require that the
 * initial values be set when switching modes.
 * We currently do not support this.
 *
 */

SetDeviceValuators (client, dev, valuators, first_valuator, num_valuators)
    register	ClientPtr	client;
    DeviceIntPtr dev;
    int		*valuators;
    int		first_valuator;
    int		num_valuators;
    {
    int i;
    HPInputDevice *d;
    HPResolutionControl ctrl;

    d = GET_HPINPUTDEVICE (dev);
    for (i=first_valuator; i<num_valuators; i++)
	if (i>=0 && i < dev->valuator->numAxes)
	    dev->valuator->axisVal[i] = *(valuators+i);
    if (d->dev_type == NULL_DEVICE)
	return Success;
    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	ctrl.first_valuator = first_valuator;
	ctrl.num_valuators = num_valuators;
	ctrl.resolutions = valuators;
	SERIAL_DRIVER_WRITE(d->file_ds, _XSetDeviceValuators, &ctrl);
	}
    return BadMatch;
    }

/****************************************************************************
 *
 * Change the resolution of valuators on an extension device.
 * This is needed for some devices that have multiple resolutions.
 * We currently do not support this.
 *
 */

int
ChangeDeviceControl (client, dev, control)
    register	ClientPtr	client;
    DeviceIntPtr dev;
    xDeviceCtl	*control;
    {
    HPInputDevice *d;
    xDeviceResolutionCtl *dctrl;
    HPResolutionControl ctrl;

    d = GET_HPINPUTDEVICE (dev);
    if (d->dev_type == NULL_DEVICE)
	return Success;
    if (d->hpflags & IS_SERIAL_DEVICE)
	{
	dctrl = (xDeviceResolutionCtl *) control;
	ctrl.first_valuator = dctrl->first_valuator;
	ctrl.num_valuators = dctrl->num_valuators;
	ctrl.resolutions =  (int *) (dctrl+1);
	SERIAL_DRIVER_WRITE(d->file_ds, _XChangeDeviceControl, &ctrl);
	}
    return BadMatch;
    }
#endif /* XINPUT */

#if defined(__hpux) || defined(__hp_osf)
#define	LEFT_SHIFT_CODE		0x05
#define	RIGHT_SHIFT_CODE	0x04
#define	LEFT_MOD1_CODE		0x03
#define	RIGHT_MOD1_CODE		0x02
#define	RIGHT_CONTROL_CODE	0x00
#define	LEFT_CONTROL_CODE	0x06
#endif /* __hpux */

#ifdef __apollo
#define	LEFT_SHIFT_CODE		0x5e
#define	RIGHT_SHIFT_CODE	0x6a
#define	LEFT_MOD1_CODE		0x75
#define	RIGHT_MOD1_CODE		0x77
#define	LEFT_CONTROL_CODE	0x43
#endif /* __apollo */

#define	LEFT_SHIFT_BIT		0x20
#define	RIGHT_SHIFT_BIT		0x10
#define	LEFT_MOD1_BIT		0x08
#define	RIGHT_MOD1_BIT		0x04
#define	RIGHT_CONTROL_BIT	0x01
#define	LEFT_CONTROL_BIT	0x40
#define	MAX_KEY_MODS		3

fix_modifierkeys()
    {
    u_char tmp[3];

    tmp[1] = 0xff;
    tmp[2] = 0xff;
    tmp[0] = pointer_amt_mods[0];
    mask_from_kcodes (tmp, &pointer_amt_bits[0]);
    tmp[0] = pointer_amt_mods[1];
    mask_from_kcodes (tmp, &pointer_amt_bits[1]);
    tmp[0] = pointer_amt_mods[2];
    mask_from_kcodes (tmp, &pointer_amt_bits[2]);

    mask_from_kcodes (pointer_key_mods, &ptr_mods);
    mask_from_kcodes (pointer_amt_mods, &mv_mods);
    mask_from_kcodes (reset_mods, &rs_mods);
    mask_from_kcodes (borrow_mode_mods, &bw_mods);
    mv_mods &= ~ptr_mods;
    }

static void
mask_from_kcodes (src, dst)
    u_char *src;
    u_char *dst;
    {
    int i;
    HPInputDevice *d;

    d = GET_HPINPUTDEVICE (inputInfo.keyboard);
    if (!(d->id_detail & HP_HIL)){
        for (i=0; i<MAX_KEY_MODS; i++, src++)
	    if (*src != 0xff)
		*dst |= inputInfo.keyboard->key->modifierMap[(*src+8)];
    return;
    }

    for (i=0; i<MAX_KEY_MODS; i++, src++)
	switch (*src)
	    {
	    case LEFT_SHIFT_CODE:
		*dst |= LEFT_SHIFT_BIT;
		break;
	    case RIGHT_SHIFT_CODE:
		*dst |= RIGHT_SHIFT_BIT;
		break;
	    case LEFT_MOD1_CODE:
		*dst |= LEFT_MOD1_BIT;
		break;
	    case RIGHT_MOD1_CODE:
		*dst |= RIGHT_MOD1_BIT;
		break;
	    case LEFT_CONTROL_CODE:
		*dst |= LEFT_CONTROL_BIT;
		break;
#if defined(__hpux) || defined(__hp_osf)
	    case RIGHT_CONTROL_CODE:
		*dst |= RIGHT_CONTROL_BIT;
		break;
#endif /* __hpux || __hp_osf */
	    default:
		break;
	    }
    }

get_down_modifiers(dev, down_mods)
    DeviceIntPtr dev;
    unsigned char *down_mods;
    {
    u_char *kptr = dev->key->down;

#if defined(__hpux) || defined(__hp_osf)
    HPInputDevice *d;

    d = GET_HPINPUTDEVICE (dev);
    if (d->id_detail & HP_HIL)
	*down_mods = kptr[1] & 0x7d;	/* mask off break and repeat cursor */
    else
	*down_mods = dev->key->state;	/* get X modifier bits 		    */
#endif /* __hpux */
#ifdef __apollo
    *down_mods = 0;
    if (kptr[9] & 0x08)
	*down_mods |= LEFT_CONTROL_BIT;
    if (kptr[12] & 0x40)
	*down_mods |= LEFT_SHIFT_BIT;
    if (kptr[14] & 0x04)
	*down_mods |= RIGHT_SHIFT_BIT;
    if (kptr[15] & 0x80)
	*down_mods |= RIGHT_MOD1_BIT;
    if (kptr[15] & 0x20)
	*down_mods |= LEFT_MOD1_BIT;
#endif /* __apollo */
    }

#ifdef __apollo

DisableAllInput ()
    {
    register DeviceIntPtr dev, next;
    HPInputDevice *d;


    for (dev = inputInfo.devices; dev; dev = next)
	{
	next = dev->next;
	d = GET_HPINPUTDEVICE (dev);
	RemoveEnabledDevice( d->file_ds);
	}
    }

EnableAllInput ()
    {
    register DeviceIntPtr dev, next;
    HPInputDevice *d;

    for (dev = inputInfo.devices; dev; dev = next)
	{
	next = dev->next;
	if (dev->inited && dev->startup)
	    {
	    d = GET_HPINPUTDEVICE (dev);
	    AddEnabledDevice( d->file_ds);
	    }
	}
    }

int xosWindowPrivateIndex;

#endif /* __apollo */

/*****************************************************************************
 *
 * Dynamically load drivers to support non-HIL input devices.
 *
 */

init_dynamic_devs(opendevs, numdev)
    struct	opendevs opendevs [];
    int		numdev;
    {
    int i, j, k, fd, sndx=0;
    HPInputDeviceHeader dhdr;
    Bool keyboard_is_pointer = FALSE;
    Bool (*driverInit)();
    shl_t ldr_module_id;
    long ldr_module_entry;
    int ret_val;
    char driver_path[255], driver_init[255];

   /*
    * See if we can access a DIN mouse and keyboard.  If so, we need to load
    * a driver to support them.
    *
    */

    find_din_kbd_use(opendevs, &keyboard_is_pointer, &numdev, &sndx);
    find_din_mouse_use(&keyboard_is_pointer, &numdev, &sndx);

   /*
    * For each device, 
    *     1). find an available device private struct,
    *     2). make sure the user has told us the device path and use,
    *     3). make sure the driver knows about this device,
    *     4). call the driver to open and configure it.
    */

    for (i=0; i<numdev; i++)
	{
	if (!serial[i].name[0])
	    continue;

	strcpy (driver_path, DRVRLIBDIR);
	strcat (driver_path, serial[i].name);
#if defined(__hp9000s300)
	strcpy (driver_init, "_");
	strcat (driver_init, serial[i].entry);
#else
	strcpy (driver_init, serial[i].entry);
#endif /* __hp9000s300 */

	for (j=0; j<num_serial_devices; j++)
	    if (strcmp (serialprocs[j].driver_name, serial[i].name)==0)
		break;
	if (j==num_serial_devices)   /* this driver wasn't previously loaded */
	    {
	    /*
	     * Dynamically load the driver.
	     */

	    ldr_module_id = shl_load( driver_path, BIND_IMMEDIATE, 0L);

	    if ( ldr_module_id == NULL ) 
		{
		ErrorF ("Failed to load serial input device driver %s, errno %d\n",
		    driver_path, errno);
		ErrorF ("Check spelling and case of device name.\n");
		continue;
		}
	    }
	else
	    ldr_module_id = serialprocs[j].ldr_module_id;

	sndx = j;

	/*
	 * Now look for the main entry point by name.
	 */

	ret_val = shl_findsym( &ldr_module_id, driver_init, TYPE_PROCEDURE,
                          &ldr_module_entry );
	if ( ret_val ) 
	    {
	    ErrorF ("Couldn't find main entry point %s in serial input device driver %s, retval is %x\n",
		driver_init, driver_path, ret_val);
	    ErrorF ("Check spelling and case of device name and entrypoint.\n");
	    continue;
	    }

	/*
	 * Call that entry point to initialize driver.
	 */

	driverInit = (pfrb) ldr_module_entry;
	ret_val = (*driverInit)(&serialprocs[sndx]);
	if (ret_val!=INIT_SUCCESS)
	    {
	    ErrorF ("Couldn't initialize serial input device driver %s\n",
		driver_path);
	    continue;
	    }

	dhdr = zdhdr;
	dhdr.file_ds = HP_VENDOR_RELEASE_NUMBER;
	strcpy (dhdr.path, serial[i].path);
	if (button_chording == CHORDING_ON)
	    dhdr.button_chording = 100;
	button_3 = 0;
	if ((*(serialprocs[sndx].configure))(&dhdr) == INIT_SUCCESS)
	    {

	    for (k=MAX_DEVICES-1; l_devs[k].hil_header.id != 1 && k>=0; k--)
	        ;
	    if (k<0)
	        for (k=MAX_DEVICES-1; (l_devs[k].hpflags & OPEN_THIS_DEVICE) && k>=0; k--)
	        ;
	    serialprocs[sndx].fd = dhdr.file_ds;
	    serialprocs[sndx].keymap_name = dhdr.keymap_name;
	    serialprocs[sndx].keymap_file = dhdr.keymap_file;
	    serialprocs[sndx].feedbacks = dhdr.feedbacks;
	    serialprocs[sndx].num_fdbk = dhdr.num_fdbk;
	    serialprocs[sndx].strf = dhdr.strf;
	    serialprocs[sndx].num_strf = dhdr.num_strf;
	    serialprocs[sndx].ledf = dhdr.ledf;
	    serialprocs[sndx].num_ledf = dhdr.num_ledf;
	    strcpy(l_devs[k].x_name, dhdr.x_name);
	    l_devs[k].x_atom = MakeAtom (dhdr.x_name, strlen(dhdr.x_name),0);
	    if (!l_devs[k].x_atom)
	        l_devs[k].x_atom = MakeAtom(dhdr.x_name, strlen(dhdr.x_name),1);
	    x_init_dynamic_device(&l_devs[k], &dhdr);
	    l_devs[k].sndx = sndx;
	    if (serial[i].use==XPOINTER)
		{
		if (hpPointer && hpPointer->file_ds != -1)
		    {
		    close (hpPointer->file_ds);
		    for (j=0, fd=hpPointer->file_ds; j<MAX_DEVICES; j++)
			if (l_devs[j].file_ds == fd)
			    l_devs[j].file_ds = -1;
		    }
		hpPointer = &l_devs[k];
		}
	    else if (serial[i].use==XKEYBOARD)
		{
		if (hpKeyboard && hpKeyboard->file_ds != -1)
		    {
		    close (hpKeyboard->file_ds);
		    for (j=0, fd=hpKeyboard->file_ds; j<MAX_DEVICES; j++)
			if (l_devs[j].file_ds == fd)
			    l_devs[j].file_ds = -1;
		    }

		hpKeyboard = &l_devs[k];
		if (dhdr.reset)
		    {
		    reset = dhdr.reset;
		    rs_mods = dhdr.reset_mods;
		    }
		}
	    if (serial[i].use==XOTHER+1)
		{
		serialprocs[sndx].close(dhdr.file_ds);
		serialprocs[sndx].fd = -1;
		}
	    else
		{
		if (serial[i].use==XOTHER) {
	            l_devs[k].hpflags |= MERGED_DEVICE;
	            l_devs[k].open_cnt = 1;
		    }
	        l_devs[k].file_ds = dhdr.file_ds;
		if (dhdr.file_ds > max_input_fd)
		    max_input_fd = dhdr.file_ds;

		if (keyboard_is_pointer)
		    {
		    hpKeyboard->hpflags |= SECOND_LOGICAL_DEVICE;
		    l_devs[spare+1] = *hpKeyboard;
		    hpPointer = &l_devs[spare+1];
		    }
		}
	    if (sndx==num_serial_devices)
		{
	        strcpy (serialprocs[sndx].driver_name, serial[i].name);
		serialprocs[sndx].ldr_module_id = ldr_module_id;
		num_serial_devices++;
		}
	    }
	else
	    ErrorF ("Couldn't initialize serial input device %s\n",
		serial[i].name);
	}
    if (!hpPointer)
	FatalError ("Couldn't open X pointer device!  Is there one attached?\n");
    if (!hpKeyboard)
	FatalError ("Couldn't open X keyboard!  Is there one attached?\n");
    }

#define DYNAMIC_DEVICE 	0xffff
x_init_dynamic_device(d, dhdr)
 HPInputDevice *d;
 HPInputDeviceHeader *dhdr;
 {
 strcpy (d->dev_name, dhdr->path);
 d->hil_header.resx = dhdr->resolution * 100;
 d->hil_header.resy = dhdr->resolution * 100;
 d->hil_header.size_x = dhdr->max_x;
 d->hil_header.size_y = dhdr->max_y;
 d->hil_header.ax_num = dhdr->ax_num;
 if (dhdr->ax_num)
 	d->x_type = MOUSE;
 d->hil_header.num_keys = dhdr->num_keys;
 if (dhdr->num_keys)
 	d->x_type = KEYBOARD;
 d->hil_header.p_button_count = dhdr->num_buttons;
 d->hil_header.v_button_count = dhdr->num_buttons;
 if (dhdr->num_ledf)
     d->hil_header.iob |= HAS_LEDS;

 if (dhdr->flags & ABSOLUTE_DATA)
    {
    d->mode = ABSOLUTE;
    d->hil_header.flags = HIL_ABSOLUTE;
    }
 else
    d->mode = RELATIVE;

 if (dhdr->flags & REPORTS_PROXIMITY)
    d->hil_header.iob = HILIOB_PIO;

 d->hpflags = IS_SERIAL_DEVICE;
 if (d->file_ds)
     d->hpflags |= OPEN_THIS_DEVICE;
 d->hpflags |= (dhdr->flags & DATA_SIZE_BITS);

 d->dev_type = DYNAMIC_DEVICE;
 d->hil_header.id = 0;
 d->id_detail &= ~HP_HIL;
}
/******************************************************************************
 *
 * shl_driver_load() is passed the path of a shared library to load, and the
 * name of an entry point to find.  It loads the library and returns the 
 * entry point address.
 */

Bool (*shl_driver_load( driver_path, driver_init ))()
char *driver_path, *driver_init;
{
   /*
    * pfrb = pointer to a function returning a Bool
    */
   typedef Bool (*pfrb)();

   shl_t            ldr_module_id;
   long             ldr_module_entry=0;
   int              ret_val;
   char             *dummy_handle = NULL;
#define ALL_SHLIBS_HANDLE &dummy_handle

   /**********************************************************************
    *
    * If the driver entrypoint is already visible within the current program,
    * skip the load and return the address of the routine we already have
    * loaded.
    *
    */

   ret_val = shl_findsym((shl_t *) PROG_HANDLE, driver_init, 
       TYPE_PROCEDURE, &ldr_module_entry );
   if ( ! ret_val ) {
      return( (pfrb) ldr_module_entry );
   }

   /*
    * If the driver entrypoint is already visible within a shared library we
    * are already accessing, skip the load and return the address.
    */
   ret_val = shl_findsym((shl_t *) ALL_SHLIBS_HANDLE, driver_init,
	TYPE_PROCEDURE, &ldr_module_entry );
   if ( ! ret_val ) {
      return( (pfrb) ldr_module_entry );
   }


   /**********************************************************************
    *
    * Load driver into the current VA space.
    */
   ldr_module_id = shl_load( driver_path, (BIND_IMMEDIATE | BIND_VERBOSE), 0L );

   if ( ldr_module_id == NULL )
	FatalError ("X server failed to load shared library %s, errno is %d\n", 
	driver_path,errno);


   /**********************************************************************
    *
    * Use the module ID to find the address of the requested entry point.
    */

   ret_val = shl_findsym( &ldr_module_id, driver_init, TYPE_PROCEDURE,
                          &ldr_module_entry );
   if (ret_val && driver_init != NULL)
	FatalError ("X server couldn't find entry point '%s' in library %s\n",
	    driver_init, driver_path);

   return( (pfrb) ldr_module_entry );
}

/***********************************************************************
 *
 * din_mouse_present
 *
 */

static Bool
din_mouse_present()
    {
    int fd;
    unsigned char cmdbuf[2];

    fd = open("/dev/ps2mouse", O_RDWR);
    ioctl (fd, PS2_PORTSTAT, &cmdbuf[0]);
    ioctl (fd, PS2_IDENT, &cmdbuf[1]);
    close (fd);
    if (cmdbuf[0] != 1 || cmdbuf[1] != 0)
	return FALSE;
    return TRUE;
    }

/***********************************************************************
 *
 * find_din_mouse_use
 *
 */

find_din_mouse_use(keyboard_is_pointer, numdev, sndx)
    int *keyboard_is_pointer, *numdev, *sndx;
    {

    if (!din_mouse_present())          		/* no DIN mouse attached */
	{
	if (!hpPointer || 			/* no HIL pointer device */
	     (hpPointer->dev_type != MOUSE &&	/* or it's not a motion device  */
	     hpPointer->pad2 != EXPLICIT_DEV))	/* and not explicitly specified */
	    *keyboard_is_pointer = TRUE;
	return;
	}

    if ((serial[XPOINTER].name[0] != '\0') || 	   /* RS232 pointer specified */
	(hpPointer && hpPointer->pad2 == EXPLICIT_DEV)||/* HIL ptr specified  */
	*keyboard_is_pointer)
	{
	*sndx = (*numdev)++;
	serial[*sndx].use = XOTHER+1;
	}
    else
	{
	*sndx = XPOINTER;
	serial[*sndx].use = XPOINTER;
	}
    strcpy (serial[*sndx].name, "hp7lc2m.sl");
    strcpy (serial[*sndx].entry, "hp7lc2m_Init");
    strcpy (serial[*sndx].path, "/dev/ps2mouse");
    }

/***********************************************************************
 *
 * din_kbd_present
 *
 */

static Bool
din_kbd_present()
    {
    int fd;
    unsigned char cmdbuf[2];

    fd = open("/dev/ps2kbd", O_RDWR);
    ioctl (fd, PS2_PORTSTAT, &cmdbuf[0]);
    ioctl (fd, PS2_IDENT, &cmdbuf[1]);
    close (fd);
    if (cmdbuf[0]!=2 && cmdbuf[1]!=0xab && cmdbuf[2]!=0x83) /* no DIN kbd  */
	return FALSE;
    return TRUE;
    }

/***********************************************************************
 *
 * find_din_kbd_use
 *
 */

find_din_kbd_use(opendevs, keyboard_is_pointer, numdev, sndx)
    struct	opendevs opendevs [];
    int *keyboard_is_pointer, *numdev, *sndx;
    {

    if (!din_kbd_present())				/* no DIN kbd  */
	return;

    if ((serial[XKEYBOARD].name[0]!='\0') ||           /* RS232 kbd specified */
	(hpKeyboard && hpKeyboard->pad2 == EXPLICIT_DEV))/* HIL kbd specified */
	{
	*sndx = (*numdev)++;
	serial[*sndx].use = XOTHER+1;
	}
    else
	{
	*sndx = XKEYBOARD;
	serial[*sndx].use = XKEYBOARD;
	}
    if (opendevs[XPOINTER].type == KEYBOARD && opendevs[XPOINTER].pos == 0)
	*keyboard_is_pointer = TRUE;

    strcpy (serial[*sndx].name, "hp7lc2k.sl");
    strcpy (serial[*sndx].entry, "hp7lc2k_Init");
    strcpy (serial[*sndx].path, "/dev/ps2kbd");
    }

char *x_basename (name)
    char *name;
    {
    int i;
    char *nptr = strchr (name, '_');
    char *ordinal[] = {"FIRST", "SECOND", "THIRD", "FOURTH", "FIFTH", "SIXTH", 
	"SEVENTH"};

    if (!nptr)
	return (name);

    for (i=0; i<7; i++)
	if (!strncmp(name, ordinal[i], strlen(ordinal[i])))
	    return(++nptr);
    return (name);
    }
