/* $XConsortium: a2x.c,v 1.3 92/03/11 12:11:46 rws Exp $ */
/*

Copyright 1992 by the Massachusetts Institute of Technology

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

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <termios.h>
#define _POSIX_SOURCE
#include <signal.h>

Display *dpy;
unsigned short modifiers[256];
KeyCode keycodes[256];
unsigned short modmask[256];
unsigned short curmods = 0;
unsigned short tempmods = 0;
KeyCode shift, control, lock, mod1, mod2, mod3, mod4, mod5;
KeySym last_sym = 0;
KeyCode last_keycode = 0;
struct termios oldterm;
int istty = 0;

usage()
{
    printf("a2x: [-d display] [-e]\n");
    exit(1);
}

quit(val)
{
    if (istty)
	tcsetattr(0, TCSANOW, &oldterm);
    exit(val);
}

void
catch(sig)
    int	sig;
{
    fprintf(stderr, "a2x: interrupt received, exiting\n");
    quit(1);
}

ioerror(Dpy)
    Display *Dpy;
{
    fprintf(stderr, "a2x: display connection lost, exiting\n");
    quit(1);
}

reset_mapping()
{
    int minkey, maxkey;
    register int i, j;
    KeySym sym;
    register int c;
    XModifierKeymap *mmap;

    XDisplayKeycodes(dpy, &minkey, &maxkey);
    bzero((char *)modifiers, sizeof(modifiers));
    bzero((char *)keycodes, sizeof(keycodes));
    bzero((char *)modmask, sizeof(modmask));
    for (i = minkey; i <= maxkey; i++) {
	sym = XKeycodeToKeysym(dpy, i, 0);
	if (sym > 0 && sym < 128) {
	    keycodes[sym] = i;
	    modifiers[sym] = 0;
	}
	sym = XKeycodeToKeysym(dpy, i, 1);
	if (sym > 0 && sym < 128) {
	    keycodes[sym] = i;
	    modifiers[sym] = ShiftMask;
	}
    }
    for (c = 0; c < 32; c++) {
	i = c + '@';
	if (i >= 'A' && i <= 'Z')
	    i += 'a' - 'A';
	keycodes[c] = keycodes[i];
	modifiers[c] = modifiers[i] | ControlMask;
    }
    c = XKeysymToKeycode(dpy, XK_Return);
    if (c) {
	keycodes['\r'] = c;
	modifiers['\r'] = 0;
    }
    c = XKeysymToKeycode(dpy, XK_Tab);
    if (c) {
	keycodes['\t'] = c;
	modifiers['\t'] = 0;
    }
    c = XKeysymToKeycode(dpy, XK_Escape);
    if (c) {
	keycodes['\033'] = c;
	modifiers['\033'] = 0;
    }
    c = XKeysymToKeycode(dpy, XK_Delete);
    if (c) {
	keycodes[127] = c;
	modifiers[127] = 0;
    }
    mmap = XGetModifierMapping(dpy);
    j = 0;
    shift = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = ShiftMask;
    j += mmap->max_keypermod; /* lock */
    control = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = ControlMask;
    mod1 = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = Mod1Mask;
    mod2 = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = Mod2Mask;
    mod3 = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = Mod3Mask;
    mod4 = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = Mod4Mask;
    mod5 = mmap->modifiermap[j];
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j]) modmask[mmap->modifiermap[j]] = Mod5Mask;
    XFreeModifiermap(mmap);
    last_sym = 0;
}

do_key(key, mods)
    int key;
    unsigned short mods;
{

    if (!key)
	return;
    if (modmask[key]) {
	if (!(tempmods & modmask[key])) {
	    switch (modmask[key])
	    {
	    case ShiftMask:
		XTestFakeKeyEvent(dpy, shift, True, 0);
		break;
	    case ControlMask:
		XTestFakeKeyEvent(dpy, control, True, 0);
		break;
	    case Mod1Mask:
		XTestFakeKeyEvent(dpy, mod1, True, 0);
		break;
	    case Mod2Mask:
		XTestFakeKeyEvent(dpy, mod2, True, 0);
		break;
	    case Mod3Mask:
		XTestFakeKeyEvent(dpy, mod3, True, 0);
		break;
	    case Mod4Mask:
		XTestFakeKeyEvent(dpy, mod4, True, 0);
		break;
	    case Mod5Mask:
		XTestFakeKeyEvent(dpy, mod5, True, 0);
		break;
	    }
	}
	curmods |= modmask[key];
	tempmods |= modmask[key];
	return;
    }
    if ((mods & ShiftMask) && !(curmods & ShiftMask)) {
	XTestFakeKeyEvent(dpy, shift, True, 0);
	curmods |= ShiftMask;
    } else if (!(mods & ShiftMask) && (curmods & ShiftMask)) {
	XTestFakeKeyEvent(dpy, shift, False, 0);
	curmods &= ~ShiftMask;
	tempmods &= ~ShiftMask;
    }
    if ((mods & ControlMask) && !(curmods & ControlMask)) {
	XTestFakeKeyEvent(dpy, control, True, 0);
	curmods |= ControlMask;
    } else if (!(mods & ControlMask) && (curmods & ControlMask)) {
	XTestFakeKeyEvent(dpy, control, False, 0);
	curmods &= ~ControlMask;
	tempmods &= ~ControlMask;
    }
    XTestFakeKeyEvent(dpy, key, True, 0);
    XTestFakeKeyEvent(dpy, key, False, 0);
    if (tempmods) {
	if (tempmods & ShiftMask)
	    XTestFakeKeyEvent(dpy, shift, False, 0);
	if (tempmods & ControlMask)
	    XTestFakeKeyEvent(dpy, control, False, 0);
	if (tempmods & Mod1Mask)
	    XTestFakeKeyEvent(dpy, mod1, False, 0);
	if (tempmods & Mod2Mask)
	    XTestFakeKeyEvent(dpy, mod2, False, 0);
	if (tempmods & Mod3Mask)
	    XTestFakeKeyEvent(dpy, mod3, False, 0);
	if (tempmods & Mod4Mask)
	    XTestFakeKeyEvent(dpy, mod4, False, 0);
	if (tempmods & Mod5Mask)
	    XTestFakeKeyEvent(dpy, mod5, False, 0);
	curmods &= ~tempmods;
	tempmods = 0;
    }
}

