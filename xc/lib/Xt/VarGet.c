#ifndef lint
static char Xrcsid[] =
    "$XConsortium: VarGet.c,v 1.4 89/11/10 17:48:21 swick Exp $";
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

#include <X11/StringDefs.h>
#include "IntrinsicI.h"
#include "VarargsI.h"

#if IncludePrototypes
void
XtVaGetSubresources(Widget widget, XtPointer base, String name, String class, XtResourceList resources, Cardinal num_resources, ...)
#else
/*VARARGS6*/
void XtVaGetSubresources(widget, base, name, class, resources, num_resources, va_alist)
    Widget widget;
    XtPointer base;
    String name;
    String class;
    XtResourceList resources;
    Cardinal num_resources;
    va_dcl
#endif
{
    va_list                 var;
    ArgList                 args;
    Cardinal                num_args;
	 
    Va_start(var, num_resources);
	      
		   
    _XtVaToArgList(widget, &args, &num_args, var);

    XtGetSubresources(widget, base, name, class, resources, num_resources, 
	args, num_args);

    if (num_args != 0) {
	XtFree((char *)args);
    }    

    va_end(var);
}


#if IncludePrototypes
void
XtVaGetApplicationResources(Widget widget, XtPointer base, XtResourceList resources, Cardinal num_resources, ...)
#else
/*VARARGS4*/
void XtVaGetApplicationResources(widget, base, resources, num_resources, va_alist)
    Widget widget;
    XtPointer base;
    XtResourceList resources;
    Cardinal num_resources;
    va_dcl
#endif
{
    va_list                 var;
    ArgList                 args; 
    Cardinal                num_args; 

    Va_start(var,num_resources); 
               
                    
    _XtVaToArgList(widget, &args, &num_args, var);
                                
    XtGetApplicationResources(widget, base, resources, num_resources, 
	args, num_args); 

    if (num_args != 0) {
	XtFree((char *)args);
    }    

    va_end(var);         
} 


static void
_XtGetTypedArg(widget, typed_arg, resources, num_resources)
    Widget              widget;
    XtTypedArgList	typed_arg;
    XtResourceList      resources;
    Cardinal            num_resources;
{
    String              from_type = NULL;
    Cardinal		from_size = 0;
    XrmValue            from_val, to_val;
    register int        i;
    Arg			arg;
    caddr_t		value;

    /* note we presume that the XtResourceList to be un-compiled */

    for (i = 0; i < num_resources; i++) {
        if (StringToName(typed_arg->name) == StringToName(resources[i].resource_name)) {
            from_type = resources[i].resource_type;
	    from_size = resources[i].resource_size;
            break;
        }
    }    

    if (i == num_resources) {
	XtAppWarningMsg(XtDisplayToApplicationContext(XtDisplay(widget)),
            "unknownType", "xtGetTypedArg", "XtToolkitError",
            "Unable to find type of resource for conversion",
            (String *)NULL, (Cardinal *)NULL);
 	return;
    }

    value = (caddr_t)XtMalloc(from_size);
    XtSetArg(arg, typed_arg->name, value);
    XtGetValues(widget, &arg, 1);

    to_val.addr = NULL;
    from_val.size = from_size;
    from_val.addr = (caddr_t)value;

    XtConvert(widget, from_type, &from_val, typed_arg->type, &to_val);

    if (to_val.addr == NULL) {
        XtAppWarningMsg(XtDisplayToApplicationContext(XtDisplay(widget)),
            "conversionFailed", "xtGetTypedArg", "XtToolkitError",
            "Type conversion failed", (String *)NULL, (Cardinal *)NULL);
	return;
    }

    if (to_val.size > typed_arg->size) {
        XtAppWarningMsg(XtDisplayToApplicationContext(XtDisplay(widget)), 
            "conversionFailed", "xtGetTypedArg", "XtToolkitError",
            "Insufficient space for converted type", (String *)NULL, 
	    (Cardinal *)NULL); 
        return; 
    }

    bcopy((char *)to_val.addr, (char *)typed_arg->value, (int)to_val.size);
    XtFree((char *)value);
}

