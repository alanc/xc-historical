#ifndef lint
static char Xrcsid[] = "$XConsortium: Display.c,v 1.24 89/09/12 16:46:49 swick Exp $";
/* $oHeader: Display.c,v 1.9 88/09/01 11:28:47 asente Exp $ */
#endif /*lint*/

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

#include <sys/param.h>

#ifdef pegasus
#undef dirty			/* some bozo put this in sys/param.h */
#endif /* pegasus */

#include <X11/Xlib.h>
#include "IntrinsicI.h"

#define HEAP_SEGMENT_SIZE 8180

static void _XtHeapInit();
static void _XtHeapFree();

ProcessContext _XtGetProcessContext()
{
    static ProcessContextRec processContextRec = {
	(XtAppContext)NULL,
	(XtAppContext)NULL,
	(ConverterTable)NULL
    };

    return &processContextRec;
}


XtAppContext _XtDefaultAppContext()
{
    register ProcessContext process = _XtGetProcessContext();
    if (process->defaultAppContext == NULL) {
	process->defaultAppContext = XtCreateApplicationContext();
    }
    return process->defaultAppContext;
}

static void XtAddToAppContext(d, app)
	Display *d;
	XtAppContext app;
{
#define DISPLAYS_TO_ADD 4

	if (app->count >= app->max) {
	    app->max += DISPLAYS_TO_ADD;
	    app->list = (Display **) XtRealloc((char *)app->list,
		    (unsigned) app->max * sizeof(Display *));
	}

	app->list[app->count++] = d;
	if (ConnectionNumber(d) + 1 > app->fds.nfds) {
	    app->fds.nfds = ConnectionNumber(d) + 1;
	}
#undef DISPLAYS_TO_ADD
}

static void XtDeleteFromAppContext(d, app)
	Display *d;
	register XtAppContext app;
{
	register int i;

	for (i = 0; i < app->count; i++) if (app->list[i] == d) break;

	if (i < app->count) {
	    if (i <= app->last && app->last > 0) app->last--;
	    for (i++; i < app->count; i++) app->list[i-1] = app->list[i];
	    app->count--;
	}
}

static void
ComputeAbbrevLen(string, name, len)
    String string;		/* the variable */
    String name;		/* the constant */
    int *len;			/* the current ambiguous length */
{
    int string_len = strlen(string);
    int name_len = strlen(name);
    int i;

    for (i=0; i<string_len && i<name_len && *string++ == *name++; i++);

    if (i < name_len && i > *len)
	*len = i;
}


Display *XtOpenDisplay(app, displayName, applName, className,
		urlist, num_urs, argc, argv)
	XtAppContext app;
	String displayName, applName, className;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	Cardinal *argc;
	char *argv[];
{
	char  displayCopy[256];
	int i;
	char *ptr, *rindex(), *index(), *strncpy();
	Display *d;
#ifdef OLDCOLONDISPLAY
	int squish = -1;
	Boolean found_display = FALSE;
#endif
	int min_display_len = 0;
	int min_name_len = 0;

	if (applName == NULL) {
	    ptr = rindex(argv[0], '/');
	    if (ptr) applName = ++ptr;
	    else applName = argv[0];
	}

	/*
	   Find the display name and open it
	   While we are at it we look for name because that is needed 
	   soon after to do the argument parsing.
	 */

	displayCopy[0] = 0;

	for (i = 0; i < num_urs; i++) {
	    ComputeAbbrevLen(urlist[i].option, "-display", &min_display_len);
	    ComputeAbbrevLen(urlist[i].option, "-name",    &min_name_len);
	}

	for(i = 1; i < *argc; i++) {
	    int len = strlen(argv[i]);
#ifdef OLDCOLONDISPLAY
	    if (!found_display && index(argv[i], ':') != NULL) {
		(void) strncpy(displayCopy, argv[i], sizeof(displayCopy));
		squish = i;
		continue;
	    }
#endif
	    if(len > min_display_len && !strncmp("-display", argv[i], len)) {
		i++;
		if (i == *argc) break;
		(void) strncpy(displayCopy, argv[i], sizeof(displayCopy));
#ifdef OLDCOLONDISPLAY
		found_display = TRUE;
#endif
		continue;
	    }
	    if(len > min_name_len && !strncmp("-name", argv[i], len)) {
		i++;
		if (i == *argc) break;
		applName = argv[i];
		continue;
	    }
	}

#ifdef OLDCOLONDISPLAY
	if(!found_display && squish != -1) {
	    (*argc)--;
	    for(i = squish; i < *argc; i++) {
		argv[i] = argv[i+1];
	    }
	}
#endif

	if (displayName == NULL) displayName = displayCopy;
	
	d = XOpenDisplay(displayName);

	if (d != NULL) {
	    XtDisplayInitialize(app, d, applName, className,
		    urlist, num_urs, argc, argv);
	}
	return d;
}

