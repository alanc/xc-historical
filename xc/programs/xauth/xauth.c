/*
 * xauth - manipulate authorization file
 *
 * This program allows you to perform the following functions:
 *
 *     - list the contents of an authorization file
 *     - search for cookies so that they can be merged into another host
 *     - remove cookies from an authorization file
 *     - merge cookies into an authorization file
 *
 *
 * The general syntax is 
 *
 *     xauth [-f authfilename] [[-c "command arg ..."] [-s scriptfile] ...]
 *
 * where commands include
 *
 *     list [displayname]
 *     merge filename [filename ...]
 *     extract filename displayname
 *     add displayname protocolname hexkey
 *     remove displayname
 */

#include "xauth.h"

/*
 * global data
 */
char *ProgramName;			/* argv[0], set at top of main() */
Bool okay_to_use_stdin = True;		/* set to false after using */
Bool print_numeric = False;		/* dump entries in hex */


/*
 * local data
 */

static char *authfilename = NULL;	/* filename of cookie file */
static int do_script ();		/* for reading scripts */

/*
 * utility routines
 */
static void usage ()
{
    static char *help[] = {
"",
"where options include:",
"    -f authfilename           name of authority file to use (must be first)",
"    -n                        print entries in numeric form",
"    -c \"command [arg ...]\"    command to perform",
"    -s scriptfile             file containing commands to perform",
"",
"and commands have the following syntax",
"    list [displayname]        list display(s) in authority file",
"    merge filename ...        merge in cookies from given files",
"    extract filename displayname  extract binary entry for display into file",
"    add displayname protocolname hexkey  add entry",
"    remove displayname        remove entry for given display",
"",
"If a single dash is specified as a filename for the merge command the data",
"will be read from the standard input.  If a single dash is specified as the",
"filename for the extract command the data will be written to the standard",
"output.",
"",
NULL };
    char **msg;

    fprintf (stderr, "usage:  %s [-options ...]\n", ProgramName);
    for (msg = help; *msg; msg++) {
	fprintf (stderr, "%s\n", *msg);
    }
    exit (1);
}


static void initialize (fn)
    char *fn;
{
    char *authfilename = NULL;

    if (!fn) {
	authfilename = XauFileName ();	/* static name, do not free */
	if (!authfilename) {
	    fprintf (stderr,
		     "%s:  unable to generate an authority file name\n",
		     ProgramName);
	    exit (1);
	}
    } else {
	authfilename = fn;
    }

    if (auth_initialize (authfilename) != 0) {
	fprintf (stderr, "%s:  unable to initialize authority file \"%s\"\n",
		 ProgramName, authfilename);
	exit (1);
    }

    return;
}


/*
 * The main routine - parses command line and calls action procedures
 */
main (argc, argv)
    int argc;
    char *argv[];
{
    int i;
    int errors = 0;
    int cmdarg = 0;
    Bool been_initialized = False;

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
	      case 'c':			/* -c "command arg" */
		if (++i >= argc) usage ();
		if (!been_initialized) {
		    initialize (authfilename);
		    been_initialized = True;
		}
		errors += process_command ("argv[]", ++cmdarg, argv[i]);
		continue;
	      case 's':			/* -s script */
		if (++i >= argc) usage ();
		if (!been_initialized) {
		    initialize (authfilename);
		    been_initialized = True;
		}
		errors += do_script (argv[i]);
		continue;
	      case 'n':			/* -n */
		print_numeric = True;
		continue;
	      default:
		usage ();
	    }
	} else {
	    usage ();
	}
    }

    exit (errors == 0 ? 0 : 1);
}


static int do_script (script)
    char *script;
{
    char buf[BUFSIZ];
    FILE *fp;
    Bool used_stdin = False;
    int lineno = 0;
    int len;
    int errors = 0, status;

    if (strcmp (script, "-") == 0) {
	if (okay_to_use_stdin) {
	    fp = stdin;
	    okay_to_use_stdin++;
	    used_stdin = True;
	} else {
	    fprintf (stderr,
	     "%s:  stdin has already been used, can't reuse for script\n",
		     ProgramName);
	    return 1;
	}
    } else {
	fp = fopen (script, "r");
	if (!fp) {
	    fprintf (stderr, "%s:  unable to open script file \"%s\"\n",
		     ProgramName, script);
	    return 1;
	}
    }

    while (1) {
	buf[0] = '\0';
	if (fgets (buf, sizeof buf, fp) == NULL) break;
	lineno++;
	len = strlen (buf);
	if (len == 0 || buf[0] == '#') continue;
	if (buf[len-1] != '\n') {
	    fprintf (stderr, "%s:  line %d of script \"%s\" too long.\n",
		     ProgramName, lineno, script);
	    errors++;
	    break;
	}
	buf[--len] = '\0';		/* remove new line */
	errors += process_command (script, lineno, buf);
    }

    if (!used_stdin) {
	(void) fclose (fp);
    }
    return errors;
}

