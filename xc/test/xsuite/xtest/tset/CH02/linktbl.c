/* $XConsortium$ */

#include "xtest.h"

extern struct linkinfo EXAllPlns;
extern struct linkinfo EXBtmpBtOrdr;
extern struct linkinfo EXBtmpPd;
extern struct linkinfo EXBtmpUnt;
extern struct linkinfo EXBlckPxl;
extern struct linkinfo EXBlckPxlOfSc;
extern struct linkinfo EXCllsOfScrn;
extern struct linkinfo EXClsDsply;
extern struct linkinfo EXCnnctnNmbr;
extern struct linkinfo EXDfltClrmp;
extern struct linkinfo EXDfltClrmpOf;
extern struct linkinfo EXDfltDpth;
extern struct linkinfo EXDfltDpthOfS;
extern struct linkinfo EXDfltGC;
extern struct linkinfo EXDfltGCOfScr;
extern struct linkinfo EXDfltRtWdw;
extern struct linkinfo EXDfltScrn;
extern struct linkinfo EXDfltScrnOfD;
extern struct linkinfo EXDfltVsl;
extern struct linkinfo EXDfltVslOfSc;
extern struct linkinfo EXDsplyClls;
extern struct linkinfo EXDsplyHght;
extern struct linkinfo EXDsplyHghtMM;
extern struct linkinfo EXDsplyOfScrn;
extern struct linkinfo EXDsplyPlns;
extern struct linkinfo EXDsplyStr;
extern struct linkinfo EXDsplyWdth;
extern struct linkinfo EXDsplyWdthMM;
extern struct linkinfo EXDsBckngStr;
extern struct linkinfo EXDsSvUndrs;
extern struct linkinfo EXEvntMskOfSc;
extern struct linkinfo EXFr;
extern struct linkinfo EXHghtMMOfScr;
extern struct linkinfo EXHghtOfScrn;
extern struct linkinfo EXImgBytOrdr;
extern struct linkinfo EXLstKnwnRqst;
extern struct linkinfo EXLstDpths;
extern struct linkinfo EXLstPxmpFrmt;
extern struct linkinfo EXMxCmpsOfScr;
extern struct linkinfo EXMnCmpsOfScr;
extern struct linkinfo EXNxtRqst;
extern struct linkinfo EXNOp;
extern struct linkinfo EXOpnDsply;
extern struct linkinfo EXPlnsOfScrn;
extern struct linkinfo EXPrtclRvsn;
extern struct linkinfo EXPrtclVrsn;
extern struct linkinfo EXQLngth;
extern struct linkinfo EXRtWdw;
extern struct linkinfo EXRtWdwOfScrn;
extern struct linkinfo EXScrnCnt;
extern struct linkinfo EXScrnNmbrOfS;
extern struct linkinfo EXScrnOfDsply;
extern struct linkinfo EXSrvrVndr;
extern struct linkinfo EXVndrRls;
extern struct linkinfo EXWhtPxl;
extern struct linkinfo EXWhtPxlOfScr;
extern struct linkinfo EXWdthMMOfScr;
extern struct linkinfo EXWdthOfScrn;

extern struct linkinfo EBtmpBtOrdr;
extern struct linkinfo EBtmpPd;
extern struct linkinfo EBtmpUnt;
extern struct linkinfo EBlckPxl;
extern struct linkinfo EBlckPxlOfScr;
extern struct linkinfo ECllsOfScrn;
extern struct linkinfo ECnnctnNmbr;
extern struct linkinfo EDfltClrmp;
extern struct linkinfo EDfltClrmpOfS;
extern struct linkinfo EDfltDpth;
extern struct linkinfo EDfltDpthOfSc;
extern struct linkinfo EDfltGC;
extern struct linkinfo EDfltGCOfScrn;
extern struct linkinfo EDfltRtWdw;
extern struct linkinfo EDfltScrn;
extern struct linkinfo EDfltScrnOfDs;
extern struct linkinfo EDfltVsl;
extern struct linkinfo EDfltVslOfScr;
extern struct linkinfo EDsplyClls;
extern struct linkinfo EDsplyHght;
extern struct linkinfo EDsplyHghtMM;
extern struct linkinfo EDsplyOfScrn;
extern struct linkinfo EDsplyPlns;
extern struct linkinfo EDsplyStr;
extern struct linkinfo EDsplyWdth;
extern struct linkinfo EDsplyWdthMM;
extern struct linkinfo EDsBckngStr;
extern struct linkinfo EDsSvUndrs;
extern struct linkinfo EEvntMskOfScr;
extern struct linkinfo EHghtMMOfScrn;
extern struct linkinfo EHghtOfScrn;
extern struct linkinfo EImgBytOrdr;
extern struct linkinfo ELstKnwnRqstP;
extern struct linkinfo EMxCmpsOfScrn;
extern struct linkinfo EMnCmpsOfScrn;
extern struct linkinfo ENxtRqst;
extern struct linkinfo EPlnsOfScrn;
extern struct linkinfo EPrtclRvsn;
extern struct linkinfo EPrtclVrsn;
extern struct linkinfo EQLngth;
extern struct linkinfo ERtWdw;
extern struct linkinfo ERtWdwOfScrn;
extern struct linkinfo EScrnCnt;
extern struct linkinfo EScrnOfDsply;
extern struct linkinfo ESrvrVndr;
extern struct linkinfo EVndrRls;
extern struct linkinfo EWhtPxl;
extern struct linkinfo EWhtPxlOfScrn;
extern struct linkinfo EWdthMMOfScrn;
extern struct linkinfo EWdthOfScrn;

