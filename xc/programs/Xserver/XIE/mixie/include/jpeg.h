/* $XConsortium$ */
/***	jpeg.h - common include file for jpeg stuff

	Ben Fahy, Oct 1993
***/


#ifndef XIE
#define XIE
#endif
#include <jinclude.h>

JSAMPIMAGE alloc_sampimage();

#define JPEG_BUFFER_BufferError			(-1)
#define JPEG_BUFFER_BufferFilled		0
#define	JPEG_BUFFER_NeedAnotherStrip		1
#define	JPEG_BUFFER_LastBuffer			2

#define	JPEG_FLUSH_Error			(-1)
#define JPEG_FLUSH_FlushedAll			1
#define JPEG_FLUSH_FlushedPart			2

#include "jpegd.h"
#include "jpege.h"
