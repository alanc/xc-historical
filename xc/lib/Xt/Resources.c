#ifndef lint
static char Xrcsid[] =
    "$XConsortium: Resources.c,v 1.69 89/09/26 10:57:21 swick Exp $";
/* $oHeader: Resources.c,v 1.6 88/09/01 13:39:14 asente Exp $ */
#endif /*lint*/
/*LINTLIBRARY*/

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

#include <stdio.h>
#include "IntrinsicI.h"
#include "Shell.h"
#include "ShellP.h"
#include "StringDefs.h"
static XrmClass	QBoolean, QString, QCallProc, QImmediate;
static XrmName QinitialResourcesPersistent, QInitialResourcesPersistent;

void XtCopyFromParent(widget, offset, value)
    Widget      widget;
    int		offset;
    XrmValue    *value;
{
    if (widget->core.parent == NULL) {
	XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"invalidParent","xtCopyFromParent","XtToolkitError",
                  "CopyFromParent must have non-NULL parent",
		  (String *)NULL, (Cardinal *)NULL);
        value->addr = NULL;
        return;
    }
    value->addr = ((XtPointer)widget->core.parent) + offset;
} /* XtCopyFromParent */

/*ARGSUSED*/
void XtCopyScreen(widget, offset, value)
    Widget      widget;
    int		offset;
    XrmValue    *value;
{
    value->addr = (XtPointer)(&widget->core.screen);
} /* XtCopyScreen */

/*ARGSUSED*/
void XtCopyDefaultColormap(widget, offset, value)
    Widget      widget;
    int		offset;
    XrmValue    *value;
{
    value->addr = (XtPointer)(&DefaultColormapOfScreen(XtScreenOfObject(widget)));
} /* XtCopyDefaultColormap */


/*ARGSUSED*/
void XtCopyAncestorSensitive(widget, offset, value)
    Widget      widget;
    int		offset;
    XrmValue    *value;
{
    static Boolean  sensitive;
	   Widget   parent = widget->core.parent;

    sensitive = (parent->core.ancestor_sensitive & parent->core.sensitive);
    value->addr = (XtPointer)(&sensitive);
} /* XtCopyAncestorSensitive */

/*ARGSUSED*/
void XtCopyDefaultDepth(widget, offset, value)
    Widget      widget;
    int		offset;
    XrmValue    *value;
{
    value->addr = (XtPointer)(&DefaultDepthOfScreen(XtScreenOfObject(widget)));
} /* XtCopyDefaultDepth */

/* If the alignment characteristics of your machine are right, these may be
   faster */

#ifdef UNALIGNED

static void CopyFromArg(src, dst, size)
    XtArgVal src;
    char* dst;
    register unsigned int size;
{
    if	    (size == sizeof(long))	*(long *)dst = (long)src;
    else if (size == sizeof(short))	*(short *)dst = (short)src;
    else if (size == sizeof(char))	*(char *)dst = (char)src;
    else if (size == sizeof(XtPointer))	*(XtPointer *)dst = (XtPointer)src;
    else if (size == sizeof(char*))	*(char **)dst = (char*)src;
    else if (size == sizeof(XtArgVal))	*(XtArgVal *)dst = src;
    else if (size > sizeof(XtArgVal))
	bcopy((char *)  src, (char *) dst, (int) size);
    else
	bcopy((char *) &src, (char *) dst, (int) size);
} /* CopyFromArg */

static void CopyToArg(src, dst, size)
    char* src;
    XtArgVal *dst;
    register unsigned int size;
{
    if (*dst == NULL) {
	/* old GetValues semantics (storing directly into arglists) are bad,
	 * but preserve for compatibility as long as arglist contains NULL.
	 */
        if	(size == sizeof(long))	   *dst = (XtArgVal)*(long*)src;
	else if (size == sizeof(short))    *dst = (XtArgVal)*(short*)src;
	else if (size == sizeof(char))	   *dst = (XtArgVal)*(char*)src;
	else if (size == sizeof(XtPointer)) *dst = (XtArgVal)*(XtPointer*)src;
	else if (size == sizeof(char*))    *dst = (XtArgVal)*(char**)src;
	else if (size == sizeof(XtArgVal)) *dst = *(XtArgVal*)src;
	else bcopy((char*)src, (char*)dst, (int)size);
    }
    else {
	/* proper GetValues semantics: argval is pointer to destination */
	if	(size == sizeof(long))	   *((long*)*dst) = *(long*)src;
	else if (size == sizeof(short))    *((short*)*dst) = *(short*)src;
	else if (size == sizeof(char))	   *((char*)*dst) = *(char*)src;
	else if (size == sizeof(XtPointer)) *((XtPointer*)*dst) = *(XtPointer*)src;
	else if (size == sizeof(char*))    *((char**)*dst) = *(char**)src;
	else if (size == sizeof(XtArgVal)) *((XtArgVal*)*dst)= *(XtArgVal*)src;
	else bcopy((char*)src, (char*)*dst, (int)size);
    }
} /* CopyToArg */

#else
static void CopyFromArg(src, dst, size)
    XtArgVal src;
    char* dst;
    register unsigned int size;
{
    if (size > sizeof(XtArgVal))
	bcopy((char *)  src, (char *) dst, (int) size);
    else {
	union {
	    long	longval;
	    short	shortval;
	    char	charval;
	    char*	charptr;
	    XtPointer	ptr;
	} u;
	char *p = (char*)&u;
	if	(size == sizeof(long))	    u.longval = (long)src;
	else if (size == sizeof(short))	    u.shortval = (short)src;
	else if (size == sizeof(char))	    u.charval = (char)src;
	else if (size == sizeof(XtPointer)) u.ptr = (XtPointer)src;
	else if (size == sizeof(char*))	    u.charptr = (char*)src;
	else				    p = (char*)&src;

	bcopy(p, (char *) dst, (int) size);
    }
} /* CopyFromArg */

