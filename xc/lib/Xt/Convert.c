#ifndef lint
static char Xrcsid[] = "$XConsortium: Convert.c,v 1.1 89/06/01 14:33:56 swick Exp $";
/* $oHeader: Convert.c,v 1.4 88/09/01 11:10:44 asente Exp $ */
#endif lint
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

#include	"StringDefs.h"
#include	"IntrinsicI.h"
#include	"Quarks.h"

#ifdef DEBUG
#include	<stdio.h>
#endif

/* Conversion procedure hash 	table */

typedef struct _ConverterRec *ConverterPtr;

typedef struct _ConverterRec {
    ConverterPtr	next;
    XrmRepresentation	from, to;
    XtNewConverter	converter;
    XtDestructor	destructor;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
    Boolean		new_style;
    Boolean		use_cache;
} ConverterRec;

#define CONVERTHASHSIZE	512
#define CONVERTHASHMASK	511
#define ProcHash(from_type, to_type) (2 * (from_type) + to_type)

void _XtSetDefaultConverterTable(table)
	ConverterTable *table;
{
    register ConverterTable globalConverterTable =
	_XtGetProcessContext()->globalConverterTable;

    *table = (ConverterTable) XtCalloc(CONVERTHASHSIZE, sizeof(ConverterPtr));
    _XtAddDefaultConverters(*table);

    if (globalConverterTable != (ConverterTable)NULL) {
	ConverterPtr rec;
	int i;
	for (i = CONVERTHASHSIZE; i; i--, globalConverterTable++) {
	    for (rec = *globalConverterTable; rec != NULL; rec = rec->next)
	       _XtTableAddConverter(*table, rec->from, rec->to, rec->converter,
				    rec->convert_args, rec->num_args,
				    rec->destructor,
				    rec->new_style, rec->use_cache);
  	}
    }
}

void _XtFreeConverterTable(table)
	ConverterTable table;
{
	register int i;
	register ConverterPtr p;

	for (i = 0; i < CONVERTHASHSIZE; i++) {
	    for (p = table[i]; p != NULL;) {
		register ConverterPtr next = p->next;
		XtFree(p);
		p = next;
	    }
	}
	XtFree(table);
}	

/* Data cache hash table */

typedef struct _CacheRec *CachePtr;

typedef struct _CacheRec {
    CachePtr	next;		/* must remain first */
    CachePtr	prev;
    caddr_t	tag;
    int		hash;
    XtNewConverter converter;
    XtDestructor   destructor;
    XrmValue	*args;
    Cardinal	num_args;
    XrmValue	from;
    XrmValue	to;
    caddr_t	closure;
    long	ref_count;
    Boolean	conversion_succeeded;
} CacheRec;

#define CACHEHASHSIZE	256
#define CACHEHASHMASK	255
typedef CachePtr CacheHashTable[CACHEHASHSIZE];

static CacheHashTable	cacheHashTable;

void _XtTableAddConverter(table, from_type, to_type, converter, convert_args, num_args, destructor, new_style, use_cache)
    ConverterTable	table;
    XrmRepresentation   from_type, to_type;
    XtNewConverter	converter;
    XtDestructor	destructor;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
    Boolean		new_style;
    Boolean		use_cache;
{
    register ConverterPtr	*pHashEntry;
    register ConverterPtr	p;
    XtDestructor nullProc = NULL; /* some compilers broken */

    pHashEntry= &table[ProcHash(from_type, to_type) & CONVERTHASHMASK];
    for (p = *pHashEntry; p != NULL; p = p->next) {
	if (p->from == from_type && p->to == to_type) break;
    }

    if (p == NULL || p->destructor != nullProc) {
	/* have to keep old record to call destructors; put new one first */
	p = (ConverterPtr) XtMalloc(sizeof(ConverterRec));
	p->next	    = *pHashEntry;
	*pHashEntry     = p;
	p->from	    = from_type;
	p->to	    = to_type;
    }
    p->converter    = converter;
    p->destructor   = destructor;
    p->convert_args = convert_args;
    p->num_args     = num_args;
    p->new_style    = new_style;
    p->use_cache    = use_cache;
}

