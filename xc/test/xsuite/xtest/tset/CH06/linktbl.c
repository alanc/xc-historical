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

extern struct linkinfo EXClrAr;
extern struct linkinfo EXClrWdw;
extern struct linkinfo EXCpyAr;
extern struct linkinfo EXCpyPln;
extern struct linkinfo EXCrtFntCrsr;
extern struct linkinfo EXCrtGlyphCrs;
extern struct linkinfo EXCrtPxmpCrsr;
extern struct linkinfo EXDfnCrsr;
extern struct linkinfo EXDrwArc;
extern struct linkinfo EXDrwArcs;
extern struct linkinfo EXDrwImgStr;
extern struct linkinfo EXDrwImgStr16;
extern struct linkinfo EXDrwLn;
extern struct linkinfo EXDrwLns;
extern struct linkinfo EXDrwPnt;
extern struct linkinfo EXDrwPnts;
extern struct linkinfo EXDrwRctngl;
extern struct linkinfo EXDrwRctngls;
extern struct linkinfo EXDrwSgmnts;
extern struct linkinfo EXDrwStr;
extern struct linkinfo EXDrwStr16;
extern struct linkinfo EXDrwTxt;
extern struct linkinfo EXDrwTxt16;
extern struct linkinfo EXFllArc;
extern struct linkinfo EXFllArcs;
extern struct linkinfo EXFllPlygn;
extern struct linkinfo EXFllRctngl;
extern struct linkinfo EXFllRctngls;
extern struct linkinfo EXFrCrsr;
extern struct linkinfo EXFrFnt;
extern struct linkinfo EXFrFntInf;
extern struct linkinfo EXFrFntNms;
extern struct linkinfo EXFrFntPth;
extern struct linkinfo EXGtFntPth;
extern struct linkinfo EXGtFntPrprty;
extern struct linkinfo EXGtImg;
extern struct linkinfo EXGtSbImg;
extern struct linkinfo EXLstFnts;
extern struct linkinfo EXLstFntsWthI;
extern struct linkinfo EXLdFnt;
extern struct linkinfo EXLdQryFnt;
extern struct linkinfo EXPtImg;
extern struct linkinfo EXQryBstCrsr;
extern struct linkinfo EXQryFnt;
extern struct linkinfo EXQryTxtExtnt;
extern struct linkinfo EXQryTxtExt16;
extern struct linkinfo EXRclrCrsr;
extern struct linkinfo EXStFntPth;
extern struct linkinfo EXTxtExtnts;
extern struct linkinfo EXTxtExtnts16;
extern struct linkinfo EXTxtWdth;
extern struct linkinfo EXTxtWdth16;
extern struct linkinfo EXUndfnCrsr;
extern struct linkinfo EXUnldFnt;

struct linkinfo *linktbl[] = {
	&EXClrAr,
	&EXClrWdw,
	&EXCpyAr,
	&EXCpyPln,
	&EXCrtFntCrsr,
	&EXCrtGlyphCrs,
	&EXCrtPxmpCrsr,
	&EXDfnCrsr,
	&EXDrwArc,
	&EXDrwArcs,
	&EXDrwImgStr,
	&EXDrwImgStr16,
	&EXDrwLn,
	&EXDrwLns,
	&EXDrwPnt,
	&EXDrwPnts,
	&EXDrwRctngl,
	&EXDrwRctngls,
	&EXDrwSgmnts,
	&EXDrwStr,
	&EXDrwStr16,
	&EXDrwTxt,
	&EXDrwTxt16,
	&EXFllArc,
	&EXFllArcs,
	&EXFllPlygn,
	&EXFllRctngl,
	&EXFllRctngls,
	&EXFrCrsr,
	&EXFrFnt,
	&EXFrFntInf,
	&EXFrFntNms,
	&EXFrFntPth,
	&EXGtFntPth,
	&EXGtFntPrprty,
	&EXGtImg,
	&EXGtSbImg,
	&EXLstFnts,
	&EXLstFntsWthI,
	&EXLdFnt,
	&EXLdQryFnt,
	&EXPtImg,
	&EXQryBstCrsr,
	&EXQryFnt,
	&EXQryTxtExtnt,
	&EXQryTxtExt16,
	&EXRclrCrsr,
	&EXStFntPth,
	&EXTxtExtnts,
	&EXTxtExtnts16,
	&EXTxtWdth,
	&EXTxtWdth16,
	&EXUndfnCrsr,
	&EXUnldFnt,
	0,
};
