/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xmu/Error.h>

char *ProgramName;

static void usage ()
{
    fprintf (stderr, "usage:  %s [-options ...]\n\n", ProgramName);
    fprintf (stderr, "where options include:\n");
    fprintf (stderr, 
	     "    -display dpy            X server to which to connect\n");
    fprintf (stderr,
	     "    -range [num]-[num]      atom values to list\n");
    fprintf (stderr,
	     "    -format string          printf format to use\n");
    fprintf (stderr, "\n");
    exit (1);
}

#define RangeLow (1 << 0)
#define RangeHigh (1 << 1)

static int parse_range (range, lowp, highp)
    char *range;
    long *lowp, *highp;
{
    char *dash;
    int mask = 0;

    if (!range) {			/* NULL means default */
	*lowp = 1;
	return RangeLow;
    }

    dash = index (range, '-');
    if (dash) {
	if (dash == range) {		/* -high */
	    *lowp = 1;
	} else {			/* low-[high] */
	    *dash = '\0';
	    *lowp = atoi (range);
	    *dash = '-';
	}
	mask |= RangeLow;
	dash++;
	if (*dash) {			/* [low]-high */
	    *highp = atoi (dash);
	    mask |= RangeHigh;
	}
    } else {				/* number (low == high) */
	*lowp = *highp = atoi (range);
	mask |= (RangeLow | RangeHigh);
    }

    return mask;
}

main (argc, argv)
    int argc;
    char **argv;
{
    char *displayname = NULL;
    int i;
    Display *dpy;
    char *range = NULL;
    int rangemask;
    long low, high;
    char *format = "%ld\t%s";

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'd':			/* -display dpy */
		if (++i >= argc) usage ();
		displayname = argv[i];
		continue;
	      case 'r':			/* -range num-[num] */
		if (++i >= argc) usage ();
		range = argv[i];
		continue;
	      case 'f':			/* -format string */
		if (++i >= argc) usage ();
		format = argv[i];
		continue;
	    }
	}
	usage ();
    }

    rangemask = parse_range (range, &low, &high);

    dpy = XOpenDisplay (displayname);
    if (!dpy) {
	fprintf (stderr, "%s:  unable to open display \"%s\"\n",
		 ProgramName, XDisplayName (displayname));
	exit (1);
    }

    list_atoms (dpy, format, rangemask, low, high);
}

static int catcher (dpy, err)
    Display *dpy;
    XErrorEvent *err;
{
    if (err->request_code != X_GetAtomName) {
	XmuPrintDefaultErrorMessage (dpy, err, stderr);
    }
    return 0;
}

list_atoms (dpy, format, mask, low, high)
    Display *dpy;
    char *format;
    int mask;
    long low, high;
{
    int (*oldhandler)() = XSetErrorHandler (catcher);

    switch (mask) {
      case RangeHigh:
	low = 1;
	/* fall through */
      case (RangeLow | RangeHigh):
	for (; low <= high; low++) {
	    char *s = XGetAtomName (dpy, (Atom)low);
	    if (s) {
		printf (format, low, s);
		putchar ('\n');
		XFree (s);
	    }
	}
	break;

      default:
	low = 1;
	/* fall through */
      case RangeLow:
	for (; ; low++) {
	    char *s = XGetAtomName (dpy, (Atom)low);
	    if (s) {
		printf (format, low, s);
		putchar ('\n');
		XFree (s);
	    } else {
		break;
	    }
	}
	break;
    }

    XSetErrorHandler (oldhandler);
    return;
}
