/* $XConsortium$ */
/******************************************************************************

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
******************************************************************************/

#include "xsm.h"


static void
ChooseSessionOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{

}



static void
ChooseSessionCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    exit (0);
}



void
create_choose_session_popup ()

{
    /*
     * Pop up for choosing session at startup
     */

    chooseSessionPopup = XtVaCreatePopupShell (
	"chooseSessionPopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    chooseSessionForm = XtVaCreateManagedWidget (
	"chooseSessionForm", formWidgetClass, chooseSessionPopup,
	NULL);


    chooseSessionOkButton = XtVaCreateManagedWidget (
	"chooseSessionOkButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        NULL);

    XtAddCallback (chooseSessionOkButton, XtNcallback,
	ChooseSessionOkXtProc, 0);


    chooseSessionCancelButton = XtVaCreateManagedWidget (
	"chooseSessionCancelButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionOkButton,
        XtNfromVert, NULL,
        NULL);

    XtAddCallback (chooseSessionCancelButton, XtNcallback,
	ChooseSessionCancelXtProc, 0);


    chooseSessionListWidget = XtVaCreateManagedWidget (
	"chooseSessionListWidget", listWidgetClass, chooseSessionForm,
	XtNresizable, True,
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, chooseSessionOkButton,
	NULL);
}
