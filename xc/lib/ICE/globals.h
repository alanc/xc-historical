/* $XConsortium: globals.h,v 1.1 93/08/19 18:25:25 mor Exp $ */
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

extern IceOCLauthStatus _IceOCLauth1proc ();
extern IceACLauthStatus _IceACLauth1proc ();
extern IceOCLauthStatus _IceOCLauth2proc ();
extern IceACLauthStatus _IceACLauth2proc ();

extern Bool _IceProcessCoreMessage ();

_IceProtocol 	_IceProtocols[255];
int         	_IceLastMajorOpcode = 0;

int		_IceAuthCount = 2;
IceOCLauthRec	_IceOCLauthRecs[] = {
		  		{"ICE-AUTH-TEST-1", _IceOCLauth1proc},
		  		{"ICE-AUTH-TEST-2", _IceOCLauth2proc}};
IceACLauthRec	_IceACLauthRecs[] = {
		  		{"ICE-AUTH-TEST-1", _IceACLauth1proc},
		  		{"ICE-AUTH-TEST-2", _IceACLauth2proc}};

int		_IceVersionCount = 1;
_IceVersion	_IceVersions[] = {
	  	    {IceProtoMajor, IceProtoMinor, _IceProcessCoreMessage}};

IceErrorHandler   _IceErrorHandler   = _IceDefaultErrorHandler;
IceIOErrorHandler _IceIOErrorHandler = _IceDefaultIOErrorHandler;
