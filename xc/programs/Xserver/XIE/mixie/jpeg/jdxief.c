/* $XConsortium$ */
/* Module jdxief.c */

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

#include "jinclude.h"

#ifdef NeedFunctionPrototypes
GLOBAL int jdXIE_init(decompress_info_ptr);
GLOBAL int jdXIE_get(decompress_info_ptr);
GLOBAL int jdXIE_term(decompress_info_ptr);
GLOBAL void jseldXIE(decompress_info_ptr);
#endif	/* NeedFunctionPrototypes */

/**********************************************************************/

GLOBAL int
#ifdef NeedFunctionPrototypes
JD_INIT(decompress_info_ptr cinfo,
	decompress_methods_ptr dcmethods, external_methods_ptr emethods)
#else
JD_INIT(cinfo, dcmethods, emethods)
	decompress_info_ptr cinfo;
	decompress_methods_ptr dcmethods;
	external_methods_ptr emethods;
#endif	/* NeedFunctionPrototypes */
{
  /* Set up links to method structures. */
  cinfo->methods = dcmethods;
  cinfo->emethods = emethods;

  /* Set restart to NULL */
  cinfo->XIErestart = XIE_RNUL;

  return(jdXIE_init(cinfo));
}

/**********************************************************************/

GLOBAL int
#ifdef NeedFunctionPrototypes
JD_BEGINFRAME(decompress_info_ptr cinfo)
#else
JD_BEGINFRAME(cinfo)
	decompress_info_ptr cinfo;
#endif	/* NeedFunctionPrototypes */
{
  (*cinfo->methods->output_init) (cinfo);

  return(XIE_NRML);    
}

/**********************************************************************/

GLOBAL int
#ifdef NeedFunctionPrototypes
JD_PROCESS(decompress_info_ptr cinfo)
#else
JD_PROCESS(cinfo)
	decompress_info_ptr cinfo;
#endif	/* NeedFunctionPrototypes */
{
    return(jdXIE_get(cinfo));
}

/**********************************************************************/

GLOBAL int
#ifdef NeedFunctionPrototypes
JD_ENDFRAME(decompress_info_ptr cinfo)
#else
JD_ENDFRAME(cinfo)
	decompress_info_ptr cinfo;
#endif	/* NeedFunctionPrototypes */
{

  jdXIE_term(cinfo);

  return(XIE_NRML);    
}

/**********************************************************************/
