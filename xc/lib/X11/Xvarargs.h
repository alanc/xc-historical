/* $XConsortium: Xvarargs.h,v 1.12 91/03/23 11:25:21 rws Exp $ */
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

#ifndef _XVARARGS_H_
#define _XVARARGS_H_

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

#define Va_start(a,b) va_start(a,b)

#else /* !__HIGHC__ */

#if NeedVarargsPrototypes
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

#endif /* __HIGHC__ */

#endif /* _XVARARGS_H_ */
