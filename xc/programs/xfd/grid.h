/* $XConsortium: grid.h,v 1.9 94/02/06 20:12:30 rws Exp $ */
/*
 * Copyright 1993 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
#ifndef _FontGrid_h_
#define _FontGrid_h_

typedef struct _FontGridRec *FontGridWidget;
extern WidgetClass fontgridWidgetClass;

#define XtNcellRows "cellRows"
#define XtCCellRows "CellRows"
#define XtNcellColumns "cellColumns"
#define XtCCellColumns "CellColumns"
#define XtNcellWidth "cellWidth"
#define XtCCellWidth "CellWidth"
#define XtNcellHeight "cellHeight"
#define XtCCellHeight "CellHeight"

#define XtNcenterChars "centerChars"
#define XtCCenterChars "CenterChars"

#define XtNboxChars "boxChars"
#define XtCBoxChars "BoxChars"

#define XtNboxColor "boxColor"
#define XtCBoxColor "BoxColor"

#define XtNstartChar "startChar"
#define XtCStartChar "StartChar"

#define XtNinternalPad "internalPad"
#define XtCInternalPad "InternalPad"

#define XtNgridWidth "gridWidth"
#define XtCGridWidth "GridWidth"

typedef struct _FontGridCharRec {
    XFontStruct *	thefont;
    XChar2b		thechar;
} FontGridCharRec;

extern void GetFontGridCellDimensions(
#if NeedFunctionPrototypes
   Widget,
   Dimension *,
   int *,
   int *
#endif
);

extern void GetPrevNextStates(
#if NeedFunctionPrototypes
    Widget,
    Bool *,
    Bool *
#endif
);

#endif /* _FontGrid_h_ */
