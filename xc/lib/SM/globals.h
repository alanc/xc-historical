/* $XConsortium: globals.h,v 1.9 94/02/07 19:21:10 mor Exp $ */
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

extern void _SmcDefaultErrorHandler ();
extern void _SmsDefaultErrorHandler ();

extern IcePoAuthStatus _IcePoMagicCookie1Proc ();
extern IcePaAuthStatus _IcePaMagicCookie1Proc ();

extern Bool _SmcProcessMessage ();
extern void _SmsProcessMessage ();

int 	_SmcOpcode = 0;
int 	_SmsOpcode = 0;

int	_SmVersionCount = 1;

IcePoVersionRec	_SmcVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmcProcessMessage}};

IcePaVersionRec _SmsVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmsProcessMessage}};

int		_SmAuthCount = 1;
char		*_SmAuthNames[] = {"MIT-MAGIC-COOKIE-1"};
IcePoAuthProc 	_SmcAuthProcs[] = {_IcePoMagicCookie1Proc};
IcePaAuthProc 	_SmsAuthProcs[] = {_IcePaMagicCookie1Proc};

SmsNewClientProc _SmsNewClientProc;
SmPointer	 _SmsNewClientData;

SmcErrorHandler _SmcErrorHandler = _SmcDefaultErrorHandler;
SmsErrorHandler _SmsErrorHandler = _SmsDefaultErrorHandler;
