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

extern struct linkinfo EXChngWdwAttr;
extern struct linkinfo EXCrcltSbws;
extern struct linkinfo EXCrcltSbwsDw;
extern struct linkinfo EXCrcltSbwsUp;
extern struct linkinfo EXCnfgrWdw;
extern struct linkinfo EXCrtSmplWdw;
extern struct linkinfo EXCrtWdw;
extern struct linkinfo EXDstrySbws;
extern struct linkinfo EXDstryWdw;
extern struct linkinfo EXLwrWdw;
extern struct linkinfo EXMpRsd;
extern struct linkinfo EXMpSbws;
extern struct linkinfo EXMpWdw;
extern struct linkinfo EXMvRszWdw;
extern struct linkinfo EXMvWdw;
extern struct linkinfo EXRsWdw;
extern struct linkinfo EXRszWdw;
extern struct linkinfo EXRstckWdws;
extern struct linkinfo EXStWdwBg;
extern struct linkinfo EXStWdwBgPxmp;
extern struct linkinfo EXStWdwBrdr;
extern struct linkinfo EXStWdwBrdrPx;
extern struct linkinfo EXStWdwBrdrWd;
extern struct linkinfo EXTrnsltCrdnt;
extern struct linkinfo EXUnmpSbws;
extern struct linkinfo EXUnmpWdw;
extern struct linkinfo EXVslIDFrmVsl;

struct linkinfo *linktbl[] = {
	&EXChngWdwAttr,
	&EXCrcltSbws,
	&EXCrcltSbwsDw,
	&EXCrcltSbwsUp,
	&EXCnfgrWdw,
	&EXCrtSmplWdw,
	&EXCrtWdw,
	&EXDstrySbws,
	&EXDstryWdw,
	&EXLwrWdw,
	&EXMpRsd,
	&EXMpSbws,
	&EXMpWdw,
	&EXMvRszWdw,
	&EXMvWdw,
	&EXRsWdw,
	&EXRszWdw,
	&EXRstckWdws,
	&EXStWdwBg,
	&EXStWdwBgPxmp,
	&EXStWdwBrdr,
	&EXStWdwBrdrPx,
	&EXStWdwBrdrWd,
	&EXTrnsltCrdnt,
	&EXUnmpSbws,
	&EXUnmpWdw,
	&EXVslIDFrmVsl,
	0,
};
