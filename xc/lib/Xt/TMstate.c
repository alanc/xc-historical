#ifndef lint
static char rcsid[] = "$Header: TMstate.c,v 1.35 87/10/29 16:54:12 swick Locked $";
#endif lint

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
/* TMstate.c -- maintains the state table of actions for the translation 
 *              manager.
 */

#ifndef VMS
#include "Xlib.h"
#include <strings.h>
#else
#include Xlib
#include string
#endif
#include "Intrinsic.h"
#include "Atoms.h"
#include "TM.h"
#include "TMprivate.h"

typedef struct _StateRec {
    int index;		/* index of event into EventObj table */
    ActionPtr actions;	/* rhs   list of actions to perform */
    StatePtr nextLevel;	/* the next level points to the next event
			   in one event sequence */
    StatePtr next;	/* points to other event state at same level */
    StatePtr forw;	/* points to next state in list of all states */
    Boolean cycle;	/* true iff nextLevel is a loop */
}  StateRec;


typedef struct _EventObjRec *EventObjPtr;

typedef struct _EventObjRec {
    Event event;	/* X event description */
    StatePtr state;	/* pointer to linked lists of state info */
} EventObjRec;

typedef struct _TranslationData {
    unsigned int	numEvents;
    unsigned int	eventTblSize;
    EventObjPtr		eventObjTbl;
    unsigned long	clickTime;
    unsigned long	lastEventTime;
    StatePtr		curState;
    StatePtr		head;	/* head of list of all states */
} TranslationData;

#define AtomToAction(atom)	((XtAction)StringToQuark(atom))

static void FreeActions(action)
  ActionPtr action;
{
    while (action != NULL) {
	ActionPtr next = action->next;

	if (action->params != NULL) {
	    Cardinal i;

	    for (i=0; i<action->num_params; i++) XtFree(action->params[i]);
	    XtFree((char *)action->params);
	}

	XtFree(action->token);
	XtFree((char *)action);
	action = next;
    }
}

static String PrintModifiers(str, mask, mod)
    String str;
    unsigned long mask, mod;
{
    if (mask & ShiftMask)
	(void) sprintf(str, "%sShift", ((mod & ShiftMask) ? "" : "~"));
    if (mask & ControlMask)
	(void) sprintf(str, "%sCtrl", ((mod & ControlMask) ? "" : "~"));
    if (mask & LockMask)
	(void) sprintf(str, "%sLock", ((mod & LockMask) ? "" : "~"));
    if (mask & Mod1Mask)
	(void) sprintf(str, "%sMeta", ((mod & Mod1Mask) ? "" : "~"));
    if (mask & Mod2Mask)
	(void) sprintf(str, "%sMod2", ((mod & Mod2Mask) ? "" : "~"));
    if (mask & Mod3Mask)
	(void) sprintf(str, "%sMod3", ((mod & Mod3Mask) ? "" : "~"));
    if (mask & Mod4Mask)
	(void) sprintf(str, "%sMod4", ((mod & Mod4Mask) ? "" : "~"));
    if (mask & Mod5Mask)
	(void) sprintf(str, "%sMod5", ((mod & Mod5Mask) ? "" : "~"));
    str += strlen(str);
    return str;
}

static String PrintEventType(str, event)
    String str;
    unsigned long event;
{
    switch (event) {
	case KeyPress:		sprintf(str, "<KeyPress>");		break;
	case KeyRelease:	sprintf(str, "<KeyRelease>");		break;
	case ButtonPress:	sprintf(str, "<ButtonPress>");		break;
	case ButtonRelease:	sprintf(str, "<ButtonRelease>");	break;
	case MotionNotify:	sprintf(str, "<MotionNotify>");		break;
	case EnterNotify:	sprintf(str, "<EnterNotify>");		break;
	case LeaveNotify:	sprintf(str, "<LeaveNotify>");		break;
	case FocusIn:		sprintf(str, "<FocusIn>");		break;
	case FocusOut:		sprintf(str, "<FocusOut>");		break;
	case KeymapNotify:	sprintf(str, "<KeymapNotify>");		break;
	case Expose:		sprintf(str, "<Expose>");		break;
	case GraphicsExpose:	sprintf(str, "<GraphicsExpose>");	break;
	case NoExpose:		sprintf(str, "<NoExpose>");		break;
	case VisibilityNotify:	sprintf(str, "<VisibilityNotify>");	break;
	case CreateNotify:	sprintf(str, "<CreateNotify>");		break;
	case DestroyNotify:	sprintf(str, "<DestroyNotify>");	break;
	case UnmapNotify:	sprintf(str, "<UnmapNotify>");		break;
	case MapNotify:		sprintf(str, "<MapNotify>");		break;
	case MapRequest:	sprintf(str, "<MapRequest>");		break;
	case ReparentNotify:	sprintf(str, "<ReparentNotify>");	break;
	case ConfigureNotify:	sprintf(str, "<ConfigureNotify>");	break;
	case ConfigureRequest:	sprintf(str, "<ConfigureRequest>");	break;
	case GravityNotify:	sprintf(str, "<GravityNotify>");	break;
	case ResizeRequest:	sprintf(str, "<ResizeRequest>");	break;
	case CirculateNotify:	sprintf(str, "<CirculateNotify>");	break;
	case CirculateRequest:	sprintf(str, "<CirculateRequest>");	break;
	case PropertyNotify:	sprintf(str, "<PropertyNotify>");	break;
	case SelectionClear:	sprintf(str, "<SelectionClear>");	break;
	case SelectionRequest:	sprintf(str, "<SelectionRequest>");	break;
	case SelectionNotify:	sprintf(str, "<SelectionNotify>");	break;
	case ColormapNotify:	sprintf(str, "<ColormapNotify>");	break;
	case ClientMessage:	sprintf(str, "<ClientMessage>");	break;
	case _XtEventTimerEventType:sprintf(str,"<EventTimer>");	break;
	case _XtTimerEventType:	sprintf(str, "<Timer>");		break;
	default: sprintf(str, "<0x%x>", (int) event);
    }
    str += strlen(str);
    return str;
}