void XtSetConverter(from_type, to_type, converter, convert_args, num_args, cached, destructor)
    register String	from_type, to_type;
    XtNewConverter	converter;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
    Boolean		cached;
    XtDestructor	destructor;
{
    ProcessContext process = _XtGetProcessContext();
    XtAppContext app = process->appContextList;
    XrmRepresentation from = XrmStringToRepresentation(from_type);
    XrmRepresentation to = XrmStringToRepresentation(to_type);

    if (process->globalConverterTable == (ConverterTable)NULL) {
	process->globalConverterTable =
	    (ConverterTable) XtCalloc(CONVERTHASHSIZE, sizeof(ConverterPtr));
    }
    _XtTableAddConverter(process->globalConverterTable, from, to,
			 converter, convert_args,
			 num_args, destructor, True, cached);
    while (app != (XtAppContext)NULL) {
	_XtTableAddConverter(app->converterTable, from, to,
			     converter, convert_args,
			     num_args, destructor, True, cached);
	app = app->next;
    }
}

void XtAppSetConverter(app, from_type, to_type, converter, convert_args, num_args, cached, destructor)
    XtAppContext	app;
    register String	from_type, to_type;
    XtNewConverter	converter;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
    Boolean		cached;
    XtDestructor	destructor;
{
    _XtTableAddConverter(app->converterTable,
	XrmStringToRepresentation(from_type),
        XrmStringToRepresentation(to_type),
	converter, convert_args, num_args,
	destructor, True, cached);
}

/* old interface */
void XtAddConverter(from_type, to_type, converter, convert_args, num_args)
    register String	from_type, to_type;
    XtConverter		converter;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
{
    ProcessContext process = _XtGetProcessContext();
    XtAppContext app = process->appContextList;
    XrmRepresentation from = XrmStringToRepresentation(from_type);
    XrmRepresentation to = XrmStringToRepresentation(to_type);

    if (process->globalConverterTable == (ConverterTable)NULL) {
	process->globalConverterTable =
	    (ConverterTable) XtCalloc(CONVERTHASHSIZE, sizeof(ConverterPtr));
    }
    _XtTableAddConverter(process->globalConverterTable, from, to,
			 (XtNewConverter)converter, convert_args, num_args,
			 (XtDestructor)NULL, False, True);
    while (app != (XtAppContext)NULL) {
	_XtTableAddConverter(app->converterTable, from, to,
			     (XtNewConverter)converter, convert_args,
			     num_args, (XtDestructor)NULL, False, True);
	app = app->next;
    }
}

/* old interface */
void XtAppAddConverter(app, from_type, to_type, converter, convert_args, num_args)
    XtAppContext	app;
    register String	from_type, to_type;
    XtConverter		converter;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
{
    _XtTableAddConverter(app->converterTable,
	XrmStringToRepresentation(from_type),
        XrmStringToRepresentation(to_type),
	(XtNewConverter)converter, convert_args, num_args,
	(XtDestructor)NULL, False, True);
}

static CachePtr
CacheEnter(heap, converter, args, num_args, from, to, succeeded, hash,
	   destructor, closure)
    Heap*		    heap;
    register XtNewConverter converter;
    register XrmValuePtr    args;
    Cardinal		    num_args;
    XrmValuePtr		    from;
    XrmValuePtr		    to;
    Boolean		    succeeded;
    register int	    hash;
    XtDestructor	    destructor;
    caddr_t		    closure;
{
    register	CachePtr *pHashEntry;
    register	CachePtr p;
    register    Cardinal i;
    extern char* _XtHeapAlloc();

    pHashEntry = &cacheHashTable[hash & CACHEHASHMASK];

    p = (CachePtr)  _XtHeapAlloc(heap, sizeof(CacheRec));
    p->next	    = *pHashEntry;
    p->prev	    = (CachePtr)pHashEntry;
    if (p->next != NULL) p->next->prev = p;
    *pHashEntry     = p;
    p->tag	    = (caddr_t)heap;
    p->hash	    = hash;
    p->converter    = converter;
    p->destructor   = destructor;
    p->from.size    = from->size;
    p->from.addr = (caddr_t) _XtHeapAlloc(heap, from->size);
    XtBCopy(from->addr, p->from.addr, from->size);
    p->num_args = num_args;
    if (num_args == 0) {
	p->args = NULL;
    } else {
	p->args = (XrmValuePtr) _XtHeapAlloc(heap, num_args * sizeof(XrmValue));
	for (i = 0; i < num_args; i++) {
	    p->args[i].size = args[i].size;
	    p->args[i].addr = (caddr_t) _XtHeapAlloc(heap, args[i].size);
	    XtBCopy(args[i].addr, p->args[i].addr, args[i].size);
	}
    }
    p->to.size  = to->size;
    if (succeeded && to->addr != NULL) {
	p->to.addr  = (caddr_t) _XtHeapAlloc(heap, to->size);
	XtBCopy(to->addr, p->to.addr, to->size);
	p->conversion_succeeded = True;
    }
    else {
	p->to.addr = NULL;
	p->conversion_succeeded = False;
    }
    p->closure = closure;
    p->ref_count = 1;
    return p;
}