dochar(c)
    int c;
{
    do_key(keycodes[c], modifiers[c]);
}

do_keysym(sym)
    KeySym sym;
{
    if (sym != last_sym) {
	last_sym = sym;
	last_keycode = XKeysymToKeycode(dpy, sym);
    }	
    do_key(last_keycode, 0);
}

quiesce()
{
    if (curmods & ControlMask)
	XTestFakeKeyEvent(dpy, control, False, 0);
    if (curmods & ShiftMask)
	XTestFakeKeyEvent(dpy, shift, False, 0);
    curmods &= ~(ControlMask|ShiftMask);
}

main(argc, argv)
    int argc;
    char **argv;
{
    register int n, i, j;
    int eventb, errorb, vmajor, vminor;
    struct termios term;
    int noecho = 1;
    char *dname = NULL;
    char buf[1024];
    XEvent ev;
    char keysym_char = '\024'; /* control T */
    KeySym sym;

    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (argv[0][0] != '-')
	    usage();
	switch (argv[0][1])
	{
	case 'd':
	    argc--; argv++;
	    if (!argc)
		usage();
	    dname = *argv;
	    break;
	case 'e':
	    noecho = 0;
	    break;
	default:
	    usage();
	}
    }
    dpy = XOpenDisplay(dname);
    if (!dpy) {
	fprintf(stderr, "%s: unable to open display '%s'\n",
		argv[0], XDisplayName(dname));
	exit(1);
    }
    if (!XTestQueryExtension(dpy, &eventb, &errorb, &vmajor, &vminor)) {
	fprintf(stderr, "%s: display '%s' does not support XTEST extension\n",
		argv[0], DisplayString(dpy));
	exit(1);
    }	
    if (tcgetattr(0, &term) >= 0) {
	istty = 1;
	oldterm = term;
	term.c_lflag &= ~(ICANON|ECHOCTL|ISIG);
	term.c_iflag &= ~(IXOFF|IXON|ICRNL);
	term.c_cc[VERASE] = '\b';
	if (noecho)
	    term.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &term);
	signal(SIGINT, catch);
	signal(SIGTERM, catch);
	XSetIOErrorHandler(ioerror);
    }
    reset_mapping(dpy);
    while (1) {
	n = read(0, buf, sizeof(buf));
	if (n < 0)
	    quit(0);
	if (i = XEventsQueued(dpy, QueuedAfterReading)) {
	    while (--i >= 0) {
		XNextEvent(dpy, &ev);
		if (ev.type == MappingNotify) {
		    XRefreshKeyboardMapping(&ev.xmapping);
		    reset_mapping(dpy);
		}
	    }
	}
	for (i = 0; i < n; i++) {
	    if (buf[i] != keysym_char) {
		dochar(buf[i]);
		continue;
	    }
	    i++;
	    for (j = i; 1; j++) {
		if (j == n) {
		    if (n == sizeof(buf))
			break;
		    n = read(0, buf+j, sizeof(buf)-j);
		    if (n < 0)
			quit(0);
		    n += j;
		}
		if (buf[j] != keysym_char)
		    continue;
		buf[j] = '\0';
		if (j == i)
		    dochar(keysym_char);
		else if (!strcmp(buf+i, "exit"))
		    quit(0);
		else if (sym = strtoul(buf+i, NULL, 16))
		    do_keysym(sym);
		else if (sym = XStringToKeysym(buf+i))
		    do_keysym(sym);
		i = j;
		break;
	    }
	}
	quiesce();
	XFlush(dpy);
    }
    quit(0);
}
