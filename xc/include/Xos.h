/*
 * $XConsortium: Xos.h,v 1.30 91/01/08 15:16:38 gildea Exp $
 * 
 * Copyright 1987 by the Massachusetts Institute of Technology
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
 * The X Window System is a Trademark of MIT.
 *
 */

/* This is a collection of things to try and minimize system dependencies
 * in a "signficant" number of source files.
 */

#ifndef _XOS_H_
#define _XOS_H_

#ifdef _POSIX_SOURCE

#include <sys/types.h>
#include <string.h>
#define index strchr
#define rindex strrchr
#include <fcntl.h>
#include <unistd.h>

#else /* _POSIX_SOURCE */

/*
 * Get major data types (esp. caddr_t)
 */

#ifdef USG
#ifndef __TYPES__
#ifdef CRAY
#define word word_t
#endif
#include <sys/types.h>			/* forgot to protect it... */
#define __TYPES__
#ifdef CRAY
#undef word
#endif
#endif /* __TYPES__ */
#else
#include <sys/types.h>
#endif /* USG */


/*
 * Just about everyone needs the strings routines.  We provide both forms here,
 * index/rindex and strchr/strrchr, so any systems that don't provide them all
 * need to have #defines here.
 */

#ifdef SYSV
#define _X_POSIX_STYLE_STRING_H
#endif
#ifdef SVR4
#define _X_POSIX_STYLE_STRING_H
#endif

#ifdef _X_POSIX_STYLE_STRING_H
#include <string.h>
#define index strchr
#define rindex strrchr
#undef _X_POSIX_STYLE_STRING_H
#else
#include <strings.h>
#define strchr index
#define strrchr rindex
#endif


/*
 * Get open(2) constants
 */
#ifdef SYSV
#ifndef macII
#include <fcntl.h>
#endif
#endif /* SYSV */
#ifdef SVR4
#include <fcntl.h>
#include <unistd.h>
#endif
#include <sys/file.h>
#ifdef USG
#include <unistd.h>
#endif

#endif /* not _POSIX_SOURCE */

/*
 * Get struct timeval
 */

#ifdef SYSV
#if defined(sgi) || defined(CRAY) || defined(stellar)
#include <sys/time.h>				/* SYSV sys/time.h */
#ifdef CRAY
#include <time.h>
#define __TIMEVAL__
#endif
#else
#include <time.h>				/* else SYSV time.h */
#endif

#ifdef USG
#ifndef __TIMEVAL__
#define __TIMEVAL__
struct timeval {
    long tv_sec;
    long tv_usec;
};
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};
#endif /* __TIMEVAL__ */
#endif /* USG */

#ifdef macII
#include <sys/time.h>				/* SYSV && macII */
#endif

#else /* not SYSV */

/* (stupid SVR4) need to omit _POSIX_SOURCE in order to get what we want */
#ifdef SVR4
#ifdef _POSIX_SOURCE
#undef _POSIX_SOURCE
#define _XposixDEF
#endif
#endif

#include <sys/time.h>

#ifdef _XposixDEF
#define _POSIX_SOURCE
#undef _XposixDEF
#endif

#endif /* SYSV */

/* use POSIX name for signal */
#if defined(SYSV) && !defined(SIGCHLD)
#if !defined(macII) && !defined(ibm) && !defined(hpux)
#define SIGCHLD SIGCLD
#endif
#endif

#endif /* _XOS_H_ */