void _XtCacheFlushTag(tag)
    caddr_t	tag;
{
    int i;
    for (i = CACHEHASHSIZE; i;) {
	register CachePtr* prev = &cacheHashTable[--i];
	register CachePtr rec = *prev;
	while (rec != NULL) {
	    while (rec != NULL && rec->tag == tag) rec = rec->next;
	    *prev = rec;
	    if (rec == NULL) break;
	    do {
		prev = &rec->next;
		rec = *prev;
	    } while (rec != NULL && rec->tag != tag);
	}
    }
}

#ifdef DEBUG
void CacheStats()
{
    register Cardinal i;
    register CachePtr p;
    register Cardinal entries;

    for (i = 0; i < CACHEHASHSIZE; i++) {
	p = cacheHashTable[i];
	if (p != NULL) {
	    for (entries = 0; p != NULL; p = p->next) {
		entries++;
	    }
	    (void) fprintf(stdout, "Index: %4d  Entries: %d\n", i, entries);
	    for (p = cacheHashTable[i]; p != NULL; p = p->next) {
		(void) fprintf(stdout, "    Size: %3d  Refs: %3d  '",
			       p->from.size, p->ref_count);
		(void) fwrite(
		    p->from.addr, 1, MIN( (int)p->from.size, 48), stdout);
		(void) fprintf(stdout, "'\n");
	    }
	    (void) fprintf(stdout, "\n");
	}
    }
}
#endif /*DEBUG*/

static Boolean ResourceQuarkToOffset(widget_class, name, offset)
    WidgetClass widget_class;
    XrmName     name;
    Cardinal    *offset;
{
    register WidgetClass     wc;
    register Cardinal        i;
    register XrmResourceList resources;

    for (wc = widget_class;
	wc != NULL;
	wc = wc->core_class.superclass) {
	resources = (XrmResourceList) wc->core_class.resources;
	for (i = 0; i < wc->core_class.num_resources; i++, resources++) {
	    if (resources->xrm_name == name) {
		*offset = -resources->xrm_offset - 1;
		return True;
	    }
	} /* for i in resources */
    } /* for wc in widget classes */
    (*offset) = 0;
    return False;
}


static void ComputeArgs(widget, convert_args, num_args, args)
    Widget		widget;
    XtConvertArgList    convert_args;
    Cardinal		num_args;
    XrmValuePtr		args;
{
    register Cardinal   i;
    Cardinal		offset;
    String              params[1];
    Cardinal		num_params = 0;

    for (i = 0; i < num_args; i++) {
	args[i].size = convert_args[i].size;
	switch (convert_args[i].address_mode) {
	case XtAddress: 
	    args[i].addr = convert_args[i].address_id;
	    break;

	case XtBaseOffset:
	    args[i].addr =
		(caddr_t) ((int) widget + (int) convert_args[i].address_id);
	    break;

	case XtImmediate:
	    args[i].addr = (caddr_t) &(convert_args[i].address_id);
	    break;

	case XtResourceString:
	    /* Convert in place for next usage */
	    convert_args[i].address_mode = XtResourceQuark;
	    convert_args[i].address_id =
	       (caddr_t) XrmStringToQuark((String) convert_args[i].address_id);
	    /* Fall through */

	case XtResourceQuark:
	    if (! ResourceQuarkToOffset(widget->core.widget_class,
		    (XrmQuark) convert_args[i].address_id, &offset)) {
		params[0]=
                  XrmQuarkToString((XrmQuark) convert_args[i].address_id);
               XtAppWarningMsg(XtWidgetToApplicationContext(widget),
		    "invalidResourceName","computeArgs","XtToolkitError",
		    "Cannot find resource name %s as argument to conversion",
                     params,&num_params);
		offset = 0;
	    }
	    args[i].addr = (caddr_t) ((int) widget + offset);
	    break;
	} /* switch */
    } /* for */
} /* ComputeArgs */

