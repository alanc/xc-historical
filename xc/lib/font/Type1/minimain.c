/* $XConsortium: minimain.c,v 1.2 91/10/10 11:18:25 rws Exp $ */
/*
 * Copyright 1993 Massachusetts Institute of Technology
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
 
#include "ximager5.h"
 
main()
{
       XYspace S;
       path p;
 
       InitImager();
       S = Scale(IDENTITY, 300.0, -300.0);
       p = Join(Line(Loc(S, 0.0, 1.0)), Line(Loc(S, 1.0, 0.0)));
       Interior(ClosePath(p), EVENODDRULE);
}
 
void Trace()
{
}
 
void *DEFAULTDEVICE;
