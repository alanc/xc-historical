/*
 * $XConsortium: multibuf.h,v 1.3 89/09/25 15:45:34 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _MULTIBUF_H_
#define _MULTIBUF_H_

#define X_MbufGetBufferVersion		0
#define X_MbufCreateImageBuffers	1
#define X_MbufDestroyImageBuffers	2
#define X_MbufDisplayImageBuffers	3
#define X_MbufSetMultiBufferAttributes	4
#define X_MbufSetBufferAttributes	6
#define X_MbufGetBufferAttributes	7
#define X_MbufGetBufferInfo		8
#define X_MbufCreateStereoWindow	9

/*
 * update_action field
 */
#define MultibufferActionUndefined	0
#define MultibufferActionBackground	1
#define MultibufferActionUntouched	2
#define MultibufferActionCopied		3

/*
 * update_hint field
 */
#define MultibufferHintFrequent		0
#define MultibufferHintIntermittent	1
#define MultibufferHintStatic		2

/*
 * valuemask fields
 */
#define MultibufferWindowUpdateHint	(1L << 0)
#define MultibufferBufferEventMask	(1L << 0)

/*
 * mono vs. stereo and left vs. right
 */
#define MultibufferModeMono		0
#define MultibufferModeStereo		1
#define MultibufferSideMono		0
#define MultibufferSideLeft	  	1
#define MultibufferSideRight		2

/*
 * clobber state
 */
#define MultibufferUnclobbered		0
#define MultibufferPartiallyClobbered	1
#define MultibufferFullyClobbered	2

/*
 * event stuff
 */
#define MultibufferClobberNotifyMask	0x02000000
#define MultibufferUpdateNotifyMask	0x04000000

#define MultibufferClobberNotify	0
#define MultibufferUpdateNotify		1
#define MultibufferNumberOfEvents	(MultibufferUpdateNotify + 1)

#define MultibufferBadBuffer		0
#define MultibufferNumberOfErrors	(MultibufferBadBuffer + 1)


#ifndef _MULTIBUF_SERVER_
/*
 * Extra definitions that will only be needed in the client
 */
typedef XID Multibuffer;

typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    int send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Multibuffer buffer;	    /* buffer of event */
    int	state;		    /* see Clobbered constants above */
} XmbufClobberNotifyEvent;

typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    int send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Multibuffer buffer;	    /* buffer of event */
} XmbufUpdateNotifyEvent;


/*
 * per-window attributes that can be got
 */
typedef struct {
    int displayed_index;	/* which buffer is being displayed */
    int update_action;		/* Undefined, Background, Untouched, Copied */
    int update_hint;		/* Frequent, Intermittent, Static */
    int window_mode;		/* Mono, Stereo */
} XmbufWindowAttributes;

/*
 * per-window attributes that can be set
 */
typedef struct {
    int update_hint;		/* Frequent, Intermittent, Static */
} XmbufSetWindowAttributes;


/*
 * per-buffer attributes that can be got
 */
typedef struct {
    Window window;		/* which window this belongs to */
    unsigned long event_mask;	/* events that have been selected */
    int buffer_index;		/* which buffer is this */
    int side;			/* Mono, Left, Right */
} XmbufBufferAttributes;

/*
 * per-buffer attributes that can be set
 */
typedef struct {
    unsigned long event_mask;	/* events that have been selected */
} XmbufSetBufferAttributes;


/*
 * per-screen buffer info (there will be lists of them)
 */
typedef struct {
    VisualID visualid;		/* visual usuable at this depth */
    int max_buffers;		/* most buffers for this visual */
    int depth;			/* depth of buffers to be created */
} XmbufBufferInfo;

extern Bool XmbufQueryExtension ();	/* is extension on server */
extern Status XmbufGetVersion ();		/* what is extension rev */
extern int XmbufCreateBuffers ();		/* create buffers for window */
extern void XmbufDestroyBuffers ();	/* destroy buffers for window */
extern void XmbufDisplayBuffers ();	/* make buffers current */
extern Status XmbufGetWindowAttributes ();   /* get extension window attrs */
extern void XmbufChangeWindowAttributes ();  /* set extension window attrs */
extern Status XmbufGetBufferAttributes ();   /* get buffer attrs */
extern void XmbufChangeBufferAttributes ();  /* set buffer attrs */
extern Status XmbufGetScreenInfo ();	/* get mono and stereo parameters */
extern void XmbufCreateStereoWindow ();	/* stereo version of XCreateWindow */

#endif /* _MULTIBUF_SERVER_ */
#endif /* _MULTIBUF_H_ */
