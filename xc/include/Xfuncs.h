/*
 * $XConsortium: Xfuncs.h,v 1.11 93/08/24 17:21:08 gildea Exp $
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

#ifndef _XFUNCS_H_
#define _XFUNCS_H_

#include <X11/Xosdefs.h>

/* the old Xfuncs.h, for pre-R6 */

#ifdef X_USEBFUNCS
void bcopy();
void bzero();
int bcmp();
#else
#if (__STDC__ && !defined(X_NOT_STDC_ENV) && !defined(sun) && !defined(macII) && !defined(apollo)) || defined(SVR4) || defined(hpux) || defined(_IBMR2)
#include <string.h>
#define _XFUNCS_H_INCLUDED_STRING_H
#define bcopy(b1,b2,len) memmove(b2, b1, (size_t)(len))
#define bzero(b,len) memset(b, 0, (size_t)(len))
#define bcmp(b1,b2,len) memcmp(b1, b2, (size_t)(len))
#else
#ifdef sgi
#include <bstring.h>
#else
#ifdef SYSV
#include <memory.h>
#if defined(_XBCOPYFUNC) && !defined(macII)
#define bcopy _XBCOPYFUNC
#define _XNEEDBCOPYFUNC
#endif
void bcopy();
#define bzero(b,len) memset(b, 0, len)
#define bcmp(b1,b2,len) memcmp(b1, b2, len)
#else /* bsd */
void bcopy();
void bzero();
int bcmp();
#endif /* SYSV */
#endif /* sgi */
#endif /* __STDC__ and relatives */
#endif /* X_USEBFUNCS */

/* the new Xfuncs.h */

#if !defined(X_NOT_STDC_ENV) && (!defined(sun) || defined(SVR4))
/* the ANSI C way */
#ifndef _XFUNCS_H_INCLUDED_STRING_H
#include <string.h>
#endif
#undef bzero
#define bzero(b,len) memset(b,0,len)
#else /* else X_NOT_STDC_ENV or SunOS 4 */
#if defined(SYSV) || defined(luna) || defined(sun)
#include <memory.h>
#define memmove(dst,src,len) bcopy((char *)(src),(char *)(dst),(int)(len))
#if defined(SYSV) && defined(_XBCOPYFUNC)
#define memmove(dst,src,len) _XBCOPYFUNC((char *)(src),(char *)(dst),(int)(len))
#define _XNEEDBCOPYFUNC
#endif
#else /* else vanilla BSD */
#define memmove(dst,src,len) bcopy((char *)(src),(char *)(dst),(int)(len))
#define memcpy(dst,src,len) bcopy((char *)(src),(char *)(dst),(int)(len))
#define memcmp(b1,b2,len) bcmp((char *)(b1),(char *)(b2),(int)(len))
#endif /* SYSV else */
#endif /* ! X_NOT_STDC_ENV else */

#endif /* _XFUNCS_H_ */
