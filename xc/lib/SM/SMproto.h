/* $XConsortium: SMproto.h,v 1.7 94/03/30 19:54:03 mor Exp $ */
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

Author: Ralph Mor, X Consortium
******************************************************************************/

#ifndef _SMPROTO_H_
#define _SMPROTO_H_

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
    CARD8	unused1[2];
    CARD32	length B32;
    CARD8	saveType;
    CARD8	shutdown;
    CARD8	interactStyle;
    CARD8	fast;
    CARD8	global;
    CARD8	unused2[3];
} smSaveYourselfRequestMsg;

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
    /* b	LISTofARRAY8	reasons */
} smCloseConnectionMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
    /* a	LISTofPROPERTY	properties */
} smSetPropertiesMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
    /* a	LISTofARRAY8	property names */
} smDeletePropertiesMsg;

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

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smSaveYourselfPhase2RequestMsg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smSaveYourselfPhase2Msg;

typedef struct {
    CARD8	majorOpcode;
    CARD8	minorOpcode;
    CARD8	unused[2];
    CARD32	length B32;
} smSaveCompleteMsg;


/*
 * SIZEOF values.  These better be multiples of 8.
 */

#define sz_smRegisterClientMsg 			8
#define sz_smRegisterClientReplyMsg 		8
#define sz_smSaveYourselfMsg 			16
#define sz_smSaveYourselfRequestMsg		16
#define sz_smInteractRequestMsg 		8
#define sz_smInteractMsg 			8
#define sz_smInteractDoneMsg 			8
#define sz_smSaveYourselfDoneMsg 		8
#define sz_smDieMsg 				8
#define sz_smShutdownCancelledMsg 		8
#define sz_smCloseConnectionMsg 		8
#define sz_smSetPropertiesMsg 			8
#define sz_smDeletePropertiesMsg 		8
#define sz_smGetPropertiesMsg 			8
#define sz_smPropertiesReplyMsg 		8
#define sz_smSaveYourselfPhase2RequestMsg	8
#define sz_smSaveYourselfPhase2Msg 		8
#define sz_smSaveCompleteMsg 			8

#endif /* _SMPROTO_H_ */