static int
_XtGetNestedArg(widget, avlist, args, resources, num_resources)
    Widget              widget;
    XtTypedArgList	avlist;
    ArgList             args;
    XtResourceList      resources;
    Cardinal            num_resources;
{
    int         count = 0;
 
    for (; avlist->name != NULL; avlist++) {
        if (avlist->type != NULL) {
	    _XtGetTypedArg(widget, avlist, resources, num_resources);
        } else if(strcmp(avlist->name, XtVaNestedList) == 0) {
            count += _XtGetNestedArg(widget, (XtTypedArgList)avlist->value,
				     args, resources, num_resources);
        } else {
            (args+count)->name = avlist->name;
            (args+count)->value = avlist->value;
            ++count;
        }
    }   
 
    return(count);
}

#if IncludePrototypes
void
XtVaGetValues(Widget widget, ...)
#else
/*VARARGS1*/
void XtVaGetValues(widget, va_alist)
    Widget widget;
    va_dcl
#endif
{
    va_list		var;
    String      	attr;
    ArgList    		args;
    XtTypedArg		typed_arg;
    XtResourceList      resources = (XtResourceList)NULL;
    Cardinal            num_resources;
    int			count, total_count, typed_count;

    Va_start(var,widget);

    _XtCountVaList(var, &total_count, &typed_count);

    if (total_count != typed_count) {
        args = (ArgList)XtMalloc((unsigned)((total_count - typed_count) 
				* sizeof(Arg)));
    }
    else args = NULL;		/* for lint; really unused */

    for(attr = va_arg(var, String), count = 0 ; attr != NULL;
			attr = va_arg(var, String)) {
	if (strcmp(attr, XtVaTypedArg) == 0) {
	    typed_arg.name = va_arg(var, String);
	    typed_arg.type = va_arg(var, String);
	    typed_arg.value = va_arg(var, XtArgVal);
	    typed_arg.size = va_arg(var, int);

	    XtGetResourceList(XtClass(widget), &resources, &num_resources);

	    _XtGetTypedArg(widget, &typed_arg, resources, num_resources);
	} else if (strcmp(attr, XtVaNestedList) == 0) {
	    XtGetResourceList(XtClass(widget), &resources, &num_resources);

	    count += _XtGetNestedArg(widget, va_arg(var, XtTypedArgList),
				     (args+count), resources, num_resources);
	} else {
	    args[count].name = attr;
	    args[count].value = va_arg(var, XtArgVal);
	    count ++;
	}
    }

    if (resources != (XtResourceList)NULL) { 
	XtFree((char *)resources); 
    }

    if (total_count != typed_count) {
	XtGetValues(widget, args, count);
	XtFree((char *)args);
    }
       
    va_end(var);
}

#if IncludePrototypes
void
XtVaGetSubvalues(XtPointer base,XtResourceList  resources, Cardinal num_resources, ...)
#else
/*VARARGS3*/
void XtVaGetSubvalues(base, resources, num_resources, va_alist)
    XtPointer base;
    XtResourceList  resources;
    Cardinal num_resources;
    va_dcl
#endif
{
    va_list	var;
    ArgList    	args;
    Cardinal   	num_args;
    int		total_count, typed_count;		

    Va_start(var,num_resources);

    _XtCountVaList(var, &total_count, &typed_count);

    if (typed_count != 0) {
	XtWarning("XtVaTypedArg is an invalid argument to XtVaGetSubvalues()\n");
    }

    _XtVaToArgList((Widget)NULL, &args, &num_args, var);

    XtGetSubvalues(base, resources, num_resources, args, num_args);

    if (num_args != 0) {
        XtFree((char *)args);
    }    

    va_end(var);
}
