#include	"copyright.h"
/* @(#)FSFlush.c	4.1	91/05/02
 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 */

#include	"FSlibint.h"

FSFlush(svr)
    FSServer     *svr;
{
    _FSFlush(svr);
}
