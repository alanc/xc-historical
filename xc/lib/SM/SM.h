/* $XConsortium: SM.h,v 1.4 93/12/15 17:31:33 mor Exp $ */
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

#ifndef SM_H
#define SM_H

/*
 * Protocol Version
 */

#define SmProtoMajor	1
#define SmProtoMinor	0


/*
 * Interact Style
 */

#define SmInteractStyleNone	0
#define SmInteractStyleErrors	1
#define SmInteractStyleAny	2


/*
 * Dialog Type
 */

#define SmDialogError		0
#define SmDialogNormal		1


/*
 * Save Type
 */

#define SmSaveGlobal	0
#define SmSaveLocal	1
#define SmSaveBoth	2


/*
 * Restart Style Hints
 */

#define SmRestartIfRunning	0
#define SmRestartAnyway		1
#define SmRestartImmediately	2


/*
 * Property names
 */

#define SmCloneCommand		"CloneCommand"
#define SmCurrentDirectory	"CurrentDirectory"
#define SmDiscardCommand	"DiscardCommand"
#define SmEnvironment		"Environment"
#define SmOperatingSystemName	"OperatingSystemName"
#define SmProcessID		"ProcessID"
#define SmProgram		"Program"
#define SmRestartCommand	"RestartCommand"
#define SmResignCommand		"ResignCommand"
#define SmRestartStyleHint	"RestartStyleHint"
#define SmShutdownCommand	"ShutdownCommand"
#define SmUserID		"UserID"


/*
 * Property types
 */

#define SmCARD8			"CARD8"
#define SmARRAY8		"ARRAY8"
#define SmLISTofARRAY8		"LISTofARRAY8"


/*
 * SM minor opcodes
 */

#define SM_Error		0
#define SM_RegisterClient 	1
#define SM_RegisterClientReply 	2
#define SM_SaveYourself 	3
#define SM_SaveYourselfRequest 	4
#define SM_InteractRequest 	5
#define SM_Interact 		6
#define SM_InteractDone 	7
#define SM_SaveYourselfDone 	8
#define SM_Die 			9
#define SM_ShutdownCancelled	10
#define SM_CloseConnection 	11
#define SM_SetProperties 	12
#define SM_DeleteProperties 	13
#define SM_GetProperties 	14
#define SM_PropertiesReply 	15

#endif /* SM_H */
