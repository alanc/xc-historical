/* $XConsortium: XKBlib.h,v 1.4 93/09/28 21:34:12 rws Exp $ */
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

#ifndef _XKBLIB_H_
#define _XKBLIB_H_

#include <X11/extensions/XKBstr.h>

typedef struct _XkbAnyEvent {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbStateNotify ... XkbCompatMapNotify */
	unsigned int device;	/* device ID */
} XkbAnyEvent;

typedef struct _XkbStateNotifyEvent {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbStateNotify */
	unsigned int device;	/* device ID */
	unsigned int keycode;	/* keycode that caused the change */
	unsigned int eventType;	/* KeyPress or KeyRelease */
	unsigned int requestMajor;/* Major opcode of request */
	unsigned int requestMinor;/* Minor opcode of request */
	unsigned int changed;	/* mask of changed state components */
	unsigned int group;	/* keyboard group */
	unsigned int baseGroup;	/* base keyboard group */
	unsigned int latchedGroup;/* latched keyboard group */
	unsigned int lockedGroup; /* locked keyboard group */
	unsigned int mods;	  /* modifier state */
	unsigned int baseMods;	  /* base modifier state */
	unsigned int latchedMods; /* latched modifiers */
	unsigned int lockedMods;  /* locked modifiers */
	unsigned int compatState; /* compatibility state */
	unsigned int unlockedMods;
	Bool groupsUnlocked;
} XkbStateNotifyEvent;

typedef struct _XkbMapNotifyEvent {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbMapNotify */
	unsigned int device;	/* device ID */
	unsigned int changed;		/* fields which have been changed */
	unsigned int resized;		/* fields which have been resized */
	unsigned int firstKeyType;	/* first changed key type */
	unsigned int nKeyTypes;		/* number of changed key types */
	unsigned int firstKeySym;	/* first changed key sym */
	unsigned int nKeySyms;		/* number of changed key syms */
	unsigned int firstKeyAction;	/* first changed key action */
	unsigned int nKeyActions;	/* number of changed key actions */
	unsigned int firstKeyBehavior;	/* first changed key behavior */
	unsigned int nKeyBehaviors;	/* number of changed key behaviors */
} XkbMapNotifyEvent;

typedef struct _XkbControlsNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbControlsNotify */
	unsigned int device;	/* device ID */
	unsigned long int changedControls;
	unsigned long int enabledControls;
	unsigned long int enabledControlChanges;
	unsigned int keycode;
	unsigned int eventType;
	unsigned int requestMajor;
	unsigned int requestMinor;
} XkbControlsNotifyEvent;

typedef struct _XkbIndicatorNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbIndicatorNotify */
	unsigned int device;	/* device ID */
	unsigned int stateChanged;/* indicators that have changed state */
	unsigned int state;	/* current state of all indicators */
	unsigned int mapChanged;/* indicators whose maps have changed */
} XkbIndicatorNotifyEvent;

typedef struct _XkbBellNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbBellNotify */
	unsigned int device;	/* device ID */
	unsigned int percent;	/* requested volume as a percent of maximum */
	unsigned int pitch;	/* requested pitch in Hz */
	unsigned int duration;	/* requested duration in milliseconds */
	unsigned int bellClass;	/* (input extension) class of feedback */
	unsigned int bellID;	/* (input extension) ID of feedback */
	Atom name;		/* "name" of requested bell */
} XkbBellNotifyEvent;

typedef struct _XkbSlowKeyNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbSlowKeyNotify */
	unsigned int device;	/* device ID */
	unsigned int slowKeyType;/* press, release, accept, reject */
	unsigned int keycode;	/* key of event */
	unsigned int delay;	/* current delay in milliseconds */
} XkbSlowKeyNotifyEvent;

typedef struct _XkbNamesNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbNamesNotify */
	unsigned int device;	/* device ID */
	unsigned int changed;	/* names that have changed */
	unsigned int firstKeyType;	/* first key type with new name */
	unsigned int nKeyTypes;		/* number of key types with new names */
	unsigned int firstLevel;	/* first key type new new level names */
	unsigned int nLevels;		/* # of key types w/new level names */
	unsigned int firstRadioGroup;	/* first radio group with new name */
	unsigned int nRadioGroups;	/* # of radio groups with new names */
	unsigned int nCharSets;		/* total number of charsets */
	unsigned int changedMods;	/* modifiers with new names */
	unsigned int changedIndicators;	/* indicators with new names */
} XkbNamesNotifyEvent;

typedef struct _XkbCompatMapNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbCompatMapNotify */
	unsigned int device;	/* device ID */
	unsigned int changedMods;	/* modifiers with new compat maps */
	unsigned int firstSym;		/* first new symbol interp */
	unsigned int nSyms;		/* number of new symbol interps */
	unsigned int nTotalSyms;	/* total # of symbol interps */
} XkbCompatMapNotifyEvent;

