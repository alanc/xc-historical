/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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
#include <X11/Intrinsic.h>

char *ProgramName;

static void usage ()
{
    fprintf (stderr, "usage:  %s  [classname]\n", ProgramName);
    exit (1);
}

main (argc, argv)
    int argc;
    char **argv;
{
    Widget toplevel;
    char *classname = "Dumpres";
    char tmpbuf[256];
    char buf[BUFSIZ];
    FILE *fp;
    int n;

    ProgramName = argv[0];
    if (argc > 1 && argv[1][0] != '-') {
	classname = argv[1];
	argv[1] = ProgramName;
	argc--, argv++;
    }
    toplevel = XtInitialize (NULL, classname, NULL, 0, &argc, argv);
    if (argc != 1) usage ();

    strcpy (tmpbuf, "/tmp/da.XXXXXX");
    mktemp (tmpbuf);

    unlink (tmpbuf);
    XrmPutFileDatabase (XtDatabase(XtDisplay(toplevel)), tmpbuf);
    
    fp = fopen (tmpbuf, "r");
    unlink (tmpbuf);
    while ((n = fread (buf, 1, sizeof buf, fp)) > 0) {
	fwrite (buf, 1, n, stdout);
    }
    fclose (fp);
    exit (0);
}