static String PrintCode(str, mask, code)
    String str;
    unsigned long mask, code;
{
    if (mask != 0) {
	if (mask != ~0L) (void) sprintf(str, "0x%lx:0x%lx", mask, code);
	else (void) sprintf(str, "0x%lx", code);
	str += strlen(str);
    }
    return str;
}

static String PrintEvent(str, event)
    String str;
    Event *event;
{
    str = PrintModifiers(str, event->modifierMask, event->modifiers);
    str = PrintEventType(str, event->eventType);
    str = PrintCode(str, event->eventCodeMask, event->eventCode);
    return str;
}

static String PrintParams(str, params, num_params)
    String str, *params;
    Cardinal num_params;
{
    Cardinal i;
    for (i = 0; i<num_params;i++) {
	if (i > 0) sprintf(str, ", ");
	str += strlen(str);
	sprintf(str, "\"%s\"", params[i]);
	str += strlen(str);
    }
    return str;
}

static String PrintActions(str, actions)
    String str;
    ActionPtr actions;
{
    while (actions != NULL && actions->token != NULL) {
        (void) sprintf(str, " %s(", actions->token);
	str += strlen(str);
	str = PrintParams(str, actions->params, actions->num_params);
	str += strlen(str);
	sprintf(str, ")");
	str += strlen(str);
	actions = actions->next;
    }
    return str;
}

static int MatchEvent(translations, eventSeq) 
  XtTranslations translations;
  EventSeqPtr eventSeq;
{
    EventObjPtr eventTbl = translations->eventObjTbl;
    int i;

    for (i=0; i < translations->numEvents; i++) {
        if ((eventTbl[i].event.eventType == eventSeq->event.eventType)
	    && (eventTbl[i].event.eventCode ==
		(eventTbl[i].event.eventCodeMask & eventSeq->event.eventCode))
/*
 * the use of "Any" can cause obscure bugs since an incoming event may match
 * the "Any" alternative even though a more specific (and correct) event is
 * in the table. It's hard to know which event in the table to match since
 * either could be correct, depending on the circumstances. It's unfortunate
 * that there isn't a unique identifier for a given event... The "any"
 * should be used only when all else fails, but this complicates the algorithms
 * quite a bit. Relying on the order of the productions in the translation
 * table helps, but is not sufficient, both because the earlier specific
 * event may not apply to the current state, and because we can merge
 * translations, resulting in events in the table that are "out of order"
 */
	    && ((eventTbl[i].event.modifiers == AnyModifier)
	        || (eventTbl[i].event.modifiers == eventSeq->event.modifiers))
/*
	    && (eventTbl[i].event.modifiers == 
		(eventTbl[i].event.modifierMask & eventSeq->event.modifiers))
*/
	   )
		return(i);
   }
    return(-1);
}


/*
 * there are certain cases where you want to ignore the event and stay
 * in the same state.
 */
static Boolean Ignore(event)
  EventSeqPtr event;   
{
    if (
           event->event.eventType == MotionNotify
/*
        || event->event.eventType == ButtonPress
	|| event->event.eventType == ButtonRelease
*/
       )
	    return TRUE;
    else
	    return FALSE;
}


