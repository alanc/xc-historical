/*
 * Copyright 1990, 1991, 1992 by the Massachusetts Institute of Technology and
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
 * $XConsortium: linktbl.c,v 1.1 92/06/11 16:12:15 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXChngPrprty;
extern struct linkinfo EXCnvrtSlctn;
extern struct linkinfo EXDltPrprty;
extern struct linkinfo EXGtAtmNm;
extern struct linkinfo EXGtGmtry;
extern struct linkinfo EXGtSlctnOwnr;
extern struct linkinfo EXGtWdwAttrbt;
extern struct linkinfo EXGtWdwPrprty;
extern struct linkinfo EXIntrnAtm;
extern struct linkinfo EXLstPrprts;
extern struct linkinfo EXQryPntr;
extern struct linkinfo EXQryTr;
extern struct linkinfo EXRttWdwPrprt;
extern struct linkinfo EXStSlctnOwnr;

struct linkinfo *linktbl[] = {
	&EXChngPrprty,
	&EXCnvrtSlctn,
	&EXDltPrprty,
	&EXGtAtmNm,
	&EXGtGmtry,
	&EXGtSlctnOwnr,
	&EXGtWdwAttrbt,
	&EXGtWdwPrprty,
	&EXIntrnAtm,
	&EXLstPrprts,
	&EXQryPntr,
	&EXQryTr,
	&EXRttWdwPrprt,
	&EXStSlctnOwnr,
	0,
};
