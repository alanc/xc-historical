/*
 * $XConsortium: process.c,v 1.5 88/12/01 16:46:57 jim Exp $
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

#ifndef DEFAULT_PROTOCOL
#define DEFAULT_PROTOCOL "MIT-MAGIC-COOKIE-1"
#endif

#ifndef DEFAULT_PROTOCOL_ABBREV
#define DEFAULT_PROTOCOL_ABBREV "."
#endif

typedef struct _AuthList {
    struct _AuthList *next;
    Xauth *auth;
} AuthList;

#define add_to_list(h,t,e) {if (t) (t)->next = (e); else (h) = (e); (t) = (e);}

typedef struct _CommandTable {		/* commands that are understood */
    char *name;				/* full name */
    int minlen;				/* unique prefix */
    int maxlen;				/* strlen(name) */
    int (*processfunc)();		/* handler */
} CommandTable;

struct _extract_data {
    FILE *fp;
    char *filename;
    int nwritten;
};


static int do_list(), do_merge(), do_extract(), do_add(), do_remove();
static int do_help(), do_source(), do_set(), do_info(), do_quit();
static int do_abort(), do_questionmark();

CommandTable command_table[] = {
    { "abort",   2, 5, do_abort },	/* abort */
    { "add",     2, 3, do_add },	/* add dpy proto hexkey */
    { "extract", 1, 7, do_extract },	/* extract filename dpy */
    { "help",    1, 4, do_help },	/* help */
    { "info",    1, 4, do_info },	/* info */
    { "list",    1, 4, do_list },	/* list [dpy] */
    { "merge",   1, 5, do_merge },	/* merge filename [filename ...] */
    { "quit",    1, 4, do_quit },	/* quit */
    { "remove",  1, 6, do_remove },	/* remove dpy */
    { "set",     1, 3, do_set },	/* set numeric [on/off] */
    { "source",  1, 6, do_source },	/* source filename */
    { "?",       1, 1, do_questionmark },  /* print xauth commands */
    { NULL,      0, 0, NULL },
};


static int do_set_numeric();

CommandTable set_table[] = {
    { "numeric", 1, 7, do_set_numeric },  /* set numeric {on,off} */
    { NULL,      0, 0, NULL },
};

#define COMMAND_NAMES_PADDED 10		/* widget than anything above */


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

static void baddisplayname (dpy, cmd)
    char *dpy, *cmd;
{
    fprintf (stderr, "bad display name \"%s\" in \"%s\" command\n",
	     dpy, cmd);
}

static void badcommandline (cmd)
    char *cmd;
{
    fprintf (stderr, "bad \"%s\" command line\n", cmd);
}

static void badset (cmd)
    char *cmd;
{
    fprintf (stderr, "bad \"%s\" parameter to set command\n", cmd);
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
	    argv = (char **) realloc (argv, total * sizeof (char *));
	    if (!argv) return NULL;
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
    Xauth proto;
    int prelen = 0;

    /*
     * check to see if the display name is of the form "host/unix:"
     * which is how the list routine prints out local connections
     */

    cp = index (displayname, '/');
    if (cp && strncmp (cp, "/unix:", 6) == 0)
      prelen = (cp - displayname);

    if (!parse_displayname (displayname + ((prelen > 0) ? prelen + 1 : 0),
			    &family, &host, &dpynum, &scrnum, &rest)) {
	return False;
    }

    proto.family = family;
    proto.address = get_address_info (family, displayname, prelen, host, &len);
    if (proto.address) {
	char buf[40];			/* want to hold largest display num */

	proto.address_length = len;
	buf[0] = '\0';
	sprintf (buf, "%d", dpynum);
	proto.number_length = strlen (buf);
	if (proto.number_length <= 0) {
	    free (proto.address);
	    proto.address = NULL;
	} else {
	    proto.number = copystring (buf, proto.number_length);
	}
    }

    if (host) free (host);
    if (rest) free (rest);

    if (proto.address) {
	auth->family = proto.family;
	auth->address = proto.address;
	auth->address_length = proto.address_length;
	auth->number = proto.number;
	auth->number_length = proto.number_length;
	auth->name = NULL;
	auth->name_length = 0;
	auth->data = NULL;
	auth->data_length = 0;
	return True;
    } else {
	return False;
    }
}

