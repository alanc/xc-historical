/*
 * xrdb - X resource manager database utility
 *
 * $XConsortium: xrdb.c,v 11.61 92/09/14 09:39:13 rws Exp $
 */

/*
 *			  COPYRIGHT 1987, 1991
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *		   MASSACHUSETTS INSTITUTE OF TECHNOLOGY
 *		       CAMBRIDGE, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/*
 * this program is used to load, or dump the resource manager database
 * in the server.
 *
 * Original Author: Jim Gettys, August 28, 1987
 * Extensively Modified: Phil Karlton, January 5, 1987
 * Modified a Bunch More: Bob Scheifler, February, 1991
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#define SCREEN_RESOURCES "SCREEN_RESOURCES"

#ifndef CPP
#define CPP "/usr/lib/cpp"
#endif /* CPP */

#define INIT_BUFFER_SIZE 10000
#define INIT_ENTRY_SIZE 500

#define RALL 0
#define RGLOBAL 1
#define RSCREEN 2
#define RSCREENS 3

#define OPSYMBOLS 0
#define OPQUERY 1
#define OPREMOVE 2
#define OPEDIT 3
#define OPLOAD 4
#define OPMERGE 5
#define OPOVERRIDE 6

#define RESOURCE_PROPERTY_NAME "RESOURCE_MANAGER"
#define BACKUP_SUFFIX ".bak"		/* for editting */

typedef struct _Entry {
    char *tag, *value;
    int lineno;
    Bool usable;
} Entry;
typedef struct _Buffer {
    char *buff;
    int  room, used;
} Buffer;
typedef struct _Entries {
    Entry *entry;
    int   room, used;
} Entries;

char *ProgramName;
Bool quiet = False;
char tmpname[32];
char *filename = NULL;
#ifdef PATHETICCPP
Bool need_real_defines = False;
char tmpname2[32];
#endif
int oper = OPLOAD;
char *editFile = NULL;
char *cpp_program = CPP;
char *backup_suffix = BACKUP_SUFFIX;
Bool dont_execute = False;
char defines[4096];
int defines_base;
Display *dpy;
Buffer buffer;
Entries newDB;

#ifndef sgi
extern FILE *popen();
#endif

#if defined(USG) && !defined(CRAY) && !defined(MOTOROLA)
int rename (from, to)
    char *from, *to;
{
    (void) unlink (to);
    if (link (from, to) == 0) {
        unlink (from);
        return 0;
    } else {
        return -1;
    }
}
#endif

void InitBuffer(b)
    Buffer *b;
{
    b->room = INIT_BUFFER_SIZE;
    b->used = 0;
    b->buff = (char *)malloc(INIT_BUFFER_SIZE*sizeof(char));
}

void FreeBuffer(b)
    Buffer *b;
{
    free(b->buff);
}

void AppendToBuffer(b, str, len)
    register Buffer *b;
    char *str;
    register int len;
{
    while (b->used + len > b->room) {
	b->buff = (char *)realloc(b->buff, 2*b->room*(sizeof(char)));
	b->room *= 2;
    }
    strncpy(b->buff + b->used, str, len);
    b->used += len;
}

void InitEntries(e)
    Entries *e;
{
    e->room = INIT_ENTRY_SIZE;
    e->used = 0;
    e->entry = (Entry *)malloc(INIT_ENTRY_SIZE*sizeof(Entry));
}

void FreeEntries(e)
    Entries *e;
{
    register int i;

    for (i = 0; i < e->used; i++) {
	if (e->entry[i].usable) {
	    free(e->entry[i].tag);
	    free(e->entry[i].value);
	}
    }
    free((char *)e->entry);
}

void AddEntry(e, entry)
    register Entries *e;
    Entry *entry;
{
    register int n;

    for (n = 0; n < e->used; n++) {
	if (!strcmp(e->entry[n].tag, entry->tag)) {
	    /* overwrite old entry */
	    if (e->entry[n].lineno && !quiet) {
		fprintf (stderr, 
			 "%s:  \"%s\" on line %d overrides entry on line %d\n",
			 ProgramName, entry->tag, entry->lineno, 
			 e->entry[n].lineno);
	    }
	    free(e->entry[n].tag);
	    free(e->entry[n].value);
	    entry->usable = True;
	    e->entry[n] = *entry;
	    return;  /* ok to leave, now there's only one of each tag in db */
	}
    }

    if (e->used == e->room) {
	e->entry = (Entry *)realloc((char *)e->entry,
				    2*e->room*(sizeof(Entry)));
	e->room *= 2;
    }
    entry->usable = True;
    e->entry[e->used++] = *entry;
}


