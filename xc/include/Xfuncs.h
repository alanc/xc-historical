/*
 * $XConsortium: Xfuncs.h,v 1.1 90/12/26 15:52:32 rws Exp $
 * 
 * Copyright 1990 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

/* Watch out: bcopy can have the order of evaluation of arguments reversed */

#ifndef _XFUNCS_H_
#define _XFUNCS_H_

#if __STDC__ && !defined(NOSTDHDRS) && !defined(_XUSEBFUNCS)
#include <string.h>
#define bcopy(b1,b2,len) memmove((void *)(b2), (void *)(b1), (size_t)(len))
#define bzero(b,len) memset((void *)(b), 0, (size_t)(len))
#define bcmp(b1,b2,len) memcmp((void *)(b1), (void *)(b2), (size_t)(len))
#else
#if defined(SYSV) && defined(hpux) && !defined(_XUSEBFUNCS)
#include <string.h>
#define bcopy(b1,b2,len) memmove(b2, b1, len)
#define bzero(b,len) memset(b, 0, len)
#define bcmp(b1,b2,len) memcmp(b1, b2, len)
#else
#if defined(SYSV) && !defined(_XUSEBFUNCS)
#include <memory.h>
void bcopy();
#define bzero(b,len) memset(b, 0, len)
#define bcmp(b1,b2,len) memcmp(b1, b2, len)
#else
void bcopy();
void bzero();
int bcmp();
#endif
#endif
#endif

#endif /* _XFUNCS_H_ */
