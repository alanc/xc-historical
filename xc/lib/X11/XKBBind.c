/* $XConsortium: XKeyBind.c,v 11.68 92/12/14 11:01:27 rws Exp $ */
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

#include <locale.h>

#include <X11/extensions/XKBproto.h>
#include <X11/extensions/XKBstr.h>
#include "XKBlibint.h"

#ifdef sgi
#define	SGI_NO_CONST_MEMBERS
#include "SGICompose.h"
#endif

#define AllMods (ShiftMask|LockMask|ControlMask| \
		 Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)

#define Is_KP_Keysym(k) ((((k) >= XK_KP_0) && ((k) <= XK_KP_9)) || \
			 ((k) == XK_KP_Decimal) || ((k)==XK_KP_Separator))

static _XkbComputeMaskFromKeytrans();
extern int _XkbInitializeDpy();

struct _XKeytrans {
	struct _XKeytrans *next;/* next on list */
	char *string;		/* string to return when the time comes */
	int len;		/* length of string (since NULL is legit)*/
	KeySym key;		/* keysym rebound */
	unsigned int state;	/* modifier state */
	KeySym *modifiers;	/* modifier keysyms you want */
	int mlen;		/* length of modifier list */
};

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
    
    if ((!dpy->xkbInfo) && (!_XkbInitializeDpy(dpy,True)))
	return _NonXkbKeycodeToKeysym(dpy,kc,col);
    xkb = dpy->xkbInfo->desc;
    if ((kc<xkb->minKeyCode)||(kc>xkb->maxKeyCode))
	return NoSymbol;

    if (( col<0 ) || (col>=XkbKeyNumSyms(xkb,kc)))
	return NoSymbol;
    
    return XkbKeySym(xkb,kc,col);
}