int CompareEntries(e1, e2)
    Entry *e1, *e2;
{
    return strcmp(e1->tag, e2->tag);
}

void AppendEntryToBuffer(buffer, entry)
    register Buffer *buffer;
    Entry *entry;
{
    AppendToBuffer(buffer, entry->tag, strlen(entry->tag));
    AppendToBuffer(buffer, ":\t", 2);
    AppendToBuffer(buffer, entry->value, strlen(entry->value));
    AppendToBuffer(buffer, "\n", 1);
}

/*
 * Return the position of the first unescaped occurrence of dest in string.
 * If lines is non-null, return the number of newlines skipped over.
 */
char *FindFirst(string, dest, lines)
    register char *string;
    register char dest;
    register int *lines;	/* RETURN */
{
    if (lines)
	*lines = 0;
    for (;;) {
	if (*string == '\0')
	    return NULL;
	if (*string == '\\') {
	    if (*++string == '\0')
		return NULL;
	} else if (*string == dest)
	    return string;
	if (*string == '\n'  &&  lines)
	    (*lines)++;
	string++;
    }
}

void GetEntries(entries, buff, bequiet)
    register Entries *entries;
    Buffer *buff;
    int bequiet;
{
    register char *line, *colon, *temp, *str;
    Entry entry;
    register int length;
    int lineno = 0;
    int lines_skipped;

    str = buff->buff;
    if (!str) return;
    for ( ; str < buff->buff + buff->used;
	  str = line + 1, lineno += lines_skipped) {
	line = FindFirst(str, '\n', &lines_skipped);
	lineno++;
	if (!line)
	    break; 
	if (*str == '!')
	    continue;
	if (*str == '\n')
	    continue;
	if (!bequiet && *str == '#') {
	    int dummy;
	    if (sscanf (str, "# %d", &dummy) == 1 ||
		sscanf (str, "# line %d", &dummy) == 1)
		lineno = dummy - 1;
	    continue;
	}
	for (temp = str; 
	     *temp && *temp != '\n' && isascii(*temp) && isspace(*temp); 
	     temp++) ;
	if (!*temp || *temp == '\n') continue;

	colon = FindFirst(str, ':', NULL);
	if (!colon || colon > line) {
	    if (!bequiet && !quiet)
		fprintf (stderr, 
			 "%s: colon missing on line %d, ignoring line\n",
			 ProgramName, lineno);
	    continue;
	}

	/* strip leading and trailing blanks from name and store result */
	while (*str == ' ' || *str == '\t')
	    str++;
	length = colon - str;
	while (length && (str[length-1] == ' ' || str[length-1] == '\t'))
	    length--;
	temp = (char *)malloc(length + 1);
	strncpy(temp, str, length);
	temp[length] = '\0';
	entry.tag = temp;

	/* strip leading and trailing blanks from value and store result */
	colon++;
	while (*colon == ' ' || *colon == '\t')
	    colon++;
	length = line - colon;
	temp = (char *)malloc(length + 1);
	strncpy(temp, colon, length);
	temp[length] = '\0';
	entry.value = temp;
	entry.lineno = bequiet ? 0 : lineno;

	AddEntry(entries, &entry);
    }
}

GetEntriesString(entries, str)
    register Entries *entries;
    char *str;
{
    Buffer buff;

    if (str && *str) {
	buff.buff = str;
	buff.used = strlen(str);
	GetEntries(entries, &buff, 1);
    }
}

void ReadFile(buffer, input)
	register Buffer *buffer;
	FILE *input;
{
	     char	buf[BUFSIZ];
    register int	bytes;

    buffer->used = 0;
    while (!feof(input) && (bytes = fread(buf, 1, BUFSIZ, input)) > 0) {
	AppendToBuffer(buffer, buf, bytes);
    }
    AppendToBuffer(buffer, "", 1);
}

