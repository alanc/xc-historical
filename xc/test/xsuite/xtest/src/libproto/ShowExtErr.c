/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology,
 * Hewlett-Packard, and UniSoft Group Limited.
 * 
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT, HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT, HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: ShowExtErr.c,v 1.3 92/06/11 15:52:00 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

/*
 *	$Header: ShowExtErr.c,v 1.1 93/04/12 11:15:00 gms Exp $
 */

#ifndef lint
static char rcsid[]="$Header: ShowExtErr.c,v 1.1 93/04/12 11:15:00 gms Exp $";
#endif

#define XInputNumErrors 5
#include "XstlibInt.h"

extern int XInputFirstError;

void
Show_Ext_Err(mp)
xError *mp;
{
int err;

	err = mp->errorCode - XInputFirstError;
	if (err>=XInputFirstError && err < (XInputFirstError+XInputNumErrors)){
		switch (err) {
		case XI_BadDevice:
			BPRINTF1("Device:\n");
			BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
			BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
			BPRINTF2("\tminorCode = %d\n",mp->minorCode);
			BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
			break;
		case XI_BadMode:
			BPRINTF1("Mode:\n");
			BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
			BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
			BPRINTF2("\tminorCode = %d\n",mp->minorCode);
			BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
			break;
		case XI_BadClass:
			BPRINTF1("Class:\n");
			BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
			BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
			BPRINTF2("\tminorCode = %d\n",mp->minorCode);
			BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
			break;
		default:
			BPRINTF1("UNKNOWN XINPUT EXTENSION ERROR TYPE:\n");
			BPRINTF2("\terrorCode = %ld\n",(long) mp->errorCode);
			BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
			BPRINTF2("\tminorCode = %d\n",mp->minorCode);
			BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
			break;
		}
	}
	else {
		BPRINTF1("UNKNOWN EXTENSION ERROR TYPE:\n");
		BPRINTF2("\terrorCode = %ld\n",(long) mp->errorCode);
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
	}
}
