/* $XConsortium$ */
/* Module jcxief.c */

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

	jcxief.c: Xie JPEG Compression Wrapper Routines 

	Gary Rogers, AGE Logic, Inc., October 1993

****************************************************************************/

#include "jinclude.h"

#ifdef THINK_C
#include <console.h>		/* command-line reader for Macintosh */
#endif

#ifdef NeedFunctionPrototypes
GLOBAL void
jselrXIE (compress_info_ptr);
GLOBAL void	
process_rgb_ycc_row (compress_info_ptr, JSAMPARRAY);
GLOBAL int
jcXIE_get (compress_info_ptr, int, JSAMPARRAY);
GLOBAL void
jcXIE_get_buffersize (compress_info_ptr);
GLOBAL int
j_add_quant_table (compress_info_ptr, int, const QUANT_VAL *, int, boolean);
GLOBAL int
add_huff_table (compress_info_ptr, HUFF_TBL **, const UINT8 *, const UINT8 *);
#endif	/* NeedFunctionPrototypes */

/******************************************************************************/

LOCAL int
#ifdef NeedFunctionPrototypes
load_quant_tables (compress_info_ptr cinfo, 
			UINT8 * q_table, int nq_table, int scale_factor)
#else
load_quant_tables (cinfo, q_table, nq_table, scale_factor)
	compress_info_ptr cinfo;
	UINT8 * q_table;
	int nq_table;
	int scale_factor;
#endif	/* NeedFunctionPrototypes */
/* Read a set of quantization tables pointed at by q_table.
 * There may be one to NUM_QUANT_TBLS tables, each of 64 values.
 * The tables are implicitly numbered 0, 1, etc.
 */
{
  /* ZIG[i] is the zigzag-order position of the i'th element of a DCT block */
  /* read in natural order (left to right, top to bottom). */
  static const short ZIG[DCTSIZE2] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
    };
  int numtables;    
  int tblno, i;
  QUANT_TBL table;

  numtables = nq_table / 64;
  if (numtables < 1 || numtables > NUM_QUANT_TBLS)
  	return(XIE_ERR);
    
  for (tblno = 0; tblno < numtables; tblno++)
  	{
    for (i = 0; i < DCTSIZE2; i++)
      table[ZIG[i]] = *q_table++;
    j_add_quant_table(cinfo, tblno, table, scale_factor, FALSE);
  }
  return(0);
}

LOCAL int
#ifdef NeedFunctionPrototypes
load_ac_tables (compress_info_ptr cinfo, 
			UINT8 * ac_table, int nac_table)
#else
load_ac_tables (cinfo, ac_table, nac_table)
	compress_info_ptr cinfo;
	UINT8 * ac_table;
	int nac_table;
#endif	/* NeedFunctionPrototypes */
/* Read a set of AC Huffman tables pointed at by ac_table.
 * There may be one or two tables.
 */
{
  int i;
  int count;
  UINT8 bits[17];
  UINT8 val[256];

  bits[0] = 0;
  /* luminance AC coefficients */  
  count = 0;
  nac_table -= 16;
  if (nac_table < 0)	/* bad length */
  	return(XIE_ERR);
  for(i = 1; i < 17; i++) {
    bits[i] = *ac_table++;
    count += bits[i];
  }
  nac_table -= count;
  if (nac_table < 0 || 256 < count)	/* bad length */
  	return(XIE_ERR);
  for(i = 0; i < count; i++) {
    val[i] = *ac_table++;
  }
  if(XIE_ERR == add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[0], bits, val))
    return(XIE_ERR);
    
  /* chrominance AC coefficients */  
  count = 0;
  nac_table -= 16;
  if (nac_table < 0)	/* bad length */
  	return(XIE_ERR);
  for(i = 1; i < 17; i++) {
    bits[i] = *ac_table++;
    count += bits[i];
  }
  nac_table -= count;
  if (nac_table < 0 || 256 < count)	/* bad length */
  	return(XIE_ERR);
  for(i = 0; i < count; i++) {
    val[i] = *ac_table++;
  }
  if(XIE_ERR == add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[1], bits, val))
    return(XIE_ERR);
    
  return(0);
}

LOCAL int
#ifdef NeedFunctionPrototypes
load_dc_tables (compress_info_ptr cinfo, 
			UINT8 * dc_table, int ndc_table)
#else
load_dc_tables (cinfo, dc_table, ndc_table)
	compress_info_ptr cinfo;
	UINT8 * dc_table;
	int ndc_table;
