/* $Header: xmodmap.c,v 1.1 87/08/29 22:03:05 toddb Exp $ */
/*
 * xmodmap is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify prkbd without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 *
 * PRKBD IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * prkbd is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY PRKBD
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#include <stdio.h>
#include <ctype.h>
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xatom.h"

extern char *XKeysymToString();

Display *dpy;

static update_map = 0;
static output_map = 0;
static FILE *fout = stdout;

StartConnectionToServer(argc, argv)
int	argc;
char	*argv[];
{
    char *display;
    int i;

    display = NULL;
    for(i = 1; i < argc; i++)
    {
        if(index(argv[i], ':') != NULL)
	    display = argv[i];
    }
    if (!(dpy = XOpenDisplay(display)))
    {
       perror("Cannot open display\n");
       exit(0);
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
	    case 0:	/* Success */
	        return (0);
	    case 1:	/* Busy */
	        fprintf (stderr, "You have %d seconds to lift your hands\n",
			 tim);
		sleep(tim);
		tim <<= 1;
	        break;
	    case 2:
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
    XModifierKeymap *map;
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
	if (keycode) {
	    int i;
	    
	    for (i = 0; i < map->max_keypermod; i++) {
		int index = mod*map->max_keypermod + i;
		if (map->modifiermap[index] == 0) {
		    map->modifiermap[index] = (unsigned char) keycode;
		    return (1);
		}
	    }
	}
    }
    return (0);
}

ClearMod(map, mod)
    XModifierKeymap *map;
    int mod;
{
    int i;

    for (i = 0; i < map->max_keypermod; i++)
	map->modifiermap[mod * map->max_keypermod + i] = '\0';
}

DecodeArgs(argc, argv, map)
    int argc;
    char **argv;
    XModifierKeymap *map;
{
    while (--argc > 0) {
	argv++;
	if (**argv == '-') {
	    switch (*++*argv) {
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
		    fout = NULL;
		    break;
		case 'C':
		case 'c':
		    ClearMod(map, 2);
		    fout = NULL;
		    break;
		case 'L':
		case 'l':
		    ClearMod(map, 1);
		    fout = NULL;
		    break;
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
	    }
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

    StartConnectionToServer(argc, argv);

    map = XGetModifierMapping(dpy);

    DecodeArgs(argc, argv, map);

    if (fout)
	PrintModifierMapping(map, fout);

    if (update_map)
	UpdateModifierMapping(map);
}

