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
 *     xauth [-f authfile] [[-c "command arg ..."] [-s scriptfile] ...]
 *
 * where commands include
 *
 *     list
 *     search [displayname]
 *     remove displayname
 *     merge filename ...
 *     create displayname protocolname key
 */

#include "xauth.h"

char *ProgramName;
char *authfile = NULL;
FILE *authfp = NULL;
int process_command(), process_script();

static void usage ()
{
    static char *help[] = {
"",
"where options include:",
"    -f authfile               name of authority file to use (must be first)",
"    -c \"command [arg ...]\"    command to perform",
"    -s scriptfile             file containing commands to perform",
"",
"and commands have the following syntax",
"    list                      list displays in authority file",
"    search [displayname]      search for cookie for given display or for all",
"    remove displayname        remove cookie for given display",
"    merge filename ...        merge in cookies from given files",
"    create displayname protocolname key      create a new key",
"    fcreate displayname protocolname file    create new key from file",
"",
"If a single dash is specified as a filename for the merge or fcreate",
"commands the standard input will be read.",
"",
NULL };
    char **msg;

    fprintf (stderr, "usage:  %s [-options ...]\n", ProgramName);
    for (msg = help; *msg; msg++) {
	fprintf (stderr, "%s\n", *msg);
    }
    exit (1);
}

main (argc, argv)
    int argc;
    char *argv[];
{
    int i;
    int errors = 0;

    ProgramName = argv[0];

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    switch (arg[1]) {
	      case 'f':			/* -f authfile */
		if (++i >= argc) usage ();
		if (authfile) {
		    fprintf (stderr,
			     "%s:  authfile already specified as \"%s\"\n",
			     ProgramName, authfile);
		    exit (1);
		}
		authfile = argv[i];
		continue;
	      case 'c':			/* -c "command arg" */
		if (++i >= argc) usage ();
		errors += process_command (argv[i]);
		continue;
	      case 's':			/* -s script */
		if (++i >= argc) usage ();
		errors += process_script (argv[i]);
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


int process_command (cmd)
    char *cmd;
{
    return 0;
}

int process_script (script)
    char *script;
{
    return 0;
}
