/* $XConsortium: a2x.c,v 1.28 92/03/24 19:11:02 rws Exp $ */
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

/*

Syntax of magic values in the input stream:

^T^B<number>^T		toggle button <number> state (press or release)
^T^C			toggle Control key for next character
^T^D			slow down moving mouse
^T^G			start continuous motion
^T^M			toggle Meta key for next character
^T^Q			quit moving mouse
^T^S			toggle Shift key for next character
^T^T			^T
^T^W<screen-num> <x-pos-num> <y-pos-num>^T
			warp to position (<x-pos-num>,<y-pos-num>) on
			screen <screen-num> (can be -1 for current)
^T^X<number>^T		move mouse <number> pixels horizontally
^T^Y<number>^T		move mouse <number> pixels vertically
^Texit^T		exit the program
^Tdbg^T			print debugging info
^T<hexnumber>^T		press and release key with numeric keysym <hexnumber>
^Tname^T		press and release key with keysym named <name>

Note: if key is attached to a modifier, pressing it is temporary, will be
released automatically at next button or non-modifier key.

*/

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <termios.h>
#define _POSIX_SOURCE
#include <signal.h>

#define control_char '\024' /* control T */
#define control_end '\224'

Display *dpy;
unsigned char button_map[256];
unsigned short modifiers[256];
KeyCode keycodes[256];
unsigned short modmask[256];
unsigned short curmods = 0;
unsigned short tempmods = 0;
KeyCode shift, control, mod1, mod2, mod3, mod4, mod5, meta;
Bool bs_is_del = True;
KeySym last_sym = 0;
KeyCode last_keycode = 0;
struct termios oldterm;
Bool istty = False;
Bool moving = False;
int moving_x = 0;
int moving_y = 0;
int (*olderror)();
int (*oldioerror)();
char history[4096];
int history_end = 0;
typedef struct {
    int bscount;
    char *seq;
    int seq_len;
    char *undo;
    int undo_len;
} undo;
undo no_undos[] = {
{
    0,
    0,
    0,
    0,
    0
}
};
undo *undos = no_undos;
int curbscount = 0;
Bool in_control_seq = False;

void process();

void
usage()
{
    printf("a2x: [-d display] [-e] [-b] [-u <undofile>]\n");
    exit(1);
}

void
reset()
{
    if (istty)
	tcsetattr(0, TCSANOW, &oldterm);
}

void
quit(val)
{
    reset();
    exit(val);
}

void
catch(sig)
int	sig;
{
    fprintf(stderr, "a2x: interrupt received, exiting\n");
    quit(1);
}

int
error(Dpy, err)
    Display *Dpy;
    XErrorEvent *err;
{
    reset();
    return (*olderror)(Dpy, err);
}

int
ioerror(Dpy)
    Display *Dpy;
{
    reset();
    return (*oldioerror)(Dpy);
}

void
reset_mapping()
{
    int minkey, maxkey;
    int i, j;
    KeySym sym;
    int c;
    XModifierKeymap *mmap;
    unsigned char bmap[256];
    
    bzero((char *)button_map, sizeof(button_map));
    j = XGetPointerMapping(dpy, bmap, sizeof(bmap));
    for (i = 0; i < j; i++)
	button_map[bmap[i]] = i + 1;
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
    shift = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[shift = mmap->modifiermap[j]] = ShiftMask;
    j += mmap->max_keypermod; /* lock */
    control = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[control = mmap->modifiermap[j]] = ControlMask;
    mod1 = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[mod1 = mmap->modifiermap[j]] = Mod1Mask;
    mod2 = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[mod2 = mmap->modifiermap[j]] = Mod2Mask;
    mod3 = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[mod3 = mmap->modifiermap[j]] = Mod3Mask;
    mod4 = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[mod4 = mmap->modifiermap[j]] = Mod4Mask;
    mod5 = 0;
    for (i = 0; i < mmap->max_keypermod; i++, j++)
	if (mmap->modifiermap[j])
	    modmask[mod5 = mmap->modifiermap[j]] = Mod5Mask;
    XFreeModifiermap(mmap);
    i = XKeysymToKeycode(dpy, XK_Meta_L);
    if (!i)
	i = XKeysymToKeycode(dpy, XK_Meta_R);
    switch (modmask[i]) {
    case Mod1Mask:
	meta = mod1;
	break;
    case Mod2Mask:
	meta = mod2;
	break;
    case Mod3Mask:
	meta = mod3;
	break;
    case Mod4Mask:
	meta = mod4;
	break;
    case Mod5Mask:
	meta = mod5;
	break;
    }
    last_sym = 0;
    if (bs_is_del) {
	keycodes['\b'] = keycodes['\177'];
	modifiers['\b'] = modifiers['\177'];
    }
}

