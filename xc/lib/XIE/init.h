/* $XConsortium: init.h,v 1.1 93/07/19 11:39:26 mor Exp $ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

#ifndef WORD64

#define GET_TECHNIQUE_REC(_pBuf, _techRec) \
    _techRec = (xieTypTechniqueRec *) _pBuf;

#else /* WORD64 */

#define GET_TECHNIQUE_REC(_pBuf, _techRec) \
    xieTypTechniqueRec temp; \
    memcpy (&temp, _pBuf, SIZEOF (xieTypTechniqueRec)); \
    _techRec = &temp;

#endif /* WORD64 */