AddDef(buff, title, value)
    char *buff, *title, *value;
{
#ifdef PATHETICCPP
    if (need_real_defines) {
	strcat(buff, "\n#define ");
	strcat(buff, title);
	if (value && (value[0] != '\0')) {
	    strcat(buff, " ");
	    strcat(buff, value);
	}
	return;
    }
#endif
    strcat(buff, " -D");
    strcat(buff, title);
    if (value && (value[0] != '\0')) {
	strcat(buff, "=");
	strcat(buff, value);
    }
}

AddDefQ(buff, title, value)
    char *buff, *title, *value;
{
#ifdef PATHETICCPP
    if (need_real_defines) {
	strcat(buff, "\n#define ");
	strcat(buff, title);
	if (value && (value[0] != '\0')) {
	    strcat(buff, " \"");
	    strcat(buff, value);
	    strcat(buff, "\"");
	}
	return;
    }
#endif
    strcat(buff, " -D");
    strcat(buff, title);
    if (value && (value[0] != '\0')) {
	strcat(buff, "=\"");
	strcat(buff, value);
	strcat(buff, "\"");
    }
}

AddNum(buff, title, value)
    char *buff, *title;
    int value;
{
    char num[20];
    sprintf(num, "%d", value);
    AddDef(buff, title, num);
}

AddSimpleDef(buff, title)
    char *buff, *title;
{
    AddDef(buff, title, (char *)NULL);
}

AddDefTok(buff, prefix, title)
    char *buff, *prefix, *title;
{
    char *s;
    char name[512];
    char c;

    strcpy(name, prefix);
    strcat(name, title);
    for (s = name; c = *s; s++) {
	if (!isalpha(c) && !isdigit(c) && c != '_')
	    *s = '_';
    }
    AddSimpleDef(buff, name);
}

int Resolution(pixels, mm)
    int pixels, mm;
{
    return ((pixels * 100000 / mm) + 50) / 100;
}


void
DoDisplayDefines(display, defs, host)
    Display *display;
    register char *defs;
    char *host;
{
#define MAXHOSTNAME 255
    char client[MAXHOSTNAME], server[MAXHOSTNAME], *colon;
    char **extnames;
    int n;
    
    XmuGetHostname(client, MAXHOSTNAME);
    strcpy(server, XDisplayName(host));
    colon = index(server, ':');
    n = 0;
    if (colon) {
	*colon++ = '\0';
	if (*colon == ':')
	    colon++;
	sscanf(colon, "%d", &n);
    }
    if (!*server || !strcmp(server, "unix") || !strcmp(server, "localhost"))
	strcpy(server, client);
    AddDef(defs, "HOST", server); /* R3 compatibility */
    AddDef(defs, "SERVERHOST", server);
    AddDefTok(defs, "SRVR_", server);
    AddNum(defs, "DISPLAY_NUM", n);
    AddDef(defs, "CLIENTHOST", client);
    AddDefTok(defs, "CLNT_", client);
    AddNum(defs, "VERSION", ProtocolVersion(display));
    AddNum(defs, "REVISION", ProtocolRevision(display));
    AddDefQ(defs, "VENDOR", ServerVendor(display));
    AddDefTok(defs, "VNDR_", ServerVendor(display));
    AddNum(defs, "RELEASE", VendorRelease(display));
    AddNum(defs, "NUM_SCREENS", ScreenCount(display));
    extnames = XListExtensions(display, &n);
    while (--n >= 0)
	AddDefTok(defs, "EXT_", extnames[n]);
}

char *ClassNames[] = {
    "StaticGray",
    "GrayScale",
    "StaticColor",
    "PseudoColor",
    "TrueColor",
    "DirectColor"
};

