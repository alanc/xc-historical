#ifndef lint
static char *sccsid = "@(#)ResourceList.c	1.10	2/25/87";
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

/* Converted to classing toolkit on 28 August 1987 by Joel McCormack */

/* XtResourceList.c -- compile and process resource lists. */

#include "Xlib.h"
#include "Intrinsic.h"
#include "Atoms.h"
#include "Xresource.h"
#include <stdio.h>


static XrmName	QreverseVideo;
static XrmClass	QBoolean, QString;

extern void bcopy();

static void CopyFromArg(src, dst, size)
    XtArgVal src, dst;
    register unsigned int size;
{
    if (size == sizeof(XtArgVal))
	*(XtArgVal *)dst = src;
#ifdef BIGENDIAN
    else if (size == sizeof(short)) 
	*(short *)dst = (short)src;
#endif BIGENDIAN
    else if (size < sizeof(XtArgVal))
	bcopy((char *) &src, (char *) dst, (int) size);
    else
	bcopy((char *) src, (char *) dst, (int) size);

}

static void CopyToArg(src, dst, size)
    XtArgVal src, *dst;
    register unsigned int size;
{
    if (size == sizeof(XtArgVal))
	*dst = *(XtArgVal *)src;
#ifdef BIGENDIAN
    else if (size == sizeof(short)) 
	*dst = (XtArgVal) *((short *) src);
#endif BIGENDIAN
    else if (size < sizeof(XtArgVal))
	bcopy((char *) src, (char *) dst, (int) size);
    else
	bcopy((char *) src, (char *) *dst, (int) size);

}

void PrintResourceList(list, count)
    register ResourceList list;
    register int count;
{
    for (; --count >= 0; list++) {
        (void) printf("    name: %s, class: %s, type: %s,\n",
	    list->resource_name, list->resource_class, list->resource_type);
	(void) printf("    size: %d, offset: %x, def_type: %s, def_addr: %x\n",
	    list->resource_size, list->resource_offset,
	    list->default_type, list->default_addr);
    }
}

static Cardinal GetNamesAndClasses(w, names, classes)
    register Widget	  w;
    register XrmNameList  names;
    register XrmClassList classes;
{
    register Cardinal length, j;
    register XrmQuark t;

    for (length = 0; w != NULL; w = (Widget) w->core.parent) {
	names[length] = w->core.xrm_name;
	classes[length] = w->core.widget_class->coreClass.xrm_class;
	length++;
     }
    /* They're in backwards order, flop them around */
    for (j = 0; j < length/2; j++) {
	t = names[j];
	names[j] = names[length-j-1];
	names[length-j-1] = t;
        t = classes[j];
	classes[j] = classes[length-j-1];
	classes[length-j-1] = t;
    }
    return length;
}


/* Spiffy fast compiled form of resource list.				*/
/* ResourceLists are compiled in-place into XrmResourceLists		*/
/* All atoms are replaced by quarks, and offsets are -offset-1 to	*/
/* indicate that this list has been compiled already			*/

typedef struct {
    XrmQuark	xrm_name;	  /* Resource name quark 		*/
    XrmQuark	xrm_class;	  /* Resource class quark 		*/
    XrmQuark	xrm_type;	  /* Resource representation type quark */
    Cardinal	xrm_size;	  /* Size in bytes of representation	*/
    long int	xrm_offset;	  /* -offset-1				*/
    XrmQuark	xrm_default_type; /* Default representation type quark 	*/
    caddr_t	xrm_default_addr; /* Default resource address		*/
} XrmResource, *XrmResourceList;

XrmResourceList XrmCompileResourceList(resources, resourceCount)
    register ResourceList resources;
    	     Cardinal	  resourceCount;
{
    register XrmResourceList xrmres;
    register Cardinal count;

    for (xrmres = (XrmResourceList) resources, count = 0;
         count < resourceCount;
	 xrmres++, resources++, count++) {
    	xrmres->xrm_name	 = XrmAtomToName(resources->resource_name);
    	xrmres->xrm_class	 = XrmAtomToClass(resources->resource_class);
    	xrmres->xrm_type	 = XrmAtomToQuark(resources->resource_type);
	xrmres->xrm_size	 = resources->resource_size;
        xrmres->xrm_offset	 = -resources->resource_offset - 1;
    	xrmres->xrm_default_type = XrmAtomToQuark(resources->default_type);
	xrmres->xrm_default_addr = resources->default_addr;
    }
} /* XrmCompileResourceList */

