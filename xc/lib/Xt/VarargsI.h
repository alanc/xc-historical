/* $XConsortium: VarargsI.h,v 1.10 90/12/12 14:54:01 rws Exp $ */
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
 
#ifdef __HIGHC__
#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG)                                           \
 (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))

#define va_end(AP)

#define va_arg(AP, TYPE)                                                \
 (AP += __va_rounded_size (TYPE),                                       \
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

#endif /* _STDARG_H */
#endif /* __HIGHC__ */

#if __STDC__
# ifndef NOSTDHDRS
#  define _XSTDVAR
# else
# ifdef __GNUC__
#  define _XSTDVAR
# endif
# endif
#endif

#ifdef _XSTDVAR
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#else
# if NeedVarargsPrototypes
#  undef NeedVarargsPrototypes
#  define NeedVarargsPrototypes 0
# endif
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg;
 
/* private routines */

extern void _XtCountVaList(
#if NeedFunctionPrototypes
    va_list /*var*/, int* /*total_count*/, int* /*typed_count*/
#endif
);

extern void _XtVaToArgList(
#if NeedFunctionPrototypes
   Widget /*widget*/, va_list /*var*/, int /*max_count*/, ArgList* /*args_return*/, Cardinal* /*num_args_return*/
#endif
);

extern void _XtVaToTypedArgList(
#if NeedFunctionPrototypes
    va_list /*var*/, int /*count*/, XtTypedArgList* /*args_return*/, Cardinal* /*num_args_return*/
#endif
);

extern XtTypedArgList _XtVaCreateTypedArgList(
#if NeedFunctionPrototypes
    va_list /*var*/, int /*count*/
#endif
);

#endif /* _VarargsI_h_ */
