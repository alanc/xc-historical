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

#include <X11/Xmd.h>

#define globaldef 
#define globalref extern

typedef int  (*int_function)();
typedef void (*void_function)();

#define XRecordMaxEvent                 (128L-1L)
#define XRecordMaxError                 (256L-1L)
#define XRecordMaxCoreRequest           (128L-1L)
#define XRecordMaxExtRequest            (256L-1L)
#define XRecordMinExtRequest            (129L-1L)

#define RecordCoreRequest               (1L<<0L)
#define RecordCoreReply                 (1L<<1L)
#define RecordExtRequest                (1L<<2L)
#define RecordExtReply                  (1L<<3L)
#define RecordEvent                     (1L<<4L)
#define RecordError                     (1L<<5L)

#define FromClient                    0
#define FromServer                    1

typedef XID XRecordConfig;

typedef struct
{
    CARD8       first;
    CARD8 	last;
} XRecordRange;

typedef struct
{
    CARD16 	first B16;
    CARD16 	last B16;
} XRecordRange16;

typedef struct
{
    CARD8        	ext_major;
    CARD8        	pad0;
    CARD16       	pad1 B16;
    XRecordRange16 	ext_minor;
} XRecordExtRange16;

typedef struct
{
    XRecordRange       core_requests;
    XRecordRange       core_replies;
    XRecordRange       events;
    XRecordRange       errors;
    XRecordExtRange16  ext_requests;
    XRecordExtRange16  ext_replies;
    CARD32              pad0 B32;
    CARD32              pad1 B32;
} XRecordFlags; /* size 32 */
#define sz_XRecordFlags		32

typedef struct
{
    BOOL 	       enabled;
    CARD8              pad0;  
    CARD16             pad1 B16; 
    XRecordFlags       intercepted;
} XRecordState; /* size 36 */

typedef struct 
{
    CARD8  reqType;
    CARD8  minor_opcode;
    CARD16 length B16;
    CARD32 pad0 B32;     
} xRecordReq;
#define sz_xRecordReq 			8

typedef struct {
    CARD8       reqType;        
    CARD8       minor_opcode;  
    CARD16      length B16;
    CARD16      majorVersion B16;
    CARD16      minorVersion B16;
} xRecordQueryVersionReq;
#define sz_xRecordQueryVersionReq 	8
  
typedef struct 
{
    CARD8     	reqType;
    CARD8     	minor_opcode;
    CARD16    	length B16;
    XRecordConfig cid B32;
    XRecordFlags  record_flags;
} xRecordCreateConfigReq;
#define sz_xRecordCreateConfigReq 	40 

typedef struct 
{
    CARD8     	reqType;
    CARD8     	minor_opcode;
    CARD16    	length B16;
    XRecordConfig cid B32;
} xRecordFreeConfigReq;
#define sz_xRecordFreeConfigReq 	8 

typedef struct 
{
    CARD8     	reqType;
    CARD8     	minor_opcode;
    CARD16    	length B16;
    XRecordConfig cid B32;
    CARD32      id_base B32 ;
    XRecordFlags  record_flags;
    BOOL	add; 
} xRecordChangeConfigReq;
#define sz_xRecordChangeConfigReq 	48

typedef struct 
{
    CARD8     	reqType;
    CARD8     	minor_opcode;
    CARD16    	length B16;
    XRecordConfig cid B32;
} xRecordGetConfigReq;
#define sz_xRecordGetConfigReq 		8   

typedef struct 
{
    CARD8     	reqType;
    CARD8     	minor_opcode;
    CARD16    	length B16;
    XRecordConfig cid B32;
    BOOL        enable;
    CARD8       pad0;
    CARD16      pad6 B16; 
} xRecordEnableConfigReq;
#define sz_xRecordEnableConfigReq 	12 

typedef struct
{
    CARD8  type;
    CARD8  detail;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
} XRecordReplyHdr;

typedef struct
{
    CARD16  majorVersion B16;
    CARD16  minorVersion B16; 	
} XRecordQueryVersionReply;

typedef struct 
{
    XRecordReplyHdr 	 hdr; 
    CARD16  majorVersion B16;
    CARD16  minorVersion B16; 	
    CARD32  pad1	 B32;
    CARD32  pad2	 B32;
    CARD32  pad3	 B32;
    CARD32  pad4	 B32;
    CARD32  pad5	 B32; 
 } xRecordQueryVersionReply;
#define sz_xRecordQueryVersionReply  	32 

typedef struct 
{
    XRecordReplyHdr 	 hdr;
    XRecordState         record_state; 
} xRecordGetConfigReply;
#define sz_xRecordGetConfigReply  	44

typedef struct
{
   union
   {
   	xEvent          event;
   	xResourceReq    req;
   	xGenericReply   reply;
   	xError          error;
   } u;
} XRecordDatum;

typedef struct 
{
    XRecordReplyHdr     hdr;
    CARD32        	nReplies B32;
    CARD32        	id_base B32;
    CARD32        	client_seq B32; 
    CARD8         	direction;
    BOOL          	client_swapped;
    XRecordDatum    data;
} xRecordEnableConfigReply;
#define sz_xRecordEnableConfigReply 	56

typedef struct 
{
    CARD32        replies_following_hint B32;
    CARD32        id_base B32;
    CARD32        client_seq B32;
    CARD8         direction;
    BOOL          client_swapped;
    XRecordDatum    data;
} XRecordEnableCGReply;  

