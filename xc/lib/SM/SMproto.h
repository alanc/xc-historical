/* $XConsortium: SMproto.h,v 1.1 93/09/03 13:25:07 mor Exp $ */
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

#ifndef SMPROTO_H
#define SMPROTO_H

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
    /* n	ARRAY8		previousId */
} smRegisterClientMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
    /* n	ARRAY8		clientId */
} smRegisterClientReplyMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused1[2];
    CARD32	length B32;
    CARD8	saveType;
    CARD8	shutdown;
    CARD8	interactStyle;
    CARD8	fast;
    CARD8	unused2[4];
} smSaveYourselfMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	dialogType;
    CARD8	unused;
    CARD32	length B32;
} smInteractRequestMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smInteractMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	cancelShutdown;
    CARD8	unused;
    CARD32	length B32;
} smInteractDoneMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8       success;
    CARD8	unused;
    CARD32	length B32;
} smSaveYourselfDoneMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smDieMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smShutdownCancelledMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
    /* a	ARRAY8		locale */
    /* b	LISTofARRAY8	reasons */
} smCloseConnectionMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused1[2];
    CARD32	length B32;
    CARD32	sequenceRef B32;
    CARD8	unused2[4];
    /* a	LISTofPROPERTY	properties */
} smSetPropertiesMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smGetPropertiesMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
    /* a	LISTofPROPERTY	properties */
} smPropertiesReplyMsg;


/*
 * SIZEOF values.  These better be multiples of 8.
 */

#define sz_smRegisterClientMsg 		8
#define sz_smRegisterClientReplyMsg 	8
#define sz_smSaveYourselfMsg 		16
#define sz_smInteractRequestMsg 	8
#define sz_smInteractMsg 		8
#define sz_smInteractDoneMsg 		8
#define sz_smSaveYourselfDoneMsg 	8
#define sz_smDieMsg 			8
#define sz_smShutdownCancelledMsg 	8
#define sz_smCloseConnectionMsg 	8
#define sz_smSetPropertiesMsg 		16
#define sz_smGetPropertiesMsg 		8
#define sz_smPropertiesReplyMsg 	8

#endif /* SMPROTO_H */
