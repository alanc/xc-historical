/* $XConsortium: pl_oc_util.h,v 1.10 92/05/07 23:31:09 mor Exp $ */

/************************************************************************
Copyright 1992 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

*************************************************************************/



/*
 * NAME:
 *	PEXCopyWordsToOC
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 *	_numWords	The number of words to copy.
 *
 *	_data		The data to copy.
 *
 * DESCRIPTION:
 *	This is a convenience macro which converts _numWords to a byte count
 *	and calls PEXCopyBytesToOC.
 */

#define PEXCopyWordsToOC(_display, _numWords, _data) \
    PEXCopyBytesToOC (_display, NUMBYTES (_numWords), _data)



/*
 * NAME:
 *	PEXFinishOC
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 * DESCRIPTION:
 *	This macro is the same as the PEXFinishOC function, but is used
 *      internally by PEXlib to avoid a function call.
 */

#define PEXFinishOC(_display) \
{ \
    UnlockDisplay (_display); \
}



/*
 * NAME:
 *	PEXAddSimpleOC
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 *	_resID		The resource ID of a renderer or structure.
 *
 *	_reqType	The OC request type
 *
 *	_ocType		The type of OC to encode.
 *
 *	_numBytes	The number of bytes after the element header.
 *
 *	_ocData		The data to encode.
 *
 * DESCRIPTION:
 *	This macro will encode a simple OC (an OC with no variable list data)
 *	into the X transport buffer.  It will fill in the OC element header
 *	and copy the additional data.
 */

#define PEXAddSimpleOC(_display, _resID, _reqType, _ocType, _numBytes, _ocData) \
{ \
    PEXDisplayInfo 	*pexDisplayInfo; \
    int 		ocLength = (PADDED_BYTES (_numBytes) + \
		    	sizeof (pexElementInfo)) >> 2; \
\
    PEXGetDisplayInfo (display, pexDisplayInfo); \
    if (PEXStartOCs (_display, _resID, _reqType, \
	pexDisplayInfo->fpFormat, 1, ocLength) == Success) \
    { \
        STORE_ELEMENT_INFO (_display->bufptr, _ocType, ocLength); \
        _display->bufptr += sizeof (pexElementInfo); \
        if (_numBytes > 0) \
	{ \
	    COPY_AREA (_ocData, _display->bufptr, _numBytes); \
	    _display->bufptr += PADDED_BYTES (_numBytes); \
	} \
        PEXFinishOC (_display); \
    } \
}



/*
 * NAME:
 *	PEXAddListOC
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 *	_resID		The resource ID of a renderer or structure.
 *
 *	_reqType	The OC request type
 *
 *	_ocType		The type of OC to encode.
 *
 *	_countNeeded	Flag to encode the count.
 *
 *	_count		The number of elements in the list.
 *
 *	_elementSize	The size (in bytes) of each element.
 *
 *	_elementList	The element list.
 *
 * DESCRIPTION:
 *	This macro will encode an OC containing a single list into the
 *	X transport buffer.  It will fill in the OC element header and
 *	copy the data list.
 */

#define PEXAddListOC(_display, _resID, _reqType, _ocType, _countNeeded, _count, _elementSize, _elementList) \
{ \
    PEXDisplayInfo 	*pexDisplayInfo; \
    int			ocListSize; \
    int         	ocHeaderBytes; \
    int			ocLength; \
    char		*pReq; \
\
    ocListSize = _count * _elementSize; \
    ocHeaderBytes = (_countNeeded) ? \
	sizeof (pexOCcListHeader) : sizeof (pexOCListHeader); \
    ocLength = NUMWORDS (ocListSize + ocHeaderBytes); \
\
    PEXGetDisplayInfo (display, pexDisplayInfo); \
    if (PEXStartOCs (_display, _resID, _reqType, \
	pexDisplayInfo->fpFormat, 1, ocLength) == Success) \
    { \
        STORE_ELEMENT_INFO (display->bufptr, _ocType, ocLength); \
    	pReq = (char *) display->bufptr; \
    	display->bufptr += ocHeaderBytes; \
	if (_countNeeded) \
	    ((pexOCcListHeader *) pReq)->length = _count; \
	_PEXCopyPaddedBytesToOC (_display, ocListSize, _elementList); \
	PEXFinishOC (_display); \
    } \
}




