/*
 * xkbev - event diagnostics for the X Keyboard Extension
 *
 * $XConsortium: xev.c,v 1.14 91/05/04 23:15:11 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <ctype.h>

#include <X11/extensions/XKBproto.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBlib.h>

#define OUTER_WINDOW_MIN_WIDTH 100
#define OUTER_WINDOW_MIN_HEIGHT 100
#define OUTER_WINDOW_DEF_WIDTH (OUTER_WINDOW_MIN_WIDTH + 100)
#define OUTER_WINDOW_DEF_HEIGHT (OUTER_WINDOW_MIN_HEIGHT + 100)
#define OUTER_WINDOW_DEF_X 100
#define OUTER_WINDOW_DEF_Y 100
				

typedef unsigned long Pixel;

char *Yes = "YES";
char *No = "NO";
char *Unknown = "unknown";

char *ProgramName;
Display *dpy;
int screen;
unsigned long wanted =	KeyPressMask|KeyReleaseMask|KeymapStateMask|
			ButtonPressMask|ButtonReleaseMask|OwnerGrabButtonMask;
int synch= 0;
int ignore= 0;
int xkbEventBase;

usage ()
{
    static char *msg[] = {
"    -display displayname                X server to contact",
"    -geometry geom                      size and location of window",
"    -bw pixels                          border width in pixels",
"    -id windowid                        use existing window",
"    -name string                        window name",
"    -synch                              synchronize requests",
"    -ignore                             ignore XKB",
"    -B                                  want button press/release events",
"    -b[1-5]                             want button motion events",
"    -m                                  want motion events",
"    -e                                  want enter events",
"    -l                                  want leave events",
NULL};
    char **cpp;

    fprintf (stderr, "usage:  %s [-options ...]\n", ProgramName);
    fprintf (stderr, "where options include:\n");

    for (cpp = msg; *cpp; cpp++) {
	fprintf (stderr, "%s\n", *cpp);
    }

    exit (1);
}

prologue (eventp, event_name)
    XEvent *eventp;
    char *event_name;
{
    XAnyEvent *e = (XAnyEvent *) eventp;

    printf ("\n%s event, serial %ld, synthetic %s, window 0x%lx,\n",
	    event_name, e->serial, e->send_event ? Yes : No, e->window);
    return;
}

xkb_prologue (eventp, event_name)
    XKBEvent *eventp;
    char *event_name;
{
    XKBAnyEvent *e = (XKBAnyEvent *) eventp;

    printf ("\n%s event, serial %ld, synthetic %s, device %d, time %ld,\n",
	    event_name, e->serial, e->send_event ? Yes : No, e->device,e->time);
    return;
}

char *
eventTypeToString(evType)
    int evType;
{
static char name[20];
    switch (evType) {
	case KeyPress:	strcpy(name,"KeyPress"); break;
	case KeyRelease:strcpy(name,"KeyRelease"); break;
	default: strcpy(name,"unknown"); break;
    }
    return name;
}

void
do_XKBStateNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBStateNotifyEvent *state= &xkbev->state;

    if (state->keycode!=0)
	 printf("    keycode %d, eventType %s,",
		state->keycode,eventTypeToString(state->eventType));
    else printf("    request %d/%d,",state->requestMajor,state->requestMinor);
    printf(" compatState = 0x%02x%s\n",
		state->compatState, (state->changed&XKBCompatStateMask?"*":""));
    printf("    group= %d%s, base= %d%s, latched= %d%s, locked= %d%s,\n",
		state->group, (state->changed&XKBGroupStateMask?"*":""),
		state->baseGroup,(state->changed&XKBGroupBaseMask?"*":""),
		state->latchedGroup,(state->changed&XKBGroupLatchMask?"*":""),
		state->lockedGroup,(state->changed&XKBGroupLockMask?"*":""));
    printf("    mods= 0x%02x%s, base= 0x%02x%s, latched= 0x%02x%s, locked= 0x%02x%s\n",
		state->mods, (state->changed&XKBModifierStateMask?"*":""),
		state->baseMods,(state->changed&XKBModifierBaseMask?"*":""),
		state->latchedMods,(state->changed&XKBModifierLatchMask?"*":""),
		state->lockedMods,(state->changed&XKBModifierLockMask?"*":""));
    printf("    unlocked mods= 0x%02x%s, groups%s unlocked\n",
	state->unlockedMods,(state->changed&XKBModifierUnlockMask?"*":""),
	(state->groupsUnlocked?"":" not"));
    return;
}

void
do_XKBMapNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBMapNotifyEvent *map = &xkbev->map;
    if (map->changed&XKBKeyTypesMask) {
	if (map->nKeyTypes>1)
	     printf("    key types %d..%d changed", map->firstKeyType,
					map->firstKeyType+map->nKeyTypes-1);
	else printf("    key type %d changed", map->firstKeyType);
	if (map->resized&XKBKeyTypesMask)
	     printf(" [possibly resized]\n");
	else printf("\n");
    }
    if (map->changed&XKBKeySymsMask) {
	if (map->nKeySyms>1)
	     printf("    symbols for keys %d..%d changed\n",map->firstKeySym,
					map->firstKeySym+map->nKeySyms-1);
	else printf("    symbols for key %d changed\n",map->firstKeySym);
    }
    if (map->changed&XKBKeyActionsMask) {
	if (map->nKeyActions>1)
	     printf("    actions for keys %d..%d changed\n",map->firstKeyAction,
					map->firstKeyAction+map->nKeyActions-1);
	else printf("    actions for key %d changed\n",map->firstKeyAction);
    }
    if (map->changed&XKBKeyBehaviorsMask) {
	if (map->nKeyBehaviors>1)
	     printf("    behavior for keys %d..%d changed\n",
				map->firstKeyBehavior,
				map->firstKeyBehavior+map->nKeyBehaviors-1);
	else printf("    behavior for key %d changed\n",map->firstKeyBehavior);
    }
    return;
}

void
do_XKBControlsNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBControlsNotifyEvent *ctrls = &xkbev->controls;
    printf("    changed= 0x%x, enabled= 0x%x\n",ctrls->changedControls,
						ctrls->enabledControls);
    return;
}

void
do_XKBIndicatorNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBIndicatorNotifyEvent *leds = &xkbev->indicators;
    printf("    stateChanged= 0x%08x, new state= 0x%08x\n",leds->stateChanged,
								leds->state);
    printf("    mapsChanged= 0x%08x\n",leds->mapChanged);
    return;
}

void
do_XKBBellNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBBellNotifyEvent *bell = &xkbev->bell;
    printf("    bell class= %d, id= %d\n",bell->bellClass,bell->bellID);
    printf("    percent= %d, pitch= %d, duration= %d",
				bell->percent,bell->pitch,bell->duration);
    if (bell->name!=None) {
	 char *name = XGetAtomName (dpy, bell->name);
	 printf("\n    name= \"%s\"\n",(name?name:""));
	 if (name)
	    XFree(name);
    }
    else printf(", no name\n");
    return;
}

void
do_XKBSlowKeyNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBSlowKeyNotifyEvent *sk = &xkbev->slowKey;
    char *what;
    switch (sk->slowKeyType) {
	case XKBSKPress:  what= "press"; break;
	case XKBSKAccept: what= "accept"; break;
	case XKBSKReject: what= "reject"; break;
	case XKBSKRelease: what= "release"; break;
	default: {
	    static char buf[20];
	    sprintf(buf,"unknown(%d)",sk->xkbType);
	    what= buf;
	    break;
	}
    }
    printf("    keycode= %d, xkbType= %s, delay= %d\n",sk->keycode,what,
								sk->delay);
    return;
}

do_XKBNamesNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBNamesNotifyEvent *names = &xkbev->names;

    if (names->changed&
		(XKBKeycodesNameMask|XKBGeometryNameMask|XKBSymbolsNameMask)) {
	int needComma= 0;
	printf("    ");
	if (names->changed&XKBKeycodesNameMask) {
	    printf("keycodes");
	    needComma++;
	}
	if (names->changed&XKBGeometryNameMask) {
	    printf("%sgeometry",(needComma?", ":""));
	    needComma++;
	}
	if (names->changed&XKBSymbolsNameMask) {
	    printf("%ssymbols",(needComma?", ":""));
	    needComma++;
	}
	printf(" name%s changed\n",(needComma>1?"s":""));
    }
    if (names->changed&XKBKeyTypeNamesMask) {
	printf("    names of key types %d..%d changed\n",
		names->firstKeyType,names->firstKeyType+names->nKeyTypes-1);
    }
    if (names->changed&XKBKTLevelNamesMask) {
	printf("    level names for key types %d..%d changed\n",
		names->firstLevel,names->firstLevel+names->nLevels-1);
    }
    if (names->changed&XKBRGNamesMask) {
	printf("    names of radio groups %d..%d changed\n",
				names->firstRadioGroup,
				names->firstRadioGroup+names->nRadioGroups-1);
    }
    if (names->changed&XKBIndicatorNamesMask) {
	printf("    names of indicators in 0x%08x changed\n",
						names->changedIndicators);
    }
    if (names->changed&XKBModifierNamesMask) {
	printf("    names of modifiers in 0x%02x changed\n",names->changedMods);
    }
    if (names->changed&XKBCharSetsMask) {
	 printf("    character set names changed (%d character sets)\n",
						names->nCharSets);
    }
    else printf("    keyboard has %d character sets\n",names->nCharSets);
    return;
}

void
do_XKBCompatMapNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBCompatMapNotifyEvent *map = &xkbev->compat;

    if (map->changedMods)
	printf("    maps for modifiers in 0x%02 changed\n",map->changedMods);
    if (map->nSyms>0) {
	printf("    symbol interpretations %d..%d (of %d) changed\n",
		map->firstSym, map->firstSym+map->nSyms-1, map->nTotalSyms);
    }
    else printf("   keyboard has %d symbol interpretations\n",map->nTotalSyms);
    return;
}

void
do_XKBAlternateSymsNotify(xkbev)
    XKBEvent	*xkbev;
{
    XKBAlternateSymsNotifyEvent *altSyms= &xkbev->altSyms;
    printf("    alternate symbol set: %d\n",altSyms->altSymsID);
    printf("    definitions of keys %d..%d changed\n",altSyms->firstKey,
					altSyms->firstKey+altSyms->nKeys-1);
    return;
}

main (argc, argv)
    int argc;
    char **argv;
{
    char *displayname = NULL;
    char *geom = NULL;
    int i,i1,i2;
    XSizeHints hints;
    int borderwidth = 2;
    Window w;
    XSetWindowAttributes attr;
    XWindowAttributes wattr;
    unsigned long mask = 0L;
    int done;
    char *name = "XKB Event Tester";
    unsigned long back, fore;

    w = 0;
    ProgramName = argv[0];
    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (strcasecmp(arg,"-synch")==0)
	    synch= 1;
	else if (strcasecmp(arg,"-ignore")==0)
	    ignore= 1;
	else if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':			/* -display host:dpy */
		if (++i >= argc) usage ();
		displayname = argv[i];
		continue;
	      case 'g':			/* -geometry geom */
		if (++i >= argc) usage ();
		geom = argv[i];
		continue;
	      case 'B':
		wanted|= ButtonPressMask|ButtonReleaseMask;
		continue;
	      case 'b':
		switch (arg[2]) {
		  case 'w':		/* -bw pixels */
		    if (++i >= argc) usage ();
		    borderwidth = atoi (argv[i]);
		    continue;
		  case '1':
		    wanted|= Button1MotionMask;
		    continue;
		  case '2':
		    wanted|= Button2MotionMask;
		    continue;
		  case '3':
		    wanted|= Button3MotionMask;
		    continue;
		  case '4':
		    wanted|= Button4MotionMask;
		    continue;
		  case '5':
		    wanted|= Button5MotionMask;
		    continue;
		  case 'm':
		    wanted|= ButtonMotionMask;
		    continue;
		  default:
		    usage ();
		}
	      case 'm':
		wanted|= PointerMotionMask;
		continue;
	      case 'e':
		wanted|= EnterWindowMask|LeaveWindowMask;
		continue;
	      case 'i':			/* -id */
		if (++i >= argc) usage ();
		sscanf(argv[i], "0x%lx", &w);
		if (!w)
		    sscanf(argv[i], "%ld", &w);
		if (!w)
		    usage ();
		continue;
	      case 'n':			/* -name */
		if (++i >= argc) usage ();
		name = argv[i];
		continue;
	      default:
		usage ();
	    }				/* end switch on - */
	} else 
	  usage ();
    }					/* end for over argc */

    if (ignore)
	XKBIgnoreExtension(1);
    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display '%s'\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }
    if (synch)
	XSynchronize(dpy,1);
    printf("%s compiled with XKB version %d.%02d\n",argv[0],XKB_MAJOR_VERSION,
							XKB_MINOR_VERSION);
    XKBLibraryVersion(&i1,&i2);
    printf("X library supports XKB version %d.%02d\n",i1,i2);
    if ( !XKBQueryExtension(dpy,&xkbEventBase,&i2)>0 ) {
	printf("XKB Extension not present on %s\n",XDisplayName(displayname));
	ignore= 1;
    }
    if (ignore)
	 printf("Intentionally ignoring XKB\n");
    else if ( !XKBUseExtension(dpy,&i1,&i2) ) {
	 printf("X server supports XKB version %d.02d (MISMATCH)\n",i1,i2);
	 ignore= 1;
    }
    else printf("versions match\n");


    screen = DefaultScreen (dpy);

    /* select for all events */
    if (!ignore)
	XKBSelectEvents(dpy,XKB_USE_CORE_KBD,XKBAllEventsMask,XKBAllEventsMask);
    attr.event_mask = wanted;
    if (w) {
	XGetWindowAttributes(dpy, w, &wattr);
	if (wattr.all_event_masks & ButtonPressMask)
	    attr.event_mask &= ~ButtonPressMask;
	attr.event_mask &= ~SubstructureRedirectMask;
	XSelectInput(dpy, w, attr.event_mask);
    } else {
	set_sizehints (&hints, OUTER_WINDOW_MIN_WIDTH, OUTER_WINDOW_MIN_HEIGHT,
		       OUTER_WINDOW_DEF_WIDTH, OUTER_WINDOW_DEF_HEIGHT, 
		       OUTER_WINDOW_DEF_X, OUTER_WINDOW_DEF_Y, geom);

	back = WhitePixel(dpy,screen);
	fore = BlackPixel(dpy,screen);

	attr.background_pixel = back;
	attr.border_pixel = fore;
	mask |= (CWBackPixel | CWBorderPixel | CWEventMask);

	w = XCreateWindow (dpy, RootWindow (dpy, screen), hints.x, hints.y,
			   hints.width, hints.height, borderwidth, 0,
			   InputOutput, (Visual *)CopyFromParent,
			   mask, &attr);

	XSetStandardProperties (dpy, w, name, NULL, (Pixmap) 0,
				argv, argc, &hints);

	XMapWindow (dpy, w);
    }

    for (done = 0; !done; ) {
	XEvent event;

	XNextEvent (dpy, &event);

	if (event.type==xkbEventBase) {
	    XKBEvent	*xkbev = (XKBEvent *)&event;
	    switch (xkbev->u.xkbType) {
		case XKBStateNotify:
		    xkb_prologue( xkbev, "XKBStateNotify" );
		    do_XKBStateNotify(xkbev);
		    break;
		case XKBMapNotify:
		    xkb_prologue( xkbev, "XKBMapNotify" );
		    do_XKBMapNotify(xkbev);
		    break;
		case XKBControlsNotify:
		    xkb_prologue( xkbev, "XKBControlsNotify" );
		    do_XKBControlsNotify(xkbev);
		    break;
		case XKBIndicatorNotify:
		    xkb_prologue( xkbev, "XKBIndicatorNotify" );
		    do_XKBIndicatorNotify(xkbev);
		    break;
		case XKBBellNotify:
		    xkb_prologue( xkbev, "XKBBellNotify" );
		    do_XKBBellNotify(xkbev);
		    break;
		case XKBSlowKeyNotify:
		    xkb_prologue( xkbev, "XKBSlowKeyNotify" );
		    do_XKBSlowKeyNotify(xkbev);
		    break;
		case XKBNamesNotify:
		    xkb_prologue( xkbev, "XKBNamesNotify" );
		    do_XKBNamesNotify(xkbev);
		    break;
		case XKBCompatMapNotify:
		    xkb_prologue( xkbev, "XKBCompatMapNotify" );
		    do_XKBCompatMapNotify(xkbev);
		    break;
		case XKBAlternateSymsNotify:
		    xkb_prologue( xkbev, "XKBAlternateSymsNotify" );
		    do_XKBAlternateSymsNotify(xkbev);
		    break;
		default:
		    xkb_prologue( xkbev, "XKB_UNKNOWN!!!" );
		    break;
	    }
	}
	else switch (event.type) {
	  case KeyPress:
	    prologue (&event, "KeyPress");
	    do_KeyPress (&event);
	    break;
	  case KeyRelease:
	    prologue (&event, "KeyRelease");
	    do_KeyRelease (&event);
	    break;
	  case ButtonPress:
	    prologue (&event, "ButtonPress");
	    do_ButtonPress (&event);
	    break;
	  case ButtonRelease:
	    prologue (&event, "ButtonRelease");
	    do_ButtonRelease (&event);
	    break;
	  case MotionNotify:
	    prologue (&event, "MotionNotify");
	    do_MotionNotify (&event);
	    break;
	  case EnterNotify:
	    prologue (&event, "EnterNotify");
	    do_EnterNotify (&event);
	    break;
	  case LeaveNotify:
	    prologue (&event, "LeaveNotify");
	    do_LeaveNotify (&event);
	    break;
	  case KeymapNotify:
	    prologue (&event, "KeymapNotify");
	    do_KeymapNotify (&event);
	    break;
	  case ClientMessage:
	    prologue (&event, "ClientMessage");
	    do_ClientMessage (&event);
	    break;
	  case MappingNotify:
	    prologue (&event, "MappingNotify");
	    do_MappingNotify (&event);
	    break;
	  default:
	    printf ("Unknown event type %d\n", event.type);
	    break;
	}
    }

    XCloseDisplay (dpy);
    exit (0);
}

