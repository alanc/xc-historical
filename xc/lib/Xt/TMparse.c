/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* TMparse.c -- parse all X events into widget specific actions. */

#include <stdio.h>
#include "Intrinsic.h"
#include "X11/Xutil.h"
#include <strings.h>
#include "Atoms.h"
#include "TM.h"
#include "TMprivate.h"

/* Private definitions. */
#define LF 0x0a
#define BSLASH '\\'

#define AtomToAction(atom)	((XtAction)XrmAtomToQuark(atom))

typedef void (*ActionProc)();

typedef struct {
    XtAction	name;
    caddr_t	value;
} CompiledAction, *CompiledActionTable;

typedef struct _EventKey {
    char    	*event;
    XrmQuark	signature;
    unsigned long mask;
    unsigned long eventType;
}EventKey, *EventKeys;

typedef struct _ModifierKey {
    char     *name;
    XrmQuark	signature;
    short    mask;
}ModifierKey, *ModifierKeys;

ModifierKey modifiers[] = {
    {"Shift",0,		ShiftMask},
    {"Ctrl", 0,		ControlMask},
    {"Lock", 0,		LockMask},
    {"Meta", 0,		Mod1Mask},
    {"Mod1", 0,		Mod1Mask},
    {"Mod2", 0,		Mod2Mask},
    {"Mod3", 0,		Mod3Mask},
    {"Mod4", 0,		Mod4Mask},
    {"Mod5", 0,		Mod5Mask},
    {NULL, NULL, NULL},
};

EventKey events[] = {
    {"Key", 		NULL, KeyPressMask,	KeyPress},
    {"Btn1Down", 	NULL, ButtonPressMask,	ButtonPress},
    {"Btn1Up", 		NULL, ButtonReleaseMask,ButtonRelease},
    {"Btn2Down", 	NULL, ButtonPressMask,	ButtonPress},
    {"Btn2Up", 		NULL, ButtonReleaseMask,ButtonRelease},
    {"Btn3Down", 	NULL, ButtonPressMask,  ButtonPress},
    {"Btn3Up", 		NULL, ButtonReleaseMask,ButtonRelease},
    {"Btn4Down", 	NULL, ButtonPressMask,	ButtonPress},
    {"Btn4Up", 		NULL, ButtonReleaseMask,ButtonRelease},
    {"Btn5Down", 	NULL, ButtonPressMask,	ButtonPress},
    {"Btn5Up", 		NULL, ButtonReleaseMask,ButtonRelease},
    {"PtrMoved", 	NULL, PointerMotionMask,MotionNotify},
    {"KeyPress", 	NULL, KeyPressMask,	KeyPress},
    {"KeyRelease", 	NULL, KeyReleaseMask,	KeyRelease},
    {"MouseMoved", 	NULL, PointerMotionMask,MotionNotify},
    {"MotionNotify", 	NULL, PointerMotionMask,MotionNotify},
    {"EnterWindow", 	NULL, EnterWindowMask,	EnterNotify},
    {"LeaveWindow", 	NULL, LeaveWindowMask,	LeaveNotify},
    {"FocusIn", 	NULL, FocusChangeMask,	FocusIn},
    {"FocusOut", 	NULL, FocusChangeMask,	FocusOut},
    {"KeymapNotify", 	NULL, KeymapStateMask,	KeymapNotify},
    {"Expose", 		NULL, ExposureMask,	Expose},
    {"GraphicsExpose", 	NULL, ExposureMask,	GraphicsExpose},
    {"NoExpose", 	NULL, ExposureMask,	NoExpose},
    {"VisibilityNotify",NULL, VisibilityChangeMask,     VisibilityNotify},
    {"CreateNotify", 	NULL, SubstructureNotifyMask,   CreateNotify},
    {"DestroyNotify", 	NULL, SubstructureNotifyMask,   DestroyNotify},
    {"UnmapNotify", 	NULL, SubstructureNotifyMask,   UnmapNotify},
    {"MapNotify", 	NULL, SubstructureNotifyMask,   MapNotify},
    {"MapRequest", 	NULL, SubstructureRedirectMask, MapRequest},
    {"ReparentNotify", 	NULL, SubstructureNotifyMask,   ReparentNotify},
    {"ConfigureNotify", NULL, SubstructureNotifyMask,   ConfigureNotify},
    {"ConfigureRequest",NULL, SubstructureRedirectMask, ConfigureRequest},
    {"GravityNotify", 	NULL, SubstructureNotifyMask,   GravityNotify},
    {"ResizeRequest", 	NULL, ResizeRedirectMask,       ResizeRequest},
    {"CirculateNotify",	NULL, SubstructureNotifyMask,	CirculateNotify},
    {"CirculateRequest",NULL, SubstructureRedirectMask,	CirculateRequest},
    {"PropertyNotify",	NULL, PropertyChangeMask,       PropertyNotify},
    {"SelectionClear",	NULL, SubstructureRedirectMask,	SelectionClear},
    {"SelectionRequest",NULL, SubstructureRedirectMask,	SelectionRequest},
    {"SelectionNotify",	NULL, SubstructureRedirectMask,	SelectionNotify},
    {"ColormapNotify",	NULL, ColormapChangeMask,       ColormapNotify},
    {"ClientMessage",	NULL, 0, 		ClientMessage},
    { NULL, NULL, NULL, NULL}
};