void XtDirectConvert(converter, args, num_args, from, to)
    XtConverter     converter;
    XrmValuePtr     args;
    Cardinal	    num_args;
    register XrmValuePtr from;
    XrmValuePtr     to;
{
    register CachePtr   p;
    register int	hash;
    register Cardinal   i;

    /* Try to find cache entry for conversion */
    hash = ((int)(converter) >> 2) + from->size + *((char *) from->addr);
    if (from->size > 1) hash += ((char *) from->addr)[1];
    
    for (p = cacheHashTable[hash & CACHEHASHMASK]; p != NULL; p = p->next) {
	if ((p->hash == hash)
	 && (p->converter == (XtNewConverter)converter)
	 && (p->from.size == from->size)
	 && (! XtBCmp(p->from.addr, from->addr, from->size))
         && (p->num_args == num_args)) {
	    /* Are all args the same data ? */
	    for (i = 0; i < num_args; i++) {
		if (   p->args[i].size != args[i].size
		    || XtBCmp(p->args[i].addr, args[i].addr, args[i].size)) {
		    break;
		}
	    }
	    if (i == num_args) {	 
		/* Perfect match */
		(*to) = p->to;
		return;
	    }
	}
    }

    /* Didn't find it, call converter procedure and entry result in cache */
    (*to).size = 0;
    (*to).addr = NULL;
    (*converter)(args, &num_args, from, to);
    /* This memory can never be freed since we don't know the Display
     * or app context from which to compute the persistance */
    {
	static Heap globalHeap = {NULL, NULL, 0};
	CacheEnter( &globalHeap, (XtNewConverter)converter, args, num_args,
		    from, to, (to->addr != NULL), hash,
		    (XtDestructor)NULL, NULL
		   );
    }
}


static ConverterPtr GetConverterEntry( app, converter )
    XtAppContext app;
    XtNewConverter converter;
{
    int entry;
    register ConverterPtr cP;
    ConverterTable converterTable = app->converterTable;
    cP = NULL;
    for (entry = 0; entry < CONVERTHASHSIZE && cP == NULL; entry++) {
	cP = converterTable[entry];
	while (cP != NULL && cP->converter != converter) cP = cP->next;
    }
    return cP;
}


Boolean XtCallConverter(dpy, converter, args, num_args, from, to, cache_ref_return)
    Display*	    dpy;
    XtNewConverter  converter;
    XrmValuePtr     args;
    Cardinal	    num_args;
    register XrmValuePtr from;
    XrmValuePtr     to;
    XtCacheRef	    *cache_ref_return;
{
    register CachePtr   p;
    register int	hash;
    register Cardinal   i;
    register ConverterPtr cP;
    XtDestructor nullProc = NULL; /* some compilers broken */

    cP = GetConverterEntry( XtDisplayToApplicationContext(dpy), converter );
    if (cP == NULL || (!cP->use_cache && cP->destructor == nullProc)) {
	char* closure;
	if (cache_ref_return != NULL) *cache_ref_return = NULL;
	return (*(XtNewConverter)converter)
	    (dpy, args, &num_args, from, to, &closure);
    }

    /* Try to find cache entry for conversion */
    hash = ((int)(converter) >> 2) + from->size + *((char *) from->addr);
    if (from->size > 1) hash += ((char *) from->addr)[1];
    
    if (cP->use_cache) {
	for (p = cacheHashTable[hash & CACHEHASHMASK]; p != NULL; p = p->next){
	    if ((p->hash == hash)
	     && (p->converter == converter)
	     && (p->from.size == from->size)
	     && (! XtBCmp(p->from.addr, from->addr, from->size))
	     && (p->num_args == num_args)) {
		/* Are all args the same data ? */
		for (i = 0; i < num_args; i++) {
		    if (p->args[i].size != args[i].size
		     || XtBCmp(p->args[i].addr, args[i].addr, args[i].size)) {
			break;
		    }
		}
		if (i == num_args) {	 
		    /* Perfect match */
		    if (p->conversion_succeeded)
			XtBCopy(p->to.addr, to->addr, to->size);
		    p->ref_count++;
		    if (cache_ref_return != NULL)
			*cache_ref_return = (XtCacheRef)p;
		    return p->conversion_succeeded;
		}
	    }
	}
    }

    /* No cache entry, call converter procedure and enter result in cache */
    {
	Heap *heap;
	char* closure;
	Boolean retval =
	    (*(XtNewConverter)converter)(dpy, args, &num_args, from, to, &closure);
	if (cP->destructor == nullProc)
	    heap = &_XtGetPerDisplay(dpy)->heap;
	else
	    heap = NULL;
	p = CacheEnter(heap, converter, args, num_args,
		       from, to, retval, hash, cP->destructor, closure);
	if (cache_ref_return != NULL) {
	    if (heap == NULL /* => has destructor */)
		*cache_ref_return = (XtCacheRef)p;
	    else
		*cache_ref_return = NULL;
	}
	return retval;
    }
}

