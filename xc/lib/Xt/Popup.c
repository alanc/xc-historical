#ifndef lint
static char rcsid[] = "$Header: Popup.c,v 1.10 87/11/01 13:23:07 swick Locked $";
#endif lint

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

#include "Intrinsic.h"
#include "Atoms.h"
#include "Shell.h"
#include "ShellP.h"
#include "Popup.h"
#include "PopupP.h"

#define Offset(x)	(XtOffset(PopupWidget, x))

static XtResource resourcesTransient[] = {
	{ XtNsaveUnder, XtCSaveUnder, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XrmRString, "TRUE"},
	{ XtNtransient, XtCTransient, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.transient), XrmRString, "TRUE"},
};


static XtResource resourcesOverride[] = {
	{ XtNsaveUnder, XtCSaveUnder, XrmRBoolean, sizeof(Boolean),
	    Offset(shell.save_under), XrmRString, "TRUE"},
	{ XtNoverrideRedirect, XtCOverrideRedirect, XrmRBoolean,
	    sizeof(Boolean),
	    Offset(shell.override_redirect), XrmRString, "TRUE"},
};


PopupClassRec TransientClassRec = {
    /* superclass         */    (WidgetClass) &shellClassRec,
    /* class_name         */    "Popup",
    /* size               */    sizeof(PopupRec),
    /* Class Initializer  */	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resourcesTransient,
    /* resource_count     */	XtNumber(resourcesTransient),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* accept_focus       */    XtInheritAcceptFocus,
    /* callback offset    */    NULL,
    /* reserved           */    NULL,
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* move_focus_to_next */    XtInheritMoveFocusToNext,
    /* move_focus_to_prev */    XtInheritMoveFocusToPrev
};

PopupClassRec OverrideClassRec = {
    /* superclass         */    (WidgetClass) &shellClassRec,
    /* class_name         */    "Popup",
    /* size               */    sizeof(PopupRec),
    /* Class Initializer  */	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resourcesOverride,
    /* resource_count     */	XtNumber(resourcesOverride),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* accept_focus       */    XtInheritAcceptFocus,
    /* callback offset    */    NULL,
    /* reserved           */    NULL,
    /* geometry_manager   */    XtInheritGeometryManager,
    /* change_managed     */    XtInheritChangeManaged,
    /* insert_child	  */	XtInheritInsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* move_focus_to_next */    XtInheritMoveFocusToNext,
    /* move_focus_to_prev */    XtInheritMoveFocusToPrev
};

WidgetClass popupWidgetClass = (WidgetClass) (&TransientClassRec);
