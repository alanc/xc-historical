/* $XConsortium: globals.h,v 1.4 93/09/27 11:47:19 mor Exp $ */
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

extern IcePoAuthStatus _SmcAuth1proc ();
extern IcePaAuthStatus _SmsAuth1proc ();
extern IcePoAuthStatus _SmcAuth2proc ();
extern IcePaAuthStatus _SmsAuth2proc ();

extern Bool _SmcProcessMessage ();
extern void _SmsProcessMessage ();

SmcConn _SmcConnectionObjs[256];
int     _SmcConnectionCount = 0;
int 	_SmcOpcode = 0;

SmsConn _SmsConnectionObjs[256];
int     _SmsConnectionCount = 0;
int 	_SmsOpcode = 0;

int		_SmVersionCount = 1;

IcePoVersionRec	_SmcVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmcProcessMessage}};

IcePaVersionRec _SmsVersions[] = {
	  	    {SmProtoMajor, SmProtoMinor, _SmsProcessMessage}};

int		_SmAuthCount = 2;

IcePoAuthRec 	_SmcAuthRecs[] = {
		    {"SM-AUTH-TEST-1", _SmcAuth1proc},
		    {"SM-AUTH-TEST-2", _SmcAuth2proc}};

IcePaAuthRec 	_SmsAuthRecs[] = {
		    {"SM-AUTH-TEST-1", _SmsAuth1proc},
		    {"SM-AUTH-TEST-2", _SmsAuth2proc}};

SmsNewClientProc _SmsNewClientProc;
SmPointer	 _SmsNewClientData;

SmcErrorHandler _SmcErrorHandler = _SmcDefaultErrorHandler;
SmsErrorHandler _SmsErrorHandler = _SmsDefaultErrorHandler;
