#include <X11/Xlib.h>
#include <stdio.h>

#define NULL 0

#define N_START 1000  /* Maximum # of fonts to start with */

#include "jdisplay.h"

usage()
{
	fprintf(stderr,"%s: usage: %s [host:display] [pattern]\n", program_name,
		program_name);
	exit(1);
}

main(argc, argv)
int argc;
char **argv;    
{
	char **fonts;
	char *pattern = "*";
	int n=N_START;
	int i;

	INIT_NAME;

	/* Handle command line arguments, open display */
	Setup_Display_And_Screen(&argc, argv);
 	for (i=1; i<argc; i++) {
	  if (!strcmp("-", argv[i]))
	    continue;
	  if (argv[i][0] == '-')
	    usage();
	  pattern = argv[i];
 	}

	/* Get list of fonts matching pattern */
 	for (;;) {
	  if (n!=N_START) {
	    if (i != n-1)
	      break;
	    XFreeFontNames(fonts);
	  }
	  n *= 2;
	  fonts = XListFonts(dpy, pattern, n, &i);
 	}

	/* Display list of fonts */
	for (n=0; n<i; n++)
	  printf("%s\n", fonts[n]);

	XFreeFontNames(fonts);
}


/*
 * Tempary replacement for not implemented Xlib routine.
 */
char *XGetDefault(dpy, name, option_name)
Display *dpy;
char *name;
char *option_name;
{
	return(NULL);
}