typedef struct _XkbAlternateSymsNotify {
	int type;		/* XkbAnyEvent */
	unsigned long serial;	/* of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Time time;		/* milliseconds */
	int xkbType;		/* XkbAlternateSymsNotify */
	unsigned int device;	/* device ID */
	unsigned int altSymsID;	/* identifier of changed alt symbol set */
	unsigned int firstKey;	/* first key with new alternate syms */
	unsigned int nKeys;	/* number of keys with alternate syms */
} XkbAlternateSymsNotifyEvent;

typedef union _XkbEvent {
	XkbAnyEvent		u;
	XkbAnyEvent		any;
	XkbStateNotifyEvent	state;
	XkbMapNotifyEvent	map;
	XkbControlsNotifyEvent	controls;
	XkbIndicatorNotifyEvent indicators;
	XkbBellNotifyEvent	bell;
	XkbSlowKeyNotifyEvent	slowKey;
	XkbNamesNotifyEvent	names;
	XkbCompatMapNotifyEvent	compat;
	XkbAlternateSymsNotifyEvent altSyms;
} XkbEvent;

#define	XkbGeometryNameLength	24

typedef	struct _XkbSimpleKeyShape {
	unsigned short	width;
	unsigned short	height;
} XkbSimpleKeyShapeRec;

typedef	struct _XkbKbdKeyRec {
	unsigned char	keycode;
	unsigned char	shape;
	short		offset;
} XkbKbdKeyRec;

typedef	struct _XkbKbdGroupRec {
	unsigned short	nKeys;
	short		top;
	XkbKbdKeyRec	*keys;
} XkbKbdGroupRec;

typedef struct _XkbKbdSectionRec {
	char		name[XkbGeometryNameLength];
	unsigned short	left;
	unsigned short	top;
	unsigned short	right;
	unsigned short	bottom;
	unsigned short	nKeys;
	unsigned short	nGroups;
	XkbKbdGroupRec	*groups;
} XkbKbdSectionRec;

typedef struct _XkbKbdGeometryRec {
	char			 desc[XkbGeometryNameLength];
	unsigned short		 widthMM;
	unsigned short		 heightMM;
	unsigned short		 nKeys;
	unsigned short		 nSections;
	XkbSimpleKeyShapeRec	*simpleShapes;
	XkbKbdSectionRec	*sections;
} XkbKbdGeometryRec;

typedef struct	_XkbKbdDpyState	XkbKbdDpyStateRec,*XkbKbdDpyStatePtr;

_XFUNCPROTOBEGIN

extern	Bool	XkbIgnoreExtension(
#if NeedFunctionPrototypes
	Bool			/* ignore */
#endif
);

extern	Bool	XkbQueryExtension(
#if NeedFunctionPrototypes
	Display *		/* display */,
	int *			/* opcodeReturn */,
	int *			/* eventBaseReturn */,
	int *			/* errorBaseReturn */,
	int *			/* majorRtrn */,
	int *			/* minorRtrn */
#endif
);

extern	Bool	XkbUseExtension(
#if NeedFunctionPrototypes
	Display *		/* display */
#endif
);

extern	Status	XkbLibraryVersion(
#if NeedFunctionPrototypes
	int *			/* libMajorRtrn */,
	int *			/* libMinorRtrn */
#endif
);

extern	Status	XkbSetAutoRepeatRate(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned		/* deviceSpec */,
	unsigned		/* delay */,
	unsigned		/* interval */
#endif
);

extern	Bool	XkbGetAutoRepeatRate(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned		/* deviceSpec */,
	unsigned *		/* delayRtrn */,
	unsigned *		/* intervalRtrn */
#endif
);

extern	Bool	XkbDeviceBell(
#if NeedFunctionPrototypes
	Display *		/* display */,
	int			/* deviceSpec */,
	int			/* bellClass */,
	int			/* bellID */,
	int			/* percent */,
	Atom			/* name */
#endif
);

extern	Bool	XkbBell(
#if NeedFunctionPrototypes
	Display *		/* display */,
	int			/* percent */,
	Atom			/* name */
#endif
);

extern	Bool	XkbSelectEvents(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned		/* deviceID */,
	unsigned long		/* affect */,
	unsigned long		/* values */
#endif
);

extern	Bool	XkbSelectEventDetails(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* deviceID */,
	unsigned 		/* eventType */,
	unsigned long 		/* affect */,
	unsigned long 		/* details */
#endif
);

extern	Status	XkbGetIndicatorState(
#if NeedFunctionPrototypes
    Display *			/* dpy */,
    unsigned int 		/* deviceSpec */,
    CARD32 *			/* pStateRtrn */
#endif
);