static void XEventToTMEvent(event, tmEvent)
    XEvent *event;
    EventPtr tmEvent;
{
    tmEvent->event.eventCodeMask = 0;
    tmEvent->event.eventCode = 0;
    tmEvent->event.modifierMask = 0;
    tmEvent->event.modifiers = 0;
    tmEvent->event.eventType = event->type;

    switch (event->type) {

	case KeyPress:
	case KeyRelease:
	    tmEvent->event.modifiers = event->xkey.state;
	    event->xkey.state = 0;
	    tmEvent->event.eventCode = XLookupKeysym(&event->xkey, 0);
	    event->xkey.state = tmEvent->event.modifiers;
	    break;

	case ButtonPress:
 	    tmEvent->event.eventCode = event->xbutton.button;
	    tmEvent->event.modifiers = event->xbutton.state;
	    break;

	case ButtonRelease:
	    tmEvent->event.eventCode = event->xbutton.button;
	    tmEvent->event.modifiers = event->xbutton.state;
	    break;

	case MotionNotify:
	    tmEvent->event.modifiers = event->xmotion.state;
	    break;

	case EnterNotify:
	case LeaveNotify:
	    tmEvent->event.modifiers = event->xcrossing.state;
	    break;

	default:
	    break;
    }
}


static unsigned long GetTime(stateTable, event)
    XtTranslations stateTable;
    XEvent *event;
{
    switch (event->type) {

        case KeyPress:
	case KeyRelease:
	    return event->xkey.time;

        case ButtonPress:
	case ButtonRelease:
	    return event->xbutton.time;

#ifdef notdef
        case EnterWindow:
	case LeaveWindow:
	    return event->xcrossing.time;
#endif

#ifdef notdef
	case MotionNotify:
	    return stateTable->lastEventTime;
#endif
	default:
	    return stateTable->lastEventTime;

    }

}


/* ARGSUSED */
static void _XtTranslateEvent (w, closure, event)
Widget w;
Opaque closure;
register    XEvent * event;
{
    XtTranslations stateTable = w->core.translations;
    StatePtr oldState;
    EventRec curEvent;
    int     index;
    ActionPtr actions;

/* gross disgusting special case ||| */
    if ((event->type == EnterNotify || event->type == LeaveNotify)
        && event->xcrossing.detail == NotifyInferior)
	return;

    XEventToTMEvent (event, &curEvent);

#ifdef notdef
    if (event->type == ButtonPress
	    && stateTable->buttonUp
	    && stateTable->curState != NULL)
	if (
		(stateTable->lastEventTime + stateTable->clickTime)
		< event->xbutton.time
	    )
	    stateTable->curState = NULL;

    if (event->type == ButtonRelease)
	stateTable->buttonUp = TRUE;
    else if (event->type != MotionNotify)
        stateTable->buttonUp = FALSE;
#endif

    index = MatchEvent (stateTable, &curEvent);
    if (index == -1)
	/* some event came in that we don't have any states for */
	/* ignore it. */
	return;

    /* are we currently in some state other than ground? */
    if (stateTable->curState != NULL) {

	oldState = stateTable->curState;

	/* find this event in the current level */
	while (stateTable->curState != NULL) {
	    Event *ev;
	    /* does this state's index match? --> done */
	    if (stateTable->curState->index == index) break;

	    /* is this an event timer? */
	    ev = &stateTable->eventObjTbl[
		stateTable->curState->index].event;
	    if (ev->eventType == _XtEventTimerEventType) {

		/* does the succeeding state match? */
		StatePtr nextState = stateTable->curState->nextLevel;

		/* is it within the timeout? */
		if (nextState != NULL && nextState->index == index) {
		    unsigned long time = GetTime(stateTable, event);
		    unsigned long delta = ev->eventCode;
		    if (delta == 0) delta = stateTable->clickTime;
		    if (stateTable->lastEventTime + delta >= time) {
			stateTable->curState = nextState;
			break;
		    }
		}
	    }

	    /* go to next state */
	    stateTable->curState = stateTable->curState->next;
	}

	if (stateTable->curState == NULL)
	    /* couldn't find it... */
	    if (Ignore(&curEvent)) {
		/* ignore it. */
	        stateTable->curState = oldState;
		return;
	    } /* do ground state */
    }

    if (stateTable->curState == NULL) {
	/* check ground level */
	stateTable->curState = stateTable->eventObjTbl[index].state;
	if (stateTable->curState == NULL) return;
    }

    stateTable->lastEventTime = GetTime (stateTable, event);

    /* perform any actions */
    actions = stateTable->curState->actions;
    while (actions != NULL) {
	/* perform any actions */
	if (actions->proc != NULL)
	    (*(actions->proc)) (
		w, event, actions->params, actions->num_params);
	actions = actions->next;
    }

    /* move into successor state */
    stateTable->curState = stateTable->curState->nextLevel;
}