/* ||| References to display should be references to screen */

void XrmGetResources(
    dpy, base, names, classes, length, resources, resourceCount, args, argCount)

    Display	  *dpy;		   /* The widget's display connection	  */
    caddr_t	  base;		   /* Base address of memory to write to  */
    register XrmNameList names;	   /* Full inheritance name of widget  	  */
    register XrmClassList classes; /* Full inheritance class of widget 	  */
    Cardinal	  length;	   /* Number of entries in names, classes */
    ResourceList  resources;	   /* The list of resources required. 	  */
    Cardinal	  resourceCount;   /* number of items in resource list    */
    ArgList 	  args;		   /* ArgList to override resources	  */
    Cardinal	  argCount;	   /* number of items in arg list	  */
{
    register 	ArgList		arg;
    register 	XrmName		argName;
		XrmResourceList	xrmres;
    register 	XrmResourceList	res;
    		XrmValue	val, defaultVal;
    register 	int		j;
    		int		length, i;
		Boolean		reverseVideo, getReverseVideo;
    		XrmHashTable	searchList[100];
    static	Boolean		found[1000];

    reverseVideo    = FALSE;
    getReverseVideo = TRUE;

    /* ||| This should be passed a compiled arg list, too, but such has to
       be allocated dynamically */

    /* ||| Should be warnings? or error? */
    if ((args == NULL) && (argCount != 0)) {
    	XtError("argument count > 0 on NULL argument list");
	argCount = 0;
    }
    if ((resources == NULL) && (resourceCount != 0)) {
    	XtError("resource count > 0 on NULL resource list");
	return;
    }

    if (resourceCount != 0) {
	/* Compile resource list if needed */
	if (resources->resource_offset >= 0) {
	    xrmres = XrmCompileResourceList(resources, resourceCount);
	} else {
	    xrmres = (XrmResourceList) resources;
        }

	/* Mark each resource as not found on arg list */
	for (j = 0; j < resourceCount; j++) {
	    found[j] = FALSE;
    	}

	/* Copy the args into the resources, mark each as found */
	for (arg = args, i = 0; i < argCount; i++, arg++) {
	    argName = XrmAtomToName(arg->name);
	    if (argName == QreverseVideo) {
		reverseVideo = (Boolean) arg->value;
		getReverseVideo = FALSE;
	    } else {
		for (j = 0, res = xrmres; j < resourceCount; j++, res++) {
		    if (argName == res->xrm_name) {
			CopyFromArg(arg->value, base - res->xrm_offset - 1,
				res->xrm_size);
			found[j] = TRUE;
			break;
		    }
		}
	    }
	}
    }

    /* Resources name and class will go into names[length], classes[length] */
    names[length+1]   = NULLQUARK;
    classes[length+1] = NULLQUARK;

#ifdef reverseVideoHack
    if (XDisplayCells(dpy, DefaultScreen(dpy)) > 2) {
    	/* Color box, ignore ReverseVideo */
	reverseVideo = FALSE;
    } else if (getReverseVideo) {
	names[length] = QreverseVideo;
	classes[length] = QBoolean;
	XrmGetResource(dpy, names, classes, QBoolean, &val);
	if (val.addr)
	    reverseVideo = *((Boolean *) val.addr);
    }
    /* ||| Nothing is done w/reverseVideo now, but something should be! */
#endif

    names[length] = NULLQUARK;
    classes[length] = NULLQUARK;

    if (resourceCount != 0) {

	/* Ask resource manager for a list of database levels that we can
	   do a single-level search on each resource */

	XrmGetSearchList(names, classes, searchList);
	
	/* go to the resource manager for those resources not found yet */
	/* if it's not in the resource database use the default value   */
    
	for (res = xrmres, j = 0; j < resourceCount; j++, res++) {
	    if (! found[j]) {
		XrmGetSearchResource(dpy, searchList, res->xrm_name,
		 res->xrm_class, res->xrm_type, &val);
		if (val.addr == NULL && res->xrm_default_addr != NULL) {
		    /* Convert default value to proper type */
		    defaultVal.addr = res->xrm_default_addr;
		    defaultVal.size = sizeof(caddr_t);
		    _XrmConvert(dpy, res->xrm_default_type, defaultVal, 
		    	res->xrm_type, &val);
		}
		if (val.addr) {
		    if (res->xrm_type == QString) {
			*((caddr_t *)(base - res->xrm_offset - 1)) = val.addr;
#ifdef BIGENDIAN
/* ||| Why? This should be handled by string to short, etc. conversions */
          	    } else if (res->xrm_size == sizeof(short)) {
		        *(short *) (base - res->xrm_offset - 1) =
				(short)*((int *)val.addr);
#endif BIGENDIAN
		    } else {
		        bcopy(
			    (char *) val.addr,
			    (char *) (base - res->xrm_offset - 1), 
			    (int) res->xrm_size);
		    }
		} else if (res->xrm_default_addr != NULL) {
		    bcopy(
			(char *) res->xrm_default_addr,
			(char *) (base - res->xrm_offset - 1),
			(int) res->xrm_size);
		}
	    }
	}
    }
}

