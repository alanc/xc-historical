/* $XConsortium: a2x.c,v 1.77 92/07/03 12:18:57 rws Exp $ */
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
^T^B0^T			release all pressed buttons
^T^C			set Control key for next character
^T^D<dx> <dy>^T		move mouse by (<dx>, <dy>) pixels
^T^E			exit the program
^T^F<options>^T
	r		start recording macro
	a		abort recording of macro
	s<digit>	save recording as macro <digit>
	e<digit>	execute macro <digit>
	d<digit>	delete macro <digit>
^T^J<options>[ <mult>]^T
			jump to next closest top-level window
	Z		no-op letter to soak up uppercase from prev word 
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
			(with b, means "key or button")
	b		require windows that select for button events
			(with k, means "key or button")			
	n<name>		require window with given name (WM_NAME)
			this must be the last option
	p<prefix>	require window with given name prefix (WM_NAME)
			this must be the last option
	N[<name>.<class>] require window with given name and/or class
			(WM_CLASS or _MIT_OBJ_CLASS)
			this must be the last option
			either <name> or <class> can be empty
	P[<name>.<class>] require window with given name and/or class prefix
			(WM_CLASS or _MIT_OBJ_CLASS)
			this must be the last option
			either <name> or <class> can be empty
	[ <mult>]	off-axis distance multiplier is <mult> (float)
^T^L<options>^T
	s<digit>	save current pointer position as location <digit>
	w<digit>	warp pointer to location <digit>
^T^M			set Meta key for next character
^T^P			print debugging info
^T^Q			quit moving (mouse or key)
^T^RD<display>^T	switch to a new display
			:0 added if <display> contains no colon
^T^S			set Shift key for next character
^T^T			^T
^T^U			re-read undo file
^T^W<screen> <x> <y>^T	warp to position (<x>,<y>) on screen <screen>
			(screen can be -1 for current)
^T^Y<options>[ <delay>]^T
			set/await trigger
	M		set MapNotify trigger
	U		set UnmapNotify trigger
	W		wait for trigger
	n<name>		require window with given name (WM_NAME)
			this must be the last option
	p<prefix>	require window with given name prefix (WM_NAME)
			this must be the last option
	N[<name>.<class>] require window with given name and/or class
			(WM_CLASS or _MIT_OBJ_CLASS)
			this must be the last option
			either <name> or <class> can be empty
	P[<name>.<class>] require window with given name and/or class prefix
			(WM_CLASS or _MIT_OBJ_CLASS)
			this must be the last option
			either <name> or <class> can be empty
	[ <delay>]	wait no more than <delay> seconds
			<delay> is floating-point
^T^Z<delay>^T		add <delay>-second delay to next event
			<delay> is floating-point
^T<hexstring>^T		press and release key with numeric keysym <hexstring>
			F<char> and F<char><char> are names, not numbers
^T<keysym>^T		press and release key with keysym named <keysym>

Note: if key is attached to a modifier, pressing it is temporary, will be
released automatically at next button or non-modifier key.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#ifdef XTEST
#include <X11/extensions/XTest.h>
#endif
#ifdef XTRAP
#include <X11/extensions/xtraplib.h>
#include <X11/extensions/xtraplibp.h>
#endif
#ifdef XTESTEXT1
#include <X11/Xmd.h>
#include <X11/extensions/xtestext1.h>
#endif
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/Xmu/WinUtil.h>
#include <ctype.h>
#include <termios.h>
#define _POSIX_SOURCE
#include <signal.h>

#define control_char '\024' /* control T */
#define control_end '\224'

typedef enum
{MatchNone, MatchName, MatchNamePrefix, MatchClass, MatchClassPrefix}
MatchType;

typedef struct {
    MatchType match;
    int namelen;
    int classlen;
    char name[100];
    char class[100];
} MatchRec;

typedef struct {
    char dir;
    double mult;
    Screen *screen;
    int rootx, rooty;
    Mask input;
    Window best;
    int bestx, besty;
    double best_dist;
    Bool recurse;
    MatchRec match;
    Region overlap;
} JumpRec;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} Box;

typedef struct {
    Window root;
    int type;
    unsigned long serial;
    struct timeval time;
    MatchRec match;
    int count;
    Window *windows;
} TriggerRec;

typedef struct _undo {
    struct _undo *next;
    int bscount;
    char *seq;
    int seq_len;
    char *undo;
    int undo_len;
} UndoRec;

typedef struct _macro {
    char *macro;
    int len;
} MacroRec;

typedef struct _location {
    Window window;
    int x, y;
} LocationRec;

char *progname;
Display *dpy;
Atom MIT_OBJ_CLASS;
int maxfd;
int Xmask;
int fdmask[10];
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
unsigned long time_delay;
int (*olderror)();
int (*oldioerror)();
char history[8192];
int history_end = 0;
int macro_start = -1;
char *undofile = NULL;
#define UNDO_SIZE 256
UndoRec *undos[UNDO_SIZE];
int curbscount = 0;
UndoRec *curbsmatch = NULL;
Bool in_control_seq = False;
Bool skip_next_control_char = False;
TriggerRec trigger;
JumpRec jump;
MacroRec macros[10];
LocationRec locations[10];
#ifdef XTRAP
XETC *tc;
#endif

void process();

