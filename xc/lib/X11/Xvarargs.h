#ifndef _XVARARGS_H_
#define _XVARARGS_H_

#if __STDC__
# ifndef NOSTDHDRS
#  define _XSTDVAR
# else
# ifdef __GNUC__
#  define _XSTDVAR
# endif
# endif
#else
#ifdef sgi
#define _XSTDVAR
#endif
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

#endif /* _XVARARGS_H_ */