do_KeyPress (eventp)
    XEvent *eventp;
{
    XKeyEvent *e = (XKeyEvent *) eventp;
    KeySym ks;
    char *ksname;
    int nbytes;
    char str[256+1];

    nbytes = XLookupString (e, str, 256, &ks, NULL);
    if (ks == NoSymbol)
	ksname = "NoSymbol";
    else if (!(ksname = XKeysymToString (ks)))
	ksname = "(no name)";
    printf ("    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    printf ("    state 0x%x, group= %d, keycode %u (keysym 0x%x, %s)\n",
	    	e->state&0x1FFF, (e->state>>13)&0x7, e->keycode, ks, ksname);
    printf ("    same_screen %s,\n",e->same_screen ? Yes : No);
    if (nbytes < 0) nbytes = 0;
    if (nbytes > 256) nbytes = 256;
    str[nbytes] = '\0';
    printf ("    XLookupString gives %d characters:  \"%s\"\n", nbytes, str);

    return;
}

do_KeyRelease (eventp)
    XEvent *eventp;
{
    do_KeyPress (eventp);		/* since it has the same info */
    return;
}

do_ButtonPress (eventp)
    XEvent *eventp;
{
    XButtonEvent *e = (XButtonEvent *) eventp;

    printf ("    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    printf ("    state 0x%x, group= %d, button %u, same_screen %s\n",
	    e->state&0x1FFF, (e->state>>13)&0x7, e->button, 
	    e->same_screen ? Yes : No);

    return;
}

