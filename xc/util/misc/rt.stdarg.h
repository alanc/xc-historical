/* $XConsortium: rt.stdarg.h,v 1.0 93/10/25 15:35:10 kaleb Exp $ */
#ifndef _STDARG_H
#define _STDARG_H
typedef int *va_list;
#define va_start(ap, arg)       ap = ((int *)&arg) + ((sizeof(arg) + 3) / 4)
#define va_end(ap)
#define va_arg(ap, type)        ((type *)(ap += (sizeof(type)+3)/4))[-1]
#endif
