/* $XConsortium: XKBBind.c,v 1.3 93/09/28 19:48:37 rws Exp $ */
/* Copyright 1985, 1987, Massachusetts Institute of Technology */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/* Beware, here be monsters (still under construction... - JG */

#define NEED_EVENTS
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>

#include <X11/extensions/XKBproto.h>
#include <X11/extensions/XKBstr.h>
#include "XKBlibint.h"

#define AllMods (ShiftMask|LockMask|ControlMask| \
		 Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)

#define Is_KP_Keysym(k) ((((k) >= XK_KP_0) && ((k) <= XK_KP_9)) || \
			 ((k) == XK_KP_Decimal) || ((k)==XK_KP_Separator))

static _XkbComputeMaskFromKeytrans();
static int _XkbLoadDpy();

struct _XKeytrans {
	struct _XKeytrans *next;/* next on list */
	char *string;		/* string to return when the time comes */
	int len;		/* length of string (since NULL is legit)*/
	KeySym key;		/* keysym rebound */
	unsigned int state;	/* modifier state */
	KeySym *modifiers;	/* modifier keysyms you want */
	int mlen;		/* length of modifier list */
};

#define	XKB_UNAVAILABLE(d) \
    (((d)->flags&XlibDisplayNoXkb) ||\
     ((!(d)->xkb_info || (!(d)->xkb_info->desc)) && !_XkbLoadDpy(d)))

#define	CHECK_PENDING_REFRESH(d) {\
    if ((d)->xkb_info->flags&XkbMapPending) {\
	if (XkbRefreshMap(dpy,(d)->xkb_info->desc, &dpy->xkb_info->changes)) {\
	    LockDisplay(dpy);\
	    dpy->xkb_info->changes.changed= 0;\
	    UnlockDisplay(dpy);\
	}\
    }\
}

#if NeedFunctionPrototypes
KeySym
XKeycodeToKeysym(Display *dpy,
#if NeedWidePrototypes
		 unsigned int kc,
#else
		 KeyCode kc,
#endif
		 int col)
#else
KeySym
XKeycodeToKeysym(dpy, kc, col)
    Display *dpy;
    KeyCode kc;
    int col;
#endif
{
    int	offset;
    XkbDescRec	*xkb;
    
    if (XKB_UNAVAILABLE(dpy))
	return _XKeycodeToKeysym(dpy,kc,col);

    CHECK_PENDING_REFRESH(dpy);

    xkb = dpy->xkb_info->desc;
    if ((kc<xkb->minKeyCode)||(kc>xkb->maxKeyCode))
	return NoSymbol;

    if (( col<0 ) || (col>=(int)XkbKeyNumSyms(xkb,kc)))
	return NoSymbol;
    
    return XkbKeySym(xkb,kc,col);
}

KeyCode
XKeysymToKeycode(dpy, ks)
    Display *dpy;
    KeySym ks;
{
    register int i, j, gotOne;

    if (XKB_UNAVAILABLE(dpy))
	return _XKeysymToKeycode(dpy,ks);
    CHECK_PENDING_REFRESH(dpy);

    j= 0;
    do {
	register XkbDescRec *xkb = dpy->xkb_info->desc;
	gotOne= 0;
	for (i = dpy->min_keycode; i <= dpy->max_keycode; i++) {
	    if ( j<(int)XkbKeyNumSyms(xkb,i) ) {
		gotOne = 1;
		if ((XkbKeySym(xkb,i,j)==ks))
		    return i;
	    }
	}
	j++;
    } while (gotOne);
    return 0;
}

