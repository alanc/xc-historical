/* $XConsortium: pl_lut.h,v 1.1 93/02/23 14:39:03 mor Exp $ */

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

#ifndef WORD64

#define BEGIN_LUTENTRY_HEADER(_name, _pBuf, _pEntry) \
    _pEntry = (_name *) _pBuf; 

#define END_LUTENTRY_HEADER(_name, _pBuf, _pEntry) \
    _pBuf += SIZEOF (_name);

#else /* WORD64 */

#define BEGIN_LUTENTRY_HEADER(_name, _pBuf, _pEntry) \
{ \
    _name tEntry; \
    _pEntry = &tEntry;

#define END_LUTENTRY_HEADER(_name, _pBuf, _pEntry) \
    memcpy (_pBuf, _pEntry, SIZEOF (_name)); \
    _pBuf += SIZEOF (_name); \
}

#endif /* WORD64 */


static PEXPointer _PEXRepackLUTEntries();

#define GetLUTEntryBuffer(_numEntries, _entryType, _buf) \
    (_buf) = (PEXPointer) PEXAllocBuf ( \
	(_numEntries) * (sizeof (_entryType)));



