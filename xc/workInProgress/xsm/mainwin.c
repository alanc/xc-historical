/* $XConsortium: mainwin.c,v 1.1 94/07/07 16:47:39 mor Exp $ */
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
#include "info.h"
#include "save.h"
#include "name.h"


void
create_main_window ()

{
    /*
     * Main window
     */

    mainWindow = XtVaCreateManagedWidget (
	"mainWindow", formWidgetClass, topLevel,
	NULL);

    clientInfoButton = XtVaCreateManagedWidget (
	"clientInfoButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
	XtNresizable, True,
	XtNjustify, XtJustifyLeft,
	NULL);

    XtAddCallback (clientInfoButton, XtNcallback, ClientInfoXtProc, 0);

    nameSessionButton = XtVaCreateManagedWidget (
	"nameSessionButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, clientInfoButton,
        XtNfromVert, NULL,
	XtNresizable, True,
	XtNjustify, XtJustifyLeft,
	NULL);

    XtAddCallback (nameSessionButton, XtNcallback, NameSessionXtProc, 0);

    checkPointButton = XtVaCreateManagedWidget (
	"checkPointButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, NULL,
        XtNfromVert, clientInfoButton,
	XtNresizable, True,
	XtNjustify, XtJustifyLeft,
	NULL);

    XtAddCallback (checkPointButton, XtNcallback, CheckPointXtProc, 0);

    shutdownButton = XtVaCreateManagedWidget (
	"shutdownButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, checkPointButton,
        XtNfromVert, clientInfoButton,
	XtNresizable, True,
	XtNjustify, XtJustifyLeft,
	NULL);

    XtAddCallback (shutdownButton, XtNcallback, ShutdownXtProc, 0);
}
