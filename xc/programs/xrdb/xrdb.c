
#ifndef lint
static char rcs_id[] = "$Header: xrdb.c,v 11.5 87/09/11 19:57:36 jg Exp $";
#endif

/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
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
 * Author: Jim Gettys, August 28, 1987
 * Extensively Modified: Phil Karlton, January 5, 1987
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <ctype.h>
#include <strings.h>

extern FILE *popen();

typedef struct _Entry {
    char *tag, *value;
    int usable;
} Entry;
typedef struct _Buffer {
    char *buff;
    int  room, used;
} Buffer;
typedef struct _Entries {
    Entry *entry;
    int   room, used;
} Entries;

#define INIT_BUFFER_SIZE 10000
#define INIT_ENTRY_SIZE 500

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

void InitBuffer(b)
    Buffer *b;
{
    b->room = INIT_BUFFER_SIZE;
    b->used = 0;
    b->buff = (char *)malloc(INIT_BUFFER_SIZE*sizeof(char));
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

void AddEntry(e, entry)
    register Entries *e;
    Entry entry;
{
    if (e->used == e->room) {
	e->entry = (Entry *)realloc(e->entry, 2*e->room*(sizeof(Entry)));
	e->room *= 2;
    }
    entry.usable = True;
    e->entry[e->used++] = entry;
}


int CompareEntries(e1, e2)
    Entry *e1, *e2;
{
    return strcmp(e1->tag, e2->tag);
}

void AppendEntryToBuffer(buffer, entry)
    register Buffer *buffer;
    Entry entry;
{
    AppendToBuffer(buffer, entry.tag, strlen(entry.tag));
    AppendToBuffer(buffer, ":\t", 2);
    AppendToBuffer(buffer, entry.value, strlen(entry.value));
    AppendToBuffer(buffer, "\n", 1);
}

char *FindFirst(string, dest)
    register char *string;
    register char dest;
{
    for (;;) {
	if (*string == '\0')
	    return NULL;
	if (*string == '\\') {
	    if (*++string == '\0')
		return NULL;
	}
	else if (*string == dest)
	    return string;
	string++;
    }
}

void GetEntries(entries, buff)
    register Entries *entries;
    Buffer *buff;
{
    register char *line, *colon, *temp, *str;
    Entry entry;
    register int length;

    str = buff->buff;
    for (str = buff->buff; str < buff->buff + buff->used ;str = line + 1) {
	line = FindFirst(str, '\n');
	if (line == NULL)
	    break; 
	if (str[0] == '!')
	    continue;
	if (str[0] == '\n')
	    continue;
	if (str[0] == '#')
	    continue;
	colon = FindFirst(str, ':');
	if (colon == NULL)
	    break;
	if (colon > line) {
	    line[0] = '\0';
	    fprintf(stderr, "line missing colon ignored: %s\n", str);
	    continue;
	}

	temp = (char *)malloc((length = colon - str) + 1);
	strncpy(temp, str, length);
	temp[length] = '\0';
	while (temp[0] == ' ' || temp[0] == '\t') {
	    temp++; length--;
	}
	while (temp[length-1] == ' ' || temp[length-1] == '\t')
	    temp[--length] = '\0';
	entry.tag = temp;

	temp = (char *)malloc((length = line - colon - 1) + 1);
	strncpy(temp, colon + 1, length);
	temp[length] = '\0';
	while (temp[0] == ' ' || temp[0] == '\t') {
	    temp++; length--;
	}
	while (temp[length-1] == ' ' || temp[length-1] == '\t')
	    temp[--length] = '\0';
	entry.value = temp;

	AddEntry(entries, entry);
    }
    qsort(entries->entry, entries->used, sizeof(Entry), CompareEntries);
}

int MergeEntries(buffer, new, old)
    Entries new, old;
    Buffer *buffer;
{
    int n, o, cmp;

    buffer->used = 0;
    n = o = 0;
    while ((n < new.used) && (o < old.used)) {
	cmp = strcmp(new.entry[n].tag, old.entry[o].tag);
	if (cmp > 0)
	    AppendEntryToBuffer(buffer, old.entry[o++]);
	else {
	    AppendEntryToBuffer(buffer, new.entry[n++]);
	    if (cmp == 0)
		o++;
	}
    }
    while (n < new.used)
	AppendEntryToBuffer(buffer, new.entry[n++]);
    while (o < old.used)
	AppendEntryToBuffer(buffer, old.entry[o++]);
    AppendToBuffer(buffer, "\0", 1);
}

void ReadFile(buffer, input)
	register Buffer *buffer;
	FILE *input;
{
    register char	buf[BUFSIZ];
    register int	bytes;

    buffer->used = 0;
    while ((bytes = fread(buf, 1, BUFSIZ, input)) > 0) {
	AppendToBuffer(buffer, buf, bytes);
    }
    AppendToBuffer(buffer, "\0", 1);
}

AddDef(buff, title, value)
    char *buff, *title, *value;
{
    strcat(buff, " -D");
    strcat(buff, title);
    if (value && (value[0] != '\0')) {
	strcat(buff, "=");
	strcat(buff, value);
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
    strcat(buff, " -D");
    strcat(buff, title);
}

int Resolution(pixels, mm)
    int pixels, mm;
{
    return ((pixels * 100000 / mm) + 50) / 100;
}


/*
 * It would probably be best to enumerate all of the screens/visuals
 * rather than just using the defaults. However, most of the current
 * servers only have one screen/visual.
 */
void
DoDefines(display, defs, prog, host)
    Display *display;
    register char *defs;
    char *prog, *host;
{
#define MAXHOSTNAME 255
    Screen *screen;
    Visual *visual;
    char temp[MAXHOSTNAME], *colon;
    
    strcpy(temp, XDisplayName(host));
    colon = index(temp, ':');
    if (colon != NULL)
	*colon = '\0';
    if (temp[0] == '\0')	/* must be connected to :0 */
	gethostname(temp, MAXHOSTNAME);
    AddDef(defs, "HOST", temp);
    screen = DefaultScreenOfDisplay(display);
    visual = DefaultVisualOfScreen(screen);
    AddNum(defs, "WIDTH", screen->width);
    AddNum(defs, "HEIGHT", screen->height);
    AddNum(defs, "X_RESOLUTION", Resolution(screen->width,screen->mwidth));
    AddNum(defs, "Y_RESOLUTION", Resolution(screen->height,screen->mheight));
    AddNum(defs, "PLANES", visual->bits_per_rgb);
    switch(visual->class) {
	case StaticGray:
	    AddDef(defs, "CLASS", "StaticGray");
	    break;
	case GrayScale:
	    AddDef(defs, "CLASS", "GrayScale");
	    break;
	case StaticColor:
	    AddDef(defs, "CLASS", "StaticColor");
	    AddSimpleDef(defs, "COLOR");
	    break;
	case PseudoColor:
	    AddDef(defs, "CLASS", "PsuedoColor");
	    AddSimpleDef(defs, "COLOR");
	    break;
	case TrueColor:
	    AddDef(defs, "CLASS", "TrueColor");
	    AddSimpleDef(defs, "COLOR");
	    break;
	case DirectColor:
	    AddDef(defs, "CLASS", "DirectColor");
	    AddSimpleDef(defs, "COLOR");
	    break;
	default:
	    fatal("%s: unexpected visual class=%d\n", prog, visual->class);
    }
}

Entry *FindEntry(db, b)
    register Entries *db;
    Buffer *b;
{
    register char *colon;
    int i, length;
    register Entry *e;
    Entries phoney;
    Entry entry;

    phoney.used = 0;
    phoney.room = 1;
    phoney.entry = &entry;
    GetEntries(&phoney, b);
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
	    if (fgets(buff, BUFSIZ, in) == NULL)
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

main (argc, argv)
    int argc;
    char **argv;
{
    Display * dpy;
    int i;
    char *displayname = NULL;
    char *filename = NULL;
    FILE *fp, *input, *output;
    Buffer buffer;
    Entries newDB, oldDB;
    char cmd[BUFSIZ];
    char defines[BUFSIZ];
    int printit = 0;
    int showDefines = 0;
    int removeProp = 0;
    int merge = 0;
    int editFile = 0;

    InitBuffer(&buffer);
    InitEntries(&newDB);
    InitEntries(&oldDB);
    defines[0] = '\0';
    for (i = 1; i < argc; i++) {
	if (index (argv[i], ':') != NULL) displayname = argv[i];
	else if (strcmp ("-q", argv[i]) == 0) printit = 1;
	else if (strcmp ("-m", argv[i]) == 0) merge = 1;
	else if (strcmp ("-e", argv[i]) == 0) editFile = 1;
	else if (strcmp ("-d", argv[i]) == 0) showDefines = 1;
	else if (strcmp ("-r", argv[i]) == NULL) removeProp = 1;
	else if (strncmp ("-D", argv[i], 2) == 0) {
	    strcat(defines, " \"");
	    strcat(defines, argv[i]);
	    strcat(defines, "\"");
	}
	else filename = argv[i];
    }

    /* Open display  */
    if (!(dpy = XOpenDisplay (displayname)))
	fatal("%s: Can't open display '%s'\n", argv[0],
		 XDisplayName (displayname));

    DoDefines(dpy, defines, argv[0], displayname);
    if (showDefines)
	fprintf(stderr, "%s:%s\n", argv[0], defines);
    if (printit == 1) {
	/* user wants to print contents */
	if (dpy->xdefaults)
	    fputs(dpy->xdefaults, stdout);
	exit(0);
	}
    else if (removeProp) {
	if (dpy->xdefaults)
	    XDeleteProperty(dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER);
	}
    else if (editFile) {
	char template[100], old[100];

	if (filename == NULL)
	    fatal("%s: must specify file name to be edited.\n", argv[0]);
	input = fopen(filename, "r");
	if (input == NULL)
	    fatal("%s: can't open file '%s'\n", argv[0], filename);
	strcpy(template, filename);
	strcat(template, "XXXXXX");
	output = fopen(mktemp(template), "w");
	if (output == NULL)
	    fatal("%s: can't open temporary file '%s'\n", argv[0], template);
	buffer.used = strlen(dpy->xdefaults);
	buffer.buff = dpy->xdefaults;		/* drop it on the floor */
	buffer.room = buffer.used;
	GetEntries(&newDB, &buffer);
	EditFile(&newDB, input, output);
	fclose(input);
	fclose(output);
	strcpy(old, filename);
	strcat(old, ".BAK");
	rename(filename, old);
	rename(template, filename);
	}
    else {
	if (filename != NULL) {
		fp = freopen (filename, "r", stdin);
		if (fp == NULL)
		    fatal("%s: can't open file '%s'\n", argv[0], filename);
		}
	sprintf(cmd, "/usr/lib/cpp %s", defines);
	if ((input = popen(cmd, "r")) == NULL)
	    fatal("%s: cannot run '%s'\n", argv[0], cmd);
	ReadFile(&buffer, input);
	GetEntries(&newDB, &buffer);
	if (merge && dpy->xdefaults) {
	    char *saveBuff = buffer.buff;
	    buffer.used = strlen(dpy->xdefaults);
	    buffer.buff = dpy->xdefaults;
	    GetEntries(&oldDB, &buffer);
	    buffer.buff = saveBuff;
	}
	else
	    oldDB.used = 0;
	buffer.used = 0;
	MergeEntries(&buffer, newDB, oldDB);
	XChangeProperty (dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER,
		XA_STRING, 8, PropModeReplace, buffer.buff, buffer.used);
	}
	XCloseDisplay(dpy);

}