/*
 * NAME:
 *	PEXInitOC
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 *	_resID		The resource ID of a renderer or structure.
 *
 *	_reqType	The OC request type
 *
 *	_ocType		The type of OC to encode.
 *
 *	_headerLength	The number of words for the OC header.
 *
 *	_dataLength	The number of words for the OC data.
 *
 *	_pReqType	The data structure type of _pInfo (for casting).
 *
 *	_pReq		Return a pointer pointing after the OC header info.
 *
 * DESCRIPTION:
 *	This macro will initialize an OC encoding in the X transport buffer.
 *	It will fill in the OC element header and return a pointer in _pReq
 *	which points directly after the header.  PEXlib will copy the rest
 *	of the data into the X transport buffer.
 */

#define PEXInitOC(_display, _resID, _reqType, _ocType, _ocHeaderLength, _ocDataLength, _pReqType, _pReq) \
{ \
    PEXDisplayInfo 	*pexDisplayInfo; \
    int			ocLength = _ocHeaderLength + _ocDataLength; \
\
    _pReq = NULL; \
    PEXGetDisplayInfo (display, pexDisplayInfo); \
    if (PEXStartOCs (_display, _resID, _reqType, \
	pexDisplayInfo->fpFormat, 1, ocLength) == Success) \
    { \
        STORE_ELEMENT_INFO (display->bufptr, _ocType, ocLength); \
    	_pReq = (_pReqType *) (display->bufptr); \
    	display->bufptr += NUMBYTES (_ocHeaderLength); \
    } \
}



/*
 * NAME:
 *	PEXEncodeSimpleOC
 *
 * ARGUMENTS:
 *	_bufPtr		The OC will be encoded at this address.
 *
 *	_ocType		The type of OC to encode.
 *
 *	_numBytes	The number of bytes after the element header.
 *
 *	_ocData		The data to encode.
 *
 * DESCRIPTION:
 *	This macro will encode a simple OC (an OC with no variable list data)
 *	into an application buffer.  It will fill in the OC element header and
 *	copy the additional data.
 */

#define PEXEncodeSimpleOC(_bufPtr, _ocType, _numBytes, _ocData) \
{ \
    int	ocLength = (PADDED_BYTES (_numBytes) + \
		    sizeof (pexElementInfo)) >> 2; \
\
    STORE_ELEMENT_INFO (_bufPtr, _ocType, ocLength); \
    _bufPtr += sizeof (pexElementInfo); \
    if (_numBytes > 0) { \
	COPY_AREA (_ocData, _bufPtr, _numBytes); \
        _bufPtr += PADDED_BYTES (_numBytes); \
    } \
}



/*
 * NAME:
 *	PEXEncodeListOC
 *
 * ARGUMENTS:
 *	_bufPtr		The OC will be encoded at this address.
 *
 *	_ocType		The type of OC to encode.
 *
 *	_countNeeded	Flag to encode the count.
 *
 *	_count		The number of elements in the list.
 *
 *	_elementSize	The size (in bytes) of each element.
 *
 *	_elementList	The element list.
 *
 * DESCRIPTION:
 *	This macro will encode an OC containing a single list into an
 *	application buffer.  It will fill in the OC element header and
 *	copy the data list.
 */

#define PEXEncodeListOC(_bufPtr, _ocType, _countNeeded, _count, _elementSize, _elementList) \
{ \
    int         	ocListSize; \
    int         	ocHeaderLength; \
    int			ocLength; \
    char		*pReq; \
\
    ocListSize = _count * _elementSize; \
    ocHeaderLength = \
	(_countNeeded) ? LENOF (pexOCcListHeader) : LENOF (pexOCListHeader); \
    ocLength = NUMWORDS (ocListSize) + ocHeaderLength; \
\
    STORE_ELEMENT_INFO (_bufPtr, _ocType, ocLength); \
    pReq = (char *) _bufPtr; \
    _bufPtr += NUMBYTES (ocHeaderLength); \
    if (_countNeeded) \
	((pexOCcListHeader *) pReq)->length = _count; \
    if (ocListSize > 0) { \
       COPY_AREA (_elementList, _bufPtr, ocListSize); \
       _bufPtr += PADDED_BYTES (ocListSize); \
       } \
}



/*
 * NAME:
 *	PEXInitEncodeOC
 *
 * ARGUMENTS:
 *	_bufPtr		The OC will be encoded at this address.
 *
 *	_ocType		The type of OC to encode.
 *
 *	_headerLength	The number of words for the OC header.
 *
 *	_dataLength	The number of words for the OC data.
 *
 *	_pInfoType	The data structure type of _pInfo (for casting).
 *
 *	_pInfo		Return a pointer pointing after the OC header info.
 *
 * DESCRIPTION:
 *	This macro will initialize an OC encoding in an application buffer.
 *	It will fill in the OC element header and return a pointer in _pInfo
 *	which points directly after the header.  PEXlib will fill in the rest.
 */

