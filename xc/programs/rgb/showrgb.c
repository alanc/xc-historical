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

#include <dbm.h>
#undef NULL
#include <stdio.h>
#include <X11/Xos.h>
#include "rgb.h"			/* off in server/include/ */
#include "site.h"

char *ProgramName;

main (argc, argv)
    int argc;
    char *argv[];
{
    int i;
    char *dbname = "/usr/lib/X11/rgb";

    ProgramName = argv[0];
    if (argc == 2)
	dbname = argv[1];
    else
	dbname = RGB_DB;

    dumprgb (dbname);
    exit (0);
}

dumprgb (filename)
    char *filename;
{
    datum key;

    if (dbminit(filename) != 0) {
	fprintf (stderr, "%s:  unable to open rgb database \"%s\"\n",
		 ProgramName, filename);
	exit (1);
    }

    for (key = firstkey(); key.dptr != NULL; key = nextkey(key)) {
	datum value;

	value = fetch (key);
	if (value.dptr) {
	    RGB rgb;
	    unsigned short r, g, b;
	    bcopy (value.dptr, (char *)&rgb, sizeof rgb);
#define N(x) (((x) >> 8) & 0xff)
	    r = N(rgb.red);
	    g = N(rgb.green);
	    b = N(rgb.blue);
#undef N
	    printf ("%3u %3u %3u\t\t", r, g, b);
	    fwrite (key.dptr, 1, key.dsize, stdout);
	    putchar ('\n');
	} else {
	    fprintf (stderr, "%s:  no value found for key \"", ProgramName);
	    fwrite (key.dptr, 1, key.dsize, stderr);
	    fprintf (stderr, "\"\n");
	}
    }

    dbmclose();
}
