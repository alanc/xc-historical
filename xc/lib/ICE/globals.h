/* $XConsortium: globals.h,v 1.4 93/09/27 11:45:29 mor Exp $ */
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

extern void _IceDefaultErrorHandler ();
extern void _IceDefaultIOErrorHandler ();

extern IcePoAuthStatus _IcePoAuth1proc ();
extern IcePaAuthStatus _IcePaAuth1proc ();
extern IcePoAuthStatus _IcePoAuth2proc ();
extern IcePaAuthStatus _IcePaAuth2proc ();

extern Bool _IceProcessCoreMessage ();

IceConn     	_IceConnectionObjs[256];
char	    	*_IceConnectionStrings[256];
int     	_IceConnectionCount = 0;

_IceProtocol 	_IceProtocols[255];
int         	_IceLastMajorOpcode = 0;

int		_IceAuthCount = 2;
IcePoAuthRec	_IcePoAuthRecs[] = {
		  		{"ICE-AUTH-TEST-1", _IcePoAuth1proc},
		  		{"ICE-AUTH-TEST-2", _IcePoAuth2proc}};
IcePaAuthRec	_IcePaAuthRecs[] = {
		  		{"ICE-AUTH-TEST-1", _IcePaAuth1proc},
		  		{"ICE-AUTH-TEST-2", _IcePaAuth2proc}};

int		_IceVersionCount = 1;
_IceVersion	_IceVersions[] = {
	  	    {IceProtoMajor, IceProtoMinor, _IceProcessCoreMessage}};

_IceWatchProc	*_IceWatchProcs = NULL;

IceErrorHandler   _IceErrorHandler   = _IceDefaultErrorHandler;
IceIOErrorHandler _IceIOErrorHandler = _IceDefaultIOErrorHandler;
