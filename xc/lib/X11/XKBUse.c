/* $XConsortium: XKB.c,v 1.7 94/02/03 18:48:37 rws Exp $ */
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

#include <stdio.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xlibint.h"
#include <X11/extensions/XKBstr.h>
#include "XKBlibint.h"

#ifdef X_NOT_STDC_ENV
extern char *getenv();
#endif

static Bool	_XkbIgnoreExtension = False;

void
XkbNoteMapChanges(old,new,wanted)
    XkbMapChangesRec	*old;
    XkbMapNotifyEvent	*new;
    unsigned int	 wanted;
{
    int first,oldLast,newLast;
    wanted&= new->changed;

    if (wanted&XkbKeyTypesMask) {
	if (old->changed&XkbKeyTypesMask) {
	    first = old->firstKeyType;
	    oldLast = old->firstKeyType+old->nKeyTypes-1;
	    newLast = new->firstKeyType+new->nKeyTypes-1;

	    if (new->firstKeyType<first)
		first = new->firstKeyType;
	    if (oldLast>newLast)
		newLast= oldLast;
	    old->firstKeyType = first;
	    old->nKeyTypes = newLast-first+1;
	}
	else {
	    old->firstKeyType= new->firstKeyType;
	    old->nKeyTypes = new->nKeyTypes;
	}
    }
    if (wanted&XkbKeySymsMask) {
	if (old->changed&XkbKeySymsMask) {
	    first = old->firstKeySym;
	    oldLast = old->firstKeySym+old->nKeySyms-1;
	    newLast = new->firstKeySym+new->nKeySyms-1;

	    if (new->firstKeySym<first)
		first = new->firstKeySym;
	    if (oldLast>newLast)
		newLast= oldLast;
	    old->firstKeySym = first;
	    old->nKeySyms = newLast-first+1;
	}
	else {
	    old->firstKeySym = new->firstKeySym;
	    old->nKeySyms = new->nKeySyms;
	}
    }
    if (wanted&XkbKeyActionsMask) {
	if (old->changed&XkbKeyActionsMask) {
	    first = old->firstKeyAction;
	    oldLast = old->firstKeyAction+old->nKeyActions-1;
	    newLast = new->firstKeyAction+new->nKeyActions-1;

	    if (new->firstKeyAction<first)
		first = new->firstKeyAction;
	    if (oldLast>newLast)
		newLast= oldLast;
	    old->firstKeyAction = first;
	    old->nKeyActions = newLast-first+1;
	}
	else {
	    old->firstKeyAction = new->firstKeyAction;
	    old->nKeyActions = new->nKeyActions;
	}
    }
    if (wanted&XkbKeyBehaviorsMask) {
	if (old->changed&XkbKeyBehaviorsMask) {
	    first = old->firstKeyBehavior;
	    oldLast = old->firstKeyBehavior+old->nKeyBehaviors-1;
	    newLast = new->firstKeyBehavior+new->nKeyBehaviors-1;

	    if (new->firstKeyBehavior<first)
		first = new->firstKeyBehavior;
	    if (oldLast>newLast)
		newLast= oldLast;
	    old->firstKeyBehavior = first;
	    old->nKeyBehaviors = newLast-first+1;
	}
	else {
	    old->firstKeyBehavior = new->firstKeyBehavior;
	    old->nKeyBehaviors = new->nKeyBehaviors;
	}
    }
    old->changed|= wanted;
    return;
}

void
XkbNoteCoreMapChanges(old,new,wanted)
    XkbMapChangesRec	*old;
    XMappingEvent	*new;
    unsigned int	 wanted;
{
    int first,oldLast,newLast;


    if ((new->request==MappingKeyboard)&&(wanted&XkbKeySymsMask)) {
	if (old->changed&XkbKeySymsMask) {
	    first = old->firstKeySym;
	    oldLast = old->firstKeySym+old->nKeySyms-1;
	    newLast = new->first_keycode+new->count-1;

	    if (new->first_keycode<first)
		first = new->first_keycode;
	    if (oldLast>newLast)
		newLast= oldLast;
	    old->firstKeySym = first;
	    old->nKeySyms = newLast-first+1;
	}
	else {
	    old->changed|= XkbKeySymsMask;
	    old->firstKeySym = new->first_keycode;
	    old->nKeySyms = new->count;
	}
    }
    return;
}