void GetResources(widgetClass, w, names, classes, length, args, argCount)
    WidgetClass	  widgetClass;
    Widget	  w;
    XrmNameList	  names;
    XrmClassList  classes;
    Cardinal	  length;
    ArgList	  args;
    Cardinal	  argCount;
{
    /* First get resources for superclasses */
    if (widgetClass->coreClass.superclass != NULL) {
        GetResources(widgetClass->coreClass.superclass,
	    w, names, classes, length, args, argCount);
    }
    /* Then for this class */
    XrmGetResources(w->core.display, (caddr_t) w, names, classes, length,
        widgetClass->coreClass.resources, widgetClass->coreClass.num_resource,
	args, argCount);
} /* GetResources */


void XtGetResources(w, args, argCount)
    register 	Widget	  w;
    		ArgList	  args;
    		Cardinal  argCount;
{
    XrmName	names[100];
    XrmClass	classes[100];
    Cardinal	length;

    /* Make sure xrm_class, xrm_name are valid */
    if (w->core.widget_class->coreClass.xrm_class == NULLQUARK) {
        w->core.widget_class->coreClass.xrm_class =
	    XrmAtomToClass(w->core.widget_class->coreClass.class_name);
    }
    w->core.xrm_name = XrmAtomToName(w->core.name);

    /* Get names, classes for widget on up */
    length = GetNamesAndClasses(w, names, classes);
   
    /* Get resources starting at CorePart on down to this widget */
    GetResources(w->core.widget_class, w, names, classes, length,
        args, argCount);
} /* XtGetResources */

void XtGetSubresources
	(w, base, name, class, resources, resourceCount, args, argCount)
    Widget	  w;		  /* Widget "parent" of subobject */
    caddr_t	  base;		  /* Base address to write to     */
    XrmAtom	  name;		  /* name of subobject		  */
    XrmAtom	  class;	  /* class of subobject		  */
    ResourceList  resources;	  /* resource list for subobject  */
    Cardinal	  resourceCount;
    ArgList	  args;		  /* arg list to override resources */
    Cardinal	  argCount;
{
    XrmName	  names[100];
    XrmClass	  classes[100];
    Cardinal	  length;

    /* Get full name, class of subobject */
    length = GetNamesAndClasses(w, names, classes);
    names[length] = XrmAtomToName(name);
    classes[length] = XrmAtomToClass(class);
    length++;

    /* Fetch resources */
    XrmGetResources(XtDisplay(w), base, names, classes, length,
        resources, resourceCount, args, argCount);
}


void XrmGetValues(base, resources, resourceCount, args, argCount)
  caddr_t		base;		/* Base address to fetch values from */
  register ResourceList resources;	/* The current resource values.      */
  register Cardinal	resourceCount;	/* number of items in resources      */
  ArgList 		args;		/* The resource values requested     */
  int			argCount;	/* number of items in arg list       */
{
    register ArgList		arg;
    register XrmResourceList	xrmres;
    register int 		i;
    register XrmName		argName;

    if (resourceCount == 0) return;

    /* Resource lists are assumed to be in compiled form already via the
       initial XtGetResources, XtGetSubresources calls */

    for (arg = args ; --argCount >= 0; arg++) {
	argName = XrmAtomToName(arg->name);
	for (xrmres = (XrmResourceList) resources, i = 0;
	     i < resourceCount;
	     i++, xrmres++) {
	    if (argName == xrmres->xrm_name) {
		CopyToArg(base - xrmres->xrm_offset - 1,
		          &arg->value,
			  xrmres->xrm_size);
		break;
	    }
	}
    }
}

