/*
 *  $XConsortium: globals.h,v 2.24 89/09/15 16:10:27 converse Exp $
 */

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
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
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

#ifdef MAIN
#define ext
#else
#define ext extern
#endif

ext Display	*theDisplay;	/* Display variable. */
ext Widget	toplevel;	/* The top level widget (A hack %%%). */
ext char	*progName;	/* Program name. */
ext char	*homeDir;	/* User's home directory. */

ext struct _resources {
    Boolean	debug;
    char	*mailDir;		/* mh's mail directory. */
    char	*tempDir;		/* Directory for temporary files. */
    char	*defMhPath;		/* Path for mh commands. */
    char	*initialFolderName;	/* Initial folder to use. */
    char	*initialIncFile;	/* -file for inc on initial folder */
    char	*defInsertFilter;	/* Insert message filter command */
    char	*draftsFolderName;	/* Folder for drafts. */
    int		defSendLineWidth;	/* How long to break lines on send. */
    int		defBreakSendLineWidth;	/* Minimum length of a line before
					   we'll break it. */
    char	*defPrintCommand;	/* Printing command. */
    int		defTocWidth;	/* How many characters wide to use in tocs */
    Boolean	SkipDeleted;		/* If true, skip over deleted msgs. */
    Boolean	SkipMoved;		/* If true, skip over moved msgs. */
    Boolean	SkipCopied;		/* If true, skip over copied msgs. */
    Boolean	defHideBoringHeaders;
    char	*defGeometry;	/* Default geometry to use for things. */
    char	*defTocGeometry;
    char	*defViewGeometry;
    char	*defCompGeometry;
    char	*defPickGeometry;
    int		defTocPercentage;
    Boolean	defNewMailCheck;	/* Whether to check for new mail. */
    Boolean	defMakeCheckpoints; /* Whether to create checkpoint files. */
    int		check_frequency;	/* in minutes, of new mail check */
    int		mailWaitingFlag;	/* If true, change icon on new mail */
    Cursor	cursor;			/* application cursor */
} app_resources;

ext char	*draftFile;	/* Filename of draft. */
ext char	*xmhDraftFile;	/* Filename for sending. */
ext Toc		*folderList;	/* Array of folders. */
ext int		numFolders;	/* Number of entries in above array. */
ext Toc		InitialFolder;	/* Toc containing initial folder. */
ext Toc		DraftsFolder;	/* Toc containing drafts. */
ext Scrn	*scrnList;	/* Array of scrns in use. */
ext int		numScrns;	/* Number of scrns in above array. */
ext Widget	NoMenuForButton;/* Flag menu widget value: no menu */
ext Widget      NullSource;	/* null text widget source */
ext Dimension	rootwidth;	/* Dimensions of root window.  */
ext Dimension	rootheight;
ext Pixmap	NoMailPixmap;	/* Icon pixmap if no new mail. */
ext Pixmap	NewMailPixmap;	/* Icon pixmap if new mail. */

ext struct _LastInput {
    Window win;
    int x;
    int y;
} lastInput;

ext Boolean	subProcessRunning; /* interlock for DoCommand/CheckMail */

#define PNullSource (NullSource != NULL ? NullSource : (Widget) \
 		     CreateFileSource(scrn->viewlabel, "/dev/null", False))