do_ButtonRelease (eventp)
    XEvent *eventp;
{
    do_ButtonPress (eventp);		/* since it has the same info */
    return;
}

do_MotionNotify (eventp)
    XEvent *eventp;
{
    XMotionEvent *e = (XMotionEvent *) eventp;

    printf ("    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    printf ("    state 0x%x, group= %d, is_hint %u, same_screen %s\n",
	    e->state&0x1FFF, (e->state>>13)&0x7, e->is_hint, 
	    e->same_screen ? Yes : No);

    return;
}

do_EnterNotify (eventp)
    XEvent *eventp;
{
    XCrossingEvent *e = (XCrossingEvent *) eventp;
    char *mode, *detail;
    char dmode[10], ddetail[10];

    switch (e->mode) {
      case NotifyNormal:  mode = "NotifyNormal"; break;
      case NotifyGrab:  mode = "NotifyGrab"; break;
      case NotifyUngrab:  mode = "NotifyUngrab"; break;
      case NotifyWhileGrabbed:  mode = "NotifyWhileGrabbed"; break;
      default:  mode = dmode, sprintf (dmode, "%u", e->mode); break;
    }

    switch (e->detail) {
      case NotifyAncestor:  detail = "NotifyAncestor"; break;
      case NotifyVirtual:  detail = "NotifyVirtual"; break;
      case NotifyInferior:  detail = "NotifyInferior"; break;
      case NotifyNonlinear:  detail = "NotifyNonlinear"; break;
      case NotifyNonlinearVirtual:  detail = "NotifyNonlinearVirtual"; break;
      case NotifyPointer:  detail = "NotifyPointer"; break;
      case NotifyPointerRoot:  detail = "NotifyPointerRoot"; break;
      case NotifyDetailNone:  detail = "NotifyDetailNone"; break;
      default:  detail = ddetail; sprintf (ddetail, "%u", e->detail); break;
    }

    printf ("    root 0x%lx, subw 0x%lx, time %lu, (%d,%d), root:(%d,%d),\n",
	    e->root, e->subwindow, e->time, e->x, e->y, e->x_root, e->y_root);
    printf ("    mode %s, detail %s, same_screen %s,\n",
	    mode, detail, e->same_screen ? Yes : No);
    printf ("    focus %s, state %u, group= %d\n", e->focus ? Yes : No, 
	    e->state&0x1FFF, (e->state>>13)&7 );

    return;
}