void
DoScreenDefines(display, scrno, defs)
    Display *display;
    int scrno;
    register char *defs;
{
    Screen *screen;
    Visual *visual;
    XVisualInfo vinfo, *vinfos;
    int nv;
    char name[50];
    
    screen = ScreenOfDisplay(display, scrno);
    visual = DefaultVisualOfScreen(screen);
    vinfo.screen = scrno;
    vinfos = XGetVisualInfo(display, VisualScreenMask, &vinfo, &nv);
    AddNum(defs, "SCREEN_NUM", scrno);
    AddNum(defs, "WIDTH", screen->width);
    AddNum(defs, "HEIGHT", screen->height);
    AddNum(defs, "X_RESOLUTION", Resolution(screen->width,screen->mwidth));
    AddNum(defs, "Y_RESOLUTION", Resolution(screen->height,screen->mheight));
    AddNum(defs, "PLANES", DisplayPlanes(display, scrno));
    AddNum(defs, "BITS_PER_RGB", visual->bits_per_rgb);
    AddDef(defs, "CLASS", ClassNames[visual->class]);
    sprintf(name, "CLASS_%s", ClassNames[visual->class]);
    AddNum(defs, name, visual->visualid);
    switch(visual->class) {
	case StaticColor:
	case PseudoColor:
	case TrueColor:
	case DirectColor:
	    AddSimpleDef(defs, "COLOR");
	    break;
    }
    while (--nv >= 0) {
	sprintf(name, "CLASS_%s_%d",
		ClassNames[vinfos[nv].class], vinfos[nv].depth);
	AddNum(defs, name, vinfos[nv].visualid);
    }
    XFree((char *)vinfos);
}

Entry *FindEntry(db, b)
    register Entries *db;
    Buffer *b;
{
    int i;
    register Entry *e;
    Entries phoney;
    Entry entry;

    entry.usable = False;
    entry.tag = NULL;
    entry.value = NULL;
    phoney.used = 0;
    phoney.room = 1;
    phoney.entry = &entry;
    GetEntries(&phoney, b, 1);
    if (phoney.used < 1)
	return NULL;
    for (i = 0; i < db->used; i++) {
	e = &db->entry[i];
	if (!e->usable)
	    continue;
	if (strcmp(e->tag, entry.tag))
	    continue;
	e->usable = False;
	if (strcmp(e->value, entry.value))
	    return e;
	return NULL;
    }
    return NULL;
}

void EditFile(new, in, out)
    register Entries *new;
    FILE *in, *out;
{
    Buffer b;
    char buff[BUFSIZ];
    register Entry *e;
    register char *c;
    int i;

    InitBuffer(&b);
    for (;;) {
	b.used = 0;
	while (1) {
	    buff[0] ='\0';
	    if (!fgets(buff, BUFSIZ, in))
		goto cleanup;
	    AppendToBuffer(&b, buff, strlen(buff));
	    c = &b.buff[b.used - 1];
	    if ((*(c--) == '\n') && (b.used == 1 || *c != '\\'))
		break;
	}
	if (e = FindEntry(new, &b))
	    fprintf(out, "%s:\t%s\n", e->tag, e->value);
	else
	    fwrite(b.buff, 1, b.used, out);
    }
cleanup:
    for (i = 0; i < new->used; i++) {
	e = &new->entry[i];
	if (e->usable)
	    fprintf(out, "%s:\t%s\n", e->tag, e->value);
    }
}

void Syntax ()
{
    fprintf (stderr, 
	     "usage:  %s [-options ...] [filename]\n\n",
	     ProgramName);
    fprintf (stderr, 
	     "where options include:\n");
    fprintf (stderr, 
	     " -display host:dpy   display to use\n");
    fprintf (stderr, 
	     " -all                do all resources [default]\n");
    fprintf (stderr, 
	     " -global             do screen-independent resources\n");
    fprintf (stderr, 
	     " -screen             do screen-specific resources for one screen\n");
    fprintf (stderr, 
	     " -screens            do screen-specific resources for all screens\n");
    fprintf (stderr,
	     " -n                  show but don't do changes\n");
    fprintf (stderr, 
	     " -cpp filename       preprocessor to use [%s]\n",
	     CPP);
    fprintf (stderr, 
	     " -nocpp              do not use a preprocessor\n");
    fprintf (stderr, 
	     " -query              query resources\n");
    fprintf (stderr,
	     " -load               load resources from file [default]\n");
    fprintf (stderr,
	     " -override           add in resources from file\n");
    fprintf (stderr, 
	     " -merge              merge resources from file & sort\n");
    fprintf (stderr, 
	     " -edit filename      edit resources into file\n");
    fprintf (stderr, 
	     " -backup string      backup suffix for -edit [%s]\n",
	     BACKUP_SUFFIX);
    fprintf (stderr, 
	     " -symbols            show preprocessor symbols\n");
    fprintf (stderr, 
	     " -remove             remove resources\n");
    fprintf (stderr, 
	     " -retain             avoid server reset (avoid using this)\n");
    fprintf (stderr,
	     " -quiet              don't warn about duplicates\n");
    fprintf (stderr, 
	     " -Dname[=value], -Uname, -Idirectory    %s\n",
	     "passed to preprocessor");
    fprintf (stderr, 
	     "\n");
    fprintf (stderr,
	     "A - or no input filename represents stdin.\n");  
    exit (1);
}

