#ifndef lint
static char Xrcsid[] = "$XConsortium: Shell.c,v 1.58 89/09/12 16:49:33 swick Exp $";
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

extern void XSetWMNormalHints(); /* this was not declared in Xlib.h... */
extern void XSetTransientForHint(); /* this was not declared in Xlib.h... */
extern void XSetClassHint(); /* this was not declared in Xlib.h... */

#include "IntrinsicI.h"
#include "StringDefs.h"
#include "Shell.h"
#include "ShellP.h"
#include "Vendor.h"
#include "VendorP.h"

#define WM_CONFIGURE_DENIED(w) (((WMShellWidget) (w))->wm.wm_configure_denied)
#define WM_MOVED(w) (((WMShellWidget) (w))->wm.wm_moved)

#define BIGSIZE ((Dimension)32123)

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

#define Offset(x)	(XtOffset(ShellWidget, x))
static XtResource shellResources[]=
{
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
	    XtRImmediate, (XtPointer)False}
};

static void Initialize();
static void Realize();
static void Resize();
static Boolean SetValues();
static void ChangeManaged(); /* XXX */
static XtGeometryResult GeometryManager();

externaldef(shellclassrec) ShellClassRec shellClassRec = {
  {
    /* superclass         */    (WidgetClass) &compositeClassRec,
    /* class_name         */    "Shell",
    /* size               */    sizeof(ShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_notify    */	NULL,		
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    shellResources,
    /* resource_count     */	XtNumber(shellResources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    NULL,
    /* set_values         */    SetValues,
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
    /* geometry_manager   */    GeometryManager,
    /* change_managed     */    ChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	    */  NULL
  },{
    /* extension	    */  NULL
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
#define Offset(x)	(XtOffset(WMShellWidget, x))

static XtResource wmResources[]=
{
	{ XtNtitle, XtCTitle, XtRString, sizeof(char *),
	    Offset(wm.title), XtRString, NULL},
	{ XtNwmTimeout, XtCWmTimeout, XtRInt, sizeof(int),
	    Offset(wm.wm_timeout), XtRImmediate, (XtPointer)5000},
	{ XtNwaitForWm, XtCWaitForWm, XtRBoolean, sizeof(Boolean),
	    Offset(wm.wait_for_wm), XtRImmediate, (XtPointer)True},
	{ XtNtransient, XtCTransient, XtRBoolean, sizeof(Boolean),
	    Offset(wm.transient), XtRImmediate, (XtPointer)False},
/* size_hints minus things stored in core */
	{ XtNbaseWidth, XtCBaseWidth, XtRInt, sizeof(int),
	    Offset(wm.size_hints.base_width),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNbaseHeight, XtCBaseHeight, XtRInt, sizeof(int),
	    Offset(wm.size_hints.base_height),
	    XtRImmediate, (XtPointer)XtUnspecifiedShellInt},
	{ XtNwinGravity, XtCWinGravity, XtRInt, sizeof(int),
	    Offset(wm.size_hints.win_gravity),
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
	{ XtNinitialState, XtCInitialState, XtRInt, sizeof(int),
	    Offset(wm.wm_hints.initial_state), XtRImmediate, (XtPointer)1},
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

static XtResource transientResources[]=
{
	{ XtNtransient, XtCTransient, XtRBoolean, sizeof(Boolean),
	    Offset(wm.transient), XtRImmediate, (XtPointer)True},
	{ XtNsaveUnder, XtCSaveUnder, XtRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XtRImmediate, (XtPointer)True},
};

externaldef(transientshellclassrec) TransientShellClassRec transientShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &vendorShellClassRec,
    /* class_name         */    "TransientShell",
    /* size               */    sizeof(TransientShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    NULL,
    /* initialize_notify    */	NULL,		
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    transientResources,
    /* resource_count     */	XtNumber(transientResources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/	FALSE,
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

externaldef(transientshellwidgetclass) WidgetClass transientShellWidgetClass =
	(WidgetClass) (&transientShellClassRec);

/***************************************************************************
 *
 * TopLevelShell class record
 *
 ***************************************************************************/

#undef Offset
#define Offset(x)	(XtOffset(TopLevelShellWidget, x))

static XtResource topLevelResources[]=
{
	{ XtNiconName, XtCIconName, XtRString, sizeof(XtPointer),
	    Offset(topLevel.icon_name), XtRString, (XtPointer) NULL},
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
#define Offset(x)	(XtOffset(ApplicationShellWidget, x))

static XtResource applicationResources[]=
{
	{ XtNargc, XtCArgc, XtRInt, sizeof(int),
	    Offset(application.argc), XtRImmediate, (XtPointer)0}, 
	{ XtNargv, XtCArgv, XtRPointer, sizeof(XtPointer),
	    Offset(application.argv), XtRPointer, (XtPointer) NULL}
};

static void ApplicationDestroy();

externaldef(applicationshellclassrec) ApplicationShellClassRec applicationShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &topLevelShellClassRec,
    /* class_name         */    "ApplicationShell",
    /* size               */    sizeof(ApplicationShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    NULL,
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

static void EventHandler();
static void _popup_set_prop();
static Boolean _ask_wm_for_size();
static void _do_setsave_under();

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
	    if (XtIsSubclass(new, topLevelShellWidgetClass) &&
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
		new != NULL && !XtIsSubclass(new, wmShellWidgetClass);
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

	if(w->topLevel.icon_name == NULL) {
	    w->topLevel.icon_name = XtNewString(w->core.name);
	} else {
	    w->topLevel.icon_name = XtNewString(w->topLevel.icon_name);
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

static void SetWindowGroups(widget, window)
	Widget widget;
	Window window;
{
	int i;
	Arg a;
	Widget pop;

	XtSetArg(a, XtNwindowGroup, window);

	for (i = 0; i < widget->core.num_popups; i++) {
	    pop = widget->core.popup_list[i];
	    if (pop->core.num_popups > 0) SetWindowGroups(pop, window);
	    if (XtIsSubclass(pop, wmShellWidgetClass) &&
		    ((WMShellWidget) pop)->wm.wm_hints.window_group
		    == XtUnspecifiedWindow) {
		XtSetValues(pop, &a, (Cardinal)1);
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
	     * user to get immediate feedback on the new dimensions.  It is
	     * especially important to have the server clear any old cruft
	     * from the display when I am resized larger */

	    if (w->composite.num_children > 0) {
		Widget child;
		int i;
		for (i = 0; i < w->composite.num_children; i++) {
		    child = w->composite.children[i];
		    if (XtIsManaged(child)) /* macro uses args twice! */
			break;
		}
		if (child->core.background_pixmap != XtUnspecifiedPixmap) {
		    mask &= ~(CWBackPixel);
		    mask |= CWBackPixmap;
		    attr->background_pixmap = child->core.background_pixmap;
		} else {
		    attr->background_pixel = child->core.background_pixel;
		}
	    }
	    else {
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
		(unsigned int) InputOutput, (Visual *) CopyFromParent,
		mask, attr);

	if (wid->core.num_popups > 0 && w->core.parent == NULL) {
	    SetWindowGroups(wid, wid->core.window);
	}

	_popup_set_prop(w);
}

/* Why isn't this in the library? */

static void SetHostName(dpy, w, name)
	register Display *dpy;
	Window w;
	char *name;
{
	XChangeProperty(dpy, w, XA_WM_CLIENT_MACHINE, XA_STRING, 
		8, PropModeReplace, (unsigned char *)name, strlen(name));
}

static void EvaluateSizeHints(w)
    WMShellWidget w;
{
	XSizeHints *sizep = &w->wm.size_hints;

	sizep->x = w->core.x;
	sizep->y = w->core.y;
	sizep->width = w->core.width;
	sizep->height = w->core.height;
	if (!(sizep->flags & USSize)) sizep->flags |= PSize;
	if (!(sizep->flags & USPosition)) sizep->flags |= PPosition;

	if (sizep->min_aspect.x != XtUnspecifiedShellInt
	    || sizep->min_aspect.y != XtUnspecifiedShellInt
	    || sizep->max_aspect.x != XtUnspecifiedShellInt
	    || sizep->max_aspect.y != XtUnspecifiedShellInt) {
	    sizep->flags |= PAspect;
	}
	if(sizep->base_width != XtUnspecifiedShellInt
	   || sizep->base_height != XtUnspecifiedShellInt) {
	    sizep->flags |= PBaseSize;
	    if (sizep->base_width == XtUnspecifiedShellInt)
		sizep->base_width = 0;
	    if (sizep->base_height == XtUnspecifiedShellInt)
		sizep->base_height = 0;
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
	Window win;
	Display *dpy = XtDisplay(w);
	Widget	ptr;
	XClassHint classhint;
	WMShellWidget wmshell = (WMShellWidget) w;
	TopLevelShellWidget tlshell = (TopLevelShellWidget) w;
	ApplicationShellWidget appshell = (ApplicationShellWidget) w;
	register XWMHints *hintp;
	static char *hostname;
	static Boolean gothost = FALSE;
	Widget wid;

	win = XtWindow(w);

	if (!gothost) {
	    char hostbuf[1000];
	    (void) _XtGetHostname (hostbuf, sizeof hostbuf);
	    hostname = XtNewString(hostbuf);
	    gothost = TRUE;
	}

	if (XtIsSubclass((Widget)w, wmShellWidgetClass) &&
		!w->shell.override_redirect) {
	    XStoreName(dpy, win, wmshell->wm.title);

	    if (XtIsSubclass((Widget)w, topLevelShellWidgetClass)) {
		XSetIconName(dpy, win, tlshell->topLevel.icon_name);

	    }

	    hintp = &wmshell->wm.wm_hints;

	    hintp->flags = StateHint | InputHint;

	    if (XtIsSubclass((Widget)w, topLevelShellWidgetClass) &&
		    tlshell->topLevel.iconic) {
		hintp->initial_state = IconicState;
	    }
	    if(hintp->icon_x != XtUnspecifiedShellInt
	       || hintp->icon_y != XtUnspecifiedShellInt) {
		hintp->flags |= IconPositionHint;
	    }
	    if(hintp->icon_pixmap != NULL) hintp->flags |= IconPixmapHint;
	    if(hintp->icon_mask != NULL)   hintp->flags |= IconMaskHint;
	    if(hintp->icon_window != NULL) hintp->flags |= IconWindowHint;

	    if(hintp->window_group == XtUnspecifiedWindow) {
		if(w->core.parent) {
		    for (ptr = w->core.parent; ptr->core.parent;
			    ptr = ptr->core.parent) {}
		    hintp->window_group = XtWindow(ptr);
		    hintp->flags |=  WindowGroupHint;
		}
	    } else if (hintp->window_group != XtUnspecifiedWindowGroup)
		hintp->flags |=  WindowGroupHint;

	    XSetWMHints(dpy, win, hintp);

	    EvaluateSizeHints(wmshell);
    
	    XSetWMNormalHints(dpy, win, &wmshell->wm.size_hints);

	    if (wmshell->wm.transient
		&& hintp->window_group != XtUnspecifiedWindowGroup) {
		XSetTransientForHint(dpy, win, hintp->window_group);
	    }

	    classhint.res_name = w->core.name;
	    /* For the class, look up to the top of the tree */
	    for (wid = (Widget) w; wid->core.parent != NULL;
		    wid = wid->core.parent) {}
    
	    if (XtIsSubclass(wid, applicationShellWidgetClass)) {
		classhint.res_class =
			((ApplicationShellWidget) wid)->application.class;
	    } else classhint.res_class = XtClass(wid)->core_class.class_name;
    
	    XSetClassHint(dpy, win, &classhint);
	    SetHostName(dpy, win, hostname);
	}

	if(XtIsSubclass((Widget)w, applicationShellWidgetClass) &&
		appshell->application.argc != -1) {
	    XSetCommand(dpy, win, appshell->application.argv,
		    appshell->application.argc);
	}
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
		if (XtIsSubclass(wid, wmShellWidgetClass) &&
			!wmshell->wm.wait_for_wm) {
		    /* Consider trusting the wm again */
		    register XSizeHints *hintp = &wmshell->wm.size_hints;
#define EQ(x) (hintp->x == w->core.x)
		    if (EQ(x) && EQ(y) && EQ(width) && EQ(height)) {
			wmshell->wm.wait_for_wm = TRUE;
		    }
#undef EQ
		}		    
		break;

	    case ClientMessage:
		if( event->xclient.message_type == WM_CONFIGURE_DENIED(wid)  &&
			XtIsSubclass(wid, wmShellWidgetClass)) {

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
		    if (XtIsSubclass((Widget)w, wmShellWidgetClass)) {
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

    if (!XtIsRealized ((Widget) wid)) {
	Boolean is_wmshell = XtIsSubclass(wid, wmShellWidgetClass);
	int x, y, width, height, win_gravity = -1, flag;
	XSizeHints hints, *hintsP;

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
		    width -= hintsP->base_width;
		    height -= hintsP->base_height;
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
	    if (wmshell->wm.size_hints.win_gravity == XtUnspecifiedShellInt) {
		if (win_gravity != -1)
		    wmshell->wm.size_hints.win_gravity = win_gravity;
		else
		    wmshell->wm.size_hints.win_gravity = NorthWestGravity;
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
	XWindowChanges xwc;

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
	xwc.x = request->x;
	xwc.y = request->y;
	xwc.width = request->width;
	xwc.height = request->height;
	xwc.border_width = request->border_width;
	if (request->request_mode & CWSibling)
	    xwc.sibling = XtWindow(request->sibling);
	xwc.stack_mode = request->stack_mode;
	if (_ask_wm_for_size(shell, &xwc, request->request_mode)) {

	    /* If approved, shell's fields have been updated */

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

static Boolean _ask_wm_for_size(w, values, mask)
ShellWidget	w;
XWindowChanges *values;
Cardinal mask;
{
	WMShellWidget wmshell = (WMShellWidget) w;
	XEvent event;
	Boolean wm = XtIsSubclass((Widget) w, wmShellWidgetClass);
	register XSizeHints *hintp;
	int oldx, oldy;

	if (wm) {
	    hintp = &wmshell->wm.size_hints;
	    oldx = hintp->x = w->core.x;
	    oldy = hintp->y = w->core.y;
	    hintp->width = w->core.width;
	    hintp->height = w->core.height;
	}

	if (mask & CWX) {
		if (w->core.x == values->x) mask &= ~CWX;
		else if (wm) {
			hintp->flags &= ~USPosition;
			hintp->flags |= PPosition;
			w->core.x = hintp->x = values->x;
		} else w->core.x = values->x;
	}
	if (mask & CWY) {
		if (w->core.y == values->y) mask &= ~CWY;
		else if (wm) {
			hintp->flags &= ~USPosition;
			hintp->flags |= PPosition;
			w->core.y = hintp->y = values->y;
		} else w->core.y = values->y;
	}
	if (mask & CWBorderWidth) {
		if (w->core.border_width == values->border_width) {
			mask &= ~CWBorderWidth;
		} else w->core.border_width = values->border_width;
	}
	if (mask & CWWidth) {
		if (w->core.width == values->width) mask &= ~CWWidth;
		else if (wm) {
			hintp->flags &= ~USSize;
			hintp->flags |= PSize;
			hintp->width = values->width;
		} else w->core.width = values->width;
	} else values->width = w->core.width; /* for _wait_for_response */
	if (mask & CWHeight) {
		if (w->core.height == values->height) mask &= ~CWHeight;
		else if (wm) {
			hintp->flags &= ~USSize;
			hintp->flags |= PSize;
			hintp->height = values->height;
		} else w->core.height = values->height;
	} else values->height = w->core.height; /* for _wait_for_response */

	if (mask == 0) return TRUE;

	if (!w->shell.override_redirect &&
		mask & (CWX | CWY | CWWidth | CWHeight | CWBorderWidth)) {
	    XSetWMNormalHints(XtDisplay(w), XtWindow(w), hintp);
	}

	XConfigureWindow(XtDisplay(w), XtWindow(w), mask, values);

	if (w->shell.override_redirect) return TRUE;

	/* If no non-stacking bits are set, there's no way to tell whether
	   or not this worked, so assume it did */

	if (!(mask & ~(CWStackMode | CWSibling))) return TRUE;

	if (wmshell->wm.wait_for_wm == FALSE) {
		/* the window manager is sick
		 * so I will do the work and 
		 * say no so if a new WM starts up,
		 * or the current one recovers
		 * my size requests will be visable
		 */
		return FALSE;
	}
	
	if (_wait_for_response(wmshell, values, &event)){
		/* got an event */
		if (event.type == ConfigureNotify) {
			w->core.width = event.xconfigure.width;
			w->core.height = event.xconfigure.height;
			w->core.border_width = event.xconfigure.border_width;
			if (event.xany.send_event /* ICCCM compliant synth */
			    || w->shell.client_specified & _XtShellNotReparented)
			{
			    w->core.x = event.xconfigure.x;
			    w->core.y = event.xconfigure.y;
			    w->shell.client_specified |= _XtShellPositionValid;
			}
			else w->shell.client_specified &= ~_XtShellPositionValid;
			return TRUE;
		} else if (event.type == ClientMessage &&
			   event.xclient.message_type == WM_CONFIGURE_DENIED(w)) {
			w->core.x = oldx;
			w->core.y = oldy;
			return FALSE;
		} else if (event.type == ClientMessage &&
			    event.xclient.message_type == WM_MOVED(w)) {
		    	w->core.x = event.xclient.data.s[0];
			w->core.y = event.xclient.data.s[1];
			w->shell.client_specified |= _XtShellPositionValid;
			return TRUE;
		} else XtAppErrorMsg(XtWidgetToApplicationContext((Widget)w),
				"internalError","shell","XtToolkitError",
                             "Shell's window manager interaction is broken",
			     (String *)NULL, (Cardinal *)NULL);
	} else {
		wmshell->wm.wait_for_wm = FALSE;
		return FALSE;
	}
	return FALSE;
}

static void _do_setsave_under(w, flag)
ShellWidget w;
Boolean flag;
{
	Mask mask;
	XSetWindowAttributes attr;

	if (XtIsRealized((Widget)w)) {
	    mask = CWSaveUnder;
	    attr.save_under = flag;
	    XChangeWindowAttributes(XtDisplay(w), XtWindow(w), mask, &attr);
	}
}

static void _do_setoverride_redirect(w, flag)
ShellWidget w;
Boolean flag;
{
	Mask mask;
	XSetWindowAttributes attr;

	if(XtIsRealized((Widget)w)) {
		mask = CWOverrideRedirect;
		attr.override_redirect = flag;
		XChangeWindowAttributes(XtDisplay(w), XtWindow(w), mask, &attr);
		_popup_set_prop(w);
	}
}

/* ARGSUSED */
static Boolean SetValues(old, ref, new)
	Widget old, ref, new;
{
	ShellWidget nw = (ShellWidget) new;
	ShellWidget ow = (ShellWidget) old;
	XWindowChanges values;
	Cardinal mask;

	if (ow->shell.save_under != nw->shell.save_under) {
	    _do_setsave_under(nw, nw->shell.save_under) ;
	}

	if (ow->shell.override_redirect != nw->shell.override_redirect) {
	    _do_setoverride_redirect(nw, nw->shell.override_redirect) ;
	}

#define COPY_GEOMETRY(w1,w2) {						\
	w1->core.x = w2->core.x; w1->core.y = w2->core.y;		\
	w1->core.width = w2->core.width; w1->core.height = w2->core.height; \
	w1->core.border_width = w2->core.border_width;	}

	if (!XtIsRealized((Widget)ow)) { 
	    COPY_GEOMETRY(ow,nw)
	} else {
	    mask = 0;
#define EQC(x) (ow->core.x == nw->core.x)
	    if (!EQC(x)) {
		mask |= CWX;
		values.x = nw->core.x;
	    }
	    if (!EQC(y)) {
		mask |= CWY;
		values.y = nw->core.y;
	    }
	    if (!EQC(width)) {
		mask |= CWWidth;
		values.width = nw->core.width;
	    }
	    if (!EQC(height)) {
		mask |= CWHeight;
		values.height = nw->core.height;
	    }
	    if (!EQC(border_width)) {
		mask |= CWBorderWidth;
		values.border_width = nw->core.border_width;
	    }
	    if (mask) (void) _ask_wm_for_size(ow, &values, mask);
	    COPY_GEOMETRY(nw,ow);
#undef EQC
	}
	return FALSE;
#undef COPY_GEOMETRY
}

/* ARGSUSED */
static Boolean WMSetValues(old, ref, new)
	Widget old, ref, new;
{
	WMShellWidget nwmshell = (WMShellWidget) new;
	WMShellWidget owmshell = (WMShellWidget) old;
	Boolean size = FALSE;
	register XSizeHints *osize, *nsize;
	register XWMHints *ohints, *nhints;

	nsize = &nwmshell->wm.size_hints;
	osize = &owmshell->wm.size_hints;

	nsize->flags = 0;

#define EQS(x) (nsize->x == osize->x)

	if (! EQS(min_width) || ! EQS(min_height)) {
	    if (nsize->min_width != XtUnspecifiedShellInt
		|| nsize->min_height != XtUnspecifiedShellInt) {
		nsize->flags |= PMinSize;
		if (nsize->min_width == XtUnspecifiedShellInt)
		    nsize->min_width = 1;
		if (nsize->min_height == XtUnspecifiedShellInt)
		    nsize->min_height = 1;
	    }
	    size = TRUE;
	}

	if ( ! EQS(max_width) || ! EQS(max_height) ) {
	    if (nsize->max_width != XtUnspecifiedShellInt
		|| nsize->max_height != XtUnspecifiedShellInt) {
		nsize->flags |= PMaxSize;
		if (nsize->max_width == XtUnspecifiedShellInt)
		    nsize->max_width = BIGSIZE;
		if (nsize->max_height == XtUnspecifiedShellInt)
		    nsize->max_height = BIGSIZE;
	    }
	    size = TRUE;
	}

	if ( ! EQS(width_inc) || ! EQS(height_inc) ) {
	    if (nsize->width_inc != XtUnspecifiedShellInt
		|| nsize->height_inc != XtUnspecifiedShellInt) {
		nsize->flags |= PResizeInc;
	    }
	    size = TRUE;
	}

	if ( ! EQS(min_aspect.x) ||! EQS(min_aspect.y) ||
		! EQS(max_aspect.x) ||! EQS(max_aspect.y)) {
		
	    if (nsize->min_aspect.x != XtUnspecifiedShellInt
		|| nsize->min_aspect.y != XtUnspecifiedShellInt
		|| nsize->max_aspect.x != XtUnspecifiedShellInt
		|| nsize->max_aspect.y != XtUnspecifiedShellInt) {
		nsize->flags |= PAspect;
	    }
	    size = TRUE;
	}

	if (size) {
	    nsize->x = new->core.x;
	    nsize->y = new->core.y;
	    nsize->width = new->core.width;
	    nsize->height = new->core.height;
	    nsize->flags |= PPosition | PSize;
	    if (XtIsRealized(new) && !nwmshell->shell.override_redirect) {
		XSetWMNormalHints(XtDisplay(new), XtWindow(new), nsize);
	    }			    
	}

	if (nwmshell->wm.title != owmshell->wm.title) {
	    XtFree(owmshell->wm.title);
	    nwmshell->wm.title = XtNewString(nwmshell->wm.title);
	    if (XtIsRealized(new) && !nwmshell->shell.override_redirect) {
		XStoreName(XtDisplay(new), XtWindow(new), nwmshell->wm.title);
	    }
	}

	ohints = &owmshell->wm.wm_hints;
	nhints = &nwmshell->wm.wm_hints;

#define EQW(x)	(ohints->x == nhints->x)

	nhints->flags = 0;
	if (! EQW(initial_state)) nhints->flags |= StateHint;
	if (! EQW (icon_x) || ! EQW (icon_y)) nhints->flags |= IconPositionHint;
	if (! EQW(icon_pixmap)) nhints->flags |= IconPixmapHint;
	if (! EQW(input)) nhints->flags |= InputHint;
	if (! EQW(icon_mask)) nhints->flags |= IconMaskHint;
	if (! EQW(icon_window)) nhints->flags |= IconWindowHint;
	if (! EQW(window_group)
	    && nhints->window_group != XtUnspecifiedWindowGroup)
	    nhints->flags |= WindowGroupHint;

	if (nhints->flags && XtIsRealized(new) &&
		!nwmshell->shell.override_redirect) {

	    XSetWMHints(XtDisplay(new), XtWindow(new), &nwmshell->wm.wm_hints);
	}
	return FALSE;
}

/* ARGSUSED */
static Boolean TopLevelSetValues(old, ref, new)
	Widget old, ref, new;
{
	TopLevelShellWidget otlshell = (TopLevelShellWidget) old;
	TopLevelShellWidget ntlshell = (TopLevelShellWidget) new;

 	if (otlshell->topLevel.icon_name != ntlshell->topLevel.icon_name) {
	    XtFree(otlshell->topLevel.icon_name);
	    ntlshell->topLevel.icon_name = XtNewString(
		ntlshell->topLevel.icon_name);
	    if (XtIsRealized(new) && !ntlshell->shell.override_redirect) {
		XSetIconName(XtDisplay(new), XtWindow(new),
		    ntlshell->topLevel.icon_name);
	    }
	}
	return FALSE;
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
