#ifndef lint
static char Xrcsid[] =
    "$XConsortium: Varargs.c,v 1.1 89/11/08 17:47:41 swick Exp $";
#endif
/*

Copyright 1985, 1986, 1987, 1988, 1989 by the
Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Quarks.h>
#include "VarargsI.h"
#include "ResourceI.h"


/*
 *   _XtVarargsInitialize()
 *
 *   Initializes Quarks for resource types ...
 *   called from XtToolkitInitialize
 *
 */

XrmQuark XtQVaNestedList;
XrmQuark XtQVaTypedArg;

void _XtVarargsInitialize()
{
	XtQVaNestedList = StringToQuark((String)XtVaNestedList);
	XtQVaTypedArg = StringToQuark((String)XtVaTypedArg);
}

/*
 *    Given a nested list, _XtCountNestedList() returns counts of the
 *    total number of attribute-value pairs and the count of those
 *    attributes that are typed. The list is counted recursively.
 */
static  void
_XtCountNestedList(avlist, total_count, typed_count)
    XtTypedArgList  avlist;
    int             *total_count;
    int             *typed_count;
{
    for (; avlist->name != NULL; avlist++) {
        if(StringToQuark(avlist->name) == XtQVaNestedList) {
            _XtCountNestedList((XtTypedArgList)avlist->value, total_count,
                	       typed_count);
        } else {
            if (avlist->type != NULL) {
                ++(*typed_count);
            }
            ++(*total_count);
        }
    }    
}


/*
 *    Given a variable length attribute-value list, _XtCountVaList()
 *    returns counts of the total number of attribute-value pairs,
 *    and the count of the number of those attributes that are typed.
 *    The list is counted recursively.
 */
void
_XtCountVaList(var, total_count, typed_count)
    va_list     var;
    int         *total_count;
    int         *typed_count;
{
    String          attr;
    
    *total_count = 0;
    *typed_count = 0;
 
    for(attr = va_arg(var, String) ; attr != NULL;
                        attr = va_arg(var, String)) {
        if (StringToQuark(attr) == XtQVaTypedArg) {
            va_arg(var, String);
            va_arg(var, String);
            va_arg(var, XtArgVal);
            va_arg(var, int);
            ++(*total_count);
            ++(*typed_count);
        } else if (StringToQuark(attr) == XtQVaNestedList) {
            _XtCountNestedList(va_arg(var, XtTypedArgList), total_count,
                typed_count);
        } else {
            va_arg(var, XtArgVal);
            ++(*total_count);
	}
    }
}


/* 
 *   Given a variable length attribute-value list, XtVaCreateArgsList()
 *   constructs an attribute-value list of type XtTypedArgList and 
 *   returns the list.
 */
#ifdef _STDC_
XtVarArgsList
XtVaCreateArgsList(XtPointer unused, ...)
#else
XtVarArgsList
XtVaCreateArgsList(unused, va_alist)
    XtPointer unused;
    va_dcl
#endif
{
    va_list    	    var;
    String	    attr;
    XtTypedArgList  avlist;
    int		    count = 0;

    /*
     * Count the number of attribute-value pairs in the list.
     * Note: The count is required only to allocate enough space to store
     * the list. Thefore nested lists are not counted recursively.
     */
    Va_start(var,unused);
    for(attr = va_arg(var, String) ; attr != NULL;
                        attr = va_arg(var, String)) {
        ++count;
        if (StringToQuark(attr) == XtQVaTypedArg) {
            va_arg(var, String);
            va_arg(var, String);
            va_arg(var, XtArgVal);
            va_arg(var, int);
        } else {
            va_arg(var, XtArgVal);
        }
    }
    va_end(var);

    avlist = (XtTypedArgList)
		XtCalloc((int)count + 1, (unsigned)sizeof(XtTypedArg));

    Va_start(var,unused);
    for(attr = va_arg(var, String), count = 0; attr != NULL; 
		attr = va_arg(var, String)) {
	if (StringToQuark(attr) == XtQVaTypedArg) {
	    avlist[count].name = va_arg(var, String);
	    avlist[count].type = va_arg(var, String);
	    avlist[count].value = va_arg(var, XtArgVal);
	    avlist[count].size = va_arg(var, int);
	} else {
	    avlist[count].name = attr;
	    avlist[count].type = NULL;
	    avlist[count].value = va_arg(var, XtArgVal);
	}
	++count;
    }
    avlist[count].name = NULL;
    va_end(var);

    return((XtVarArgsList)avlist);
}


