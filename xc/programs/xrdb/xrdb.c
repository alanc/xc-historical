/*
 * this program is used to load, or dump the resource manager database
 * in the server.
 *
 * Author: Jim Gettys, August 25, 1987
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <strings.h>
#include <X11/Xatom.h>
#include <ctype.h>

#define MAXRDB 100000
#define DBOK 0

char buffer[MAXRDB];
char *SanityCheck();

main (argc, argv)
    int argc;
    char **argv;
{
    Display * dpy;
    int i;
    char *displayname = NULL;
    char *filename = NULL;
    FILE *fp;
    int nbytes;
    char *bptr, *endptr;
    int printit = 0;

    for (i = 1; i < argc; i++) {
	if (index (argv[i], ':') != NULL) displayname = argv[i];
	else filename = argv[i];
	if (strcmp ("-q", argv[i]) == NULL) printit = 1;
    }

    /* Open display  */
    if (!(dpy = XOpenDisplay (displayname))) {
	(void) fprintf (stderr, "%s: Can't open display '%s'\n",
		argv[0], XDisplayName (displayname));
	exit (1);
    }

    if (printit == 1) {
	/* user wants to print contents */
	fputs(dpy->xdefaults, stdout);
	exit(0);
	}
    else {
	if (filename != NULL) {
		fp = freopen (filename, "r", stdin);
		if (fp == NULL) {
			fprintf(stderr, "%s: can't open file '%s'\n", 
				argv[0], filename);
			exit(1);
			}
		}
	nbytes = fread(buffer, sizeof(char), MAXRDB, stdin);
	if ((bptr = SanityCheck (buffer)) != DBOK) {
		fprintf(stderr, "%s: database fails sanity check \n'%s'\n", 
			argv[0], bptr);
		exit(1);
		}
	XChangeProperty (dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER,
		XA_STRING, 8, PropModeReplace, buffer, nbytes);
	}
	XCloseDisplay(dpy);

}

char *getline(buffer, buf)
	register char *buffer;
	register char *buf;
{
	register char c;
	while (*buffer != '\0') {
		c = *buffer++;
		if (c == '\n') {
			*buf = '\0';
			return (buffer);
			}
		if ( ! isspace(c)) *buf++ = c;
	}
	return (NULL);
}

/*
 * does simple sanity check on data base.  Lines can either be
 * commented, be all white space, or must contain ':'.
 */
char *SanityCheck (buffer)
	char *buffer;
{
    static char buf[BUFSIZ];
    register char *s;
    char *b = buffer;
    register char *i;
    while (1) {
	if ((b = getline(b, buf)) == NULL) return (DBOK);
	if (buf[0] == '#' || buf[0] == '\0') continue;
	if ((i = index (buf, ':')) == NULL) return buf;
	}
}