#if defined(XTRAP) || defined(XTESTEXT1)
void
delay_time() /* we have to approximate the delay */
{
    struct timeval delay;

    XFlush(dpy);
    delay.tv_sec = time_delay / 1000;
    delay.tv_usec = (time_delay % 1000) * 1000;
    select(0, NULL, NULL, NULL, &delay);
}
#endif

void
generate_key(key, press)
    int key;
    Bool press;
{
#ifdef XTEST
    XTestFakeKeyEvent(dpy, key, press, time_delay);
#endif
#ifdef XTRAP
    delay_time();
    XESimulateXEventRequest(tc, press ? KeyPress : KeyRelease, key, 0, 0, 0);
#endif
#ifdef XTESTEXT1
    XTestPressKey(dpy, 1, time_delay, key, press ? XTestPRESS : XTestRELEASE);
#endif
    time_delay = 0;
}

void
generate_button(button, press)
    int button;
    Bool press;
{
#ifdef XTEST
    XTestFakeButtonEvent(dpy, button, press, time_delay);
#endif
#ifdef XTRAP
    delay_time();
    XESimulateXEventRequest(tc, press ? ButtonPress : ButtonRelease,
			    button, 0, 0, 0);
#endif
#ifdef XTESTEXT1
    XTestPressButton(dpy, 2, time_delay, button,
		     press ? XTestPRESS : XTestRELEASE);
#endif
    time_delay = 0;
}

void
generate_motion(dx, dy)
    int dx, dy;
{
#ifdef XTEST
    XTestFakeRelativeMotionEvent(dpy, dx, dy, time_delay);
#endif
#if defined(XTRAP) || defined(XTESTEXT1)
    delay_time();
    XWarpPointer(dpy, None, None, 0, 0, 0, 0, dx, dy);
#endif
    time_delay = 0;
}

void
generate_warp(screen, x, y)
    int screen, x, y;
{
#ifdef XTEST
    XTestFakeMotionEvent(dpy, screen, x, y, time_delay);
#endif
#ifdef XTRAP
    delay_time();
    XESimulateXEventRequest(tc, MotionNotify, 0, x, y, 0);
#endif
#ifdef XTESTEXT1
    delay_time();
    XWarpPointer(dpy, None, DefaultRootWindow(dpy), 0, 0, 0, 0, x, y);
#endif
    time_delay = 0;
}

void
usage()
{
    printf("%s: [-d display] [-e] [-b] [-u <undofile>]\n", progname);
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
    if (err->error_code == BadWindow || err->error_code == BadDrawable)
	return 0;
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
	!(sym = strtol(buf, &endptr, 16)) || *endptr)
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
    if (!button) {
	for (button = 1; button < 256; button++) {
	    if (button_state[button])
		do_button(button);
	}
	return;
    }
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

void
get_region(reg, w, wa, getshape)
    Region reg;
    Window w;
    XWindowAttributes *wa;
    Bool getshape;
{
    XRectangle rect;
    XRectangle *rects;
    int n;
    int order;

    if (getshape &&
	(rects = XShapeGetRectangles(dpy, w, ShapeBounding, &n, &order))) {
	while (--n >= 0) {
	    rects[n].x += wa->x;
	    rects[n].y += wa->y;
	    XUnionRectWithRegion(&rects[n], reg, reg);
	}
	XFree((char *)rects);
    } else {
	rect.x = wa->x;
	rect.y = wa->y;
	rect.width = wa->width + (2 * wa->border_width);
	rect.height = wa->height + (2 * wa->border_width);
	XUnionRectWithRegion(&rect, reg, reg);
    }
}

Region
compute_univ(puniv, iuniv, w, wa, level)
    Region puniv;
    Region iuniv;
    Window w;
    XWindowAttributes *wa;
    int level;
{
    Region univ;

    univ = XCreateRegion();
    XIntersectRegion(iuniv, univ, iuniv);
    get_region(iuniv, w, wa, !level);
    if (!level && jump.overlap) {
	XIntersectRegion(iuniv, jump.overlap, univ);
	if (!XEmptyRegion(univ)) {
	    XDestroyRegion(univ);
	    return NULL;
	}
    }
    XIntersectRegion(puniv, iuniv, univ);
    if (XEmptyRegion(univ)) {
	XDestroyRegion(univ);
	return NULL;
    }
    XSubtractRegion(iuniv, univ, iuniv);
    XSubtractRegion(puniv, univ, puniv);
    return univ;
}

void
compute_box(univ, box)
    Region univ;
    Box *box;
{
    XRectangle rect;

    XClipBox(univ, &rect);
    box->x1 = rect.x;
    box->y1 = rect.y;
    box->x2 = rect.x + (int)rect.width;
    box->y2 = rect.y + (int)rect.height;
}

Bool
box_left(univ, iuniv)
    Region univ;
    Region iuniv;
{
    XRectangle rect;

    XUnionRegion(iuniv, univ, iuniv);
    XClipBox(iuniv, &rect);
    return (XRectInRegion(iuniv, rect.x, rect.y, rect.width, rect.height) ==
	    RectangleIn);
}