struct linkinfo *linktbl[] = {
	&EXAllPlns,
	&EXBtmpBtOrdr,
	&EXBtmpPd,
	&EXBtmpUnt,
	&EXBlckPxl,
	&EXBlckPxlOfSc,
	&EXCllsOfScrn,
	&EXClsDsply,
	&EXCnnctnNmbr,
	&EXDfltClrmp,
	&EXDfltClrmpOf,
	&EXDfltDpth,
	&EXDfltDpthOfS,
	&EXDfltGC,
	&EXDfltGCOfScr,
	&EXDfltRtWdw,
	&EXDfltScrn,
	&EXDfltScrnOfD,
	&EXDfltVsl,
	&EXDfltVslOfSc,
	&EXDsplyClls,
	&EXDsplyHght,
	&EXDsplyHghtMM,
	&EXDsplyOfScrn,
	&EXDsplyPlns,
	&EXDsplyStr,
	&EXDsplyWdth,
	&EXDsplyWdthMM,
	&EXDsBckngStr,
	&EXDsSvUndrs,
	&EXEvntMskOfSc,
	&EXFr,
	&EXHghtMMOfScr,
	&EXHghtOfScrn,
	&EXImgBytOrdr,
	&EXLstKnwnRqst,
	&EXLstDpths,
	&EXLstPxmpFrmt,
	&EXMxCmpsOfScr,
	&EXMnCmpsOfScr,
	&EXNxtRqst,
	&EXNOp,
	&EXOpnDsply,
	&EXPlnsOfScrn,
	&EXPrtclRvsn,
	&EXPrtclVrsn,
	&EXQLngth,
	&EXRtWdw,
	&EXRtWdwOfScrn,
	&EXScrnCnt,
	&EXScrnNmbrOfS,
	&EXScrnOfDsply,
	&EXSrvrVndr,
	&EXVndrRls,
	&EXWhtPxl,
	&EXWhtPxlOfScr,
	&EXWdthMMOfScr,
	&EXWdthOfScrn,

	&EBtmpBtOrdr,
	&EBtmpPd,
	&EBtmpUnt,
	&EBlckPxl,
	&EBlckPxlOfScr,
	&ECllsOfScrn,
	&ECnnctnNmbr,
	&EDfltClrmp,
	&EDfltClrmpOfS,
	&EDfltDpth,
	&EDfltDpthOfSc,
	&EDfltGC,
	&EDfltGCOfScrn,
	&EDfltRtWdw,
	&EDfltScrn,
	&EDfltScrnOfDs,
	&EDfltVsl,
	&EDfltVslOfScr,
	&EDsplyClls,
	&EDsplyHght,
	&EDsplyHghtMM,
	&EDsplyOfScrn,
	&EDsplyPlns,
	&EDsplyStr,
	&EDsplyWdth,
	&EDsplyWdthMM,
	&EDsBckngStr,
	&EDsSvUndrs,
	&EEvntMskOfScr,
	&EHghtMMOfScrn,
	&EHghtOfScrn,
	&EImgBytOrdr,
	&ELstKnwnRqstP,
	&EMxCmpsOfScrn,
	&EMnCmpsOfScrn,
	&ENxtRqst,
	&EPlnsOfScrn,
	&EPrtclRvsn,
	&EPrtclVrsn,
	&EQLngth,
	&ERtWdw,
	&ERtWdwOfScrn,
	&EScrnCnt,
	&EScrnOfDsply,
	&ESrvrVndr,
	&EVndrRls,
	&EWhtPxl,
	&EWhtPxlOfScrn,
	&EWdthMMOfScrn,
	&EWdthOfScrn,

	0,
};
