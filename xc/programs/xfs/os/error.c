/* $XConsortium: error.c,v 1.6 92/05/29 18:04:45 gildea Exp $ */
/*
 * error message handling
 */
/*
 * Copyright 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include	<stdio.h>
#include	<X11/Xos.h>
#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif
#ifndef PATH_MAX
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif

#ifdef USE_SYSLOG
#include	<syslog.h>
#endif

#include	"misc.h"

Bool        UseSyslog;
char        ErrorFile[PATH_MAX];

static void
abort_server()
{
    fflush(stderr);

#ifdef SABER
    saber_stop();
#else
    abort();
#endif
}

void
InitErrors()
{
    int         i;

#ifdef USE_SYSLOG
    if (UseSyslog && !log_open) {
	openlog("Font Server", LOG_PID, LOG_LOCAL0);
	log_open = TRUE;
	return;
    }
#endif

    if (ErrorFile[0]) {
	i = creat(ErrorFile, 0666);
	if (i != -1) {
	    dup2(i, 2);
	    close(i);
	} else {
	    ErrorF("Can't open error file \"%s\"\n", ErrorFile);
	}
    }
}

void
CloseErrors()
{
#ifdef USE_SYSLOG
    if (UseSyslog) {
	closelog();
	log_open = FALSE;
	return;
    }
#endif
}

void
Error(str)
    char       *str;
{
    /* XXX this should also go to syslog() */
    perror(str);
}

/*
 * used for informational messages
 */
/* VARARGS1 */
void
NoticeF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)	/* limit of 10 args */
    char       *f;
    char       *s0,
               *s1,
               *s2,
               *s3,
               *s4,
               *s5,
               *s6,
               *s7,
               *s8,
               *s9;
{

#ifdef USE_SYSLOG
    if (UseSyslog) {
	syslog(LOG_NOTICE, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	return;
    }
#endif

    /* XXX should Notices just be ignored if not using syslog? */
    fprintf(stderr, "xfs notice: ");
    fprintf(stderr, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
}

/*
 * used for non-fatal error messages
 */
/* VARARGS1 */
void
ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)	/* limit of 10 args */
    char       *f;
    char       *s0,
               *s1,
               *s2,
               *s3,
               *s4,
               *s5,
               *s6,
               *s7,
               *s8,
               *s9;
{

#ifdef USE_SYSLOG
    if (UseSyslog) {
	syslog(LOG_ERR, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
	return;
    }
#endif

    fprintf(stderr, "xfs error: ");
    fprintf(stderr, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
}

/* VARARGS1 */
void
FatalError(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9)	/* limit of 10 args */
    char       *f;
    char       *s0,
               *s1,
               *s2,
               *s3,
               *s4,
               *s5,
               *s6,
               *s7,
               *s8,
               *s9;
{
    ErrorF("Fatal font server error:\n");
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
    abort_server();
    /* NOTREACHED */
}
