#include "xauth.h"
#include <ctype.h>

static int do_list(), do_search(), do_remove();
static int do_merge(), do_create(), do_fcreate();

struct _cmdtab {			/* commands that are understood */
    char *name;
    int length;
    int (*processfunc)();
} command_table[] = {
    { "list", 4, do_list },
    { "search", 6, do_search },
    { "remove", 6, do_remove },
    { "merge", 5, do_merge },
    { "create", 6, do_create },
    { "fcreate", 7, do_fcreate },
    { NULL, NULL },
};


/*
 * private utility procedures
 */

static char *skip_space (s)
    register char *s;
{
    register char c;

    if (!s) return NULL;

    for (; (c = *s) && isascii(c) && isspace(c); s++) ;
    return s;
}


static char *skip_nonspace (s)
    register char *s;
{
    register char c;

    if (!s) return NULL;

    for (; (c = *s) && isascii(c) && !isspace(c); s++) ;
    return s;
}


/*
 * public procedures for parsing lines of input
 */

void initialize_auth ()
{
    /* XXX */
    return;
}

int process_command (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    struct _cmdtab *ct;
    int n, status;
    char *cp;

    cp = skip_space (cmd);
    if (!cp) return 1;
    cmd = cp;

    cp = skip_nonspace (cmd);
    if (!cp) return 1;
    n = (cp - cmd);

    for (ct = command_table; ct->name; ct++) {
	if (n == ct->length && strncmp (cmd, ct->name, n) == 0) {
	    cmd = cp;
	    cp = skip_space (cmd);
	    status = (*(ct->processfunc)) (inputfilename, lineno, cp);
	    return status;
	}
    }

    fprintf (stderr, "%s:  unknown command \"", ProgramName);
    fwrite (cmd, sizeof (char), n, stderr);
    fprintf (stderr, "\" on line %d of file \"%s\"\n", lineno,
	     inputfilename);
    return 1;
}

int process_script (script)
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


/*
 * private procedures 
 */

/*
 * list
 */
static int do_list (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    /* XXX */
    return 0;
}

/*
 * search [displayname]
 */
static int do_search (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    /* XXX */
    return 0;
}

/*
 * remove displayname
 */
static int do_remove (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{ 
    /* XXX */
   return 0;
}

/*
 * merge filename ...
 */
static int do_merge (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    /* XXX */
    return 0;
}

/*
 * create displayname protocolname key
 */
static int do_create (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    /* XXX */
    return 0;
}

/*
 * fcreate displayname protocolname file
 */
static int do_fcreate (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    /* XXX */
    return 0;
}