static void free_list (list)
    AuthList *list;
{
    while (list) {
	AuthList *next = list->next;

	XauDisposeAuth (list->auth);
	free ((char *) list);
	list = next;
    }
    return;
}


static int cvthexkey (hexstr, ptrp)
    char *hexstr;
    char **ptrp;
{
    int i;
    int len = 0;
    char *retval, *s;
    unsigned char *us;
    char savec;
    int whichchar;
    static char *hexdigits = "0123456789abcdef";

    /* convert to lower case and count */
    for (s = hexstr; *s; s++) {
	if (!isascii(*s)) return -1;
	if (isspace(*s)) continue;
	if (!isxdigit(*s)) return -1;
	len++;
    }

    /* if odd then there was an error */
    if ((len & 1) == 1) return -1;


    /* now we know that the input is good */
    len >>= 1;
    retval = malloc (len);
    if (!retval) {
	fprintf (stderr, "%s:  unable to allocate %d bytes for hexkey\n",
		 ProgramName, len);
	return -1;
    }

    whichchar = 0;
    for (us = (unsigned char *) retval, i = len; i > 0; hexstr++) {
	if (isspace(*hexstr)) continue;	 /* already know it is ascii */
	if (whichchar) {
#define atoh(c) ((c) - (((c) >= '0' && (c) <= '9') ? '0' : 'a'))
	    *us = (unsigned char)((atoh(savec) << 4) + atoh(*hexstr));
#undef atoh
	    whichchar = 0;		/* ready for next character */
	    us++;
	    i--;
	} else {
	    savec = *hexstr;
	    whichchar = 1;
	}
    }
    *ptrp = retval;
    return len;
}

static int dispatch_command (inputfilename, lineno, argc, argv, tab, statusp)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
    CommandTable *tab;
    int *statusp;
{
    CommandTable *ct;
    char *cmd;
    int n;
					/* scan table for command */
    cmd = argv[0];
    n = strlen (cmd);
    for (ct = tab; ct->name; ct++) {
					/* look for unique prefix */
	if (n >= ct->minlen && n <= ct->maxlen &&
	    strncmp (cmd, ct->name, n) == 0) {
	    *statusp = (*(ct->processfunc))(inputfilename, lineno, argc, argv);
	    return 1;
	}
    }

    *statusp = 1;
    return 0;
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
	printf ("Writing authority file \"%s\"\n", xauth_filename);
    }
    return 0;
}

