#ifndef lint
static char rcsid[] = "$Header: Grip.c,v 1.8 88/01/07 08:04:53 swick Exp $";
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
/*
 * Grip.c - Grip composite Widget (Used by VPane Widget)
 *
 */


#include <X/Intrinsic.h>
#include <X/Atoms.h>
#include <X/Grip.h>
#include "GripP.h"

static char *defaultTranslation[] = {
    NULL
};
static caddr_t defaultTranslations = (caddr_t)defaultTranslation;

static XtResource resources[] = {
   {XtNwidth, XtCWidth, XrmRInt, sizeof(int),
      XtOffset(GripWidget, core.width), XtRString, "6" },
   {XtNheight, XtCHeight, XrmRInt, sizeof(int),
      XtOffset(GripWidget, core.height), XtRString, "6" },
   {XtNbackground, XtCBackground, XrmRPixel, sizeof(Pixel),
      XtOffset(GripWidget, core.background_pixel), XtRString, "Black"},
   {XtNborderWidth, XtCBorderWidth, XrmRInt, sizeof(int),
      XtOffset(GripWidget, core.border_width), XtRString, "0"},
   {XtNcallback, XtCCallback, XtRPointer, sizeof(caddr_t), 
      XtOffset(GripWidget, grip.grip_action), XtRPointer, (caddr_t)NULL},
   {XtNtranslations, XtCTranslations, XtRTranslationTable,
      sizeof(XtTranslations),
      XtOffset(GripWidget, core.translations),XtRTranslationTable,
      (caddr_t)&defaultTranslations}
};

void GripAction( /* Widget, XEvent*, String*, Cardinal */ );

static XtActionsRec actionsList[] =
{
  {"GripAction",	GripAction},
};


GripClassRec gripClassRec = {
   {
/* core class fields */
    /* superclass         */   (WidgetClass) &widgetClassRec,
    /* class name         */   "Grip",
    /* size               */   sizeof(GripRec),
    /* class initialize   */   NULL,
    /* class_inited       */   FALSE,
    /* initialize         */   NULL,
    /* realize            */   XtInheritRealize,
    /* actions            */   actionsList,
    /* num_actions        */   XtNumber(actionsList),
    /* resourses          */   resources,
    /* resource_count     */   XtNumber(resources),
    /* xrm_class          */   NULLQUARK,
    /* compress_motion    */   TRUE,
    /* compress_exposure  */   TRUE,
    /* visible_interest   */   FALSE,
    /* destroy            */   NULL,
    /* resize             */   NULL,
    /* expose             */   NULL,
    /* set_values         */   NULL,
    /* accept_focus       */   NULL,
    /* callback_private   */   NULL,
    /* reserved_private   */   NULL,
   }, {
    /* mumble             */   0  /* make C compiler happy */
   }
};

WidgetClass gripWidgetClass = (WidgetClass) &gripClassRec;

static void GripAction( widget, event, params, num_params )
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal num_params;
{
    GripCallDataRec call_data;

    call_data.event = event;
    call_data.params = params;
    call_data.num_params = num_params;

    XtCallCallbacks( widget, XtNcallback, &call_data );
}