do_LeaveNotify (eventp)
    XEvent *eventp;
{
    do_EnterNotify (eventp);		/* since it has same information */
    return;
}

do_KeymapNotify (eventp)
    XEvent *eventp;
{
    XKeymapEvent *e = (XKeymapEvent *) eventp;
    int i;

    printf ("    keys:  ");
    for (i = 0; i < 32; i++) {
	if (i == 16) printf ("\n           ");
	printf ("%-3u ", (unsigned int) e->key_vector[i]);
    }
    printf ("\n");
    return;
}

do_ClientMessage (eventp)
    XEvent *eventp;
{
    XClientMessageEvent *e = (XClientMessageEvent *) eventp;
    char *mname = XGetAtomName (dpy, e->message_type);

    printf ("    message_type 0x%lx (%s), format %d\n",
	    e->message_type, mname ? mname : Unknown, e->format);

    if (mname) XFree (mname);
    return;
}

do_MappingNotify (eventp)
    XEvent *eventp;
{
    XMappingEvent *e = (XMappingEvent *) eventp;
    char *r;
    char rdummy[10];

    switch (e->request) {
      case MappingModifier:  r = "MappingModifier"; break;
      case MappingKeyboard:  r = "MappingKeyboard"; break;
      case MappingPointer:  r = "MappingPointer"; break;
      default:  r = rdummy; sprintf (rdummy, "%d", e->request); break;
    }

    printf ("    request %s, first_keycode %d, count %d\n",
	    r, e->first_keycode, e->count);
    XRefreshKeyboardMapping(e);
    return;
}



