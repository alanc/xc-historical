#include "copyright.h"

/* $XConsortium: XKeyBind.c,v 11.52 89/10/06 09:04:36 rws Exp $ */
/* Copyright 1985, 1987, Massachusetts Institute of Technology */

/* Beware, here be monsters (still under construction... - JG */

#define NEED_EVENTS
#include "Xlib.h"
#include "Xlibint.h"
#include "Xutil.h"
#define XK_MISCELLANY
#define XK_LATIN1
#include "keysymdef.h"
#include <stdio.h>

#define AllMods (ShiftMask|LockMask|ControlMask| \
		 Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)

static ComputeMaskFromKeytrans();
static int Initialize();
static void XConvertCase();

struct XKeytrans {
	struct XKeytrans *next;/* next on list */
	char *string;		/* string to return when the time comes */
	int len;		/* length of string (since NULL is legit)*/
	KeySym key;		/* keysym rebound */
	unsigned int state;	/* modifier state */
	KeySym *modifiers;	/* modifier keysyms you want */
	int mlen;		/* length of modifier list */
};

static KeySym
KeyCodetoKeySym(dpy, keycode, col)
    register Display *dpy;
    KeyCode keycode;
    int col;
{
    register int per = dpy->keysyms_per_keycode;
    register KeySym *syms;
    KeySym lsym, usym;

    if ((col < 0) || ((col >= per) && (col > 3)) ||
	(keycode < dpy->min_keycode) || (keycode > dpy->max_keycode))
      return NoSymbol;

    syms = &dpy->keysyms[(keycode - dpy->min_keycode) * per];
    if (col < 4) {
	if (col > 1) {
	    while ((per > 2) && (syms[per - 1] == NoSymbol))
		per--;
	    if (per < 3)
		col -= 2;
	}
	if ((per <= (col|1)) || (syms[col|1] == NoSymbol)) {
	    XConvertCase(dpy, syms[col&~1], &lsym, &usym);
	    if (!(col & 1))
		return lsym;
	    else if (usym == lsym)
		return NoSymbol;
	    else
		return usym;
	}
    }
    return syms[col];
}

KeySym
XKeycodeToKeysym(dpy, kc, col)
    Display *dpy;
    KeyCode kc;
    int col;
{
    if (!dpy->keysyms)
	Initialize(dpy);
    return KeyCodetoKeySym(dpy, kc, col);
}

KeyCode
XKeysymToKeycode(dpy, ks)
    Display *dpy;
    KeySym ks;
{
    register int i, j;

     if (!dpy->keysyms)
         Initialize(dpy);
    for (i = dpy->min_keycode; i <= dpy->max_keycode; i++) {
	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    if (KeyCodetoKeySym(dpy, i, j) == ks)
		return i;
	}
    }
    return 0;
}

KeySym
XLookupKeysym(event, col)
    register XKeyEvent *event;
    int col;
{
    if (!event->display->keysyms)
	Initialize(event->display);
    return KeyCodetoKeySym(event->display, event->keycode, col);
}

static
InitModMap(dpy)
    Display *dpy;
{
    register XModifierKeymap *map;
    register int i, j, n;
    KeySym sym;
    register struct XKeytrans *p;

    dpy->modifiermap = map = XGetModifierMapping(dpy);
    if (!dpy->keysyms)
	Initialize(dpy);
    LockDisplay(dpy);
    /* If any Lock key contains Caps_Lock, then interpret as Caps_Lock,
     * else if any contains Shift_Lock, then interpret as Shift_Lock,
     * else ignore Lock altogether.
     */
    dpy->lock_meaning = NoSymbol;
    /* Lock modifiers are in the second row of the matrix */
    n = 2 * map->max_keypermod;
    for (i = map->max_keypermod; i < n; i++) {
	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    sym = KeyCodetoKeySym(dpy, map->modifiermap[i], j);
	    if (sym == XK_Caps_Lock) {
		dpy->lock_meaning = XK_Caps_Lock;
		break;
	    } else if (sym == XK_Shift_Lock) {
		dpy->lock_meaning = XK_Shift_Lock;
	    }
	}
    }
    /* Now find any Mod<n> modifier acting as the Group modifier */
    dpy->mode_switch = 0;
    n *= 4;
    for (i = 3*map->max_keypermod; i < n; i++) {
	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    sym = KeyCodetoKeySym(dpy, map->modifiermap[i], j);
	    if (sym == XK_Mode_switch)
		dpy->mode_switch |= 1 << (i / map->max_keypermod);
	}
    }
    for (p = dpy->key_bindings; p; p = p->next)
	ComputeMaskFromKeytrans(dpy, p);
    UnlockDisplay(dpy);
}

