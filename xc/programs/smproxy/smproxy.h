/* $XConsortium: smproxy.h,v 1.3 94/07/27 16:05:47 mor Exp mor $ */
/******************************************************************************

Copyright (c) 1994  X Consortium

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

Author:  Ralph Mor, X Consortium
******************************************************************************/

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/SM/SMlib.h>
#include <stdio.h>

#include <X11/Xosdefs.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern char *malloc(), *calloc(), *realloc(), *getenv();
extern void free();
#endif

typedef struct WinInfo {
    Window window;
    SmcConn smc_conn;
    XtInputId input_id;
    char *client_id;
    char **wm_command;
    int wm_command_count;
    XClassHint class;
    char *wm_name;
    XTextProperty wm_client_machine;
    struct WinInfo *next;

    unsigned int waiting_for_required_props : 1;
    unsigned int got_wm_state : 1;
    unsigned int has_save_yourself : 1;
    unsigned int waiting_for_update : 1;
    unsigned int got_first_save_yourself : 1;

} WinInfo;


extern int WriteProxyFileEntry ();
extern int ReadProxyFileEntry ();
extern void ReadProxyFile ();
extern char *LookupClientID ();