static Boolean initialized = FALSE;

static void FreeEventSeq(event)
  EventSeqPtr event;
{
  if (event->next != NULL)
	FreeEventSeq(event->next);
  XtFree((char *)event->str);
  XtFree((char *)event);
}

static CompiledActionTable CompileActionTable(actions)
    struct _XtActionsRec *actions;
{
    int i, n;
    CompiledActionTable compiledActionTable;

    for (n=0; actions[n].string; n++) {};
    compiledActionTable = (CompiledActionTable) XtCalloc(
	n+1, sizeof(CompiledAction));

    for (i=0; i<n; i++) {
	compiledActionTable[i].name = AtomToAction(actions[i].string);
	compiledActionTable[i].value = actions[i].value;
    }
    compiledActionTable[n].name = NULL;
    compiledActionTable[n].value = NULL;

    return compiledActionTable;

}

static void FreeCompiledActionTable(compiledActionTable)
    CompiledActionTable compiledActionTable;
{
    XtFree((char *)compiledActionTable);
}

static Boolean ParseChar(s, c)
  char **s;
  char *c;
{
    Boolean result = FALSE;
    register char *ptr;

    ptr = *s;
    *c = *ptr++;
    if (*c == '\\') {
	result = TRUE;
	*c = *ptr++;
	switch (*c) {
	    case 't': *c = '\t';	break;
	    case 'n': *c = '\n';	break;
	    case 'r': *c = '\r';	break;
	    case 'e': *c = '\033';	break;
	}
	if (*c >= '0' && *c <= '9') {
	    *c = (*c - '0') * 64 + (ptr[0] - '0') * 8 + ptr[1] - '0';
	    ptr += 2;
	}
    }
    *s = ptr;
    return result;
}

static int StrToNum(s)
    char *s;
{
    int base = 10;
    register int val = 0;
    register int c;
    if (*s == '0') {
	s++;
	if (*s == 'x' || *s == 'X') {
	    base = 16;
	    s++;
	} else
	    base = 8;
    }
    while (*s)
        switch (c = *s++) {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	        val = (val * base) + (c - '0');
		break;
	    case '8':
	    case '9':
	    	if (base < 10)
			return -1;
	        val = (val * base) + (c - '0');
		break;
	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
	    case 'f':
	        if (base != 16)
			return -1;
	        val = (val * base) + (c - 'a' + 10);
		break;
	    case 'A':
	    case 'B':
	    case 'C':
	    case 'D':
	    case 'E':
	    case 'F':
	        if (base != 16)
			return -1;
	        val = (val * base) + (c - 'A' + 10);
		break;
	    default:
	    	return -1;
	}
	return val;		
}

static Syntax(str)
    char *str;
{
    (void) fprintf(stderr,
     "Event Sequence description error (ignored): %s\n", str);
}



static void LookupEventType(eventStr, eventType, eventMask)
  char *eventStr;
  unsigned long *eventType;
  unsigned long *eventMask;
{
    int i;
    XrmQuark	eventSignature;

    eventSignature = XrmAtomToQuark(eventStr);
    for (i = 0; events[i].event != NULL; i++) {
        if (events[i].signature == eventSignature) {
	    *eventType = events[i].eventType;
	    *eventMask |= events[i].mask;
	    break;
	}
    }
}


