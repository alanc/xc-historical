/* $XConsortium: XcmsInt.c,v 1.11 91/01/28 14:33:58 alt Exp $" */

/*
 * (c) Copyright 1990 1991, Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		XcmsMath.c - TekCMS Math Routines
 *
 *	DESCRIPTION
 *		Math routines provided in lieu of those provided
 *		in the math library (libm.a) so that device-independent
 *		color spaces that need them can be accessed by the color
 *		management system without the need of calling
 *		XcmsAddDIColorSpace().
 *
 *		CIELab -- requires cuberoot
 *		CIELuv -- requires cuberoot
 *		TekHVC -- requires cuberoot, squareroot, and arctangent
 *
 *
 */



/*
 *      EXTERNAL INCLUDES
 */

/*
 *      DEFINES
 */
#define MAXERROR 0.0001
#define MAXITER  10000
#define MYABS(z) ((z) < 0.0 ? -(z) : (z))



/************************************************************************
 *									*
 *			   API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		Xcms_CubeRoot
 *
 *	SYNOPSIS
 */
double
XcmsCubeRoot(x)
    double x;
/*
 *	DESCRIPTION
 *		Computes the cuberoot.
 *
 *	RETURNS
 *		Returns the cuberoot
 */
{
    double y;
    double maxerror;
    int i;

    if (x <= 0.0)  {
	return (0.0);
    }
    if (x == 1.0) {
	return (1.0);
    }
    if (x < 1.0) {
	maxerror = x * MAXERROR;
	y = x * 3.0;
    } else {
	maxerror = MAXERROR;
	y = x / 3.0;
    }
    for (i = 0; i < MAXITER; i++) {
	if ((MYABS((y*y*y) - x) < maxerror)) {
	    return (y);
	}
	y = (y + (((3 * x) / 2.0) / ((y * y) + ((x / 2.0) / y)))) / 2.0;
    }

    /* Max interations reached */
    return (y);
}


/*
 *	NAME
 *		Xcms_SquareRoot
 *
 *	SYNOPSIS
 */
double
XcmsSquareRoot(x)
    double x;
/*
 *	DESCRIPTION
 *		Computes the square root.
 *
 *	RETURNS
 *		Returns the square root
 */
{
    double y; 
    int i;
    double maxerror;

    if (x <= 0.0)  {
	return (0.0);
    }
    if (x == 1.0) {
	return (1.0);
    }
    if (x < 1.0) {
	maxerror = x * MAXERROR;
	y = x * 2.0;
    } else {
	maxerror = MAXERROR;
	y = x / 2.0;
    }
    for (i = 0; i < MAXITER; i++) {
	if (MYABS((y*y) - x) < maxerror) {
	    return (y);
	}
	y += (((x  / y) - y) / 2.0);
    }
    return (y);
}
