/* $XConsortium: SM.h,v 1.2 93/09/22 11:25:08 mor Exp $ */
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
 * Property names
 */

#define SmProgram		"Program"
#define SmUserID		"UserID"
#define SmRestartCommand	"RestartCommand"
#define SmCurrentDirectory	"CurrentDirectory"
#define SmEnvironment		"Environment"
#define SmKillCommand		"KillCommand"
#define SmRestartStyleHint	"RestartStyleHint"
#define SmDiscardCommand	"DiscardCommand"
#define SmOperatingSystemName	"OperatingSystemName"
#define SmCloneCommand		"CloneCommand"
#define SmProcessID		"ProcessID"


/*
 * Property types
 */

#define SmARRAY8		"ARRAY8"
#define SmCard8			"Card8"


/*
 * SM minor opcodes
 */

#define SM_Error		0
#define SM_RegisterClient 	1
#define SM_RegisterClientReply 	2
#define SM_SaveYourself 	3
#define SM_InteractRequest 	4
#define SM_Interact 		5
#define SM_InteractDone 	6
#define SM_SaveYourselfDone 	7
#define SM_Die 			8
#define SM_ShutdownCancelled	9
#define SM_CloseConnection 	10
#define SM_SetProperties 	11
#define SM_GetProperties 	12
#define SM_PropertiesReply 	13

#endif /* SM_H */