#endif	/* NeedFunctionPrototypes */
/* Read a set of DC Huffman tables pointed at by dc_table.
 * There may be one or two tables.
 */
{
  int i;
  int count;
  UINT8 bits[17];
  UINT8 val[256];

  bits[0] = 0;
  /* luminance DC coefficients */  
  count = 0;
  ndc_table -= 16;
  if (ndc_table < 0)				/* bad length */
  	return(XIE_ERR);
  for(i = 1; i < 17; i++) {
    bits[i] = *dc_table++;
    count += bits[i];
  }
  ndc_table -= count;
  if (ndc_table < 0 || 256 < count)	/* bad length */
  	return(XIE_ERR);
  for(i = 0; i < count; i++) {
    val[i] = *dc_table++;
  }
  if(XIE_ERR == add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[0], bits, val))
    return(XIE_ERR);
    
  /* chrominance DC coefficients */  
  count = 0;
  ndc_table -= 16;
  if (ndc_table < 0)				/* bad length */
  	return(XIE_ERR);
  for(i = 1; i < 17; i++) {
    bits[i] = *dc_table++;
    count += bits[i];
  }
  ndc_table -= count;
  if (ndc_table < 0 || 256 < count)	/* bad length */
  	return(XIE_ERR);
  for(i = 0; i < count; i++) {
    val[i] = *dc_table++;
  }
  if(XIE_ERR == add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[1], bits, val))
    return(XIE_ERR);
    
  return(0);
}

/******************************************************************************/

GLOBAL int
#ifdef NeedFunctionPrototypes
JC_INIT (compress_info_ptr cinfo,
	compress_methods_ptr cmethods, external_methods_ptr emethods)
#else
JC_INIT (cinfo, cmethods, emethods)
	compress_info_ptr cinfo;
	compress_methods_ptr cmethods;
	external_methods_ptr emethods;
#endif	/* NeedFunctionPrototypes */
{
  /* Set up links to method structures. */
  cinfo->methods = cmethods;
  cinfo->emethods = emethods;

  /* Set restart to NULL */
  cinfo->XIErestart = XIE_RNUL;

  jselrXIE (cinfo);

  return (XIE_NRML);
}

GLOBAL int
#ifdef NeedFunctionPrototypes
JC_BEGINFRAME (compress_info_ptr cinfo,
	long components, long width, long height,
	UINT8 * q_table, int nq_table, 
	UINT8 * ac_table, int nac_table,
	UINT8 * dc_table, int ndc_table)
#else
JC_BEGINFRAME (cinfo, components, width, height,
		q_table, nq_table, ac_table, nac_table, dc_table, ndc_table)
	compress_info_ptr cinfo; 
	long components; 
	long width; 
	long height;
	UINT8 * q_table;
	int nq_table;
	UINT8 * ac_table;
	int nac_table;
	UINT8 * dc_table;
	int ndc_table;
#endif	/* NeedFunctionPrototypes */
{
  int status;
  int scale_factor = 100;
  	
  if (cinfo->XIErestart == XIE_RNUL) {
    cinfo->input_components = (short)components;
    cinfo->image_width = width;
    cinfo->image_height = height;
    if (components == 1)
      cinfo->in_color_space = CS_GRAYSCALE;
    else
      cinfo->in_color_space = CS_RGB;

    if (((*cinfo->methods->input_init) (cinfo)) == XIE_ERR)	/* jcXIE_init (cinfo); */
      return(XIE_ERR);
  }
  
  if (0 < nq_table) {
  	if ((load_quant_tables (cinfo, q_table, nq_table, scale_factor)) == XIE_ERR)
		return(XIE_ERR);
  }
        
  if (0 < nac_table) {
  	if ((load_ac_tables (cinfo, ac_table, nac_table)) == XIE_ERR)
		return(XIE_ERR);
  }
        
  if (0 < ndc_table) {
  	if ((load_dc_tables (cinfo, dc_table, ndc_table)) == XIE_ERR)
		return(XIE_ERR);
  }
        
  if ((cinfo->XIErestart == XIE_RNUL) || (cinfo->XIErestart == XIE_RWFH)) {
    if ((status = ((*cinfo->methods->write_file_header) (cinfo))) < 0) {
   	  if (status == XIE_ERR)
        return(XIE_ERR);
      cinfo->XIErestart = XIE_RWFH;	/* set restart status */
      return(XIE_OUT);	
    }
  }
  if ((cinfo->XIErestart == XIE_RNUL) || (cinfo->XIErestart == XIE_RWSH)) {
	if ((status = ((*cinfo->methods->write_scan_header) (cinfo))) < 0) {
   	  if (status == XIE_ERR)
        return(XIE_ERR);
      cinfo->XIErestart = XIE_RWSH;	/* set restart status */
      return(XIE_OUT);	
    }
  }

  return (XIE_NRML);
}

