/* $XConsortium: a2x.c,v 1.35 92/04/03 10:07:36 rws Exp $ */
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

^T^A<kdelay> <delta> <mdelay>^T
			autorepeat every <kdelay> seconds for keys
			else change non-zero motion dx, dy to <delta>
			and autorepeat every <mdelay> seconds for motion
			<kdelay> and <mdelay> are floating point
			zero for any value means don't change it
^T^B<button>^T		toggle button <button> state (press or release)
^T^C			set Control key for next character
^T^D<dx> <dy>^T		move mouse by (<dx>, <dy>) pixels
^T^E			exit the program
^T^J<options> [mult]^T	jump to next closest top-level window
	C		closest top-level window
	D		top-level window going down
	L		top-level window going left
	R		top-level window going right
	U		top-level window going up
	c		closest widget
	d		top-level widget going down
	l		top-level widget going left
	r		top-level widget going right
	u		top-level widget going up
	k		require windows that select for key events
	b		require windows that select for button events
	[mult]		off-axis distance multiplier
^T^M			set Meta key for next character
^T^P			print debugging info
^T^Q			quit moving (mouse or key)
^T^S			set Shift key for next character
^T^T			^T
^T^U			re-read undo file
^T^W<screen> <x> <y>^T	warp to position (<x>,<y>) on screen <screen>
			(screen can be -1 for current)
^T<hexstring>^T		press and release key with numeric keysym <hexstring>
			F<char> and F<char><char> are names, not numbers
^T<keysym>^T		press and release key with keysym named <keysym>

Note: if key is attached to a modifier, pressing it is temporary, will be
released automatically at next button or non-modifier key.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h> /* Sun needs it */
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
Bool button_state[256];
unsigned short modifiers[256];
KeyCode keycodes[256];
unsigned short modmask[256];
unsigned short curmods = 0;
unsigned short tempmods = 0;
KeyCode shift, control, mod1, mod2, mod3, mod4, mod5, meta;
Bool bs_is_del = True;
KeySym last_sym = 0;
KeyCode last_keycode_for_sym = 0;
struct termios oldterm;
Bool istty = False;
struct timeval timeout;
struct timeval *moving_timeout = NULL;
int moving_x = 0;
int moving_y = 0;
int last_keycode = 0;
unsigned short last_mods = 0;
int (*olderror)();
int (*oldioerror)();
char history[4096];
int history_end = 0;
char *undofile = NULL;
typedef struct _undo {
    struct _undo *next;
    int bscount;
    char *seq;
    int seq_len;
    char *undo;
    int undo_len;
} undo;
undo *undos[256];
int curbscount = 0;
Bool in_control_seq = False;

void process();

/* To generate events another way, change the next four functions. */

void
generate_key(key, press)
    int key;
    Bool press;
{
    XTestFakeKeyEvent(dpy, key, press, 0);
}

void
generate_button(button, press)
    int button;
    Bool press;
{
    XTestFakeButtonEvent(dpy, button, press, 0);
}

void
generate_motion(dx, dy)
    int dx, dy;
{
    XTestFakeRelativeMotionEvent(dpy, dx, dy, 0);
}

void
generate_warp(screen, x, y)
    int screen, x, y;
{
    XTestFakeMotionEvent(dpy, screen, x, y, 0);
}

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