static int
_XkbComputeModmap(dpy)
    Display *dpy;
{
XModifierKeymap	*modmap;

    if (!dpy->xkb_info)		return 0;
    if (dpy->xkb_info->modmap)	return 1;
    dpy->xkb_info->modmap = (char *)Xmalloc(dpy->xkb_info->desc->maxKeyCode+1);
    if (!dpy->xkb_info->modmap)	return 0;
    bzero(dpy->xkb_info->modmap,dpy->xkb_info->desc->maxKeyCode+1);
    modmap = XGetModifierMapping(dpy);
    if (modmap) {
	register int m,k;
	KeyCode *map;
	map= modmap->modifiermap;
	for (m=0;m<8;m++) {
	    for (k=0;k<modmap->max_keypermod;k++,map++) {
		if (((*map)>=dpy->xkb_info->desc->minKeyCode)&&
		    ((*map)<=dpy->xkb_info->desc->maxKeyCode)) {
		    dpy->xkb_info->modmap[*map]|= (1<<m);
		}
	    }
	}
	XFreeModifiermap(modmap);
	return 1;
    }
    return 0;
}

unsigned
XkbKeysymToModifiers(dpy,ks)
    Display *dpy;
    KeySym ks;
{
    XkbDescRec *xkb;
    register int i,j;
    register KeySym *pSyms;
    CARD8 mods;

    if (XKB_UNAVAILABLE(dpy))
	return _XKeysymToModifiers(dpy,ks);
    CHECK_PENDING_REFRESH(dpy);

    if ((!dpy->xkb_info->modmap)&&(!_XkbComputeModmap(dpy)))
	return _XKeysymToModifiers(dpy,ks);

    xkb= dpy->xkb_info->desc;
    mods= 0;
    for (i = xkb->minKeyCode; i <= (int)xkb->maxKeyCode; i++) {
	pSyms= XkbKeySymsPtr(xkb,i);
	for (j=XkbKeyNumSyms(xkb,i)-1;j>=0;j--) {
	    if (pSyms[j]==ks) {
		mods|= dpy->xkb_info->modmap[i];
		break;
	    }
	}
    }
    return mods;
}

KeySym
XLookupKeysym(event, col)
    register XKeyEvent *event;
    int col;
{
    Display *dpy = event->display;
    if (XKB_UNAVAILABLE(dpy))
	return _XLookupKeysym(event, col);
    CHECK_PENDING_REFRESH(dpy);
    return XKeycodeToKeysym(dpy, event->keycode, col);
}

#define	XkbModifiersGroup(m)	(((m)>>13)&0x7)

int
XkbTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return)
    register Display *dpy;
    KeyCode keycode;
    register unsigned int modifiers;
    unsigned int *modifiers_return;
    KeySym *keysym_return;
{
    register XkbInfoPtr xkb;
    XkbKeyTypeRec *keyType;
    int col;
    KeySym *syms;

    if (XKB_UNAVAILABLE(dpy))
	return _XTranslateKey(dpy,keycode,modifiers,
						modifiers_return,keysym_return);
    CHECK_PENDING_REFRESH(dpy);

    if (modifiers_return!=NULL)
	*modifiers_return = modifiers;
    if (((int)keycode < dpy->min_keycode) || ((int)keycode > dpy->max_keycode))
    {
	if (keysym_return!=NULL)
	    *keysym_return = NoSymbol;
	return 0;
    }

    xkb= dpy->xkb_info;

    /* find the row */
    syms = XkbKeySymsPtr(xkb->desc,keycode);

    /* find the offset of the effective group */
    col = 0;
    keyType = XkbKeyKeyType(xkb->desc,keycode);
    if ( XkbModifiersGroup(modifiers)!=0 ) {
	unsigned effectiveGroup = XkbModifiersGroup(modifiers);
	if ( effectiveGroup >= XkbKeyNumGroups(xkb->desc,keycode) ) {
	    if ( XkbKeyGroupsWrap(xkb->desc,keycode) )
		 effectiveGroup %= XkbKeyNumGroups(xkb->desc,keycode);
	    else effectiveGroup = XkbKeyNumGroups(xkb->desc,keycode)-1;
	}
	col= effectiveGroup*keyType->groupWidth;
    }

    /* find the column within the group */
    col+= keyType->map[modifiers&keyType->mask];

    if (keysym_return!=NULL)
	*keysym_return= syms[col];
    if (modifiers_return)
	*modifiers_return&= ~keyType->mask;
    return (syms[col]!=NoSymbol);
}