/*
 *    _XtTypedArgToArg() invokes a resource converter to convert the
 *    passed typed arg into a name/value pair and stores the name/value
 *    pair in the passed Arg structure. It returns 1 if the conversion
 *    succeeded and 0 if the conversion failed.
 */
static int
_XtTypedArgToArg(widget, typed_arg, arg_return, resources, num_resources)
    Widget              widget;
    XtTypedArgList      typed_arg;
    ArgList             arg_return;
    XtResourceList      resources;
    Cardinal            num_resources;
{     
    String              to_type = NULL;
    XrmValue            from_val, to_val;
    register int        i;
      
    if (widget == NULL) {
        XtWarning("XtVaTypedArg conversion needs non-NULL widget handle\n");
        return(0);
    }
       
    /* again we assume that the XtResourceList is un-compiled */

    for (i = 0; i < num_resources; i++) {
        if (StringToName(typed_arg->name) ==
            StringToName(resources[i].resource_name)) {
            to_type = resources[i].resource_type;
            break;
        }
    }

    if (to_type == NULL) {
        XtAppWarningMsg(XtDisplayToApplicationContext(XtDisplay(widget)),
            "unknownType", "xtConvertVarToArgList", "XtToolkitError",
            "Unable to find type of resource for conversion",
            (String *)NULL, (Cardinal *)NULL);
        return(0);
    }
       
    to_val.addr = NULL;
    from_val.size = typed_arg->size;
    if ((StringToQuark(typed_arg->type) == XtQString) ||
            (typed_arg->size > sizeof(XtArgVal))) {
        from_val.addr = (caddr_t)typed_arg->value;
    } else {
            from_val.addr = (caddr_t)&typed_arg->value;
    }
       
    XtConvert(widget, typed_arg->type, &from_val, to_type, &to_val);
 
    if (to_val.addr == NULL) {
        XtAppWarningMsg(XtDisplayToApplicationContext(XtDisplay(widget)),
            "conversionFailed", "xtConvertVarToArgList", "XtToolkitError",
            "Type conversion failed", (String *)NULL, (Cardinal *)NULL);
        return(0);
    }

    arg_return->name = typed_arg->name;
    if (to_val.size == sizeof(long))
        arg_return->value = *(long *)to_val.addr;
    else if (to_val.size == sizeof(short))
        arg_return->value = *(short *)to_val.addr;
    else if (to_val.size == sizeof(char))
        arg_return->value = *(char *)to_val.addr;
    else if (to_val.size == sizeof(XtArgVal))
        arg_return->value = *(XtArgVal *)to_val.addr;
       
    return(1);
}


/*
 *    _XtNestedArgtoArg() converts the passed nested list into
 *    an ArgList/count.
 */
static int
_XtNestedArgtoArg(widget, avlist, args, resources, num_resources)
    Widget              widget;
    XtTypedArgList      avlist;
    ArgList             args;
    XtResourceList      resources;
    Cardinal            num_resources;
{
    int         count = 0;
 
    for (; avlist->name != NULL; avlist++) {
        if (avlist->type != NULL) {
            /* If widget is NULL, the typed arg is ignored */
            if (widget != NULL) {
                /* this is a typed arg */
                count += _XtTypedArgToArg(widget, avlist, (args+count),
                             resources, num_resources);
            }
        } else if(StringToQuark(avlist->name) == XtQVaNestedList) {
            count += _XtNestedArgtoArg(widget, (XtTypedArgList)avlist->value,
                        (args+count), resources, num_resources);
        } else {
            (args+count)->name = avlist->name;
            (args+count)->value = avlist->value;
            ++count;
        }
    }

    return(count);
}
 
 
/* 
 *    Given a variable argument list, _XtVaToArgList() returns the 
 *    equivalent ArgList and count. _XtVaToArgList() handles nested 
 *    lists and typed arguments. 
 */