int process_command (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    CommandTable *ct;
    int n, status;

    if (argc < 1 || !argv || !argv[0]) return 1;

    if (dispatch_command (inputfilename, lineno, argc, argv,
			  command_table, &status))
      return status;

    prefix (inputfilename, lineno);
    fprintf (stderr, "unknown command \"%s\"\n", argv[0]);
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

static int dump_entry (inputfilename, lineno, auth, data)
    char *inputfilename;
    int lineno;
    Xauth *auth;
    char *data;
{
    FILE *fp = (FILE *) data;

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
	    fwrite (auth->address, sizeof (char), auth->address_length, fp);
	    fprintf (fp, "/unix");
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
    return 0;
}

static void dumpauthlist (inputfilename, lineno, l)
    char *inputfilename;
    int lineno;
    register AuthList *l;
{
    for (; l; l = l->next) {
	dump_entry (inputfilename, lineno, l->auth, (char *) stdout);
    }
    return;
}

static int extract_entry (inputfilename, lineno, auth, data)
    char *inputfilename;
    int lineno;
    Xauth *auth;
    char *data;
{
    struct _extract_data *ed = (struct _extract_data *) data;

    if (!ed->fp) {
	ed->fp = fopen (ed->filename, "w");
	if (!ed->fp) {
	    prefix (inputfilename, lineno);
	    fprintf (stderr,
		     "unable to open extraction file \"%s\"\n",
		     ed->filename);
	    return -1;
	}
    }
    XauWriteAuth (ed->fp, auth);
    ed->nwritten++;

    return 0;
}


static int match_auth (a, b)
    register Xauth *a, *b;
{
    return ((a->family == b->family &&
	     a->address_length == b->address_length &&
	     a->number_length == b->number_length &&
	     bcmp (a->address, b->address, a->address_length) == 0 &&
	     bcmp (a->number, b->number, a->number_length) == 0) ? 1 : 0);
}


static int merge_entries (firstp, second)
    AuthList **firstp, *second;
{
    AuthList *a, *b, *first, *tail;
    int n = 0;

    if (!second) return 0;

    if (!*firstp) {			/* if nothing to merge into */
	*firstp = second;
	for (tail = *firstp, n = 0; tail->next; n++, tail = tail->next) ;
	return n;
    }

    first = *firstp;
    /*
     * find end of first list and stick second list on it
     */
    for (tail = first; tail->next; tail = tail->next) ;
    tail->next = second;

    /*
     * run down list freeing duplicate entries; if an entry is okay, then
     * bump the tail up to include it, otherwise, cut the entry out of
     * the chain.
     */
    for (b = second; b; ) {
	AuthList *next = b->next;	/* in case we free it */

	a = first;
	while (1) {
	    if (match_auth (a->auth, b->auth)) {  /* found a duplicate */
		AuthList tmp;		/* swap it in for old one */
		tmp = *a;
		*a = *b;
		*b = tmp;
		a->next = b->next;
		XauDisposeAuth (b->auth);
		free ((char *) b);
		b = NULL;
		tail->next = next;
		break;
	    }
	    if (a == tail) break;	/* if have looked at left side */
	    a = a->next;
	}
	if (b) {			/* if we didn't remove it */
	    tail = b;			/* bump end of first list */
	}
	b = next;
	n++;
    }

    return n;

}


static int iterdpy (inputfilename, lineno, start,
		    argc, argv, yfunc, nfunc, data)
    char *inputfilename;
    int lineno;
    int start;
    int argc;
    char *argv[];
    int (*yfunc)(), (*nfunc)();
    char *data;
{
    int i;
    int status;
    int errors = 0;
    Xauth proto;
    AuthList *l;

    /*
     * iterate
     */
    for (i = start; i < argc; i++) {
	char *displayname = argv[i];
	proto.address = proto.number = NULL;
	if (!get_displayname_auth (displayname, &proto)) {
	    prefix (inputfilename, lineno);
	    baddisplayname (displayname, argv[0]);
	    errors++;
	    continue;
	}
	status = 0;
	for (l = xauth_head; l; l = l->next) {
	    if (match_auth (&proto, l->auth)) {
		if (yfunc) {
		    status = (*yfunc) (inputfilename, lineno,
				       l->auth, data);
		    if (status < 0) break;
		}
	    } else {
		if (nfunc) {
		    status = (*nfunc) (inputfilename, lineno,
				       l->auth, data);
		    if (status < 0) break;
		}
	    }
	}
	if (proto.address) free (proto.address);
	if (proto.number) free (proto.number);
	if (status < 0) {
	    errors -= status;		/* since status is negative */
	    break;
	}
    }

    return errors;
}

static int remove_entry (listp, auth)
    AuthList **listp;
    Xauth *auth;
{
    AuthList *prev, *list;
    int removed = 0, notremoved = 0;

    if (!*listp) return -1;		/* if nothing to remove */

    for (prev = NULL, list = *listp; list; prev = list, list = list->next) {
	if (match_auth (list->auth, auth)) {
	    if (prev) prev->next = list->next;
	    XauDisposeAuth (list->auth);
	    free (list);
	    list = prev;		/* back it up before continuing */
	    removed++;
	} else
	  notremoved++;
    }

    if (notremoved == 0) *listp = NULL;
    return removed;
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
 * questionmark
 */
static int do_questionmark (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    CommandTable *ct;
    int i;
#define WIDEST_COLUMN 72
    int col = WIDEST_COLUMN;

    printf ("Commands:\n");
    for (ct = command_table; ct->name; ct++) {
	if ((col + ct->maxlen) > WIDEST_COLUMN) {
	    if (ct != command_table) {
		putc ('\n', stdout);
	    }
	    fputs ("        ", stdout);
	    col = 8;			/* length of string above */
	}
	fputs (ct->name, stdout);
	col += ct->maxlen;
	for (i = ct->maxlen; i < COMMAND_NAMES_PADDED; i++) {
	    putc (' ', stdout);
	    col++;
	}
    }
    if (col != 0) {
	putc ('\n', stdout);
    }

    /* allow bad lines since this is help */
    return 0;
}

/*
 * list [displayname ...]
 */
static int do_list (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    int i;
    AuthList *l;
    int family, dpynum, scrnum;
    char *host = NULL, *rest = NULL;
    Xauth proto;
    int errors = 0;

    if (argc == 1) {
	dumpauthlist (inputfilename, lineno, xauth_head);
	return 0;
    }

    return iterdpy (inputfilename, lineno, 1, argc, argv,
		    dump_entry, NULL, (char *) stdout);
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
    int errors = 0;
    AuthList *head, *tail, *listhead, *listtail;
    int nentries;

    if (argc < 2) {
	prefix (inputfilename, lineno);
	badcommandline (argv[0]);
	return 1;
    }

    listhead = listtail = NULL;

    for (i = 1; i < argc; i++) {
	char *filename = argv[i];
	FILE *fp;
	Bool used_stdin = False;

	if (strcmp (filename, "-") == 0) {
	    if (okay_to_use_stdin) {
		fp = stdin;
		okay_to_use_stdin = False;
		used_stdin = True;
		filename = "(stdin)";
	    } else {
		prefix (inputfilename, lineno);
		fprintf (stderr, "%s:  stdin already in use\n", argv[0]);
		errors++;
		continue;
	    }
	} else {
	    fp = fopen (filename, "r");
	}

	if (!fp) {
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "unable to open file \"%s\" for merging\n",
		     filename);
	    errors++;
	    continue;
	}

	head = tail = NULL;
	nentries = read_auth_entries (fp, format_numeric, &head, &tail);
	if (nentries == 0) {
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "unable to read any entries from file \"%s\"\n",
		     filename);
	    errors++;
	} else {			/* link it in */
	    add_to_list (listhead, listtail, head);
 	}

	if (!used_stdin) (void) fclose (fp);
    }

    /*
     * if we have new entries, merge them in (freeing any duplicates)
     */
    if (listhead) {
	nentries = merge_entries (&xauth_head, listhead);
	printf ("%d new entries read in.\n", nentries);
	if (nentries > 0) xauth_modified = True;
    }

    return 0;
}

