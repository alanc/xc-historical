/* $XConsortium$ */
/******************************************************************************

Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Ralph Mor, X Consortium
******************************************************************************/

#include <X11/ICE/ICElib.h>
#include <X11/ICE/ICElibint.h>


/*
 * NOT IMPLEMENTED YET
 */


Status
IceInitThreads ()

{
    return (0);
}


void
IceLockConn (iceConn)

IceConn iceConn;

{
    LockIceConn (iceConn);
}


void
IceUnlockConn (iceConn)

IceConn iceConn;

{
    UnlockIceConn (iceConn);
}