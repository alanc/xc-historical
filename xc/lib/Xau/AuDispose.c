/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include   "Xauth.h"

void
XauDisposeAuth (auth)
Xauth	*auth;
{
    auth->address ? free (auth->address) : 0;
    auth->number ? free (auth->number) : 0;
    auth->name ? free (auth->name) : 0;
    auth->data ? free (auth->data) : 0;
    free ((char *) auth);
}
