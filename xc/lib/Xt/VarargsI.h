#ifndef _VarargsI_h_ 
#define _VarargsI_h_ 
 
#ifdef _STDC_
#include <stdarg.h>
#define Va_start(a,b) va_start(a,b)
#else
#include <varargs.h>
#define Va_start(a,b) va_start(a)
#endif

typedef struct _XtTypedArg{
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg, *XtTypedArgList;
 
/* private routines */
extern void _XtCountVaList(
#ifdef _STDC_
    va_list var, int total_count, int typed_count
#endif
);

extern void _XtVaToArgList(
#ifdef _STDC_
 Widget widget, int num_args, ArgList *args_return, int *num_args_return,
 va_list var
#endif
);

extern void _XtVaToTypedArgList(
#ifdef _STDC_
 va_list var, XtTypedArgList *args_return, Cardinal *num_args_return
#endif
);

#endif /* _VarargsI_h_ */