void
XtDisplayInitialize(app, dpy, name, classname, urlist, num_urs, argc, argv)
	XtAppContext app;
	Display *dpy;
	String name, classname;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	Cardinal *argc;
	char *argv[];
{
	XtPerDisplay pd;
	static XtPerDisplay NewPerDisplay();

	if (app == NULL) app = _XtDefaultAppContext();
	XtAddToAppContext(dpy, app);

	pd = NewPerDisplay(dpy);
	pd->region = XCreateRegion();
        pd->defaultCaseConverter = _XtConvertCase;
        pd->defaultKeycodeTranslator = XtTranslateKey;
        pd->keysyms = NULL;
	pd->modKeysyms = NULL;
        pd->modsToKeysyms = NULL;
	pd->appContext = app;
	pd->name = XrmStringToName(name);
	pd->class = XrmStringToClass(classname);
	pd->being_destroyed = False;
	pd->GClist = NULL;
	pd->drawables = NULL;
	pd->drawable_count = 0;
	_XtHeapInit(&pd->heap);

	_XtDisplayInitialize(dpy, app, name, classname, urlist, 
		num_urs, argc, argv);
}

XtAppContext XtCreateApplicationContext()
{
	XtAppContext app = XtNew(XtAppStruct);

	app->process = _XtGetProcessContext();
	app->next = app->process->appContextList;
	app->process->appContextList = app;
	app->list = NULL;
	app->count = app->max = app->last = 0;
	app->timerQueue = NULL;
	app->workQueue = NULL;
	app->outstandingQueue = NULL;
	app->errorDB = NULL;
	_XtSetDefaultErrorHandlers(&app->errorMsgHandler, 
		&app->warningMsgHandler, &app->errorHandler, 
		&app->warningHandler);
	app->action_table = NULL;
	_XtSetDefaultSelectionTimeout(&app->selectionTimeout);
	_XtSetDefaultConverterTable(&app->converterTable);
	app->sync = app->rv = app->being_destroyed = app->error_inited = FALSE;
	app->fds.nfds = app->fds.count = 0;
	FD_ZERO(&app->fds.rmask);
	FD_ZERO(&app->fds.wmask);
	FD_ZERO(&app->fds.emask);
	_XtHeapInit(&app->heap);
	return app;
}

static XtAppContext *appDestroyList = NULL;
int _XtAppDestroyCount = 0;

static void DestroyAppContext(app)
	XtAppContext app;
{
	XtAppContext* prev_app = &app->process->appContextList;
	while (app->count-- > 0) XtCloseDisplay(app->list[app->count]);
	if (app->list != NULL) XtFree((char *)app->list);
	_XtFreeConverterTable(app->converterTable);
	_XtCacheFlushTag(app, (XtPointer)&app->heap);
	_XtHeapFree(&app->heap);
	while (*prev_app != app) prev_app = &(*prev_app)->next;
	*prev_app = app->next;
	if (app->process->defaultAppContext == app)
	    app->process->defaultAppContext = NULL;
	XtFree((char *)app);
}

void XtDestroyApplicationContext(app)
	XtAppContext app;
{
	if (app->being_destroyed) return;

	if (_XtSafeToDestroy) DestroyAppContext(app);
	else {
	    app->being_destroyed = TRUE;
	    _XtAppDestroyCount++;
	    appDestroyList =
		    (XtAppContext *) XtRealloc((char *) appDestroyList,
		    (unsigned) (_XtAppDestroyCount * sizeof(XtAppContext)));
	    appDestroyList[_XtAppDestroyCount-1] = app;
	}
}

void _XtDestroyAppContexts()
{
	int i;

	for (i = 0; i < _XtAppDestroyCount; i++) {
	    DestroyAppContext(appDestroyList[i]);
	}
	_XtAppDestroyCount = 0;
	XtFree((char *) appDestroyList);
	appDestroyList = NULL;
}

XrmDatabase XtDatabase(dpy)
	Display *dpy;
{
    return (XrmDatabase) dpy -> db;
}

typedef struct _PerDisplayTable {
	Display *dpy;
	XtPerDisplayStruct perDpy;
	struct _PerDisplayTable *next;
} PerDisplayTable, *PerDisplayTablePtr;

static PerDisplayTablePtr perDisplayList = NULL;

XtPerDisplay _XtGetPerDisplay(dpy)
	Display *dpy;
{
	register PerDisplayTablePtr pd, opd;

	for (pd = perDisplayList; pd != NULL && pd->dpy != dpy; pd = pd->next){
	    opd = pd;
	}

	if (pd == NULL) {
	    XtErrorMsg("noPerDisplay", "getPerDisplay", "XtToolkitError",
		    "Couldn't find per display information",
		    (String *) NULL, (Cardinal *)NULL);
	}

	if (pd != perDisplayList) {	/* move it to the front */
	    /* opd points to the previous one... */

	    opd->next = pd->next;
	    pd->next = perDisplayList;
	    perDisplayList = pd;
	}

	return &(pd->perDpy);
}

