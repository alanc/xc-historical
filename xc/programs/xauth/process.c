#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#ifdef DNETCONN
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif
#include "xauth.h"
#include <X11/X.h>

extern int errno;			/* for stupid errno.h files */

typedef struct _AuthList {
    struct _AuthList *next;
    Xauth *auth;
} AuthList;

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

static int do_list(), do_merge(), do_extract(), do_add(), do_remove();
static char *get_hostname();

struct _cmdtab {			/* commands that are understood */
    char *name;				/* full name */
    int minlen;				/* unique prefix */
    int maxlen;				/* strlen(name) */
    int (*processfunc)();		/* handler */
} command_table[] = {
    { "list",    1, 4, do_list },	/* list [dpy] */
    { "merge",   1, 5, do_merge },	/* merge filename [filename ...] */
    { "extract", 1, 7, do_extract },	/* extract filename dpy */
    { "add",     1, 5, do_add },	/* add dpy proto hexkey */
    { "remove",  1, 6, do_remove },	/* remove dpy */
    { NULL, 0, 0, NULL },
};

static Bool nameserver_timedout = False;


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

static FILE *authfp = NULL;
static AuthList *head;			/* list of auth entries */
int nauths = 0;				/* count of auth entries */

static void read_auth_entries (fp)
    FILE *fp;
{
    Xauth *auth;
    AuthList *tail;

    head = tail = NULL;
    nauths = 0;
					/* put all records into linked list */
    while ((auth = XauReadAuth (fp)) != NULL) {
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
	nauths++;
    }
    return;
}
	

int auth_initialize (authfilename)
    char *authfilename;
{
    authfp = fopen (authfilename, "r");
    if (!authfp) {
	int olderrno = errno;

	if (access (authfilename, F_OK) == 0) {	 /* then file does exist! */
	    errno = olderrno;
	    return -1;
	}
    } else {
	read_auth_entries (authfp);
    }

    return 0;
}

int process_command (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    struct _cmdtab *ct;
    int n, status;
    char *cp;

    cp = skip_space (cmd);		/* skip space to start of command */
    if (!cp) return 1;
    cmd = cp;

    cp = skip_nonspace (cmd);		/* skip command */
    if (!cp) return 1;
    n = (cp - cmd);			/* length of token */

					/* scan table for command */
    for (ct = command_table; ct->name; ct++) {
					/* look for unique prefix */
	if (n >= ct->minlen && n <= ct->maxlen &&
	    strncmp (cmd, ct->name, n) == 0) {
	    cmd = cp;
	    cp = skip_space (cmd);
	    status = (*(ct->processfunc)) (inputfilename, lineno, cp);
	    return status;
	}
    }

    fprintf (stderr, "%s:  unknown command \"", ProgramName);
    fwrite (cmd, sizeof (char), n, stderr);
    fprintf (stderr, "\" on line %d of file \"%s\"\n", lineno, inputfilename);
    return 1;
}


/*
 * private procedures 
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
    if (print_numeric) {
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
	    putc ('/', fp);
#ifdef UNIXCONN
	    fprintf (fp, "unix");
#else
	    fprintf (fp, "localhost");
#endif
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
 * list [displayname]
 */
static int do_list (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    AuthList *l;
    char *cp;
    Bool specific_display = False;

    cp = skip_space (cmd);
    if (!cp) return 1;
    cmd = cp;

    if (cmd[0]) {			/* if a specific dpy was given */
	/* XXX - parse the rest of the command */
	specific_display = True;
    }

    for (l = head; l; l = l->next) {
	if (specific_display /* XXX - && !match(,l) */) continue;
	dump_entry (stdout, l->auth);
    }

    /* XXX */
    return 0;
}

/*
 * merge filename [filename ...]
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
 * extract filename displayname
 */
static int do_extract (inputfilename, lineno, cmd)
    char *inputfilename;
    int lineno;
    char *cmd;
{
    /* XXX */
    return 0;
}

/*
 * add displayname protocolname hexkey
 */
static int do_add (inputfilename, lineno, cmd)
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
 * get_hostname - Given an internet address, return a name (CHARON.MIT.EDU)
 * or a string representing the address (18.58.0.13) if the name cannot
 * be found.  Stolen from xhost.
 */

static jmp_buf env;
static nameserver_lost()
{
  nameserver_timedout = True;
  longjmp (env, -1);
}


static char *get_hostname (auth)
    Xauth *auth;
{
    struct hostent *hp = NULL;
    int nameserver_lost();
    char *inet_ntoa();
#ifdef DNETCONN
    struct nodeent *np;
    static char nodeaddr[16];
#endif /* DNETCONN */

    if (auth->family == FamilyInternet) {
	/* gethostbyaddr can take a LONG time if the host does not exist.
	   Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
	   that something is wrong and do not make the user wait.
	   gethostbyaddr will continue after a signal, so we have to
	   jump out of it. 
	   */
	nameserver_timedout = False;
	signal (SIGALRM, nameserver_lost);
	alarm (4);
	if (setjmp(env) == 0) {
	    hp = gethostbyaddr (auth->address, auth->address_length, AF_INET);
	}
	alarm (0);
	if (hp)
	  return (hp->h_name);
	else
	  return (inet_ntoa(*((struct in_addr *)(auth->address))));
    }
#ifdef DNETCONN
    if (auth->family == FamilyDECnet) {
	if (np = getnodebyaddr(auth->address, auth->address_length,
			       AF_DECnet)) {
	    sprintf(nodeaddr, "%s:", np->n_name);
	} else {
	    sprintf(nodeaddr, "%s:", dnet_htoa(auth->address));
	}
	return(nodeaddr);
    }
#endif

    return (NULL);
}
