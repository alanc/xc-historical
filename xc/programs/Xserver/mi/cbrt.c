/* $XConsortium: cbrt.c,v 1.0 90/09/29 10:24:02 rws Exp $ */

/*
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* simple cbrt, in case your math library doesn't have a good one */

double pow();

double
cbrt(x)
    double x;
{
    return pow(x, 1.0/3.0);
}
