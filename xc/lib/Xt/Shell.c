#ifndef lint
static char Xrcsid[] = "$XConsortium: Shell.c,v 1.67 89/09/29 14:42:56 swick Exp $";
/* $oHeader: Shell.c,v 1.7 88/09/01 11:57:00 asente Exp $ */
#endif /* lint */

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

#define SHELL

#include <pwd.h>
#include <stdio.h>
#include <sys/param.h>

#ifdef pegasus
#undef dirty			/* some bozo put this in sys/param.h */
#endif /* pegasus */

#include <X11/Xatom.h>

extern void XSetTransientForHint(); /* this was not declared in Xlib.h... */

#include "IntrinsicI.h"
#include "StringDefs.h"
#include "Shell.h"
#include "ShellP.h"
#include "Vendor.h"
#include "VendorP.h"

#define WM_CONFIGURE_DENIED(w) (((WMShellWidget) (w))->wm.wm_configure_denied)
#define WM_MOVED(w) (((WMShellWidget) (w))->wm.wm_moved)

#define BIGSIZE ((Dimension)32767)

/***************************************************************************
 *
 * Default values for resource lists
 *
 ***************************************************************************/

static void ShellDepth();
static void ShellColormap();
static void ShellAncestorSensitive();

/***************************************************************************
 *
 * Shell class record
 *
 ***************************************************************************/

#define Offset(x)	(XtOffsetOf(ShellRec, x))
static XtResource shellResources[]=
{
	{XtNx, XtCPosition, XtRPosition, sizeof(Position),
	    Offset(core.x), XtRImmediate, (XtPointer)BIGSIZE},
	{XtNy, XtCPosition, XtRPosition, sizeof(Position),
	    Offset(core.y), XtRImmediate, (XtPointer)BIGSIZE},
	{ XtNdepth, XtCDepth, XtRInt, sizeof(int),
	    Offset(core.depth), XtRCallProc, (XtPointer) ShellDepth},
	{ XtNcolormap, XtCColormap, XtRPointer, sizeof(Colormap),
	    Offset(core.colormap), XtRCallProc, (XtPointer) ShellColormap},
	{ XtNancestorSensitive, XtCSensitive, XtRBoolean, sizeof(Boolean),
	    Offset(core.ancestor_sensitive), XtRCallProc,
	    (XtPointer) ShellAncestorSensitive},
	{ XtNallowShellResize, XtCAllowShellResize, XtRBoolean,
	    sizeof(Boolean), Offset(shell.allow_shell_resize),
	    XtRImmediate, (XtPointer)False},
	{ XtNgeometry, XtCGeometry, XtRString, sizeof(XtPointer), 
	    Offset(shell.geometry), XtRString, (XtPointer) NULL},
	{ XtNcreatePopupChildProc, XtCCreatePopupChildProc, XtRFunction,
	    sizeof(XtCreatePopupChildProc), Offset(shell.create_popup_child_proc),
	    XtRFunction, NULL},
	{ XtNsaveUnder, XtCSaveUnder, XtRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XtRImmediate, (XtPointer)False},
	{ XtNpopupCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	    Offset(shell.popup_callback), XtRCallback, (XtPointer) NULL},
	{ XtNpopdownCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	    Offset(shell.popdown_callback), XtRCallback, (XtPointer) NULL},
	{ XtNoverrideRedirect, XtCOverrideRedirect,
	    XtRBoolean, sizeof(Boolean), Offset(shell.override_redirect),
	    XtRImmediate, (XtPointer)False},
	{ XtNvisual, XtCVisual, XtRVisual, sizeof(Visual*),
	    Offset(shell.visual), XtRImmediate, CopyFromParent}
};

static void ClassPartInitialize(), Initialize();
static void Realize();
static void Resize();
static Boolean SetValues();
static void ChangeManaged(); /* XXX */
static XtGeometryResult GeometryManager(), RootGeometryManager();

static ShellClassExtensionRec shellClassExtRec = {
    NULL,
    NULLQUARK,
    XtShellExtensionVersion,
    sizeof(ShellClassExtensionRec),
    RootGeometryManager
};

