/*
 * $XConsortium: Tree.h,v 1.5 90/02/05 11:51:15 jim Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 * Copyright 1989 Prentice Hall
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation.
 * 
 * M.I.T., Prentice Hall and the authors disclaim all warranties with regard
 * to this software, including all implied warranties of merchantability and
 * fitness.  In no event shall M.I.T., Prentice Hall or the authors be liable
 * for any special, indirect or cosequential damages or any damages whatsoever
 * resulting from loss of use, data or profits, whether in an action of
 * contract, negligence or other tortious action, arising out of or in
 * connection with the use or performance of this software.
 * 
 * Authors:  Jim Fulton, MIT X Consortium,
 *           based on a version by Douglas Young, Prentice Hall
 * 
 * This widget is based on the Tree widget described on pages 397-419 of
 * Douglas Young's book "The X Window System, Programming and Applications 
 * with Xt OSF/Motif Edition."  The layout code has been rewritten to use
 * additional blank space to make the structure of the graph easier to see
 * as well as to support vertical trees.
 */


#ifndef _XawTree_h
#define _XawTree_h

/******************************************************************************
 * 
 * Tree Widget (subclass of ConstraintClass)
 * 
 ******************************************************************************
 * 
 * Parameters:
 * 
 *  Name                Class              Type            Default
 *  ----                -----              ----            -------
 * 
 *  autoReconfigure     AutoReconfigure    Boolean         FALSE
 *  background          Background         Pixel           XtDefaultBackground
 *  foreground          Foreground         Pixel           XtDefaultForeground
 *  hSpace              HSpace             Dimension       20
 *  lineWidth           LineWidth          Dimension       0
 *  orientation         Orientation        XtOrientation   XtorientHorizontal
 *  vSpace              VSpace             Dimension       6
 * 
 * 
 * Constraint Resources attached to children:
 * 
 *  treeGC              TreeGC             GC              NULL
 *  treeParent          TreeParent         Widget          NULL
 * 
 * 
 *****************************************************************************/

                                        /* new instance field names */
#define XtNautoReconfigure "autoReconfigure"
#define XtNhSpace "hSpace"
#define XtNlineWidth "lineWidth"
#define XtNtreeGC "treeGC"
#define XtNtreeParent "treeParent"
#define XtNvSpace "vSpace"

                                        /* new class field names */
#define XtCAutoReconfigure "AutoReconfigure"
#define XtCHSpace "HSpace"
#define XtCLineWidth "LineWidth"
#define XtCTreeGC "TreeGC"
#define XtCTreeParent "TreeParent"
#define XtCVSpace "VSpace"

#define XtRGC "GC"
                                        /* external declarations */
extern WidgetClass treeWidgetClass;

typedef struct _TreeClassRec *TreeWidgetClass;
typedef struct _TreeRec      *TreeWidget;

extern void XawTreeForceLayout (
#if NeedFunctionPrototypes
    TreeWidget /* tree */
#endif
);

#endif /* _XawTree_h */
