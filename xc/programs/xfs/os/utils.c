/* $XConsortium$ */
/*
 * misc os utilities
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)utils.c	4.4	5/6/91
 *
 */

#include	<stdio.h>
#include	<X11/Xos.h>
#include	<sys/param.h>
#include	"misc.h"
#include	"globals.h"

extern int  serverNum;
extern char *configfilename;
char       *progname;
Bool        UseSyslog;
Bool        CloneSelf;
char        ErrorFile[MAXPATHLEN];

AutoResetServer()
{

#ifdef DEBUG
    fprintf(stderr, "got a reset signal\n");
#endif

    dispatchException |= DE_RESET;
    isItTimeToYield = TRUE;
}

GiveUp()
{

#ifdef DEBUG
    fprintf(stderr, "got a TERM signal\n");
#endif

    dispatchException |= DE_TERMINATE;
    isItTimeToYield = TRUE;
}

ServerReconfig()
{

#ifdef DEBUG
    fprintf(stderr, "got a re-config signal\n");
#endif

    dispatchException |= DE_RECONFIG;
    isItTimeToYield = TRUE;
}

ServerCacheFlush()
{

#ifdef DEBUG
    fprintf(stderr, "got a flush signal\n");
#endif

    dispatchException |= DE_FLUSH;
    isItTimeToYield = TRUE;
}

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
Error(str)
    char       *str;
{
    perror(str);
}

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
    ErrorF("\nFatal server bug!\n");
    ErrorF(f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);
    ErrorF("\n");
    abort_server();
    /* NOTREACHED */
}

long
GetTimeInMillis()
{
    struct timeval tp;

    gettimeofday(&tp, 0);
    return ((tp.tv_sec * 1000) + (tp.tv_usec / 1000));
}

static void
usage()
{
    fprintf(stderr, "%s: [-cf config-file] [-s server_number]\n", progname);
    exit(-1);
}

/* ARGSUSED */
void
ProcessCmdLine(argc, argv)
    int         argc;
    char      **argv;
{
    int         i;

    progname = argv[0];
    for (i = 1; i < argc; i++) {
	if (!strncmp(argv[i], "-s", 2)) {
	    if (argv[i + 1])
		serverNum = atoi(argv[++i]);
	    else
		usage();
	} else if (!strncmp(argv[i], "-cf", 3)) {
	    if (argv[i + 1])
		configfilename = argv[++i];
	    else
		usage();
	} else
	    usage();
    }
}

unsigned long *
FSalloc(amount)
    unsigned long amount;
{
    pointer     ptr;

    if (!amount)

#ifdef undef
	return (unsigned long *) NULL;
#else
	amount++;		/* to deal with ALLOCATE_LOCAL(0) */
#endif

    amount = (amount + 3) & ~3;
    ptr = (pointer) malloc(amount);

#if defined(DEBUG) && !defined(SABER)
    bzero((char *) ptr, amount);
#endif

    return (unsigned long *) ptr;
}

unsigned long *
FScalloc(amount)
    unsigned long amount;
{
    unsigned long *ret;

    ret = FSalloc(amount);
    if (ret)
	bzero((char *) ret, (int) amount);
    return ret;
}

unsigned long *
FSrealloc(ptr, amount)
    pointer     ptr;
    unsigned long amount;
{
    if (!amount) {
	if (ptr)
	    free(ptr);
	return (unsigned long *) NULL;
    }
    amount = (amount + 3) & ~3;
    if (ptr)
	ptr = (pointer) realloc((char *) ptr, amount);
    else
	ptr = (pointer) malloc(amount);
    return (unsigned long *) ptr;
}

void
FSfree(ptr)
    pointer     ptr;
{
    if (ptr)
	free((char *) ptr);
}
