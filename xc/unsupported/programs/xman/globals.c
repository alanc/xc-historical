/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: globals.c,v 1.1 88/08/31 22:52:32 jim Exp $
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
  static char rcs_version[] = "$Athena: globals.c,v 4.0 88/08/31 22:11:49 kit Exp $";
#endif

#include "man.h"

/* Resource manager sets these. */

XmanFonts fonts;		/* The fonts used for the man pages. */
XmanCursors cursors;		/* The cursors for xman. */
Boolean both_shown_initial;	/* The initial state of the manual pages
				   show two screens or only one. */
Boolean top_box_active;	        /* Put up the Top Box. */
int directory_height;	        /* The default height of directory in 
				   both_shown mode. */
char * help_file;		/* The name of the help file. */

Widget help_widget;		/* The help widget. */

/* bookkeeping global variables. */

int default_height,default_width; /* Approximately the default with and
					    height, of the manpage when shown,
					    the the top level manual page 
					    window */

Manual manual[MAXSECT];		/* The manual structure. */
int sections;			/* The number of manual sections. */

int man_pages_shown;		/* The current number of manual
				   pages being shown, if 0 we exit. */