void
find_closest_point(univ, px, py)
    Region univ;
    int *px;
    int *py;
{
    int x, y;
    int max, i;
    Box box;

    x = *px;
    y = *py;
    if (XPointInRegion(univ, x, y) || XEmptyRegion(univ))
	return;
    compute_box(univ, &box);
    if (box.x2 < x)
	max = x - box.x2;
    else if (box.x1 > x)
	max = box.x1 - x;
    else
	max = 0;
    if (box.y2 < y)
	i = y - box.y2;
    else if (box.y1 > y)
	i = box.y1 - y;
    else
	i = 0;
    if (i > max)
	max = i;
    for (; 1; max++) {
	for (i = 0; i <= max; i++) {
	    if (XPointInRegion(univ, x - max, y - i)) {
		*px -= max;
		*py -= i;
		return;
	    }
	    if (XPointInRegion(univ, x - max, y + i)) {
		*px -= max;
		*py += i;
		return;
	    }
	    if (XPointInRegion(univ, x + max, y - i)) {
		*px += max;
		*py -= i;
		return;
	    }
	    if (XPointInRegion(univ, x + max, y + i)) {
		*px += max;
		*py += i;
		return;
	    }
	    if (XPointInRegion(univ, x - i, y - max)) {
		*px -= i;
		*py -= max;
		return;
	    }
	    if (XPointInRegion(univ, x - i, y + max)) {
		*px -= i;
		*py += max;
		return;
	    }
	    if (XPointInRegion(univ, x + i, y - max)) {
		*px += i;
		*py -= max;
		return;
	    }
	    if (XPointInRegion(univ, x + i, y + max)) {
		*px += i;
		*py += max;
		return;
	    }
	}
    }
}


void
compute_point(univ, wa)
    Region univ;
    XWindowAttributes *wa;
{
    jump.bestx = wa->x + wa->width / 2 + wa->border_width;
    jump.besty = wa->y + wa->height / 2 + wa->border_width;
    find_closest_point(univ, &jump.bestx, &jump.besty);
}

Region
destroy_region(univ)
    Region univ;
{
    if (univ)
	XDestroyRegion(univ);
    return NULL;
}

Bool
match_class(w, rec, prop)
    Window w;
    MatchRec *rec;
    Atom prop;
{
    Atom type;
    int format;
    int i;
    unsigned long len;
    unsigned long left;
    char *data;
    Bool ok;

    ok = False;
    if (XGetWindowProperty(dpy, w, prop, 0L, 10000L, False,
			   XA_STRING, &type, &format, &len, &left,
			   (unsigned char **)&data) == Success) {
	if (format == 8) {
	    i = strlen(data) + 1;
	    if (i > len)
		i = len;
	    if (rec->match == MatchClass) {
		if ((!rec->name[0] || !strcmp(rec->name, data)) &&
		    (!rec->class[0] || !strcmp(rec->class, data + i)))
		    ok = True;
	    } else {
		if ((!rec->name[0] ||
		     !strncmp(rec->name, data, rec->namelen)) &&
		    (!rec->class[0] ||
		     !strncmp(rec->class, data + i, rec->classlen)))
		    ok = True;
	    }
	}
	XFree(data);
    }
    return ok;
}

Bool
matches(w, rec, getcw)
    Window w;
    MatchRec *rec;
    Bool getcw;
{
    Bool ok;
    char *name;

    switch (rec->match) {
    case MatchName:
    case MatchNamePrefix:
	if (getcw)
	    w = XmuClientWindow(dpy, w);
	if (!XFetchName(dpy, w, &name))
	    return False;
	if (rec->match == MatchName)
	    ok = !strcmp(rec->name, name);
	else
	    ok = !strncmp(rec->name, name, rec->namelen);
	XFree(name);
	break;
    case MatchClass:
    case MatchClassPrefix:
	if (getcw)
	    w = XmuClientWindow(dpy, w);
	ok = ((getcw && match_class(w, rec, XA_WM_CLASS)) ||
	      match_class(w, rec, MIT_OBJ_CLASS));
	break;
    default:
	ok = True;
	break;
    }
    return ok;
}

double
compute_best_right(univ, cx, cy)
    Region univ;
    int cx;
    int cy;
{
    int maxy, maxx, x, y, Y, Y2, i;

    cx -= jump.rootx;
    cy -= jump.rooty;
    if (cy < 0)
	cy = -cy;
    maxy = HeightOfScreen(jump.screen) - jump.rooty;
    if (jump.rooty > maxy)
	maxy = jump.rooty;
    i = sqrt(jump.best_dist / jump.mult);
    if (i < maxy)
	maxy = i;
    maxx = WidthOfScreen(jump.screen) - jump.rootx;
    i = sqrt(jump.best_dist);
    if (i < maxx)
	maxx = i;
    if (XRectInRegion(univ, jump.rootx, jump.rooty - maxy,
		      maxx + 1, maxy + maxy + 1) == RectangleOut)
	return -1;
    for (Y = cy; Y <= maxy; Y++) {
	Y2 = Y * Y;
	for (y = cy; y <= Y; y++) {
	    x = sqrt(jump.mult * (Y2 - y*y));
	    if (x < cx)
		break;
	    if (XRectInRegion(univ, jump.rootx, jump.rooty - y,
			      x + 1, y + y + 1) == RectangleOut)
		continue;
	    for (i = 0; i <= x; i++) {
		if (!XPointInRegion(univ, jump.rootx + i, jump.rooty - y) &&
		    !XPointInRegion(univ, jump.rootx + i, jump.rooty + y))
		    continue;
		return i * i + jump.mult * y * y;
	    }
	}
    }
    return -1;
}

