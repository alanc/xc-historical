#include "copyright.h"

/* $Header: XChPntCon.c,v 11.6 87/05/24 21:32:54 jg Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#include "Xlibint.h"

XChangePointerControl(dpy, do_acc, do_thresh, acc_numerator,
		      acc_denominator, threshold)
     register Display *dpy;
     int do_acc, do_thresh;  /*boolean*/
     int acc_numerator, acc_denominator, threshold;

{
    register xChangePointerControlReq *req;

    LockDisplay(dpy);
    GetReq(ChangePointerControl, req);
    req->doAccel = do_acc;
    req->doThresh = do_thresh;
    req->accelNum = acc_numerator;
    req->accelDenum = acc_denominator;
    req->threshold = threshold;
    UnlockDisplay(dpy);
    SyncHandle();
}

