/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/* $Header: xmodmap.c,v 1.6 88/02/05 17:16:17 jim Locked $ */
#include <stdio.h>
#include <ctype.h>
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xatom.h"

char *ProgramName;

extern char *XKeysymToString();

Display *dpy;

static update_map = 0;
static output_map = 0;
static FILE *fout = stdout;

usage () 
{
    fprintf (stderr,
	"usage:  %s [-display host:dpy] [-options ...]\n\n", ProgramName);
    fprintf (stderr,
	"where options include:\n");
    fprintf (stderr,
	"    -S, -L, -C,         remove all keys for Shift, Lock, Control\n");
    fprintf (stderr,
	"    -[12345]            remove all keys for indicated ModN\n");
    fprintf (stderr,
	"    -s keysym, -l keysym, -c keysym,     remove indicated keysym\n");
    fprintf (stderr,
	"    +s keysym, +l keysym, +c keysym,     added indicated keysym\n");
    fprintf (stderr,
	"    +[12345] keysym     add keysym to modifier\n");
    fprintf (stderr,
	"\n");
    exit (1);
}


StartConnectionToServer(argc, argv)
int	argc;
char	*argv[];
{
    char *display;
    int i;

    display = NULL;
    for(i = 1; i < argc; i++)
    {
	if (strcmp ("-display", argv[i]) == 0 || strcmp ("-d", argv[i]) == 0) {
	    if (++i >= argc) usage ();
	    display = argv[i];
	    continue;
	} else if (index(argv[i], ':') != NULL)		/* obsolete */
	    display = argv[i];
    }
    if (!(dpy = XOpenDisplay(display)))
    {
       perror("Cannot open display\n");
       exit(1);
   }
}
UpdateModifierMapping(map)
    XModifierKeymap *map;
{
    int i = 5, res, tim = 2;

    while (i--) {
	res = XSetModifierMapping(dpy, map);
	fprintf(stderr, "res %d\n", res);
	switch (res) {
	    case MappingSuccess:	/* Success */
	        return (0);
	    case MappingBusy:		/* Busy */
	        fprintf (stderr, "You have %d seconds to lift your hands\n",
			 tim);
		sleep(tim);
		tim <<= 1;
	        break;
	    case MappingFailed:
	        fprintf(stderr, "Re-map failed\n");
		return (1);
	    default:
	    fprintf(stderr, "bad return %d\n", res);
	}
    }
    fprintf(stderr,  "I warned you\n");
    return (1);
}

SetMod(argc, argv, map, mod)
    int argc;
    char **argv;
    XModifierKeymap **map;
    int mod;
{
    if (argc) {
	int keycode = 0;

	argv++;
	if (isdigit(**argv)) {
	    keycode = atoi(*argv);
	} else {
	    KeySym ks = XStringToKeysym(*argv);

	    if (ks != NoSymbol)
		keycode = XKeysymToKeycode(dpy, ks);
	}
	fprintf(stderr, "%s: 0x%x\n", *argv, keycode);
	if (keycode)
	    *map = XInsertModifiermapEntry(*map, keycode, mod);
    }
    return (0);
}

ClearMod(map, mod)
    register XModifierKeymap **map;
    int mod;
{
    int i;

    for (i = 0; i < (*map)->max_keypermod; i++)
	(*map)->modifiermap[mod * (*map)->max_keypermod + i] = '\0';
}

DecodeArgs(argc, argv, map)
    int argc;
    char **argv;
    XModifierKeymap **map;
{
    while (--argc > 0) {
	argv++;
	if (**argv == '-') {
	    switch (*++*argv) {
		case 'd':		/* display */
		    --argc, argv++;
		    continue;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		    ClearMod(map, **argv - '0' + 2);
		    update_map++;
		    fout = NULL;
		    break;
		case 'S':
		case 's':
		    ClearMod(map, 0);
		    update_map++;
		    fout = NULL;
		    break;
		case 'C':
		case 'c':
		    ClearMod(map, 2);
		    update_map++;
		    fout = NULL;
		    break;
		case 'L':
		case 'l':
		    ClearMod(map, 1);
		    update_map++;
		    fout = NULL;
		    break;
		default:
		    usage ();
	    }
	}
	else if (**argv == '+') {
	    switch (*++*argv) {
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		    SetMod(argc, argv, map, **argv - '0' + 2);
		    argc--; argv++;
		    update_map++;
		    fout = NULL;
		    break;
		case 'S':
		case 's':
		    SetMod(argc, argv, map, 0);
		    argc--; argv++;
		    update_map++;
		    fout = NULL;
		    break;
		case 'C':
		case 'c':
		    SetMod(argc, argv, map, 2);
		    argc--; argv++;
		    update_map++;
		    fout = NULL;
		    break;
		case 'L':
		case 'l':
		    SetMod(argc, argv, map, 1);
		    argc--; argv++;
		    update_map++;
		    fout = NULL;
		    break;
		default:
		    usage ();
	    }
	} else {
	    usage ();
	}
    }
}

static char *modType[] = {
    "Shift",
    "Lock",
    "Ctrl",
    "One",
    "Two",
    "Three",
    "Four",
    "Five",
};

PrintModifierMapping(map, fout)
    XModifierKeymap *map;
    FILE *fout;
{
    int i, k = 0;

    fprintf(fout, "Currently max %d keys/mod\n", map->max_keypermod);
    for (i = 0; i < 8; i++) {
	int j;

	fprintf(fout, "%s:", modType[i]);
	for (j = 0; j < map->max_keypermod; j++) {
	    if (map->modifiermap[k]) {
		KeySym ks = XKeycodeToKeysym(dpy, map->modifiermap[k], 0);
		char *nm = XKeysymToString(ks);

		if (nm) {
		    fprintf(fout, "\t%s", nm);
		} else {
		    fprintf(fout, "\tBadKey");
		}
	    }
	    k++;
	}
	fprintf(fout, "\n");
    }
}

main(argc, argv)
    int argc;
    char **argv;
{
    XModifierKeymap *map;

    ProgramName = argv[0];

    StartConnectionToServer(argc, argv);

    map = XGetModifierMapping(dpy);

    DecodeArgs(argc, argv, &map);

    if (fout)
	PrintModifierMapping(map, fout);

    if (update_map)
	UpdateModifierMapping(map);

    XCloseDisplay (dpy);
    exit (0);
}