/*ARGSUSED*/
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
reflect_modifiers(mods)
    unsigned int mods;
{
    unsigned int downmods = mods & ~curmods;
    unsigned int upmods = curmods & ~mods;

    if (upmods) {
	if (upmods & ShiftMask)
	    generate_key(shift, False);
	if (upmods & ControlMask)
	    generate_key(control, False);
	if (upmods & Mod1Mask)
	    generate_key(mod1, False);
	if (upmods & Mod2Mask)
	    generate_key(mod2, False);
	if (upmods & Mod3Mask)
	    generate_key(mod3, False);
	if (upmods & Mod4Mask)
	    generate_key(mod4, False);
	if (upmods & Mod5Mask)
	    generate_key(mod5, False);
    }
    if (downmods) {
	if (downmods & ShiftMask)
	    generate_key(shift, True);
	if (downmods & ControlMask)
	    generate_key(control, True);
	if (downmods & Mod1Mask)
	    generate_key(mod1, True);
	if (downmods & Mod2Mask)
	    generate_key(mod2, True);
	if (downmods & Mod3Mask)
	    generate_key(mod3, True);
	if (downmods & Mod4Mask)
	    generate_key(mod4, True);
	if (downmods & Mod5Mask)
	    generate_key(mod5, True);
    }
    curmods = mods;
}

void
do_key(key, mods)
    int key;
    unsigned short mods;
{

    if (!key)
	return;
    if (modmask[key]) {
	tempmods |= modmask[key];
	return;
    }
    reflect_modifiers(tempmods | mods);
    generate_key(key, True);
    generate_key(key, False);
    moving_timeout = NULL;
    last_keycode = key;
    last_mods = tempmods | mods;
    tempmods = 0;
}

void
do_char(c)
    unsigned char c;
{
    do_key(keycodes[c], modifiers[c]);
}

KeyCode
parse_keysym(buf, len)
    char *buf;
    int len;
{
    KeySym sym;
    char *endptr;

    if ((*buf == 'F' && len <= 3) ||
	!(sym = strtoul(buf, &endptr, 16)) || *endptr)
	sym = XStringToKeysym(buf);
    if (!sym)
	return 0;
    if (sym != last_sym) {
	last_sym = sym;
	last_keycode_for_sym = XKeysymToKeycode(dpy, sym);
    }
    return last_keycode_for_sym;
}

void
do_keysym(buf, len)
    char *buf;
    int len;
{
    KeyCode key;

    key = parse_keysym(buf, len);
    if (key)
	do_key(key, 0);
}

void
do_button(button)
    int button;
{
    if (button < 1 || button > 255)
	return;
    button = button_map[button];
    if (button) {
	reflect_modifiers(tempmods);
	button_state[button] = !button_state[button];
	generate_button(button, button_state[button]);
	tempmods = 0;
    }
}

void
do_autorepeat(buf)
    char *buf;
{
    double delay, mdelay;
    int delta;
    char *endptr;

    delay = strtod(buf, &endptr);
    if (*endptr) {
	delta = strtol(endptr + 1, &endptr, 10);
	if (*endptr) {
	    mdelay = atof(endptr + 1);
	    if (!last_keycode) {
		delay = mdelay;
		if (delta) {
		    if (moving_x < 0)
			moving_x = -delta;
		    else if (moving_x > 0)
			moving_x = delta;
			if (moving_y < 0)
			moving_y = -delta;
		    else if (moving_y > 0)
			moving_y = delta;
		}
	    }
	    if (delay) {
		timeout.tv_sec = delay;
		timeout.tv_usec = (delay - timeout.tv_sec) * 1000000;
		moving_timeout = &timeout;
	    }
	}
    }
}

void
do_motion(buf)
    char *buf;
{
    int dx, dy;
    char *endptr;

    dx = strtol(buf, &endptr, 10);
    if (*endptr) {
	dy = atoi(endptr + 1);
	if (!moving_timeout)
	    generate_motion(dx, dy);
	else if (last_keycode)
	    moving_timeout = NULL;
	moving_x = dx;
	moving_y = dy;
	last_keycode = 0;
    }
}

void
do_warp(buf)
    char *buf;
{
    int screen;
    int x;
    char *endptr;

    screen = strtol(buf, &endptr, 10);
    if (*endptr) {
	x = strtol(endptr + 1, &endptr, 10);
	if (*endptr)
	    generate_warp(screen, x, atoi(endptr + 1));
    }
}

