/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 * 
 * $NCDId: @(#)bufferstr.h,v 1.3 1994/01/21 19:13:22 dct Exp $
 */
#ifndef _BUFFERSTR_H_
#define _BUFFERSTR_H_

#include	"lbxbuf.h"

/*
 * LZW Input/Output buffer
 */
typedef struct _lzwbuffer {
	char 	*bufbase;
	char	*bufend;
	char	*bufptr;
	int  	bufcnt;
} LzwBuffer;

#define BYTES_AVAIL(inbuf, len)	\
	(((inbuf)->bufcnt >= (len)) ? (inbuf)->bufptr : NULL)

#endif	/* _BUFFERSTR_H_ */
