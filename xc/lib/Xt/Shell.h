/*
* $Header: Shell.h,v 1.10 87/10/09 13:11:49 chow BL5 $
*/

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#ifndef _XtShell_h
#define _XtShell_h

/***********************************************************************
 *
 * Shell Widget
 *
 ***********************************************************************/
/*
 * Shell specific atoms
 */
#define XtNiconName	"iconName"
#define XtCIconName	"IconName"
#define XtNiconPixmap	"iconPixmap"
#define XtCIconPixmap	"IconPixmap"
#define XtNiconWindow	"iconWindow"
#define XtCIconWindow	"IconWindow"
#define XtNiconMask	"iconMask"
#define XtCIconMask	"IconMask"
#define XtNwindowGroup	"windowGroup"
#define XtCWindowGroup	"WindowGroup"

#define XtNiconifyPixmap	"iconifyPixmap"
#define XtCIconifyPixmap	"IconifyPixmap"
#define XtNiconBox		"iconBox"
#define XtCIconBox		"IconBox"
#define XtNiconBoxX		"iconBoxX"
#define XtCIconBoxX		"IconBoxX"
#define XtNiconBoxY		"iconBoxY"
#define XtCIconBoxY		"IconBoxY"
#define XtNtiled		"tiled"
#define XtCTiled		"Tiled"
#define XtNsticky		"sticky"
#define XtCSticky		"Sticky"
#define XtNnoIconify		"noIconify"
#define XtCNoIconify		"NoIconify"
#define XtNnoLower		"noLower"
#define XtCNoLower		"NoLower"
#define XtNnoResize		"noResize"
#define XtCNoResize		"NoResize"
#define XtNsaveUnder		"saveUnder"
#define XtCSaveUnder		"SaveUnder"
#define XtNtransient		"transient"
#define XtCTransient		"Transient"
#define XtNoverrideRedirect		"overrideRedirect"
#define XtCOverrideRedirect		"OverrideRedirect"

#define XtNallowshellresize	"allowShellResizeRequest"
#define XtCAllowshellresize	"AllowShellResizeRequest"
#define XtNcreatePopupChildProc	"createPopupChildProc"
#define XtCCreatePopupChildProc	"CreatePopupChildProc"

#define XtNtitle	"title"
#define XtCTitle	"Title"

/* 
 * The following are only used at creation and can not be changed via 
 * SetValues.
 */
#define XtNiconX	"iconX"
#define XtCIconX	"iconX"
#define XtNiconY	"iconY"
#define XtCIconY	"iconY"
#define XtNinput	"input"
#define XtCInput	"Input"
#define XtNiconic	"iconic"
#define XtCIconic	"Iconic"
#define XtNinitial	"initialstate"
#define XtCInitial	"InitialState"
#define XtNgeometry	"geometry"
#define XtCGeometry	"Geometry"
#define XtNminWidth	"minWidth"
#define XtCMinWidth	"MinWidth"
#define XtNminHeight	"minHeight"
#define XtCMinHeight	"MinHeight"
#define XtNmaxWidth	"maxWidth"
#define XtCMaxWidth	"MaxWidth"
#define XtNmaxHeight	"maxHeight"
#define XtCMaxHeight	"MaxHeight"
#define XtNwidthInc	"widthInc"
#define XtCWidthInc	"WidthInc"
#define XtNheightInc	"heightInc"
#define XtCHeightInc	"HeightInc"
#define XtNminAspectDenom	"minAspectDenom"
#define XtCMinAspectDenom	"MinAspectDenom"
#define XtNmaxAspectDenom	"maxAspectDenom"
#define XtCMaxAspectDenom	"MaxAspectDenom"
#define XtNminAspectNum		"minAspectNum"
#define XtCMinAspectNum		"MinAspectNum"
#define XtNmaxAspectNum		"maxAspectNum"
#define XtCMaxAspectNum		"MaxAspectNum"

/* Class record constants */

typedef struct _ShellClassRec *ShellWidgetClass;

extern WidgetClass shellWidgetClass;

#endif _XtShell_h
/* DON'T ADD STUFF AFTER THIS #endif */