#define PEXInitEncodeOC(_bufPtr, _ocType, _headerLength, _dataLength, _pInfoType, _pInfo) \
{ \
    int	ocLength = _headerLength + _dataLength; \
\
    STORE_ELEMENT_INFO (_bufPtr, _ocType, ocLength); \
    _bufPtr += sizeof (pexElementInfo); \
    _pInfo = (_pInfoType *) _bufPtr; \
}



/*
 * NAME:
 *	PEXEncodeBytes
 *
 * ARGUMENTS:
 *	_src		The source buffer pointer.
 *
 *	_dest		The destination buffer pointer.
 *
 *	_numBytes	The number of bytes to copy.
 *
 * DESCRIPTION:
 *	This macro will copy _numBytes from _src to _dest, advancing
 *	_dest by NUMWORDS (_numBytes).  This macro should be used
 *	after PEXInitEncodeOC is called.
 */

#define PEXEncodeBytes(_src, _dest, _numBytes) \
    COPY_AREA (_src, _dest, _numBytes); \
    _dest += PADDED_BYTES (_numBytes);



/*
 * NAME:
 *	StoreDefaultMonoString
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 *	_count		The number of characters in the string.
 *
 *	_string		The string.
 *
 * DESCRIPTION:
 *      Store a text string in the current oc requests as a mono encoded
 * 	string which uses character set 1 and byte character widths.
 */

#define StoreDefaultMonoString(_display, _count, _string) \
{ \
    pexMonoEncoding *_pMonoEncoding; \
\
    if (_pMonoEncoding = (pexMonoEncoding *) PEXGetOCAddr (_display, \
	sizeof (pexMonoEncoding))) \
    { \
        _pMonoEncoding->characterSet = (INT16) 1; \
        _pMonoEncoding->characterSetWidth = (CARD8) PEXCSByte; \
        _pMonoEncoding->encodingState = 0;   \
        _pMonoEncoding->numChars = (CARD16) (_count); \
    } \
\
    _PEXCopyPaddedBytesToOC (_display, _count, _string); \
}



/*
 * NAME:
 *	GetStringsLength
 *
 * ARGUMENTS:
 *	_numStrings	The number of strings in the mono encoded text.
 *
 *	_stringList	The list of strings in the mono encoded text.
 *
 *	_lenofStrings	Return the length of the strings.
 *
 * DESCRIPTION:
 * 	Compute length of mono-encoded strings.  For efficiency, this macro
 *	uses the following globals:
 *
 *      int 			i;
 *      PEXEncodedTextData      *nextString;
 */

#define GetStringsLength(_numStrings, _stringList, _lenofStrings) \
{ \
    (_lenofStrings) = 0; \
    nextString = (_stringList); \
    for (i = 0; i < (_numStrings); i++, nextString++) \
    { \
	lenofStrings += LENOF (pexMonoEncoding); \
	if (nextString->character_set_width == PEXCSLong) \
	    (_lenofStrings) += NUMWORDS (nextString->string.length * \
                                         sizeof (long)); \
	else if (nextString->character_set_width == PEXCSShort) \
	    (_lenofStrings) += NUMWORDS (nextString->string.length * \
                                         sizeof (short)); \
	else /* ( nextString->character_set_width == PEXCSByte) */ \
	    (_lenofStrings) += NUMWORDS (nextString->string.length); \
    } \
}



/*
 * NAME:
 *	StoreEncodedStrings
 *
 * ARGUMENTS:
 *	_display	The display pointer.
 *
 *	_numStrings	The number of strings in the mono encoded text.
 *
 *	_stringList	The mono encoded text strings.
 *
 * DESCRIPTION:
 * 	Store a mono encoded string in the current oc request.  For
 *	efficiency, this macro uses the following globals:
 *
 *    	int	       		i;
 *    	PEXEncodedTextData	*nextString;
 */

#define StoreEncodedStrings(_display, _numStrings, _stringList) \
{ \
    nextString = (_stringList); \
    for (i = 0; i < (_numStrings); i++, nextString++) \
    { \
	PEXCopyWordsToOC (_display, LENOF (pexMonoEncoding), \
            (char *) nextString); \
\
	if (nextString->character_set_width == PEXCSLong) \
	    _PEXCopyPaddedBytesToOC (_display, \
		nextString->string.length * sizeof (long), \
		(char *) nextString->string.ch); \
	else if (nextString->character_set_width == PEXCSShort) \
	    _PEXCopyPaddedBytesToOC (_display, \
		nextString->string.length * sizeof (short), \
		(char *) nextString->string.ch); \
	else /* nextString->character_set_width == PEXCSByte) */ \
	    _PEXCopyPaddedBytesToOC (_display, \
		nextString->string.length, \
		(char *) nextString->string.ch); \
    } \
}