static void CopyToArg(src, dst, size)
    char* src;
    XtArgVal *dst;
    register unsigned int size;
{
    if (*dst == NULL) {
	/* old GetValues semantics (storing directly into arglists) are bad,
	 * but preserve for compatibility as long as arglist contains NULL.
	 */
	union {
	    long	longval;
	    short	shortval;
	    char	charval;
	    char*	charptr;
	    XtPointer	ptr;
	} u;
	if (size <= sizeof(XtArgVal)) {
	    bcopy( (char*)src, (char*)&u, (int)size );
	    if	    (size == sizeof(long)) 	*dst = (XtArgVal)u.longval;
	    else if (size == sizeof(short))	*dst = (XtArgVal)u.shortval;
	    else if (size == sizeof(char))	*dst = (XtArgVal)u.charval;
	    else if (size == sizeof(char*))	*dst = (XtArgVal)u.charptr;
	    else if (size == sizeof(XtPointer))	*dst = (XtArgVal)u.ptr;
	    else bcopy( (char*)src, (char*)dst, (int)size );
	}
	else
	    bcopy( (char*)src, (char*)dst, (int)size );
    }
    else {
	/* proper GetValues semantics: argval is pointer to destination */
	bcopy( (char*)src, (char*)*dst, (int)size );
    }
} /* CopyToArg */

#endif