static Boolean EqualEvents(event1, event2)
    Event *event1, *event2;
{
    return (
	   event1->eventType     == event2->eventType
	&& event1->eventCode     == event2->eventCode
	&& event1->eventCodeMask == event2->eventCodeMask
	&& event1->modifiers     == event2->modifiers
	&& event1->modifierMask  == event2->modifierMask);
}

static int GetEventIndex(stateTable, event)
    XtTranslations stateTable;
    EventPtr event;
{
    int	index;
    EventObjPtr new;
    EventObjPtr eventTbl = stateTable->eventObjTbl;

    for (index=0; index < stateTable->numEvents; index++)
        if (EqualEvents(&eventTbl[index].event, &event->event)) return(index);

    if (stateTable->numEvents == stateTable->eventTblSize) {
        stateTable->eventTblSize += 100;
	stateTable->eventObjTbl = (EventObjPtr) XtRealloc(
	    (char *)stateTable->eventObjTbl, 
	    stateTable->eventTblSize*sizeof(EventObjRec));
    }

    new = &stateTable->eventObjTbl[stateTable->numEvents];

    new->event = event->event;
    new->state = NULL;

    return stateTable->numEvents++;
}

static StatePtr NewState(index, stateTable)
    int index;
    XtTranslations stateTable;
{
    StatePtr state = XtNew(StateRec);

    state->index = index;
    state->nextLevel = NULL;
    state->next = NULL;
    state->actions = NULL;
    state->forw = stateTable->head;
    state->cycle = FALSE;
    stateTable->head = state;
/*
    state->back = NULL;
    if (state->forw != NULL) state->forw->back = state;
*/

    return state;
}

typedef NameValueRec CompiledAction;
typedef NameValueTable CompiledActionTable;

static CompiledActionTable CompileActionTable(actions, count)
    struct _XtActionsRec *actions;
    Cardinal count;
{
    int i;
    CompiledActionTable compiledActionTable;

    compiledActionTable = (CompiledActionTable) XtCalloc(
	count+1, (unsigned) sizeof(CompiledAction));

    for (i=0; i<count; i++) {
	compiledActionTable[i].name = actions[i].string;
	compiledActionTable[i].signature = AtomToAction(actions[i].string);
	compiledActionTable[i].value = (Value) actions[i].proc;
    }

    compiledActionTable[count].name = NULL;
    compiledActionTable[count].signature = NULL;
    compiledActionTable[count].value = NULL;

    return compiledActionTable;
}

/***********************************************************************
 * InterpretAction
 * Given an action, it returns a pointer to the appropriate procedure.
 ***********************************************************************/

static XtActionProc InterpretAction(compiledActionTable, action)
    CompiledActionTable compiledActionTable;
    String action;
{
    Value actionProc;

    if (_XtLookupTableSym(compiledActionTable, action, &actionProc))
	return (XtActionProc) actionProc;

    return (XtActionProc) NULL;
}

static int BindActions(stateTable, compiledActionTable)
    XtTranslations stateTable;
    CompiledActionTable compiledActionTable;
{
    StatePtr state;
    ActionPtr action;
    int unbound = 0;

    for (state=stateTable->head; state != NULL; state=state->forw)
        for (action=state->actions; action != NULL; action=action->next)
	    if (action->proc == NULL) {
	      action->proc = InterpretAction(
	        compiledActionTable, action->token);
	      if (action->proc == NULL) unbound++;
	    }
    return(unbound);
}


static void FreeCompiledActionTable(compiledActionTable)
    CompiledActionTable compiledActionTable;
{
    XtFree((char *)compiledActionTable);
}