GLOBAL int
#ifdef NeedFunctionPrototypes
JC_ENDFRAME (compress_info_ptr cinfo)
#else
JC_ENDFRAME (cinfo)
	compress_info_ptr cinfo;
#endif	/* NeedFunctionPrototypes */
{
  int status;
  	
  /* jcXIE_term (cinfo); */
  if ((cinfo->XIErestart == XIE_RNUL) || (cinfo->XIErestart == XIE_RMCU)) {
	if ((status = ((*cinfo->methods->input_term) (cinfo))) < 0) {
      if (status == XIE_ERR)
      	return(XIE_ERR);
      cinfo->XIErestart = XIE_RMCU;	/* set restart status */
      return(XIE_OUT);	
    }
  }
  
  (*cinfo->methods->write_scan_trailer) (cinfo); /* does nothing */
  if ((cinfo->XIErestart == XIE_RNUL) || (cinfo->XIErestart == XIE_RWFT)) {
	if (((*cinfo->methods->write_file_trailer) (cinfo)) == XIE_OUT) {
	  cinfo->XIErestart = XIE_RWFT;	/* set restart status */
      return(XIE_OUT);	
    }
  }
  return (XIE_NRML);
}

GLOBAL int
#ifdef NeedFunctionPrototypes
JC_SCANLINE_RGB (compress_info_ptr cinfo, 
	int * scanline, JSAMPROW ptr0, JSAMPROW ptr1, JSAMPROW ptr2)
#else
JC_SCANLINE_RGB (cinfo, scanline, ptr0, ptr1, ptr2)
	compress_info_ptr cinfo; 
	int * scanline; 
	JSAMPROW ptr0; 
	JSAMPROW ptr1; 
	JSAMPROW ptr2;
#endif	/* NeedFunctionPrototypes */
{
  JSAMPROW pixel_row[3];
  int row, rows_this_time;
  int status;

  row = *scanline;	
  if (cinfo->XIErestart == XIE_RNUL) {
    if (cinfo->cur_pixel_row <= 0)	{
      rows_this_time = (int) MIN((long) cinfo->rows_in_mem,
                    cinfo->image_height - row);
      if (rows_this_time <= 0)
        return (XIE_NRML);
      cinfo->rows_this_time = rows_this_time;
    }
    pixel_row[0] = ptr0;
    pixel_row[1] = ptr1;
    pixel_row[2] = ptr2;
  }
  if ((cinfo->XIErestart == XIE_RNUL) || (cinfo->XIErestart == XIE_RMCU)) {
    if ((status = jcXIE_get (cinfo, row, pixel_row)) < 0) {
      if (status == XIE_ERR)
      	return(XIE_ERR);
      cinfo->XIErestart = XIE_RMCU;	/* set restart status */
      return(XIE_OUT);	
    }
  }
  
  return (XIE_NRML);
}

GLOBAL int
#ifdef NeedFunctionPrototypes
JC_SCANLINE_GRAY (compress_info_ptr cinfo, int * scanline, JSAMPROW ptr0)
#else
JC_SCANLINE_GRAY (cinfo, scanline, ptr0)
	compress_info_ptr cinfo; 
	int * scanline; 
	JSAMPROW ptr0;
#endif	/* NeedFunctionPrototypes */
{
  JSAMPROW pixel_row[3];
  int row, rows_this_time;
  int status;

  row = *scanline;	
  if (cinfo->XIErestart == XIE_RNUL) {
    if (cinfo->cur_pixel_row <= 0)	{
      rows_this_time = (int) MIN((long) cinfo->rows_in_mem,
                    cinfo->image_height - row);
      if (rows_this_time <= 0)
        return (XIE_NRML);
      cinfo->rows_this_time = rows_this_time;
    }
    pixel_row[0] = ptr0;
    pixel_row[1] = 0;
    pixel_row[2] = 0;
  }
  if ((cinfo->XIErestart == XIE_RNUL) || (cinfo->XIErestart == XIE_RMCU)) {
    if ((status = jcXIE_get (cinfo, row, pixel_row)) < 0) {
      if (status == XIE_ERR)
      	return(XIE_ERR);
      cinfo->XIErestart = XIE_RMCU;	/* set restart status */
      return(XIE_OUT);	
    }
  }
  
  return (XIE_NRML);
}