static Cardinal GetNamesAndClasses(w, names, classes)
    register Widget	  w;
    register XrmNameList  names;
    register XrmClassList classes;
{
    register Cardinal length, j;
    register XrmQuark t;
    WidgetClass class;

    /* Return null-terminated quark arrays, with length the number of
       quarks (not including NULL) */

    for (length = 0; w != NULL; w = (Widget) w->core.parent) {
	names[length] = w->core.xrm_name;
	class = XtClass(w);
	/* KLUDGE KLUDGE KLUDGE KLUDGE */
	if (w->core.parent == NULL && 
		XtIsSubclass(w, applicationShellWidgetClass)) {
	    classes[length] =
		((ApplicationShellWidget) w)->application.xrm_class;
	} else classes[length] = class->core_class.xrm_class;
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
    names[length] = NULLQUARK;
    classes[length] = NULLQUARK;
    return length;
} /* GetNamesAndClasses */


/* Spiffy fast compiled form of resource list.				*/
/* XtResourceLists are compiled in-place into XrmResourceLists		*/
/* All atoms are replaced by quarks, and offsets are -offset-1 to	*/
/* indicate that this list has been compiled already			*/

void  XrmCompileResourceList(resources, num_resources)
    register XtResourceList resources;
    	     Cardinal       num_resources;
{
    register Cardinal count;

#define xrmres  ((XrmResourceList) resources)

    for (count = 0; count < num_resources; resources++, count++) {
    	xrmres->xrm_name	 = StringToName(resources->resource_name);
    	xrmres->xrm_class	 = StringToClass(resources->resource_class);
    	xrmres->xrm_type	 = StringToQuark(resources->resource_type);
/*	xrmres->xrm_size	 = resources->resource_size; */
        xrmres->xrm_offset	 = -resources->resource_offset - 1;
    	xrmres->xrm_default_type = StringToQuark(resources->default_type);
/*	xrmres->xrm_default_addr = resources->default_addr; */
    }
#undef xrmres
} /* XrmCompileResourceList */

static void BadType(type, name)
    XrmQuark type, name;
{
    String params[2];
    Cardinal num_params = 2;

    params[0] = XrmQuarkToString(type);
    params[1] = XrmQuarkToString(name);
    XtWarningMsg("invalidTypeOverride", "xtDependencies", "XtToolkitError",
	"Representation type %s must match superclass's to override %s",
	params, &num_params);
} /* BadType */

static void BadSize(size, name)
    Cardinal size;
    XrmQuark name;
{
    String params[2];
    Cardinal num_params = 2;

    params[0] = (String) size;
    params[1] = XrmQuarkToString(name);
    XtWarningMsg("invalidSizeOverride", "xtDependencies", "XtToolkitError",
	"Representation size %d must match superclass's to override %s",
	params, &num_params);
} /* BadType */

/*
 * Create a new resource list, with the class resources following the
 * superclass's resources.  If a resource in the class list overrides
 * a superclass resource, then just replace the superclass entry in place.
 *
 * At the same time, add a level of indirection to the XtResourceList to
 * create and XrmResourceList.
 */
void _XtDependencies(class_resp, class_num_resp, super_res, super_num_res,
		     super_widget_size)
    XtResourceList  *class_resp;	/* VAR */
    Cardinal	    *class_num_resp;    /* VAR */
    XrmResourceList *super_res;
    Cardinal	    super_num_res;
    Cardinal	    super_widget_size;
{
    register XrmResourceList *new_res;
	     Cardinal	     new_num_res;
	     XrmResourceList class_res = (XrmResourceList) *class_resp;
	     Cardinal        class_num_res = *class_num_resp;
    register Cardinal	     i, j;
	     Cardinal        new_next;

    if (class_num_res == 0) {
	/* Just point to superclass resource list */
	*class_resp = (XtResourceList) super_res;
	*class_num_resp = super_num_res;
	return;
    }

    /* Allocate and initialize new_res with superclass resource pointers */
    new_num_res = super_num_res + class_num_res;
    new_res = (XrmResourceList *) XtMalloc(new_num_res*sizeof(XrmResourceList));
    XtBCopy(super_res, new_res, super_num_res * sizeof(XrmResourceList));
    
    /* Put pointers to class resource entries into new_res */
    new_next = super_num_res;
    for (i = 0; i < class_num_res; i++) {
	if (-class_res[i].xrm_offset-1 < super_widget_size) {
	    /* Probably an override of superclass resources--look for overlap */
	    for (j = 0; j < super_num_res; j++) {
		if (class_res[i].xrm_offset == new_res[j]->xrm_offset) {
		    /* Ensure type, size identical to superclass */
		    if (class_res[i].xrm_type != new_res[j]->xrm_type) {
			BadType(class_res[i].xrm_type, class_res[i].xrm_name);
			class_res[i].xrm_type = new_res[j]->xrm_type;
		    }
		    if (class_res[i].xrm_size != new_res[j]->xrm_size) {
			BadSize(class_res[i].xrm_size, class_res[i].xrm_name);
			class_res[i].xrm_size = new_res[j]->xrm_size;
		    }
		    new_res[j] = &(class_res[i]);
		    new_num_res--;
		    goto NextResource;
		}
	    } /* for j */
	}
	/* Not an overlap, add an entry to new_res */
	new_res[new_next++] = &(class_res[i]);
NextResource:;
    } /* for i */

    /* Okay, stuff new resources back into class record */
    *class_resp = (XtResourceList) new_res;
    *class_num_resp = new_num_res;
} /* _XtDependencies */


void _XtResourceDependencies(wc)
    WidgetClass wc;
{
    WidgetClass sc;

    sc = wc->core_class.superclass;
    if (sc == NULL) {
	_XtDependencies(&(wc->core_class.resources),
			&(wc->core_class.num_resources),
			(XrmResourceList *) NULL, (unsigned)0, (unsigned)0);
    } else {
	_XtDependencies(&(wc->core_class.resources),
			&(wc->core_class.num_resources),
			(XrmResourceList *) sc->core_class.resources,
			sc->core_class.num_resources,
			sc->core_class.widget_size);
    }
} /* _XtResourceDependencies */

void _XtConstraintResDependencies(wc)
    ConstraintWidgetClass wc;
{
    ConstraintWidgetClass sc;

    if (wc == (ConstraintWidgetClass) constraintWidgetClass) {
	_XtDependencies(&(wc->constraint_class.resources),
			&(wc->constraint_class.num_resources),
			(XrmResourceList *)NULL, (unsigned)0, (unsigned)0);
    } else {
	sc = (ConstraintWidgetClass) wc->core_class.superclass;
	_XtDependencies(&(wc->constraint_class.resources),
			&(wc->constraint_class.num_resources),
			(XrmResourceList *) sc->constraint_class.resources,
			sc->constraint_class.num_resources,
			sc->constraint_class.constraint_size);
    }
} /* _XtConstraintResDependencies */



    
static XrmResourceList* CreateIndirectionTable (resources, num_resources)
    XtResourceList  resources;
    Cardinal	    num_resources;
{
    register int index;
    XrmResourceList* table;

    table = (XrmResourceList*)XtMalloc(num_resources * sizeof(XrmResourceList));
    for (index = 0; index < num_resources; index++)
        table[index] = (XrmResourceList)(&(resources[index]));
    return table;
}

static XtCacheRef *GetResources(widget, base, names, classes,
	table, num_resources, quark_args, args, num_args)
    Widget	    widget;	    /* Widget resources are associated with */
    char*	    base;	    /* Base address of memory to write to   */
    XrmNameList     names;	    /* Full inheritance name of widget      */
    XrmClassList    classes;	    /* Full inheritance class of widget     */
    XrmResourceList*  table;	    /* The list of resources required.      */
    Cardinal	    num_resources;  /* number of items in resource list     */
    XrmQuarkList    quark_args;     /* Arg names quarkified		    */
    ArgList	    args;	    /* ArgList to override resources	    */
    Cardinal	    num_args;       /* number of items in arg list	    */
{
#define SEARCHLISTLEN 100

    XrmValue	    value;
    XrmQuark	    rawType;
    XrmValue	    rawValue;
    XrmHashTable    stackSearchList[SEARCHLISTLEN];
    XrmHashTable    *searchList = stackSearchList;
    unsigned int    searchListSize = SEARCHLISTLEN;
    Bool            status;
    Boolean	    found[400];
    XtCacheRef	    cache_ref[400];
    int		    cache_ref_size = 0;
    Display	    *dpy;
    Boolean	    persistent_resources = True;
    Boolean	    found_persistence = False;

    if ((args == NULL) && (num_args != 0)) {
    	XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"invalidArgCount","getResources","XtToolkitError",
                 "argument count > 0 on NULL argument list",
                   (String *)NULL, (Cardinal *)NULL);
	num_args = 0;
    }
    if (num_resources == 0) {
	return NULL;
    } else if (table == NULL) {
    	XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		"invalidResourceCount","getResources","XtToolkitError",
              "resource count > 0 on NULL resource list",
	      (String *)NULL, (Cardinal *)NULL);
	return NULL;
    }

    /* Mark each resource as not found on arg list */
    bzero((char *) found, (int) (num_resources * sizeof(Boolean)));

    dpy = XtDisplayOfObject(widget);
    
    /* Copy the args into the resources, mark each as found */
    {
	register ArgList	    arg;
	register XrmName	    argName;
	register int	    j;
	register int	    i;
	register XrmResourceList rx;
	register XrmResourceList *res;
	for (arg = args, i = 0; i < num_args; i++, arg++) {
	    argName = quark_args[i];
	    if (argName == QinitialResourcesPersistent) {
		persistent_resources = (Boolean)arg->value;
		found_persistence = True;
		break;
	    }
	    for (j = 0, res = table; j < num_resources; j++, res++) {
		rx = *res;
		if (argName == rx->xrm_name) {
		    CopyFromArg(
			arg->value,
			base - rx->xrm_offset - 1,
			rx->xrm_size);
		    found[j] = TRUE;
		    break;
		}
	    }
	}
    }

    /* Ask resource manager for a list of database levels that we can
       do a single-level search on each resource */

    status = XrmQGetSearchList(dpy->db, names, classes,
			       searchList, searchListSize);

    if (!status) {
	searchList = NULL;
	do {
	    searchList = (XrmHashTable*)
		XtRealloc((char*)searchList,
			  sizeof(XrmHashTable) * (searchListSize *= 2));
	    status = XrmQGetSearchList(dpy->db, names, classes,
				       searchList, searchListSize);
	} while (!status);
    }


    
    /* go to the resource manager for those resources not found yet */
    /* if it's not in the resource database use the default value   */

    {
	register XrmResourceList  rx;
	register XrmResourceList  *res;
	register int		  j;
	register XrmValue	  *pv = &value;
	register XrmRepresentation xrm_type;
	register XrmRepresentation xrm_default_type;
	char	char_val;
	short	short_val;
	int	int_val;
	long	long_val;
	char*	char_ptr;

	if (!found_persistence) {
	    if (XrmQGetSearchResource(searchList, QinitialResourcesPersistent,
			QInitialResourcesPersistent, &rawType, &value)) {
		if (rawType != QBoolean) {
		    rawValue = value;
		    value.size = sizeof(Boolean);
		    value.addr = &persistent_resources;
		    if (!_XtConvert(widget, rawType, &rawValue, QBoolean,
				    &value, NULL))
			persistent_resources = *(Boolean*)value.addr;
		}
		else
		    persistent_resources = *(Boolean*)value.addr;
	    }
	}
	for (res = table, j = 0; j < num_resources; j++, res++) {
	    if (! found[j]) {
		Boolean	already_copied = False;
		Boolean have_value = False;
		rx = *res;
		xrm_type = rx->xrm_type;
		if (XrmQGetSearchResource(searchList,
			rx->xrm_name, rx->xrm_class, &rawType, &value)) {
		    if (rawType != xrm_type) {
			rawValue = *pv;
			value.size = rx->xrm_size;
			value.addr = (XtPointer)(base - rx->xrm_offset - 1);
			already_copied = have_value =
			    _XtConvert(widget, rawType, &rawValue,
				       xrm_type, &value,
				       persistent_resources ?
				          NULL : &cache_ref[cache_ref_size]);
			if ((persistent_resources == False)
			  && cache_ref[cache_ref_size] != NULL)
			    cache_ref_size++;
		    } else have_value = True;
		}
		if (!have_value
		    && ((rx->xrm_default_type == QImmediate)
			|| (rx->xrm_default_addr != NULL))) {
		    /* Convert default value to proper type */
		    xrm_default_type = rx->xrm_default_type;
		    if (xrm_default_type == QCallProc) {
			(*(XtProc)(rx->xrm_default_addr))(
			      widget,-(rx->xrm_offset+1), pv);
		    } else if (xrm_default_type == QImmediate) {
			if (rx->xrm_size == sizeof(int)) {
			    int_val = (int)rx->xrm_default_addr;
			    pv->addr = (XtPointer) &int_val;
			} else if (rx->xrm_size == sizeof(short)) {
			    short_val = (short)rx->xrm_default_addr;
			    pv->addr = (XtPointer) &short_val;
			} else if (rx->xrm_size == sizeof(char)) {
			    char_val = (char)rx->xrm_default_addr;
			    pv->addr = (XtPointer) &char_val;
			} else if (rx->xrm_size == sizeof(long)) {
			    long_val = (long)rx->xrm_default_addr;
			    pv->addr = (XtPointer) &long_val;
			} else if (rx->xrm_size == sizeof(char*)) {
			    char_ptr = (char*)rx->xrm_default_addr;
			    pv->addr = (XtPointer) &char_ptr;
			} else {
			    pv->addr = (XtPointer) &(rx->xrm_default_addr);
			}
		    } else if (xrm_default_type == xrm_type) {
			pv->addr = rx->xrm_default_addr;
		    } else {
			rawValue.addr = rx->xrm_default_addr;
			if (xrm_default_type == QString) {
			    rawValue.size = strlen((char *)rawValue.addr) + 1;
			} else {
			    rawValue.size = sizeof(XtPointer);
			}
			value.size = rx->xrm_size;
			value.addr = (XtPointer)(base - rx->xrm_offset - 1);
			already_copied =
			    _XtConvert(widget, xrm_default_type,
				       &rawValue, xrm_type, &value,
				       &cache_ref[cache_ref_size]);
			if (cache_ref[cache_ref_size] != NULL)
			    cache_ref_size++;
		    }
		}
		if (!already_copied) {
		    if (pv->addr != NULL) {
			if (xrm_type == QString) {
			    *((String*)(base - rx->xrm_offset - 1)) = pv->addr;
			} else {
			    XtBCopy(pv->addr, base - rx->xrm_offset - 1,
				    rx->xrm_size);
			}
		    } else {
			/* didn't get value, initialize to NULL... */
			XtBZero(base - rx->xrm_offset - 1, rx->xrm_size);
		    }
		}
	    }
	}
    }
    if (searchList != stackSearchList) XtFree((char*)searchList);
    if (cache_ref_size > 0) {
	XtCacheRef *refs = (XtCacheRef*)
	    XtMalloc((unsigned)sizeof(XtCacheRef)*(cache_ref_size + 1));
	bcopy( cache_ref, refs, sizeof(XtCacheRef)*cache_ref_size );
	refs[cache_ref_size] = NULL;
	return refs;
    }
    else return (XtCacheRef*)NULL;
}