/*
 * The following is a hack until XrmParseCommand is ready.  It determines
 * whether or not the given string is an abbreviation of the arg.
 */

Bool isabbreviation (arg, s, minslen)
    char *arg;
    char *s;
    int minslen;
{
    int arglen;
    int slen;

    /* exact match */
    if (!strcmp (arg, s)) return (True);

    arglen = strlen (arg);
    slen = strlen (s);

    /* too long or too short */
    if (slen >= arglen || slen < minslen) return (False);

    /* abbreviation */
    if (strncmp (arg, s, slen) == 0) return (True);

    /* bad */
    return (False);
}

main (argc, argv)
    int argc;
    char **argv;
{
    int i;
    char *displayname = NULL;
    int whichResources = RALL;
    int retainProp = 0;
    FILE *fp = NULL;
    Bool need_newline;

    ProgramName = argv[0];

    defines[0] = '\0';

    /* needs to be replaced with XrmParseCommand */

    for (i = 1; i < argc; i++) {
	char *arg = argv[i];

	if (arg[0] == '-') {
	    if (arg[1] == '\0') {
		filename = NULL;
		continue;
	    } else if (isabbreviation ("-help", arg, 2)) {
		Syntax ();
		/* doesn't return */
	    } else if (isabbreviation ("-display", arg, 2)) {
		if (++i >= argc) Syntax ();
		displayname = argv[i];
		continue;
	    } else if (isabbreviation ("-geometry", arg, 3)) {
		if (++i >= argc) Syntax ();
		/* ignore geometry */
		continue;
	    } else if (isabbreviation ("-cpp", arg, 2)) {
		if (++i >= argc) Syntax ();
		cpp_program = argv[i];
		continue;
	    } else if (!strcmp ("-n", arg)) {
		dont_execute = True;
		continue;
	    } else if (isabbreviation ("-nocpp", arg, 3)) {
		cpp_program = NULL;
		continue;
	    } else if (isabbreviation ("-query", arg, 2)) {
		oper = OPQUERY;
		continue;
	    } else if (isabbreviation ("-load", arg, 2)) {
		oper = OPLOAD;
		continue;
	    } else if (isabbreviation ("-merge", arg, 2)) {
		oper = OPMERGE;
		continue;
	    } else if (isabbreviation ("-override", arg, 2)) {
		oper = OPOVERRIDE;
		continue;
	    } else if (isabbreviation ("-symbols", arg, 3)) {
		oper = OPSYMBOLS;
		continue;
	    } else if (isabbreviation ("-remove", arg, 4)) {
		oper = OPREMOVE;
		continue;
	    } else if (isabbreviation ("-edit", arg, 2)) {
		if (++i >= argc) Syntax ();
		oper = OPEDIT;
		editFile = argv[i];
		continue;
	    } else if (isabbreviation ("-backup", arg, 2)) {
		if (++i >= argc) Syntax ();
		backup_suffix = argv[i];
		continue;
	    } else if (isabbreviation ("-all", arg, 2)) {
		whichResources = RALL;
		continue;
	    } else if (isabbreviation ("-global", arg, 3)) {
		whichResources = RGLOBAL;
		continue;
	    } else if (isabbreviation ("-screen", arg, 3)) {
		whichResources = RSCREEN;
		continue;
	    } else if (!strcmp ("-screens", arg)) {
		whichResources = RSCREENS;
		continue;
	    } else if (isabbreviation ("-retain", arg, 4)) {
		retainProp = 1;
		continue;
	    } else if (isabbreviation ("-quiet", arg, 2)) {
		quiet = True;
		continue;
	    } else if (arg[1] == 'I' || arg[1] == 'U' || arg[1] == 'D') {
		strcat(defines, " \"");
		strcat(defines, arg);
		strcat(defines, "\"");
		continue;
	    }
	    Syntax ();
	} else if (arg[0] == '=') 
	    continue;
	else
	    filename = arg;
    }							/* end for */

    while ((i = open("/dev/null", 0)) < 3)
	; /* make sure later freopen won't clobber things */
    (void) close(i);
    /* Open display  */
    if (!(dpy = XOpenDisplay (displayname)))
	fatal("%s: Can't open display '%s'\n", ProgramName,
		 XDisplayName (displayname));

    if (whichResources == RALL && ScreenCount(dpy) == 1)
	whichResources = RGLOBAL;

#ifdef PATHETICCPP
    if (cpp_program &&
	(oper == OPLOAD || oper == OPMERGE || oper == OPOVERRIDE)) {
	need_real_defines = True;
	strcpy(tmpname2, "/tmp/xrdbD_XXXXXX");
	(void) mktemp(tmpname);
    }
#endif

    if (!filename &&
#ifdef PATHETICCPP
	need_real_defines
#else
	(oper == OPLOAD || oper == OPMERGE || oper == OPOVERRIDE) &&
	(whichResources == RALL || whichResources == RSCREENS)
#endif
	) {
	strcpy(tmpname, "/tmp/xrdb_XXXXXX");
	(void) mktemp(tmpname);
	filename = tmpname;
	fp = fopen(filename, "w");
	if (!fp)
	    fatal("%s: Failed to open temp file: %s\n", ProgramName,
		  filename);
	while ((i = getc(stdin)) != EOF)
	    putc(i, fp);
	fclose(fp);
    }
	
    DoDisplayDefines(dpy, defines, displayname);
    defines_base = strlen(defines);
    need_newline = (oper == OPQUERY || oper == OPSYMBOLS ||
		    (dont_execute && oper != OPREMOVE));
    InitBuffer(&buffer);
    if (whichResources == RGLOBAL)
	Process(DefaultScreen(dpy), False, True);
    else if (whichResources == RSCREEN)
	Process(DefaultScreen(dpy), True, True);
    else if (whichResources == RSCREENS ||
	     (oper != OPLOAD && oper != OPMERGE && oper != OPOVERRIDE)) {
	if (whichResources == RALL && oper != OPSYMBOLS) {
	    if (need_newline)
		printf("! screen-independent resources\n");
	    Process(0, False, True);
	    if (need_newline)
		printf("\n");
	}
	for (i = 0; i < ScreenCount(dpy); i++) {
	    if (need_newline) {
		if (oper == OPSYMBOLS)
		    printf("# screen %d symbols\n", i);
		else {
		    printf("! screen %d resources\n", i);
		    printf("#if SCREEN_NUM == %d\n", i);
		}
	    }
	    Process(i, True, True);
	    if (need_newline) {
		if (oper != OPSYMBOLS)
		    printf("#endif\n");
		if (i+1 != ScreenCount(dpy))
		    printf("\n");
	    }
	}
    }
    else {
	Entries *dbs;

	dbs = (Entries *)malloc(ScreenCount(dpy) * sizeof(Entries));
	for (i = 0; i < ScreenCount(dpy); i++) {
	    Process(i, True, False);
	    dbs[i] = newDB;
	}
	InitEntries(&newDB);
	if (oper == OPMERGE || oper == OPOVERRIDE)
	    GetEntriesString(&newDB, XResourceManagerString(dpy));
	ShuffleEntries(&newDB, dbs, ScreenCount(dpy));
	if (need_newline)
	    printf("! screen-independent resources\n");
	ReProcess(0, False);
	if (need_newline)
	    printf("\n");
	for (i = 0; i < ScreenCount(dpy); i++) {
	    newDB = dbs[i];
	    if (need_newline) {
		printf("! screen %d resources\n", i);
		printf("#if SCREEN_NUM == %d\n", i);
	    }
	    ReProcess(i, True);
	    if (need_newline) {
		printf("#endif\n");
		if (i+1 != ScreenCount(dpy))
		    printf("\n");
	    }
	}
    }

    if (fp)
	unlink(filename);
    if (retainProp)
	XSetCloseDownMode(dpy, RetainPermanent);
    XCloseDisplay(dpy);
    exit (0);
}