static Bool
wire_to_event(dpy,re,event)
    Display *dpy;
    XEvent *re;
    xEvent *event;
{
    xkbEvent *xkbevent= (xkbEvent *)event;
    XkbInfoPtr xkbi;

    if ((dpy->flags & XlibDisplayNoXkb) ||
	(!dpy->xkb_info && !XkbUseExtension(dpy)))
	return False;
    xkbi = dpy->xkb_info;
    if (((event->u.u.type&0x7f)-xkbi->codes->first_event)!=XkbEventCode)
	return False;

    switch (xkbevent->u.any.xkbType&0x7f) {
	case XkbStateNotify:
	    {
		xkbStateNotify *sn = (xkbStateNotify *)event;
		if ( xkbi->selected_events&XkbStateNotifyMask ) {
		    XkbStateNotifyEvent *sev=(XkbStateNotifyEvent *)re;
		    sev->type = XkbEventCode+xkbi->codes->first_event;
		    sev->xkbType = XkbStateNotify;
		    sev->serial = _XSetLastRequestRead(dpy,
					(xGenericReply *)event);
		    sev->send_event = ((event->u.u.type & 0x80) != 0);
		    sev->display = dpy;
		    sev->time = sn->time; 
		    sev->device = sn->deviceID;
		    sev->keycode = sn->keycode;
		    sev->eventType = sn->eventType;
		    sev->requestMajor = sn->requestMajor;
		    sev->requestMinor = sn->requestMinor;
		    sev->changed = sn->changed;
		    sev->group = sn->group;
		    sev->baseGroup = sn->baseGroup;
		    sev->latchedGroup = sn->latchedGroup;
		    sev->lockedGroup = sn->lockedGroup;
		    sev->mods = sn->mods;
		    sev->baseMods = sn->baseMods;
		    sev->latchedMods = sn->latchedMods;
		    sev->lockedMods = sn->lockedMods;
		    sev->compatState = sn->compatState;
		    sev->unlockedMods = sn->unlockedMods;
		    sev->groupsUnlocked = sn->groupsUnlocked;
		    return True;
		}
	    }
	    break;
	case XkbMapNotify:
	    {
		xkbMapNotify *mn = (xkbMapNotify *)event;
		if (xkbi->selected_events&XkbMapNotifyMask) {
		    XkbMapNotifyEvent *mev;
		    mev =(XkbMapNotifyEvent *)re;
		    mev->type = XkbEventCode+xkbi->codes->first_event;
		    mev->xkbType = XkbMapNotify;
		    mev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    mev->send_event = ((event->u.u.type&0x80)!=0);
		    mev->display = dpy;
		    mev->time = mn->time;
		    mev->device = mn->deviceID;
		    mev->changed =  mn->changed;
		    mev->firstKeyType = mn->firstKeyType;
		    mev->nKeyTypes = mn->nKeyTypes;
		    mev->firstKeySym = mn->firstKeySym;
		    mev->nKeySyms = mn->nKeySyms;
		    mev->firstKeyAction = mn->firstKeyAction;
		    mev->nKeyActions = mn->nKeyActions;
		    mev->firstKeyBehavior = mn->firstKeyBehavior;
		    mev->nKeyBehaviors = mn->nKeyBehaviors;
		    XkbNoteMapChanges(&xkbi->changes,mev,XKB_XLIB_MAP_MASK);
		    if (xkbi->changes.changed)
			xkbi->flags|= XkbMapPending;
		    return True;
		}
		else if (mn->nKeySyms>0) {
		    register XMappingEvent *ev = (XMappingEvent *)re;
		    ev->type = MappingNotify;
		    ev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    ev->send_event = ((event->u.u.type&0x80)!=0);
		    ev->display = dpy;
		    ev->window = 0;
		    ev->first_keycode = mn->firstKeySym;
		    ev->request = MappingKeyboard;
		    ev->count = mn->nKeySyms;
		    XkbNoteCoreMapChanges(&xkbi->changes,ev,XKB_XLIB_MAP_MASK);
		    if (xkbi->changes.changed)
			xkbi->flags|= XkbMapPending;
		    return True;
		}
	    }
	    break;
	case XkbControlsNotify:
	    {
		if (xkbi->selected_events&XkbControlsNotifyMask) {
		    xkbControlsNotify *cn =(xkbControlsNotify *)event;
		    XkbControlsNotifyEvent *cev;
		    cev =(XkbControlsNotifyEvent *)re;
		    cev->type = XkbEventCode+xkbi->codes->first_event;
		    cev->xkbType = XkbControlsNotify;
		    cev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    cev->send_event = ((event->u.u.type&0x80)!=0);
		    cev->display = dpy;
		    cev->time = cn->time;
		    cev->device = cn->deviceID;
		    cev->changedControls =  cn->changedControls;
		    cev->enabledControlChanges = cn->enabledControlChanges;
		    cev->enabledControls =  cn->enabledControls;
		    cev->keycode = cn->keycode;
		    cev->eventType = cn->eventType;
		    cev->requestMajor = cn->requestMajor;
		    cev->requestMinor = cn->requestMinor;
		    return True;
		}
		return False;
	    }
	    break;
	case XkbIndicatorNotify:
	    {
		if (xkbi->selected_events&XkbIndicatorNotifyMask) {
		    xkbIndicatorNotify *in =(xkbIndicatorNotify *)event;
		    XkbIndicatorNotifyEvent *iev;
		    iev =(XkbIndicatorNotifyEvent *)re;
		    iev->type = XkbEventCode+xkbi->codes->first_event;
		    iev->xkbType = XkbIndicatorNotify;
		    iev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    iev->send_event = ((event->u.u.type&0x80)!=0);
		    iev->display = dpy;
		    iev->time = in->time;
		    iev->device = in->deviceID;
		    iev->stateChanged =  in->stateChanged;
		    iev->state=  in->state;
		    iev->mapChanged =  in->mapChanged;
		    return True;
		}
		return False;
	    }
	    break;
	case XkbBellNotify:
	    {
		if (xkbi->selected_events&XkbBellNotifyMask) {
		    xkbBellNotify *bn =(xkbBellNotify *)event;
		    XkbBellNotifyEvent *bev;
		    bev =(XkbBellNotifyEvent *)re;
		    bev->type = XkbEventCode+xkbi->codes->first_event;
		    bev->xkbType = XkbBellNotify;
		    bev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    bev->send_event = ((event->u.u.type&0x80)!=0);
		    bev->display = dpy;
		    bev->time = bn->time;
		    bev->device = bn->deviceID;
		    bev->percent = bn->percent;
		    bev->pitch = bn->percent;
		    bev->duration = bn->duration;
		    bev->bellClass = bn->bellClass;
		    bev->bellID = bn->bellID;
		    bev->name = bn->name;
		    return True;
		}
		return False;
	    }
	    break;
	case XkbSlowKeyNotify:
	    {
		if (xkbi->selected_events&XkbSlowKeyNotifyMask) {
		    xkbSlowKeyNotify *skn =(xkbSlowKeyNotify *)event;
		    XkbSlowKeyNotifyEvent *skev;
		    skev =(XkbSlowKeyNotifyEvent *)re;
		    skev->type = XkbEventCode+xkbi->codes->first_event;
		    skev->xkbType = XkbSlowKeyNotify;
		    skev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    skev->send_event = ((event->u.u.type&0x80)!=0);
		    skev->display = dpy;
		    skev->time = skn->time;
		    skev->device = skn->deviceID;
		    skev->slowKeyType = skn->slowKeyType;
		    skev->keycode = skn->keycode;
		    skev->delay = skn->delay;
		    return True;
		}
		return False;
	    }
	    break;
	case XkbNamesNotify:
	    {
		if (xkbi->selected_events&XkbNamesNotifyMask) {
		    xkbNamesNotify *nn =(xkbNamesNotify *)event;
		    XkbNamesNotifyEvent *nev;
		    nev =(XkbNamesNotifyEvent *)re;
		    nev->type = XkbEventCode+xkbi->codes->first_event;
		    nev->xkbType = XkbNamesNotify;
		    nev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    nev->send_event = ((event->u.u.type&0x80)!=0);
		    nev->display = dpy;
		    nev->time = nn->time;
		    nev->device = nn->deviceID;
		    nev->changed = nn->changed;
		    nev->firstKeyType = nn->firstKeyType;
		    nev->nKeyTypes = nn->nKeyTypes;
		    nev->firstLevel = nn->firstLevelName;
		    nev->nLevels = nn->nLevelNames;
		    nev->nRadioGroups = nn->nRadioGroups;
		    nev->nCharSets = nn->nCharSets;
		    nev->changedMods = nn->changedMods;
		    nev->changedIndicators = nn->changedIndicators;
		    return True;
		}
		return False;
	    }
	    break;
	case XkbCompatMapNotify:
	    {
		if (xkbi->selected_events&XkbCompatMapNotifyMask) {
		    xkbCompatMapNotify *cmn =(xkbCompatMapNotify *)event;
		    XkbCompatMapNotifyEvent *cmev;
		    cmev =(XkbCompatMapNotifyEvent *)re;
		    cmev->type = XkbEventCode+xkbi->codes->first_event;
		    cmev->xkbType = XkbCompatMapNotify;
		    cmev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    cmev->send_event = ((event->u.u.type&0x80)!=0);
		    cmev->display = dpy;
		    cmev->time = cmn->time;
		    cmev->device = cmn->deviceID;
		    cmev->changedMods = cmn->changedMods;
		    cmev->firstSym = cmn->firstSym;
		    cmev->nSyms = cmn->nSyms;
		    cmev->nTotalSyms = cmn->nTotalSyms;
		    return True;
		}
		return False;
	    }
	    break;
	case XkbAlternateSymsNotify:
	    {
		if (xkbi->selected_events&XkbAlternateSymsNotifyMask) {
		    xkbAlternateSymsNotify *asn=(xkbAlternateSymsNotify *)event;
		    XkbAlternateSymsNotifyEvent *asev;
		    asev =(XkbAlternateSymsNotifyEvent *)re;
		    asev->type = XkbEventCode+xkbi->codes->first_event;
		    asev->xkbType = XkbAlternateSymsNotify;
		    asev->serial = _XSetLastRequestRead(dpy,
						(xGenericReply *)event);
		    asev->send_event = ((event->u.u.type&0x80)!=0);
		    asev->display = dpy;
		    asev->time = asn->time;
		    asev->device = asn->deviceID;
		    asev->altSymsID = asn->altSymsID;
		    asev->firstKey = asn->firstKey;
		    asev->nKeys = asn->nKeys;
		    return True;
		}
		return False;
	    }
	    break;
	default:
	    fprintf(stderr,"Got unknown Xkb event (%d, base=%d)\n",re->type,
						xkbi->codes->first_event);
	    break;
    }
    return False;
}