void
_XtVaToArgList(widget, args_return, num_args_return, var)
    Widget		widget;
    ArgList		*args_return;
    Cardinal		*num_args_return;
    va_list     	var;
{
    String		attr;
    int			count = 0, total_count = 0, typed_count = 0;
    ArgList		args = (ArgList)NULL;
    XtTypedArg		typed_arg;
    XtResourceList	resources = (XtResourceList)NULL;
    Cardinal		num_resources = 0;
    Boolean		fetched_resource_list = False;

    _XtCountVaList(var, &total_count, &typed_count);

    if (total_count  == 0) {
	*num_args_return = 0;
	*args_return = (ArgList)NULL;
	return;
    }


    args = (ArgList)XtMalloc((unsigned)(total_count * sizeof(Arg)));

    for(attr = va_arg(var, String) ; attr != NULL;
			attr = va_arg(var, String)) {
	if (StringToQuark(attr) == XtQVaTypedArg) {
	    typed_arg.name = va_arg(var, String);
	    typed_arg.type = va_arg(var, String);
	    typed_arg.value = va_arg(var, XtArgVal);
	    typed_arg.size = va_arg(var, int);

	    /* if widget is NULL, typed args are ignored */
	    if (widget != NULL) {
		if (!fetched_resource_list) {
		    XtGetResourceList(XtClass(widget), &resources, &num_resources);
		    fetched_resource_list = True;
		}
		count += _XtTypedArgToArg(widget, &typed_arg, &args[count],
			     resources, num_resources);
	    }
	} else if (StringToQuark(attr) == XtQVaNestedList) {
	    if (widget != NULL || !fetched_resource_list) {
		XtGetResourceList(XtClass(widget), &resources, &num_resources);
		fetched_resource_list = True;
	    }

	    count += _XtNestedArgtoArg(widget, va_arg(var, XtTypedArgList),
			&args[count], resources, num_resources);
	} else {
	    args[count].name = attr;
	    args[count].value = va_arg(var, XtArgVal);
	    count ++;
	}
    }

    if (resources != (XtResourceList)NULL) {
	XtFree((char *)resources);
    }

    *num_args_return = (Cardinal)count;
    *args_return = (ArgList)args;
}


static int _XtNestedArgtoTypedArg(args, avlist) 
    XtTypedArgList      args;
    XtTypedArgList      avlist;
{    
    int         count = 0;
     
    for (; avlist->name != NULL; avlist++) { 
        if (avlist->type != NULL) { 
            (args+count)->name = avlist->name; 
            (args+count)->type = avlist->type; 
            (args+count)->size = avlist->size;
            (args+count)->value = avlist->value;
            ++count; 
        } else if(StringToQuark(avlist->name) == XtQVaNestedList) {             
            count += _XtNestedArgtoTypedArg((args+count),  
                            (XtTypedArgList)avlist->value); 
        } else {                             
            (args+count)->name = avlist->name; 
	    (args+count)->type = NULL;
            (args+count)->value = avlist->value; 
            ++count;
        }                                     
    }         
    return(count);
}


/*
 *    Given a variable argument list, _XtVaToTypedArgList() returns 
 *    the equivalent TypedArgList. _XtVaToTypedArgList() handles nested
 *    lists.
 *    Note: _XtVaToTypedArgList() does not do type conversions.
 */
void
_XtVaToTypedArgList(var, args_return, num_args_return)
    va_list             var;
    XtTypedArgList   	*args_return;
    Cardinal            *num_args_return;
{
    XtTypedArgList	args = NULL;
    String              attr;
    int			count, total_count, typed_count;

    _XtCountVaList(var, &total_count, &typed_count);

    args = (XtTypedArgList)XtMalloc((unsigned)(total_count * 
				sizeof(XtTypedArg))); 

    for(attr = va_arg(var, String), count = 0 ; attr != NULL;
		    attr = va_arg(var, String)) {
        if (StringToQuark(attr) == XtQVaTypedArg) {
	    args[count].name = va_arg(var, String);
	    args[count].type = va_arg(var, String);
	    args[count].value = va_arg(var, XtArgVal);
	    args[count].size = va_arg(var, int);
	    ++count;
	} else if (StringToQuark(attr) == XtQVaNestedList) {
   	    count += _XtNestedArgtoTypedArg(&args[count], 
			va_arg(var, XtTypedArgList));
	} else {
	    args[count].name = attr;
	    args[count].type = NULL;
	    args[count].value = va_arg(var, XtArgVal);
	    ++count;
	}
    }

    *args_return = args;
    *num_args_return = count;
}