void FormatEntries(buffer, entries)
    register Buffer *buffer;
    register Entries *entries;
{
    register int i;

    buffer->used = 0;
    if (!entries->used)
	return;
    if (oper == OPMERGE)
	qsort(entries->entry, entries->used, sizeof(Entry), CompareEntries);
    for (i = 0; i < entries->used; i++) {
	if (entries->entry[i].usable)
	    AppendEntryToBuffer(buffer, &entries->entry[i]);
    }
}

StoreProperty(dpy, root, res_prop)
    Display *dpy;
    Window root;
    Atom res_prop;
{
    int len = buffer.used;
    int mode = PropModeReplace;
    unsigned char *buf = (unsigned char *)buffer.buff;
    int max = (XMaxRequestSize(dpy) << 2) - 28;

    if (len > max) {
	XGrabServer(dpy);
	do {
	    XChangeProperty(dpy, root, res_prop, XA_STRING, 8, mode, buf, max);
	    buf += max;
	    len -= max;
	    mode = PropModeAppend;
	} while (len > max);
    }
    XChangeProperty(dpy, root, res_prop, XA_STRING, 8, mode, buf, len);
    if (mode != PropModeReplace)
	XUngrabServer(dpy);
}

Process(scrno, doScreen, execute)
    int scrno;
    Bool doScreen;
    Bool execute;
{
    char *xdefs;
    Window root;
    Atom res_prop;
    FILE *input, *output;
    char cmd[BUFSIZ];

    defines[defines_base] = '\0';
    buffer.used = 0;
    InitEntries(&newDB);
    DoScreenDefines(dpy, scrno, defines);
    if (doScreen) {
	xdefs = XScreenResourceString (ScreenOfDisplay(dpy, scrno));
	root = RootWindow(dpy, scrno);
	res_prop = XInternAtom(dpy, SCREEN_RESOURCES, False);
    } else {
	xdefs = XResourceManagerString (dpy);
	root = RootWindow(dpy, 0);
	res_prop = XA_RESOURCE_MANAGER;
    }
    if (oper == OPSYMBOLS) {
	printf ("%s\n", defines);
    } else if (oper == OPQUERY) {
	if (xdefs)
	    printf ("%s", xdefs);	/* fputs broken in SunOS 4.0 */
    } else if (oper == OPREMOVE) {
	if (xdefs)
	    XDeleteProperty(dpy, root, res_prop);
    } else if (oper == OPEDIT) {
	char template[100], old[100];

	input = fopen(editFile, "r");
	if (!input) {
	    input = fopen (editFile, "w");
	    if (!input) {
		fatal ("%s:  unable to create file '%s' for editing\n",
		       ProgramName, editFile);
		/* doesn't return */
	    }
	    (void) fclose (input);
	    input = fopen (editFile, "r");
	    if (!input) {
		fatal ("%s:  unable to open file '%s' for editing\n",
		       ProgramName, editFile);
		/* doesn't return */
	    }
	}
	strcpy(template, editFile);
	strcat(template, "XXXXXX");
	(void) mktemp(template);
	output = fopen(template, "w");
	if (!output)
	    fatal("%s: can't open temporary file '%s'\n", ProgramName, template);
	GetEntriesString(&newDB, xdefs);
	EditFile(&newDB, input, output);
	fclose(input);
	fclose(output);
	strcpy(old, editFile);
	strcat(old, backup_suffix);
	if (dont_execute) {		/* then write to standard out */
	    char buf[BUFSIZ];
	    int n;

	    output = fopen (template, "r");
	    if (output) {
		while ((n = fread (buf, 1, sizeof buf, output)) > 0) {
		    fwrite (buf, 1, n, stdout);
		}
		fclose (output);
	    }
	    unlink (template);
	} else {
	    rename (editFile, old);
	    rename (template, editFile);
	}
    } else {
	if (oper == OPMERGE || oper == OPOVERRIDE)
	    GetEntriesString(&newDB, xdefs);
#ifdef PATHETICCPP
	if (need_real_defines) {
	    if (!freopen(tmpname2, "w+", stdin))
		fatal("%s: can't open file '%s'\n", ProgramName, tmpname2);
	    fputs(defines, stdin);
	    fprintf(stdin, "\n#include \"%s\"\n", filename);
	    fflush(stdin);
	    fseek(stdin, 0, 0);
	    sprintf(cmd, "%s", cpp_program);
	    if (!(input = popen(cmd, "r")))
		fatal("%s: cannot run '%s'\n", ProgramName, cmd);
	} else {
#endif
	if (filename) {
	    if (!freopen (filename, "r", stdin))
		fatal("%s: can't open file '%s'\n", ProgramName, filename);
	}
	if (cpp_program) {
	    sprintf(cmd, "%s %s", cpp_program, defines);
	    if (!(input = popen(cmd, "r")))
		fatal("%s: cannot run '%s'\n", ProgramName, cmd);
	} else {
	    input = stdin;
	}
#ifdef PATHETICCPP
	}
#endif
	ReadFile(&buffer, input);
	if (cpp_program)
	    pclose(input);
#ifdef PATHETICCPP
	if (need_real_defines)
	    unlink(tmpname2);
#endif
	GetEntries(&newDB, &buffer, 0);
	if (execute) {
	    FormatEntries(&buffer, &newDB);
	    if (dont_execute) {
		if (buffer.used > 0) {
		    fwrite (buffer.buff, 1, buffer.used, stdout);
		    if (buffer.buff[buffer.used - 1] != '\n') putchar ('\n');
		}
	    } else if (buffer.used > 1 || !doScreen)
		StoreProperty (dpy, root, res_prop);
	    else
		XDeleteProperty (dpy, root, res_prop);
	}
    }
    if (execute)
	FreeEntries(&newDB);
    if (doScreen && xdefs)
	XFree(xdefs);
}