double
compute_best_left(univ, cx, cy)
    Region univ;
    int cx;
    int cy;
{
    int maxy, maxx, x, y, Y, Y2, i;

    cx = jump.rootx - cx;
    cy -= jump.rooty;
    if (cy < 0)
	cy = -cy;
    maxy = HeightOfScreen(jump.screen) - jump.rooty;
    if (jump.rooty > maxy)
	maxy = jump.rooty;
    i = sqrt(jump.best_dist / jump.mult);
    if (i < maxy)
	maxy = i;
    maxx = jump.rootx;
    i = sqrt(jump.best_dist);
    if (i < maxx)
	maxx = i;
    if (XRectInRegion(univ, jump.rootx - maxx, jump.rooty - maxy,
		      maxx + 1, maxy + maxy + 1) == RectangleOut)
	return -1;
    for (Y = cy; Y <= maxy; Y++) {
	Y2 = Y * Y;
	for (y = cy; y <= Y; y++) {
	    x = sqrt(jump.mult * (Y2 - y*y));
	    if (x < cx)
		break;
	    if (XRectInRegion(univ, jump.rootx - x, jump.rooty - y,
			      x + 1, y + y + 1) == RectangleOut)
		continue;
	    for (i = 0; i <= x; i++) {
		if (!XPointInRegion(univ, jump.rootx - i, jump.rooty - y) &&
		    !XPointInRegion(univ, jump.rootx - i, jump.rooty + y))
		    continue;
		return i * i + jump.mult * y * y;
	    }
	}
    }
    return -1;
}

double
compute_best_up(univ, cx, cy)
    Region univ;
    int cx;
    int cy;
{
    int maxy, maxx, x, y, X, X2, i;

    cx -= jump.rootx;
    if (cx < 0)
	cx = -cx;
    cy = jump.rooty - cy;
    maxx = WidthOfScreen(jump.screen) - jump.rootx;
    if (jump.rootx > maxx)
	maxx = jump.rootx;
    i = sqrt(jump.best_dist / jump.mult);
    if (i < maxx)
	maxx = i;
    maxy = jump.rooty;
    i = sqrt(jump.best_dist);
    if (i < maxy)
	maxy = i;
    if (XRectInRegion(univ, jump.rootx - maxx, jump.rooty - maxy,
		      maxx + maxx + 1, maxy + 1) == RectangleOut)
	return -1;
    for (X = cx; X <= maxx; X++) {
	X2 = X * X;
	for (x = cx; x <= X; x++) {
	    y = sqrt(jump.mult * (X2 - x*x));
	    if (y < cy)
		break;
	    if (XRectInRegion(univ, jump.rootx - x, jump.rooty - y,
			      x + x + 1, y + 1) == RectangleOut)
		continue;
	    for (i = 0; i <= y; i++) {
		if (!XPointInRegion(univ, jump.rootx - x, jump.rooty - i) &&
		    !XPointInRegion(univ, jump.rootx + x, jump.rooty - i))
		    continue;
		return jump.mult * x * x + i * i;
	    }
	}
    }
    return -1;
}

double
compute_best_down(univ, cx, cy)
    Region univ;
    int cx;
    int cy;
{
    int maxy, maxx, x, y, X, X2, i;

    cx -= jump.rootx;
    if (cx < 0)
	cx = -cx;
    cy -= jump.rooty;
    maxx = WidthOfScreen(jump.screen) - jump.rootx;
    if (jump.rootx > maxx)
	maxx = jump.rootx;
    i = sqrt(jump.best_dist / jump.mult);
    if (i < maxx)
	maxx = i;
    maxy = HeightOfScreen(jump.screen) - jump.rooty;
    i = sqrt(jump.best_dist);
    if (i < maxy)
	maxy = i;
    if (XRectInRegion(univ, jump.rootx - maxx, jump.rooty,
		      maxx + maxx + 1, maxy + 1) == RectangleOut)
	return -1;
    for (X = cx; X <= maxx; X++) {
	X2 = X * X;
	for (x = cx; x <= X; x++) {
	    y = sqrt(jump.mult * (X2 - x*x));
	    if (y < cy)
		break;
	    if (XRectInRegion(univ, jump.rootx - x, jump.rooty,
			      x + x + 1, y + 1) == RectangleOut)
		continue;
	    for (i = 0; i <= y; i++) {
		if (!XPointInRegion(univ, jump.rootx - x, jump.rooty + i) &&
		    !XPointInRegion(univ, jump.rootx + x, jump.rooty + i))
		    continue;
		return jump.mult * x * x + i * i;
	    }
	}
    }
    return -1;
}

double
compute_best_close(univ, cx, cy)
    Region univ;
    int cx;
    int cy;
{
    find_closest_point(univ, &cx, &cy);
    return ((cx - jump.rootx) * (cx - jump.rootx) +
	    (cy - jump.rooty) * (cy - jump.rooty));
}

