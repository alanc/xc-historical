/* $XConsortium: bbox.h,v 2.5 89/05/04 15:05:10 converse Exp $ */
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

#ifndef _bbox_h
#define _bbox_h

extern ButtonBox BBoxRadioCreate();
extern ButtonBox BBoxCreate();
extern void BBoxSetRadio();
extern char *BBoxGetRadioName();
extern void BBoxAddButton();
extern void BBoxDeleteButton();
extern void BBoxDeleteMenuEntry();
extern void BBoxEnable();
extern void BBoxDisable();
extern Button BBoxFindButtonNamed();
extern Button BBoxFindButton();
extern Button BBoxButtonNumber();
extern int BBoxNumButtons();
extern char *BBoxNameOfButton();
extern void BBoxLockSize();
extern void BBoxChangeBorderWidth();
#endif /* _bbox_h */
