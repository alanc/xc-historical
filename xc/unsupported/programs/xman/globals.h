/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: globals.h,v 1.1 88/08/31 22:52:33 jim Exp $
 * $Athena: globals.h,v 4.0 88/08/31 22:11:54 kit Exp $
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

#include "man.h"

/* Resource manager sets these. */

extern XmanFonts fonts;			/* The fonts used for the man pages. */
extern XmanCursors cursors;		/* The cursors for xman. */
extern Boolean both_shown_initial;      /* The initial state of the manual 
				         pages show two screens or only one. */
extern Boolean top_box_active;	        /* Put up the Top Box. */
extern int directory_height;	        /* The default height of directory in 
				            both_shown mode. */
extern char * help_file;		/* The name of the help file. */

extern Widget help_widget;		/* The help widget. */

/* bookkeeping global variables. */

extern int default_height,default_width; /* Approximately the default with and
					    height, of the manpage when shown,
					    the the top level manual page 
					    window */
extern int man_pages_shown;		/* The current number of manual
					   pages being shown, if 0 we exit. */

extern Manual manual[MAXSECT];		/* The manual structure. */
extern int sections;			/* The number of manual sections. */