XRefreshKeyboardMapping(event)
    register XMappingEvent *event;
{
    XkbAnyEvent	*xkbevent = (XkbAnyEvent *)event;
    Display *dpy = event->display;
    XkbMapChangesRec changes;
    XkbInfoPtr xkbi;
    int	modsChanged;

    if (XKB_UNAVAILABLE(dpy))
	return _XRefreshKeyboardMapping(event);

    xkbi = dpy->xkb_info;

    if ((xkbi->flags&XkbMapPending)||(event->request==MappingKeyboard)) {
	if (xkbi->flags&XkbMapPending) {
	    changes= xkbi->changes;
	    XkbNoteCoreMapChanges(&changes,event,XKB_XLIB_MAP_MASK);
	}
	else {
	    bzero(&changes,sizeof(changes));
	    changes.changed= XkbKeySymsMask;
	    if (xkbi->desc->minKeyCode<xkbi->desc->maxKeyCode) {
		changes.firstKeySym= xkbi->desc->minKeyCode;
		changes.nKeySyms= xkbi->desc->maxKeyCode-
						xkbi->desc->minKeyCode+1;
	    }
	    else {
		changes.firstKeySym= event->first_keycode;
		changes.nKeySyms= event->count;
	    }
	}

	if (!XkbRefreshMap(dpy,xkbi->desc, &changes)){
		fprintf(stderr,"XkbRefreshMap failed\n");
	}
	LockDisplay(dpy);
	if (xkbi->flags&XkbMapPending) {
	    xkbi->flags&= ~XkbMapPending;
	    bzero(&xkbi->changes,sizeof(XkbMapChangesRec));
	}
	UnlockDisplay(dpy);
    }
    if (event->request==MappingModifier) {
	if (xkbi->modmap) {
	    Xfree(xkbi->modmap);
	    xkbi->modmap= NULL;
	}
	if (dpy->key_bindings) {
	    register struct _XKeytrans *p;
	    for (p = dpy->key_bindings; p; p = p->next) {
		register int i;
		p->state = AnyModifier;
		for (i = 0; i < p->mlen; i++) {
		    p->state|= XkbKeysymToModifiers(dpy,p->modifiers[i]);
		}
		p->state &= AllMods;
	    }
	}
	UnlockDisplay(dpy);
    }
    return;
}

static int
_XkbLoadDpy(dpy)
    Display *dpy;
{
    XkbInfoPtr xkbi;
    unsigned query,oldEvents;
    XkbDescRec *desc;

    if (!XkbUseExtension(dpy))
	return 0;

    xkbi = dpy->xkb_info;
    query = XkbFullClientInfoMask;
    desc = XkbGetMap(dpy,query,XkbUseCoreKbd);
    LockDisplay(dpy);
    xkbi->desc = desc;

    xkbi->charset = _XkbGetCharset(NULL);
    if ( xkbi->charset ) {
	_XkbGetConverters(xkbi->charset,&xkbi->cvt);
    }
    UnlockDisplay(dpy);
    oldEvents= xkbi->selected_events;
    XkbSelectEventDetails(dpy,xkbi->desc->deviceSpec,XkbMapNotify,
				XkbAllMapComponentsMask,XkbFullClientInfoMask);
    LockDisplay(dpy);
    xkbi->selected_events= oldEvents;
    UnlockDisplay(dpy);
}