KeyCode
XKeysymToKeycode(dpy, ks)
    Display *dpy;
    KeySym ks;
{
    register int i, j, gotOne;

    if ((! dpy->xkbInfo) && (! _XkbInitializeDpy(dpy,True)))
	return _NonXkbKeysymToKeycode(dpy,ks);

    j= 0;
    do {
	register XkbDescRec *xkb = dpy->xkbInfo->desc;
	gotOne= 0;
	for (i = dpy->min_keycode; i <= dpy->max_keycode; i++) {
	    if ( j<XkbKeyNumSyms(xkb,i) ) {
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

    if (!dpy->xkbInfo)		return 0;
    if (dpy->xkbInfo->modmap)	return 1;
    dpy->xkbInfo->modmap = (CARD8 *)Xmalloc(dpy->xkbInfo->desc->maxKeyCode+1);
    if (!dpy->xkbInfo->modmap)	return 0;
    bzero(dpy->xkbInfo->modmap,dpy->xkbInfo->desc->maxKeyCode+1);
    modmap = XGetModifierMapping(dpy);
    if (modmap) {
	register int m,k;
	KeyCode *map;
	map= modmap->modifiermap;
	for (m=0;m<8;m++) {
	    for (k=0;k<modmap->max_keypermod;k++,map++) {
		if (((*map)>=dpy->xkbInfo->desc->minKeyCode)&&
		    ((*map)<=dpy->xkbInfo->desc->maxKeyCode)) {
		    dpy->xkbInfo->modmap[*map]|= (1<<m);
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

    if ((! dpy->xkbInfo) && (! _XkbInitializeDpy(dpy,True)))
	return _NonXkbKeysymToModifiers(dpy,ks);

    if ((!dpy->xkbInfo->modmap)&&(!_XkbComputeModmap(dpy)))
	return _NonXkbKeysymToModifiers(dpy,ks);

    xkb= dpy->xkbInfo->desc;
    mods= 0;
    for (i = xkb->minKeyCode; i <= xkb->maxKeyCode; i++) {
	pSyms= XkbKeySymsPtr(xkb,i);
	for (j=XkbKeyNumSyms(xkb,i)-1;j>=0;j--) {
	    if (pSyms[j]==ks) {
		mods|= dpy->xkbInfo->modmap[i];
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
    if ((!event->display->xkbInfo)&&(!_XkbInitializeDpy(event->display,True)))
	return _NonXkbLookupKeysym(event, col);
    return XKeycodeToKeysym(event->display, event->keycode, col);
}

#define	XkbModifiersGroup(m)	(((m)>>13)&0x7)

int
XTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return)
    register Display *dpy;
    KeyCode keycode;
    register unsigned int modifiers;
    unsigned int *modifiers_return;
    KeySym *keysym_return;
{
    register XkbLibInfoRec *xkb;
    XkbKeyTypeRec *keyType;
    int col;
    KeySym *syms;

    if ((!dpy->xkbInfo) && (!_XkbInitializeDpy(dpy,True)))
	return _NonXkbTranslateKey(dpy,keycode,modifiers,
						modifiers_return,keysym_return);

    if (modifiers_return!=NULL)
	*modifiers_return = modifiers;
    if (((int)keycode < dpy->min_keycode) || ((int)keycode > dpy->max_keycode))
    {
	if (keysym_return!=NULL)
	    *keysym_return = NoSymbol;
	return 0;
    }

    xkb= dpy->xkbInfo;

    /* find the row */
    syms = XkbKeySymsPtr(xkb->desc,keycode);

    /* find the offset of the effective group */
    col = 0;
    keyType = XkbKeyKeyType(xkb->desc,keycode);
    if ( XkbModifiersGroup(modifiers)!=0 ) {
	int effectiveGroup = XkbModifiersGroup(modifiers);
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
XkbAnyEvent	*xkbEvent = (XkbAnyEvent *)event;

    if ((!event->display->xkbInfo) && (!_XkbInitializeDpy(event->display,True)))
	return _NonXkbRefreshKeyboardMapping(event);

    if ((event->request == event->display->xkb_ext_event_base) &&
					(xkbEvent->xkbType==XkbMapNotify)) {
	
#ifdef NOTDEF
	if (!XkbRefreshMap(event->display,event->display->xkbInfo->desc,
					event)) {
	    fprintf(stderr,"XkbRefreshMapping failed\n");
	}
#else
	fprintf(stderr,"explicit map not implemented yet\n");
#endif
    }
    else if (event->display->xkb_flags&XKB_MAP_PENDING) {

	event->display->xkb_flags&= ~XKB_MAP_PENDING;
	if (!XkbRefreshMap(event->display,event->display->xkbInfo->desc,
					&event->display->xkbInfo->changes)){
		fprintf(stderr,"XkbRefreshMap failed\n");
	}
	LockDisplay(event->display);
	if (event->display->xkbInfo->modmap) {
	    Xfree(event->display->xkbInfo->modmap);
	    event->display->xkbInfo->modmap= NULL;
	}
	if (event->display->key_bindings) {
	    _XkbRefreshKeyBindings(event->display);
	}
	UnlockDisplay(event->display);
    }
    return;
}

int
_XkbInitializeDpy(dpy,load)
    Display *dpy;
    Bool load;
{
    extern int		_XkbIgnoreExtension;
    XkbLibInfoRec	tmp,*xkb;
    unsigned query;

    if ( dpy->keysyms || (dpy->xkb_flags&XKB_ABSENT) || _XkbIgnoreExtension) {
	dpy->xkb_flags|= XKB_ABSENT;
	return 0;
    }
    else if ( dpy->xkbInfo )
	return 1;
    else {
	char *ignore = getenv("_XKB_IGNORE_EXTENSION");
	if ( ignore ) {
	    LockDisplay(dpy);
	    dpy->xkb_flags = XKB_ABSENT;
	    UnlockDisplay(dpy);
	    return 0;
	}
    }

    if ((!(dpy->xkb_flags&XKB_IN_USE))&&(!XkbUseExtension(dpy,NULL,NULL))) {
	return 0;
    }

    /* 
     * maybe get keyboard info from the server.
     */
    if (load) {
	xkb = (XkbLibInfoRec *)Xmalloc(sizeof(XkbLibInfoRec));
	if ( !xkb )
	    return 0;
	bzero(xkb,sizeof(XkbLibInfoRec));

	query = XkbFullClientInfoMask;
	xkb->desc = XkbGetMap(dpy,query,XKB_USE_CORE_KBD);
	LockDisplay(dpy);
	dpy->xkbInfo = xkb;
	dpy->xkb_flags |= XKB_IN_USE;
	UnlockDisplay(dpy);

	xkb->charset = XkbGetCharset(NULL);
	if ( xkb->charset ) {
	    XkbGetConverters(xkb->charset,&xkb->cvt);
	}
    }
    return 1;
}

int
XTranslateKeySym(dpy, sym_return, modifiers, buffer, nbytes)
    Display *dpy;
    register KeySym *sym_return;
    unsigned int modifiers;
    char *buffer;
    int nbytes;
{
    char tmp[4];
    register XkbLibInfoRec *xkb;
    register struct _XKeytrans *p; 
    int n;

    if ((!dpy->xkbInfo) && (!_XkbInitializeDpy(dpy,True)))
	return _NonXkbTranslateKeySym(dpy,*sym_return,modifiers,buffer,nbytes);
    if ((buffer==NULL)||(nbytes==0)) {
	buffer= tmp;
	nbytes= 4;
    }

    xkb = dpy->xkbInfo;
    /* see if symbol rebound, if so, return that string. */
    for (p = dpy->key_bindings; p; p = p->next) {
	if (((modifiers & AllMods) == p->state) && (*sym_return == p->key)) {
	    int tmp = p->len;
	    if (tmp > nbytes) tmp = nbytes;
	    bcopy (p->string, buffer, tmp);
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
	    for (i=0;i<n;i++) {
		buffer[i] = toupper(buffer[i]);
	    }
	    if (n==1)
		 *sym_return=(*xkb->cvt.MBToKS)(xkb->cvt.MBToKSPriv,buffer,n,0);
	    else *sym_return= NoSymbol;
	}
    }

    if ( modifiers&ControlMask ) {
	*sym_return = NoSymbol;
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

    if ((!event->display->xkbInfo)&&(!_XkbInitializeDpy(event->display,True)))
	return _NonXkbLookupString(event, buffer, nbytes, keysym, status);

    if (keysym==NULL)
	keysym= &dummy;

    if (!XTranslateKey(event->display,event->keycode,event->state,
							&new_mods,keysym))
	return 0;

    rtrnLen = XTranslateKeySym(event->display,keysym,new_mods,buffer,nbytes);

#ifdef sgi
    if ( status ) {
	static int been_here= 0;
	if ( !been_here ) {
	    SGIInitComposeTables();
	    been_here = 1;
	}
	if ( !SGILegalXComposeStatus(status) ) {
	    status->compose_ptr = NULL;
	    status->chars_matched = 0;
	}
	if ( ((status->chars_matched>0)&&(status->compose_ptr!=NULL)) || 
			SGIIsComposeKey(*keysym,event->keycode,status) ) {
	    SGIComposeRtrn rtrn;
	    XkbLibInfoRec *xkb;

	    xkb = event->display->xkbInfo;
	    switch (SGIProcessComposeSym(status,*keysym,&rtrn)) {
		case SGI_COMPOSE_IGNORE:
		    break;
		case SGI_COMPOSE_IN_PROGRESS:
		    *keysym = NoSymbol;
		    return 0;
		case SGI_COMPOSE_FAIL:
		{
		    int n = 0, len= 0;
		    for (n=len=0;rtrn.sym[n]!=XK_VoidSymbol;n++) {
			if ( nbytes-len > 0 ) {
    			    len += (*xkb->cvt.KSToMB)(xkb->cvt.KSToMBPriv,
							rtrn.sym[n],
							buffer+len,nbytes-len,
							NULL);
			}
		    }
		    if ( n==1 )	*keysym = rtrn.sym[0];
		    else	*keysym = NoSymbol;
		    return len;
		}
		case SGI_COMPOSE_SUCCEED:
		{
		    int len,n = 0;

		    *keysym = rtrn.matchSym;
		    if ( rtrn.str[0]!='\0' ) {
			strncpy(buffer,rtrn.str,nbytes-1);
			buffer[nbytes-1]= '\0';
			len = strlen(buffer);
		    }
		    else {
			len = (*xkb->cvt.KSToMB)(xkb->cvt.KSToMBPriv,
							rtrn.matchSym,
							buffer,nbytes,
							NULL);
		    }
		    for (n=0;rtrn.sym[n]!=XK_VoidSymbol;n++) {
			if ( nbytes-len > 0 ) {
			    len+= (*xkb->cvt.KSToMB)(xkb->cvt.KSToMBPriv,
							rtrn.sym[n],
							buffer+len,nbytes-len,
							NULL);
			}
		    }
		    if (n>0)
			*keysym = NoSymbol;
		    return len;
		}
	    }
	}
    }
#endif
    return rtrnLen;
}

