/* $XConsortium: globals.h,v 1.2 93/09/08 20:21:13 mor Exp $ */
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

extern void _SmcDefaultErrorHandler ();
extern void _SmsDefaultErrorHandler ();

extern IceOCLauthStatus _SmOCLauth1proc ();
extern IceACLauthStatus _SmACLauth1proc ();
extern IceOCLauthStatus _SmOCLauth2proc ();
extern IceACLauthStatus _SmACLauth2proc ();

extern Bool _SmcProcessMessage ();
extern void _SmsProcessMessage ();

SmcConn _SmcConnectionObjs[256];
int     _SmcConnectionCount = 0;
int 	_SmcOpcode = 0;

SmsConn _SmsConnectionObjs[256];
int     _SmsConnectionCount = 0;
int 	_SmsOpcode = 0;

int		 _SmVersionCount = 1;

IceOCLversionRec _SmcVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmcProcessMessage}};

IceACLversionRec _SmsVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmsProcessMessage}};

int	      _SmAuthCount = 2;

IceOCLauthRec _SmcAuthRecs[] = {
		    {"SM-AUTH-TEST-1", _SmOCLauth1proc},
		    {"SM-AUTH-TEST-2", _SmOCLauth2proc}};

IceACLauthRec _SmsAuthRecs[] = {
		    {"SM-AUTH-TEST-1", _SmACLauth1proc},
		    {"SM-AUTH-TEST-2", _SmACLauth2proc}};

SmsNewClientProc _SmsNewClientProc;
SmPointer	 _SmsNewClientData;

SmcErrorHandler _SmcErrorHandler = _SmcDefaultErrorHandler;
SmsErrorHandler _SmsErrorHandler = _SmsDefaultErrorHandler;