double
compute_distance(univ)
    Region univ;
{
    Box box;
    int x, y;

    if (XPointInRegion(univ, jump.rootx, jump.rooty))
	return -1;
    compute_box(univ, &box);
    switch (jump.dir) {
    case 'R':
    case 'L':
	if (jump.dir == 'R' && box.x1 >= jump.rootx)
	    x = box.x1;
	else if (jump.dir == 'L' && box.x2 <= jump.rootx)
	    x = box.x2;
	else
	    x = jump.rootx;
	if (box.y2 < jump.rooty)
	    y = box.y2;
	else if (box.y1 > jump.rooty)
	    y = box.y1;
	else
	    y = jump.rooty;
	if (((x - jump.rootx) * (x - jump.rootx) +
	     jump.mult * (y - jump.rooty) * (y - jump.rooty)) >=
	    jump.best_dist)
	    return -1;
	if (jump.dir == 'R')
	    return compute_best_right(univ, x, y);
	return compute_best_left(univ, x, y);
    case 'U':
    case 'D':
	if (box.x2 < jump.rootx)
	    x = box.x2;
	else if (box.x1 > jump.rootx)
	    x = box.x1;
	else
	    x = jump.rootx;
	if (jump.dir == 'U' && box.y2 <= jump.rooty)
	    y = box.y2;
	else if (jump.dir == 'D' && box.y1 > jump.rooty)
	    y = box.y1;
	else
	    y = jump.rooty;
	if ((jump.mult * (x - jump.rootx) * (x - jump.rootx) +
	     (y - jump.rooty) * (y - jump.rooty)) >= jump.best_dist)
	    return -1;
	if (jump.dir == 'U')
	    return compute_best_up(univ, x, y);
	return compute_best_down(univ, x, y);
    default:
	if (box.x2 < jump.rootx)
	    x = box.x2;
	else if (box.x1 > jump.rootx)
	    x = box.x1;
	else
	    x = jump.rootx;
	if (box.y2 < jump.rooty)
	    y = box.y2;
	else if (box.y1 > jump.rooty)
	    y = box.y1;
	else
	    y = jump.rooty;
	if (((x - jump.rootx) * (x - jump.rootx) +
	     (y - jump.rooty) * (y - jump.rooty)) >= jump.best_dist)
	    return -1;
	return compute_best_close(univ, x, y);
    }
}

Bool
find_closest(parent, pwa, puniv, level)
    Window parent;
    XWindowAttributes *pwa;
    Region puniv;
    int level;
{
    Window *children;
    unsigned int nchild;
    Window child;
    XWindowAttributes wa;
    int i;
    Bool found;
    double dist;
    Box box;
    Region iuniv, univ;

    XQueryTree(dpy, parent, &wa.root, &wa.root, &children, &nchild);
    if (!nchild)
	return False;
    found = False;
    iuniv = XCreateRegion();
    univ = NULL;
    for (i = nchild; --i >= 0; univ = destroy_region(univ)) {
	child = children[i];
	if (!XGetWindowAttributes(dpy, child, &wa))
	    continue;
	if (wa.map_state != IsViewable)
	    continue;
	wa.x += pwa->x;
	wa.y += pwa->y;
	univ = compute_univ(puniv, iuniv, child, &wa, level);
	if (!univ)
	    continue;
	compute_box(univ, &box);
	switch (jump.dir) {
	case 'U':
	    if (box.y1 >= jump.rooty)
		continue;
	    break;
	case 'D':
	    if (box.y2 <= jump.rooty)
		continue;
	    break;
	case 'R':
	    if (box.x2 <= jump.rootx)
		continue;
	    break;
	case 'L':
	    if (box.x1 >= jump.rootx)
		continue;
	    break;
	}
	if (jump.recurse &&
	    find_closest(child, &wa, univ, level + 1))
	    found = True;
	if (jump.input && !(wa.all_event_masks & jump.input))
	    continue;
	if (XEmptyRegion(univ))
	    continue;
	if (jump.recurse && !box_left(univ, iuniv))
	    continue;
	if (!matches(child, &jump.match, !level))
	    continue;
	dist = compute_distance(univ);
	if (dist < 0 || dist >= jump.best_dist)
	    continue;
	jump.best = children[i];
	jump.best_dist = dist;
	compute_point(univ, &wa);
	found = True;
    }
    if (children)
	XFree((XPointer)children);
    return found;
}

char *
parse_class(buf, rec)
    char *buf;
    MatchRec *rec;
{
    char *endptr;
    char *cptr;

    cptr = index(buf, '.');
    endptr = index(buf, ' ');
    if (endptr)
	*endptr = '\0';
    if (cptr) {
	bcopy(buf, rec->name, cptr - buf);
	rec->name[cptr - buf + 1] = '\0';
	strcpy(rec->class, cptr + 1);
    } else {
	strcpy(rec->name, buf);
	rec->class[0] = '\0';
    }
    rec->namelen = strlen(rec->name);
    rec->classlen = strlen(rec->class);
    if (endptr) {
	*endptr = ' ';
	buf = endptr - 1;
    } else
	buf += strlen(buf) - 1;
    return buf;
}

char *
parse_name(buf, rec)
    char *buf;
    MatchRec *rec;
{
    char *endptr;

    endptr = index(buf, ' ');
    if (endptr)
	*endptr = '\0';
    strcpy(rec->name, buf);
    rec->namelen = strlen(rec->name);
    if (endptr) {
	*endptr = ' ';
	buf = endptr - 1;
    } else
	buf += strlen(buf) - 1;
    return buf;
}

