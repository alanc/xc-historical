/*
 * $XConsortium: showrgb.c,v 1.9 93/09/20 17:58:21 hersh Exp $
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

#ifndef USE_RGB_TXT
#ifdef NDBM
#include <ndbm.h>
#else
#ifdef SVR4
#include <rpcsvc/dbm.h>
#else
#include <dbm.h>
#endif
#define dbm_open(name,flags,mode) (!dbminit(name))
#define dbm_firstkey(db) (firstkey())
#define dbm_fetch(db,key) (fetch(key))
#define dbm_close(db) dbmclose()
#endif
#endif /* USE_RGB_TXT */

#undef NULL
#include <stdio.h>
#include <X11/Xos.h>
#include "rgb.h"			/* off in server/include/ */
#include "site.h"
#include <X11/Xfuncs.h>

char *ProgramName;

main (argc, argv)
    int argc;
    char *argv[];
{
    char *dbname = RGB_DB;

    ProgramName = argv[0];
    if (argc == 2)
	dbname = argv[1];

    dumprgb (dbname);
    exit (0);
}

#ifndef USE_RGB_TXT

dumprgb (filename)
    char *filename;
{
#ifdef NDBM
    DBM *rgb_dbm;
#else
    int rgb_dbm;
#endif
    datum key;

    rgb_dbm = dbm_open (filename, O_RDONLY, 0);
    if (!rgb_dbm) {
	fprintf (stderr, "%s:  unable to open rgb database \"%s\"\n",
		 ProgramName, filename);
	exit (1);
    }

#ifndef NDBM
#define dbm_nextkey(db) (nextkey(key))	/* need variable called key */
#endif

    for (key = dbm_firstkey(rgb_dbm); key.dptr != NULL;
	 key = dbm_nextkey(rgb_dbm)) {
	datum value;

	value = dbm_fetch(rgb_dbm, key);
	if (value.dptr) {
	    RGB rgb;
	    unsigned short r, g, b;
	    memcpy( (char *)&rgb, value.dptr, sizeof rgb);
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

    dbm_close (rgb_dbm);
}

#else /* USE_RGB_TXT */

dumprgb (filename)
    char *filename;
{
    FILE *rgb;
    char *path;
    char line[BUFSIZ];
    char name[BUFSIZ];
    int lineno = 0;
    int red, green, blue;
   
    path = (char *)malloc(strlen(filename) + 5);
    strcpy(path, filename);
    strcat(path, ".txt");

    if (!(rgb = fopen(path, "r"))) {
	fprintf (stderr, "%s:  unable to open rgb database \"%s\"\n",
		 ProgramName, filename);
	free(path);
	exit (1);
    }

    while(fgets(line, sizeof(line), rgb)) {
	lineno++;
	if (sscanf(line, "%d %d %d %[^\n]\n", &red, &green, &blue, name) == 4) {
	    if (red >= 0 && red <= 0xff &&
		green >= 0 && green <= 0xff &&
		blue >= 0 && blue <= 0xff) {
		printf ("%3u %3u %3u\t\t%s\n", red, green, blue, name);
	    } else {
		fprintf(stderr, "%s:  value for \"%s\" out of range: %s:%d\n",
		        ProgramName, name, path, lineno);
	    }
	} else if (*line && *line != '!') {
	    fprintf(stderr, "%s:  syntax error: %s:%d\n", ProgramName,
		    path, lineno);
	}
    }

    free(path);
    fclose(rgb);
}

#endif /* USE_RGB_TXT */
