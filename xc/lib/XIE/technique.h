/* $XConsortium: technique.h,v 1.2 93/09/23 11:43:41 mor Exp $ */

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

#define BEGIN_TECHNIQUE(_name, _bufDest, _dstParam) \
    _dstParam = (_name *) _bufDest;

#define END_TECHNIQUE(_name, _bufDest, _dstParam) \
    _bufDest += SIZEOF (_name);

#else /* WORD64 */

#define BEGIN_TECHNIQUE(_name, _bufDest, _dstParam) \
{ \
    _name tParam; \
    _dstParam = &tParam;

#define END_TECHNIQUE(_name, _bufDest, _dstParam) \
    memcpy (_bufDest, _dstParam, SIZEOF (_name)); \
    _bufDest += SIZEOF (_name); \
}

#endif /* WORD64 */



#ifndef WORD64

#define STORE_CARD32(_val, _pBuf) \
{ \
    *((CARD32 *) _pBuf) = _val; \
    _pBuf += SIZEOF (CARD32); \
}

#else /* WORD64 */

typedef struct {
    int value   :32;
} Long;

#define STORE_CARD32(_val, _pBuf) \
{ \
    Long _d; \
    _d.value = _val; \
    memcpy (_pBuf, &_d, SIZEOF (CARD32)); \
    _pBuf += SIZEOF (CARD32); \
}

#endif /* WORD64 */


#define _XieRGBToCIEXYZParam         _XieRGBToCIELabParam
#define _XieCIEXYZToRGBParam         _XieCIELabToRGBParam
#define _XieDecodeG32DParam          _XieDecodeG31DParam
#define _XieDecodeG42DParam          _XieDecodeG31DParam
#define _XieDecodeTIFF2Param         _XieDecodeG31DParam
#define _XieDecodeJPEGLosslessParam  _XieDecodeJPEGBaselineParam