void
do_jump(buf)
    char *buf;
{
    Window root, child;
    XWindowAttributes wa;
    int screen;
    Region univ;
    XRectangle rect;
    char *endptr;
    Bool overlap;
    unsigned int mask;

    jump.dir = 0;
    jump.mult = 10.0;
    jump.recurse = False;
    jump.input = 0;
    jump.best_dist = 4e9;
    jump.match.match = MatchNone;
    jump.overlap = NULL;
    overlap = False;
    for (; *buf; buf++) {
	switch (*buf) {
	case 'C':
	case 'D':
	case 'U':
	case 'L':
	case 'R':
	    jump.dir = *buf;
	    jump.recurse = False;
	    break;
	case 'c':
	case 'd':
	case 'u':
	case 'l':
	case 'r':
	    jump.dir = *buf - 'a' + 'A';
	    jump.recurse = True;
	    break;
	case 'k':
	    jump.input |= KeyPressMask|KeyReleaseMask;
	    break;
	case 'b':
	    jump.input |= ButtonPressMask|ButtonReleaseMask;
	    break;
	case 'N':
	case 'P':
	    if (*buf == 'N')
		jump.match.match = MatchClass;
	    else
		jump.match.match = MatchClassPrefix;
	    buf = parse_class(buf + 1, &jump.match);
	    break;
	case 'n':
	case 'p':
	    if (*buf == 'n')
		jump.match.match = MatchName;
	    else
		jump.match.match = MatchNamePrefix;
	    buf = parse_name(buf + 1, &jump.match);
	    break;
	case 'O':
	    overlap = True;
	    break;
	case ' ':
	    jump.mult = strtod(buf+1, &endptr);
	    if (*endptr)
		return;
	    buf = endptr - 1;
	    break;
	case 'Z':
	    break;
	default:
	    return;
	}
    }
    root = DefaultRootWindow(dpy);
    while (1) {
	for (screen = 0; RootWindow(dpy, screen) != root; screen++)
	    ;
	if (XQueryPointer(dpy, root, &root, &child,
			  &jump.rootx, &jump.rooty,
			  &jump.bestx, &jump.besty, &mask))
	    break;
    }
    jump.screen = ScreenOfDisplay(dpy, screen);
    if (child && (jump.recurse || overlap))
	XGetWindowAttributes(dpy, child, &wa);
    if (jump.recurse && child) {
	root = child;
    } else {
	if (overlap && child) {
	    jump.overlap = XCreateRegion();
	    get_region(jump.overlap, child, &wa, True);
	}
	wa.x = 0;
	wa.y = 0;
	wa.width = WidthOfScreen(jump.screen);
	wa.height = HeightOfScreen(jump.screen);
    }
    univ = XCreateRegion();
    rect.x = wa.x;
    rect.y = wa.y;
    rect.width = wa.width;
    rect.height = wa.height;
    XUnionRectWithRegion(&rect, univ, univ);
    if (find_closest(root, &wa, univ, 0))
	generate_warp(screen, jump.bestx, jump.besty);
    if (jump.overlap)
	XDestroyRegion(jump.overlap);
    XDestroyRegion(univ);
}

void
unset_trigger()
{
    trigger.type = 0;
    if (trigger.windows) {
	XFree((char *)trigger.windows);
	trigger.windows = NULL;
	trigger.count = 0;
    }
    XSelectInput(dpy, trigger.root, 0L);
}

void
set_unmap_trigger()
{
    Window w, child;
    Window *children;
    unsigned int nchild;
    int i;
    int j;

    XQueryTree(dpy, trigger.root, &w, &child, &children, &nchild);
    if (!nchild) {
	unset_trigger();
	return;
    }
    for (i = nchild, j = 0; --i >= 0; ) {
	w = children[i];
	if (matches(w, &trigger.match, True))
	    children[j++] = w;
    }
    trigger.windows = children;
    trigger.count = j;
}

void
process_events()
{
    int i;
    int j;
    XEvent ev;

    for (i = XEventsQueued(dpy, QueuedAfterReading); --i >= 0; ) {
	XNextEvent(dpy, &ev);
	switch (ev.type) {
	case MappingNotify:
	    XRefreshKeyboardMapping(&ev.xmapping);
	    reset_mapping();
	    break;
	case MapNotify:
	    if (trigger.type == MapNotify &&
		ev.xmap.serial >= trigger.serial &&
		ev.xmap.event == trigger.root &&
		matches(ev.xmap.window, &trigger.match, True))
		unset_trigger();
	    break;
	case UnmapNotify:
	    if (trigger.type == UnmapNotify &&
		ev.xunmap.serial >= trigger.serial &&
		ev.xunmap.event == trigger.root) {
		for (j = 0; j < trigger.count; j++) {
		    if (trigger.windows[j] == ev.xunmap.window) {
			unset_trigger();
			break;
		    }
		}
	    }
	    break;
	}
    }
}

