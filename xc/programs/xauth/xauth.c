/*
 * $XConsortium: process.c,v 1.11 88/12/11 18:05:09 jim Exp $
 *
 * xauth - manipulate authorization file
 *
 * Copyright 1988 Massachusetts Institute of Technology
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

#include "xauth.h"


/*
 * global data
 */
char *ProgramName;			/* argv[0], set at top of main() */
Bool verbose = True;			/* print certain messages */
Bool ignore_locks = False;		/* for error recovery */
Bool break_locks = False;		/* for error recovery */

/*
 * local data
 */

static char *authfilename = NULL;	/* filename of cookie file */
static int do_script ();		/* for reading scripts */
static char *defcmds[] = { "source", "-", NULL };  /* default command */
static int ndefcmds = 2;
static char *defsource = "(stdin)";

/*
 * utility routines
 */
void print_help (printall)
    Bool printall;
{
    static char *help[] = {
"",
"where options include:",
"    -f authfilename           name of authority file to use",
"    -q                        print no unsolicited messages",
"    -i                        ignore locks on authority file",
"    -b                        break locks on authority file",
"",
"and commands have the following syntax",
NULL };
    static char *cmds[] = {
"    add dpyname protoname hexkey      add entry to authority file",
"    [n]extract filename dpyname...    extract auth entries into file",
"    [n]list [dpyname...]              list auth entries",
"    [n]merge filename...              merge in cookies from given files",
"    remove dpyname...                 remove entry for given display",
"    source filename                   read command from the given file",
"    info                              print out info about inputs",
"    exit                              exit program (same as end of file)",
"    quit                              exit program and abort any changes",
"    help                              display this message",
"",
"A dash may be used with the \"merge\" and \"source\" to read from the",
"standard input.  Commands beginning with [n] use numeric format.",
"",
NULL };
    char **msg;

    if (printall) {
	fprintf (stderr, "usage:  %s [-options ...] [command arg ...]\n",
		 ProgramName);
	for (msg = help; *msg; msg++) {
	    fprintf (stderr, "%s\n", *msg);
	}
    }
    for (msg = cmds; *msg; msg++) {
	fprintf (stderr, "%s\n", *msg);
    }
    return;
}

static void usage ()
{
    print_help (True);
    exit (1);
}


/*
 * The main routine - parses command line and calls action procedures
 */
main (argc, argv)
    int argc;
    char *argv[];
{
    int i;
    char *sourcename = defsource;
    char **arglist = defcmds;
    int nargs = ndefcmds;
    int status;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    char *flag;

	    for (flag = (arg + 1); *flag; flag++) {
		switch (*flag) {
		  case 'f':			/* -f authfilename */
		    if (++i >= argc) usage ();
		    authfilename = argv[i];
		    continue;
		  case 'q':			/* -q */
		    verbose = False;
		    continue;
		  case 'b':			/* -b */
		    break_locks = True;
		    continue;
		  case 'i':			/* -i */
		    ignore_locks = True;
		    continue;
		  default:
		    usage ();
		}
	    }
	} else {
	    sourcename = "(argv)";
	    nargs = argc - i;
	    arglist = argv + i;
	    break;
	}
    }

    if (!authfilename) {
	authfilename = XauFileName ();	/* static name, do not free */
	if (!authfilename) {
	    fprintf (stderr,
		     "%s:  unable to generate an authority file name\n",
		     ProgramName);
	    exit (1);
	}
    }
    if (auth_initialize (authfilename) != 0) {
	/* error message printed in auth_initialize */
	exit (1);
    }

    status = process_command (sourcename, 1, nargs, arglist);

    (void) auth_finalize ();
    exit ((status != 0) ? 1 : 0);
}


