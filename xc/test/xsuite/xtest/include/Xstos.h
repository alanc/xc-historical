/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: Xstos.h,v 1.5 92/06/11 15:31:47 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/* Header file for UNIX library for X Server tests. */

/*
 * System header files.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

/*
 * System calls, library functions and other definitions in UNIX/DYNIX.
 */


/* Comment out the unecessary (or wrong) definitions */

extern int errno;

#ifdef NOT_REQUIRED

char	*alloca();
/* char	*calloc(); */
void	exit();
int	fclose();
/* void	fflush(); */
FILE	*fopen();
int	fprintf();
void	free();
int	fscanf();
int	getitimer();
char	*index();
/* char	*malloc(); */
void	perror();
/* char	*realloc(); */
int	setitimer();
int	sigblock();
/* int	(*signal())(); */
int	sigsetmask();
char	*strcat();
int	strcmp();
char	*strncat();
char	*strncpy();

void	bcopy();
void    bzero();

#endif /* NOT_REQUIRED */

#define SearchString(string, char) strchr((string), (char))


/*
 * Note that some machines do not return a valid pointer for malloc(0), in
 * which case we provide an alternate under the control of the
 * define MALLOC_0_RETURNS_NULL.  This is necessary because some
 * Xst code expects malloc(0) to return a valid pointer to storage.
 */
#define EXTRA 16
#ifdef MALLOC_0_RETURNS_NULL

# define Xstmalloc(size) malloc(max((size)+EXTRA,1))
# define Xstrealloc(ptr, size) realloc((ptr), max((size)+EXTRA,1))
# define Xstcalloc(nelem, elsize) calloc(max((nelem)+EXTRA,1), (elsize))

#else

# define Xstmalloc(size) malloc((size)+EXTRA)
# define Xstrealloc(ptr, size) realloc((ptr), (size)+EXTRA)
# define Xstcalloc(nelem, elsize) calloc((nelem)+EXTRA, (elsize))

#endif