static void CacheArgs(args, num_args, quark_cache, num_quarks, pQuarks)
    ArgList	    args;
    Cardinal	    num_args;
    XrmQuarkList    quark_cache;
    Cardinal	    num_quarks;
    XrmQuarkList    *pQuarks;       /* RETURN */
{
    register XrmQuarkList   quarks;
    register Cardinal       i;

    if (num_quarks < num_args) {
	quarks = (XrmQuarkList) XtMalloc(num_args * sizeof(XrmQuark));
    } else {
	quarks = quark_cache;
    }
    *pQuarks = quarks;

    for (i = 0; i < num_args; i++) {
	quarks[i] = StringToQuark(args[i].name);
    }
}

#define FreeCache(cache, pointer) \
	  if (cache != pointer) XtFree((char *)pointer)


XtCacheRef *_XtGetResources(w, args, num_args)
    register 	Widget	  w;
    		ArgList	  args;
    		Cardinal  num_args;
{
    XrmName	    names[100];
    XrmClass	    classes[100];
    XrmQuark	    quark_cache[100];
    XrmQuarkList    quark_args;
    WidgetClass     wc;
    ConstraintWidgetClass   cwc;
    XtCacheRef	    *cache_refs;

    wc = XtClass(w);

    /* Make sure xrm_class is valid */
    /* ||| Class quarkifying should be part of Core.c */
    if (wc->core_class.xrm_class == NULLQUARK) {
        wc->core_class.xrm_class = StringToClass(wc->core_class.class_name);
    }

    /* Get names, classes for widget and ancestors */
    (void) GetNamesAndClasses(w, names, classes);
   
    /* Compile arg list into quarks */
    CacheArgs(args, num_args, quark_cache, XtNumber(quark_cache), &quark_args);
    /* Get normal resources */
    cache_refs = GetResources(w, (char*)w, names, classes,
	(XrmResourceList *) wc->core_class.resources,
	wc->core_class.num_resources, quark_args, args, num_args);
    if (w->core.constraints != NULL) {
	cwc = (ConstraintWidgetClass) XtClass(w->core.parent);
	GetResources(w, (char*)w->core.constraints, names, classes,
	    (XrmResourceList *) cwc->constraint_class.resources,
	    cwc->constraint_class.num_resources,
	    quark_args, args, num_args);
    }
    FreeCache(quark_cache, quark_args);
    return cache_refs;
} /* XtGetResources */


