/* $XConsortium: ICEproto.h,v 1.2 93/11/18 11:15:29 mor Exp $ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

#ifndef ICEPROTO_H
#define ICEPROTO_H

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	data[2];
    CARD32	length B32;
} iceMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD16	errorClass B16;
    CARD32	length B32;
    CARD8	offendingMinorOpcode;
    CARD8	severity;
    CARD16	unused B16;
    CARD32	offendingSequenceNum B32;
    /* n	varying values */
    /* p	p = pad (n) */
} iceErrorMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	byteOrder;
    CARD8	unused;
    CARD32	length B32;
} iceByteOrderMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	versionCount;
    CARD8	authCount;
    CARD32	length B32;
    CARD8	mustAuthenticate;
    CARD8	unused[7];
    /* i	XPCS		vendor */
    /* j	XPCS		release */
    /* k	LIST of XPCS	authentication-protocol-names */
    /* m	LIST of VERSION version-list */
    /* p	p = pad (i+j+k+m) */
} iceConnectionSetupMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	authIndex;
    CARD8	unused1;
    CARD32	length B32;
    CARD16	authDataLength B16;
    CARD8	unused2[6];
    /* n	varying data */
    /* p	p = pad (n) */
} iceAuthRequiredMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused1[2];
    CARD32	length B32;
    CARD16	authDataLength B16;
    CARD8	unused2[6];
    /* n	varying data */
    /* p	p = pad (n) */
} iceAuthReplyMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused1[2];
    CARD32	length B32;
    CARD16	authDataLength B16;
    CARD8	unused2[6];
    /* n	varying data */
    /* p	p = pad (n) */
} iceAuthNextPhaseMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	versionIndex;
    CARD8	unused;
    CARD32	length B32;
    /* i	XPCS		vendor */
    /* j	XPCS		release */
    /* p	p = pad (i+j) */
} iceConnectionReplyMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	protocolOpcode;
    CARD8	mustAuthenticate;
    CARD32	length B32;
    CARD8	versionCount;
    CARD8	authCount;
    CARD8	unused[6];
    /* i	XPCS		protocol-name */
    /* j	XPCS		vendor */
    /* k	XPCS		release */
    /* m	LIST of XPCS	authentication-protocol-names */
    /* n	LIST of VERSION version-list */
    /* p        p = pad (i+j+k+m+n) */
} iceProtocolSetupMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	versionIndex;
    CARD8	protocolOpcode;
    CARD32	length B32;
    /* i	XPCS		vendor */
    /* j	XPCS		release */
    /* p	p = pad (i+j) */
} iceProtocolReplyMsg;

typedef iceMsg  icePingMsg;
typedef iceMsg  icePingReplyMsg;
typedef iceMsg  iceWantToCloseMsg;
typedef iceMsg  iceNoCloseMsg;


/*
 * SIZEOF values.  These better be multiples of 8.
 */

#define sz_iceMsg			8
#define sz_iceErrorMsg			16
#define sz_iceByteOrderMsg		8
#define sz_iceConnectionSetupMsg        16
#define sz_iceAuthRequiredMsg		16
#define sz_iceAuthReplyMsg		16
#define sz_iceAuthNextPhaseMsg		16
#define sz_iceConnectionReplyMsg	8
#define sz_iceProtocolSetupMsg		16
#define sz_iceProtocolReplyMsg		8
#define sz_icePingMsg			8
#define sz_icePingReplyMsg		8
#define sz_iceWantToCloseMsg		8
#define sz_iceNoCloseMsg		8

#endif /* ICEPROTO_H */
