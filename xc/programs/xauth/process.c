/*
 * $XConsortium$
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

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "xauth.h"
#include <X11/X.h>

extern int errno;			/* for stupid errno.h files */

#define DEFAULT_PROTOCOL "MIT-MAGIC-COOKIE-1"

typedef struct _AuthList {
    struct _AuthList *next;
    Xauth *auth;
} AuthList;

static int do_list(), do_merge(), do_extract(), do_add(), do_remove();
static int do_help(), do_source(), do_numeric(), do_info(), do_quit();

struct _cmdtab {			/* commands that are understood */
    char *name;				/* full name */
    int minlen;				/* unique prefix */
    int maxlen;				/* strlen(name) */
    int (*processfunc)();		/* handler */
} command_table[] = {
    { "add",     1, 5, do_add },	/* add dpy proto hexkey */
    { "extract", 1, 7, do_extract },	/* extract filename dpy */
    { "help",    1, 4, do_help },	/* help */
    { "info",    1, 4, do_info },	/* info */
    { "list",    1, 4, do_list },	/* list [dpy] */
    { "merge",   1, 5, do_merge },	/* merge filename [filename ...] */
    { "numeric", 1, 7, do_numeric },	/* numeric [on/off] */
    { "quit",    1, 4, do_quit },	/* quit */
    { "remove",  1, 6, do_remove },	/* remove dpy */
    { "source",  1, 6, do_source },	/* source filename */
    { "?",       1, 1, do_help },	/* synonym for help */
    { NULL,      0, 0, NULL },
};

static Bool okay_to_use_stdin = True;	/* set to false after using */

static char *hex_table[] = {		/* for printing hex digits */
    "00", "01", "02", "03", "04", "05", "06", "07", 
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f", 
    "10", "11", "12", "13", "14", "15", "16", "17", 
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f", 
    "20", "21", "22", "23", "24", "25", "26", "27", 
    "28", "29", "2a", "2b", "2c", "2d", "2e", "2f", 
    "30", "31", "32", "33", "34", "35", "36", "37", 
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f", 
    "40", "41", "42", "43", "44", "45", "46", "47", 
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f", 
    "50", "51", "52", "53", "54", "55", "56", "57", 
    "58", "59", "5a", "5b", "5c", "5d", "5e", "5f", 
    "60", "61", "62", "63", "64", "65", "66", "67", 
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f", 
    "70", "71", "72", "73", "74", "75", "76", "77", 
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f", 
    "80", "81", "82", "83", "84", "85", "86", "87", 
    "88", "89", "8a", "8b", "8c", "8d", "8e", "8f", 
    "90", "91", "92", "93", "94", "95", "96", "97", 
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f", 
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af", 
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", 
    "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf", 
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", 
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf", 
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", 
    "d8", "d9", "da", "db", "dc", "dd", "de", "df", 
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", 
    "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef", 
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", 
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff", 
};

extern char *get_hostname();
extern Bool nameserver_timedout;


/*
 * private utility procedures
 */

static void prefix (fn, n)
    char *fn;
    int n;
{
    fprintf (stderr, "%s: line %d of \"%s\":  ", ProgramName, n, fn);
}

static int parse_boolean (s)
    char *s;
{
    static char *true_names[] = { "on", "t", "true", "yes", "y", NULL };
    static char *false_names[] = { "off", "f", "false", "no", "n", NULL };
    register char **cpp;

    if (s) {
	for (cpp = false_names; *cpp; cpp++) {
	    if (strcmp (s, *cpp) == 0) return 0;
	}
	for (cpp = true_names; *cpp; cpp++) {
	    if (strcmp (s, *cpp) == 0) return 1;
	}
    }
    return -1;
}

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

    /* put quoting into loop if need be */
    for (; (c = *s) && isascii(c) && !isspace(c); s++) ;
    return s;
}

