/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
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
 * $XConsortium: RcvExtErr.c,v 1.3 92/06/11 15:52:00 rws Exp $
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
 *	$Header: RcvExtErr.c,v 1.1 93/04/12 11:12:46 gms Exp $
 */

#ifndef lint
static char rcsid[]="$Header: RcvExtErr.c,v 1.1 93/04/12 11:12:46 gms Exp $";
#endif

#define ERROR_HEADER	4	/* size of constant header */
#define XInputNumErrors 5
#include "XstlibInt.h"

extern int XInputFirstError;

int
Rcv_Ext_Err(rp,rbuf,client)
xError *rp;
char rbuf[];
int client;
{
	int err;
	int needswap = Xst_clients[client].cl_swap;
	char *rbp = rbuf;
	int valid = 1;			/* assume all is OK */

	rbp += ERROR_HEADER;

	err = rp->errorCode - XInputFirstError;
	if (err>=0 && err < XInputNumErrors){
		switch (err) {
		case XI_BadDevice:
		case XI_BadMode:
		case XI_BadClass:
			((xError *)rp)->resourceID = unpack4(&rbp,needswap);
			((xError *)rp)->minorCode = unpack2(&rbp,needswap);
			((xError *)rp)->majorCode = unpack1(&rbp);
			break;
		default:
			DEFAULT_ERROR;
			break;
		}
	}
	else 	{
		DEFAULT_ERROR;
	}
	return valid;
}