Bool
find_closest(dir, mult, rootx, rooty, parent, pwa, input,
	     bestx, besty, best_dist, recurse)
    char dir;
    double mult;
    int rootx, rooty;
    Window parent;
    XWindowAttributes *pwa;
    Mask input;
    int *bestx, *besty;
    double *best_dist;
    Bool recurse;
{
    Window *children;
    unsigned int nchild;
    XWindowAttributes wa;
    int i;
    Bool found = False;
    double dist;
    int x1, y1, x2, y2, x, y;
    double xmult = 1.0;
    double ymult = 1.0;

    XQueryTree(dpy, parent, &wa.root, &wa.root, &children, &nchild);
    for (i = 0; i < nchild; i++) {
	if (!XGetWindowAttributes(dpy, children[i], &wa))
	    continue;
	if (wa.map_state != IsViewable)
	    continue;
	if (input && !(wa.all_event_masks & input))
	    continue;
	x1 = wa.x;
	y1 = wa.y;
	x2 = x1 + wa.width + (2 * wa.border_width);
	y2 = y1 + wa.height + (2 * wa.border_width);
	if (x1 >= pwa->width || x2 <= 0 || y1 >= pwa->height || y2 <= 0)
	    continue;
	x1 += pwa->x;
	y1 += pwa->y;
	wa.x = x1;
	wa.y = y1;
	x2 += pwa->x;
	y2 += pwa->y;
	if (recurse &&
	    find_closest(dir, mult, rootx, rooty, children[i], &wa, input,
	    		 bestx, besty, best_dist, recurse)) {
	    found = True;
	    continue;
	}
	if (rootx >= x1 && rootx < x2 && rooty >= y1 && rooty < y2)
	    continue;
	switch (dir) {
	case 'U':
	    if (y2 > rooty)
		continue;
	    if (x2 < rootx)
		x = x2;
	    else if (x1 > rootx)
		x = x1;
	    else
		x = rootx;
	    xmult = mult;
	    y = y2;
	    break;
	case 'D':
	    if (y1 < rooty)
		continue;
	    if (x2 < rootx)
		x = x2;
	    else if (x1 > rootx)
		x = x1;
	    else
		x = rootx;
	    xmult = mult;
	    y = y1;
	    break;
	case 'R':
	    if (x1 < rootx)
	    	continue;
	    if (y2 < rooty)
		y = y2;
	    else if (y1 > rooty)
		y = y1;
	    else
		y = rooty;
	    ymult = mult;
	    x = x1;
	    break;
	case 'L':
	    if (x2 > rootx)
		continue;
	    if (y2 < rooty)
		y = y2;
	    else if (y1 > rooty)
		y = y1;
	    else
		y = rooty;
	    ymult = mult;
	    x = x2;
	    break;
	}
	dist = (xmult * (x - rootx) * (x - rootx) +
		ymult * (y - rooty) * (y - rooty));
	if (dist > *best_dist)
	    continue;
	*bestx = (x1 + x2) / 2;
	*besty = (y1 + y2) / 2;
	*best_dist = dist;
	found = True;
    }
    if (children)
	XFree((XPointer)children);
    return found;
}