void
setup_tempmods()
{
    if (tempmods) {
	if (tempmods & ShiftMask)
	    XTestFakeKeyEvent(dpy, shift, True, 0);
	if (tempmods & ControlMask)
	    XTestFakeKeyEvent(dpy, control, True, 0);
	if (tempmods & Mod1Mask)
	    XTestFakeKeyEvent(dpy, mod1, True, 0);
	if (tempmods & Mod2Mask)
	    XTestFakeKeyEvent(dpy, mod2, True, 0);
	if (tempmods & Mod3Mask)
	    XTestFakeKeyEvent(dpy, mod3, True, 0);
	if (tempmods & Mod4Mask)
	    XTestFakeKeyEvent(dpy, mod4, True, 0);
	if (tempmods & Mod5Mask)
	    XTestFakeKeyEvent(dpy, mod5, True, 0);
	curmods |= tempmods;
    }
}

void
teardown_tempmods()
{
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

void
do_key(key, mods)
    int key;
    unsigned short mods;
{

    if (!key)
	return;
    if (modmask[key]) {
	tempmods ^= modmask[key];
	return;
    }
    setup_tempmods();
    if (!(tempmods & ShiftMask)) {
	if ((mods & ShiftMask) && !(curmods & ShiftMask)) {
	    XTestFakeKeyEvent(dpy, shift, True, 0);
	    curmods |= ShiftMask;
	} else if (!(mods & ShiftMask) && (curmods & ShiftMask)) {
	    XTestFakeKeyEvent(dpy, shift, False, 0);
	    curmods &= ~ShiftMask;
	    tempmods &= ~ShiftMask;
	}
    }
    if (!(tempmods & ControlMask)) {
	if ((mods & ControlMask) && !(curmods & ControlMask)) {
	    XTestFakeKeyEvent(dpy, control, True, 0);
	    curmods |= ControlMask;
	} else if (!(mods & ControlMask) && (curmods & ControlMask)) {
	    XTestFakeKeyEvent(dpy, control, False, 0);
	    curmods &= ~ControlMask;
	    tempmods &= ~ControlMask;
	}
    }
    XTestFakeKeyEvent(dpy, key, True, 0);
    XTestFakeKeyEvent(dpy, key, False, 0);
    teardown_tempmods();
}

void
do_char(c)
    unsigned char c;
{
    do_key(keycodes[c], modifiers[c]);
}

void
do_keysym(sym)
    KeySym sym;
{
    if (sym != last_sym) {
	last_sym = sym;
	last_keycode = XKeysymToKeycode(dpy, sym);
    }	
    do_key(last_keycode, 0);
}

void
do_button(button)
    int button;
{
    Window root, child;
    int rx, ry, x, y;
    unsigned int state;

    if (button < 1 || button > 5)
	return;
    XQueryPointer(dpy, DefaultRootWindow(dpy), &root, &child, &rx, &ry,
		  &x, &y, &state);
    setup_tempmods();
    XTestFakeButtonEvent(dpy, button_map[button],
			 (state & (Button1Mask << (button - 1))) == 0, 0);
    teardown_tempmods();
}

void
move_pointer(dx, dy)
    int dx, dy;
{
    XTestFakeRelativeMotionEvent(dpy, dx, dy, 0);
}

void
do_x(delta)
    int delta;
{
    if (moving)
	moving_x = delta;
    else
	move_pointer(delta, 0);
}

void
do_y(delta)
    int delta;
{
    if (moving)
	moving_y = delta;
    else
	move_pointer(0, delta);
}

void
do_warp(screen, x, y)
    int screen, x, y;
{
    XTestFakeMotionEvent(dpy, screen, x, y, 0);
}

void
start_moving()
{
    moving = True;
    moving_x = 0;
    moving_y = 0;
}

void
stop_moving()
{
    moving = False;
    moving_x = 0;
    moving_y = 0;
}

void
slow_down()
{
    if (moving_x < 0)
	moving_x = -1;
    else if (moving_x > 0)
	moving_x = 1;
    if (moving_y < 0)
	moving_y = -1;
    else if (moving_y > 0)
	moving_y = 1;
}

void
quiesce()
{
    if (curmods & ControlMask)
	XTestFakeKeyEvent(dpy, control, False, 0);
    if (curmods & ShiftMask)
	XTestFakeKeyEvent(dpy, shift, False, 0);
    curmods &= ~(ControlMask|ShiftMask);
}

void
trim_history()
{
    if (history_end < (sizeof(history)/2))
	history_end = 0;
    else {
	bcopy(history + (sizeof(history)/2), history, history_end);
	history_end -= sizeof(history)/2;
    }
    bzero(history + history_end, sizeof(history) - history_end);
}

void
save_unit(buf, i, j)
    char *buf;
    int i, j;
{
    if (buf[j] == control_char)
	buf[j] = control_end;
    j = j - i + 1;
    if (history_end + j > sizeof(history))
	trim_history();
    bcopy(buf + i, history + history_end, j);
    history_end += j;
}

Bool
has_bs(c)
    char c;
{
    return (c != control_end) && (!iscntrl(c) || (c == '\r'));
}

void
undo_stroke()
{
    char c;
    int i;

    if (!history_end) {
	in_control_seq = False;
	for (; curbscount; curbscount--)
	    do_char('\b');
	return;
    }
    c = history[history_end-1];
    if (!in_control_seq &&
	((c == control_end) ||
	 ((history_end > 1) &&
	  (history[history_end-2] == control_char)))) {
	in_control_seq = True;
    }
    if (has_bs(c)) {
	curbscount--;
	if (!in_control_seq) {
	    if ((history_end < 3) ||
		(history[history_end-2] != '\015') ||
		(history[history_end-3] != control_char))
		do_char('\b');
	}
    }
    history_end--;
    if (in_control_seq) {
	for (i = history_end - 1; i >= 0; i--) {
	    c = history[i];
	    if (c == control_char) {
		history_end = i;
		in_control_seq = False;
		break;
	    } else if (has_bs(c))
		break;
	}
    }
}

void
do_backspace(c)
    char c;
{
    undo *u;
    Bool partial = False;

    curbscount++;
    while (curbscount) {
	if (!in_control_seq) {
	    for (u = undos; u->bscount; u++) {
		if (history_end >= u->seq_len &&
		    !bcmp(history+history_end-u->seq_len, u->seq,
			  u->seq_len)) {
		    if (curbscount < u->bscount)
			partial = True;
		    else {
			history_end -= u->seq_len;
			curbscount -= u->bscount;
			process(u->undo, u->undo_len, 0);
			return;
		    }
		}
	    }
	    if (partial)
		return;
	}
	undo_stroke();
    }
}

char *
parse_string(buf, ip, lenp, term)
    char *buf;
    int *ip;
    int *lenp;
    char term;
{
    int i, j;
    char c;
    char *seq;

    j = 0;
    for (i = *ip; (c = buf[i]) && (c != term); i++) {
	if (c == '^') {
	    c = buf[++i];
	    if (c == '?')
		buf[j++] = '\177';
	    else {
		if (c >= 'a' && c <= 'z')
		    i -= 'a' - 'A';
		buf[j++] = c - '@';
	    }
	} else if (c == '\\')
	    buf[j++] = buf[++i];
	else
	    buf[j++] = c;
    }
    if (c != term)
	return NULL;
    *ip = i + 1;
    *lenp = j;
    seq = malloc(j + 1);
    bcopy(buf, seq, j);
    seq[j] = '\0';
    return seq;
}

int
bscount(s, len)
    char *s;
    int len;
{
    int n = 0;
    char c;

    while (--len >= 0) {
	c = *s++;
	if (has_bs(c))
	    n++;
    }
    return n;
}

void
mark_controls(s, len)
    char *s;
    int len;
{
    Bool in_seq = False;

    for ( ; --len >= 0; s++) {
	if (*s != control_char)
	    continue;
	if (in_seq)
	    *s = control_end;
	else
	    switch (s[1]) {
	    case '\003': /* control c */
	    case '\004': /* control d */
	    case '\007':
	    case '\015': /* control m */
	    case '\021': /* control q */
	    case '\023': /* control s */
		break;
	    default:
		in_seq = True;
	    }
    }
}

void
get_undofile(undofile)
    char *undofile;
{
    FILE *fp;
    char buf[1024];
    int i, len;
    undo *up;
    int idx;

    fp = fopen(undofile, "r");
    if (!fp)
	return;
    up = (undo *)malloc(sizeof(undo));
    idx = 0;
    while (fgets(buf, sizeof(buf), fp)) {
	if (buf[0] == '\n' || buf[0] == '!')
	    continue;
	i = 0;
	if (!(up[idx].seq = parse_string(buf, &i, &up[idx].seq_len, ':')) ||
	    !(up[idx].undo = parse_string(buf, &i, &up[idx].undo_len, '\n'))) {
	    fprintf(stderr, "bad sequence, line %d\n", idx + 1);
	    continue;
	}
	mark_controls(up[idx].seq, up[idx].seq_len);
	up[idx].bscount = bscount(up[idx].seq, up[idx].seq_len);
	if (!up[idx].bscount) {
	    fprintf(stderr, "bad sequence, no bs count, line %d\n", idx + 1);
	    continue;
	}
	idx++;
	up = (undo *)realloc((char *)up, (idx + 1) * sizeof(undo));
    }
    up[idx].bscount = 0;
    fclose(fp);
    undos = up;
}

void
debug_state()
{
    int i, max;

    fprintf(stderr, "in_control_seq: %d\n", in_control_seq);
    fprintf(stderr, "bscount: %d\n", curbscount);
    fprintf(stderr, "history: ");
    max = sizeof(history) - 1;
    while (max >= history_end && !history[max])
	max--;
    if (max > history_end + 20)
	max = history_end + 20;
    for (i = history_end - 70; i <= max; i++) {
	if (i < 0)
	    continue;
	if (i >= sizeof(history))
	    break;
	if (i == history_end)
	    fprintf(stderr, "\ndeleted: ");
	if (history[i] == '\177')
	    fprintf(stderr, "^?");
	else if (history[i] == control_end)
	    fprintf(stderr, "^T");
	else if (iscntrl(history[i]))
	    fprintf(stderr, "^%c", history[i] + '@');
	else
	    fprintf(stderr, "%c", history[i]);
    }
    fprintf(stderr, "\n");
    in_control_seq = False;
}

void
process(buf, n, len)
    char *buf;
    int n;
    int len;
{
    int i, j;
    KeySym sym;
    char *endptr;

    for (i = 0; i < n; i++) {
	if (len) {
	    if (buf[i] == '\b') {
		do_backspace(buf[i]);
		continue;
	    } else if (curbscount) {
		while (curbscount)
		    undo_stroke();
	    } else if (in_control_seq) {
		fprintf(stderr, "still in control seq\n");
		debug_state();
	    }
	}
	if (buf[i] != control_char) {
	    if (len) {
		if (history_end == sizeof(history))
		    trim_history();
		history[history_end++] = buf[i];
	    }
	    do_char(((unsigned char *)buf)[i]);
	    continue;
	}
	i++;
	for (j = i; 1; j++) {
	    if (j == n) {
		if (!len)
		    return;
		if (n == len)
		    break;
		n = read(0, buf+j, len-j);
		if (n < 0)
		    quit(0);
		n += j;
	    }
	    if (buf[j] != control_char) {
		if (j != i)
		    continue;
		switch (buf[j]) {
		case '\003': /* control c */
		    do_key(control, 0);
		    break;
		case '\004': /* control d */
		    slow_down();
		    break;
		case '\007':
		    start_moving();
		    break;
		case '\015': /* control m */
		    do_key(meta, 0);
		    break;
		case '\021': /* control q */
		    stop_moving();
		    break;
		case '\023': /* control s */
		    do_key(shift, 0);
		    break;
		default:
		    continue;
		}
		if (len)
		    save_unit(buf, i - 1, j);
		break;
	    }
	    buf[j] = '\0';
	    if (j == i)
		do_char(control_char);
	    else if (buf[i] == '\002') /* control b */
		do_button(atoi(buf+i+1));
	    else if (buf[i] == '\027') { /* control w */
		int screen = strtol(buf+i+1, &endptr, 10);
		int x;
		if (*endptr) {
		    x = strtol(endptr + 1, &endptr, 10);
		    if (*endptr)
			do_warp(screen, x, atoi(endptr + 1));
		}
	    } else if (buf[i] == '\030') /* control x */
		do_x(atoi(buf+i+1));
	    else if (buf[i] == '\031') /* control y */
		do_y(atoi(buf+i+1));
	    else if (!strcmp(buf+i, "exit"))
		quit(0);
	    else if (!strcmp(buf+i, "dbg"))
		debug_state();
	    else if ((sym = strtoul(buf+i, &endptr, 16)) && !*endptr)
		do_keysym(sym);
	    else if (sym = XStringToKeysym(buf+i))
		do_keysym(sym);
	    if (len) {
		buf[j] = control_char;
		save_unit(buf, i - 1, j);
	    }
	    i = j;
	    break;
	}
    }
}

main(argc, argv)
    int argc;
    char **argv;
{
    int n, i, j;
    int eventb, errorb, vmajor, vminor;
    struct termios term;
    Bool noecho = True;
    char *dname = NULL;
    char buf[1024];
    XEvent ev;
    int mask[10];
    struct timeval timeout;
    char *undofile = NULL;

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000 * 100;
    bzero((char *)mask, sizeof(mask));
    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (argv[0][0] != '-')
	    usage();
	switch (argv[0][1]) {
	case 'd':
	    argc--; argv++;
	    if (!argc)
		usage();
	    dname = *argv;
	    break;
	case 'e':
	    noecho = False;
	    break;
	case 'b':
	    bs_is_del = False;
	    break;
	case 'u':
	    argc--; argv++;
	    if (!argc)
		usage();
	    undofile = *argv;
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
    if (!undofile) {
	strcpy(buf, getenv("HOME"));
	strcat(buf, "/.a2x");
	undofile = buf;
    }
    get_undofile(undofile);
    signal(SIGPIPE, SIG_IGN);
    if (tcgetattr(0, &term) >= 0) {
	istty = True;
	oldterm = term;
	term.c_lflag &= ~(ICANON|ISIG);
	term.c_iflag &= ~(IXOFF|IXON|ICRNL);
	if (noecho)
	    term.c_lflag &= ~ECHO;
#ifdef _POSIX_VDISABLE
	for (i = 0; i < NCCS; i++)
	    term.c_cc[i] = _POSIX_VDISABLE;
#else
	bzero((char *)term.c_cc, sizeof(term.c_cc));
#endif
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &term);
	signal(SIGINT, catch);
	signal(SIGTERM, catch);
	oldioerror = XSetIOErrorHandler(ioerror);
	olderror = XSetErrorHandler(error);
    }
    reset_mapping(dpy);
    while (1) {
	XFlush(dpy);
	mask[0] = 1 | (1 << ConnectionNumber(dpy));
	i = select(ConnectionNumber(dpy)+1, mask, NULL, NULL,
		   ((moving_x | moving_y) ? &timeout : NULL));
	if (i < 0)
	    quit(1);
	if (!i) {
	    if (moving_x | moving_y) {
		move_pointer(moving_x, moving_y);
		XFlush(dpy);
	    }
	    continue;
	}
	if (mask[0] & (1 << ConnectionNumber(dpy))) {
	    for (i = XEventsQueued(dpy, QueuedAfterReading); --i >= 0; ) {
		XNextEvent(dpy, &ev);
		if (ev.type == MappingNotify) {
		    XRefreshKeyboardMapping(&ev.xmapping);
		    reset_mapping(dpy);
		}
	    }
	}
	if (!(mask[0] & 1))
	    continue;
	n = read(0, buf, 1 /* sizeof(buf) */);
	if (n <= 0)
	    quit(0);
	process(buf, n, sizeof(buf));
	quiesce();
    }
}
