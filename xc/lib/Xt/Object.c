/* $XConsortium: Object.c,v 1.23 94/03/31 15:37:24 converse Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital, MIT, or Sun not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

#define OBJECT
#include "IntrinsicI.h"
#include "StringDefs.h"

static XtResource resources[] = {
        {XtNdestroyCallback, XtCCallback, XtRCallback,sizeof(XtPointer),
         XtOffsetOf(ObjectRec,object.destroy_callbacks),
	 XtRCallback, (XtPointer)NULL}
    };

static void ObjectClassPartInitialize();
static Boolean ObjectSetValues();
static void ObjectDestroy();

externaldef(objectclassrec) ObjectClassRec objectClassRec = {
  {
    /* superclass	  */	NULL,
    /* class_name	  */	"Object",
    /* widget_size	  */	sizeof(ObjectRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/	ObjectClassPartInitialize,
    /* class_inited       */	FALSE,
    /* initialize	  */	NULL,
    /* initialize_hook    */	NULL,		
    /* pad                */    NULL,
    /* pad		  */	NULL,
    /* pad       	  */	0,
    /* resources	  */	resources,
    /* num_resources	  */    XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* pad                */    FALSE,
    /* pad                */    FALSE,
    /* pad                */    FALSE,
    /* pad                */    FALSE,
    /* destroy		  */	ObjectDestroy,
    /* pad		  */	NULL,
    /* pad		  */	NULL,
    /* set_values	  */	ObjectSetValues,
    /* set_values_hook    */	NULL,			
    /* pad                */    NULL,
    /* get_values_hook    */	NULL,			
    /* pad                */    NULL,
    /* version		  */	XtVersion,
    /* callback_offsets   */    NULL,
    /* pad                */    NULL,
    /* pad                */    NULL,
    /* pad                */    NULL,
    /* extension	    */  NULL
}
};

externaldef(objectClass) WidgetClass objectClass
                          = (WidgetClass)&objectClassRec;

/*
 * Start of object routines.
 */


static void ConstructCallbackOffsets(widgetClass)
    WidgetClass widgetClass;
{
    static XrmQuark QCallback = NULLQUARK;
    register int i;
    register int tableSize;
    register CallbackTable newTable;
    register CallbackTable superTable;
    register XrmResourceList resourceList;
    ObjectClass objectClass = (ObjectClass)widgetClass;

    /*
      This function builds an array of pointers to the resource
      structures which describe the callbacks for this widget class.
      This array is special in that the 0th entry is the number of
      callback pointers.
     */

    if (QCallback == NULLQUARK)
	QCallback = XrmPermStringToQuark(XtRCallback);

    if (objectClass->object_class.superclass != NULL) {
	superTable = (CallbackTable)
	    ((ObjectClass) objectClass->object_class.superclass)->
		object_class.callback_private;
	tableSize = (int) superTable[0];
    } else { 
	superTable = (CallbackTable) NULL;
	tableSize = 0;
    }

    /* Count the number of callbacks */
    resourceList = (XrmResourceList) objectClass->object_class.resources;
    for (i = objectClass->object_class.num_resources; --i >= 0; resourceList++)
	if (resourceList->xrm_type == QCallback)
	    tableSize++;
	
    /*
     * Allocate and load the table.  Make sure that the new callback
     * offsets occur in the table ahead of the superclass callback
     * offsets so that resource overrides work.
     */
    newTable = (CallbackTable)
	XtMalloc(sizeof(XrmResource *) * (tableSize + 1));
	
    newTable[0] = (XrmResource *) tableSize;

    if (superTable)
	tableSize -= (int) superTable[0];
    resourceList = (XrmResourceList) objectClass->object_class.resources;
    for (i=1; tableSize > 0; resourceList++)
	if (resourceList->xrm_type == QCallback) {
	    newTable[i++] = resourceList;
	    tableSize--;
	}

    if (superTable)
	for (tableSize = (int) *superTable++; --tableSize >= 0; superTable++)
	    newTable[i++] = *superTable;
    
    objectClass->object_class.callback_private = (XtPointer) newTable;
}