externaldef(shellclassrec) ShellClassRec shellClassRec = {
  {   /* Core */
    /* superclass	  */	(WidgetClass) &compositeClassRec,
    /* class_name	  */	"Shell",
    /* size		  */	sizeof(ShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	ClassPartInitialize,
    /* Class init'ed ?	  */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_notify  */	NULL,		
    /* realize		  */	Realize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	shellResources,
    /* resource_count	  */	XtNumber(shellResources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	FALSE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	Resize,
    /* expose		  */	NULL,
    /* set_values	  */	SetValues,
    /* set_values_hook	  */	NULL,			
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook	  */	NULL,			
    /* accept_focus	  */	NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets	  */	NULL,
    /* tm_table		  */	NULL,
    /* query_geometry	  */	NULL,
    /* display_accelerator*/	NULL,
    /* extension	  */	NULL
  },{ /* Composite */
    /* geometry_manager	  */	GeometryManager,
    /* change_managed	  */	ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL
  },{ /* Shell */
    /* extension	  */	(XtPointer)&shellClassExtRec
  }
};

externaldef(shellwidgetclass) WidgetClass shellWidgetClass = (WidgetClass) (&shellClassRec);

/***************************************************************************
 *
 * OverrideShell class record
 *
 ***************************************************************************/

static XtResource overrideResources[]=
{
	{ XtNoverrideRedirect, XtCOverrideRedirect,
	    XtRBoolean, sizeof(Boolean), Offset(shell.override_redirect),
	    XtRImmediate, (XtPointer)True},
	{ XtNsaveUnder, XtCSaveUnder, XtRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XtRImmediate, (XtPointer)True},
};

externaldef(overrideshellclassrec) OverrideShellClassRec overrideShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &shellClassRec,
    /* class_name         */    "OverrideShell",
    /* size               */    sizeof(OverrideShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    NULL,
    /* initialize_notify    */	NULL,		
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    overrideResources,
    /* resource_count     */	XtNumber(overrideResources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook      */	NULL,			
    /* set_values_almost    */	XtInheritSetValuesAlmost,  
    /* get_values_hook      */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		    */  NULL,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */  NULL,
    /* extension	    */  NULL
  },{
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  }
};

externaldef(overrideshellwidgetclass) WidgetClass overrideShellWidgetClass = 
	(WidgetClass) (&overrideShellClassRec);

/***************************************************************************
 *
 * WMShell class record
 *
 ***************************************************************************/

#undef Offset
#define Offset(x)	(XtOffsetOf(WMShellRec, x))

static XtResource wmResources[]=
{
	{ XtNtitle, XtCTitle, XtRString, sizeof(char *),
	    Offset(wm.title), XtRString, NULL},
	{ XtNtitleEncoding, XtCEncoding, XtRAtom, sizeof(Atom),
	    Offset(wm.title_encoding), XtRImmediate, (XtPointer)XA_STRING},
	{ XtNwmTimeout, XtCWmTimeout, XtRInt, sizeof(int),
	    Offset(wm.wm_timeout), XtRImmediate, (XtPointer)5000},
	{ XtNwaitForWm, XtCWaitForWm, XtRBoolean, sizeof(Boolean),
	    Offset(wm.wait_for_wm), XtRImmediate, (XtPointer)True},
	{ XtNtransient, XtCTransient, XtRBoolean, sizeof(Boolean),
	    Offset(wm.transient), XtRImmediate, (XtPointer)False},
/* size_hints minus things stored in core */
	{ XtNbaseWidth, XtCBaseWidth, XtRInt, sizeof(int),
	    Offset(wm.base_width),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNbaseHeight, XtCBaseHeight, XtRInt, sizeof(int),
	    Offset(wm.base_height),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNwinGravity, XtCWinGravity, XtRInt, sizeof(int),
	    Offset(wm.win_gravity),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNminWidth, XtCMinWidth, XtRInt, sizeof(int),
	    Offset(wm.size_hints.min_width),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNminHeight, XtCMinHeight, XtRInt, sizeof(int),
	    Offset(wm.size_hints.min_height),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNmaxWidth, XtCMaxWidth, XtRInt, sizeof(int),
	    Offset(wm.size_hints.max_width),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNmaxHeight, XtCMaxHeight, XtRInt, sizeof(int),
	    Offset(wm.size_hints.max_height),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNwidthInc, XtCWidthInc, XtRInt, sizeof(int),
	    Offset(wm.size_hints.width_inc),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNheightInc, XtCHeightInc, XtRInt, sizeof(int),
	    Offset(wm.size_hints.height_inc),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNminAspectX, XtCMinAspectX, XtRInt, sizeof(int),
	    Offset(wm.size_hints.min_aspect.x),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNminAspectY, XtCMinAspectY, XtRInt, sizeof(int),
	    Offset(wm.size_hints.min_aspect.y),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNmaxAspectX, XtCMaxAspectX, XtRInt, sizeof(int),
	    Offset(wm.size_hints.max_aspect.x),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNmaxAspectY, XtCMaxAspectY, XtRInt, sizeof(int),
	    Offset(wm.size_hints.max_aspect.y),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
/* wm_hints */
	{ XtNinput, XtCInput, XtRBool, sizeof(Bool),
	    Offset(wm.wm_hints.input), XtRImmediate, (XtPointer)False},
	{ XtNinitialState, XtCInitialState, XtRInitialState, sizeof(int),
	    Offset(wm.wm_hints.initial_state),
	    XtRImmediate, (XtPointer)NormalState},
	{ XtNiconPixmap, XtCIconPixmap, XtRBitmap, sizeof(XtPointer),
	    Offset(wm.wm_hints.icon_pixmap), XtRPixmap, NULL},
	{ XtNiconWindow, XtCIconWindow, XtRWindow, sizeof(XtPointer),
	    Offset(wm.wm_hints.icon_window), XtRWindow,   (XtPointer) NULL},
	{ XtNiconX, XtNiconX, XtRInt, sizeof(int),
	    Offset(wm.wm_hints.icon_x),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNiconY, XtNiconY, XtRInt, sizeof(int),
	    Offset(wm.wm_hints.icon_y),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNiconMask, XtCIconMask, XtRBitmap, sizeof(XtPointer),
	    Offset(wm.wm_hints.icon_mask), XtRPixmap, NULL},
	{ XtNwindowGroup, XtCWindowGroup, XtRWindow, sizeof(XID),
	    Offset(wm.wm_hints.window_group),
	    XtRImmediate, (XtPointer)XtUnspecifiedWindow}
};

static void WMInitialize();
static Boolean WMSetValues();
static void WMDestroy();

externaldef(wmshellclassrec) WMShellClassRec wmShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &shellClassRec,
    /* class_name         */    "WMShell",
    /* size               */    sizeof(WMShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    WMInitialize,
    /* initialize_notify    */	NULL,		
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    wmResources,
    /* resource_count     */	XtNumber(wmResources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    WMDestroy,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    WMSetValues,
    /* set_values_hook      */	NULL,			
    /* set_values_almost    */	XtInheritSetValuesAlmost,  
    /* get_values_hook      */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		    */  NULL,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */  NULL,
    /* extension	    */  NULL
  },{
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  }
};

externaldef(wmshellwidgetclass) WidgetClass wmShellWidgetClass = (WidgetClass) (&wmShellClassRec);

/***************************************************************************
 *
 * TransientShell class record
 *
 ***************************************************************************/

#undef Offset
#define Offset(x)	(XtOffsetOf(TransientShellRec, x))

static XtResource transientResources[]=
{
	{ XtNtransient, XtCTransient, XtRBoolean, sizeof(Boolean),
	    Offset(wm.transient), XtRImmediate, (XtPointer)True},
	{ XtNtransientFor, XtCTransientFor, XtRWidget, sizeof(Widget),
	    Offset(transient.transient_for), XtRWidget, NULL},
	{ XtNsaveUnder, XtCSaveUnder, XtRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XtRImmediate, (XtPointer)True},
};

static void TransientRealize();
static Boolean TransientSetValues();

externaldef(transientshellclassrec) TransientShellClassRec transientShellClassRec = {
  {
    /* superclass	  */	(WidgetClass) &vendorShellClassRec,
    /* class_name	  */	"TransientShell",
    /* size		  */	sizeof(TransientShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?	  */	FALSE,
    /* initialize	  */	NULL,
    /* initialize_notify  */	NULL,		
    /* realize		  */	TransientRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	transientResources,
    /* resource_count	  */	XtNumber(transientResources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	FALSE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	XtInheritResize,
    /* expose		  */	NULL,
    /* set_values	  */	TransientSetValues,
    /* set_values_hook	  */	NULL,			
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook	  */	NULL,			
    /* accept_focus	  */	NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets	  */	NULL,
    /* tm_table		  */	XtInheritTranslations,
    /* query_geometry	  */	NULL,
    /* display_accelerator*/	NULL,
    /* extension	  */	NULL
  },{
    /* geometry_manager	  */	XtInheritGeometryManager,
    /* change_managed	  */	XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL
  },{
    /* extension	  */	NULL
  },{
    /* extension	  */	NULL
  },{
    /* extension	  */	NULL
  },{
    /* extension	  */	NULL
  }
};

externaldef(transientshellwidgetclass) WidgetClass transientShellWidgetClass =
	(WidgetClass) (&transientShellClassRec);

/***************************************************************************
 *
 * TopLevelShell class record
 *
 ***************************************************************************/

#undef Offset
#define Offset(x)	(XtOffsetOf(TopLevelShellRec, x))

static XtResource topLevelResources[]=
{
	{ XtNiconName, XtCIconName, XtRString, sizeof(XtPointer),
	    Offset(topLevel.icon_name), XtRString, (XtPointer) NULL},
	{ XtNiconNameEncoding, XtCEncoding, XtRAtom, sizeof(Atom),
	    Offset(topLevel.icon_name_encoding), XtRImmediate,
	    (XtPointer)XA_STRING},
	{ XtNiconic, XtCIconic, XtRBoolean, sizeof(Boolean),
	    Offset(topLevel.iconic), XtRImmediate, (XtPointer)False}
};

static void TopLevelInitialize();
static Boolean TopLevelSetValues();
static void TopLevelDestroy();

externaldef(toplevelshellclassrec) TopLevelShellClassRec topLevelShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &vendorShellClassRec,
    /* class_name         */    "TopLevelShell",
    /* size               */    sizeof(TopLevelShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    TopLevelInitialize,
    /* initialize_notify    */	NULL,		
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    topLevelResources,
    /* resource_count     */	XtNumber(topLevelResources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    TopLevelDestroy,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    TopLevelSetValues,
    /* set_values_hook      */	NULL,			
    /* set_values_almost    */	XtInheritSetValuesAlmost,  
    /* get_values_hook      */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		    */  XtInheritTranslations,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */  NULL,
    /* extension	    */  NULL
  },{
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  }
};

externaldef(toplevelshellwidgetclass) WidgetClass topLevelShellWidgetClass =
	(WidgetClass) (&topLevelShellClassRec);

/***************************************************************************
 *
 * ApplicationShell class record
 *
 ***************************************************************************/

#undef Offset
#define Offset(x)	(XtOffsetOf(ApplicationShellRec, x))

static XtResource applicationResources[]=
{
	{ XtNargc, XtCArgc, XtRInt, sizeof(int),
	    Offset(application.argc), XtRImmediate, (XtPointer)0}, 
	{ XtNargv, XtCArgv, XtRPointer, sizeof(XtPointer),
	    Offset(application.argv), XtRPointer, (XtPointer) NULL}
};

static void ApplicationInitialize();
static void ApplicationDestroy();

externaldef(applicationshellclassrec) ApplicationShellClassRec applicationShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &topLevelShellClassRec,
    /* class_name         */    "ApplicationShell",
    /* size               */    sizeof(ApplicationShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    ApplicationInitialize,
    /* initialize_notify    */	NULL,		
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    applicationResources,
    /* resource_count     */	XtNumber(applicationResources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/    FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    ApplicationDestroy,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook      */	NULL,			
    /* set_values_almost    */	XtInheritSetValuesAlmost,
    /* get_values_hook      */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		    */  XtInheritTranslations,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */  NULL,
    /* extension	    */  NULL
  },{
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
  }
};

externaldef(applicationshellwidgetclass) WidgetClass applicationShellWidgetClass =
	(WidgetClass) (&applicationShellClassRec);

/****************************************************************************
 * Whew!
 ****************************************************************************/

static void ComputeWMSizeHints(w, hints)
    WMShellWidget w;
    XSizeHints *hints;
{
    register long flags;
    hints->flags = flags = w->wm.size_hints.flags;
#define copy(field) hints->field = w->wm.size_hints.field
    if (flags & (USPosition | PPosition)) {
	copy(x);
	copy(y);
    }
    if (flags & (USSize | PSize)) {
	copy(width);
	copy(height);
    }
    if (flags & PMinSize) {
	copy(min_width);
	copy(min_height);
    }
    if (flags & PMaxSize) {
	copy(max_width);
	copy(max_height);
    }
    if (flags & PResizeInc) {
	copy(width_inc);
	copy(height_inc);
    }
    if (flags & PAspect) {
	copy(min_aspect.x);
	copy(min_aspect.y);
	copy(max_aspect.x);
	copy(max_aspect.y);
    }
#undef copy
#define copy(field) hints->field = w->wm.field
    if (flags & PBaseSize) {
	copy(base_width);
	copy(base_height);
    }
    if (flags & PWinGravity)
	copy(win_gravity);
#undef copy
}

static void _SetWMSizeHints(w)
    WMShellWidget w;
{
    XSizeHints *size_hints = XAllocSizeHints();

    if (size_hints == NULL) _XtAllocError("XAllocSizeHints");
    ComputeWMSizeHints(w, size_hints);
    XSetWMNormalHints(XtDisplay((Widget)w), XtWindow((Widget)w), size_hints);
}

static ShellClassExtension _FindClassExtension(widget_class)
    WidgetClass widget_class;
{
    ShellClassExtension ext;
    for (ext = (ShellClassExtension)((ShellWidgetClass)widget_class)
	       ->shell_class.extension;
	 ext != NULL && ext->record_type != NULLQUARK;
	 ext = (ShellClassExtension)ext->next_extension);

    if (ext != NULL) {
	if (  ext->version == XtShellExtensionVersion
	      && ext->record_size == sizeof(ShellClassExtensionRec)) {
	    /* continue */
	} else {
	    String params[1];
	    Cardinal num_params = 1;
	    params[0] = widget_class->core_class.class_name;
	    XtErrorMsg( "invalidExtension", "shellClassPartInitialize",
		        "XtToolkitError",
		 "widget class %s has invalid ShellClassExtension record",
		 params, &num_params);
	}
    }
    return ext;
}

static void ClassPartInitialize(widget_class)
    WidgetClass widget_class;
{
    ShellClassExtension ext = _FindClassExtension(widget_class);
    if (ext != NULL) {
	if (ext->root_geometry_manager == XtInheritRootGeometryManager) {
	    ext->root_geometry_manager =
		_FindClassExtension(widget_class->core_class.superclass)
		    ->root_geometry_manager;
	}
    } else {
	/* if not found, spec requires XtInheritRootGeometryManager */
	XtPointer *extP
	    = &((ShellWidgetClass)widget_class)->shell_class.extension;
	ext = XtNew(ShellClassExtensionRec);
	bcopy((char*)_FindClassExtension(widget_class->core_class.superclass),
	      (char*)ext,
	      sizeof(ShellClassExtensionRec));
	ext->next_extension = *extP;
	*extP = (XtPointer)ext;
    }
}


static void EventHandler();
static void _popup_set_prop();

static void ShellDepth(widget,closure,value)
    Widget widget;
    int closure;
    XrmValue *value;
{
   if (widget->core.parent == NULL) XtCopyDefaultDepth(widget,closure,value);
   else XtCopyFromParent (widget,closure,value);
}

static void ShellColormap(widget,closure,value)
    Widget widget;
    int closure;
    XrmValue *value;
{
   if (widget->core.parent == NULL)
	   XtCopyDefaultColormap(widget,closure,value);
   else XtCopyFromParent (widget,closure,value);
}

static void ShellAncestorSensitive(widget,closure,value)
    Widget widget;
    int closure;
    XrmValue *value;
{
   static Boolean true = True;
   if (widget->core.parent == NULL) value->addr = (XtPointer)(&true);
   else XtCopyFromParent (widget,closure,value);
}

/* ARGSUSED */
static void Initialize(req, new)
	Widget req, new;
{
	ShellWidget w = (ShellWidget) new;

	w->shell.popped_up = FALSE;
	w->shell.client_specified =
	    _XtShellNotReparented | _XtShellPositionValid;

	if (w->core.x == BIGSIZE) {
	    w->core.x = 0;
	    if (w->core.y == BIGSIZE) w->core.y = 0;
	} else {
	    if (w->core.y == BIGSIZE) w->core.y = 0;
	    else w->shell.client_specified |= _XtShellPPositionOK;
	}

	XtAddEventHandler(new, (EventMask) StructureNotifyMask,
		TRUE, EventHandler, (XtPointer) NULL);
}

/* ARGSUSED */
static void WMInitialize(req, new)
	Widget req,new;
{
	WMShellWidget w = (WMShellWidget) new;
	TopLevelShellWidget tls = (TopLevelShellWidget) new;	/* maybe */

	if(w->wm.title == NULL) {
	    if (XtIsTopLevelShell(new) &&
		    tls->topLevel.icon_name != NULL &&
		    strlen(tls->topLevel.icon_name) != 0) {
		w->wm.title = XtNewString(tls->topLevel.icon_name);
	    } else {
		w->wm.title = XtNewString(w->core.name);
	    }
	} else {
	    w->wm.title = XtNewString(w->wm.title);
	}
	w->wm.size_hints.flags = 0;
	w->wm.wm_hints.flags = 0;

	/* Find the values of the atoms, somewhere... */

	for (new = new->core.parent;
		new != NULL && !XtIsWMShell(new);
		new = new->core.parent) {}
	if (new == NULL) {
	    w->wm.wm_configure_denied =
		    XInternAtom(XtDisplay(w), "WM_CONFIGURE_DENIED", FALSE);
	    w->wm.wm_moved = XInternAtom(XtDisplay(w), "WM_MOVED", FALSE);
	} else {
	    w->wm.wm_configure_denied = WM_CONFIGURE_DENIED(new);
	    w->wm.wm_moved = WM_MOVED(new);
	}
}


/* ARGSUSED */
static void TopLevelInitialize(req, new)
	Widget req, new;
{
	TopLevelShellWidget w = (TopLevelShellWidget) new;

	if (w->topLevel.icon_name == NULL) {
	    w->topLevel.icon_name = XtNewString(w->core.name);
	} else {
	    w->topLevel.icon_name = XtNewString(w->topLevel.icon_name);
	}

	if (w->topLevel.iconic)
	    w->wm.wm_hints.initial_state = IconicState;
}

/* ARGSUSED */
static void ApplicationInitialize(req, new)
    Widget req, new;
{
    ApplicationShellWidget w = (ApplicationShellWidget)new;
    /* copy the argv if passed */
    if (w->application.argc > 0) {
	int i = w->application.argc;
	char **argv = (char**)XtMalloc( (unsigned)i*sizeof(char*) );
	char **argp = w->application.argv + i;
	while (--i >= 0) {
	    argv[i] = *--argp;
	}
	w->application.argv = argv;
    }
}

static void Resize(w)
    Widget w;
{
    register ShellWidget sw = (ShellWidget)w;    
    Widget childwid;
    int i;
    for(i = 0; i < sw->composite.num_children; i++) {
        if(sw->composite.children[i]->core.managed) {
             childwid = sw->composite.children[i];
             XtResizeWidget(childwid, sw->core.width, sw->core.height,
                           childwid->core.border_width);
        }
    }
}


static void Realize(wid, vmask, attr)
	Widget wid;
	Mask *vmask;
	XSetWindowAttributes *attr;
{
	ShellWidget w = (ShellWidget) wid;
        Mask mask = *vmask;

	if (w->core.background_pixmap == XtUnspecifiedPixmap) {
	    /* I attempt to inherit my child's background to avoid screen flash
	     * if there is latency between when I get resized and when my child
	     * is resized.  Background=None is not satisfactory, as I want the
	     * user to get immediate feedback on the new dimensions (most
	     * particularly in the case of a non-reparenting wm).  It is
	     * especially important to have the server clear any old cruft
	     * from the display when I am resized larger */

	    Boolean found_pixmap = False;
	    if (w->composite.num_children > 0) {
		Widget child;
		int i;
		for (i = 0; i < w->composite.num_children; i++) {
		    child = w->composite.children[i];
		    if (XtIsWidget(child) && XtIsManaged(child)) {
			if (child->core.background_pixmap
			    != XtUnspecifiedPixmap) {
			    mask &= ~(CWBackPixel);
			    mask |= CWBackPixmap;
			    w->core.background_pixmap =
				attr->background_pixmap =
				    child->core.background_pixmap;
			    found_pixmap = True;
			} else {
			    w->core.background_pixel = 
				child->core.background_pixel;
			}
			break;
		    }
		}
	    }
	    if (!found_pixmap) {
		mask |= CWBackPixel;
		mask &= ~(CWBackPixmap);
		attr->background_pixel = w->core.background_pixel;
	    }
	}
	else {
	    mask &= ~(CWBackPixel);
	    mask |= CWBackPixmap;
	    attr->background_pixmap = w->core.background_pixmap;
	}

	if(w->shell.save_under) {
		mask |= CWSaveUnder;
		attr->save_under = TRUE;
	}
	if(w->shell.override_redirect) {
		mask |= CWOverrideRedirect;
		attr->override_redirect = TRUE;
	}
	if (wid->core.width == 0 || wid->core.height == 0) {
	    Cardinal count = 1;
	    XtErrorMsg("invalidDimension", "shellRealize", "XtToolkitError",
		       "Shell widget %s has zero width and/or height",
		       &wid->core.name, &count);
	}
	wid->core.window = XCreateWindow(XtDisplay(wid),
		wid->core.screen->root, (int)wid->core.x, (int)wid->core.y,
		(unsigned int)wid->core.width, (unsigned int)wid->core.height,
		(unsigned int)wid->core.border_width, (int) wid->core.depth,
		(unsigned int) InputOutput, w->shell.visual,
		mask, attr);

	_popup_set_prop(w);
}


static void _SetTransientForHint(w, delete)
     TransientShellWidget w;
     Boolean delete;
{
    Window window_group;

    if (w->wm.transient) {
	if (w->transient.transient_for != NULL
	    && XtIsRealized(w->transient.transient_for))
	    window_group = XtWindow(w->transient.transient_for);
	else if ((window_group = w->wm.wm_hints.window_group)
		 == XtUnspecifiedWindowGroup) {
	    if (delete)
		XDeleteProperty( XtDisplay((Widget)w),
				 XtWindow((Widget)w),
				 XA_WM_TRANSIENT_FOR
				);
	    return;
	}

	XSetTransientForHint( XtDisplay((Widget)w),
			      XtWindow((Widget)w),
			      window_group
			     );
    }
}


static void TransientRealize(w, vmask, attr)
     Widget w;
     Mask *vmask;
     XSetWindowAttributes *attr;
{
    (*transientShellWidgetClass->core_class.superclass->
     core_class.realize) (w, vmask, attr);

    _SetTransientForHint((TransientShellWidget)w, False);
}


static void EvaluateWMHints(w)
    WMShellWidget w;
{
	XWMHints *hintp = &w->wm.wm_hints;

	hintp->flags = StateHint | InputHint;

	if (XtIsTopLevelShell((Widget)w)
	    && ((TopLevelShellWidget)w)->topLevel.iconic) {
	    hintp->initial_state = IconicState;
	}
	if (hintp->icon_x == XtUnspecifiedShellInt)
	    hintp->icon_x = -1;
	else
	    hintp->flags |= IconPositionHint;

	if (hintp->icon_y == XtUnspecifiedShellInt)
	    hintp->icon_y = -1;
	else
	    hintp->flags |= IconPositionHint;

	if (hintp->icon_pixmap != NULL) hintp->flags |= IconPixmapHint;
	if (hintp->icon_mask != NULL)   hintp->flags |= IconMaskHint;
	if (hintp->icon_window != NULL) hintp->flags |= IconWindowHint;

	if (hintp->window_group == XtUnspecifiedWindow) {
	    if(w->core.parent) {
		Widget p;
		for (p = w->core.parent; p->core.parent; p = p->core.parent);
		if (XtIsRealized(p)) {
		    hintp->window_group = XtWindow(p);
		    hintp->flags |=  WindowGroupHint;
		}
	    }
	} else if (hintp->window_group != XtUnspecifiedWindowGroup)
	    hintp->flags |=  WindowGroupHint;
}


static void EvaluateSizeHints(w)
    WMShellWidget w;
{
	struct _OldXSizeHints *sizep = &w->wm.size_hints;

	sizep->x = w->core.x;
	sizep->y = w->core.y;
	sizep->width = w->core.width;
	sizep->height = w->core.height;

	if (sizep->flags & USSize) {
	    if (sizep->flags & PSize) sizep->flags &= ~PSize;
	} else
	    sizep->flags |= PSize;

	if (sizep->flags & USPosition) {
	    if (sizep->flags & PPosition) sizep->flags &= ~PPosition;
	} else if (w->shell.client_specified & _XtShellPPositionOK)
	    sizep->flags |= PPosition;

	if (sizep->min_aspect.x != XtUnspecifiedShellInt
	    || sizep->min_aspect.y != XtUnspecifiedShellInt
	    || sizep->max_aspect.x != XtUnspecifiedShellInt
	    || sizep->max_aspect.y != XtUnspecifiedShellInt) {
	    sizep->flags |= PAspect;
	}
	if(w->wm.base_width != XtUnspecifiedShellInt
	   || w->wm.base_height != XtUnspecifiedShellInt) {
	    sizep->flags |= PBaseSize;
	    if (w->wm.base_width == XtUnspecifiedShellInt)
		w->wm.base_width = 0;
	    if (w->wm.base_height == XtUnspecifiedShellInt)
		w->wm.base_height = 0;
	}
	if (sizep->width_inc != XtUnspecifiedShellInt
	    || sizep->height_inc != XtUnspecifiedShellInt) {
	    if (sizep->width_inc < 1) sizep->width_inc = 1;
	    if (sizep->height_inc < 1) sizep->height_inc = 1;
	    sizep->flags |= PResizeInc;
	}
	if (sizep->max_width != XtUnspecifiedShellInt
	    || sizep->max_height != XtUnspecifiedShellInt) {
	    sizep->flags |= PMaxSize;
	    if (sizep->max_width == XtUnspecifiedShellInt)
		sizep->max_width = BIGSIZE;
	    if (sizep->max_height == XtUnspecifiedShellInt)
		sizep->max_height = BIGSIZE;
	}
	if(sizep->min_width != XtUnspecifiedShellInt
	   || sizep->min_height != XtUnspecifiedShellInt) {
	    sizep->flags |= PMinSize;
	    if (sizep->min_width == XtUnspecifiedShellInt)
		sizep->min_width = 1;
	    if (sizep->min_height == XtUnspecifiedShellInt)
		sizep->min_height = 1;
	}
}

static void _popup_set_prop(w)
	ShellWidget w;
{
	Widget p;
	WMShellWidget wmshell = (WMShellWidget) w;
	TopLevelShellWidget tlshell = (TopLevelShellWidget) w;
	ApplicationShellWidget appshell = (ApplicationShellWidget) w;
	XTextProperty icon_name;
	XTextProperty window_name;
	char **argv;
	int argc;
	XSizeHints *size_hints;
	Window window_group;
	XClassHint classhint;

	if (!XtIsWMShell((Widget)w) || w->shell.override_redirect) return;

	if ((size_hints = XAllocSizeHints()) == NULL)
	    _XtAllocError("XAllocSizeHints");

	window_name.value = (unsigned char*)wmshell->wm.title;
	window_name.encoding = wmshell->wm.title_encoding;
	window_name.format = 8;
	window_name.nitems = strlen(window_name.value) + 1;

	if (XtIsTopLevelShell((Widget)w)) {
	    icon_name.value = (unsigned char*)tlshell->topLevel.icon_name;
	    icon_name.encoding = tlshell->topLevel.icon_name_encoding;
	    icon_name.format = 8;
	    icon_name.nitems = strlen(icon_name.value) + 1;
	}

	EvaluateWMHints(wmshell);
	EvaluateSizeHints(wmshell);
	ComputeWMSizeHints(wmshell, size_hints);

	if (wmshell->wm.transient
	    && !XtIsTransientShell((Widget)w)
	    && (window_group = wmshell->wm.wm_hints.window_group)
	       != XtUnspecifiedWindowGroup) {

	    XSetTransientForHint(XtDisplay((Widget)w),
				 XtWindow((Widget)w),
				 window_group
				 );
	}

	classhint.res_name = w->core.name;
	/* For the class, look up to the top of the tree */
	for (p = (Widget)w; p->core.parent != NULL; p = p->core.parent);
	if (XtIsApplicationShell(p)) {
	    classhint.res_class =
		((ApplicationShellWidget)p)->application.class;
	} else classhint.res_class = XtClass(p)->core_class.class_name;

	if (XtIsApplicationShell((Widget)w)
	    && (argc = appshell->application.argc) != -1)
	    argv = (char**)appshell->application.argv;
	else {
	    argv = NULL;
	    argc = 0;
	}

	XSetWMProperties(XtDisplay((Widget)w), XtWindow((Widget)w),
			 &window_name,
			 (XtIsTopLevelShell((Widget)w)) ? &icon_name : NULL,
			 argv, argc,
			 size_hints,
			 &wmshell->wm.wm_hints,
			 &classhint);
	XFree((char*)size_hints);
}

/* ARGSUSED */
static void EventHandler(wid, closure, event)
	Widget wid;
	XtPointer closure;
	XEvent *event;
{
	register ShellWidget w = (ShellWidget) wid;
	WMShellWidget wmshell = (WMShellWidget) w;
	Boolean  sizechanged = FALSE;
	unsigned int width, height, border_width, tmpdepth;
	int tmpx, tmpy, tmp2x, tmp2y;
	Window tmproot, tmpchild;

	if(w->core.window != event->xany.window) {
		XtAppErrorMsg(XtWidgetToApplicationContext(wid),
			"invalidWindow","eventHandler","XtToolkitError",
                        "Event with wrong window",
			(String *)NULL, (Cardinal *)NULL);
		return;
	}

	switch(event->type) {
	    case ConfigureNotify:
#define NEQ(x)	( w->core.x != event->xconfigure.x )
		if( NEQ(width) || NEQ(height) || NEQ(border_width) ) {
			sizechanged = TRUE;
		}
#undef NEQ
		w->core.width = event->xconfigure.width;
		w->core.height = event->xconfigure.height;
		w->core.border_width = event->xconfigure.border_width;
		if (event->xany.send_event /* ICCCM compliant synthetic ev */
		    /* || w->shell.override_redirect */
		    || w->shell.client_specified & _XtShellNotReparented)
	        {
		    w->core.x = event->xconfigure.x;
		    w->core.y = event->xconfigure.y;
		    w->shell.client_specified |= _XtShellPositionValid;
		}
		else w->shell.client_specified &= ~_XtShellPositionValid;
		if (XtIsWMShell(wid) && !wmshell->wm.wait_for_wm) {
		    /* Consider trusting the wm again */
		    register struct _OldXSizeHints *hintp
			= &wmshell->wm.size_hints;
#define EQ(x) (hintp->x == w->core.x)
		    if (EQ(x) && EQ(y) && EQ(width) && EQ(height)) {
			wmshell->wm.wait_for_wm = TRUE;
		    }
#undef EQ
		}		    
		break;

	    case ClientMessage:
		if( event->xclient.message_type == WM_CONFIGURE_DENIED(wid)
		    && XtIsWMShell(wid)) {

		    /* 
		     * UT Oh! the window manager has come back alive
		     * This means either I didn't wait long enough or
		     * The WM is sick.
		     * Query my real size and position, and adjust my child
		     * it needs be.
		     */

		    if(wmshell->wm.wait_for_wm) {
			XtAppWarningMsg(XtWidgetToApplicationContext(wid),
				"communicationError","windowManager",
                                  "XtToolkitError",
                                  "Window Manager is confused",
				  (String *)NULL, (Cardinal *)NULL);
		    }
		    wmshell->wm.wait_for_wm = TRUE;
		    (void) XGetGeometry(XtDisplay(w), XtWindow(w), &tmproot,
			    &tmpx, &tmpy, &width, &height, &border_width,
			    &tmpdepth);
		    (void) XTranslateCoordinates(XtDisplay(w), XtWindow(w), 
			    tmproot, (int) tmpx, (int) tmpy,
			    &tmp2x, &tmp2y, &tmpchild);
		    w->core.x = tmp2x;
		    w->core.y = tmp2y;
		    if( width != w->core.width || height != w->core.height
		       || border_width != w->core.border_width ) {
			    w->core.width = width;
			    w->core.height = height;
			    w->core.border_width = border_width;
			    sizechanged = TRUE;
		    }

		    break;
		}
		if(event->xclient.message_type == WM_MOVED(wid)) {
		    w->core.x = event->xclient.data.s[0];
		    w->core.y  = event->xclient.data.s[1];
		    if (XtIsWMShell((Widget)w)) {
			WMShellWidget wmshell = (WMShellWidget) w;
			/* Any window manager which sends this must be 
			   good guy.  Let's reset our flag. */
			wmshell->wm.wait_for_wm = TRUE;
		    }
		}
		break;

	      case ReparentNotify:
		if (event->xreparent.window == XtWindow(w)) {
		   if (event->xreparent.parent != RootWindowOfScreen(XtScreen(w)))
		       w->shell.client_specified &= ~_XtShellNotReparented;
		   else
		       w->shell.client_specified |= _XtShellNotReparented;
		   w->shell.client_specified &= ~_XtShellPositionValid;
	        }
		return;

	      default:
		 return;
	 } 

	 if (sizechanged && 
                 XtClass(w)->core_class.resize != (XtWidgetProc) NULL)
                    (*(XtClass(w)->core_class.resize))(w);

}

static void WMDestroy(wid)
	Widget wid;
{
	WMShellWidget w = (WMShellWidget) wid;

	XtFree((char *) w->wm.title);
}

static void TopLevelDestroy(wid)
	Widget wid;
{
	TopLevelShellWidget w = (TopLevelShellWidget) wid;

	XtFree((char *) w->topLevel.icon_name);
}

static void ApplicationDestroy(wid)
	Widget wid;
{
	ApplicationShellWidget w = (ApplicationShellWidget) wid;

	if(w->application.argv != NULL) XtFree((char *) w->application.argv);
	w->application.argv = NULL;
}

/*
 * There is some real ugliness here.  If I have a width and a height which are
 * zero, and as such suspect, and I have not yet been realized then I will 
 * grow to match my child.
 *
 */
static void ChangeManaged(wid)
    Widget wid;
{
    register ShellWidget w = (ShellWidget) wid;
    Widget childwid = NULL;
    int i;

    for (i = 0; i < w->composite.num_children; i++) {
	if (XtIsManaged(w->composite.children[i])) {
	    childwid = w->composite.children[i];
	    break;
	}
    }

    if (!XtIsRealized (wid)) {
	Boolean is_wmshell = XtIsWMShell(wid);
	int x, y, width, height, win_gravity = -1, flag;
	struct _OldXSizeHints hints, *hintsP;

	if (w->core.width == 0 && w->core.height == 0 && childwid != NULL) {
	    /* we inherit our child's attributes */
	    w->core.width = childwid->core.width;
	    w->core.height = childwid->core.height;
	    if (is_wmshell)
		((WMShellWidget)wid)->wm.size_hints.flags |= PSize;
	}
	if(w->shell.geometry != NULL) {
	    char def_geom[64];
	    x = w->core.x;
	    y = w->core.y;
	    width = w->core.width;
	    height = w->core.height;
	    if (is_wmshell) {
		EvaluateSizeHints((WMShellWidget)w);
		hintsP = &((WMShellWidget)w)->wm.size_hints;
		if (hintsP->flags & PBaseSize) {
		    width -= ((WMShellWidget)w)->wm.base_width;
		    height -= ((WMShellWidget)w)->wm.base_height;
		}
		else if (hintsP->flags & PMinSize) {
		    width -= hintsP->min_width;
		    height -= hintsP->min_height;
		}
		if (hintsP->flags & PResizeInc) {
		    width /= hintsP->width_inc;
		    height /= hintsP->height_inc;
		}
	    }
	    else {
		hintsP = &hints;
		hints.flags = 0;
	    }
	    sprintf( def_geom, "%dx%d+%d+%d", width, height, x, y );
	    flag = XWMGeometry( XtDisplay(wid),
			        XScreenNumberOfScreen(XtScreen(wid)),
			        w->shell.geometry, def_geom,
			        (unsigned int)w->core.border_width,
			        hintsP, &x, &y, &width, &height,
			        &win_gravity
			       );
	    if (flag & XValue) w->core.x = (Position)x;
	    if (flag & YValue) w->core.y = (Position)y;
	    if (flag & WidthValue) w->core.width = (Dimension)width;
	    if (flag & HeightValue) w->core.height = (Dimension)height;
	}
	else
	    flag = 0;

	if (is_wmshell) {
	    WMShellWidget wmshell = (WMShellWidget) w;
	    if (wmshell->wm.win_gravity == XtUnspecifiedShellInt) {
		if (win_gravity != -1)
		    wmshell->wm.win_gravity = win_gravity;
		else
		    wmshell->wm.win_gravity = NorthWestGravity;
	    }
	    wmshell->wm.size_hints.flags |= PWinGravity;
	    if ((flag & (XValue|YValue)) == (XValue|YValue))
		wmshell->wm.size_hints.flags |= USPosition;
	    if ((flag & (WidthValue|HeightValue)) == (WidthValue|HeightValue))
		wmshell->wm.size_hints.flags |= USSize;
	}
    }

    if (childwid != NULL) {
	XtConfigureWidget (childwid, (Position)0, (Position)0,
			   w->core.width, w->core.height, (Dimension)0 );
	XtSetKeyboardFocus(wid, childwid);
    }
}

/*
 * This is gross, I can't wait to see if the change happened so I will ask
 * the window manager to change my size and do the appropriate X work.
 * I will then tell the requester that he can.  Care must be taken because
 * it is possible that some time in the future the request will be
 * asynchronusly denied and the window reverted to it's old size/shape.
 */
 
static XtGeometryResult GeometryManager( wid, request, reply )
	Widget wid;
	XtWidgetGeometry *request;
	XtWidgetGeometry *reply;
{
	ShellWidget shell = (ShellWidget)(wid->core.parent);
	XtWidgetGeometry my_request;

	if(shell->shell.allow_shell_resize == FALSE && XtIsRealized(wid))
		return(XtGeometryNo);

	if(!XtIsRealized((Widget)shell)){
		if (request->request_mode & (CWX | CWY)) {
			return(XtGeometryNo);
		}
		*reply = *request;
		if(request->request_mode & CWWidth)
		   wid->core.width = shell->core.width = request->width;
		if(request->request_mode & CWHeight) 
		   wid->core.height = shell->core.height = request->height;
		if(request->request_mode & CWBorderWidth)
		   wid->core.border_width = shell->core.border_width =
		   	request->border_width;
		return(XtGeometryYes);
	}

	/* %%% worry about XtCWQueryOnly */
	my_request.request_mode = 0;
	if (request->request_mode & CWWidth) {
	    my_request.width = request->width;
	    my_request.request_mode |= CWWidth;
	}
	if (request->request_mode & CWHeight) {
	    my_request.height = request->height;
	    my_request.request_mode |= CWHeight;
	}
	if (request->request_mode & CWBorderWidth) {
	    my_request.border_width = request->border_width;
	    my_request.request_mode |= CWBorderWidth;
	}
	if (XtMakeGeometryRequest((Widget)shell, &my_request, NULL)
		== XtGeometryYes) {
	    if (request->request_mode & CWWidth) {
		wid->core.width = request->width;
	    }
	    if (request->request_mode & CWHeight) {
		wid->core.height = request->height;
	    }
	    if (request->request_mode & CWBorderWidth) {
		wid->core.x = wid->core.y = -request->border_width;
	    }
	    return XtGeometryYes;
	} else return XtGeometryNo;
}

typedef struct {
	Widget w;
	XWindowChanges *values;
	int others;
} QueryStruct;

static Bool isMine(dpy, event, arg)
	Display *dpy;
	register XEvent  *event;
	char *arg;
{
	QueryStruct *q = (QueryStruct *) arg;
	Widget w = q->w;
	
	if ( (dpy != XtDisplay(w)) || (event->xany.window != XtWindow(w)) ) {
	    return FALSE;
	}
	if (event->type == ConfigureNotify) {
	    XConfigureEvent *ce = (XConfigureEvent *) event;
	    if (ce->width == q->values->width && 
		    ce->height == q->values->height) {
		return TRUE;
	    } else {
		q->others++;
		return FALSE;
	    }
	}
	if (event->type == ClientMessage &&
		(event->xclient.message_type == WM_CONFIGURE_DENIED(w) ||
		 event->xclient.message_type == WM_MOVED(w))) {
	    return TRUE;
	}
	return FALSE;
}

static Bool findOthers(dpy, event, arg)
	Display *dpy;
	register XEvent  *event;
	char *arg;
{
	QueryStruct *q = (QueryStruct *) arg;
	Widget w = q->w;
	
	if ( (dpy != XtDisplay(w)) || (event->xany.window != XtWindow(w)) ) {
	    return FALSE;
	}
	if (event->type == ConfigureNotify) {
	    XConfigureEvent *ce = (XConfigureEvent *) event;
	    if (ce->width != q->values->width || 
		    ce->height != q->values->height) {
		return TRUE;
	    } 
	}
	return FALSE;
}

static _wait_for_response(w, values, event)
	WMShellWidget     w;
	XWindowChanges *values;
	XEvent		*event;
{
	XtAppContext app = XtWidgetToApplicationContext((Widget) w);
	QueryStruct q;
	XEvent junkevent;
	unsigned long timeout = w->wm.wm_timeout;

	XFlush(XtDisplay(w));
	q.w = (Widget) w;
	q.values = values;
	for(;;) {
	    q.others = 0;
	    if (XCheckIfEvent(XtDisplay(w), event, isMine, (char *) &q)) {
		/* The event we want is there; but maybe others too.  If so,
		   get them out of the queue */
		if (event->xany.type != ConfigureNotify) return TRUE;
		for (; q.others > 0; q.others--) {
		    if (!XCheckIfEvent(XtDisplay(w), &junkevent,
			    findOthers, (char *) &q)) {
			XtAppErrorMsg(XtWidgetToApplicationContext((Widget)w),
				"missingEvent","shell","XtToolkitError",
				"Events are disappearing from under Shell",
				(String *)NULL, (Cardinal *)NULL);
		    }
		}
		return TRUE;
	    } else {
		if (_XtwaitForSomething(TRUE, TRUE, FALSE, TRUE, &timeout,
			app) != -1) continue;
		if (timeout == 0)
		  return FALSE;
	    }
	}
}

/*ARGSUSED*/
static XtGeometryResult RootGeometryManager(w, request, reply)
    Widget w;
    XtWidgetGeometry *request, *reply;
{
    XWindowChanges values;
    unsigned int mask = request->request_mode;
    WMShellWidget wmshell = (WMShellWidget)w;
    XEvent event;
    Boolean wm = XtIsWMShell(w);
    register struct _OldXSizeHints *hintp;
    int oldx, oldy;

    if (wm) {
	hintp = &wmshell->wm.size_hints;
	oldx = hintp->x = w->core.x;
	oldy = hintp->y = w->core.y;
	hintp->width = w->core.width;
	hintp->height = w->core.height;
    }

    if (mask & CWX) {
	    if (w->core.x == request->x) mask &= ~CWX;
	    else if (wm) {
		    hintp->flags &= ~USPosition;
		    hintp->flags |= PPosition;
		    w->core.x = hintp->x = values.x = request->x;
	    } else w->core.x = values.x = request->x;
    }
    if (mask & CWY) {
	    if (w->core.y == request->y) mask &= ~CWY;
	    else if (wm) {
		    hintp->flags &= ~USPosition;
		    hintp->flags |= PPosition;
		    w->core.y = hintp->y = values.y = request->y;
	    } else w->core.y = values.y = request->y;
    }
    if (mask & CWBorderWidth) {
	    if (w->core.border_width == request->border_width) {
		    mask &= ~CWBorderWidth;
	    } else w->core.border_width = values.border_width
				= request->border_width;
    }
    if (mask & CWWidth) {
	    values.width = request->width;
	    if (w->core.width == values.width) mask &= ~CWWidth;
	    else if (wm) {
		    hintp->flags &= ~USSize;
		    hintp->flags |= PSize;
		    hintp->width = values.width;
	    } else w->core.width = values.width;
    } else values.width = w->core.width; /* for _wait_for_response */
    if (mask & CWHeight) {
	    values.height = request->height;
	    if (w->core.height == values.height) mask &= ~CWHeight;
	    else if (wm) {
		    hintp->flags &= ~USSize;
		    hintp->flags |= PSize;
		    hintp->height = values.height;
	    } else w->core.height = values.height;
    } else values.height = w->core.height; /* for _wait_for_response */
    if (mask & CWStackMode) {
	values.stack_mode = request->stack_mode;
	if (mask & CWSibling)
	    values.sibling = XtWindow(request->sibling);
    }

    if (!XtIsRealized(w)) return XtGeometryDone;

    if (wm && !wmshell->shell.override_redirect
	&& mask & (CWX | CWY | CWWidth | CWHeight | CWBorderWidth)) {
	_SetWMSizeHints(wmshell);
    }

    XConfigureWindow(XtDisplay(w), XtWindow(w), mask, &values);

    if (wmshell->shell.override_redirect) return XtGeometryDone;

    /* If no non-stacking bits are set, there's no way to tell whether
       or not this worked, so assume it did */

    if (!(mask & ~(CWStackMode | CWSibling))) return XtGeometryDone;

    if (wmshell->wm.wait_for_wm == FALSE) {
	    /* the window manager is sick
	     * so I will do the work and 
	     * say no so if a new WM starts up,
	     * or the current one recovers
	     * my size requests will be visable
	     */
	    return XtGeometryNo;
    }

    if (_wait_for_response(wmshell, &values, &event)){
	/* got an event */
	if (event.type == ConfigureNotify) {
	    w->core.width = event.xconfigure.width;
	    w->core.height = event.xconfigure.height;
	    w->core.border_width = event.xconfigure.border_width;
	    if (event.xany.send_event /* ICCCM compliant synth */
		|| wmshell->shell.client_specified & _XtShellNotReparented)
	    {
		w->core.x = event.xconfigure.x;
		w->core.y = event.xconfigure.y;
		wmshell->shell.client_specified |= _XtShellPositionValid;
	    }
	    else wmshell->shell.client_specified &= ~_XtShellPositionValid;
	    return XtGeometryDone;
	} else if (event.type == ClientMessage &&
		   event.xclient.message_type == WM_CONFIGURE_DENIED(w)) {
	    w->core.x = oldx;
	    w->core.y = oldy;
	    return XtGeometryNo;
	} else if (event.type == ClientMessage &&
		    event.xclient.message_type == WM_MOVED(w)) {
	    w->core.x = event.xclient.data.s[0];
	    w->core.y = event.xclient.data.s[1];
	    wmshell->shell.client_specified |= _XtShellPositionValid;
	    return XtGeometryDone;
	} else XtAppErrorMsg(XtWidgetToApplicationContext((Widget)w),
			     "internalError", "shell", "XtToolkitError",
			     "Shell's window manager interaction is broken",
			     (String *)NULL, (Cardinal *)NULL);
    } else {
	wmshell->wm.wait_for_wm = FALSE;
	return XtGeometryNo;
    }
    return XtGeometryNo;
}

/* ARGSUSED */
static Boolean SetValues(old, ref, new, args, num_args)
	Widget old, ref, new;
	ArgList args;		/* unused */
	Cardinal *num_args;	/* unused */
{
	ShellWidget nw = (ShellWidget) new;
	ShellWidget ow = (ShellWidget) old;
	Mask mask = 0;
	XSetWindowAttributes attr;

	if (ow->shell.save_under != nw->shell.save_under) {
	    mask = CWSaveUnder;
	    attr.save_under = nw->shell.save_under;
	}

	if (ow->shell.override_redirect != nw->shell.override_redirect) {
	    mask |= CWOverrideRedirect;
	    attr.override_redirect = nw->shell.override_redirect;
	}

	if (mask && XtIsRealized(new)) {
	    XChangeWindowAttributes(XtDisplay(new),XtWindow(new), mask, &attr);
	    if ((mask & CWOverrideRedirect) && !nw->shell.override_redirect)
		_popup_set_prop(nw);
	}
	return FALSE;
}

/* ARGSUSED */
static Boolean WMSetValues(old, ref, new, args, num_args)
	Widget old, ref, new;
	ArgList args;		/* unused */
	Cardinal *num_args;	/* unused */
{
	WMShellWidget nwmshell = (WMShellWidget) new;
	WMShellWidget owmshell = (WMShellWidget) old;
	Boolean set_prop
	    = XtIsRealized(new) && !nwmshell->shell.override_redirect;
	Boolean title_changed;

	EvaluateSizeHints(nwmshell);

#define NEQ(f) (nwmshell->wm.size_hints.f != owmshell->wm.size_hints.f)

	if (set_prop
	    && (NEQ(flags) || NEQ(min_width) || NEQ(min_height)
		|| NEQ(max_width) || NEQ(max_height)
		|| NEQ(width_inc) || NEQ(height_inc)
		|| NEQ(min_aspect.x) || NEQ(min_aspect.y)
		|| NEQ(max_aspect.x) || NEQ(max_aspect.y)
#undef NEQ
#define NEQ(f) (nwmshell->wm.f != owmshell->wm.f)

		|| NEQ(base_width) || NEQ(base_height) || NEQ(win_gravity))) {
	    _SetWMSizeHints(nwmshell);
	}
#undef NEQ

	if (nwmshell->wm.title != owmshell->wm.title) {
	    XtFree(owmshell->wm.title);
	    nwmshell->wm.title = XtNewString(nwmshell->wm.title);
	    title_changed = True;
	} else
	    title_changed = False;

	if (set_prop
	    && (title_changed ||
		nwmshell->wm.title_encoding != owmshell->wm.title_encoding)) {

	    XTextProperty title;
	    title.value = (unsigned char*)nwmshell->wm.title;
	    title.encoding = nwmshell->wm.title_encoding;
	    title.format = 8;
	    title.nitems = strlen(nwmshell->wm.title) + 1;
	    XSetWMName(XtDisplay(new), XtWindow(new), &title);
	}

	EvaluateWMHints(nwmshell);

#define NEQ(f)	(nwmshell->wm.wm_hints.f != owmshell->wm.wm_hints.f)

	if (set_prop
	    && (NEQ(flags) || NEQ(input) || NEQ(initial_state)
		|| NEQ(icon_x) || NEQ(icon_y)
		|| NEQ(icon_pixmap) || NEQ(icon_mask) || NEQ(icon_window)
		|| NEQ(window_group))) {

	    XSetWMHints(XtDisplay(new), XtWindow(new), &nwmshell->wm.wm_hints);
	}
#undef NEQ

	return FALSE;
}

/*ARGSUSED*/
static Boolean TransientSetValues(oldW, refW, newW, args, num_args)
     Widget oldW, refW, newW;
     ArgList args;		/* unused */
     Cardinal *num_args;	/* unused */
{
    TransientShellWidget old = (TransientShellWidget)oldW;
    TransientShellWidget new = (TransientShellWidget)newW;
    
    if (XtIsRealized(newW)
	&& ((new->transient.transient_for != old->transient.transient_for)
	    || (new->transient.transient_for == NULL
		&& (new->wm.wm_hints.window_group
		    != old->wm.wm_hints.window_group)))) {

	_SetTransientForHint(new, True);
    }
    return False;
}


/* ARGSUSED */
static Boolean TopLevelSetValues(oldW, refW, newW, args, num_args)
     Widget oldW, refW, newW;
     ArgList args;		/* unused */
     Cardinal *num_args;	/* unused */
{
    TopLevelShellWidget old = (TopLevelShellWidget)oldW;
    TopLevelShellWidget new = (TopLevelShellWidget)newW;
    Boolean name_changed;

    if (old->topLevel.icon_name != new->topLevel.icon_name) {
	XtFree((XtPointer)old->topLevel.icon_name);
	new->topLevel.icon_name = XtNewString(
	    new->topLevel.icon_name);
	name_changed = True;
    } else
	name_changed = False;

    if (XtIsRealized(newW) && !new->shell.override_redirect) {
	if (new->topLevel.iconic != old->topLevel.iconic) {
	    if (new->topLevel.iconic)
		XIconifyWindow(XtDisplay(newW),
			       XtWindow(newW),
			       XScreenNumberOfScreen(XtScreen(newW))
			       );
	    else
		XtPopup(newW, XtGrabNone);
	}

	if (name_changed ||
	    (old->topLevel.icon_name_encoding
	     != new->topLevel.icon_name_encoding)) {

	    XTextProperty icon_name;
	    icon_name.value = (unsigned char *)new->topLevel.icon_name;
	    icon_name.encoding = new->topLevel.icon_name_encoding;
	    icon_name.format = 8;
	    icon_name.nitems = strlen(icon_name.value) + 1;
	    XSetWMIconName(XtDisplay(newW), XtWindow(newW), &icon_name);
	}
    }
    return False;
}


void _XtShellGetCoordinates( widget, x, y)
    Widget widget;
    Position* x;
    Position* y;
{
    ShellWidget w = (ShellWidget)widget;
    if (!(w->shell.client_specified & _XtShellPositionValid)) {
	int tmpx, tmpy;
	Window tmpchild;
	(void) XTranslateCoordinates(XtDisplay(w), XtWindow(w), 
				     RootWindowOfScreen(XtScreen(w)),
				     (int) -w->core.border_width,
				     (int) -w->core.border_width,
				     &tmpx, &tmpy, &tmpchild);
	w->core.x = tmpx;
	w->core.y = tmpy;
	w->shell.client_specified |= _XtShellPositionValid;
    }
    *x = w->core.x;
    *y = w->core.y;
}
