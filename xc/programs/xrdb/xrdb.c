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

#define MAXRDB	100000

char buffer[MAXRDB];

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

    for (i = 0; i < argc; i++) {
	if (index (argv[i], ':') != NULL) displayname = argv[i];
	if (strcmp ("-l", argv[i]) == NULL) filename = argv[i+1];
    }

    /* Open display  */
    if (!(dpy = XOpenDisplay (displayname))) {
	(void) fprintf (stderr, "%s: Can't open display '%s'\n",
		argv[0], XDisplayName (displayname));
	exit (1);
    }

    if (filename == NULL) {
	/* user wants to print contents */
	fputs(dpy->xdefaults, stdout);
	exit(0);
	}
    else {
	fp = fopen (filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "%s: can't open file '%s'\n", 
			argv[0], filename);
		exit(1);
		}
	nbytes = fread(buffer, sizeof(char), MAXRDB, fp);
	XChangeProperty (dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER,
		XA_STRING, 8, PropModeReplace, buffer, nbytes);
	}
	XCloseDisplay(dpy);

}
