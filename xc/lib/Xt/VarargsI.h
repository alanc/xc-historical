/* $XConsortium: VarargsI.h,v 1.4 89/11/13 13:09:23 swick Exp $ */
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

#ifndef _VarargsI_h_ 
#define _VarargsI_h_ 
 
#ifdef __STDC__
# ifndef MISSING_STDARG_H
#  include <stdarg.h>
# endif
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

typedef struct _XtTypedArg{
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg, *XtTypedArgList;
 
/* private routines */

extern void _XtCountVaList(
#if IncludePrototypes
    va_list /*var*/, int* /*total_count*/, int* /*typed_count*/
#endif
);

extern void _XtVaToArgList(
#if IncludePrototypes
   Widget /*widget*/, va_list /*var*/, int /*max_count*/, ArgList* /*args_return*/, Cardinal* /*num_args_return*/
#endif
);

extern void _XtVaToTypedArgList(
#if IncludePrototypes
    va_list /*var*/, int /*count*/, XtTypedArgList* /*args_return*/, Cardinal* /*num_args_return*/
#endif
);

extern XtTypedArgList _XtVaCreateTypedArgList(
#if IncludePrototypes
    va_list /*var*/, int /*count*/
#endif
);

#endif /* _VarargsI_h_ */