static EventMask EventToMask(event)
    EventObjPtr	event;
{
static EventMask masks[] = {
        0,			/* Error, should never see */
        0,			/* Reply, should never see */
        KeyPressMask,		/* KeyPress */
        KeyReleaseMask,		/* KeyRelease */
        ButtonPressMask,	/* ButtonPress */
        ButtonReleaseMask,	/* ButtonRelease */
        PointerMotionMask	/* MotionNotify */
		| Button1MotionMask
		| Button2MotionMask
		| Button3MotionMask
		| Button4MotionMask
		| Button5MotionMask
		| ButtonMotionMask,
        EnterWindowMask,	/* EnterNotify */
        LeaveWindowMask,	/* LeaveNotify */
        FocusChangeMask,	/* FocusIn */
        FocusChangeMask,	/* FocusOut */
        KeymapStateMask,	/* KeymapNotify */
        ExposureMask,		/* Expose */
        0,			/* GraphicsExpose, in GC */
        0,			/* NoExpose, in GC */
        VisibilityChangeMask,	/* VisibilityNotify */
        SubstructureNotifyMask,	/* CreateNotify, should never come see*/
        StructureNotifyMask,	/* DestroyNotify */
/*		| SubstructureNotifyMask, */
        StructureNotifyMask,	/* UnmapNotify */
/*		| SubstructureNotifyMask, */
        StructureNotifyMask,	/* MapNotify */
/*		| SubstructureNotifyMask, */
        SubstructureRedirectMask,/* MapRequest */
        StructureNotifyMask,	/* ReparentNotify */
        StructureNotifyMask,	/* ConfigureNotify */
/*		| SubstructureNotifyMask, */
        SubstructureRedirectMask,/* ConfigureRequest */
        StructureNotifyMask,	/* GravityNotify */
/*		| SubstructureNotifyMask, */
        ResizeRedirectMask,	/* ResizeRequest */
        StructureNotifyMask,	/* CirculateNotify */
/*		| SubstructureNotifyMask, */
        SubstructureRedirectMask,/* CirculateRequest */
        PropertyChangeMask,	/* PropertyNotify */
        0,			/* SelectionClear */
        0,			/* SelectionRequest */
        0,			/* SelectionNotify */
        ColormapChangeMask,	/* ColormapNotify */
        0,			/* ClientMessage */
        0 ,			/* MappingNotify */
    };

    /* Events sent with XSendEvent will have high bit set. */
    unsigned long eventType = event->event.eventType & 0x7f;

    return ((eventType >= XtNumber(masks)) ?  0 : masks[eventType]);
}
/*** Public procedures ***/

void _XtInstallTranslations(widget, stateTable)
    Widget widget;
    XtTranslations stateTable;
{
    EventMask	eventMask = 0;
    Boolean	nonMaskable = FALSE;
    Cardinal	i;
    static struct {
        unsigned long	modifier;
	EventMask	mask;
      } buttonMotionMask[] = {
	{Button1Mask, Button1MotionMask}, 
	{Button2Mask, Button2MotionMask}, 
	{Button3Mask, Button3MotionMask}, 
	{Button4Mask, Button4MotionMask}, 
	{Button5Mask, Button5MotionMask},
      };

    widget->core.translations = stateTable;
    if (stateTable == NULL) return;

    for (i = 0; i < stateTable->numEvents; i++) {
        EventObjPtr eventObj = &stateTable->eventObjTbl[i];
	EventMask mask = EventToMask(eventObj);
	unsigned long modifiers = eventObj->event.modifiers;

	if ((eventObj->event.eventType == MotionNotify)
	    && !(modifiers & ~AnyButtonModifier)) {
	    /* optimize traffic when PointerMotion only with button down */
	    if (modifiers == AnyButtonModifier)
	        mask = ButtonMotionMask;
	    else {
	        int mod;
	        mask = 0;
		for (mod = 0; mod < XtNumber(buttonMotionMask); mod++) {
		    if (modifiers & buttonMotionMask[mod].modifier)
		        mask |= buttonMotionMask[mod].mask;
		}
	    }
	}

	eventMask |= mask;
	nonMaskable |= (mask == 0);
    }

    /* double click needs to make sure that you have selected on both
	button down and up. */

    if (eventMask & ButtonPressMask) eventMask |= ButtonReleaseMask;
    if (eventMask & ButtonReleaseMask) eventMask |= ButtonPressMask;

    XtAddEventHandler(
        widget, eventMask, nonMaskable, _XtTranslateEvent, (Opaque) NULL);

}

typedef struct _ActionListRec *ActionList;
typedef struct _ActionListRec {
   XrmQuark name;
   XtActionProc proc;
   ActionList next;
} ActionListRec;

static ActionList globalActionList = NULL;

static void BindToGlobalActions(stateTable)
    XtTranslations stateTable;
{
    StatePtr state;
    ActionPtr action;
    ActionList list;

    if (globalActionList == NULL) return;
    for (state=stateTable->head; state != NULL; state=state->forw)
        for (action=state->actions; action != NULL; action=action->next)
	    if (action->proc == NULL)  {
	      XrmQuark actionName = XrmAtomToQuark(action->token);
	      for (list = globalActionList; 
	           (list != NULL) && (action->proc == NULL); 
	           list = list->next)
	        if (list->name == actionName)
	            action->proc = list->proc;
	    }

}

void _XtBindActions(widget, stateTable)
    Widget widget;
    XtTranslations stateTable;
{
    int count = -1; /* initialize to non-zero */

    while (count && (widget->core.widget_class->core_class.actions != NULL)) {
	CompiledActionTable compiledActionTable =
	    CompileActionTable(
		widget->core.widget_class->core_class.actions,
		widget->core.widget_class->core_class.num_actions);
	count = BindActions(stateTable, compiledActionTable);
	FreeCompiledActionTable(compiledActionTable);
	widget = widget->core.parent;
    }
    if (count) BindToGlobalActions(stateTable);
}

