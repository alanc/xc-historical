/* $XConsortium: lbxext.h,v 1.1 94/12/01 20:39:01 mor Exp $ */
/*
 * Copyright 1994 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Network Computing Devices, Inc. not be
 * used in advertising or publicity pertaining to distribution of this
 * software without specific, written prior permission.
 *
 * THIS SOFTWARE IS PROVIDED `AS-IS'.  NETWORK COMPUTING DEVICES, INC.,
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NONINFRINGEMENT.  IN NO EVENT SHALL NETWORK
 * COMPUTING DEVICES, INC., BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE, DATA,
 * OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND REGARDLESS OF
 * WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $NCDId: @(#)lbxext.h,v 1.3 1994/09/14 21:57:05 lemke Exp $
 */

#ifndef _LBXEXT_H_
#define _LBXEXT_H_

/* number of extensions the proxy understands */
#define	MAX_SUPPORTED_EXTENSIONS	10

typedef int (*ExtensionRequests) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*ExtensionReplies) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*ExtensionEvents) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*ExtensionErrors) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*SExtensionRequests) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*SExtensionReplies) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*SExtensionEvents) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
typedef int (*SExtensionErrors) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

typedef struct _vectors {
    char       *name;
    ExtensionRequests req_vector;
    ExtensionReplies rep_vector;
    ExtensionEvents ev_vector;
    ExtensionErrors err_vector;
    SExtensionRequests sreq_vector;
}           ExtensionVectors;

extern int  (*ProcVector[256]) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);
extern int  (*SwappedProcVector[256]) (
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void RegisterExtension(
#if NeedFunctionPrototypes
    ExtensionVectors * /*new*/
#endif
);

typedef struct _LbxQueryExtensionReply *xLbxQueryExtensionReplyPtr;

extern Bool AddExtension(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*name*/,
    xLbxQueryExtensionReplyPtr /*reply*/,
    CARD8 * /*rep_mask*/,
    CARD8 * /*ev_mask*/
#endif
);

extern void DeleteClientExtensions(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern void RegisterAllExtensions(
#if NeedFunctionPrototypes
    void
#endif
);

extern void InitExtensions(
#if NeedFunctionPrototypes
    void
#endif
);

extern void ShutdownExtensions(
#if NeedFunctionPrototypes
    void
#endif
);

extern void HandleExtensionError(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    xError * /*err*/
#endif
);

extern void HandleExtensionEvent(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    xEvent * /*ev*/
#endif
);

typedef struct _replystuff *_ReplyStuffPtr;

extern Bool HandleExtensionReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    xReply * /*reply*/,
    _ReplyStuffPtr /*nr*/
#endif
);

extern Bool CheckExtensionForEvents(
#if NeedFunctionPrototypes
    xReq * /*req*/
#endif
);

extern Bool CheckExtensionForReplies(
#if NeedFunctionPrototypes
    xReq * /*req*/
#endif
);

extern Bool HandleLbxQueryExtensionReply(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    char * /*data*/
#endif
);

extern int ProcLBXQueryExtension(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

#endif				/* _LBXEXT_H_ */
