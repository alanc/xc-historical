/*
 * $XConsortium: externs.h,v 2.21 89/09/15 16:10:25 converse Exp $
 */

/*
 *		       COPYRIGHT 1987, 1989
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
extern void	InitWaitCursor();

	/* from compfuncs.c */

extern void	XmhResetCompose();
extern void	XmhSend();
extern void	XmhSave();

	/* from folder.c */

extern void	XmhClose();
extern void	XmhComposeMessage();
extern void 	XmhOpenFolder();
extern void	XmhOpenFolderInNewWindow();
extern void	XmhCreateFolder();
extern void	XmhDeleteFolder();
extern void	XmhPopupFolderMenu();
extern void	XmhSetCurrentFolder();
extern void	XmhLeaveFolderButton();
extern void	XmhOpenFolderFromMenu();

	/* from icon.c */

extern void	IconInit();

	/* from menu.c */
extern void	CreateMenu();
extern void 	AddMenuEntry();
extern void	SendMenuEntryEnableMsg();

	/* from msg.c */

extern Widget   CreateFileSource();
extern void	XmhInsert();

	/* from popup.c */

extern void	XmhPromptOkayAction();
extern void	PopupPrompt();
extern void	PopupConfirm();
extern void	PopupNotice();
extern void 	PopupError();
extern void 	PopdownAlert();
extern void	PopupAlert();

	/* from screen.c */

extern void	EnableProperButtons();
extern Scrn	CreateNewScrn();
extern Scrn	NewViewScrn();
extern Scrn	NewCompScrn();
extern void	ScreenSetAssocMsg();
extern void	DestroyScrn();
extern void	MapScrn();
extern Scrn	ScrnFromWidget();

	/* from tocfuncs.c */

extern void	XmhIncorporateNewMail();
extern void 	Inc();
extern void	XmhCommitChanges();
extern void 	DoCommit();
extern void	DoPack();
extern void	XmhPackFolder();
extern void	DoSort();
extern void	XmhSortFolder();
extern void 	Rescan();
extern void	XmhForceRescan();
extern void	DoNextView();
extern void	XmhViewNextMessage();
extern void	DoPrevView();
extern void	XmhViewPreviousMessage();
extern void	DoDelete();
extern void	XmhMarkDelete();
extern void	DoMove();
extern void	XmhMarkMove();
extern void	DoCopy();
extern void	XmhMarkCopy();
extern void	DoUnmark();
extern void	XmhUnmark();
extern void	DoViewNew();
extern void	XmhViewInNewWindow();
extern void	Reply();
extern void	XmhReply();
extern void	DoForward();
extern void	XmhForward();
extern void	DoTocUseAsComp();
extern void	XmhUseAsComposition();
extern void	DoPrint();
extern void	XmhPrint();
extern void	DoPickMessages();
extern void	XmhPickMessages();
extern void	DoOpenSeq();
extern void	XmhOpenSequence();
extern void 	DoAddToSeq();
extern void	XmhAddToSequence();
extern void 	DoRemoveFromSeq();
extern void	XmhRemoveFromSequence();
extern void	DoDeleteSeq();
extern void	XmhDeleteSequence();

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
extern Boolean	IsSubfolder();
extern char *	MakeParentFolderName();
extern char *	MakeSubfolderName();
extern char *	MakeSubfolderLabel();
extern void 	SetCurrentFolderName();
extern void	ChangeLabel();
extern Widget	CreateTextSW();
extern Widget	CreateTitleBar();
extern void	Feep();
extern MsgList	CurMsgListOrCurMsg();
extern int	GetWidth();
extern int	GetHeight();
extern Toc	SelectedToc();
extern Toc	CurrentToc();
extern int	strncmpIgnoringCase();
extern void 	StoreWindowName();

	/* from version.c */

extern char *	Version();

	/* from viewfuncs.c */

extern void	DoCloseView();
extern void	XmhCloseView();
extern void	DoViewReply();
extern void	XmhViewReply();
extern void 	DoViewForward();
extern void	XmhViewForward();
extern void	DoViewUseAsComposition();
extern void	XmhViewUseAsComposition();
extern void	DoEditView();
extern void	XmhEditView();
extern void	DoSaveView();
extern void	XmhSaveView();
extern void	DoPrintView();
extern void	XmhPrintView();