static void InheritObjectExtensionMethods(widget_class)
    WidgetClass widget_class;
{
    ObjectClass oc = (ObjectClass) widget_class;
    ObjectClassExtension ext, super_ext = NULL;

    ext = (ObjectClassExtension)
	XtGetClassExtension(widget_class,
			    XtOffsetOf(ObjectClassPart, extension),
			    NULLQUARK, XtObjectExtensionVersion, 
			    sizeof(ObjectClassExtensionRec));

    if (oc->object_class.superclass)
	super_ext = (ObjectClassExtension)
	    XtGetClassExtension(oc->object_class.superclass,
				XtOffsetOf(ObjectClassPart, extension),
				NULLQUARK, XtObjectExtensionVersion, 
				sizeof(ObjectClassExtensionRec));
    if (ext) {
	if (ext->allocate == XtInheritAllocate)
	    ext->allocate = (super_ext ? super_ext->allocate : NULL);
	if (ext->deallocate == XtInheritDeallocate)
	    ext->deallocate = (super_ext ? super_ext->deallocate : NULL);
    } else if (super_ext) {
	/* Be careful to inherit only what is appropriate */
	ext = (ObjectClassExtension) 
	    XtCalloc(1, sizeof(ObjectClassExtensionRec));
	ext->next_extension = oc->object_class.extension;
	ext->record_type = NULLQUARK;
	ext->version = XtObjectExtensionVersion;
	ext->record_size = sizeof(ObjectClassExtensionRec);
	ext->allocate = super_ext->allocate;
	ext->deallocate = super_ext->deallocate;
	oc->object_class.extension = (XtPointer) ext;
    }
}

static void ObjectClassPartInitialize(wc)
    register WidgetClass wc;
{
   ObjectClass oc = (ObjectClass)wc;

   oc->object_class.xrm_class =
       XrmPermStringToQuark(oc->object_class.class_name);

   if (oc->object_class.resources)
       _XtCompileResourceList(oc->object_class.resources,
			      oc->object_class.num_resources);

   ConstructCallbackOffsets(wc);
   _XtResourceDependencies(wc);
   InheritObjectExtensionMethods(wc);
}


/*ARGSUSED*/
static Boolean ObjectSetValues(old, request, widget, args, num_args)
    Widget	old, request, widget;
    ArgList args;
    Cardinal *num_args;
{
    register CallbackTable offsets;
    register int i;
    register InternalCallbackList *ol, *nl;

    LOCK_PROCESS;
    /* Compile any callback lists into internal form */
    offsets = (CallbackTable) XtClass(widget)->core_class.callback_private;

    for (i= (int) *(offsets++); --i >= 0; offsets++) {
	ol = (InternalCallbackList *)
	    ((char *) old - (*offsets)->xrm_offset - 1);
	nl = (InternalCallbackList *)
	    ((char *) widget - (*offsets)->xrm_offset - 1);
	if (*ol != *nl) {
	    if (*ol != NULL)
		XtFree((char *) *ol);
	    if (*nl != NULL)
		*nl = _XtCompileCallbackList((XtCallbackList) *nl);
	}
    }
    UNLOCK_PROCESS;
    return False;
}


static void ObjectDestroy (widget)
    register Widget    widget;
{
    register CallbackTable offsets;
    register int i;
    register InternalCallbackList cl;

    /* Remove all callbacks associated with widget */
    LOCK_PROCESS;
    offsets = (CallbackTable)
	widget->core.widget_class->core_class.callback_private;

    for (i = (int) *(offsets++); --i >= 0; offsets++) {
	cl = *(InternalCallbackList *)
	    ((char *) widget - (*offsets)->xrm_offset - 1);
	if (cl) XtFree((char *) cl);
    }
    UNLOCK_PROCESS;
} /* ObjectDestroy */


