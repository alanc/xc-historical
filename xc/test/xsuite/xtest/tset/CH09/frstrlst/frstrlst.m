/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
>>TITLE XFreeStringList CH09
void
XFreeStringList(list)
char	**list = (char **) NULL;
>>ASSERTION Good A
A call to xname frees the memory allocated by a call to
.S XTextPropertyToStringList
or
.S XGetCommand . 
>>STRATEGY
Create a window using XCreateWindow.
Allocate a text property structure using XStringListToTextProperty.
Set the WM_COMMAND property using XSetCommand.
Obtain the value of the WM_COMMAND property using XGetCommand.
Obtain the strings from the XTextPropertyStructure using XTextPropertyToStringlist.
Release the memory allocated in the call to XGetCommand.
Release the memory allocated in the call to XTextPropertyToStringList.
>>CODE
char		*str1 = "TestString1";
char		*str2 = "TestString2";
char		*str3 = "TestString3";
int		argc = 3;
char		*argv[3];
int		rargc;
int		rargc1;
char		**rargv = (char **) NULL;
char		**rargv1 = (char **) NULL;
Window		w;
XVisualInfo	*vp;
XTextProperty	tp;

	argv[0] = str1;
	argv[1] = str2;
	argv[2] = str3;


	if( XStringListToTextProperty(argv, 3, &tp) == 0) {
		delete("XStringListToTextProperty() returned zero.");
		return;
	} else
		CHECK;

	if( XTextPropertyToStringList(&tp, &rargv, &rargc) == 0) {
		delete("XTextPropertyToStringList() returned zero.");
		return;
	} else
		CHECK;

	XFree((char*)tp.value);
	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(Dsp, vp);

	XSetCommand(Dsp, w, argv, argc);

	if(XGetCommand(Dsp, w, &rargv1, &rargc) == 0 ) {
		delete("XGetCommand() returned zero.");
		return;
	} else
		CHECK;

	XFreeStringList(rargv1);
	XFreeStringList(rargv);

	CHECKPASS(3);
