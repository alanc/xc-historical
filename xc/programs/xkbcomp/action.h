/* $XConsortium: action.h,v 1.4 93/09/28 20:16:45 rws Exp $ */
/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be 
 used in advertising or publicity pertaining to distribution 
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability 
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.
 
 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#ifndef ACTION_H
#define ACTION_H 1

#define	F_ClearLocks	0
#define	F_LatchToLock	1
#define	F_GenKeyEvent	2
#define	F_Report	3
#define	F_Default	4
#define	F_Affect	5
#define	F_Increment	6
#define	F_Modifiers	7
#define	F_Group		8
#define	F_X		9
#define	F_Y		10
#define	F_Button	11
#define	F_Value		12
#define	F_Controls	13
#define	F_Type		14
#define	F_Count		15
#define	F_Screen	16
#define	F_Same		17
#define	F_Data		18
#define	F_LastField	F_Data
#define	F_NumFields	(F_LastField+1)

#define	PrivateAction	(XkbSA_LastAction+1)

typedef struct _ActionInfo {
	unsigned		action;
	unsigned		field;
	ExprDef *		array_ndx;
	ExprDef *		value;
	struct _ActionInfo *	next;
} ActionInfo;

extern int HandleActionDef(
#if NeedFunctionPrototypes
	ExprDef *		/* def */,
	XkbDescPtr		/* xkb */,
	XkbAnyAction *		/* action */,
	unsigned		/* mergeMode */,
	ActionInfo *		/* info */
#endif
);

extern int SetActionField(
#if NeedFunctionPrototypes
	XkbDescPtr		/* xkb */,
	char *			/* elem */,
	char *			/* field */,
	ExprDef *		/* index */,
	ExprDef *		/* value */,
	ActionInfo **		/* info_rtrn */
#endif
);

extern void ActionsInit(
#if NeedFunctionPrototypes
	void
#endif
);

#endif /* ACTION_H */
