/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/* $Header: inputstr.h,v 1.10 87/05/31 00:55:34 toddb Locked $ */

#ifndef INPUTSTRUCT_H
#define INPUTSTRUCT_H

#include "input.h"
#include "window.h"
#include "dixstruct.h"

typedef struct _OtherClients {
    OtherClientsPtr	next;
    ClientPtr		client;	  /* which client is slecting on this window */
    int			resource; /* id for putting into resource manager */
    long		mask;
} OtherClients;

typedef struct _DeviceIntRec *DeviceIntPtr;

/*
 * The following structure gets used for both active and passive grabs. For
 * active grabs some of the fields (e.g. modifiers) are not used. However,
 * that is not much waste since there are at most 2 active grabs (keyboard
 * and pointer) going at once in the server.
 */
  typedef struct _GrabRec {
    GrabPtr		next;		/* for chain of passive grabs */
    int			resource;
    ClientPtr		client;
    DeviceIntPtr	device;
    WindowPtr		window;
    Bool		ownerEvents;
    Bool		keyboardMode, pointerMode;
    Mask		modifiers;
    Mask		eventMask;
    union {
	struct {
	    int		button;
	    WindowPtr	confineTo;
	    CursorPtr	cursor;
	} ptr;
	struct {
	    int		key;
	} keybd;
    } u;
} GrabRec;

typedef struct {
    WindowPtr	win;
    int		revert;
    TimeStamp	time;
} FocusRec, *FocusPtr;

/* states for devices */

#define NOT_GRABBED		0
#define THAWED			1
#define FREEZE_NEXT_EVENT	2
#define FROZEN			3	/* any state >= has device frozen */
#define FROZEN_NO_EVENT		3
#define FROZEN_WITH_EVENT	4

typedef struct _DeviceIntRec {
    DeviceRec	public;
    TimeStamp	grabTime;
    Bool	startup;		/* true if needs to be turned on at
				          server intialization time */
    DeviceProc	deviceProc;		/* proc(DevicePtr, DEVICE_??). It is
					  used to initialize, turn on, or
					  turn off the device */
    Bool	inited;			/* TRUE if INIT returns Success */
    GrabPtr	grab;			/* the grabber - used by DIX */
    struct {
	Bool		frozen;
	int		state;
	GrabPtr		other;		/* if other grab has this frozen */
	xEvent		event;		/* saved to be replayed */
    } sync;
    BYTE	down[DOWN_LENGTH];
    union {
	struct {
#ifdef	NEW_MODIFIERS
	    /* here we store a bitmask for each key of the modifiers it sets */
	    CARD8	modifierMap[MAP_LENGTH];
#else
	    ModifierMapRec modMap;
#endif
	    KeybdCtrl	ctrl;
	    void	(*BellProc) ();
	    void	(*CtrlProc) ();
	    FocusRec	focus;
	    Bool	passiveGrab;
	} keybd;
	struct {
	    PtrCtrl	ctrl;
	    void	(*CtrlProc) ();
	    int		(*GetMotionProc) ();
	    Bool	autoReleaseGrab;	/* any button delivery */
	    BYTE	map[MAP_LENGTH];
	    int		mapLength;		/* valid entries in the map */
	} ptr;
	struct {
	    FocusRec	focus;
	    BYTE	map[MAP_LENGTH];
	    int		mapLength;		/* valid entries in the map */
	} other;
    } u;
} DeviceIntRec;

typedef struct {
    int			numMotionEvents;
    int			numDevices;
    int			arraySize;
    DeviceIntPtr	*devices;	/* all the devices (including below) */
    DeviceIntPtr	keyboard;	/* the main one for the server */
    DeviceIntPtr	pointer;
} InputInfo;

/* for keeping the events for devices grabbed synchronously */
typedef struct _QdEvent *QdEventPtr;
typedef struct _QdEvent {
    QdEventPtr		forw;
    QdEventPtr		back;
    DeviceIntPtr	device;
    xEvent		event;
} QdEventRec;    

#endif /* INPUTSTRUCT_H */