extern	Status	 XkbGetIndicatorMap(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned long		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	 XkbSetIndicatorMap(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned long 		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbLockModifiers(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* deviceSpec */,
	unsigned 		/* affect */,
	unsigned 		/* values */
#endif
);

extern	Status	XkbLatchModifiers(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* deviceSpec */,
	unsigned 		/* affect */,
	unsigned 		/* values */
#endif
);

extern	Status	XkbSetServerInternalMods(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* deviceSpec */,
	unsigned 		/* affect */,
	unsigned 		/* values */
#endif
);

extern	Status	XkbSetKeyOnlyMods(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* deviceSpec */,
	unsigned 		/* affect */,
	unsigned 		/* values */
#endif
);

extern	XkbDescPtr XkbGetMap(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* which */,
	unsigned 		/* deviceSpec */
#endif
);

extern	Status	XkbGetUpdatedMap(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetMapChanges(
#if NeedFunctionPrototypes
	Display *		/* display */,
	XkbDescPtr		/* desc */,
	XkbChangesPtr		/* changes */
#endif
);

extern	Status	XkbGetActions(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* firstAction */,
	unsigned		/* nActions */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetRadioGroups(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* firstRadioGroup */,
	unsigned		/* nRadioGroups */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetKeySyms(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* firstKey */,
	unsigned		/* nKeys */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetKeyBehaviors(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* firstKey */,
	unsigned		/* nKeys */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetKeyActions(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* firstKey */,
	unsigned		/* nKeys */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetControls(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned long		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbSetControls(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned long		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern Status XkbGetCompatMap(
#if NeedFunctionPrototypes
	Display *		/* dpy */,
	unsigned 		/* which */,
	XkbDescPtr 		/* xkb */
#endif
);

extern Status XkbSetCompatMap(
#if NeedFunctionPrototypes
	Display *		/* dpy */,
	unsigned 		/* which */,
	XkbDescPtr 		/* xkb */,
	Bool			/* updateActions */
#endif
);

extern	Status	XkbGetNames(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbSetNames(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned		/* which */,
	unsigned		/* firstColMap */,
	unsigned		/* nColMaps */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbGetState(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned 		/* deviceSpec */,
	XkbStatePtr		/* rtrnState */
#endif
);

extern	Status	XkbSetMap(
#if NeedFunctionPrototypes
	Display *		/* display */,
	unsigned		/* which */,
	XkbDescPtr		/* desc */
#endif
);

extern	Status	XkbChangeMap(
#if NeedFunctionPrototypes
	Display*		/* display */,
	XkbDescPtr		/* desc */,
	XkbMapChangesRec *	/* changes */
#endif
);

extern	KeySym *XkbEnlargeKeySymbols(
#if NeedFunctionPrototypes
	XkbDescPtr		/* desc */,
	int 			/* forKey */,
	int 			/* symsNeeded */
#endif
);

extern	XkbAction *XkbEnlargeKeyActions(
#if NeedFunctionPrototypes
	XkbDescPtr		/* desc */,
	int 			/* forKey */,
	int 			/* actsNeeded */
#endif
);

extern	Status XkbChangeTypeOfKey(
#if NeedFunctionPrototypes
	XkbDescPtr		/* xkb */,
	unsigned 		/* key */,
	unsigned		/* newType */,
	XkbMapChangesRec *	/* pChanges */
#endif
);

extern	Status XkbChangeSymsForKey(
#if NeedFunctionPrototypes
	XkbDescPtr		/* xkb */,
	unsigned 		/* key */,
	unsigned		/* count */,
	KeySym *                /* syms */,
	XkbMapChangesRec *	/* pChanges */
#endif
);

extern	XkbKbdGeometryRec	*XkbGetKbdGeometry(
#if NeedFunctionPrototypes
    Display *dpy, char *name
#endif
);

extern	XkbKbdDpyStateRec	*XkbKbdZoom(
#if NeedFunctionPrototypes
    XkbKbdGeometryRec*,int w,int h
#endif
);
extern	XkbKbdKeyRec		*XkbKbdNextKey(
#if NeedFunctionPrototypes
    XkbKbdDpyStatePtr pState,
    int *left,int *top,
    int *right,int *bottom
#endif
);

extern	Status			 XkbSetDebuggingFlags(
#if NeedFunctionPrototypes
    Display *		/* dpy */,
    unsigned int	/* mask */,
    unsigned int	/* flags */,
    char *		/* msg */,
    unsigned int *	/* rtrnFlags */,
    Bool *		/* disableLocks */
#endif
);

_XFUNCPROTOEND

#endif /* _XKBLIB_H_ */