XRefreshKeyboardMapping(event)
    register XMappingEvent *event;
{
    extern void XFreeModifiermap();

    if(event->request == MappingKeyboard) {
	/* XXX should really only refresh what is necessary
	 * for now, make initialize test fail
	 */
	LockDisplay(event->display);
	if (event->display->keysyms) {
	     Xfree ((char *)event->display->keysyms);
	     event->display->keysyms = NULL;
	}
	UnlockDisplay(event->display);
    }
    if(event->request == MappingModifier) {
	LockDisplay(event->display);
	if (event->display->modifiermap) {
	    XFreeModifiermap(event->display->modifiermap);
	    event->display->modifiermap = NULL;
	}
	UnlockDisplay(event->display);
	/* go ahead and get it now, since initialize test may not fail */
	InitModMap(event->display);
    }
}

static
Initialize(dpy)
    Display *dpy;
{
    int per, n;
    KeySym *keysyms;

    /* 
     * lets go get the keysyms from the server.
     */
    if (!dpy->keysyms) {
	n = dpy->max_keycode - dpy->min_keycode + 1;
	keysyms = XGetKeyboardMapping (dpy, (KeyCode) dpy->min_keycode,
				       n, &per);
	LockDisplay(dpy);
	dpy->keysyms = keysyms;
	dpy->keysyms_per_keycode = per;
	UnlockDisplay(dpy);
    }
    if (!dpy->modifiermap)
        InitModMap(dpy);
}

/*ARGSUSED*/
static void
XConvertCase(dpy, sym, lower, upper)
    Display *dpy;
    register KeySym sym;
    KeySym *lower;
    KeySym *upper;
{
    *lower = sym;
    *upper = sym;
    switch(sym >> 8) {
    case 0:
	if ((sym >= XK_A) && (sym <= XK_Z))
	    *lower += (XK_a - XK_A);
	else if ((sym >= XK_a) && (sym <= XK_z))
	    *upper -= (XK_a - XK_A);
	else if ((sym >= XK_Agrave) && (sym <= XK_Odiaeresis))
	    *lower += (XK_agrave - XK_Agrave);
	else if ((sym >= XK_agrave) && (sym <= XK_odiaeresis))
	    *upper -= (XK_agrave - XK_Agrave);
	else if ((sym >= XK_Ooblique) && (sym <= XK_Thorn))
	    *lower += (XK_oslash - XK_Ooblique);
	else if ((sym >= XK_oslash) && (sym <= XK_thorn))
	    *upper -= (XK_oslash - XK_Ooblique);
	break;
    default:
	/* XXX do all other sets */
	break;
    }
}

static void
XTranslateKey(dpy, keycode, modifiers, modifiers_return, keysym_return)
    register Display *dpy;
    KeyCode keycode;
    register unsigned int modifiers;
    unsigned int *modifiers_return;
    KeySym *keysym_return;
{
    int per;
    register KeySym *syms;
    KeySym sym, lsym, usym;

    if (!dpy->keysyms)
	Initialize(dpy);
    *modifiers_return = (ShiftMask|LockMask) | dpy->mode_switch;
    if ((keycode < dpy->min_keycode) || (keycode > dpy->max_keycode))  {
	*keysym_return = NoSymbol;
	return;
    }
    per = dpy->keysyms_per_keycode;
    syms = &dpy->keysyms[(keycode - dpy->min_keycode) * per];
    while ((per > 2) && (syms[per - 1] == NoSymbol))
	per--;
    if ((per > 2) && (modifiers & dpy->mode_switch)) {
	syms += 2;
	per -= 2;
    }
    if (!(modifiers & ShiftMask) &&
	(!(modifiers & LockMask) || (dpy->lock_meaning == NoSymbol))) {
	if ((per == 1) || (syms[1] == NoSymbol))
	    XConvertCase(dpy, syms[0], keysym_return, &usym);
	else
	    *keysym_return = syms[0];
    } else if (!(modifiers & LockMask) ||
	       (dpy->lock_meaning != XK_Caps_Lock)) {
	if ((per == 1) || ((usym = syms[1]) == NoSymbol))
	    XConvertCase(dpy, syms[0], &lsym, &usym);
	*keysym_return = usym;
    } else {
	if ((per == 1) || ((sym = syms[1]) == NoSymbol))
	    sym = syms[0];
	XConvertCase(dpy, sym, &lsym, &usym);
	if (!(modifiers & ShiftMask) && (sym != syms[0]) &&
	    ((sym != usym) || (lsym == usym)))
	    XConvertCase(dpy, syms[0], &lsym, &usym);
	*keysym_return = usym;
    }
    if (*keysym_return == XK_VoidSymbol)
	*keysym_return = NoSymbol;
}