XtAppContext XtDisplayToApplicationContext(dpy)
	Display *dpy;
{
	return _XtGetPerDisplay(dpy)->appContext;
}

static void _XtHeapInit(heap)
    Heap*	heap;
{
    heap->start = NULL;
    heap->bytes_remaining = 0;
}

char* _XtHeapAlloc(heap, bytes)
    Heap*	heap;
    Cardinal	bytes;
{
    register char* heap_loc;
    if (heap == NULL) return XtMalloc(bytes);
    if (heap->bytes_remaining < bytes) {
	int segment_size = ((bytes + sizeof(char*)) > HEAP_SEGMENT_SIZE)
			    ? bytes + sizeof(char*) : HEAP_SEGMENT_SIZE;
	heap_loc = XtMalloc((unsigned)segment_size);
	*(char**)heap_loc = heap->start;
	heap->start = heap_loc;
	heap->current = heap_loc + sizeof(char*);
	heap->bytes_remaining = segment_size - sizeof(char*);
    }
#ifdef WORD64
    /* round to nearest 8-byte boundary */
    bytes = (bytes + 7) & (~7);
#else
    /* round to nearest 4-byte boundary */
    bytes = (bytes + 3) & (~3);
#endif /* WORD64 */
    heap_loc = heap->current;
    heap->current += bytes;
    heap->bytes_remaining -= bytes; /* can be negative */
    return heap_loc;
}

static void _XtHeapFree(heap)
    Heap*	heap;
{
    char* segment = heap->start;
    while (segment != NULL) {
	char* next_segment = *(char**)segment;
	XtFree(segment);
	segment = next_segment;
    }
    heap->start = NULL;
    heap->bytes_remaining = 0;
}

static XtPerDisplay NewPerDisplay(dpy)
	Display *dpy;
{
	PerDisplayTablePtr pd;

	pd = XtNew(PerDisplayTable);

	pd->dpy = dpy;
	pd->next = perDisplayList;
	perDisplayList = pd;

	return &(pd->perDpy);
}

static Display **dpyDestroyList = NULL;
int _XtDpyDestroyCount = 0;

static void CloseDisplay(dpy)
	Display *dpy;
{
        register XtPerDisplay xtpd;
	register PerDisplayTablePtr pd, opd;
	
	for (pd = perDisplayList; pd != NULL && pd->dpy != dpy; pd = pd->next){
	    opd = pd;
	}

	if (pd == NULL) {
	    XtErrorMsg("noPerDisplay", "closeDisplay", "XtToolkitError",
		    "Couldn't find per display information",
		    (String *) NULL, (Cardinal *)NULL);
	}

	if (pd == perDisplayList) perDisplayList = pd->next;
	else opd->next = pd->next;

	xtpd = &(pd->perDpy);

        if (xtpd != NULL) {
	    extern void _XtGClistFree();
	    XtDeleteFromAppContext(dpy, xtpd->appContext);
            XtFree((char *) xtpd->keysyms);
            XtFree((char *) xtpd->modKeysyms);
            XtFree((char *) xtpd->modsToKeysyms);
            xtpd->keysyms_per_keycode = 0;
            xtpd->being_destroyed = FALSE;
            xtpd->keysyms = NULL;
            xtpd->modKeysyms = NULL;
            xtpd->modsToKeysyms = NULL;
	    XDestroyRegion(xtpd->region);
	    _XtCacheFlushTag(xtpd->appContext, (XtPointer)&xtpd->heap);
	    _XtHeapFree(&xtpd->heap);
	    _XtGClistFree(xtpd->GClist);
	    XtFree((char *) xtpd->drawables);
        }
	XtFree((char*)pd);
	XrmDestroyDatabase(dpy->db);
	dpy->db = NULL;
	XCloseDisplay(dpy);
}

void XtCloseDisplay(dpy)
	Display *dpy;
{
	XtPerDisplay pd = _XtGetPerDisplay(dpy);
	
	if (pd->being_destroyed) return;

	if (_XtSafeToDestroy) CloseDisplay(dpy);
	else {
	    pd->being_destroyed = TRUE;
	    _XtDpyDestroyCount++;
	    dpyDestroyList = (Display **) XtRealloc((char *) dpyDestroyList,
		    (unsigned) (_XtDpyDestroyCount * sizeof(Display *)));
	    dpyDestroyList[_XtDpyDestroyCount-1] = dpy;
	}
}

void _XtCloseDisplays()
{
	int i;

	for (i = 0; i < _XtDpyDestroyCount; i++) {
	    CloseDisplay(dpyDestroyList[i]);
	}
	_XtDpyDestroyCount = 0;
	XtFree((char *) dpyDestroyList);
	dpyDestroyList = NULL;
}

XtAppContext XtWidgetToApplicationContext(w)
	Widget w;
{
	return _XtGetPerDisplay(XtDisplayOfObject(w))->appContext;
}
