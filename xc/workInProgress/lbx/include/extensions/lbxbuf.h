/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 * 
 * $NCDId: @(#)buffer.h,v 1.2 1994/01/17 20:59:40 dct Exp $
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#define INBUFFER_SIZE		(1 << 13)	/* 8K */
#define OUTBUFFER_SIZE		(1 << 12)	/* 4K */

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

typedef struct _lzwbuffer  *LzwBufferPtr;

extern int  InitLzwBuffer();
extern void FreeLzwBuffer();
extern char *ReserveOutBuf();

#endif	/* _BUFFER_H_ */
