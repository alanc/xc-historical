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

#include "Xlib.h"
#include "Intrinsic.h"
#include "Atoms.h"
#include "TM.h"
#include "TMprivate.h"

extern char *strcpy();

static void FreeActionRecs(action)
  ActionPtr action;
{
    int i;
    if (action->next != NULL)
        FreeActionRecs(action->next);
    if (action->param != NULL)
        for (i=0; i<action->paramNum; i++)
            XtFree(action->param[i]);
    XtFree(action->token);
    XtFree((char *)action->param);
    XtFree((char *)action);
}


static void FreeStateRecs(state)
  StatePtr state;
{
    if (state->next != NULL)
	FreeStateRecs(state->next);
    if (state->nextLevel != NULL)
	FreeStateRecs(state->nextLevel);
    if (state->actions != NULL)
        FreeActionRecs(state->actions);
    XtFree((char *)state->actions);
    XtFree((char *)state->next);
    XtFree((char *)state->nextLevel);
}

static void PrintModifiers(mod)
    unsigned long mod;
{
    if (mod & ShiftMask)
	(void) printf("<Shift>");
    if (mod & ControlMask)
	(void) printf("<Ctrl>");
    if (mod & LockMask)
	(void) printf("<Lock>");
    if (mod & Mod1Mask)
	(void) printf("<Meta>");
    if (mod & Mod2Mask)
	(void) printf("<Mod2>");
    if (mod & Mod3Mask)
	(void) printf("<Mod3>");
    if (mod & Mod4Mask)
	(void) printf("<Mod4>");
    if (mod & Mod5Mask)
	(void) printf("<Mod5>");
}

static void PrintEvent(event, endStr)
    unsigned long event;
    char * endStr;
{
    (void) strcpy(endStr, "");
    switch (event) {
	case KeyPressMask:
	    (void) printf("<Key>");
	    break;
	case KeyReleaseMask:
	    (void) printf("<Key>");
	    break;
	case ButtonPressMask:
	    (void) printf("<Btn");
	    (void) strcpy(endStr, "Down>");
	    break;
	case ButtonReleaseMask:
	    (void) printf("<Btn");
	    (void) strcpy(endStr, "Up>");
	    break;
    }
}

static void PrintCode(code)
    unsigned long code;
{
    char ch;
    ch = code - 'a' + 'A';
    if (ch >= 'A' && ch <= 'Z')
	(void) printf("%c",ch);
    else
        (void) printf("%d", code);
}

static void PrintActions(ev, code, modif, actions, j)
    unsigned long ev[], modif[], code[];
    ActionPtr actions;
    int j;
{
    char endStr[20];
    int i;
    for (i=0; i<j; i++) {
	PrintModifiers(modif[i]);
	PrintEvent(ev[i], endStr);
	PrintCode(code[i]);
        (void) printf("%s ",endStr);
    }
    while (actions != NULL && actions->token != NULL) {
        (void) printf("---------> action = %s\n", actions->token);
	actions = actions->next;
    }
    (void) printf("\n");
}

/*
 * there are certain cases where you want to ignore the event and stay
 * in the same state.
 */
static Boolean SpecialCase(event)
  EventSeqPtr event;   
{
    if (event->eventType == MotionNotify || event->eventType == ButtonPress ||
	event->eventType == ButtonRelease)
	    return TRUE;
    else
	    return FALSE;
}


