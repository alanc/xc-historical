/* $XConsortium$ */
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
#include <signal.h>

Display *dpy;
unsigned short modifiers[256];
KeyCode keycodes[256];
KeyCode shift, control;
struct termios oldterm;

usage()
{
    printf("a2x: [-d display] [-e]\n");
    exit(1);
}

void
catch(sig)
    int	sig;
{
    tcsetattr(0, TCSANOW, &oldterm);
    exit(1);
}

ioerror(Dpy)
    Display *Dpy;
{
    tcsetattr(0, TCSANOW, &oldterm);
    exit(1);
}

reset_mapping()
{
    int minkey, maxkey;
    register int i;
    KeySym sym;
    register int c;
    XModifierKeymap *mmap;

    XDisplayKeycodes(dpy, &minkey, &maxkey);
    bzero((char *)modifiers, sizeof(modifiers));
    bzero((char *)keycodes, sizeof(keycodes));
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
    shift = mmap->modifiermap[0];
    control = mmap->modifiermap[2 * mmap->max_keypermod];
    XFreeModifiermap(mmap);
}

unsigned short
dochar(c, curmods)
    int c;
    unsigned short curmods;
{
    register int i;
    unsigned short mods;

    i = keycodes[c];
    if (!i)
	return curmods;
    mods = modifiers[c];
    if ((mods & ShiftMask) && !(curmods & ShiftMask)) {
	XTestFakeKeyEvent(dpy, shift, True, 0);
	curmods |= ShiftMask;
    } else if (!(mods & ShiftMask) && (curmods & ShiftMask)) {
	XTestFakeKeyEvent(dpy, shift, False, 0);
	curmods &= ~ShiftMask;
    }
    if ((mods & ControlMask) && !(curmods & ControlMask)) {
	XTestFakeKeyEvent(dpy, control, True, 0);
	curmods |= ControlMask;
    } else if (!(mods & ControlMask) && (curmods & ControlMask)) {
	XTestFakeKeyEvent(dpy, control, False, 0);
	curmods &= ~ControlMask;
    }
    XTestFakeKeyEvent(dpy, i, True, 0);
    XTestFakeKeyEvent(dpy, i, False, 0);
    return curmods;
}

unsigned short
quiesce(curmods)
    unsigned short curmods;
{
    if (curmods & ControlMask)
	XTestFakeKeyEvent(dpy, control, False, 0);
    if (curmods & ShiftMask)
	XTestFakeKeyEvent(dpy, shift, False, 0);
    return curmods & ~(ControlMask|ShiftMask);
}

main(argc, argv)
    int argc;
    char **argv;
{
    register int n, i;
    int eventb, errorb, vmajor, vminor;
    struct termios term;
    int noecho = 1;
    char *dname = NULL;
    int istty = 0;
    unsigned short mods = 0;
    char buf[1024];

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
	XSetIOErrorHandler(ioerror);
    }
    reset_mapping(dpy);
    while (1) {
	n = read(0, buf, sizeof(buf));
	if (n < 0)
	    break;
	for (i = 0; i < n; i++)
	    mods = dochar(buf[i], mods);
	mods = quiesce(mods);
	XFlush(dpy);
    }
    if (istty)
	tcsetattr(0, TCSANOW, &oldterm);
    exit(0);
}