void
do_jump(buf)
    char *buf;
{
    Window root, child;
    int rootx, rooty;
    XWindowAttributes wa;
    int bestx, besty;
    double best_dist;
    int screen;
    char dir;
    Bool widget = False;
    Mask input = 0;
    double mult = 10.0;
    char *endptr;

    for (; *buf; buf++) {
	switch (*buf) {
	case 'C':
	case 'D':
	case 'U':
	case 'L':
	case 'R':
	    dir = *buf;
	    widget = False;
	    break;
	case 'c':
	case 'd':
	case 'u':
	case 'l':
	case 'r':
	    dir = *buf - 'a' + 'A';
	    widget = True;
	    break;
	case 'k':
	    input |= KeyPressMask|KeyReleaseMask;
	    break;
	case 'b':
	    input |= ButtonPressMask|ButtonReleaseMask;
	    break;
	case ' ':
	    mult = strtod(buf+1, &endptr);
	    if (*endptr)
		return;
	    buf = endptr - 1;
	    break;
	default:
	    return;
	}
    }
    XQueryPointer(dpy, DefaultRootWindow(dpy), &root, &child, &rootx, &rooty,
		  &bestx, &besty, (unsigned int *)&screen);
    for (screen = 0; RootWindow(dpy, screen) != root; screen++)
	;
    if (widget && child) {
	XGetWindowAttributes(dpy, child, &wa);
	root = child;
    } else {
	wa.x = 0;
	wa.y = 0;
	wa.width = WidthOfScreen(ScreenOfDisplay(dpy, screen));
	wa.height = HeightOfScreen(ScreenOfDisplay(dpy, screen));
    }
    best_dist = 4e9;
    if (find_closest(dir, mult, rootx, rooty, root, &wa, input,
		     &bestx, &besty, &best_dist, widget))
	generate_warp(screen, bestx, besty);
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
save_control(buf, i, j)
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
    KeyCode key;

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
    if (!in_control_seq) {
	if (c == control_end)
	    in_control_seq = True;
	else if ((history_end > 1) &&
		 (history[history_end-2] == control_char)) {
	    in_control_seq = True;
	    switch (c) {
	    case '\003': /* control c */
		if (control)
		    tempmods &= ~modmask[control];
		break;
	    case '\015': /* control m */
		if (meta)
		    tempmods &= ~modmask[meta];
		break;
	    case '\023': /* control s */
		if (shift)
		    tempmods &= ~modmask[shift];
		break;
	    }
	}
    }
    if (has_bs(c)) {
	curbscount--;
	if (!in_control_seq) {
	    if ((history_end < 3) ||
		(history[history_end-2] != '\015' &&
		 history[history_end-2] != '\003') ||
		(history[history_end-3] != control_char))
		do_char('\b');
	}
    }
    history_end--;
    if (in_control_seq) {
	for (i = history_end - 1; i >= 0; i--) {
	    c = history[i];
	    if (c == control_char) {
		if (tempmods && !iscntrl(history[i+1])) {
		    history[history_end] = '\0';
		    key = parse_keysym(history+i+1, history_end - i - 1);
		    if (key)
			tempmods &= ~modmask[key];
		    history[history_end] = control_end;
		}
		history_end = i;
		in_control_seq = False;
		break;
	    } else if (has_bs(c))
		break;
	}
    }
}

