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
 * $XConsortium$
 */

#include "xtest.h"

extern struct linkinfo EXChckIfEvnt;
extern struct linkinfo EXChckMskEvnt;
extern struct linkinfo EXChckTypdEvn;
extern struct linkinfo EXChckTypdWdw;
extern struct linkinfo EXChckWdwEvnt;
extern struct linkinfo EXDsplyMtnBff;
extern struct linkinfo EXDsplyNm;
extern struct linkinfo EXEvntsQd;
extern struct linkinfo EXFlsh;
extern struct linkinfo EXGtErrrDtbsT;
extern struct linkinfo EXGtErrrTxt;
extern struct linkinfo EXGtMtnEvnts;
extern struct linkinfo EXIfEvnt;
extern struct linkinfo EXMskEvnt;
extern struct linkinfo EXNxtEvnt;
extern struct linkinfo EXPkEvnt;
extern struct linkinfo EXPkIfEvnt;
extern struct linkinfo EXPndng;
extern struct linkinfo EXPtBckEvnt;
extern struct linkinfo EXSlctInpt;
extern struct linkinfo EXSndEvnt;
extern struct linkinfo EXStAftrFnctn;
extern struct linkinfo EXStErrrHndlr;
extern struct linkinfo EXStIOErrrHnd;
extern struct linkinfo EXSync;
extern struct linkinfo EXSynchrnz;
extern struct linkinfo EXWdwEvnt;
extern struct linkinfo EBttnPrss;
extern struct linkinfo EBttnRls;
extern struct linkinfo ECrcltNtfy;
extern struct linkinfo ECrcltRqst;
extern struct linkinfo EClntMssg;
extern struct linkinfo EClrmpNtfy;
extern struct linkinfo ECnfgrNtfy;
extern struct linkinfo ECnfgrRqst;
extern struct linkinfo ECrtNtfy;
extern struct linkinfo EDstryNtfy;
extern struct linkinfo EEntrNtfy;
extern struct linkinfo EExps;
extern struct linkinfo EFcsIn;
extern struct linkinfo EFcsOt;
extern struct linkinfo EGrphcsExps;
extern struct linkinfo EGrvtyNtfy;
extern struct linkinfo EKyPrss;
extern struct linkinfo EKyRls;
extern struct linkinfo EKympNtfy;
extern struct linkinfo ELvNtfy;
extern struct linkinfo EMpNtfy;
extern struct linkinfo EMpRqst;
extern struct linkinfo EMppngNtfy;
extern struct linkinfo EMtnNtfy;
extern struct linkinfo ENExps;
extern struct linkinfo EPrprtyNtfy;
extern struct linkinfo ERprntNtfy;
extern struct linkinfo ERszRqst;
extern struct linkinfo ESlctnClr;
extern struct linkinfo ESlctnNtfy;
extern struct linkinfo ESlctnRqst;
extern struct linkinfo EUnmpNtfy;
extern struct linkinfo EVsbltyNtfy;

struct linkinfo *linktbl[] = {
	&EXChckIfEvnt,
	&EXChckMskEvnt,
	&EXChckTypdEvn,
	&EXChckTypdWdw,
	&EXChckWdwEvnt,
	&EXDsplyMtnBff,
	&EXDsplyNm,
	&EXEvntsQd,
	&EXFlsh,
	&EXGtErrrDtbsT,
	&EXGtErrrTxt,
	&EXGtMtnEvnts,
	&EXIfEvnt,
	&EXMskEvnt,
	&EXNxtEvnt,
	&EXPkEvnt,
	&EXPkIfEvnt,
	&EXPndng,
	&EXPtBckEvnt,
	&EXSlctInpt,
	&EXSndEvnt,
	&EXStAftrFnctn,
	&EXStErrrHndlr,
	&EXStIOErrrHnd,
	&EXSync,
	&EXSynchrnz,
	&EXWdwEvnt,
	&EBttnPrss,
	&EBttnRls,
	&ECrcltNtfy,
	&ECrcltRqst,
	&EClntMssg,
	&EClrmpNtfy,
	&ECnfgrNtfy,
	&ECnfgrRqst,
	&ECrtNtfy,
	&EDstryNtfy,
	&EEntrNtfy,
	&EExps,
	&EFcsIn,
	&EFcsOt,
	&EGrphcsExps,
	&EGrvtyNtfy,
	&EKyPrss,
	&EKyRls,
	&EKympNtfy,
	&ELvNtfy,
	&EMpNtfy,
	&EMpRqst,
	&EMppngNtfy,
	&EMtnNtfy,
	&ENExps,
	&EPrprtyNtfy,
	&ERprntNtfy,
	&ERszRqst,
	&ESlctnClr,
	&ESlctnNtfy,
	&ESlctnRqst,
	&EUnmpNtfy,
	&EVsbltyNtfy,
	0,
};
