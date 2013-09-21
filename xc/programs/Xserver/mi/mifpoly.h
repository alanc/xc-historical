/* $XConsortium: mifpoly.h,v 1.9 94/04/17 20:27:35 dpw Exp $ */
/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
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
#define ISEQUAL(a,b) (Fabs((a) - (b)) <= EPSILON)
#define UNEQUAL(a,b) (Fabs((a) - (b)) > EPSILON)
#define WITHINHALF(a, b) (((a) - (b) > 0.0) ? (a) - (b) < 0.5 : \
					     (b) - (a) <= 0.5)
#define ROUNDTOINT(x)   ((int) (((x) > 0.0) ? ((x) + 0.5) : ((x) - 0.5)))
#define ISZERO(x) 	(Fabs((x)) <= EPSILON)
#define PTISEQUAL(a,b) (ISEQUAL(a.x,b.x) && ISEQUAL(a.y,b.y))
#define PTUNEQUAL(a,b) (UNEQUAL(a.x,b.x) || UNEQUAL(a.y,b.y))
#define PtEqual(a, b) (((a).x == (b).x) && ((a).y == (b).y))

#define NotEnd		0
#define FirstEnd	1
#define SecondEnd	2

#define SQSECANT 108.856472512142 /* 1/sin^2(11/2) - for 11o miter cutoff */
#define D2SECANT 5.21671526231167 /* 1/2*sin(11/2) - max extension per width */

#ifdef NOINLINEICEIL
#define ICEIL(x) ((int)ceil(x))
#else
#ifdef __GNUC__
static __inline int ICEIL(x)
    double x;
{
    int _cTmp = x;
    return ((x == _cTmp) || (x < 0.0)) ? _cTmp : _cTmp+1;
}
#else
#define ICEIL(x) ((((x) == (_cTmp = (x))) || ((x) < 0.0)) ? _cTmp : _cTmp+1)
#define ICEILTEMPDECL static int _cTmp;
#endif
#endif

/* Point with sub-pixel positioning.  In this case we use doubles, but
 * see mifpolycon.c for other suggestions 
 */
typedef struct _SppPoint {
	double	x, y;
} SppPointRec, *SppPointPtr;

typedef struct _SppArc {
	double	x, y, width, height;
	double	angle1, angle2;
} SppArcRec, *SppArcPtr;

/* mifpolycon.c */

extern void miFillSppPoly(
#if NeedFunctionPrototypes
    DrawablePtr /*dst*/,
    GCPtr /*pgc*/,
    int /*count*/,
    SppPointPtr /*ptsIn*/,
    int /*xTrans*/,
    int /*yTrans*/,
    double /*xFtrans*/,
    double /*yFtrans*/
#endif
);