static char **split_into_words (src, argcp)
    char *src;
    int *argcp;
{
    char *word;
    char savec;
    char **argv;
    int cur, total;

    *argcp = 0;
#define WORDSTOALLOC 4			/* most lines are short */
    argv = (char **) malloc (WORDSTOALLOC * sizeof (char *));
    if (!argv) return NULL;
    cur = 0;
    total = WORDSTOALLOC;

    /*
     * split the line up into separate, nul-terminated tokens; the last
     * "token" will point to the empty string so that it can be bashed into
     * a null pointer.
     */

    do {
	word = skip_space (src);
	src = skip_nonspace (word);
	savec = *src;
	*src = '\0';
	if (cur == total) {
	    total += WORDSTOALLOC;
	    cur = realloc (argv, total * sizeof (char *));
	    if (!cur) return NULL;
	}
	argv[cur++] = word;
	if (savec) src++;		/* if not last on line advance */
    } while (word != src);

    argv[--cur] = NULL;			/* smash empty token to end list */
    *argcp = cur;
    return argv;
}


static Xauth *read_numeric (fp)
    FILE *fp;
{
    /* XXX */
    return NULL;
}

static int read_auth_entries (fp, numeric, headp, tailp)
    FILE *fp;
    Bool numeric;
    AuthList **headp, **tailp;
{
    Xauth *((*readfunc)()) = (numeric ? read_numeric : XauReadAuth);
    Xauth *auth;
    AuthList *head, *tail;
    int n;

    head = tail = NULL;
    n = 0;
					/* put all records into linked list */
    while ((auth = ((*readfunc) (fp))) != NULL) {
	AuthList *l = (AuthList *) malloc (sizeof (AuthList));
	if (!l) {
	    fprintf (stderr,
		     "%s:  unable to alloc entry while reading auth file\n",
		     ProgramName);
	    exit (1);
	}
	l->next = NULL;
	l->auth = auth;
	if (tail) 			/* if not first time through append */
	  tail->next = l;
	else
	  head = l;			/* first time through, so assign */
	tail = l;
	n++;
    }
    *headp = head;
    *tailp = tail;
    return n;
}

static Bool get_displayname_auth (displayname, auth)
    char *displayname;
    Xauth *auth;			/* fill in */
{
    int family;
    char *host = NULL, *rest = NULL;
    int dpynum, scrnum;
    char *cp;
    int len;
    extern char *get_address_info();

    if (!parse_displayname (displayname, &family, &host, &dpynum, &scrnum,
			    &rest)) {
	return False;
    }

    auth->family = family;
    auth->address = get_address_info (family, host, &len);
    if (auth->address) {
	char buf[40];			/* want to hold largest display num */

	auth->address_length = len;
	buf[0] = '\0';
	sprintf (buf, "%d", dpynum);
	auth->number_length = strlen (buf);
	if (auth->number_length > 0) 
	  auth->number = copystring (buf, auth->number_length);
    }

    if (host) free (host);
    if (rest) free (rest);
    return (auth->address ? True : False);
}

	
/*
 * public procedures for parsing lines of input
 */

static FILE *authfp = NULL;
static AuthList *xauth_head = NULL;	/* list of auth entries */
static Bool xauth_modified = False;
static char *xauth_filename = NULL;

int auth_initialize (authfilename)
    char *authfilename;
{
    int n;
    AuthList *head, *tail;

    authfp = fopen (authfilename, "r");
    if (!authfp) {
	int olderrno = errno;

					/* if file there then error */
	if (access (authfilename, F_OK) == 0) {	 /* then file does exist! */
	    errno = olderrno;
	    return -1;
	}				/* else ignore it */
    } else {
	n = read_auth_entries (authfp, False, &head, &tail);
	if (n < 1) {
	    fprintf (stderr,
		     "%s:  unable to read auth entries from file \"%s\"\n",
		     ProgramName, authfilename);
	    return -1;
	}
	xauth_head = head;
    }

    n = strlen (authfilename);
    xauth_filename = malloc (n + 1);
    if (xauth_filename) strcpy (xauth_filename, authfilename);
    xauth_modified = False;
    return 0;
}