void XtAddActions(actions, num_actions)
    XtActionList actions;
    Cardinal num_actions;
{
    
    ActionList rec;
    Cardinal i;

    for (i=0; i<num_actions; i++) {
	rec = XtNew(ActionListRec);
	rec->name = XrmAtomToQuark(actions[i].string);
	rec->proc = actions[i].proc;
	rec->next = globalActionList;
	globalActionList = rec;
    }
}

void _XtInitializeStateTable(stateTable)
    XtTranslations *stateTable;
{
    (*stateTable) = XtNew(TranslationData);
    (*stateTable)->numEvents = 0;
    (*stateTable)->eventTblSize = 0;
    (*stateTable)->eventObjTbl = NULL;
    (*stateTable)->clickTime = 50; /* ||| need some way of setting this !!! */
    (*stateTable)->lastEventTime = 0;
    (*stateTable)->curState = NULL;
    (*stateTable)->head = NULL;
}

void _XtAddEventSeqToStateTable(eventSeq, stateTable)
    EventSeqPtr eventSeq;
    XtTranslations stateTable;
{
    int     index;
    StatePtr *state;

    if (eventSeq == NULL) return;

    /* initialize event index and state ptr */
    /* note that all states in the event seq passed in start out null */
    /* we fill them in with the matching state as we traverse the list */

    index = GetEventIndex (stateTable, eventSeq);
    state = &stateTable->eventObjTbl[index].state;

    for (;;) {
    /* index is eventIndex for event */
    /* *state is head of state chain for current state */

	while (*state != NULL && (*state)->index != index)
	    state = &(*state)->next;
	if (*state == NULL) *state = NewState (index, stateTable);

	/* *state now points at state record matching event */
	eventSeq->state = *state;

	if (eventSeq->actions != NULL) {
	    if ((*state)->actions != NULL) {
		XtWarning ("Overriding earlier translation manager actions.");
		FreeActions((*state)->actions);
	    }
	    (*state)->actions = eventSeq->actions;
	}

    /* are we done? */
	eventSeq = eventSeq->next;
	if (eventSeq == NULL) break;
	if (eventSeq->state != NULL) {
	    /* we've been here before... must be a cycle in the event seq. */
	    (*state)->nextLevel = eventSeq->state;
	    (*state)->cycle = TRUE;
	    break;
	}

	if ((*state)->cycle) {

	    /* unroll the loop one state */
	    /* this code hurts my head... ||| think about multiple */
	    /* states pointing at same "next" state record */

	    StatePtr oldNextLevel = (*state)->nextLevel;
	    StatePtr newNextLevel = NewState(oldNextLevel->index, stateTable);

	    newNextLevel->actions = oldNextLevel->actions;
	    newNextLevel->nextLevel = oldNextLevel->nextLevel;
	    newNextLevel->next = oldNextLevel->next;
	    newNextLevel->cycle = TRUE;
	    (*state)->cycle = FALSE;
	    (*state)->nextLevel = newNextLevel;
	}
	state = &(*state)->nextLevel;
	index = GetEventIndex (stateTable, eventSeq);
    }
}

static void _XtFreeTranslations(translations, free_actions)
    XtTranslations translations;
    Boolean	free_actions;
{
    StatePtr state;

    /* !!! ref count this, it may be shared ||| */

    state = translations->head;
    while (state != NULL) {
	StatePtr next = state->next;
	if (state->actions != NULL && free_actions)
	    FreeActions(state->actions);
	XtFree((char *) state);
	state = next;
    }

    XtFree((char *)translations->eventObjTbl);
    XtFree((char *)translations);
}

#ifdef DWTVMS

void XT$FREETRANSLATIONS(translations)
    XtTranslations translations;
{ _XtFreeTranslations(translations, TRUE); }

#endif


void XtFreeTranslations(translations)
    XtTranslations translations;
{_XtFreeTranslations(translations, TRUE); }

typedef struct _StateMapRec *StateMap;
typedef struct _StateMapRec {
    StatePtr	old, new;
    StateMap	next;
} StateMapRec;

