/*
* $XConsortium: Shell.h,v 1.21 89/12/13 08:57:08 swick Exp $
*/
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XtShell_h
#define _XtShell_h

/***********************************************************************
 *
 * Shell Widget
 *
 ***********************************************************************/
/*
 * Shell-specific resources names, classes, and a representation type.
 */

#ifdef XTSTRINGDEFINES

#define XtNiconName "iconName"
#define XtCIconName "IconName"
#define XtNiconPixmap "iconPixmap"
#define XtCIconPixmap "IconPixmap"
#define XtNiconWindow "iconWindow"
#define XtCIconWindow "IconWindow"
#define XtNiconMask "iconMask"
#define XtCIconMask "IconMask"
#define XtNwindowGroup "windowGroup"
#define XtCWindowGroup "WindowGroup"
#define XtNvisual "visual"
#define XtCVisual "Visual"
#define XtNtitleEncoding "titleEncoding"
#define XtCTitleEncoding "TitleEncoding"
 
#define XtNsaveUnder "saveUnder"
#define XtCSaveUnder "SaveUnder"
#define XtNtransient "transient"
#define XtCTransient "Transient"
#define XtNoverrideRedirect "overrideRedirect"
#define XtCOverrideRedirect "OverrideRedirect"
#define XtNtransientFor "transientFor"
#define XtCTransientFor "TransientFor"

#define XtNiconNameEncoding "iconNameEncoding"
#define XtCIconNameEncoding "IconNameEncoding"

#define XtNallowShellResize "allowShellResize"
#define XtCAllowShellResize "AllowShellResize"
#define XtNcreatePopupChildProc "createPopupChildProc"
#define XtCCreatePopupChildProc "CreatePopupChildProc"

#define XtNtitle "title"
#define XtCTitle "Title"

#define XtRAtom "Atom"

#else 
extern char XtNiconName[];
extern char XtCIconName[];
extern char XtNiconPixmap[];
extern char XtCIconPixmap[];
extern char XtNiconWindow[];
extern char XtCIconWindow[];
extern char XtNiconMask[];
extern char XtCIconMask[];
extern char XtNwindowGroup[];
extern char XtCWindowGroup[];
extern char XtNvisual[];
extern char XtCVisual[];
extern char XtNtitleEncoding[];
extern char XtCTitleEncoding[];
 
extern char XtNsaveUnder[];
extern char XtCSaveUnder[];
extern char XtNtransient[];
extern char XtCTransient[];
extern char XtNoverrideRedirect[];
extern char XtCOverrideRedirect[];
extern char XtNtransientFor[];
extern char XtCTransientFor[];

extern char XtNiconNameEncoding[];
extern char XtCIconNameEncoding[];

extern char XtNallowShellResize[];
extern char XtCAllowShellResize[];
extern char XtNcreatePopupChildProc[];
extern char XtCCreatePopupChildProc[];

extern char XtNtitle[];
extern char XtCTitle[];

extern char XtRAtom[];
#endif

/* 
 * The following are only used at creation and can not be changed via 
 * SetValues.
 */

#ifdef XTSTRINGDEFINES
#define XtNargc	"argc"
#define XtCArgc "Argc"
#define XtNargv "argv"
#define XtCArgv "Argv"
#define XtNiconX "iconX"
#define XtCIconX "IconX"
#define XtNiconY "iconY"
#define XtCIconY "IconY"
#define XtNinput "input"
#define XtCInput "Input"
#define XtNiconic "iconic"
#define XtCIconic "Iconic"
#define XtNinitialState "initialState"
#define XtCInitialState "InitialState"
#define XtNgeometry "geometry"
#define XtCGeometry "Geometry"
#define XtNbaseWidth "baseWidth"
#define XtCBaseWidth "BaseWidth"
#define XtNbaseHeight "baseHeight"
#define XtCBaseHeight "BaseHeight"
#define XtNwinGravity "winGravity"
#define XtCWinGravity "WinGravity"
#define XtNminWidth "minWidth"
#define XtCMinWidth "MinWidth"
#define XtNminHeight "minHeight"
#define XtCMinHeight "MinHeight"
#define XtNmaxWidth "maxWidth"
#define XtCMaxWidth "MaxWidth"
#define XtNmaxHeight "maxHeight"
#define XtCMaxHeight "MaxHeight"
#define XtNwidthInc "widthInc"
#define XtCWidthInc "WidthInc"
#define XtNheightInc "heightInc"
#define XtCHeightInc "HeightInc"
#define XtNminAspectY "minAspectY"
#define XtCMinAspectY "MinAspectY"
#define XtNmaxAspectY "maxAspectY"
#define XtCMaxAspectY "MaxAspectY"
#define XtNminAspectX "minAspectX"
#define XtCMinAspectX "MinAspectX"
#define XtNmaxAspectX "maxAspectX"
#define XtCMaxAspectX "MaxAspectX"
#define XtNwmTimeout "wmTimeout"
#define XtCWmTimeout "WmTimeout"
#define XtNwaitForWm "waitforwm"
#define XtCWaitForWm "Waitforwm"
#else
extern char XtNargc[];
extern char XtCArgc[];
extern char XtNargv[];
extern char XtCArgv[];
extern char XtNiconX[];
extern char XtCIconX[];
extern char XtNiconY[];
extern char XtCIconY[];
extern char XtNinput[];
extern char XtCInput[];
extern char XtNiconic[];
extern char XtCIconic[];
extern char XtNinitialState[];
extern char XtCInitialState[];
extern char XtNgeometry[];
extern char XtCGeometry[];
extern char XtNbaseWidth[];
extern char XtCBaseWidth[];
extern char XtNbaseHeight[];
extern char XtCBaseHeight[];
extern char XtNwinGravity[];
extern char XtCWinGravity[];
extern char XtNminWidth[];
extern char XtCMinWidth[];
extern char XtNminHeight[];
extern char XtCMinHeight[];
extern char XtNmaxWidth[];
extern char XtCMaxWidth[];
extern char XtNmaxHeight[];
extern char XtCMaxHeight[];
extern char XtNwidthInc[];
extern char XtCWidthInc[];
extern char XtNheightInc[];
extern char XtCHeightInc[];
extern char XtNminAspectY[];
extern char XtCMinAspectY[];
extern char XtNmaxAspectY[];
extern char XtCMaxAspectY[];
extern char XtNminAspectX[];
extern char XtCMinAspectX[];
extern char XtNmaxAspectX[];
extern char XtCMaxAspectX[];
extern char XtNwmTimeout[];
extern char XtCWmTimeout[];
extern char XtNwaitForWm[];
extern char XtCWaitForWm[];
#endif

/* Class record constants */

typedef struct _ShellClassRec *ShellWidgetClass;
typedef struct _OverrideShellClassRec *OverrideShellWidgetClass;
typedef struct _WMShellClassRec *WMShellWidgetClass;
typedef struct _TransientShellClassRec *TransientShellWidgetClass;
typedef struct _TopLevelShellClassRec *TopLevelShellWidgetClass;
typedef struct _ApplicationShellClassRec *ApplicationShellWidgetClass;

#ifndef SHELL 
externalref WidgetClass shellWidgetClass;
externalref WidgetClass overrideShellWidgetClass;
externalref WidgetClass wmShellWidgetClass;
externalref WidgetClass transientShellWidgetClass;
externalref WidgetClass topLevelShellWidgetClass;
externalref WidgetClass applicationShellWidgetClass;
#endif

#endif /* _XtShell_h */
/* DON'T ADD STUFF AFTER THIS #endif */