static EventObjPtr  CreateStates(translations, index, eventSeq)
    Translations translations;
    int index;
    EventSeqPtr eventSeq;
{
    EventObjPtr eventObjTbl = translations->eventObjTbl;
    StatePtr state;
    Boolean found = FALSE;
    ActionPtr actions = eventSeq->actions;

    if (eventObjTbl[index].state == NULL) {  
	eventObjTbl[index].state = (StatePtr) XtMalloc (sizeof(StateRec));
        state = eventObjTbl[index].state;
        state->index = index;
        state->nextLevel = NULL;
        state->next = NULL;
        state->actions = NULL; ;
    } else {
        state = eventObjTbl[index].state;
	do {
	    found = FALSE;
	    if (eventSeq->next != NULL && state->nextLevel != NULL) {
		eventSeq = eventSeq->next;
		index = EventIndex(translations, eventSeq);
		state = state->nextLevel;
		if (state->index == index) 
		    found = TRUE;
		while (state->next != NULL && !found) {
	            state = state->next;
		    if (state->index == index) 
		        found = TRUE;
		}
		if (!found) {
		    state->next = (StatePtr) XtMalloc(sizeof(StateRec));
		    state = state->next;
		    state->index = index;
		    state->nextLevel = NULL;
		    state->next = NULL;
		    state->actions = NULL;
		}
	    }
	} while (found);
    }
    while (eventSeq->next != NULL) {
	eventSeq = eventSeq->next;
 	index = EventIndex(translations, eventSeq);
	state->nextLevel = (StatePtr) XtMalloc(sizeof(StateRec));
	state = state->nextLevel;
	state->index = index;
	state->nextLevel = NULL;
	state->next = NULL;
	state->actions = NULL;
    }
    if (state->actions != NULL) 
	FreeActionRecs(state->actions);
    state->actions = actions;

    return eventObjTbl;
}


/*** Public procedures ***/
EventObjPtr EventMapObjectCreate(translations, eventSeq)
  Translations	translations;
  EventSeqPtr eventSeq;
{
    if (EventIndex(translations, eventSeq) >= 0)
	return translations->eventObjTbl;

    if (translations->numEvents == translations->eventTblSize) {
        translations->eventTblSize += 100;
	translations->eventObjTbl = (EventObjPtr) XtRealloc(
	    translations->eventObjTbl, 
	    translations->eventTblSize*sizeof(EventObjRec));
    }

    translations->eventObjTbl[translations->numEvents].eventType =
	eventSeq->eventType;
    translations->eventObjTbl[translations->numEvents].eventCode =
	eventSeq->eventCode;
    translations->eventObjTbl[translations->numEvents].modifiersMask =
	eventSeq->modifiersMask;
    translations->eventObjTbl[translations->numEvents].state = NULL;
    translations->numEvents++;
    return translations->eventObjTbl;
}


EventObjPtr EventMapObjectGet(translations, eventSeq)
  Translations translations;
  EventSeqPtr eventSeq;
{
    EventObjPtr eventTbl = translations->eventObjTbl;
    int index;
    if ((index = EventIndex(translations, eventSeq)) < 0)
	return NULL;
    else
        return &eventTbl[index];
}


EventObjPtr EventMapObjectSet(translations, eventSeq)
  Translations translations;
  EventSeqPtr eventSeq;
{
    EventObjPtr eventTbl = translations->eventObjTbl;
    int index;
    if ((index = EventIndex(translations, eventSeq)) >= 0)
        eventTbl = CreateStates(translations, index, eventSeq);
    return eventTbl;
}


void TranslateEvent(w, event)
  Widget w;
  register XEvent *event;
{
    static unsigned long upTime=0;
    static Boolean buttonUp = FALSE;
    static StatePtr curState = NULL;
    StatePtr oldState;
    EventSeqRec curEvent;
    int index;
    ActionPtr actions;
    Boolean specialCase;

    oldState = 0;
    specialCase = FALSE;
    curEvent.eventCode = 0;
    curEvent.modifiersMask = 0;
    curEvent.eventType = event->type;
    switch (event->type) {
	case KeyPress:
	case KeyRelease:
	    buttonUp = FALSE;
	    curEvent.eventCode = XLookupKeysym(&event->xkey, 0);
	    curEvent.modifiersMask = event->xkey.state; 
	    break;
	case ButtonPress:
	    if (buttonUp && curState != NULL) 
		if ((unsigned long)
	            (upTime - event->xbutton.time) >
		    w->core.translations->clickTime)
		    curState = NULL;
	    buttonUp = FALSE;
 	    curEvent.eventCode = event->xbutton.button;
	    curEvent.modifiersMask = event->xbutton.state;
	    break;
	case ButtonRelease:
	    buttonUp = TRUE;
	    upTime = event->xbutton.time;
	    curEvent.eventCode = event->xbutton.button;
	    curEvent.modifiersMask = event->xbutton.state & 0x00FF; /* ||| gross hack */

	    break;
	case MotionNotify:
	    buttonUp = FALSE;
	    curEvent.modifiersMask = event->xmotion.state;
	    break;
	case EnterNotify:
	case LeaveNotify:
	    buttonUp = FALSE;
	    curEvent.modifiersMask = event->xcrossing.state;
	    break;
	default:
	    buttonUp = FALSE;
	    break;
    }
    if (curState != NULL) {	/* check the current level */
	index = EventIndex(w->core.translations, &curEvent);
	oldState = curState;
	while (curState != NULL && curState->index != index)
	    curState = curState->next;
	if (curState == NULL)
	    if (SpecialCase(&curEvent)) {
	        curState = oldState;
	        specialCase = TRUE;
	    } else {  /* nothing at level but you performed an action on the
			 last event---> start over with this new event. */
		if (oldState->actions != NULL) {
		    curState = oldState;
		    index = EventIndex(w->core.translations, &curEvent);
        	    curState = w->core.translations->eventObjTbl[index].state;
		}
           }
    } else {
	index = EventIndex(w->core.translations, &curEvent);
        if (index == -1) return;
        curState = w->core.translations->eventObjTbl[index].state;
    }
    if (curState != NULL && !specialCase) {
	actions = curState->actions;
	while (actions != NULL)  { /* perform any actions */
	    if (actions->proc != NULL)
			/*!!!!! should have params here */
		(*(actions->proc))(w, event);
	    actions = actions->next;
        }
        curState = curState->nextLevel;
    }
}

