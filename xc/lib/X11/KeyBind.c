#include "copyright.h"

/* $Header: XKeyBind.c,v 11.38 88/02/07 11:56:07 jim Exp $ */
/* Copyright 1985, 1987, Massachusetts Institute of Technology */

/* Beware, here be monsters (still under construction... - JG */

#define NEED_EVENTS
#include "Xlib.h"
#include "Xlibint.h"
#include "Xutil.h"
#include "keysym.h"
#include <stdio.h>

#define HAS_CTRL(c)  (((c) >= '@' && (c) <= '\177') || (c) == ' ')

struct XKeytrans {
	struct XKeytrans *next;/* next on list */
	char *string;		/* string to return when the time comes */
	int len;		/* length of string (since NULL is legit)*/
	KeySym key;		/* keysym rebound */
	unsigned int state;	/* modifier state */
	KeySym *modifiers;	/* modifier keysyms you want */
	int mlen;		/* length of modifier list */
};

static struct XKeytrans *trans = NULL;

static KeySym KeyCodetoKeySym(dpy, keycode, col)
     register Display *dpy;
     KeyCode keycode;
     int col;
{
     int ind;
     /*
      * if keycode not defined in set, this should really be impossible.
      * in any case, if sanity check fails, return NoSymbol.
      */
     if (col < 0 || col > dpy->keysyms_per_keycode) return (NoSymbol);
     if (keycode < dpy->min_keycode || keycode > dpy->max_keycode) 
       return(NoSymbol);

     ind = (keycode - dpy->min_keycode) * dpy->keysyms_per_keycode + col;
     return (dpy->keysyms[ind]);
}

KeySym XKeycodeToKeysym(dpy, kc, col)
    Display *dpy;
    KeyCode kc;
    int col;
{
     if (dpy->keysyms == NULL)
         Initialize(dpy);
     return (KeyCodetoKeySym(dpy, kc, col));
}

KeyCode XKeysymToKeycode(dpy, ks)
    Display *dpy;
    KeySym ks;
{
    int         i;

     if (dpy->keysyms == NULL)
         Initialize(dpy);
    for (i = dpy->min_keycode; i <= dpy->max_keycode; i++) {
	int         j;

	for (j = 0; j < dpy->keysyms_per_keycode; j++) {
	    int ind = (i - dpy->min_keycode) * dpy->keysyms_per_keycode + j;

	    if (ks == dpy->keysyms[ind])
		return (i);
	}
    }
    return (0);
}

KeySym XLookupKeysym(event, col)
     register XKeyEvent *event;
     int col;
{
     if (event->display->keysyms == NULL)
         Initialize(event->display);
     return (XKeycodeToKeysym(event->display, event->keycode, col));
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
	    if (event->display->keysyms != NULL) {
	         Xfree ((char *)event->display->keysyms);
	         event->display->keysyms = NULL;
	    }
	    UnlockDisplay(event->display);
     }
     if(event->request == MappingModifier) {
	    LockDisplay(event->display);
	    if (event->display->modifiermap != NULL) {
		XFreeModifiermap(event->display->modifiermap);
		event->display->modifiermap = NULL;
	    }
	    UnlockDisplay(event->display);
	    /* go ahead and get it now, since initialize test may not fail */
	    event->display->modifiermap = XGetModifierMapping(event->display);
     }
}
static InitTranslationList()
{
	/* not yet implemented */
	/* should read keymap file and initialize list */
}

/*ARGSUSED*/
int XUseKeymap(filename) 
    char *filename;
{
  /* not yet implemented */
}

static Initialize(dpy)
Display *dpy;
{
    register KeySym *keysyms, *sym, *old, *endp, *temp;
    int per, n;

    if (trans == NULL) InitTranslationList();
    /* 
     * lets go get the keysyms from the server.
     */
    if (dpy->keysyms == NULL) {
	n = dpy->max_keycode - dpy->min_keycode + 1;
	keysyms = XGetKeyboardMapping (dpy, (KeyCode) dpy->min_keycode,
				       n, &per);
	/* need at least two per keycode, to have room for case conversion */
	if (per == 1) {
	    temp = (KeySym *) Xmalloc((unsigned)(n * sizeof(KeySym) * 2));
	    for (sym = temp, old = keysyms, endp = keysyms + n; old < endp;) {
		*sym++ = *old++;
	        *sym++ = NoSymbol;
	    }
	    Xfree((char *)keysyms);
	    keysyms = temp;
	    per = 2;
	    n <<= 1;
	}
	for (sym = keysyms, endp = keysyms + n*per; sym < endp; sym += per) {
	  if ((*(sym + 1) == NoSymbol) && (*sym >= XK_A) && (*sym <= XK_Z)) {
	      *(sym + 1) = *sym;
	      *sym += (XK_a - XK_A);
	      }
 	}
	LockDisplay(dpy);
	dpy->keysyms = keysyms;
	dpy->keysyms_per_keycode = per;
	UnlockDisplay(dpy);
    }
    if (dpy->modifiermap == NULL) {
	dpy->modifiermap = XGetModifierMapping(dpy);
    }
}

static int KeySymRebound(event, buf, symbol)
    XKeyEvent *event;
    char *buf;
    KeySym symbol;
{
    register struct XKeytrans *p;

    p = trans;
    while (p != NULL) {
	if (MatchEvent(event, symbol, p)) {
		bcopy (p->string, buf, p->len);
		return p->len;
		}
	p = p->next;
	}
    return -1;
}
  
