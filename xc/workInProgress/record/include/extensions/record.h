/* $XConsortium$ */
/***************************************************************************
 * Copyright 1994 Network Computing Devices;
 * Portions Copyright 1988 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 **************************************************************************/
#ifndef _XRECORD_H_
#define _XRECORD_H_

#define XRecordExtName 			"RECORD"
#define XRecordMajorVersion   		1L
#define XRecordMinorVersion   		0L
#define XRecordLowestMajorVersion   	1L
#define XRecordLowestMinorVersion   	0L

#define X_RecordQueryVersion    0L     /* Must be first request from client */
#define X_RecordCreateConfig    1L     /* Create client XRecord environment */
#define X_RecordFreeConfig      2L     /* Free client XRecord environment */
#define X_RecordChangeConfig    3L     /* Modify client XRecord environment */
#define X_RecordGetConfig       4L     /* Get client current environment */
#define X_RecordEnableConfig    5L     /* Enable client XRecord trapping */

#define XRecordBadConfiguration  0L     /* Not a valid configuration */
#define XRecordNumErrors         	(XRecordBadConfiguration + 1) 

#define XRecordActive         0L    	/* If sending/receiving between client/ext */

#define XRecordNumEvents      1L
#ifndef MAX
# define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

extern int XRecordRequestVector(
#if NeedFunctionPrototypes
	ClientPtr client
#endif
);

extern int  XRecordEventVector(
#if NeedFunctionPrototypes
	ClientPtr client, 
        xEvent *x_event
#endif
);
extern int XRecordErrorVector(
#if NeedFunctionPrototypes
	ClientPtr client, 
        xError *x_error
#endif
); 

extern int ProcRecordDispatch(
#if NeedFunctionPrototypes
	ClientPtr client
#endif
);

extern int sProcRecordDispatch(
#if NeedFunctionPrototypes
	ClientPtr client
#endif
);

#ifndef _XRECORD_SERVER_

_XFUNCPROTOBEGIN

/* 
 *  Prototypes
 */

extern Status XRecordQueryVersion(
#if NeedFunctionPrototypes
	Display* 			/* dpy */,
        int 				/* cmajor */, 
        int 				/* cminor */, 
        XRecordQueryVersionReply* 	/* ret */   
#endif
);

extern Status XRecordFreeCG(
#if NeedFunctionPrototypes
    	Display* 		/* dpy */, 
        XRecordConfig 		/* config */
#endif
);

extern Status XRecordChangeCG(
#if NeedFunctionPrototypes
    	Display* 		/* dpy */, 
        XRecordConfig 		/* config */, 
        CARD32 		        /* id_base */, 
    	XRecordFlags* 		/* record_flags */,  
        BOOL			/* add/delete id_base */
#endif
);

extern Status XRecordGetCG(
#if NeedFunctionPrototypes
    	Display*		/* dpy */,
        XRecordConfig 		/* config */, 
    	XRecordState* 		/* ret */ 
#endif
);

extern Status XRecordEnableCG(
#if NeedFunctionPrototypes
    	Display*		/* dpy */,
        XRecordConfig 		/* config */, 
        BOOL 		        /* enable */
#endif
); 

/*
 *  Errors 
 */

typedef struct {
    int type;			
    Display *display;		/* Display the event was read from */
    XRecordConfig config;	/* resource id */
    unsigned long serial;	/* serial number of failed request */
    unsigned char error_code;	/* error base + XRecordBadConfiguration */
    unsigned char request_code;	/* Major op-code of failed request */
    unsigned char minor_code;	/* Minor op-code of failed request */
} XRecordConfigError;

_XFUNCPROTOEND

#endif /* _XRECORD_SERVER_ */

#endif /* _XRECORD_H_ */
