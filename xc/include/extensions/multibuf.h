/*
 * $XConsortium: multibuffer.txt,v 1.3 89/09/21 16:37:46 jim Exp $
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

#define X_GetBufferVersion		0
#define X_CreateImageBuffers		1
#define X_DestroyImageBuffers		2
#define X_DisplayImageBuffers		3
#define X_SetMultiBufferAttributes	4
#define X_SetBufferAttributes		6
#define X_GetBufferAttributes		7
#define X_GetBufferInfo			8
#define X_CreateStereoWindow		9

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
#define MultibufferNumberEvents		(MultibufferUpdateNotify + 1)

#define MultibufferBadBuffer		0
#define MultibufferNumberErrors		(MultibufferBadBuffer + 1)


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
} XMbufClobberNotifyEvent;

typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    int send_event;	    /* true if this came frome a SendEvent request */
    Display *display;	    /* Display the event was read from */
    Multibuffer buffer;	    /* buffer of event */
} XMbufUpdateNotifyEvent;


/*
 * per-window attributes that can be got
 */
typedef struct {
    int displayed_index;	/* which buffer is being displayed */
    int update_action;		/* Undefined, Background, Untouched, Copied */
    int update_hint;		/* Frequent, Intermittent, Static */
    int window_mode;		/* Mono, Stereo */
} XMbufWindowAttributes;

/*
 * per-window attributes that can be set
 */
typedef struct {
    int update_hint;		/* Frequent, Intermittent, Static */
} XMbufSetWindowAttributes;


/*
 * per-buffer attributes that can be got
 */
typedef struct {
    Window window;		/* which window this belongs to */
    unsigned long event_mask;	/* events that have been selected */
    int buffer_index;		/* which buffer is this */
    int side;			/* Mono, Left, Right */
} XMbufBufferAttributes;

/*
 * per-buffer attributes that can be set
 */
typedef struct {
    unsigned long event_mask;	/* events that have been selected */
} XMbufSetBufferAttributes;


/*
 * per-screen buffer info (there will be lists of them)
 */
typedef struct {
    VisualID visualid;		/* visual usuable at this depth */
    int max_buffers;		/* most buffers for this visual */
    int depth;			/* depth of buffers to be created */
} XMbufBufferInfo;

extern Bool XMbufQueryExtension ();	/* is extension on server */
extern Status XMbufGetVersion ();		/* what is extension rev */
extern int XMbufCreateBuffers ();		/* create buffers for window */
extern void XMbufDestroyBuffers ();	/* destroy buffers for window */
extern void XMbufDisplayBuffers ();	/* make buffers current */
extern Status XMbufGetWindowAttributes ();   /* get extension window attrs */
extern void XMbufChangeWindowAttributes ();  /* set extension window attrs */
extern Status XMbufGetBufferAttributes ();   /* get buffer attrs */
extern void XMbufChangeBufferAttributes ();  /* set buffer attrs */
extern Status XMbufGetScreenInfo ();	/* get mono and stereo parameters */
extern void XMbufCreateStereoWindow ();	/* stereo version of XCreateWindow */

#endif /* _MULTIBUF_SERVER_ */
#endif /* _MULTIBUF_H_ */