static Boolean ProcessModifier(modifierStr, curEvent)
  char *modifierStr;
  EventSeqPtr curEvent;
{
    XrmQuark	modifierSignature;
    int i;
    short modifierMask = 0;
    modifierSignature = XrmAtomToQuark(modifierStr);
    for (i = 0; modifiers[i].name != NULL; i++) {
        if (modifiers[i].signature == modifierSignature) {
	    modifierMask = modifiers[i].mask;
	    break;
	}
    }
    curEvent->modifiersMask |= modifierMask;
    if (modifierMask) 
	return TRUE;
    else
	return FALSE;
}

/*
 * Stores the necessary info for one event on the rhs of a TranslationTable line 
 */
static void ProcessEvent(eventTypeStr, eventCodeStr, eventCharStr, repsStr, 
			 curEvent, eventMask)
  char *eventTypeStr, *eventCodeStr, *eventCharStr, *repsStr;
  EventSeqPtr curEvent;
  unsigned long *eventMask;
{
    int i=0;

    /*** if not first event on line, traverse the linked list of events ***/
    if (curEvent->eventType != NULL) {
	while (curEvent->next != NULL) curEvent = curEvent->next;
	curEvent->next = (EventSeqPtr) XtMalloc(sizeof(EventSeqRec));
	curEvent = curEvent->next;
	curEvent->str = NULL;
	curEvent->next = NULL;
    }

    /*** Parse the events ***/
    if (strcmp(eventTypeStr, "") != NULL) 
	LookupEventType(eventTypeStr,&curEvent->eventType, eventMask);

    
    /*** Parse the key code or character ***/
    if (strcmp(eventCodeStr, "") != NULL) {
        if (eventCodeStr[0] >= '0' && eventCodeStr[0] <= '9')
            curEvent->eventCode = StrToNum (eventCodeStr);
        else 
	    curEvent->eventCode = (eventCodeStr[0] >= 'A' && eventCodeStr[0] <= 'Z')
                                ? (eventCodeStr[0] + 'a' - 'A') : eventCodeStr[0];
    }

    /*** Parse the repetitions, for double click... ***/
    if (strcmp(repsStr, "") != NULL) {
	EventSeqPtr tempEvent = curEvent;
	int reps;
        if (repsStr[0] >= '0' && repsStr[0] <= '9')
            reps = StrToNum (repsStr);
        else
	    reps = 1;
	for (i=1; i<reps; i++) {
	    curEvent->next = (EventSeqPtr) XtMalloc(sizeof(EventSeqRec));
	    curEvent = curEvent->next;
	    curEvent->str = NULL;
	    curEvent->next = NULL;
	    curEvent->eventCode = tempEvent->eventCode;
	    curEvent->eventType = tempEvent->eventType;
	    curEvent->modifiersMask = tempEvent->modifiersMask;
	}
    }

    /*** Parse the commands that are in double quotes (keyboard sequences) **/
    if (strcmp(eventCharStr, "") != NULL) {
        do {
            int j;
            if (i > 0) {
	        curEvent->next = (EventSeqPtr) XtMalloc(sizeof(EventSeqRec));
	        curEvent = curEvent->next;
		curEvent->modifiersMask = 0;
	        curEvent->next = NULL;
		curEvent->str = NULL;
	    }
	    for (j=0; j++ < 2; )
	        if (eventCharStr[i] == '^') {
		    i++;
		    (void) ProcessModifier("Ctrl", curEvent);
	        } else if (eventCharStr[i] == '@') {
		    i++;
		    (void) ProcessModifier("Meta", curEvent);
                }
	    LookupEventType("Key", &curEvent->eventType, eventMask);
	    curEvent->eventCode = (eventCharStr[i] >= 'A' && eventCharStr[i] <='Z')
                            ? (eventCharStr[i] + 'a' - 'A') : eventCharStr[i];
            i++;
        } while (eventCharStr[i] != 0);
    }
    /* if (curEvent->eventType == 0) bogus line.... */
}


