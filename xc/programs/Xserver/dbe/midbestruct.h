/* $XConsortium: midbestruct.h,v 1.2 95/06/07 20:19:57 dpw Exp $ */
/******************************************************************************
 * 
 * Copyright (c) 1994, 1995  Hewlett-Packard Company
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Hewlett-Packard
 * Company shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Hewlett-Packard Company.
 * 
 * $Header: /x/programs/Xserver/dbe/RCS/midbestruct.h,v 1.2 95/06/07 20:19:57 dpw Exp $
 *
 *     Header file for users of machine-independent DBE code
 * 
 *****************************************************************************/


#ifndef MIDBE_STRUCT_H
#define MIDBE_STRUCT_H


/* DEFINES */

#define MI_DBE_WINDOW_PRIV_PRIV(pDbeWindowPriv) \
    (((miDbeWindowPrivPrivIndex < 0) || (!pDbeWindowPriv)) ? \
    NULL : \
    ((MiDbeWindowPrivPrivPtr) \
     ((pDbeWindowPriv)->devPrivates[miDbeWindowPrivPrivIndex].ptr)))

#define MI_DBE_WINDOW_PRIV_PRIV_FROM_WINDOW(pWin)\
    MI_DBE_WINDOW_PRIV_PRIV(DBE_WINDOW_PRIV(pWin))

#define MI_DBE_SCREEN_PRIV_PRIV(pDbeScreenPriv) \
    (((miDbeScreenPrivPrivIndex < 0) || (!pDbeScreenPriv)) ? \
    NULL : \
    ((MiDbeScreenPrivPrivPtr) \
     ((pDbeScreenPriv)->devPrivates[miDbeScreenPrivPrivIndex].ptr)))


/* TYPEDEFS */

typedef struct _MiDbeWindowPrivPrivRec
{
    /* Place machine-specific fields in here.
     * Since this is mi code, we do not have anything here except a pointer
     * back to the window private.
     */

    DbeWindowPrivPtr	pDbeWindowPriv;

} MiDbeWindowPrivPrivRec, *MiDbeWindowPrivPrivPtr;

typedef struct _MiDbeScreenPrivPrivRec
{
    /* Place machine-specific fields in here.
     * Since this is mi code, we do not have anything here except a pointer
     * back to the screen private.
     */

    DbeScreenPrivPtr	pDbeScreenPriv;

} MiDbeScreenPrivPrivRec, *MiDbeScreenPrivPrivPtr;

#endif /* MIDBE_STRUCT_H */

