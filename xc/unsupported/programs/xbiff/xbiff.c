/*
 * $Header: xbiff.c,v 1.5 87/11/09 10:45:11 jim Exp $
 *
 * Copyright 1987 by Cognition Inc.  All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Cognition Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Cognition Inc. makes no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 */

#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Atoms.h>
#include "Mailbox.h"

extern void exit();

char *ProgramName;

static XrmOptionDescRec options[] = {
{ "-update", "*mailbox.update", XrmoptionSepArg, (caddr_t) NULL },
};

static void Usage ()
{
    static char *help_message[] = {
"where options include:",
"    -update seconds                how often to check for mail",
"    -bg color                      background color",
"    -fg color                      foreground color",
"    -rv                            reverse video",
"    -geometry geom                 size of mailbox",
"    -display host:dpy              X server to contact",
NULL};
    char **cpp;

    fprintf (stderr, "usage:  %s [-options ...]\n", ProgramName);
    for (cpp = help_message; *cpp; cpp++) {
	fprintf (stderr, "%s\n", *cpp);
    }
    fprintf (stderr, "\n");
    exit (1);
}


void main (argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel, w;

    ProgramName = argv[0];

    toplevel = XtInitialize ("main", "XBiff", options, XtNumber (options),
			     &argc, argv);
    if (argc != 1) Usage ();

    w = XtCreateManagedWidget ("mailbox", mailboxWidgetClass, toplevel,
			       NULL, 0);
    XtRealizeWidget (toplevel);
    XtMainLoop ();
}

