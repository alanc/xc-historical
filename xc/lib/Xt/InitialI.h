/* $xHeader$ */
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

******************************************************************/

/****************************************************************
 *
 * Displays
 *
 ****************************************************************/

#include <sys/param.h>

#include "fd.h"

typedef struct _TimerEventRec {
        struct timeval   te_timer_value;
	struct _TimerEventRec *te_next;
	Display *te_dpy;
	XtTimerCallbackProc	te_proc;
	XtAppContext app;
	Opaque	te_closure;
} TimerEventRec;

typedef struct _InputEvent {
	XtInputCallbackProc  ie_proc;
	Opaque ie_closure;
	struct	_InputEvent	*ie_next;
	struct  _InputEvent	*ie_oq;
	XtAppContext app;
	int	ie_source;
} InputEvent;

typedef struct _WorkProcRec {
	XtWorkProc proc;
	Opaque closure;
	struct _WorkProcRec *next;
	XtAppContext app;
} WorkProcRec;


typedef struct 
{
  	Fd_set rmask;
	Fd_set wmask;
	Fd_set emask;
	int	nfds;
	int	count;
} FdStruct;

typedef struct _XtAppStruct {
    Display **list;
    TimerEventRec *timerQueue;
    WorkProcRec *workQueue;
    InputEvent *selectRqueue[NOFILE], *selectWqueue[NOFILE],
	    *selectEqueue[NOFILE];
    InputEvent *outstandingQueue;
    XrmDatabase errorDB;
    XtErrorMsgHandler errorMsgHandler, warningMsgHandler;
    XtErrorHandler errorHandler, warningHandler;
    struct _ActionListRec *action_table;
    unsigned long selectionTimeout;
    FdStruct fds;
    short count, max, last;
    Boolean sync, rv, being_destroyed, error_inited;
} XtAppStruct;

extern void _SetDefaultErrorHandlers();
extern void _SetDefaultSelectionTimeout();

extern XtAppContext _XtDefaultAppContext();
extern void _XtDestroyAppContexts();
extern void _XtCloseDisplays();
extern int _XtAppDestroyCount;
extern int _XtDpyDestroyCount;

extern int _XtwaitForSomething(); /* ignoreTimers, ignoreInputs, block, 
					howlong, dset */
    /* Boolean ignoreTimers; */
    /* Boolean ignoreInputs; */
    /* Boolean block; */
    /* unsigned long *howlong; */
    /* XtAppContext app */

typedef struct _XtPerDisplayStruct {
    Region region;
    XtCaseProc defaultCaseConverter;
    XtKeyProc defaultKeycodeTranslator;
    XtAppContext appContext;
    KeySym *keysyms;                   /* keycode to keysym table */
    int keysyms_per_keycode;           /* number of keysyms for each keycode */
    KeySym *modKeysyms;                /* keysym values for modToKeysysm */
    ModToKeysymTable *modsToKeysyms;   /* modifiers to Keysysms index table*/
    Boolean being_destroyed;
} XtPerDisplayStruct, *XtPerDisplay;

extern void _XtPerDisplayInitialize();

extern XtPerDisplay _XtGetPerDisplay();
    /* Display *dpy */

extern void _XtDisplayInitialize();
    /* 	Display *dpy; */
    /* 	String name, classname; */
    /* 	XrmOptionDescRec *urlist */;
    /* 	Cardinal num_urs; */
    /* 	Cardinal *argc; */
    /* 	char *argv[];  */

