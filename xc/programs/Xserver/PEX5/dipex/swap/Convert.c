/* $XConsortium$ */

/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "pex_site.h"
#include "floatdef.h"


extern FLOAT SwapIEEEToVax();
extern FLOAT SwapVaxToIEEE();
extern FLOAT ConvertIEEEToVax();
extern FLOAT ConvertVaxToIEEE();

unsigned char temp;	/* only used for conversions */


/* Byte swap a long */
CARD32
SwapCARD32(i)
CARD32 *i;
{
    CARD32 *j = i;
    CARD8  n;
    CARD8  *x = (CARD8 *)j;

    n = x[0];
    x[0] = x[3];
    x[3] = n;
    n = x[1];
    x[1] = x[2];
    x[2] = n;

    return (*j);
}

/* Byte swap a short */
CARD16
SwapCARD16(i)
CARD16 *i;
{
    CARD16 *j = i;
    CARD8  n;
    CARD8  *x = (CARD8 *)j;

    n = x[0];
    x[0] = x[1];
    x[1] = n;

    return (*j);
}


/* Byte swap and convert a float */
FLOAT
SwapIEEEToVax(f)
FLOAT f;
{
    FLOAT x;
    CARD32 i;

    i = SwapCARD32((CARD32 *)(&f));

    x = ConvertIEEEToVax((FLOAT *)(&i));

    return (x);
}


FLOAT
SwapVaxToIEEE(f)
FLOAT f;
{
    FLOAT x;
    CARD32 i;

    i = SwapCARD32((CARD32 *)(&f));

    x = ConvertVaxToIEEE((FLOAT *)(&i));

    return (x);
}


