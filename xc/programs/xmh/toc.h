/* $Header$ */
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

#ifndef _toc_h
#define _toc_h

extern void TocInit();
extern Toc TocCreateFolder();
extern void TocDeleteFolder();
extern void TocCheckForNewMail();
extern void TocSetScrn();
extern void TocRemoveMsg();
extern void TocRecheckValidity();
extern void TocSetCurMsg();
extern Msg TocGetCurMsg();
extern Msg TocMsgAfter();
extern Msg TocMsgBefore();
extern void TocForceRescan();
extern void TocReloadSeqLists();
extern int TocHasSequences();
extern void TocChangeViewedSeq();
extern Sequence TocViewedSequence();
extern Sequence TocGetSeqNamed();
extern MsgList TocCurMsgList();
extern void TocUnsetSelection();
extern Msg TocMakeNewMsg();
extern void TocStopUpdate();
extern void TocStartUpdate();
extern void TocSetCacheValid();
extern char *TocGetFolderName();
extern Toc TocGetNamed();
extern int TocConfirmCataclysm();
extern void TocCommitChanges();
extern int TocCanIncorporate();
extern void TocIncorporate();
extern void TocMsgChanged();
extern Msg TocMsgFromId();

#endif _toc_h