Boolean _XtConvert(widget, from_type, from, to_type, to, cache_ref_return)
             Widget		widget;
    register XrmRepresentation	from_type;
	     XrmValuePtr	from;
    register XrmRepresentation	to_type;
    register XrmValuePtr	to;
    XtCacheRef			*cache_ref_return;
{
    XtAppContext	app = XtWidgetToApplicationContext(widget);
    register ConverterPtr	p;
    Cardinal		num_args;
    XrmValue		stack_args[20], *args;
    String              params[2];
    Cardinal		num_params = 0;

    /* Look for type converter */
    p = app->converterTable[ProcHash(from_type, to_type) & CONVERTHASHMASK];
    for (; p != NULL; p = p->next) {
	if (from_type == p->from && to_type == p->to) {
	    Boolean retval;
	    /* Compute actual arguments from widget and arg descriptor */
	    num_args = p->num_args;
	    if (num_args > XtNumber(stack_args)) {
		args = (XrmValuePtr) XtMalloc(num_args * sizeof (XrmValue));
	    } else {
		args = stack_args;
	    }
	    if (num_args != 0)
		ComputeArgs(widget, p->convert_args, num_args, args);
	    if (p->new_style) {
		retval =
		    XtCallConverter(XtDisplay(widget),
				    p->converter, args, num_args,
				    from, to, cache_ref_return);
	    }
	    else { /* is old-style (non-app) converter */
		XtDirectConvert((XtConverter)p->converter, args, num_args,
				from, to);
		retval = (to->addr != NULL);
	    }
	    if (args != stack_args) XtFree((char *) args);
	    return retval;
	}
    }

	params[0] = XrmRepresentationToString(from_type);
	params[1] = XrmRepresentationToString(to_type);
    XtAppWarningMsg(app, "typeConversionError","noConverter","XtToolkitError",
	     "No type converter registered for '%s' to '%s' conversion.",
             params,&num_params);
    return False;
}

void XtConvert(widget, from_type_str, from, to_type_str, to)
    Widget	widget;
    String	from_type_str;
    XrmValuePtr	from;
    String	to_type_str;
    XrmValuePtr	to;
{
    XrmQuark    from_type, to_type;

    from_type = XrmStringToRepresentation(from_type_str);
    to_type = XrmStringToRepresentation(to_type_str);
    if (from_type != to_type) {
	XtCacheRef ref;
	to->addr = NULL;
	to->size = 0;
	_XtConvert(widget, from_type, from, to_type, to, &ref);
	if (ref != NULL) {
	    XtAddCallback( widget, XtNdestroyCallback,
			   XtCallbackReleaseCacheRef, (caddr_t)ref );
	}
    }
    else
	(*to) = *from;
}


void XtAppReleaseCacheRefs(app, refs)
    XtAppContext app;
    XtCacheRef *refs;
{
    register CachePtr *r;
    void (*nullProc)() = NULL;	/* some compilers broken */
    for (r = (CachePtr*)refs; *r != NULL; r++) {
	register CachePtr p = *r;
	register int i;
	if (--(p->ref_count) == 0) {
	    if (p->destructor != nullProc)
		(*p->destructor) (app, &p->to, p->closure, p->args,
				  &p->num_args);
	    XtFree(p->from.addr);
	    for (i = p->num_args; i;) {
		XtFree(p->args[--i].addr);
	    }
	    XtFree(p->args);
	    XtFree(p->to.addr);
	    p->prev->next = p->next;
	    XtFree(p);
	}
    }
}


/* ARGSUSED */
void XtCallbackReleaseCacheRefList(widget, closure, call_data)
    Widget widget;		/* unused */
    caddr_t closure;
    caddr_t call_data;		/* unused */
{
    XtAppReleaseCacheRefs( XtWidgetToApplicationContext(widget),
			   (XtCacheRef*)closure );
    XtFree(closure);
}


/* ARGSUSED */
void XtCallbackReleaseCacheRef(widget, closure, call_data)
    Widget widget;		/* unused */
    caddr_t closure;
    caddr_t call_data;		/* unused */
{
    XtCacheRef cache_refs[2];
    cache_refs[0] = (XtCacheRef)closure;
    cache_refs[1] = NULL;
    XtAppReleaseCacheRefs( XtWidgetToApplicationContext(widget), cache_refs );
}