/*
 * extract filename displayname [displayname ...]
 */
static int do_extract (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    Xauth proto;
    int errors;
    struct _extract_data ed;

    if (argc < 3) {
	prefix (inputfilename, lineno);
	badcommandline (argv[0]);
	return 1;
    }

    ed.fp = NULL;
    ed.filename = argv[1];
    ed.nwritten = 0;

    errors = iterdpy (inputfilename, lineno, 2, argc, argv, 
		      extract_entry, NULL, (char *) &ed);

    if (!ed.fp) {
	printf ("No matches found, authority file \"%s\" not written.\n",
		ed.filename);
    } else {
	printf ("%d entries written to \"%s\"\n", ed.nwritten, ed.filename);
	(void) fclose (ed.fp);
    }

    return errors;
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
    int n;
    int len;
    char *dpyname;
    char *protoname;
    char *hexkey;
    char *key;
    Xauth *auth;
    AuthList *list;

    if (argc != 4 || !argv[1] || !argv[2] || !argv[3]) {
	prefix (inputfilename, lineno);
	badcommandline (argv[0]);
	return 1;
    }

    dpyname = argv[1];
    protoname = argv[2];
    hexkey = argv[3];

    len = cvthexkey (hexkey, &key);
    if (len < 0) {
	prefix (inputfilename, lineno);
	fprintf (stderr,
		 "hexkey contains odd or non-hexidecimal characters\n");
	return 1;
    }

    auth = (Xauth *) malloc (sizeof (Xauth));
    if (!auth) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "unable to allocate %d bytes for Xauth structure\n",
		 sizeof (Xauth));
	free (key);
	return 1;
    }

    if (!get_displayname_auth (dpyname, auth)) {
	prefix (inputfilename, lineno);
	baddisplayname (dpyname, argv[0]);
	free (auth);
	free (key);
	return 1;
    }

    /*
     * allow an abbreviation for common protocol names
     */
    if (strcmp (protoname, DEFAULT_PROTOCOL_ABBREV) == 0) {
	protoname = DEFAULT_PROTOCOL;
    }

    auth->name_length = strlen (protoname);
    auth->name = copystring (protoname, auth->name_length);
    if (!auth->name) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "unable to allocate %d character protocol name\n",
		 auth->name_length);
	free (auth);
	free (key);
	return 1;
    }
    auth->data_length = len;
    auth->data = key;

    list = (AuthList *) malloc (sizeof (AuthList));
    if (!list) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "unable to allocate %d bytes for auth list\n",
		 sizeof (AuthList));
	free (auth);
	free (key);
	free (auth->name);
	return 1;
    }

    list->next = NULL;
    list->auth = auth;

    /*
     * merge it in; note that merge will deal with allocation
     */
    n = merge_entries (&xauth_head, list);
    if (n == 0) {
	prefix (inputfilename, lineno);
	fprintf (stderr, "unable to merge in added record\n");
	return 1;
    }

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
	badcommandline (argv[0]);
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
	badcommandline (argv[0]);
	return 1;
    }
    printf ("Authority file:  %s%s\n", 
	    xauth_filename ? xauth_filename : "(none)",
	    xauth_modified ? " (needs to be written out)" : "");
    printf ("Numeric format:  %s\n", format_numeric ? "on" : "off");
    printf ("Current input:   %s, line %d\n", inputfilename, lineno);
    return 0;
}