set_sizehints (hintp, min_width, min_height,
	       defwidth, defheight, defx, defy, geom)
    XSizeHints *hintp;
    int min_width, min_height, defwidth, defheight, defx, defy;
    char *geom;
{
    int geom_result;

    /* set the size hints, algorithm from xlib xbiff */

    hintp->width = hintp->min_width = min_width;
    hintp->height = hintp->min_height = min_height;
    hintp->flags = PMinSize;
    hintp->x = hintp->y = 0;
    geom_result = NoValue;
    if (geom != NULL) {
        geom_result = XParseGeometry (geom, &hintp->x, &hintp->y,
				      (unsigned int *)&hintp->width,
				      (unsigned int *)&hintp->height);
	if ((geom_result & WidthValue) && (geom_result & HeightValue)) {
#define max(a,b) ((a) > (b) ? (a) : (b))
	    hintp->width = max (hintp->width, hintp->min_width);
	    hintp->height = max (hintp->height, hintp->min_height);
	    hintp->flags |= USSize;
	}
	if ((geom_result & XValue) && (geom_result & YValue)) {
	    hintp->flags += USPosition;
	}
    }
    if (!(hintp->flags & USSize)) {
	hintp->width = defwidth;
	hintp->height = defheight;
	hintp->flags |= PSize;
    }
/*
    if (!(hintp->flags & USPosition)) {
	hintp->x = defx;
	hintp->y = defy;
	hintp->flags |= PPosition;
    }
 */
    if (geom_result & XNegative) {
	hintp->x = DisplayWidth (dpy, DefaultScreen (dpy)) + hintp->x -
		    hintp->width;
    }
    if (geom_result & YNegative) {
	hintp->y = DisplayHeight (dpy, DefaultScreen (dpy)) + hintp->y -
		    hintp->height;
    }
    return;
}