void TranslateTableFree(translations)
    Translations translations;
{
    EventObjPtr tbl = translations->eventObjTbl;
    int i;

    /* !!! ref count this, it may be shared */

    for (i=0; i<translations->numEvents; i++) {
	if (tbl[i].state != NULL) FreeStateRecs(tbl[i].state);
    }
    XtFree((char *)tbl);
    XtFree((char *)translations);
}


int EventIndex(translations, eventSeq) 
  Translations translations;
  EventSeqPtr eventSeq;
{
    EventObjPtr eventTbl = translations->eventObjTbl;
    int i;

    for (i=0; i < translations->numEvents; i++) {
        if ((eventTbl[i].eventType == eventSeq->eventType) &&
            (eventTbl[i].eventCode == eventSeq->eventCode) &&
	    (eventTbl[i].modifiersMask == eventSeq->modifiersMask)) 
		return(i);
   }
    return(-1);
}


void TranslateTablePrint(translations)
    Translations translations;
{
    EventObjPtr tbl = translations->eventObjTbl;
    int i, j;
    unsigned long ev[100], code[100], modif[100];
    StatePtr stack[100], state;
	
    for (i=0; i<translations->numEvents; i++) {
	j=0;
	if (tbl[i].state != NULL) {
	    state = tbl[i].state;
	    stack[j] = tbl[i].state;
	    ev[j] = tbl[i].eventType;
	    code[j] = tbl[i].eventCode;
	    modif[j++] = tbl[i].modifiersMask;	
            if (state->actions != NULL)
	        PrintActions(ev, code, modif, state->actions, j);
	    do {
	        do {
	            while (state->nextLevel != NULL) {
	                state = state->nextLevel;
	                stack[j] = state;
	                ev[j] = tbl[state->index].eventType;
		        code[j] = tbl[state->index].eventCode;
	                modif[j++] = tbl[state->index].modifiersMask;
                    	if (state->actions != NULL)
	                    PrintActions(ev, code, modif, state->actions, j);
	            }
	            j--;
	            if (state->next != NULL) {
	            	state = state->next;
	            	stack[j] = state;
	            	ev[j] = tbl[state->index].eventType;
	            	code[j] = tbl[state->index].eventCode;
	            	modif[j++] = tbl[state->index].modifiersMask;
                    	if (state->actions != NULL)
	                    PrintActions(ev, code, modif, state->actions, j);
	            }
	     	} while (state->next != NULL &&  state->nextLevel != NULL);
	    	j--;
	    	state = stack[j];
	    	if (state->next != NULL) {
	            state = state->next;
		    stack[j] = state;
	            ev[j] = tbl[state->index].eventType;
	            code[j] = tbl[state->index].eventCode;
	            modif[j++] = tbl[state->index].modifiersMask;
                    if (state->actions != NULL)
	                PrintActions(ev, code, modif, state->actions, j);
	        } else
		    j--;
	    } while (j-1>0);
	}
    }
}