/*
 * set ...
 */
static int do_set (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    int status;

    if (argc < 2) {
	prefix (inputfilename, lineno);
	badcommandline (argv[0]);
	return 1;
    }

    if (dispatch_command (inputfilename, lineno, argc - 1, argv + 1,
			  set_table, &status))
      return status;

    prefix (inputfilename, lineno);
    fprintf (stderr, "unknown set parameter \"%s\"\n", argv[1]);
    return 1;
}

static int do_set_numeric (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    if (argc != 2) {
	prefix (inputfilename, lineno);
	badset (argv[0]);
	return 1;
    }

    switch (parse_boolean (argv[1])) {
      case 0:
	format_numeric = False;
	break;
      case 1:
	format_numeric = True;
	break;
      default:
	prefix (inputfilename, lineno);
	fprintf (stderr, "numeric takes \"on\" or \"off\"\n");
	return 1;
    }

    printf ("Numeric mode turned %s\n", format_numeric ? "on" : "off");
    return 0;
}


/*
 * quit
 */
static Bool quit = False;

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
 * abort
 */
static int do_abort (inputfilename, lineno, argc, argv)
    char *inputfilename;
    int lineno;
    int argc;
    char **argv;
{
    /* allow bogus stuff */
    exit (0);
    /* NOTREACHED */
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
	badcommandline (argv[0]);
	return 1;
    }

    script = argv[1];

    if (strcmp (script, "-") == 0) {
	if (okay_to_use_stdin) {
	    fp = stdin;
	    okay_to_use_stdin = False;
	    used_stdin = True;
	    script = "(stdin)";
	} else {
	    prefix (inputfilename, lineno);
	    fprintf (stderr, "%s:  stdin already in use\n", argv[0]);
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
	    printf ("xauth> ");
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
