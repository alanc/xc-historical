/* $XConsortium: mifpoly.h,v 1.2 88/08/08 12:49:10 matt Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#define EPSILON	0.000001
#define ISEQUAL(a,b) (fabs((a) - (b)) <= EPSILON)
#define UNEQUAL(a,b) (fabs((a) - (b)) > EPSILON)
#define WITHINHALF(a, b) (((a) - (b) > 0.0) ? (a) - (b) < 0.5 : \
					     (b) - (a) <= 0.5)
#define ROUNDTOINT(x)   ((int) (((x) > 0.0) ? ((x) + 0.5) : ((x) - 0.5)))
#define ISZERO(x) 	(fabs((x)) <= EPSILON)
#define PTISEQUAL(a,b) (ISEQUAL(a.x,b.x) && ISEQUAL(a.y,b.y))
#define PTUNEQUAL(a,b) (UNEQUAL(a.x,b.x) || UNEQUAL(a.y,b.y))
#define PtEqual(a, b) (((a).x == (b).x) && ((a).y == (b).y))

#define NotEnd		0
#define FirstEnd	1
#define SecondEnd	2

#define SQSECANT 108.856472512142 /* 1/sin^2(11/2) - for 11o miter cutoff */
#define D2SECANT 5.21671526231167 /* 1/2*sin(11/2) - max extension per width */

/* Point with sub-pixel positioning.  In this case we use doubles, but
 * see mifpolycon.c for other suggestions 
 */
typedef struct _SppPoint {
	double	x, y;
} SppPointRec, *SppPointPtr;

extern SppPointRec miExtendSegment();