void XtGetSubresources
	(w, base, name, class, resources, num_resources, args, num_args)
    Widget	  w;		  /* Widget "parent" of subobject   */
    XtPointer	  base;		  /* Base address to write to       */
    String	  name;		  /* name of subobject		    */
    String	  class;	  /* class of subobject		    */
    XtResourceList resources;	  /* resource list for subobject    */
    Cardinal	  num_resources;
    ArgList	  args;		  /* arg list to override resources */
    Cardinal	  num_args;
{
    XrmName	  names[100];
    XrmClass	  classes[100];
    register Cardinal	  length;
    XrmQuark	  quark_cache[100];
    XrmQuarkList  quark_args;
    XrmResourceList* table;

    if (num_resources == 0) return;

    /* Get full name, class of subobject */
    length = GetNamesAndClasses(w, names, classes);
    names[length] = StringToName(name);
    classes[length] = StringToClass(class);
    length++;
    names[length] = NULLQUARK;
    classes[length] = NULLQUARK;

    /* Compile arg list into quarks */
    CacheArgs(args, num_args, quark_cache, XtNumber(quark_cache), &quark_args);
    /* Compile resource list if needed */
    if (((int) resources->resource_offset) >= 0) {
	XrmCompileResourceList(resources, num_resources);
    }
    table = CreateIndirectionTable(resources, num_resources); 
    (void) GetResources(w, (char*)base, names, classes,
        table, num_resources, quark_args, args, num_args);
    FreeCache(quark_cache, quark_args);
    XtFree((char *)table);
}


void XtGetApplicationResources
	(w, base, resources, num_resources, args, num_args)
    Widget	    w;		  /* Application shell widget       */
    XtPointer	    base;	  /* Base address to write to       */
    XtResourceList  resources;	  /* resource list for subobject    */
    Cardinal	    num_resources;
    ArgList	    args;	  /* arg list to override resources */
    Cardinal	    num_args;
{
    XrmName	    names[100];
    XrmClass	    classes[100];
    XrmQuark	    quark_cache[100];
    XrmQuarkList    quark_args;
    XrmResourceList* table;

    if (num_resources == 0) return;

    /* Get full name, class of application */
    if (w == NULL) {
	/* hack for R2 compatibility */
	XtPerDisplay pd = _XtGetPerDisplay(_XtDefaultAppContext()->list[0]);
	names[0] = pd->name;
	names[1] = NULLQUARK;
	classes[0] = pd->class;
	classes[1] = NULLQUARK;
    }
    else {
	(void) GetNamesAndClasses(w, names, classes);
    }

    /* Compile arg list into quarks */
    CacheArgs(args, num_args, quark_cache, XtNumber(quark_cache), &quark_args);
    /* Compile resource list if needed */
    if (((int) resources->resource_offset) >= 0) {
	XrmCompileResourceList(resources, num_resources);
    }
    table = CreateIndirectionTable(resources,num_resources);

    (void) GetResources(w, (char*)base, names, classes,
        table, num_resources, quark_args, args, num_args);
    FreeCache(quark_cache, quark_args);
    XtFree((char *)table);
}


static void GetValues(base, res, num_resources, args, num_args)
  char*			base;		/* Base address to fetch values from */
  XrmResourceList*      res;		/* The current resource values.      */
  register Cardinal	num_resources;	/* number of items in resources      */
  ArgList 		args;		/* The resource values requested     */
  Cardinal		num_args;	/* number of items in arg list       */
{
    register ArgList		arg;
    register int 		i;
    register XrmName		argName;
    register XrmResourceList*   xrmres;
    register XrmQuark		QCallback = XrmStringToQuark(XtRCallback);
    extern XtCallbackList	_XtGetCallbackList();

    /* Resource lists should be in compiled form already  */

    for (arg = args ; num_args != 0; num_args--, arg++) {
	argName = StringToName(arg->name);
	for (xrmres = res, i = 0; i < num_resources; i++, xrmres++) {
	    if (argName == (*xrmres)->xrm_name) {
		if ((*xrmres)->xrm_type == QCallback) {
		    /* hack; do this here instead of a get_values_hook
		     * because get_values_hook looses info as to
		     * whether arg->value == NULL for CopyToArg.
		     * It helps performance, too...
		     */
		    XtCallbackList callback = _XtGetCallbackList(
			      base - (*xrmres)->xrm_offset - 1);
		    CopyToArg(
			      (char*)&callback, &arg->value,
			      (*xrmres)->xrm_size);
		}
		else {
		    CopyToArg(
			      base - (*xrmres)->xrm_offset - 1,
			      &arg->value,
			      (*xrmres)->xrm_size);
		}
		break;
	    }
	}
    }
} /* GetValues */

