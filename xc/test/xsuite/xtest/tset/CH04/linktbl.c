/* $XConsortium$ */

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
