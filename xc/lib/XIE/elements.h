/* $XConsortium: elements.h,v 1.2 93/09/23 11:43:35 mor Exp $ */

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

#ifndef _ELEMENTS_H_
#define _ELEMENTS_H_

#if (__STDC__ && !defined(UNIXCPP)) || defined(ANSICPP)
#define ELEM_NAME(_name_) xieFlo##_name_
#define ELEM_SIZE(_name_) sz_xieFlo##_name_
#else
#define ELEM_NAME(_name_) xieFlo/**/_name_
#define ELEM_SIZE(_name_) sz_xieFlo/**/_name_
#endif


#ifndef WORD64

#define BEGIN_ELEM_HEAD(_name, _elemSrc, _elemLength, _bufDest, _elemDest) \
    _elemDest = (ELEM_NAME(_name) *) _bufDest; \
    _elemDest->elemType = elemSrc->elemType; \
    _elemDest->elemLength = _elemLength;

#define END_ELEM_HEAD(_name, _bufDest, _elemDest) \
    _bufDest += ELEM_SIZE(_name);

#else /* WORD64 */

#define BEGIN_ELEM_HEAD(_name, _elemSrc, _elemLength, _bufDest, _elemDest) \
{ \
    ELEM_NAME(_name) tElem; \
    _elemDest = &tElem; \
    _elemDest->elemType = elemSrc->elemType; \
    _elemDest->elemLength = _elemLength;

#define END_ELEM_HEAD(_name, _bufDest, _elemDest) \
    memcpy (_bufDest, _elemDest, ELEM_SIZE(_name)); \
    _bufDest += ELEM_SIZE(_name); \
}

#endif /* WORD64 */


#ifndef WORD64

#define STORE_LISTOF_TILES(_tiles, _count, _pBuf) \
{ \
    xieTypTile *tileDest = (xieTypTile *) _pBuf; \
    int     i; \
\
    for (i = 0; i < _count; i++) \
    { \
	tileDest->src  = _tiles[i].src; \
        tileDest->dstX = _tiles[i].dst_x; \
        tileDest->dstY = _tiles[i].dst_y; \
        tileDest++; \
    } \
    _pBuf += (SIZEOF (xieTypTile) * _count); \
}

#else /* WORD64 */

#define STORE_LISTOF_TILES(_tiles, _count, _pBuf) \
{ \
    xieTypTile temp; \
    int     i; \
\
    for (i = 0; i < _count; i++) \
    { \
	temp.src  = _tiles[i].src; \
        temp.dstX = _tiles[i].dst_x; \
        temp.dstY = _tiles[i].dst_y; \
	memcpy (_pBuf, &temp, SIZEOF (xieTypTile)); \
        _pBuf += SIZEOF (xieTypTile); \
    } \
}

#endif

#endif /* _ELEMENTS_H_ */
