/*
 * xman - X window system manual page display program.
 *
 * $Header: defs.h,v 4.0 88/08/31 22:11:42 kit Exp $
 * $Athena: defs.h,v 4.0 88/08/31 22:11:42 kit Exp $
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
#define MANPAGE_NORMAL   "8x13"
#define MANPAGE_BOLD     "8x13bold"
#define MANPAGE_ITALIC   "8x13"
#define DIRECTORY_NORMAL "8x13"
#endif ATHENA

#define TOPBOXNAME  "topBox"	/* Name of the Top Box. */
#define MANNAME "manualBrowser"	/* name for each manual page widget. */
#define POPUPNAME "xmanCommands" /* The name of the popup menu */
#define SPOPUPNAME "xmanSections" /* The name of the section popup name. */
#define SEARCHNAME "xmanSearch" /* The name for the search widget. */
#define HELPNAME  "help"	/* The name of the help widget. */
#define DIRECTORY_NAME "directory" /* name of the directory widget. */
#define MANUALPAGE "manualpage"	/* name of the Scrollbyline widget that
				 contains the man page. */

/* definitions of string to use for show both and show one. */

#define SHOW_BOTH "Show Both Screens"
#define SHOW_ONE "Show One Screen"

/* 
 * Things will not look right if you change these names to make 
 * MANUALSEARCH longer APROPOSSEARCH, see search.c for details.
 */

#define MANUALSEARCH "Manual Page"
#define APROPOSSEARCH "Apropos"

#define MANUAL 0
#define APROPOS 1

#define INIT_SEARCH_STRING "xman"     /* Intial search string. */
#define SEARCH_STRING_LENGTH 30

/*
 * The command filters for the manual and apropos searches.
 */

#define APROPOSCOMMAND "apropos -M"
#define APROPOSFILTER "pr -h Apropos"
#define MANUALCOMMAND "man -M"
#define FORMAT "nroff -man"	/* The format command. */

#define CANCEL "Cancel"

#define MAXSECT 25		/* The maximum number of sections. */
#define FIXEDSECT 11		/* The number of predefined sections.
				   0 - 8 and n, and l = 11. */
#define MAXENTRY 800		/* The maximum number of entries in one 
				   section. */
#define NLINES  66		/* This is the number of lines to wait until
				   we boldify the line again, this allows 
				   me to bold the first line of each page.*/


#define LMAN 3			/* Name and length of the man and cat dirs. */
#define MAN "man"
#define LCAT 3
#define CAT "cat"

#define SEARCHDIR  MAN
#define LSEARCHDIR LMAN		/* The directories to search we are making 
				 the assumption that the manual directories 
				 are more complete that the cat directories. 
				 but you can change it if you like. */

#define COPY "cp"		/* The unix copy command.  */

#define BACKSPACE 010		/* I doubt you would want to change this. */
#define MANDESC "mandesc"	/* Name of the mandesc files.  */
#define MANDIR "/usr/man"	/* The default manual page directory. */

#define INDENT 15
#define TYP20STR "MMMMMMMMMMMMMMMMMMMM"

#define FILE_SAVE "Yes"
#define CANCEL_FILE_SAVE "No"
#define MANTEMP "/tmp/xmanXXXXXX"

/* 
 * function defintions 
 */

/* Standard library function definitions. */

char * mktemp(), * getenv(), * malloc();
void exit();

/* Toolkit standard definitions. */

void XtResizeWidget(), XtMoveWidget();

/* buttons.c */

void MakeTopMenuWidget(), CreateManpage(), StartManpage();
void CreateManpageWidget(), MakeSaveWidgets(), WriteLabel();
void MakeTopPopUpWidget(),MakeDirPopUpWidget(), MakeDirectoryBox();
char * CreateManpageName();

/* handler.c */

void DirectoryHandler(),SearchCallback(),PopUpMenu(),SaveCallback();
void TopCallback(),TopPopUpCallback(),DirPopUpCallback();
void PopDown(),ManpageButtonPress(), GotoManpage();

/* help.c */

void PopupHelp();
Boolean MakeHelpWidget(), OpenHelpfile();

/* main.c */

void main(),Quit();

/* man.c */

int Man();
void MatchEntries(),AddStruct(),SortAndRemove();
Boolean GetEntry();
void SetManNames();
char * StrAlloc();

/* menu.c is self contained */

/* misc.c */

void PrintError(),PrintWarning(),KillManpage(), ChangeLabel();
void RemovePixmaps(),PositionCenter(),AddCursor();
FILE *FindFilename(),*Format(), *OpenEntryFile();

/* pages.c */

void InitManpage();
void PrintManpage();
Boolean Boldify();

/* search */

void MakeSearchWidget();
FILE * DoSearch();

/* tkfunctions.c */

int Width(), Height(), BorderWidth();
Widget PopupChild(), Child();
char * Name();
Boolean MakeLong();