int auth_finalize ()
{
    if (xauth_modified) {
	/* XXX - need to write stuff back out */
    }
    return 0;
}

int process_command (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    struct _cmdtab *ct;
    int n, status;
    char *cmd;

    if (argc < 1 || !argv || !argv[0]) return 1;

					/* scan table for command */
    cmd = argv[0];
    n = strlen (cmd);
    for (ct = command_table; ct->name; ct++) {
					/* look for unique prefix */
	if (n >= ct->minlen && n <= ct->maxlen &&
	    strncmp (cmd, ct->name, n) == 0) {
	    status = (*(ct->processfunc)) (inputfilename, lineno, argc, argv);
	    return status;
	}
    }

    prefix (inputfilename, lineno);
    fprintf (stderr, "unknown command \"");
    fwrite (cmd, sizeof (char), n, stderr);
    fprintf (stderr, "\"\n");
    return 1;
}


/*
 * utility routines
 */

static void fprintfhex (fp, len, cp)
    register FILE *fp;
    int len;
    char *cp;
{
    unsigned char *ucp = (unsigned char *) cp;

    for (; len > 0; len--, ucp++) {
	register char *s = hex_table[*ucp];
	putc (s[0], fp);
	putc (s[1], fp);
    }
    return;
}

static void dump_entry (fp, auth)
    FILE *fp;
    Xauth *auth;
{
    if (format_numeric) {
	fprintf (fp, "%04x", auth->family);  /* unsigned short */
	fprintf (fp, " %04x ", auth->address_length);  /* short */
	fprintfhex (fp, auth->address_length, auth->address);
	fprintf (fp, " %04x ", auth->number_length);  /* short */
	fprintfhex (fp, auth->number_length, auth->number);
	fprintf (fp, " %04x ", auth->name_length);  /* short */
	fprintfhex (fp, auth->name_length, auth->name);
	fprintf (fp, " %04x ", auth->data_length);  /* short */
	fprintfhex (fp, auth->data_length, auth->data);
    } else {
	char *dpyname = NULL;
	char numbuf[10];

	switch (auth->family) {
	  case FamilyLocal:
	    putc ('/', fp);
	    fwrite (auth->address, sizeof (char), auth->address_length, fp);
	    putc ('/', fp);
	    break;
	  case FamilyInternet:
	  case FamilyDECnet:
	    dpyname = get_hostname (auth);
	    if (dpyname) {
		fprintf (fp, "%s", dpyname);
		break;
	    }
	    /* else fall through to default */
	  default:
	    fprintf (fp, "#%04x#", auth->family);
	    fprintfhex (fp, auth->address_length, auth->address);
	    putc ('#', fp);
	}
	putc (':', fp);
	fwrite (auth->number, sizeof (char), auth->number_length, fp);
	putc (' ', fp);
	putc (' ', fp);
	fwrite (auth->name, sizeof (char), auth->name_length, fp);
	putc (' ', fp);
	putc (' ', fp);
	fprintfhex (fp, auth->data_length, auth->data);
    }
    putc ('\n', fp);
    return;
}


/*
 * action routines
 */

/*
 * help
 */
static int do_help (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    /* allow bad lines since this is help */
    print_help ();
    return 0;
}

/*
 * list [displayname]
 */
static int do_list (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    AuthList *l;
    Bool specific_display = False;
    int family, dpynum, scrnum;
    char *host = NULL, *rest = NULL;
    Xauth proto;

    if (argc > 2 || (argc == 2 && !argv[1])) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"list\" command line\n");
	return 1;
    }

    if (argc == 2) {
	if (!get_displayname_auth (argv[1], &proto)) {
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "bad display name \"%s\" in \"list\"\n", argv[1]);
	    return 1;
	}
	specific_display = True;
    }

    for (l = xauth_head; l; l = l->next) {
	if (specific_display /* XXX - && !match(,l) */) continue;
	dump_entry (stdout, l->auth);
    }

    /* XXX */
    return 0;
}