static void CallGetValuesHook(widget_class, w, args, num_args)
    WidgetClass	  widget_class;
    Widget	  w;
    ArgList	  args;
    Cardinal	  num_args;
{
    if (widget_class->core_class.superclass != NULL) {
	CallGetValuesHook
	    (widget_class->core_class.superclass, w, args, num_args);
    }
    if (widget_class->core_class.get_values_hook != NULL) {
	(*(widget_class->core_class.get_values_hook)) (w, args, &num_args);
    }
}



static void CallConstraintGetValuesHook(widget_class, w, args, num_args)
    WidgetClass	  widget_class;
    Widget	  w;
    ArgList	  args;
    Cardinal	  num_args;
{
    ConstraintClassExtension ext;

    if (widget_class->core_class.superclass
	->core_class.class_inited & ConstraintClassFlag) {
	CallConstraintGetValuesHook
	    (widget_class->core_class.superclass, w, args, num_args);
    }

    for (ext = (ConstraintClassExtension)((ConstraintWidgetClass)widget_class)
		 ->constraint_class.extension;
	 ext != NULL && ext->record_type != NULLQUARK;
	 ext = (ConstraintClassExtension)ext->next_extension);

    if (ext != NULL) {
	if (  ext->version == XtConstraintExtensionVersion
	      && ext->record_size == sizeof(ConstraintClassExtensionRec)) {
	    if (ext->get_values_hook != NULL)
		(*(ext->get_values_hook)) (w, args, &num_args);
	} else {
	    String params[1];
	    Cardinal num_params = 1;
	    params[0] = widget_class->core_class.class_name;
	    XtAppWarningMsg(XtWidgetToApplicationContext(w),
		 "invalidExtension", "xtCreateWidget", "XtToolkitError",
		 "widget class %s has invalid ConstraintClassExtension record",
		 params, &num_params);
	}
    }
}


void XtGetValues(w, args, num_args)
    register Widget   w;
    register ArgList  args;
    register Cardinal num_args;
{
    WidgetClass wc = XtClass(w);

    if (num_args == 0) return;
    if ((args == NULL) && (num_args != 0)) {
	XtAppErrorMsg(XtWidgetToApplicationContext(w),
		"invalidArgCount","xtGetValues","XtToolkitError",
            "Argument count > 0 on NULL argument list in XtGetValues",
              (String *)NULL, (Cardinal *)NULL);
    }
    /* Get widget values */
    GetValues((char*)w, (XrmResourceList *) wc->core_class.resources,
	wc->core_class.num_resources, args, num_args);

    /* Get constraint values if necessary */
    /* if (!XtIsShell(w) && XtIsConstraint(w->core.parent)) */
    if (w->core.constraints != NULL) {
	ConstraintWidgetClass cwc
	    = (ConstraintWidgetClass) XtClass(w->core.parent);
	GetValues((char*)w->core.constraints, 
	    (XrmResourceList *)(cwc->constraint_class.resources),
	    cwc->constraint_class.num_resources, args, num_args);
    }
    /* Notify any class procedures that we have performed get_values */
    CallGetValuesHook(wc, w, args, num_args);

    /* Notify constraint get_values if necessary */
    /* if (!XtIsShell(w) && XtIsConstraint(w->core.parent)) */
    if (w->core.constraints != NULL)
	CallConstraintGetValuesHook(XtClass(w->core.parent), w, args,num_args);
} /* XtGetValues */

void XtGetSubvalues(base, resources, num_resources, args, num_args)
  XtPointer	    base;           /* Base address to fetch values from */
  XtResourceList    resources;      /* The current resource values.      */
  Cardinal	    num_resources;  /* number of items in resources      */
  ArgList	    args;           /* The resource values requested     */
  Cardinal	    num_args;       /* number of items in arg list       */
{
      XrmResourceList* xrmres;
      xrmres = CreateIndirectionTable(resources, num_resources);
      GetValues((char*)base, xrmres, num_resources, args, num_args);
      XtFree((char *)xrmres);
}


static void SetValues(base, res, num_resources, args, num_args)
  char*			base;		/* Base address to write values to   */
  XrmResourceList*	res;		/* The current resource values.      */
  register Cardinal	num_resources;	/* number of items in resources      */
  ArgList 		args;		/* The resource values to set        */
  Cardinal		num_args;	/* number of items in arg list       */
{
    register ArgList		arg;
    register int 	        i;
    register XrmName		argName;
    register XrmResourceList*   xrmres;

    /* Resource lists are assumed to be in compiled form already via the
       initial XtGetResources, XtGetSubresources calls */

    for (arg = args ; num_args != 0; num_args--, arg++) {
	argName = StringToName(arg->name);
	for (xrmres = res, i = 0; i < num_resources; i++, xrmres++) {
	    if (argName == (*xrmres)->xrm_name) {
		CopyFromArg(arg->value,
		    base - (*xrmres)->xrm_offset - 1,
		    (*xrmres)->xrm_size);
		break;
	    }
	}
    }
} /* SetValues */

static Boolean CallSetValues (class, current, request, new, args, num_args)
    WidgetClass class;
    Widget      current, request, new;
    ArgList     args;
    Cardinal    num_args;
{
    Boolean redisplay = FALSE;

    if (class->core_class.superclass != NULL)
        redisplay = CallSetValues(
	  class->core_class.superclass, current, request, new, args, num_args);
    if (class->core_class.set_values != NULL)
        redisplay |= (*class->core_class.
		      set_values) (current, request, new, args, &num_args);
    if (class->core_class.set_values_hook != NULL)
	redisplay |=
	    (*class->core_class.set_values_hook) (new, args, &num_args);
    return (redisplay);
}