void
do_backspace()
{
    undo *u;
    Bool partial = False;

    curbscount++;
    while (curbscount) {
	if (!in_control_seq && history_end) {
	    for (u = undos[((unsigned char *)history)[history_end-1]];
		 u;
		 u = u->next) {
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
		    c -= 'a' - 'A';
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
	    case '\005': /* control e */
	    case '\015': /* control m */
	    case '\020': /* control p */
	    case '\021': /* control q */
	    case '\023': /* control s */
	    case '\025': /* control u */
		break;
	    default:
		in_seq = True;
	    }
    }
}

void
free_undo(up)
    undo *up;
{
    free(up->seq);
    free(up->undo);
    free((char *)up);
}

void
get_undofile()
{
    FILE *fp;
    char buf[1024];
    int i;
    undo *up, **upp;
    int line;

    fp = fopen(undofile, "r");
    if (!fp)
	return;
    for (i = 0; i < sizeof(undos); i++) {
	while (up = undos[i]) {
	    undos[i] = up->next;
	    free_undo(up);
	}
    }
    line = 1;
    for (; fgets(buf, sizeof(buf), fp); line++) {
	if (buf[0] == '\n' || buf[0] == '!')
	    continue;
	up = (undo *)malloc(sizeof(undo));
	up->seq = NULL;
	up->undo = NULL;
	i = 0;
	if (!(up->seq = parse_string(buf, &i, &up->seq_len, ':')) ||
	    !(up->undo = parse_string(buf, &i, &up->undo_len, '\n'))) {
	    fprintf(stderr, "bad sequence, line %d\n", line);
	    free_undo(up);
	    continue;
	}
	mark_controls(up->seq, up->seq_len);
	up->bscount = bscount(up->seq, up->seq_len);
	if (!up->bscount) {
	    free_undo(up);
	    fprintf(stderr, "bad sequence, no bs count, line %d\n", line);
	    continue;
	}
	i = ((unsigned char *)up->seq)[up->seq_len - 1];
	for (upp = &undos[i]; *upp; upp = &(*upp)->next)
	    ;
	*upp = up;
	up->next = NULL;
    }
    fclose(fp);
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

    for (i = 0; i < n; i++) {
	if (len) {
	    if (buf[i] == '\b') {
		do_backspace();
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
		case '\005': /* control e */
		    quit(0);
		    break;
		case '\015': /* control m */
		    do_key(meta, 0);
		    break;
		case '\020': /* control p */
		    debug_state();
		    break;
		case '\021': /* control q */
		    moving_timeout = NULL;
		    break;
		case '\023': /* control s */
		    do_key(shift, 0);
		    break;
		case '\025': /* control u */
		    get_undofile();
		    break;
		default:
		    continue;
		}
		if (len)
		    save_control(buf, i - 1, j);
		break;
	    }
	    buf[j] = '\0';
	    switch (buf[i]) {
	    case '\0': /* control t */
		do_char(control_char);
		break;
	    case '\001': /* control a */
		do_autorepeat(buf + i + 1);
		break;
	    case '\002': /* control b */
		do_button(atoi(buf + i + 1));
		break;
	    case '\004': /* control d */
		do_motion(buf + i + 1);
		break;
	    case '\012': /* control j */
	    	do_jump(buf + i + 1);
		break;
	    case '\027': /* control w */
		do_warp(buf + i + 1);
		break;
	    default:
		do_keysym(buf + i, j - i);
	    }
	    if (len) {
		buf[j] = control_char;
		save_control(buf, i - 1, j);
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
    int n, i;
    int eventb, errorb, vmajor, vminor;
    struct termios term;
    Bool noecho = True;
    char *dname = NULL;
    char buf[1024];
    char fbuf[1024];
    XEvent ev;
    int maxfd;
    int Xmask;
    int mask[10];

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
	strcpy(fbuf, getenv("HOME"));
	strcat(fbuf, "/.a2x");
	undofile = fbuf;
    }
    get_undofile();
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
    reset_mapping();
    Xmask = 1 << ConnectionNumber(dpy);
    maxfd = ConnectionNumber(dpy) + 1;
    while (1) {
	XFlush(dpy);
	mask[0] = 1 | Xmask;
	i = select(maxfd, mask, NULL, NULL, moving_timeout);
	if (i < 0)
	    quit(1);
	if (!i) {
	    if (moving_timeout) {
		if (last_keycode)
		    do_key(last_keycode, last_mods);
		else
		    generate_motion(moving_x, moving_y);
		moving_timeout = &timeout;
		XFlush(dpy);
	    }
	    continue;
	}
	if (mask[0] & Xmask) {
	    for (i = XEventsQueued(dpy, QueuedAfterReading); --i >= 0; ) {
		XNextEvent(dpy, &ev);
		if (ev.type == MappingNotify) {
		    XRefreshKeyboardMapping(&ev.xmapping);
		    reset_mapping();
		}
	    }
	}
	if (!(mask[0] & 1))
	    continue;
	n = read(0, buf, sizeof(buf));
	if (n <= 0)
	    quit(0);
	process(buf, n, sizeof(buf));
	reflect_modifiers(0);
    }
}
