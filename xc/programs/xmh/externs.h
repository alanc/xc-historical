/*
 * $XConsortium: externs.h,v 2.17 89/07/21 18:56:08 converse Exp $
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

extern int errno;
extern char *getenv();

	/* from command.c */

extern char *	DoCommandToFile();
extern char *	DoCommandToString();

	/* from icon.c */

extern void	IconInit();

	/* from popup.c */

extern void	PopupPrompt();
extern void	PopupConfirm();
extern void	PopupNotice();
extern void 	PopupError();
extern void 	DestroyPopupAlert();
extern Widget	PopupAlert();

	/* from screen.c */

extern void	EnableProperButtons();
extern Scrn	CreateNewScrn();
extern Scrn	NewViewScrn();
extern Scrn	NewCompScrn();
extern void	ScreenSetAssocMsg();
extern void	DestroyScrn();
extern void	MapScrn();
extern Scrn	ScrnFromWidget();

	/* from tsource.c */

extern XawTextSource TSourceCreate();
extern XawTextSource CreateFileSource();

	/* from util.c */

extern void	Punt();
extern int	myopen();
extern FILE *	myfopen();
extern int	myclose();
extern int	myfclose();
extern char *	MakeNewTempFileName();
extern char **	MakeArgv();
extern char **	ResizeArgv();
extern FILEPTR	FOpenAndCheck();
extern char *	ReadLine();
extern char *	ReadLineWithCR();
extern void	DeleteFileAndCheck();
extern void	CopyFileAndCheck();
extern void	RenameAndCheck();
extern char *	CreateGeometry();
extern Boolean	IsSubFolder();
extern char *	MakeParentFolderName();
extern char *	MakeSubFolderName();
extern void 	SetCurrentFolderName();
extern void	ChangeLabel();
extern Widget	CreateTextSW();
extern Widget	CreateTitleBar();
extern void	Feep();
extern MsgList	CurMsgListOrCurMsg();
extern int	GetWidth();
extern int	GetHeight();
extern Toc	SelectedToc();
extern int	strncmpIgnoringCase();
extern void 	StoreWindowName();

	/* from version.c */

extern char *	Version();