/*
 * merge filename [filename ...]
 */
static int do_merge (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    int i;
    AuthList *head, *tail, *list;

    if (argc < 2) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"merge\" command line\n");
	return 1;
    }

    return 0;
}

/*
 * extract filename displayname
 */
static int do_extract (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    if (argc != 3 || !argv[1] || !argv[2]) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"extract\" command line\n");
	return 1;
    }
    /* XXX */
    return 0;
}

/*
 * add displayname protocolname hexkey
 */
static int do_add (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{ 
    if (argc != 4 || !argv[1] || !argv[2] || !argv[3]) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"add\" command line\n");
	return 1;
    }
    /* XXX */
    return 0;
}

/*
 * remove displayname
 */
static int do_remove (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    if (argc != 2 || !argv[1]) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"remove\" command line\n");
	return 1;
    }
    /* XXX */
    return 0;
}

/*
 * info
 */
static int do_info (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    if (argc != 1) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"info\" command line\n");
	return 1;
    }
    printf ("Authority file:  %s\n", 
	    xauth_filename ? xauth_filename : "(none)");
    printf ("Numeric format:  %s\n", format_numeric ? "on" : "off");
    printf ("Current input:   %s, line %d\n", inputfilename, lineno);
    return 0;
}

/*
 * numeric [on/off]
 */
static int do_numeric (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    switch (argc) {
      case 1:				/* numeric - toggle */
	format_numeric = (!format_numeric);
	break;
      case 2:
	switch (parse_boolean (argv[1])) {
	  case 0:
	    format_numeric = False;
	    break;
	  case 1:
	    format_numeric = True;
	    break;
	  default:
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "use \"on\" or \"off\" with numeric\n");
	    return 1;
	}
	break;
      default:
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"numeric\" command line\n");
	return 1;
    }
    return 0;
}

static Bool quit = False;

/*
 * quit
 */
static int do_quit (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    /* allow bogus stuff */
    quit = True;
    return 0;
}


/*
 * source filename
 */
static int do_source (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    char *script;
    char buf[BUFSIZ];
    FILE *fp;
    Bool used_stdin = False;
    int len;
    int errors = 0, status;
    int sublineno = 0;
    char **subargv;
    int subargc;
    Bool prompt = False;		/* only true if reading from tty */

    if (argc != 2 || !argv[1]) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "bad \"source\" command line\n");
	return 1;
    }

    script = argv[1];

    if (strcmp (script, "-") == 0) {
	if (okay_to_use_stdin) {
	    fp = stdin;
	    okay_to_use_stdin++;
	    used_stdin = True;
	    script = "(stdin)";
	} else {
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "stdin already in use\n");
	    return 1;
	}
    } else {
	fp = fopen (script, "r");
	if (!fp) {
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "unable to open script file \"%s\"\n", script);
	    return 1;
	}
    }

    if (isatty (fileno (fp))) prompt = True;

    while (!quit) {
	buf[0] = '\0';
	if (prompt) {
	    printf ("\r\nxauth> ");
	    fflush (stdout);
	}
	if (fgets (buf, sizeof buf, fp) == NULL) break;
	sublineno++;
	len = strlen (buf);
	if (len == 0 || buf[0] == '#') continue;
	if (buf[len-1] != '\n') {
	    prefix (script, sublineno);
	    fprintf (stderr, "line too long\n");
	    errors++;
	    break;
	}
	buf[--len] = '\0';		/* remove new line */
	subargv = split_into_words (buf, &subargc);
	if (argv) {
	    status = process_command (script, sublineno, subargc, subargv);
	    free ((char *) subargv);
	    errors += status;
	} else {
	    prefix (script, sublineno);
	    fprintf (stderr, "unable to break line into words\n");
	    errors++;
	}
    }

    if (!used_stdin) {
	(void) fclose (fp);
    }
    return errors;
}
