/*
 * $XConsortium: externs.h,v 2.9 88/02/06 09:38:02 swick Exp $
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
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

extern int errno;

extern char *getenv();

extern XtTextSource XtCreateEDiskSource();

extern char *DoCommandToFile();
extern char *DoCommandToString();

extern void NoOp();
extern int myopen();
extern FILE *myfopen();
extern int myclose();
extern int myfclose();
extern char *MakeNewTempFileName();
extern char **MakeArgv();
extern char **ResizeArgv();
extern FILEPTR FOpenAndCheck();
extern char *ReadLine();
extern char *ReadLineWithCR();
extern char *MallocACopy();
extern int DenyGeoRequest();
extern char *CreateGeometry();
extern char *MakeFileName();
extern Widget CreateTextSW();
extern Widget CreateTitleBar();
extern MsgList CurMsgListOrCurMsg();
extern Toc SelectedToc();

extern Scrn CreateNewScrn();
extern Scrn NewViewScrn();
extern Scrn NewCompScrn();
extern void MapScrn();
extern void DestroyScrn();
extern void EnableProperButtons();
extern Scrn ScrnFromWidget();

extern Scrn LastButtonScreen();

extern char *Version();

XtTextSource TSourceCreate();

extern void IconInit();

extern void OpenFolder();
extern void OpenSequence();

extern int GetWidth();
extern int GetHeight();