static void MergeStates(old, new, override, indexMap, oldTable, stateMap)
    StatePtr *old, new;
    Boolean override;
    Cardinal *indexMap;
    XtTranslations oldTable;
    StateMap stateMap;
{
    StatePtr state;
    StateMap oldStateMap = stateMap;

    while (new != NULL) {
	Cardinal index = indexMap[new->index];

	/* make sure old and new match */
	for (state = *old; ; state=state->next) {
	    if (state == NULL) {
		/* corresponding arc doesn't exist, add it */
		state = NewState(index, oldTable);
		state->next = *old;
		*old = state;
		break;
	    }

	    if (state->index == index) /* found it */ break;
	}
    
	/* state and new are pointing at corresponding state records */
	{
	    StateMap temp = XtNew(StateMapRec);
	    temp->next = stateMap;
	    stateMap = temp;
	    temp->old = state;
	    temp->new = new;
	}
    
	/* merge the actions */
	if (state->actions == NULL || override) state->actions = new->actions;
    
	if (new->cycle) {
	    /* we're in a cycle, search state map for corresponding state */
	    StateMap temp;
	    for (
		temp=stateMap;
		temp->new != new->nextLevel;
		temp=temp->next)
	        if (temp == NULL) XtError(
"Trying to merge translation tables with cycles, and can't resolve this cycle."
);
	    (*old)->nextLevel = temp->old;
	} else if (! (*old)->cycle || override) {
	    if ((*old)->cycle) (*old)->nextLevel = NULL;
	    MergeStates(
	        &(*old)->nextLevel,
		new->nextLevel,
		override,
		indexMap,
		oldTable,
		stateMap);
	}
    
	new = new->next;
    }
    while (stateMap != oldStateMap) {
	StateMap temp = stateMap;
	stateMap = stateMap->next;
	XtFree((char *)temp);
    }
}


static void MergeTables(old, new, override)
    XtTranslations old, new;
    Boolean override;
{
    Cardinal i;
    Cardinal *indexMap;

    if (new == NULL) return;
    if (old == NULL) {
	XtWarning("Old translation table was null, cannot modify.");
	return;
    }

    indexMap = (Cardinal *)XtCalloc(new->eventTblSize, sizeof(Cardinal));

    for (i=0; i < new->numEvents; i++) {
	Cardinal j;
	EventObjPtr newEvent = &new->eventObjTbl[i];

	for (j=0; j < old->numEvents; j++)
	    if (EqualEvents(
	        &newEvent->event, &old->eventObjTbl[j].event)) break;

	if (j==old->numEvents) {
	    if (j == old->eventTblSize) {
		old->eventTblSize += 100;
		old->eventObjTbl = (EventObjPtr) XtRealloc(
		    (char *)old->eventObjTbl, 
		    old->eventTblSize*sizeof(EventObjRec));
	    }
	    old->eventObjTbl[j] = *newEvent;
	    old->eventObjTbl[j].state = NULL;
	    old->numEvents++;
	}
	indexMap[i] = j;
    }

    for (i=0; i < new->numEvents; i++)
	MergeStates(
	    &old->eventObjTbl[indexMap[i]].state,
	    new->eventObjTbl[i].state,
	    override,
	    indexMap,
	    old,
	    NULL);
}


void _XtOverrideTranslations(old, new)
    XtTranslations old, new;
{
    XtTranslations temp = XtNew(TranslationData);
    TranslationData temp2;

    _XtInitializeStateTable(&temp);
    temp->clickTime = new->clickTime;
    /* merge in new table, overriding any existing bindings from old */
    MergeTables(temp, new, FALSE);
    MergeTables(temp, old, FALSE);
    temp2 = *old;
    *old = *temp;
    *temp = temp2;
    _XtFreeTranslations(temp, FALSE);
}


void _XtAugmentTranslations(old, new)
    XtTranslations old, new;
{
    /* merge in extra bindings, keeping old binding if any */
    MergeTables(old, new, FALSE);
}

