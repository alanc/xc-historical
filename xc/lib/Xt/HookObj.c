/* $XConsortium: HookObj.c,v 1.0 93/10/25 15:35:12 kaleb Exp $ */

#include "IntrinsicI.h"
#include "HookObjP.h"
#include "StringDefs.h"
/******************************************************************
 *
 * Hook Object Resources
 *
 ******************************************************************/

static XtResource resources[] = {
  { XtNcreateHook, XtCCallback, XtRCallback, sizeof(XtPointer),
    XtOffsetOf(HookObjRec, hooks.createhook_callbacks),
    XtRCallback, (XtPointer)NULL},
  { XtNchangeHook, XtCCallback, XtRCallback, sizeof(XtPointer),
    XtOffsetOf(HookObjRec, hooks.changehook_callbacks),
    XtRCallback, (XtPointer)NULL},
  { XtNconfigureHook, XtCCallback, XtRCallback, sizeof(XtPointer),
    XtOffsetOf(HookObjRec, hooks.confighook_callbacks),
    XtRCallback, (XtPointer)NULL},
  { XtNgeometryHook, XtCCallback, XtRCallback, sizeof(XtPointer),
    XtOffsetOf(HookObjRec, hooks.geometryhook_callbacks),
    XtRCallback, (XtPointer)NULL},
  { XtNdestroyHook, XtCCallback, XtRCallback, sizeof(XtPointer),
    XtOffsetOf(HookObjRec, hooks.destroyhook_callbacks),
    XtRCallback, (XtPointer)NULL},
  { XtNshells, XtCReadOnly, XtRWidgetList, sizeof(WidgetList),
    XtOffsetOf(HookObjRec, hooks.shells), XtRImmediate, (XtPointer) NULL },
  { XtNnumShells, XtCReadOnly, XtRCardinal, sizeof(Cardinal),
    XtOffsetOf(HookObjRec, hooks.num_shells), XtRImmediate, (XtPointer) 0 }
};

static void GetValuesHook(), Initialize();

externaldef(hookobjclassrec) HookObjClassRec hookObjClassRec = {
  { /* Object Class Part */
    /* superclass	  */	(WidgetClass)&objectClassRec,
    /* class_name	  */	"Hook",
    /* widget_size	  */	sizeof(HookObjRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/	NULL,
    /* class_inited       */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook    */	NULL,		
    /* realize		  */	NULL,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	FALSE,
    /* compress_exposure  */	TRUE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	NULL,
    /* resize		  */	NULL,
    /* expose		  */	NULL,
    /* set_values	  */	NULL,
    /* set_values_hook    */	NULL,			
    /* set_values_almost  */	NULL,  
    /* get_values_hook    */	GetValuesHook,			
    /* accept_focus	  */	NULL,
    /* version		  */	XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry	    */  NULL,
    /* display_accelerator  */	NULL,
    /* extension	    */  NULL
  },
  { /* HookObj Class Part */
    /* unused               */	0
  }
};

externaldef(hookObjectClass) WidgetClass hookObjectClass = 
	(WidgetClass)&hookObjClassRec;

static void FreeShellList(w, closure, call_data)
    Widget w;
    XtPointer closure, call_data;
{
    HookObject h = (HookObject)w;
    if (h->hooks.shells != NULL)
	XtFree((char*)h->hooks.shells);
}

static void Initialize(req, new, args, num_args)
    Widget req, new;
    ArgList args;
    Cardinal* num_args;
{
    HookObject w = (HookObject) new;
    w->hooks.max_shells = 0;
    XtAddCallback (new, XtNdestroyCallback, FreeShellList, (XtPointer) NULL);
}

static void GetValuesHook(widget, args, num_args)
    Widget widget;
    ArgList args;
    Cardinal* num_args;
{
    /* get the XtNshells and XtNnumShells pseudo-resources */
}