int
XkbTranslateKeySym(dpy, sym_return, modifiers, buffer, nbytes)
    Display *dpy;
    register KeySym *sym_return;
    unsigned int modifiers;
    char *buffer;
    int nbytes;
{
    char tmp[4];
    register XkbInfoPtr xkb;
    register struct _XKeytrans *p; 
    int n;

    if (XKB_UNAVAILABLE(dpy))
	return _XTranslateKeySym(dpy,*sym_return,modifiers,buffer,nbytes);
    CHECK_PENDING_REFRESH(dpy);

    if ((buffer==NULL)||(nbytes==0)) {
	buffer= tmp;
	nbytes= 4;
    }

    xkb = dpy->xkb_info;
    /* see if symbol rebound, if so, return that string. */
    for (p = dpy->key_bindings; p; p = p->next) {
	if (((modifiers & AllMods) == p->state) && (*sym_return == p->key)) {
	    int tmp = p->len;
	    if (tmp > nbytes) tmp = nbytes;
	    memcpy (buffer, p->string, tmp);
	    return tmp;
	}
    }

    if ( nbytes>0 )
	buffer[0]= '\0';

    if ( xkb->cvt.KSToUpper && (modifiers&LockMask) ) {
	*sym_return = (*xkb->cvt.KSToUpper)(*sym_return);
    }
    n = (*xkb->cvt.KSToMB)(xkb->cvt.KSToMBPriv,*sym_return,buffer,nbytes,NULL);

    if ((!xkb->cvt.KSToUpper)&&( modifiers&LockMask )) {
	register int i;

	if (!xkb->cvt.KSToUpper) {
	    int change;
	    char ch;
	    for (i=change=0;i<n;i++) {
		ch= toupper(buffer[i]);
		change= (change||(buffer[i]!=ch));
		buffer[i] = ch;
	    }
	    if (change) {
		if (n==1)
		     *sym_return=(*xkb->cvt.MBToKS)(xkb->cvt.MBToKSPriv,
								buffer,n,0);
		else *sym_return= NoSymbol;
	    }
	}
    }

    if ( modifiers&ControlMask ) {
	if ( n==1 ) {
	    register char c = buffer[0];

	    if ((c >= '@' && c < '\177') || c == ' ') c &= 0x1F;
	    else if (c == '2') c = '\000';
	    else if (c >= '3' && c <= '7') c -= ('3' - '\033');
	    else if (c == '8') c = '\177';
	    else if (c == '/') c = '_' & 0x1F;

	    buffer[0]= c;
	    if ( nbytes>1 )
		buffer[1]= '\0';
	    return 1;
	}
	if ( nbytes > 0 )
	    buffer[0]= '\0';
	return 0;
    }
    return n;
}

int
XLookupString (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;	/* buffer */
    int nbytes;	/* space in buffer for characters */
    KeySym *keysym;
    XComposeStatus *status;
{
    KeySym	dummy;
    int rtrnLen;
    unsigned int *new_mods;
    Display *dpy = event->display;

    if (XKB_UNAVAILABLE(dpy))
	return _XLookupString(event, buffer, nbytes, keysym, status);
    CHECK_PENDING_REFRESH(dpy);

    if (keysym==NULL)
	keysym= &dummy;

    if (!XkbTranslateKey(dpy,event->keycode,event->state, &new_mods,keysym))
	return 0;

    rtrnLen = XkbTranslateKeySym(dpy,keysym,new_mods,buffer,nbytes);
    return rtrnLen;
}


int
XkbLookupKeyBinding(dpy, sym_return, modifiers, buffer, nbytes)
    Display *dpy;
    register KeySym *sym_return;
    unsigned int modifiers;
    char *buffer;
    int nbytes;
{
    char tmp[4];
    register struct _XKeytrans *p; 
    int n;

    for (p = dpy->key_bindings; p; p = p->next) {
	if (((modifiers & AllMods) == p->state) && (*sym_return == p->key)) {
	    int tmp = p->len;
	    if (tmp > nbytes) tmp = nbytes;
	    memcpy (buffer, p->string, tmp);
	    if (tmp < nbytes)	buffer[tmp]= '\0';
	    return tmp;
	}
    }
    return 0;
}

char
XkbToControl( c )
    char c;
{
    if ((c >= '@' && c < '\177') || c == ' ') c &= 0x1F;
    else if (c == '2') c = '\000';
    else if (c >= '3' && c <= '7') c -= ('3' - '\033');
    else if (c == '8') c = '\177';
    else if (c == '/') c = '_' & 0x1F;
    return c;
}
