/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: defs.h,v 1.12 89/03/11 09:49:27 rws Exp $
 * $Athena: defs.h,v 4.8 89/01/06 15:56:19 kit Exp $
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

#ifndef HELPFILE
#define HELPFILE "/usr/lib/X11/xman.help" /* name of the default helpfile. */ 
#endif

/* The default cursors */

#define XMAN_CURSOR "left_ptr"		/* Top level cursor. */
#define HELP_CURSOR "left_ptr"	        /* The help cursor. */
#define MANPAGE_CURSOR "left_ptr"	/* The manpage cursor. */
#define SEARCH_ENTRY_CURSOR "question_arrow"	/* The search text widget
						   cursor. */
/* The default fonts */

#ifdef ATHENA
#define MANPAGE_NORMAL   "fixed"
#define MANPAGE_BOLD     "helvetica-bold12"
#define MANPAGE_ITALIC   "helvetica-boldoblique12"
#define DIRECTORY_NORMAL "fixed"
#else
#define MANPAGE_NORMAL   "*-new century schoolbook-medium-r-normal--*-120-*"
#define MANPAGE_BOLD     "*-new century schoolbook-bold-r-normal--*-120-*"
#define MANPAGE_ITALIC   "*-new century schoolbook-bold-i-normal--*-120-*"
#define DIRECTORY_NORMAL "fixed"
#endif ATHENA

#define OPTION_MENU "optionMenu" /* Name of the Option Menu. */
#define SECTION_MENU "sectionMenu" /* Name of the Section Menu. */

#define HELP_BUTTON "helpButton" /* Name of top help button */
#define QUIT_BUTTON "quitButton" /* Name of top quit button */
#define MANPAGE_BUTTON "manpageButton" /* Name of top manpage button */

#define TOPBOXNAME  "topBox"	/* Name of the Top Box. */
#define MANNAME "manualBrowser"	/* name for each manual page widget. */
#define SEARCHNAME "search" /* The name for the search widget. */
#define HELPNAME  "help"	/* The name of the help widget. */
#define DIRECTORY_NAME "directory" /* name of the directory widget. */
#define MANUALPAGE "manualPage"	/* name of the Scrollbyline widget that
				 contains the man page. */
#define DIALOG         "dialog"

/* Names of the menu buttons */

#define NUM_OPTIONS 9		/* Number of menu options. */

#define DIRECTORY      "displayDirectory"
#define MANPAGE        "displayManualPage"
#define HELP           "help"
#define SEARCH         "search"
#define BOTH_SCREENS   "showBothScreens"
#define REMOVE_MANPAGE "removeThisManpage"
#define OPEN_MANPAGE   "openNewManpage"
#define SHOW_VERSION   "showVersion"
#define QUIT           "quit"

/* definitions of string to use for show both and show one. */

#define SHOW_BOTH "Show Both Screens"
#define SHOW_ONE "Show One Screen"

/* 
 * Things will not look right if you change these names to make 
 * MANUALSEARCH longer APROPOSSEARCH, see search.c for details.
 */

#define MANUALSEARCH "manualPage"
#define APROPOSSEARCH "apropos"
#define CANCEL "cancel"

#define MANUAL 0
#define APROPOS 1

#define NO_SECTION_DEFAULTS ("no default sections")

/*
 * The command filters for the manual and apropos searches.
 */

#define APROPOSFILTER ("man -M %s -k %s | pr -h Apropos > %s")
#define MANUALCOMMAND "man -M"

#if defined( macII )
#  define FORMAT "pcat"		              /* The format command. */
#else
#  if defined( ultrix )
#    define FORMAT "| nroff -man"             /* The format command. */
#  else
#    define FORMAT "| neqn | nroff -man"      /* The format command. */
#  endif
#endif

#define TBL "tbl"

#define DEFAULT_WIDTH 500	/* The default width of xman. */
#define SECTALLOC  8		/* The number of entries allocated
				   at a time for the manual structures. */
#define ENTRYALLOC 100		/* The number of entries allocated
				   at a time for a section. */

#define NLINES  66		/* This is the number of lines to wait until
				   we boldify the line again, this allows 
				   me to bold the first line of each page.*/

#define INITIAL_DIR 0		/* The Initial Directory displayed. */

#define LMAN 3			/* Name and length of the man and cat dirs. */
#define MAN "man"
#define LCAT 3
#define CAT "cat"

#ifdef pegasus
#define SEARCHDIR  CAT
#else
#define SEARCHDIR  MAN
#endif

#define LSEARCHDIR LMAN		/* The directories to search we are making 
				 the assumption that the manual directories 
				 are more complete that the cat directories. 
				 but you can change it if you like. */

#define COPY "cp"		/* The unix copy command.  */

#define BACKSPACE 010		/* I doubt you would want to change this. */
#define MANDESC "mandesc"	/* Name of the mandesc files.  */
#ifdef macII
#define MANDIR "/usr/catman/u_man:/usr/catman/a_man"	/* The default manual page directory. */
#else
#define MANDIR "/usr/man"	/* The default manual page directory. */
#endif

#define INDENT 15
#define TYP20STR "MMMMMMMMMMMMMMMMMMMM"

#define FILE_SAVE "yes"
#define CANCEL_FILE_SAVE "no"
#define MANTEMP "/tmp/xmanXXXXXX"

/*
 * Macro Definitions.
 */

#define streq(a, b)        ( strcmp((a), (b)) == 0 )

/* 
 * Function definitions moved to man.h
 */