void
do_trigger(buf)
    char *buf;
{
    char *endptr;
    double delay;
    MatchType match;
    Bool wait;
    int type;
    Window child;
    int x, y;

    type = 0;
    match = MatchNone;
    wait = False;
    trigger.time.tv_sec = 10;
    trigger.time.tv_usec = 0;
    for (; *buf; buf++) {
	switch (*buf) {
	case 'M':
	    type = MapNotify;
	    break;
	case 'U':
	    type = UnmapNotify;
	    break;
	case 'W':
	    wait = True;
	    break;
	case 'N':
	case 'P':
	    if (*buf == 'n')
		match = MatchClass;
	    else
		match = MatchClassPrefix;
	    buf = parse_class(buf + 1, &trigger.match);
	    break;
	case 'n':
	case 'p':
	    if (*buf == 'n')
		match = MatchName;
	    else
		match = MatchNamePrefix;
	    buf = parse_name(buf + 1, &trigger.match);
	    break;
	case ' ':
	    delay = strtod(buf+1, &endptr);
	    if (*endptr)
		return;
	    trigger.time.tv_sec = delay;
	    trigger.time.tv_usec = (delay - trigger.time.tv_sec) * 1000000;
	    buf = endptr - 1;
	    break;
	default:
	    return;
	}
    }
    if (type) {
	if (trigger.type)
	    unset_trigger();
	trigger.type = type;
	trigger.match.match = match;
	XQueryPointer(dpy, DefaultRootWindow(dpy), &trigger.root, &child,
		      &x, &y, &x, &y, (unsigned int *)&x);
	trigger.serial = NextRequest(dpy);
	XSelectInput(dpy, trigger.root, SubstructureNotifyMask);
	if (type == UnmapNotify)
	    set_unmap_trigger();
    }
    if (!wait || !trigger.type)
	return;
    while (trigger.type) {
#ifdef XTESTEXT1
	XTestFlush(dpy);
#endif
	if (XPending(dpy)) {
	    process_events();
	    continue;
	}
	fdmask[0] = Xmask;
	type = select(maxfd, fdmask, NULL, NULL, &trigger.time);
	if (type < 0)
	    quit(1);
	if (!type)
	    unset_trigger();
	else
	    process_events();
    }
}