static Boolean
CallConstraintSetValues (class, current, request, new, args, num_args)
    ConstraintWidgetClass class;
    Widget      current, request, new;
    ArgList     args;
    Cardinal    num_args;
{
    Boolean redisplay = FALSE;

    if ((WidgetClass)class != constraintWidgetClass) {
	if (class == NULL)
	    XtAppErrorMsg(XtWidgetToApplicationContext(current),
		    "invalidClass","constraintSetValue","XtToolkitError",
                 "Subclass of Constraint required in CallConstraintSetValues",
                  (String *)NULL, (Cardinal *)NULL);
	redisplay = CallConstraintSetValues(
	    (ConstraintWidgetClass) (class->core_class.superclass),
	    current, request, new, args, num_args);
    }
    if (class->constraint_class.set_values != NULL)
        redisplay |= (*class->constraint_class.
		      set_values) (current, request, new, args, &num_args);
    return (redisplay);
}

void XtSetSubvalues(base, resources, num_resources, args, num_args)
  XtPointer             base;           /* Base address to write values to   */
  register XtResourceList resources;    /* The current resource values.      */
  register Cardinal     num_resources;  /* number of items in resources      */
  ArgList               args;           /* The resource values to set        */
  Cardinal              num_args;       /* number of items in arg list       */
{
      register XrmResourceList*   xrmres;
      xrmres = CreateIndirectionTable (resources, num_resources);
      SetValues((char*)base,xrmres,num_resources, args, num_args);
      XtFree((char *)xrmres);
}


void XtSetValues(w, args, num_args)
    register Widget   w;
	     ArgList  args;
	     Cardinal num_args;
{
    register Widget oldw, reqw;
    char	    oldwCache[500], reqwCache[500];
    char	    oldcCache[100], reqcCache[100];
    Cardinal	    widgetSize, constraintSize;
    Boolean	    redisplay, reconfigured = False;
    XtGeometryResult result;
    XtWidgetGeometry geoReq, geoReply;
    WidgetClass     wc = XtClass(w);
    ConstraintWidgetClass cwc;

    if ((args == NULL) && (num_args != 0)) {
        XtAppErrorMsg(XtWidgetToApplicationContext(w),
		"invalidArgCount","xtSetValues","XtToolkitError",
                "Argument count > 0 on NULL argument list in XtSetValues",
                 (String *)NULL, (Cardinal *)NULL);
    }

    /* Allocate and copy current widget into old widget */

    widgetSize = wc->core_class.widget_size;
    oldw = (Widget) XtStackAlloc(widgetSize, oldwCache);
    reqw = (Widget) XtStackAlloc (widgetSize, reqwCache);
    bcopy((char *) w, (char *) oldw, (int) widgetSize);

    /* Set resource values */

    SetValues((char*)w, (XrmResourceList *) wc->core_class.resources,
	wc->core_class.num_resources, args, num_args);

    bcopy ((char *) w, (char *) reqw, (int) widgetSize);

    if (w->core.constraints != NULL) {
	/* Allocate and copy current constraints into oldw */
	cwc = (ConstraintWidgetClass) XtClass(w->core.parent);
	constraintSize = cwc->constraint_class.constraint_size;
	oldw->core.constraints = XtStackAlloc(constraintSize, oldcCache);
	reqw->core.constraints = XtStackAlloc(constraintSize, reqcCache);
	bcopy((char *) w->core.constraints, 
		(char *) oldw->core.constraints, (int) constraintSize);

	/* Set constraint values */
	SetValues((char*)w->core.constraints,
	    (XrmResourceList *)(cwc->constraint_class.resources),
	    cwc->constraint_class.num_resources, args, num_args);
	bcopy((char *) w->core.constraints,
	      (char *) reqw->core.constraints, (int) constraintSize);
    }

    /* Inform widget of changes, then inform parent of changes */
    redisplay = CallSetValues (wc, oldw, reqw, w, args, num_args);
    if (w->core.constraints != NULL) {
	redisplay |= CallConstraintSetValues(cwc, oldw, reqw, w, args, num_args);
    }

    if (XtIsRectObj(w)) {
	/* Now perform geometry request if needed */
	geoReq.request_mode = 0;
	if (oldw->core.x	!= w->core.x) {
	    geoReq.x		= w->core.x;
	    w->core.x		= oldw->core.x;
	    geoReq.request_mode |= CWX;
	}
	if (oldw->core.y	!= w->core.y) {
	    geoReq.y		= w->core.y;
	    w->core.y		= oldw->core.y;
	    geoReq.request_mode |= CWY;
	}
	if (oldw->core.width	!= w->core.width) {
	    geoReq.width	= w->core.width;
	    w->core.width	= oldw->core.width;
	    geoReq.request_mode |= CWWidth;
	}
	if (oldw->core.height	!= w->core.height) {
	    geoReq.height	= w->core.height;
	    w->core.height	= oldw->core.height;
	    geoReq.request_mode |= CWHeight;
	}
	if (oldw->core.border_width != w->core.border_width) {
	    geoReq.border_width	    = w->core.border_width;
	    w->core.border_width    = oldw->core.border_width;
	    geoReq.request_mode	    |= CWBorderWidth;
	}
    
	if (geoReq.request_mode != 0) {
	    do {
		result = XtMakeGeometryRequest(w, &geoReq, &geoReply);
		if (result == XtGeometryYes) {
		    reconfigured = True;
		    break;
		}
		/* An Almost or No reply.  Call widget and let it munge
		   request, reply */
		if (wc->core_class.set_values_almost == NULL) {
		    XtAppWarningMsg(XtWidgetToApplicationContext(w),
			    "invalidProcedure","set_values_almost",
			  "XtToolkitError",
			  "set_values_almost procedure shouldn't be NULL",
			  (String *)NULL, (Cardinal *)NULL);
		    break;
		}
		(*(wc->core_class.set_values_almost))
		    (oldw, w, &geoReq, &geoReply);
	    } while (geoReq.request_mode != 0);
	    /* call resize proc if we changed size */
	    if (reconfigured
		&& (geoReq.request_mode & (CWWidth | CWHeight))
		&& wc->core_class.resize != (XtWidgetProc) NULL) {
		(*(wc->core_class.resize))(w);
	    }
	}
	/* Redisplay if needed */
        if (XtIsWidget(w)) {
            /* widgets can distinguish between redisplay and resize, since
             the server will cause an expose on resize */
            if (redisplay && XtIsRealized(w))
                XClearArea (XtDisplay(w), XtWindow(w), 0, 0, 0, 0, TRUE);
        }else { /*non-window object */
        if ((redisplay || reconfigured) && XtIsManaged (w)) {
            Widget pw = w;
            RectObj r = (RectObj) oldw;
            while ((pw!=NULL) && ( ! XtIsWidget(pw) ))
                pw = pw->core.parent;
            if ((pw!=NULL) && XtIsRealized (pw)) {
                int bw2 = r->rectangle.border_width << 1;
                XClearArea (XtDisplay (pw), XtWindow (pw),
                    r->rectangle.x,r->rectangle.y,
                    r->rectangle.width + bw2,r->rectangle.height + bw2,TRUE);
                if (reconfigured) {
                    r = (RectObj) w;
                    bw2 = r->rectangle.border_width << 1;
                    XClearArea (XtDisplay (pw), XtWindow (pw),
                        r->rectangle.x,r->rectangle.y,
                        r->rectangle.width + bw2,r->rectangle.height + bw2,
                        TRUE);
                }
            }
        }
        }
    }


    /* Free dynamic storage */
    if (w->core.constraints != NULL) {
        XtStackFree(oldw->core.constraints, oldcCache);
        XtStackFree(reqw->core.constraints,
        reqcCache);
    }
    XtStackFree((XtPointer)oldw, oldwCache);
    XtStackFree((XtPointer)reqw, reqwCache);

} /* XtSetValues */
 