Bool MatchEvent(event, symbol, p)
    XKeyEvent *event;
    KeySym symbol;
    register struct XKeytrans *p;
{
    if ((event->state == p->state) && (symbol == p->key)) return True;
    return False;
}

int XLookupString (event, buffer, nbytes, keysym, status)
     XKeyEvent *event;
     char *buffer;	/* buffer */
     int nbytes;	/* space in buffer for characters */
     KeySym *keysym;
     XComposeStatus *status;	/* not implemented */
{
     register KeySym symbol, lsymbol, usymbol;
     int length = 0;
     char buf[BUFSIZ];
     unsigned char byte3, byte4;

#ifdef lint
     status = status;
#endif

     if (event->display->keysyms == NULL)
         Initialize(event->display);

     lsymbol =  XKeycodeToKeysym(event->display, event->keycode, 0);
     usymbol =  XKeycodeToKeysym(event->display, event->keycode, 1);
     /*
      * we have to find out what kind of lock we are dealing with, if any.
      * if caps lock, only shift caps.
      */
     symbol = lsymbol;
     if (event->state & LockMask) {
	XModifierKeymap *m = event->display->modifiermap;
	int i;

	if (usymbol != NoSymbol)
	    symbol = usymbol;
	for (i = m->max_keypermod; i < 2*m->max_keypermod; i++) {
	    /*
	     *	Run through all the keys setting LOCK and,  if
	     *  ANY of them are CAPS_LOCK,  do Caps Lock.
	     *  This is kind of bogus,  but what else to do?
	     *  Supposing we have CAPS_LOCK,  but on the shifted
	     *  part of the key?
	     */
	    if (XKeycodeToKeysym(event->display, m->modifiermap[i], 0)
		    == XK_Caps_Lock) {
			if (usymbol >= XK_A && usymbol <= XK_Z)
			    symbol = usymbol;
			else
			    symbol = lsymbol;
			break;
		    }
	}
     }
     if ((event->state & ShiftMask) && usymbol != NoSymbol)
	     symbol = usymbol;

     if (keysym != NULL) *keysym = symbol;

     byte4 = symbol & 0xFF;
     byte3 = (symbol >> 8 ) & 0xFF;

     /*
      * see if symbol rebound, if so, return that string.
      * if any of high order 16 bits set, can only do ascii.
      * (reserved for future use, and for vendors).
      */
     if ((length = KeySymRebound(event, buf, symbol)) == -1) {
	    if ( IsModifierKey(symbol)   || IsCursorKey(symbol)
		|| IsPFKey (symbol)      || IsFunctionKey(symbol)
		|| IsMiscFunctionKey(symbol)
		|| (symbol == XK_Multi_key) || (symbol == XK_Kanji))  return 0;
            buf[0] = byte4;
	    /* if X keysym, convert to ascii by grabbing low 7 bits */
	    if (byte3 == 0xFF) buf[0] &= 0x7F;
	    /* only apply Control key if it makes sense, else ignore it */
	    if ((event->state & ControlMask) && HAS_CTRL(buf[0]))
	        buf[0] = buf[0] & 0x1F;
     	    length = 1;
      }
      if (length > nbytes) length = nbytes;
      bcopy (buf, buffer, length);
      return (length);
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

    if (dpy->keysyms == NULL)
    	Initialize(dpy);
    LockDisplay(dpy);
    tmp = trans;
    trans = p = (struct XKeytrans *)Xmalloc(sizeof(struct XKeytrans));
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

/*
 * given a KeySym, returns the first keycode found after the index value
 * in the table.  (Hopefully, can be found quickly).
 */
static CARD8 FindKeyCode(dpy, ind, code)
    register Display *dpy;
    int ind;
    register KeySym code;
{

    register KeySym *kmax = dpy->keysyms + 
	(dpy->max_keycode - dpy->min_keycode + 1) * dpy->keysyms_per_keycode;
    register KeySym *k = dpy->keysyms;	/* XXX not yet dealing with ind */
    if ((ind < dpy->min_keycode) || (ind > dpy->max_keycode)) return 0;
    while (k < kmax) {
	if (*k == code)
	    return(((k - dpy->keysyms)
		/ dpy->keysyms_per_keycode) + dpy->min_keycode);
	k += 1;
	}
    return 0;
}

	
/*
 * given a list of modifiers, computes the mask necessary for later matching.
 * This routine must lookup the key in the Keymap and then search to see
 * what modifier it is bound to, if any.
 */
static ComputeMaskFromKeytrans(dpy, p)
    Display *dpy;
    register struct XKeytrans *p;
{
    register int i;
    register CARD8 code;
    register XModifierKeymap *m = dpy->modifiermap;

    p->state = 0;
    for (i = 0; i < p->mlen; i++) {
	/* if not found, then not on current keyboard */
	if ((code = FindKeyCode(dpy, dpy->min_keycode, p->modifiers[i])) == 0)
		continue;
	/* code is now the keycode for the modifier you want */
	{
	    register int j;

	    for (j = 0; j < (m->max_keypermod<<3); j++) {
		if (m->modifiermap[j] && code == m->modifiermap[j])
		    p->state |= (1<<(j/m->max_keypermod));
	    }
	}
    }
    return;
}
