/* $XConsortium: Xlocale.h,v 1.1 91/02/20 22:20:01 rws Exp $ */

#ifndef _XLOCALE_H_
#define _XLOCALE_H_

#include <X11/Xfuncproto.h>

#define LC_ALL      0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME     5

_XFUNCPROTOBEGIN
extern char *_Xsetlocale(
#if NeedFunctionPrototypes
    int /* category */,
    _Xconst char* /* name */
#endif
);
_XFUNCPROTOEND

#define setlocale _Xsetlocale

#ifndef NULL
#define NULL 0
#endif

#endif /* _XLOCALE_H_ */
