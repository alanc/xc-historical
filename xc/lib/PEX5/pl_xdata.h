/* $XConsortium: pl_xdata.h,v 1.3 94/03/31 19:18:13 rws Exp $ */

/******************************************************************************
Copyright 1992 by the Massachusetts Institute of Technology

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

/*
 * Read data from the X buffer
 */

#ifndef WORD64

#define XREAD_LISTOF_CARD32(_display, _count, _pList) \
    _XRead (_display, (char *) _pList, (long) (SIZEOF (CARD32) * _count));


#define XREAD_LISTOF_CARD16(_display, _count, _pList) \
    _XRead (_display, (char *) _pList, (long) (SIZEOF (CARD16) * _count));


#define XREAD_LISTOF_FLOAT32(_display, _count, _pList, _fpConvert, _fpFormat) \
{ \
    if (!fpConvert) \
    { \
        _XRead (_display, (char *) _pList, (long) (_count * SIZEOF (float))); \
    } \
    else \
    { \
        char *_pBuf, *_pBufSave; \
        XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (float)); \
	_pBuf = _pBufSave; \
    	EXTRACT_LISTOF_FLOAT32 (_count, _pBuf, _pList, _fpConvert, _fpFormat); \
	FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (float)); \
    } \
}


#define XREAD_LISTOF_COORD3D(_display, _count, _pList, _fpConvert, _fpFormat) \
{ \
    if (!fpConvert) \
    { \
        _XRead (_display, (char *) _pList, \
	    (long) (_count * SIZEOF (pexCoord3D))); \
    } \
    else \
    { \
        char *_pBuf, *_pBufSave; \
        XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexCoord3D)); \
	_pBuf = _pBufSave; \
    	EXTRACT_LISTOF_COORD3D (_count, _pBuf, _pList, _fpConvert, _fpFormat); \
	FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexCoord3D)); \
    } \
}


#define XREAD_LISTOF_DEVCOORD(_display, _count, _pList, _fpConvert, _fpFormat)\
{ \
    if (!fpConvert) \
    { \
        _XRead (_display, (char *) _pList, \
	    (long) (_count * SIZEOF (pexDeviceCoord))); \
    } \
    else \
    { \
        char *_pBuf, *_pBufSave; \
        XREAD_INTO_SCRATCH (_display, _pBufSave, \
	    _count * SIZEOF (pexDeviceCoord)); \
	_pBuf = _pBufSave; \
        EXTRACT_LISTOF_DEVCOORD (_count, _pBuf, _pList, \
	    _fpConvert, _fpFormat); \
	FINISH_WITH_SCRATCH (_display, _pBufSave, \
	    _count * SIZEOF (pexDeviceCoord)); \
    } \
}


#define XREAD_LISTOF_DEVRECT(_display, _count, _pList) \
    _XRead (_display, (char *) _pList, \
        (long) (_count * SIZEOF (pexDeviceRect)));


#define XREAD_LISTOF_ELEMINFO(_display, _count, _pList) \
    _XRead (_display, (char *) _pList, \
        (long) (SIZEOF (pexElementInfo) * _count));


#define XREAD_LISTOF_ELEMREF(_display, _count, _pList) \
    _XRead (_display, (char *) _pList, \
        (long) (_count * SIZEOF (pexElementRef)));


#define XREAD_LISTOF_PICKELEMREF(_display, _count, _pList) \
    _XRead (_display, (char *) _pList, \
        (long) (_count * SIZEOF (pexPickElementRef)));


#else /* WORD64 */


#define XREAD_LISTOF_CARD32(_display, _count, _pList) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (CARD32)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_CARD32 (_count, _pBuf, _pList); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (CARD32)); \
}


#define XREAD_LISTOF_CARD16(_display, _count, _pList) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (CARD16)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_CARD16 (_count, _pBuf, _pList); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (CARD16)); \
}


#define XREAD_LISTOF_FLOAT32(_display, _count, _pList, _fpConvert, _fpFormat) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (float)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_FLOAT32 (_count, _pBuf, _pList, _fpConvert, _fpFormat); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (float)); \
}


#define XREAD_LISTOF_COORD3D(_display, _count, _pList, _fpConvert, _fpFormat) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexCoord3D)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_COORD3D (_count, _pBuf, _pList, _fpConvert, _fpFormat); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexCoord3D)); \
}


#define XREAD_LISTOF_DEVCOORD(_display, _count, _pList, _fpConvert, _fpFormat)\
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexDeviceCoord)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_DEVCOORD (_count, _pBuf, _pList, _fpConvert, _fpFormat); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexDeviceCoord)); \
}


#define XREAD_LISTOF_DEVRECT(_display, _count, _pList) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexDeviceRect)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_DEVRECT (_count, _pBuf, _pList); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexDeviceRect)); \
}


#define XREAD_LISTOF_ELEMINFO(_display, _count, _pList) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexElementInfo)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_ELEMINFO (_count, _pBuf, _pList); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexElementInfo)); \
}


#define XREAD_LISTOF_ELEMREF(_display, _count, _pList) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexElementRef)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_ELEMREF (_count, _pBuf, _pList); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, _count * SIZEOF (pexElementRef)); \
}


#define XREAD_LISTOF_PICKELEMREF(_display, _count, _pList) \
{ \
    char *_pBuf, *_pBufSave; \
    XREAD_INTO_SCRATCH (_display, _pBufSave, \
	_count * SIZEOF (pexPickElementRef)); \
    _pBuf = _pBufSave; \
    EXTRACT_LISTOF_PICKELEMREF (_count, _pBuf, _pList); \
    FINISH_WITH_SCRATCH (_display, _pBufSave, \
	_count * SIZEOF (pexPickElementRef)); \
}

#endif /* WORD64 */


/*
 * Send data to the X buffer
 */

#ifndef WORD64

#define XDATA_CARD32(_display, _val) \
    Data (_display, (char *) &_val, SIZEOF (CARD32));

#define XDATA_MONOENCODING(_display, _encoding) \
    Data (_display, (char *) _encoding, SIZEOF (pexMonoEncoding));

#else /* WORD64 */

#define XDATA_CARD32(_display, _val) \
{ \
    char tVal[4]; \
    CARD64_TO_32 (_val, tVal); \
    Data (_display, tVal, SIZEOF (CARD32)); \
}

#define XDATA_MONOENCODING(_display, _encoding) \
{ \
    pexMonoEncoding tEncoding; \
    tEncoding.characterSet = _encoding->character_set; \
    tEncoding.characterSetWidth = _encoding->character_set_width; \
    tEncoding.encodingState = _encoding->encoding_state; \
    tEncoding.numChars = _encoding->length; \
    Data (_display, (char *) &tEncoding, SIZEOF (pexMonoEncoding)); \
}

#endif /* WORD64 */
