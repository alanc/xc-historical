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

extern struct linkinfo EXAllwDvcEvnt;
extern struct linkinfo EXChngDvcCntr;
extern struct linkinfo EXChngFdbckCn;
extern struct linkinfo EXChngKybrdDv;
extern struct linkinfo EXChngDvcDntP;
extern struct linkinfo EXChngPntrDvc;
extern struct linkinfo EXChngDvcKyMp;
extern struct linkinfo EXClsDvc;
extern struct linkinfo EXDvcBll;
extern struct linkinfo EXGtDvcCntrl;
extern struct linkinfo EXGtFdbckCntr;
extern struct linkinfo EXGtDvcFcs;
extern struct linkinfo EXGtDvcDntPrp;
extern struct linkinfo EXGtExtnsnVrs;
extern struct linkinfo EXGrbDvc;
extern struct linkinfo EXGrbDvcBttn;
extern struct linkinfo EXGrbDvcKy;
extern struct linkinfo EXGtDvcBttnMp;
extern struct linkinfo EXGtDvcKyMppn;
extern struct linkinfo EXGtDvcMdfrMp;
extern struct linkinfo EXGtDvcMtnEvn;
extern struct linkinfo EXGtSlctdExtn;
extern struct linkinfo EXLstInptDvcs;
extern struct linkinfo EMscllns;
extern struct linkinfo EXOpnDvc;
extern struct linkinfo EXQryDvcStt;
extern struct linkinfo EXStDvcVltrs;
extern struct linkinfo EXStDvcBttnMp;
extern struct linkinfo EXStDvcFcs;
extern struct linkinfo EXStDvcMdfrMp;
extern struct linkinfo EXStDvcMd;
extern struct linkinfo EXSlctExtnsnE;
extern struct linkinfo EXSndExtnsnEv;
extern struct linkinfo EXUngrbDvc;
extern struct linkinfo EXUngrbDvcBtt;
extern struct linkinfo EXUngrbDvcKy;

struct linkinfo *linktbl[] = {
        &EXAllwDvcEvnt,
        &EXChngDvcCntr,
        &EXChngFdbckCn,
        &EXChngKybrdDv,
        &EXChngDvcDntP,
        &EXChngPntrDvc,
        &EXChngDvcKyMp,
        &EXClsDvc,
        &EXDvcBll,
        &EXGtDvcCntrl,
        &EXGtFdbckCntr,
        &EXGtDvcFcs,
        &EXGtDvcDntPrp,
        &EXGtExtnsnVrs,
        &EXGrbDvc,
        &EXGrbDvcBttn,
        &EXGrbDvcKy,
        &EXGtDvcBttnMp,
        &EXGtDvcKyMppn,
        &EXGtDvcMdfrMp,
        &EXGtDvcMtnEvn,
        &EXGtSlctdExtn,
        &EXLstInptDvcs,
        &EMscllns,
        &EXOpnDvc,
        &EXQryDvcStt,
        &EXStDvcVltrs,
        &EXStDvcBttnMp,
        &EXStDvcFcs,
        &EXStDvcMdfrMp,
        &EXStDvcMd,
        &EXSlctExtnsnE,
        &EXSndExtnsnEv,
        &EXUngrbDvc,
        &EXUngrbDvcBtt,
        &EXUngrbDvcKy,

	0,
};
