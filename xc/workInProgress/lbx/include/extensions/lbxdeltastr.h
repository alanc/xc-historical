/* $XConsortium: lbxdeltastr.h,v 1.2 95/03/16 18:25:34 mor Exp $ */
/*
 * $NCDId: @(#)lbxdeltastr.h,v 1.1 1993/12/22 22:23:09 dct Exp $
 *
 * Copyright 1993 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Dale Tonogai, Network Computing Devices
 */

#ifndef LBX_DELTA_STRUCT_H
#define LBX_DELTA_STRUCT_H

#define MIN_CACHEABLE_LEN		8

#define DELTA_CACHEABLE(pcache, len)	\
	((len) >= MIN_CACHEABLE_LEN && (len) <= (pcache)->maxDeltasize)

/*
 * An entry in the delta compactor cache
 */
typedef struct _LBXDeltaElem {
    int			length;
    unsigned char	*buf;
} LBXDeltaElemRec, *LBXDeltaElemPtr;


/*
 * Delta compactor cache
 */
typedef struct _LBXDeltas {
    unsigned short	nDeltas;
    unsigned short	maxDeltasize;
    LBXDeltaElemPtr	deltas;
    unsigned short	nextDelta;
    unsigned short	activeDeltas;	/* only needed for outgoing */
} LBXDeltasRec, *LBXDeltasPtr;


/*
 * Motion delta cache
 */

typedef struct {
    BYTE swapped;
    BYTE detail;
    CARD16 sequenceNumber B16;
    Time time B32;
    Window root B32, event B32, child B32;
    INT16 rootX B16, rootY B16, eventX B16, eventY B16;
    KeyButMask state B16;
    BOOL sameScreen;		
} lbxMotionCache;


/*
 * Motion delta events
 */

typedef struct {
    BYTE type;
    CARD8 deltaTime;
    INT8 deltaX;
    INT8 deltaY;
} lbxQuickMotionDeltaEvent;

typedef struct {
    BYTE type;
    INT8 deltaX;
    INT8 deltaY;
    BYTE pad;
    CARD16 deltaTime B16;
    CARD16 deltaSequence B16;
} lbxMotionDeltaEvent;

#endif /* LBX_DELTA_STRUCT_H */
