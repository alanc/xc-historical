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
 * $XConsortium: linktbl.c,v 1.2 92/06/30 18:14:09 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EAllwDvcEvnts;
extern struct linkinfo EChngDvcCntrl;
extern struct linkinfo EChngDvcDntPr;
extern struct linkinfo EChngFdbckCnt;
extern struct linkinfo EChngKybrdDvc;
extern struct linkinfo EChngPntrDvc;
extern struct linkinfo EChngDvcKyMpp;
extern struct linkinfo EClsDvc;
extern struct linkinfo EDvcBll;
extern struct linkinfo EGtDvcMdfrMpp;
extern struct linkinfo EGtFdbckCntrl;
extern struct linkinfo EGtExtnsnVrsn;
extern struct linkinfo EGrbDvc;
extern struct linkinfo EGrbDvcBttn;
extern struct linkinfo EGrbDvcKy;
extern struct linkinfo EGtDvcBttnMpp;
extern struct linkinfo EGtDvcCntrl;
extern struct linkinfo EGtDvcFcs;
extern struct linkinfo EGtDvcMtnEvnt;
extern struct linkinfo EGtDvcDntPrpg;
extern struct linkinfo EGtDvcKyMppng;
extern struct linkinfo EGtSlctdExtns;
extern struct linkinfo ELstInptDvcs;
extern struct linkinfo EMscllns;
extern struct linkinfo ESlctExtnsnEv;
extern struct linkinfo EOpnDvc;
extern struct linkinfo EQryDvcStt;
extern struct linkinfo EStDvcVltrs;
extern struct linkinfo EStDvcBttnMpp;
extern struct linkinfo EStDvcFcs;
extern struct linkinfo EStDvcMdfrMpp;
extern struct linkinfo EStDvcMd;
extern struct linkinfo ESndExtnsnEvn;
extern struct linkinfo EUngrbDvc;
extern struct linkinfo EUngrbDvcBttn;
extern struct linkinfo EUngrbDvcKy;

struct linkinfo *linktbl[] = {
        &EAllwDvcEvnts,
        &EChngDvcCntrl,
        &EChngDvcDntPr,
        &EChngFdbckCnt,
        &EChngKybrdDvc,
        &EChngPntrDvc,
        &EChngDvcKyMpp,
        &EClsDvc,
        &EDvcBll,
        &EGtDvcMdfrMpp,
        &EGtFdbckCntrl,
        &EGtExtnsnVrsn,
        &EGrbDvc,
        &EGrbDvcBttn,
        &EGrbDvcKy,
        &EGtDvcBttnMpp,
        &EGtDvcCntrl,
        &EGtDvcFcs,
        &EGtDvcMtnEvnt,
        &EGtDvcDntPrpg,
        &EGtDvcKyMppng,
        &EGtSlctdExtns,
        &ELstInptDvcs,
        &EMscllns,
        &ESlctExtnsnEv,
	&EOpnDvc,
	&EQryDvcStt,
	&EStDvcVltrs,
	&EStDvcBttnMpp,
	&EStDvcFcs,
	&EStDvcMdfrMpp,
	&EStDvcMd,
	&ESndExtnsnEvn,
	&EUngrbDvc,
	&EUngrbDvcBttn,
	&EUngrbDvcKy,
	0,
};