void GetValues(widgetClass, w, args, argCount)
    WidgetClass	  widgetClass;
    Widget	  w;
    ArgList	  args;
    Cardinal	  argCount;
{
    /* First get resource values for superclass */
    if (widgetClass->coreClass.superclass != NULL) {
        GetValues(widgetClass->coreClass.superclass, w, args, argCount);
    }
    /* Then for this class */
    XrmGetValues((caddr_t) w,
        widgetClass->coreClass.resources, widgetClass->coreClass.num_resource,
	args, argCount);
} /* GetValues */

void XtGetValues(w, args, argCount)
    	 	Widget	  w;
    		ArgList	  args;
    		Cardinal  argCount;
{
    if (argCount == 0) return;
    if ((args == NULL) && (argCount != 0)) {
	XtError("argument count > 0 on NULL argument list");
	return;
    }
    /* Get resource values starting at CorePart on down to this widget */
    GetValues(w->core.widget_class, w, args, argCount);
} /* XtGetValues */
 
void XrmSetValues(base, resources, resourceCount, args, argCount)
  caddr_t		base;		/* Base address to write values to   */
  register ResourceList resources;	/* The current resource values.      */
  register Cardinal	resourceCount;	/* number of items in resources      */
  ArgList 		args;		/* The resource values to set        */
  int			argCount;	/* number of items in arg list       */
{
    register ArgList		arg;
    register XrmResourceList	xrmres;
    register int 	        i;
    register XrmName		argName;

    if (resourceCount == 0) return;

    /* Resource lists are assumed to be in compiled form already via the
       initial XtGetResources, XtGetSubresources calls */

    for (arg = args ; --argCount >= 0; arg++) {
	argName = XrmAtomToName(arg->name);
	for (xrmres = (XrmResourceList) resources, i = 0;
	     i < resourceCount;
	     i++, xrmres++) {
	    if (argName == xrmres->xrm_name) {
		CopyFromArg(arg->value,
			    base - xrmres->xrm_offset - 1,
			    xrmres->xrm_size);
		break;
	    }
	}
    }
} /* XrmSetValues */

void SetValues(widgetClass, w, args, argCount)
    WidgetClass	  widgetClass;
    Widget	  w;
    ArgList	  args;
    Cardinal	  argCount;
{
    /* First set resource values for superclass */
    if (widgetClass->coreClass.superclass != NULL) {
        SetValues(widgetClass->coreClass.superclass, w, args, argCount);
    }
    /* Then for this class */
    XrmSetValues((caddr_t) w,
        widgetClass->coreClass.resources, widgetClass->coreClass.num_resource,
	args, argCount);
} /* SetValues */

void XtSetValues(w, args, argCount)
    	 	Widget	  w;
    		ArgList	  args;
    		Cardinal  argCount;
{
    Widget	newWidget;
    Cardinal	widgetSize;

    if (argCount == 0) return;
    if ((args == NULL) && (argCount != 0)) {
	XtError("argument count > 0 on NULL argument list");
	return;
    }

    /* Allocate and copy current widget into newWidget */
    newWidget = (Widget) XtMalloc(w->core.widget_class->coreClass.size);
    bcopy((char *) w, (char *) newWidget, (int) widgetSize);

    /* Set resource values starting at CorePart on down to this widget */
    GetValues(w->core.widget_class, newWidget, args, argCount);

    /* Inform widget of changes and deallocate newWidget */
    w->core.widget_class->coreClass.set_values(w, newWidget);
    XtFree(newWidget);
} /* XtSetValues */
 

static Boolean initialized = FALSE;

extern void ResourceListInitialize()
{
    if (initialized)
    	return;
    initialized = TRUE;

    QreverseVideo = XrmAtomToName(XtNreverseVideo);
    QBoolean = XrmAtomToClass(XtCBoolean);
    QString = XrmAtomToClass(XtCString);
}
