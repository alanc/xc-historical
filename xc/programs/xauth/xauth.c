/*
 * xauth - manipulate authorization file
 *
 * If no command is given on the command line, the standard input will
 * be read.
 */

#include "xauth.h"

/*
 * global data
 */
char *ProgramName;			/* argv[0], set at top of main() */
Bool format_numeric = False;		/* dump entries in hex */


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
void print_help ()
{
    static char *help[] = {
"",
"where options include:",
"    -f authfilename           name of authority file to use (must be first)",
"    -n                        start in numeric mode",
"",
"and commands have the following syntax",
"    add dpyname protoname hexkey      add entry to authority file",
"    extract filename dpyname...       extract auth entries into file",
"    list [dpyname...]                 list auth entries",
"    merge filename...                 merge in cookies from given files",
"    remove dpyname...                 remove entry for given display",
"    set numeric {on,off}              turn numeric mode on or off",
"    source filename                   read command from the given file",
"    info                              print out info about inputs",
"    quit                              exit program (same as end of file)",
"    abort                             exit program and abort any changes",
"    help                              display this message",
"",
"If a single dash is specified as a filename for the merge or source commands",
"the data will be read from the standard input.  If a single dash is",
"specified as the filename for the extract command the data will be written",
"to the standard output.",
"",
NULL };
    char **msg;

    fprintf (stderr, "usage:  %s [-options ...] [command arg ...]\n",
	     ProgramName);
    for (msg = help; *msg; msg++) {
	fprintf (stderr, "%s\n", *msg);
    }
    return;
}

static void usage ()
{
    print_help ();
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
	    switch (arg[1]) {
	      case 'f':			/* -f authfilename */
		if (++i >= argc) usage ();
		if (authfilename) {
		    fprintf (stderr,
			     "%s:  authfilename already specified as \"%s\"\n",
			     ProgramName, authfilename);
		    exit (1);
		}
		authfilename = argv[i];
		continue;
	      case 'n':			/* -n */
		format_numeric = True;
		continue;
	      default:
		usage ();
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
	fprintf (stderr, "%s:  unable to initialize authority file \"%s\"\n",
		 ProgramName, authfilename);
	exit (1);
    }

    status = process_command (sourcename, 1, nargs, arglist);

    (void) auth_finalize ();
    exit ((status != 0) ? 1 : 0);
}