Bool
XkbIgnoreExtension(ignore)
    Bool ignore;
{
    _XkbIgnoreExtension = ignore;
    return True;
}

static void
_XkbFreeInfo(dpy)
    Display *dpy;
{
    XkbInfoPtr xkbi = dpy->xkb_info;
    if (xkbi) {
	if (xkbi->desc)
	    Xfree(xkbi->desc);
	if (xkbi->modmap)
	    Xfree(xkbi->modmap);
	Xfree(xkbi);
      }
}

Bool
XkbUseExtension(dpy)
    Display *dpy;
{
    xkbUseExtensionReply rep;
    register xkbUseExtensionReq *req;
    XExtCodes		*codes;
    int	ev_base;
    XkbInfoPtr xkbi;
#ifdef DEBUG
    static int debugMsg;
#endif
    static int been_here= 0;

    if ( dpy->xkb_info )
	return True;
    if (!been_here) {
#ifdef DEBUG
	debugMsg= (getenv("XKB_DEBUG")!=NULL);
#endif
	been_here= 1;
    }
    if ((dpy->flags&XlibDisplayNoXkb) || dpy->keysyms ||
	_XkbIgnoreExtension || getenv("XKB_DISABLE")) {
	LockDisplay(dpy);
	dpy->flags |= XlibDisplayNoXkb;
	UnlockDisplay(dpy);
#ifdef DEBUG
	if (debugMsg)
	    fprintf(stderr,"XKB extension disabled or missing\n");
#endif
	return False;
    }

    xkbi = (XkbInfoPtr)Xcalloc(1, sizeof(XkbInfoRec));
    if ( !xkbi )
	return False;

    if ( (codes=XInitExtension(dpy,XkbName))==NULL ) {
	LockDisplay(dpy);
	dpy->flags |= XlibDisplayNoXkb;
	UnlockDisplay(dpy);
	Xfree(xkbi);
#ifdef DEBUG
	if (debugMsg)
	    fprintf(stderr,"XKB extension not present\n");
#endif
	return False;
    }
    xkbi->codes = codes;
    LockDisplay(dpy);

    GetReq(kbUseExtension, req);
    req->reqType = xkbi->codes->major_opcode;
    req->xkbReqType = X_kbUseExtension;
    req->wantedMajor = XkbMajorVersion;
    req->wantedMinor = XkbMinorVersion;
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse) || !rep.supported) {
#ifdef DEBUG
	if (debugMsg)
	    fprintf(stderr,
		"XKB version mismatch (want %d.%02d, got %d.%02d)\n", 
		XkbMajorVersion,XkbMinorVersion,
		rep.serverMajor, rep.serverMinor);
#endif
	/* could theoretically try for an older version here */
	dpy->flags |= XlibDisplayNoXkb;
	UnlockDisplay(dpy);
	SyncHandle();
	Xfree(xkbi);
	return False;
    }
    xkbi->srv_major= rep.serverMajor;
    xkbi->srv_minor= rep.serverMinor;
#ifdef DEBUG
    if (debugMsg)
	fprintf(stderr,"XKB (version %d.%02d/%d.%02d) OK!\n",
				XkbMajorVersion,XkbMinorVersion,
				rep.serverMajor,rep.serverMinor);
#endif
    dpy->xkb_info = xkbi;
    dpy->free_funcs->xkb = _XkbFreeInfo;
    ev_base = codes->first_event;
    XESetWireToEvent(dpy,ev_base+XkbEventCode,wire_to_event);
    UnlockDisplay(dpy);
    SyncHandle();
    return True;
}
