/* $XConsortium: pexTM.c,v 5.3 94/04/17 20:36:09 hersh Exp $ */
/*

Copyright (c) 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


*/


#include "X.h"
#include "Xproto.h"
#include "pexError.h"
#include "PEXproto.h"
#include "dipex.h"
#include "pexLookup.h"

#ifdef min
#undef min
#endif
 
#ifdef max
#undef max
#endif



/*++	PEXCreateColorMipMapTM
 --*/
ErrorCode
PEXCreateColorMipMapTM (cntxtPtr, strmPtr)
pexContext              *cntxtPtr;
pexCreateColorMipMapTMReq     *strmPtr;
{
    ErrorCode err = Success;
    ErrorCode FreeColorMipMapTM (), CreateColorMipMapTM ();

    if (!LegalNewID(strmPtr->id, cntxtPtr->client))
	PEX_ERR_EXIT(BadIDChoice,strmPtr->id,cntxtPtr);


    err = CreateColorMipMapTM(nsptr);


    return( err );

} /* end-PEXCreateColorMipMapTM() */

/*++	PEXFreeTM
 --*/
ErrorCode
PEXFreeTM (cntxtPtr, strmPtr)
pexContext              *cntxtPtr;
pexFreeTMReq       *strmPtr;
{
    ErrorCode err = Success;

    LU_NAMESET(strmPtr->id, pns);

    FreeResource(strmPtr->id, RT_NONE);

    return(err);

} /* end-PEXFreeTM() */

/*++
 *
 * 	End of File
 --*/