ShuffleEntries(db, dbs, num)
    Entries *db;
    Entries *dbs;
    int num;
{
    int *hits;
    register int i, j, k;
    Entries cur, cmp;
    char *curtag, *curvalue;

    hits = (int *)malloc(num * sizeof(int));
    cur = dbs[0];
    for (i = 0; i < cur.used; i++) {
	curtag = cur.entry[i].tag;
	curvalue = cur.entry[i].value;
	for (j = 1; j < num; j++) {
	    cmp = dbs[j];
	    for (k = 0; k < cmp.used; k++) {
		if (cmp.entry[k].usable &&
		    !strcmp(curtag, cmp.entry[k].tag) &&
		    !strcmp(curvalue, cmp.entry[k].value))
		{
		    hits[j] = k;
		    break;
		}
	    }
	    if (k == cmp.used)
		break;
	}
	if (j == num) {
	    AddEntry(db, &cur.entry[i]);
	    hits[0] = i;
	    for (j = 0; j < num; j++)
		dbs[j].entry[hits[j]].usable = False;
	}
    }
    free((char *)hits);
}

ReProcess(scrno, doScreen)
    int scrno;
    Bool doScreen;
{
    Window root;
    Atom res_prop;
    register int i;

    FormatEntries(&buffer, &newDB);
    if (doScreen) {
	root = RootWindow(dpy, scrno);
	res_prop = XInternAtom(dpy, SCREEN_RESOURCES, False);
    } else {
	root = RootWindow(dpy, 0);
	res_prop = XA_RESOURCE_MANAGER;
    }
    if (dont_execute) {
	if (buffer.used > 0) {
	    fwrite (buffer.buff, 1, buffer.used, stdout);
	    if (buffer.buff[buffer.used - 1] != '\n') putchar ('\n');
	}
    } else {
	if (buffer.used > 1 || !doScreen)
	    StoreProperty (dpy, root, res_prop);
	else
	    XDeleteProperty (dpy, root, res_prop);
    }
    FreeEntries(&newDB);
}

fatal(msg, prog, x1, x2, x3, x4, x5)
    char *msg, *prog;
    int x1, x2, x3, x4, x5;
{
    extern int errno;

    if (errno)
	perror(prog);
    (void) fprintf(stderr, msg, prog, x1, x2, x3, x4, x5);
    exit(1);
}