#ifdef DWTVMS
void XT$OVERRIDETRANSLATIONS(widget, new)
    Widget widget;
    XtTranslations new;
{

#else

void XtOverrideTranslations(widget, new)
    Widget widget;
    XtTranslations new;
{
#endif
/*
    MergeTables(widget->core.translations, new, TRUE);
*/
    _XtOverrideTranslations(widget->core.translations, new);
}

#ifdef DWTVMS
void XT$AUGMENTTRANSLATIONS(widget, new)
    Widget widget;
    XtTranslations new;
{

#else

void XtAugmentTranslations(widget, new)
    Widget widget;
    XtTranslations new;
{
#endif
    MergeTables(widget->core.translations, new, FALSE);
}

static void PrintState(start, str, state, eot)
    String start, str;
    StatePtr state;
    EventObjPtr eot;
{
    String old = str;
    /* print the current state */
    if (state == NULL) return;

    str = PrintEvent(str, &eot[state->index].event);
    str += strlen(str);
    if (state->actions != NULL) {
	String temp = str;
	(void) sprintf(str, "%s: ", (state->cycle ? "(+)" : ""));
	while (*str) str++;
	(void) PrintActions(str, state->actions);
	printf("%s\n", start);
	str = temp; *str = '\0';
    }

    /* print succeeding states */
    if (!state->cycle)
	PrintState(start, str, state->nextLevel, eot);

    str = old; *str = '\0';

    /* print sibling states */
    PrintState(start, str, state->next, eot);
    *str = '\0';

}

static void TranslateTablePrint(translations)
    XtTranslations translations;
{
    Cardinal i;
    char buf[1000];

    for (i = 0; i < translations->numEvents; i++) {
	buf[0] = '\0';
	PrintState(
	   &buf[0],
	   &buf[0],
	   translations->eventObjTbl[i].state,
	   translations->eventObjTbl);
    }
}

/***********************************************************************
 *
 * Pop-up and Grab stuff
 *
 ***********************************************************************/

static Boolean XtConvertStringToBoolean(s, bP)
    String s;
    Boolean *bP;
{
    XrmValue fromVal, toVal;

    fromVal.addr = s;
    fromVal.size = strlen(s)+1;

    XrmConvert(NULL, XrmRString, fromVal, XrmRBoolean, &toVal);
    if (toVal.addr == NULL) return FALSE;

    *bP = (Boolean) *(int *)toVal.addr;

    return TRUE;
}

static Widget _XtFindPopup(widget, name)
    Widget widget;
    String name;
{
    Cardinal i;
    XrmQuark q;
    Widget w;

    q = XrmAtomToQuark(name);

    for (w=widget; w != NULL; w=w->core.parent)
	for (i=0; i<w->core.num_popups; i++)
	    if (w->core.popup_list[i]->core.xrm_name == q)
		return w->core.popup_list[i];

    return NULL;
}

static void _XtMenuPopup(widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal num_params;
{
    Boolean spring_loaded;
    Widget popup_shell;

    if (num_params != 2)
	XtError("_XtMenuPopup called with num_params != 2.");

    if (! XtConvertStringToBoolean(params[1], &spring_loaded))
	XtError("Bad first parameter to _XtMenuPopup.");

    popup_shell = _XtFindPopup(widget, params[0]);
    if (popup_shell == NULL) XtError("Can't find popup in _XtMenuPopup");

    if (spring_loaded) _XtPopup(popup_shell, XtGrabExclusive, TRUE);
    else _XtPopup(popup_shell, XtGrabNonexclusive, FALSE);
}

static void _XtMenuPopupAction(widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal num_params;
{
    String newParams[2];

    if (num_params != 1) XtError("MenuPopup wants exactly one argument.");

    newParams[0] = params[0];
    if (event->type == ButtonPress) newParams[1] = "True";
    else if (event->type == EnterNotify) newParams[1] = "False";
    else {
	XtWarning(
"Pop-up menu creation is only supported on ButtonPress or EnterNotify events."
);
	newParams[1] = "True";
    }

    _XtMenuPopup(widget, event, newParams, 2);
}


static void _XtMenuPopdownAction(widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal num_params;
{
    XtPopdown(widget);
}


void _XtRegisterGrabs(widget)
    Widget widget;
{
    XtTranslations stateTable;
    StatePtr state;
    ActionPtr action;

    if (! XtIsRealized(widget)) return;

    /* walk the state list looking for people with actions of */
    /* _XtMenuPopupAction */
    /* when you find one, do a grab on the triggering event */
    stateTable = widget->core.translations;

    for (state = stateTable->head; state != NULL; state = state->forw) {
	for (action = state->actions; action != NULL; action = action->next) {
	    if (action->proc == (XtActionProc) _XtMenuPopupAction) {
		Event *event;
		event = &stateTable->eventObjTbl[state->index].event;
		switch (event->eventType) {
		    case ButtonPress:
		    case ButtonRelease:
			XGrabButton(
			    XtDisplay(widget),
			    event->eventCode,
			    event->modifiers,
			    XtWindow(widget),
			    TRUE,
			    NULL,
			    GrabModeAsync,
			    GrabModeAsync,
			    None,
			    None
			);
			break;

		    case KeyPress:
		    case KeyRelease:
			XGrabKey(
			    XtDisplay(widget),
			    event->eventCode,
			    event->modifiers,
			    XtWindow(widget),
			    TRUE,
			    NULL,
			    GrabModeAsync,
			    GrabModeAsync
			);
			break;

		    default: break;
/*
XtWarning("Popping up menus is only supported on key and button events.");
*/
		}
	    }
	}
    }
}

static XtActionsRec tmActions[] = {
    {"MenuPopup", _XtMenuPopupAction},
    {"MenuPopdown", _XtMenuPopdownAction},
};


void _XtPopupInitialize() { XtAddActions(tmActions, XtNumber(tmActions)); }