void
trim_history()
{
    if (history_end < (sizeof(history)/2))
	history_end = 0;
    else {
	bcopy(history + (sizeof(history)/2), history, history_end);
	history_end -= sizeof(history)/2;
	if (macro_start >= 0) {
	    macro_start -= sizeof(history)/2;
	    if (macro_start < 0)
		fprintf(stderr, "macro definition overflowed\n");
	}
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
undo_curbsmatch()
{
    if (curbsmatch) {
	history_end -= curbsmatch->seq_len;
	curbscount -= curbsmatch->bscount;
	process(curbsmatch->undo, curbsmatch->undo_len, 0);
	curbsmatch = NULL;
    }
}

void
do_backspace()
{
    UndoRec *u;
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
		    else if (!curbsmatch || curbsmatch->seq_len < u->seq_len)
			curbsmatch = u;
		}
	    }
	    if (partial)
		return;
	    if (curbsmatch) {
		undo_curbsmatch();
		return;
	    }
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
    UndoRec *up;
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
    UndoRec *up, **upp;
    int line;

    fp = fopen(undofile, "r");
    if (!fp)
	return;
    for (i = 0; i < UNDO_SIZE; i++) {
	while (up = undos[i]) {
	    undos[i] = up->next;
	    free_undo(up);
	}
    }
    line = 1;
    for (; fgets(buf, sizeof(buf), fp); line++) {
	if (buf[0] == '\n' || buf[0] == '!')
	    continue;
	up = (UndoRec *)malloc(sizeof(UndoRec));
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

Bool
init_display(dname)
    char *dname;
{
    Display *ndpy;
#ifdef XTEST
    int eventb, errorb, vmajor, vminor;
#endif
#ifdef XTRAP
    XETC *ntc;
#endif
#ifdef XTESTEXT1
    int majop, eventb, errorb;
#endif

    ndpy = XOpenDisplay(dname);
    if (!ndpy) {
	fprintf(stderr, "%s: unable to open display '%s'\n",
		progname, XDisplayName(dname));
	return False;
    }
#ifdef XTEST
    if (!XTestQueryExtension(ndpy, &eventb, &errorb, &vmajor, &vminor)) {
	fprintf(stderr,
		"%s: display '%s' does not support XTEST extension\n",
		progname, DisplayString(ndpy));
	return False;
    }	
#endif
#ifdef XTRAP
    if (!(ntc = XECreateTC(ndpy, 0L, NULL)))
    {
	fprintf(stderr,
		"%s: display '%s' does not support DEC-XTRAP extension\n",
		progname, DisplayString(ndpy));
	return False;
    }
    if (tc)
	XEFreeTC(tc);      
    (void)XEStartTrapRequest(ntc);
    tc = ntc;
#endif
#ifdef XTESTEXT1
    if (!XQueryExtension(ndpy, XTestEXTENSION_NAME,
			 &majop, &eventb, &errorb)) {
	fprintf(stderr,
		"%s: display '%s' does not support %s extension\n",
		progname, DisplayString(ndpy), XTestEXTENSION_NAME);
	return False;
    }
#endif
    if (dpy)
	XCloseDisplay(dpy);
    dpy = ndpy;
    reset_mapping();
    MIT_OBJ_CLASS = XInternAtom(dpy, "_MIT_OBJ_CLASS", False);
    Xmask = 1 << ConnectionNumber(dpy);
    maxfd = ConnectionNumber(dpy) + 1;
    return True;
}

void
do_display(buf)
    char *buf;
{
    Display *ndpy;
    char name[1024];

    if (*buf++ != 'D')
	return;
    if (!index(buf, ':')) {
	strcpy(name, buf);
	strcat(name, ":0");
	buf = name;
    }
    (void)init_display(buf);
}

void
do_macro(buf)
    char *buf;
{
    int n, i;
    char *macro;

    switch (*buf) {
    case 'a':
	macro_start = -1;
	break;
    case 'd':
	if (isdigit(buf[1]) && !buf[2]) {
	    n = buf[1] - '0';
	    if (macros[n].macro)
		free(macros[n].macro);
	    macros[n].len = 0;
	    macros[n].macro = NULL;
	}
	break;
    case 'e':
	if (isdigit(buf[1]) && !buf[2]) {
	    n = buf[1] - '0';
	    process(macros[n].macro, macros[n].len, 0);
	}
	break;
    case 'r':
	if (!buf[1])
	    macro_start = history_end + 4;
	break;
    case 's':
	if (isdigit(buf[1]) && !buf[2] && macro_start >= 0) {
	    n = buf[1] - '0';
	    i = history_end - macro_start;
	    macro = malloc(i);
	    if (macros[n].macro)
		free(macros[n].macro);
	    macros[n].len = i;
	    macros[n].macro = macro;
	    bcopy(history + macro_start, macro, i);
	    for (; --i >= 0; macro++)
		if (*macro == control_end)
		    *macro = control_char;
	}
	macro_start = -1;
	break;
    }
}

void
do_location(buf)
    char *buf;
{
    int n;
    Window root, w;
    unsigned int mask;
    int screen;

    switch (*buf) {
    case 's':
	if (isdigit(buf[1]) && !buf[2]) {
	    n = buf[1] - '0';
	    root = DefaultRootWindow(dpy);
	    while (1) {
		for (screen = 0; RootWindow(dpy, screen) != root; screen++)
		    ;
		if (XQueryPointer(dpy, root, &root, &locations[n].window,
				  &locations[n].x, &locations[n].y,
				  &locations[n].x, &locations[n].y, &mask))
		    break;
	    }
	    if (!locations[n].window)
		locations[n].window = root;
	    else {
		w = XmuClientWindow(dpy, locations[n].window);
		if (w)
		    locations[n].window = w;
		XTranslateCoordinates(dpy, root, locations[n].window,
				      locations[n].x, locations[n].y,
				      &locations[n].x, &locations[n].y, &w);
	    }
	}
	break;
    case 'w':
	if (isdigit(buf[1]) && !buf[2]) {
	    n = buf[1] - '0';
	    if (locations[n].window)
		XWarpPointer(dpy, None, locations[n].window, 0, 0, 0, 0,
			     locations[n].x, locations[n].y);
	}
	break;
    }
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
		undo_curbsmatch();
		while (curbscount)
		    undo_stroke();
	    } else if (in_control_seq)
		skip_next_control_char = True;
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
	if (skip_next_control_char) {
	    skip_next_control_char = False;
	    in_control_seq = False;
	    save_control(buf, i, i);
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
		if (j != i) {
		    if (iscntrl(buf[j])) {
			if (buf[j] != '\010') { /* abort */
			    i = j;
			    break;
			}
			bcopy(buf + j + 1, buf + j - 1, n - j - 1);
			n -= 2;
			j -= 2;
		    }
		    continue;
		}
		switch (buf[j]) {
		case '\003': /* control c */
		    do_key(control, 0);
		    break;
		case '\005': /* control e */
		    quit(0);
		    break;
		case '\010': /* control h */
		    j = i - 1;
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
		if (len && (i == j))
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
	    case '\006': /* control f */
		do_macro(buf + i + 1);
		break;
	    case '\012': /* control j */
	    	do_jump(buf + i + 1);
		break;
	    case '\014': /* control l */
	    	do_location(buf + i + 1);
		break;
	    case '\022': /* control r */
		do_display(buf + i + 1);
		break;
	    case '\027': /* control w */
		do_warp(buf + i + 1);
		break;
	    case '\031': /* control y */
		do_trigger(buf + i + 1);
		break;
	    case '\032': /* control z */
		time_delay = atof(buf + i + 1) * 1000;
		break;
	    default:
		do_keysym(buf + i, j - i);
	    }
	    buf[j] = control_char;
	    if (len)
		save_control(buf, i - 1, j);
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
    struct termios term;
    Bool noecho = True;
    char *dname = NULL;
    char buf[1024];
    char fbuf[1024];

    progname = argv[0];
    bzero((char *)fdmask, sizeof(fdmask));
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
    if (!dname && !*(XDisplayName(dname)))
	dname = ":0";
    if (!init_display(dname))
	quit(1);
    if (!undofile) {
	strcpy(fbuf, getenv("HOME"));
	strcat(fbuf, "/.a2x");
	undofile = fbuf;
    }
    get_undofile();
    while (1) {
#ifdef XTESTEXT1
	XTestFlush(dpy);
#endif
	if (XPending(dpy)) {
	    process_events();
	    continue;
	}
	fdmask[0] = 1 | Xmask;
	i = select(maxfd, fdmask, NULL, NULL, moving_timeout);
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
	if (fdmask[0] & Xmask)
	    process_events();
	if (!(fdmask[0] & 1))
	    continue;
	n = read(0, buf, sizeof(buf));
	if (n <= 0)
	    quit(0);
	process(buf, n, sizeof(buf));
	reflect_modifiers(0);
    }
}
