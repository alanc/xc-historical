/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: RcvErr.c,v 1.3 92/12/22 09:12:48 rws Exp $
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

#include "XstlibInt.h"
#include "DataMove.h"

#define ERROR_HEADER	4	/* size of constant header */

int
Rcv_Err(rp,rbuf,client)
xError *rp;
char rbuf[];
int client;
{
	int needswap = Xst_clients[client].cl_swap;
	char *rbp = rbuf;
	int valid = 1;			/* assume all is OK */

	rbp += ERROR_HEADER;

	if (rp->errorCode > FirstExtensionError) {
	    Rcv_Ext_Err(rp, rbuf, client);
	    return(valid);
	}

	switch (rp->errorCode) {
	case BadRequest:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadValue:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadWindow:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadPixmap:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadAtom:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadCursor:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadFont:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadMatch:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadDrawable:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadAccess:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadAlloc:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadColor:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadGC:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadIDChoice:
		((xError *)rp)->resourceID = unpack4(&rbp,needswap);
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadName:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadLength:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	case BadImplementation:
		rbp += 4;
		((xError *)rp)->minorCode = unpack2(&rbp,needswap);
		((xError *)rp)->majorCode = unpack1(&rbp);
		break;
	default:
		DEFAULT_ERROR;
		break;
	}
	return(valid);

}
