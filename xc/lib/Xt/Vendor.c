/* $XConsortium: Vendor.c,v 1.40 91/02/05 16:59:31 gildea Exp $ */

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

/* Make sure all wm properties can make it out of the resource manager */

#include "IntrinsicI.h"
#include "StringDefs.h"
#include "Shell.h"
#include "ShellP.h"
#include "Vendor.h"
#include "VendorP.h"

#include <pwd.h>
#include <stdio.h>


/***************************************************************************
 *
 * Vendor shell class record
 *
 ***************************************************************************/

static void _VendorShellInitialize();
static Boolean _VendorShellSetValues();
static void Realize();

externaldef(vendorshellclassrec) VendorShellClassRec vendorShellClassRec = {
  {
    /* superclass         */    (WidgetClass) &wmShellClassRec,
    /* class_name         */    "VendorShell",
    /* size               */    sizeof(VendorShellRec),
    /* Class Initializer  */	NULL,
    /* class_part_initialize*/	NULL,
    /* Class init'ed ?    */	FALSE,
    /* initialize         */    _VendorShellInitialize,
    /* initialize_notify    */	NULL,		
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    NULL,
    /* resource_count     */	0,
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    NULL,
    /* set_values         */    _VendorShellSetValues,
    /* set_values_hook      */	NULL,			
    /* set_values_almost    */	XtInheritSetValuesAlmost,  
    /* get_values_hook      */	NULL,
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */	NULL,
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
  }
};

#if !defined(AIXSHLIB) || !defined(SHAREDCODE)
externaldef(vendorshellwidgetclass) WidgetClass vendorShellWidgetClass =
	(WidgetClass) (&vendorShellClassRec);
#endif

/* ARGSUSED */
static void _VendorShellInitialize(req, new, args, num_args)
	Widget req, new;
	ArgList args;
	Cardinal *num_args;
{
}

/* ARGSUSED */
static Boolean _VendorShellSetValues(old, ref, new, args, num_args)
	Widget old, ref, new;
	ArgList args;
	Cardinal *num_args;
{
	return FALSE;
}

static void Realize(wid, vmask, attr)
	Widget wid;
	Mask *vmask;
	XSetWindowAttributes *attr;
{
	WidgetClass super = wmShellWidgetClass;

	/* Make my superclass do all the dirty work */

	(*super->core_class.realize) (wid, vmask, attr);
}

