/* $XConsortium: globals.h,v 1.3 93/09/08 20:43:22 mor Exp $ */
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

extern IcePOauthStatus _IcePOauth1proc ();
extern IcePAauthStatus _IcePAauth1proc ();
extern IcePOauthStatus _IcePOauth2proc ();
extern IcePAauthStatus _IcePAauth2proc ();

extern Bool _IceProcessCoreMessage ();

IceConn     	_IceConnectionObjs[256];
char	    	*_IceConnectionStrings[256];
int     	_IceConnectionCount = 0;

_IceProtocol 	_IceProtocols[255];
int         	_IceLastMajorOpcode = 0;

int		_IceAuthCount = 2;
IcePOauthRec	_IcePOauthRecs[] = {
		  		{"ICE-AUTH-TEST-1", _IcePOauth1proc},
		  		{"ICE-AUTH-TEST-2", _IcePOauth2proc}};
IcePAauthRec	_IcePAauthRecs[] = {
		  		{"ICE-AUTH-TEST-1", _IcePAauth1proc},
		  		{"ICE-AUTH-TEST-2", _IcePAauth2proc}};

int		_IceVersionCount = 1;
_IceVersion	_IceVersions[] = {
	  	    {IceProtoMajor, IceProtoMinor, _IceProcessCoreMessage}};

_IceWatchProc	*_IceWatchProcs = NULL;

IceErrorHandler   _IceErrorHandler   = _IceDefaultErrorHandler;
IceIOErrorHandler _IceIOErrorHandler = _IceDefaultIOErrorHandler;
