/* $XHeader: Xos.h,v 1.4 88/07/19 18:53:19 jim Exp $ */
/* 
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

/*
 * Get major data types (esp. caddr_t)
 */

#include <sys/types.h>


/*
 * Just about everyone needs the strings routines.  For uniformity, we use
 * the BSD-style index() and rindex() in application code, so any systems that
 * don't provide them need to have #defines here.  Unfortunately, we can't
 * use #if defined() here since makedepend will get confused.
 */

#ifdef SYSV
#define SYSV_STRINGS
#endif /* SYSV */

#ifdef macII
#define SYSV_STRINGS
#endif /* macII */

#ifdef CRAY
#define SYSV_STRINGS
#endif /* CRAY */


#ifdef SYSV_STRINGS
#include <string.h>
#define index strchr
#define rindex strrchr
#undef SYSV_STRINGS
#else
#include <strings.h>
#endif /* SYSV_STRINGS */


/*
 * Get open(2) constants
 */

#ifdef SYSV
#include <fcntl.h>
#endif /* SYSV */
#include <sys/file.h>


/*
 * Get struct timeval
 */

#ifdef macII
#include <time.h>
#include <sys/time.h>
#else /* macII */
#ifdef SYSV
#include <time.h>
#else /* SYSV */
#include <sys/time.h>
#endif /* SYSV */
#endif /* macII */

/*
 * More BSDisms
 */

#ifdef SYSV
#define SIGCHLD SIGCLD
#endif /* SYSV */


/*
 * Put system-specific definitions here
 */

#ifdef hpux
#define sigvec sigvector
#endif /* hpux */

#endif /* _XOS_H_ */
