/* $XConsortium$ */
/* Module jmemsys.c */

/****************************************************************************
				NOTICE
                              
This software is being provided by AGE Logic, Inc. and MIT under the
following license.  By obtaining, using and/or copying this software,
you agree that you have read, understood, and will comply with these
terms and conditions:

     Permission to use, copy, modify, distribute and sell this
     software and its documentation for any purpose and without
     fee or royalty and to grant others any or all rights granted
     herein is hereby granted, provided that you agree to comply
     with the following copyright notice and statements, including
     the disclaimer, and that the same appears on all copies and
     derivative works of the software and documentation you make.
     
     "Copyright 1993 by AGE Logic, Inc. and the Massachusetts
     Institute of Technology"
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC AND MIT
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC OR MIT OR LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The names of AGE Logic, Inc. and MIT may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic and MIT.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
*****************************************************************************

	Gary Rogers, AGE Logic, Inc., October 1993

****************************************************************************/

/*
 * jmemnobs.c  (jmemsys.c)
 *
 * Copyright (C) 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a really simple implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that no backing-store files are needed: all required space
 * can be obtained from malloc().
 * This is very portable in the sense that it'll compile on almost anything,
 * but you'd better have lots of main memory (or virtual memory) if you want
 * to process big images.
 * Note that the max_memory_to_use option is ignored by this implementation.
 */

#include "jinclude.h"
#include "jmemsys.h"

#ifdef _XIEC_MEMORY
#ifdef malloc
#undef malloc
#endif
#ifdef free
#undef free
#endif
#define malloc(size)    XieMalloc(size)
#define free(ptr)       XieFree(ptr)
#else
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare malloc(), free() */
#else
extern void * malloc PP((size_t size));
extern void free PP((void *ptr));
#endif
#endif  /* _XIEC_MEMORY */


#ifndef XIE_SUPPORTED
static external_methods_ptr methods; /* saved for access to error_exit */
#endif	/* XIE_SUPPORTED */

/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

GLOBAL void *
#ifdef XIE_SUPPORTED
#ifdef NeedFunctionPrototypes
jget_small (size_t sizeofobject)
#else
jget_small (sizeofobject)
	size_t sizeofobject;
#endif	/* NeedFunctionPrototypes */
#else
jget_small (size_t sizeofobject)
#endif	/* XIE_SUPPORTED */
{
  return (void *) malloc(sizeofobject);
}

GLOBAL void
#ifdef XIE_SUPPORTED
#ifdef NeedFunctionPrototypes
jfree_small (void * object)
#else
jfree_small (object)
	void * object;
#endif	/* NeedFunctionPrototypes */
#else
jfree_small (void * object)
#endif	/* XIE_SUPPORTED */
{
  free(object);
}

/*
 * We assume NEED_FAR_POINTERS is not defined and so the separate entry points
 * jget_large, jfree_large are not needed.
 */


/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

GLOBAL long
#ifdef XIE_SUPPORTED
#ifdef NeedFunctionPrototypes
jmem_available (long min_bytes_needed, long max_bytes_needed)
#else
jmem_available (min_bytes_needed, max_bytes_needed)
	long min_bytes_needed;
	long max_bytes_needed;
#endif	/* NeedFunctionPrototypes */
#else
jmem_available (long min_bytes_needed, long max_bytes_needed)
#endif	/* XIE_SUPPORTED */
{
  return max_bytes_needed;
}


/*
 * Backing store (temporary file) management.
 * This should never be called and we just error out.
 */

GLOBAL void
#ifdef XIE_SUPPORTED
#ifdef NeedFunctionPrototypes
jopen_backing_store (backing_store_ptr info, long total_bytes_needed)
#else
jopen_backing_store (info, total_bytes_needed)
	backing_store_ptr info;
	long total_bytes_needed;
#endif	/* NeedFunctionPrototypes */
{
}
#else
jopen_backing_store (backing_store_ptr info, long total_bytes_needed)
{
  ERREXIT(methods, "Backing store not supported");
}
#endif	/* XIE_SUPPORTED */


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Keep in mind that jmem_term may be called more than
 * once.
 */

#ifndef XIE_SUPPORTED
GLOBAL void
jmem_init (external_methods_ptr emethods)
{
  methods = emethods;		/* save struct addr for error exit access */
  emethods->max_memory_to_use = 0;
}
#endif	/* XIE_SUPPORTED */

GLOBAL void
#ifdef XIE_SUPPORTED
#ifdef NeedFunctionPrototypes
jmem_term (void)
#else
jmem_term ()
#endif	/* NeedFunctionPrototypes */
#else
jmem_term (void)
#endif	/* XIE_SUPPORTED */
{
  /* no work */
}