static int
XTranslateKeySym(dpy, symbol, modifiers, buffer, nbytes)
    Display *dpy;
    register KeySym symbol;
    unsigned int modifiers;
    char *buffer;
    int nbytes;
{
    register struct XKeytrans *p; 
    int length;
    unsigned long hiBytes;
    register unsigned char c;

    if (!symbol)
	return 0;
    /* see if symbol rebound, if so, return that string. */
    for (p = dpy->key_bindings; p; p = p->next) {
	if (((modifiers & AllMods) == p->state) && (symbol == p->key)) {
	    length = p->len;
	    if (length > nbytes) length = nbytes;
	    bcopy (p->string, buffer, length);
	    return length;
	}
    }
    /* try to convert to Latin-1, handling control */
    hiBytes = symbol >> 8;
    if (!nbytes ||
	((hiBytes != 0) && (hiBytes != 0xFF)) ||
	IsModifierKey(symbol) ||
	IsCursorKey(symbol) ||
	IsPFKey(symbol) ||
	IsFunctionKey(symbol) ||
	IsMiscFunctionKey(symbol) ||
	(symbol == XK_Scroll_Lock) ||
	(symbol == XK_Multi_key) ||
	(symbol == XK_Kanji))
	return 0;

    /* if X keysym, convert to ascii by grabbing low 7 bits */
    if (symbol == XK_KP_Space)
	c = XK_space & 0x7F; /* patch encoding botch */
    else if (symbol == XK_hyphen)
	c = XK_minus & 0xFF; /* map to equiv character */
    else if (hiBytes == 0xFF)
	c = symbol & 0x7F;
    else
	c = symbol & 0xFF;
    /* only apply Control key if it makes sense, else ignore it */
    if (modifiers & ControlMask) {
	if ((c >= '@' && c < '\177') || c == ' ') c &= 0x1F;
	else if (c == '2') c = '\000';
	else if (c >= '3' && c <= '7') c -= ('3' - '\033');
	else if (c == '8') c = '\177';
	else if (c == '/') c = '_' & 0x1F;
    }
    buffer[0] = c;
    return 1;
}
  
/*ARGSUSED*/
int
XLookupString (event, buffer, nbytes, keysym, status)
    register XKeyEvent *event;
    char *buffer;	/* buffer */
    int nbytes;	/* space in buffer for characters */
    KeySym *keysym;
    XComposeStatus *status;	/* not implemented */
{
    unsigned int modifiers;
    KeySym symbol;

    XTranslateKey(event->display, event->keycode, event->state,
		  &modifiers, &symbol);
    if (keysym)
	*keysym = symbol;
    /* arguable whether to use (event->state & ~modifiers) here */
    return XTranslateKeySym(event->display, symbol, event->state,
			    buffer, nbytes);
}

XRebindKeysym (dpy, keysym, mlist, nm, str, nbytes)
    Display *dpy;
    KeySym keysym;
    KeySym *mlist;
    int nm;		/* number of modifiers in mlist */
    unsigned char *str;
    int nbytes;
{
    register struct XKeytrans *tmp, *p;
    int nb;

    if (!dpy->keysyms)
    	Initialize(dpy);
    LockDisplay(dpy);
    tmp = dpy->key_bindings;
    dpy->key_bindings = p = (struct XKeytrans *)Xmalloc(sizeof(struct XKeytrans));
    p->next = tmp;	/* chain onto list */
    p->string = (char *) Xmalloc(nbytes);
    bcopy (str, p->string, nbytes);
    p->len = nbytes;
    nb = sizeof (KeySym) * nm;
    p->modifiers = (KeySym *) Xmalloc(nb);
    bcopy (mlist, p->modifiers, nb);
    p->key = keysym;
    p->mlen = nm;
    ComputeMaskFromKeytrans(dpy, p);
    UnlockDisplay(dpy);
    return;
}

_XFreeKeyBindings (dpy)
    Display *dpy;
{
    register struct XKeytrans *p, *np;

    for (p = dpy->key_bindings; p; p = np) {
	np = p->next;
	Xfree(p->string);
	Xfree((char *)p->modifiers);
	Xfree((char *)p);
    }   
}

/*
 * given a KeySym, returns the first keycode containing it, if any.
 */
static CARD8
FindKeyCode(dpy, code)
    register Display *dpy;
    register KeySym code;
{

    register KeySym *kmax = dpy->keysyms + 
	(dpy->max_keycode - dpy->min_keycode + 1) * dpy->keysyms_per_keycode;
    register KeySym *k = dpy->keysyms;
    while (k < kmax) {
	if (*k == code)
	    return (((k - dpy->keysyms) / dpy->keysyms_per_keycode) +
		    dpy->min_keycode);
	k += 1;
	}
    return 0;
}

	
/*
 * given a list of modifiers, computes the mask necessary for later matching.
 * This routine must lookup the key in the Keymap and then search to see
 * what modifier it is bound to, if any.  Sets the AnyModifier bit if it
 * can't map some keysym to a modifier.
 */
static
ComputeMaskFromKeytrans(dpy, p)
    Display *dpy;
    register struct XKeytrans *p;
{
    register int i;
    register CARD8 code;
    register XModifierKeymap *m = dpy->modifiermap;

    p->state = AnyModifier;
    for (i = 0; i < p->mlen; i++) {
	/* if not found, then not on current keyboard */
	if ((code = FindKeyCode(dpy, p->modifiers[i])) == 0)
		return;
	/* code is now the keycode for the modifier you want */
	{
	    register int j = m->max_keypermod<<3;

	    while ((--j >= 0) && (code != m->modifiermap[j]))
		;
	    if (j < 0)
		return;
	    p->state |= (1<<(j/m->max_keypermod));
	}
    }
    p->state &= AllMods;
}
