/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: main.c,v 1.2 88/09/06 17:48:09 jim Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   October 22, 1987
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: main.c,v 4.0 88/08/31 22:12:26 kit Exp $";
#endif

#include "globals.h"

#if ( !defined(lint) && !defined(SABER)) /* Version can be retrieve */
  static char version[] = XMAN_VERSION;  /* via strings. */
#endif

static void ArgError();

/* XtOffset() hack for ibmrt BandAidCompiler */

static XtResource my_resources[] = {
  {"manualFontNormal", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     (Cardinal) &(fonts.normal), XtRString, MANPAGE_NORMAL},
  {"manualFontBold", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     (Cardinal) &(fonts.bold), XtRString, MANPAGE_BOLD},
  {"manualFontItalic", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     (Cardinal) &(fonts.italic), XtRString, MANPAGE_ITALIC},
  {"directoryFontNormal", XtCFont, XtRFontStruct, sizeof(XFontStruct *),
     (Cardinal) &(fonts.directory), XtRString, DIRECTORY_NORMAL},
  {"bothShown", XtCBoolean, XtRBoolean, sizeof(Boolean),
     (Cardinal) &both_shown_initial, XtRString, "False"},
  {"directoryHeight", "DirectoryHeight", XtRInt, sizeof(int),
     (Cardinal) &directory_height, XtRString, "150"},  
  {"topCursor", XtCCursor, XtRCursor, sizeof(Cursor), 
     (Cardinal) &(cursors.top), XtRString, XMAN_CURSOR},
  {"helpCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     (Cardinal) &(cursors.help), XtRString, HELP_CURSOR},
  {"manpageCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     (Cardinal) &(cursors.manpage), XtRString, MANPAGE_CURSOR},
  {"searchEntryCursor", XtCCursor, XtRCursor, sizeof(Cursor),
     (Cardinal) &(cursors.search_entry), XtRString, SEARCH_ENTRY_CURSOR},
  {"helpFile", XtCFile, XtRString, sizeof(char *),
     (Cardinal) &(help_file), XtRString, HELPFILE},
  {"topBox", XtCBoolean, XtRBoolean, sizeof(Boolean),
     (Cardinal) &top_box_active, XtRString, "True"},
};

/*
 * This is necessary to keep all TopLevel shells from becoming
 * the size that is specified on the command line.
 */

static XrmOptionDescRec xman_options[] = {
{"-geometry", "*topBox.geometry", XrmoptionSepArg, (caddr_t) NULL},
{"=",         "*topBox.geometry", XrmoptionIsArg, (caddr_t) NULL},
{"-pagesize", "*manualBrowser.geometry", XrmoptionSepArg, (caddr_t) NULL},
{"-notopbox", "topBox", XrmoptionNoArg, (caddr_t) "False"},
{"-helpfile", "helpFile", XrmoptionSepArg, (caddr_t) NULL},
{"-bothshown", "bothShown", XrmoptionNoArg, (caddr_t) "True"},
};

/*	Function Name: main
 *	Description: This is the main driver for Xman.
 *	Arguments: argc, argv - the command line arguments.
 *	Returns: return, what return.
 */

void 
main(argc,argv)
char ** argv;
int argc;
{
  Widget top_menu;

  top_menu = XtInitialize(TOPBOXNAME,"XMan", 
			  xman_options, XtNumber(xman_options),
			  (unsigned int*) &argc,argv);
  if (argc != 1) {
    ArgError(argc, argv);
    exit(42);
  }

  XtGetApplicationResources( (Widget) top_menu, (caddr_t) NULL, 
			    my_resources, XtNumber(my_resources),
			    NULL, (Cardinal) 0);
  if (!fonts.normal)
	XtError("failed to get the manualFontNormal font");
  if (!fonts.bold)
	fonts.bold = fonts.normal;
  if (!fonts.italic)
	fonts.italic = fonts.bold;
  if (!fonts.directory)
	fonts.directory = fonts.normal;

#ifdef DEBUG
  printf("debugging mode\n");
  XSynchronize( XtDisplay(top_menu), TRUE);
#endif

  /* set default width and height. */

  default_width = DisplayWidth(XtDisplay(top_menu), 
			       XtDisplay(top_menu)->default_screen);
  default_width /= 2; 
  default_height=DisplayHeight(XtDisplay(top_menu), 
			       XtDisplay(top_menu)->default_screen);
  default_height *= 3;
  default_height /= 4;

  sections = Man();

  MakeTopMenuWidget(top_menu);	

  if (top_box_active) {
    XtRealizeWidget(top_menu);
    AddCursor(top_menu,cursors.top);
  }
  else
    CreateManpage();

/*
 * We need to keep track of the number of manual pages that are shown on
 * the screen so that if this user does not have a top box then when he
 * removes all his manual pages we can kill off the xman process.
 * To make things easier we will consider the top box a shown manual page
 * here, but since you cannot remove it, man_page_show only goes to zero when
 * no top box is present.
 */

  man_pages_shown = 1;		

  XtMainLoop();
}

/*	Function Name: Quit
 *	Description: closes the display and quits.
 *	Arguments: widget - and widget.
 *	Returns: none.
 */

void
Quit(w)
Widget w;
{
  XCloseDisplay(XtDisplay(w));
  exit(0);
}


/*	Function Name: ArgError
 *	Description:  Prints error message about unknow arguments.
 *	Arguments: argc, argv - args not understood.
 *	Returns: none.
 */

static void 
ArgError(argc, argv)
char ** argv;
int argc;
{
  int i;
  char * prog;

  static char **syntax, *syntax_def[] = {
  "-helpfile <filename>",    "Specifies the helpfile to use.",
  "-bothshown",              "Show both the directory and manpage at once.",
  "-notopbox",               "Starts with manpage rather than topbox.",
  "-geometery <geom>",       "Specifies the geometry of the top box.",
  "=<geom>",                 "Specifies the geometry of the top box.",
  "-pagesize <geom>",        "Specifies the geometry of the manual page.",
  "-bw <pixels>",            "Width of all window borders.",
  "-borderwidth <pixels>",   "Width of all window borders.",
  "-bd <color>",             "Color of all window borders.",
  "-bordercolor <color>",    "Color of all window borders.",
  "-fg <color>",             "Foreground color for the application.",
  "-foreground <color>",     "Foreground color for the application.",
  "-bg <color>",             "Background color for the application.",
  "-background <color>",     "Background color for the application.",
  "-display <display name>", "Specify a display that is not the default",
  "-fn <font>",              "Font to be used for button and label text.",
  "-font <font>",            "Font to be used for button and label text.",
  "-name <name>",            "Change the name used for retrieving resources.",
  "-title <name>",           "Change the name without affecting resources.",
  "-xrm <resource>",         "Specifies a resource on the command line.",
  NULL, NULL,
  };
  
  syntax = syntax_def;

  for (i = 1; i < argc ; i++) 
    (void) printf("This argument is unknown to Xman: %s\n", argv[i]);
  
  if ( (prog = rindex(argv[0], '/')) == NULL)
    prog = argv[0];

  (void) printf("\nKnown arguments are:\n");

  while ( *syntax != NULL ) {
    printf("%-20s - %s\n", syntax[0], syntax[1]);
    syntax += 2;
  }
}