/***********************************************************************
 * InterpretAction
 * Given an action, it returns a pointer to the appropriate procedure.
 ***********************************************************************/

static caddr_t InterpretAction(compiledActionTable, action)
  CompiledActionTable compiledActionTable;
  XtAction action; 
{
/* !!! should implement via hash or something else faster than linear search */

    int i;

    for (i=0;compiledActionTable[i].name != NULL;i++) {
	if (compiledActionTable[i].name == action)
	    return compiledActionTable[i].value;
    }

    return NULL;
}


/***********************************************************************
 * ParseTranslationTableProduction
 * Parses one line of event bindings.
 ***********************************************************************/

static void ParseTranslationTableProduction(w, compiledActionTable, str)
  Widget w;
  CompiledActionTable	compiledActionTable;
  char *str;
{
    char    str2[500], 
	    eventTypeStr[50],  /* string between < > */
	    eventCodeStr[50],  /* string after <Key> or <button...> */
	    eventCharStr[50],  /* string inside of "" */
	    repsStr[50];       /* # of button clicks, found after <button...>*/
    char   *ptr, *ptr2, c;
    unsigned long   eventType;
    EventSeqPtr curEvent, firstEvent;
    ActionPtr actions;
    Boolean inQuotes = FALSE, done = FALSE;

    ptr = str;
    eventTypeStr[0] = 0;
    eventCodeStr[0] = 0;
    eventCharStr[0] = 0;
    repsStr[0] = 0;
    ptr2 = eventTypeStr;
    curEvent = (EventSeqPtr) XtMalloc(sizeof(EventSeqRec));
    firstEvent = curEvent;
    curEvent->modifiersMask = 0;
    curEvent->eventType = 0;
    curEvent->eventCode = 0;
    curEvent->next = NULL;
    curEvent->actions = NULL;
    curEvent->str = XtMalloc(strlen(str)+1);
    (void) strcpy(curEvent->str, str);
    while (*ptr != ':') {	/* find end of lhs  and collect pieces */
	switch (*ptr) {
	    case 0: 
	    case LF: 
		Syntax(str);
		return;
	    case BSLASH: 
		ptr++;
		*ptr2++ = *ptr++;
		break;
	    case '<': 
		*ptr2 = 0;
		if (strcmp(eventCodeStr,"") || strcmp(repsStr,"")) 
		    done = TRUE;
		ptr2 = eventTypeStr;
		ptr++;
		break;
	    case '>': 
		*ptr2 = 0;
		if (ProcessModifier(eventTypeStr, curEvent)) {
			eventTypeStr[0] = 0;
			ptr2 = eventTypeStr;
			ptr++;
			break;
		}
		LookupEventType(eventTypeStr, &eventType, &w->core.event_mask);
		switch (eventType) {
		    case KeyPress:
		    case KeyRelease:
			ptr2 = eventCodeStr;
			break;
		    case ButtonPress:
		    case ButtonRelease:
			(void) strncpy(eventCodeStr, eventTypeStr+3, 1);
			eventCodeStr[1] = 0;
			ptr2 = repsStr;
			break;
		    case -1:
			Syntax(str);   /* Error */
			FreeEventSeq(curEvent);
			return;
		    default:
			ptr2 = eventTypeStr;
			done = TRUE;
		}
		ptr++;
		break;
	    case '\"':
		*ptr2 = 0;
		if (inQuotes) {
		    inQuotes = FALSE;		
		    done = TRUE;
		} else {
		    inQuotes = TRUE;
		    ptr2 = eventCharStr;
		}
		ptr++;
		break;
	    case ' ':
  		*ptr2 = 0;
		ptr++;
		done = TRUE;
		break;
	    default: 
		*ptr2++ = *ptr++;
	}
        if (done) {   /** done with one event **/
	    done = FALSE;
	    ProcessEvent(eventTypeStr, eventCodeStr, eventCharStr, repsStr, 
		         curEvent, &w->core.event_mask);
	    eventTypeStr[0] = 0;
	    eventCodeStr[0] = 0;
	    eventCharStr[0] = 0;
	    repsStr[0] = 0;
	}
    } 
    *ptr2 = 0;
    if (*--ptr != ':')
    if (strcmp(eventTypeStr, ""))
        ProcessEvent(eventTypeStr, eventCodeStr, eventCharStr, 
		     repsStr, curEvent, &w->core.event_mask);
    ptr++;
    ptr++;			/* step over ':' */

    /*** process rhs ***/
    curEvent->actions = (ActionPtr) XtMalloc(sizeof(ActionRec));
    actions = curEvent->actions;
    actions->proc = NULL;
    actions->param = NULL;
    actions->token = NULL;
    actions->next = NULL;
    while (*ptr && !done) {
	while (*ptr == ' ' || *ptr == '\t')
	    ptr++;
	switch (*ptr) {
	    case '\n': 
	    case '#':
	    case 0: 
		done = TRUE;
		break;
	    default: 
		ptr2 = str2;
		do {
		    (void) ParseChar (&ptr, ptr2);
		    c = *ptr2++;
		} while (
		       c != ' '
		    && c != '\t'
		    && c != '\n'
		    && c != 0
		    && c != '(');
		*--ptr2 = 0;
		actions->proc = (ActionProc) InterpretAction(
		    compiledActionTable, AtomToAction (str2));
		actions->token = (char *) XtMalloc(strlen(str2)+1);
		(void) strcpy(actions->token, str2);
		if (actions->proc == NULL) {
		    Syntax(str);
		    FreeEventSeq(curEvent);
		    return;
                }
		/** only temporary **/
		actions->param = NULL;
		actions->paramNum = 0;
	}
	if (!done) {
 	    actions->next = (ActionPtr) XtMalloc(sizeof(ActionRec));
	    actions = actions->next;
	    actions->proc = NULL;
	    actions->param = NULL;
	    actions->token = NULL;
	    actions->next = NULL;
	}
    }
    while (curEvent != NULL && curEvent->eventType > 0) {
        w->core.translations->eventObjTbl = EventMapObjectCreate(
	    w->core.translations, curEvent); 
	curEvent = curEvent->next;
    }
    curEvent = firstEvent;
    w->core.translations->eventObjTbl = EventMapObjectSet(
	w->core.translations, curEvent); 
    FreeEventSeq(curEvent);
}