void XtGetResourceList(widget_class, resources, num_resources)
	WidgetClass widget_class;
	XtResourceList *resources;
	Cardinal *num_resources;
{
	int size = widget_class->core_class.num_resources * sizeof(XtResource);
	register int i, dest = 0;
	register XtResourceList *list, dlist;

	*resources = (XtResourceList) XtMalloc((unsigned) size);

	if (!widget_class->core_class.class_inited) {
	    /* Easy case */

	    bcopy((char *)widget_class->core_class.resources,
		    (char *) *resources, size);
	    *num_resources = widget_class->core_class.num_resources;
	    return;
	}

	/* Nope, it's the hard case */

	list = (XtResourceList *) widget_class->core_class.resources;
	dlist = *resources;
	for (i = 0; i < widget_class->core_class.num_resources; i++) {
	    if (list[i] != NULL) {
		dlist[dest].resource_name = (String)
			XrmQuarkToString((XrmQuark) list[i]->resource_name);
		dlist[dest].resource_class = (String) 
			XrmQuarkToString((XrmQuark) list[i]->resource_class);
		dlist[dest].resource_type = (String)
			XrmQuarkToString((XrmQuark) list[i]->resource_type);
		dlist[dest].resource_size = list[i]->resource_size;
		dlist[dest].resource_offset = -(list[i]->resource_offset + 1);
		dlist[dest].default_type = (String)
			XrmQuarkToString((XrmQuark) list[i]->default_type);
		dlist[dest].default_addr = list[i]->default_addr;
		dest++;
	    }
	}
	*num_resources = dest;
}


static Boolean ClassIsSubclassOf(class, superclass)
    WidgetClass class, superclass;
{
    for (; class != NULL; class = class->core_class.superclass) {
	if (class == superclass) return True;
    }
    return False;
}

void XtGetConstraintResourceList(widget_class, resources, num_resources)
	WidgetClass widget_class;
	XtResourceList *resources;
	Cardinal *num_resources;
{
	int size;
	register int i, dest = 0;
	register XtResourceList *list, dlist;
	ConstraintWidgetClass class = (ConstraintWidgetClass)widget_class;

	if (   (class->core_class.class_inited &&
		!(class->core_class.class_inited & ConstraintClassFlag))
	    || (!class->core_class.class_inited &&
		!ClassIsSubclassOf(widget_class, constraintWidgetClass))
	    || class->constraint_class.num_resources == 0) {

	    *resources = NULL;
	    *num_resources = 0;
	    return;
	}

	size = class->constraint_class.num_resources * sizeof(XtResource);
	*resources = (XtResourceList) XtMalloc((unsigned) size);

	if (!class->core_class.class_inited) {
	    /* Easy case */

	    bcopy((char *)class->constraint_class.resources,
		    (char *) *resources, size);
	    *num_resources = class->constraint_class.num_resources;
	    return;
	}

	/* Nope, it's the hard case */

	list = (XtResourceList *) class->constraint_class.resources;
	dlist = *resources;
	for (i = 0; i < class->constraint_class.num_resources; i++) {
	    if (list[i] != NULL) {
		dlist[dest].resource_name = (String)
			XrmQuarkToString((XrmQuark) list[i]->resource_name);
		dlist[dest].resource_class = (String) 
			XrmQuarkToString((XrmQuark) list[i]->resource_class);
		dlist[dest].resource_type = (String)
			XrmQuarkToString((XrmQuark) list[i]->resource_type);
		dlist[dest].resource_size = list[i]->resource_size;
		dlist[dest].resource_offset = -(list[i]->resource_offset + 1);
		dlist[dest].default_type = (String)
			XrmQuarkToString((XrmQuark) list[i]->default_type);
		dlist[dest].default_addr = list[i]->default_addr;
		dest++;
	    }
	}
	*num_resources = dest;
}


static Boolean initialized = FALSE;

void _XtResourceListInitialize()
{
    if (initialized) {
	XtWarningMsg("initializationError","xtInitialize","XtToolkitError",
                  "Initializing Resource Lists twice",
		  (String *)NULL, (Cardinal *)NULL);
    	return;
    }
    initialized = TRUE;

    QBoolean = StringToClass(XtCBoolean);
    QString = StringToClass(XtCString);
    QCallProc = XrmStringToRepresentation(XtRCallProc);
    QImmediate = XrmStringToRepresentation(XtRImmediate);
    QinitialResourcesPersistent = StringToName(XtNinitialResourcesPersistent);
    QInitialResourcesPersistent = StringToName(XtCInitialResourcesPersistent);
}