/*
 * Parses a user's or applications translation table
 */

static void ParseTranslationTable(w, compiledActionTable)
    Widget w;
    CompiledActionTable	compiledActionTable;
{
    char **translationTableSource = (char **)w->core.translations;
    int i;

    w->core.translations =
	(Translations) XtMalloc(sizeof(TranslationData));
    w->core.translations->numEvents = 0;
    w->core.translations->eventTblSize = 0;
    w->core.translations->eventObjTbl = NULL;

    /* !!! need some way of setting this !!! */
    w->core.translations->clickTime = 50;

    i = 0;
    while (translationTableSource[i]) {
        ParseTranslationTableProduction(
	    w, compiledActionTable, translationTableSource[i]);
	i++;
    }

}

/*** public procedures ***/

void DefineTranslation(w)
  Widget w;
{
    /* this procedure assumes that there is a string table in the */
    /* core.translations field. It compiles it, combines it with */
    /* the action bindings and puts the resulting internal data */
    /* structure into the core.translations field. Note that this means */
    /* that if you call DefineTranslation twice, bad things will happen */

    CompiledActionTable	compiledActionTable;

    if (w->core.widget_class->core_class.actions == NULL) return;

    compiledActionTable =
	CompileActionTable(w->core.widget_class->core_class.actions);
    ParseTranslationTable(w, compiledActionTable);
    FreeCompiledActionTable(compiledActionTable);


    /* double click needs to make sure that you have selected on both
       button down and up. */
    if (w->core.event_mask & ButtonPressMask || 
        w->core.event_mask & ButtonReleaseMask)
	   w->core.event_mask |= ButtonPressMask | ButtonReleaseMask;


    XtSetEventHandler(XtDisplay(w), w->core.window, TranslateEvent,
		      w->core.event_mask, NULL);
}

void TranslateInitialize()
{
    int i;
    if (initialized)
        return;
    initialized = TRUE;
    for (i = 0; events[i].event != NULL; i++)
         events[i].signature = XrmAtomToQuark(events[i].event);
    for (i = 0; modifiers[i].name != NULL; i++)
         modifiers[i].signature = XrmAtomToQuark(modifiers[i].name);
} 
