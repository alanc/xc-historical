/* $XConsortium$ */

/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#include "mipex.h"
#include "misc.h"
#include "miscstruct.h"
#include "PEXErr.h"
#include "miRender.h"
#include "miLight.h"

#include <stdio.h>

#define IRINT(x) ( ((x) >= 0.0) ? ((int)((x) + 0.5)) : ((int)((x) - 0.5)) )

/*++
 |
 | miTransform(pddc, vinput, voutput, 
 |             vert_mat, norm_mat, outtype)
 |
 | Generalized vertex list transform routine.
 |
 | This routine takes in four parameters: a list of vertices,
 | an output point type, and two matrices: one for transforming
 | vertex data, the second for transforming normal data. 
 | The following assumptions are made:
 | 	a)  2DS input is never used 
 |	b)  2D input does not support normal, color or edge info
 |	c)  output types do not have more attributes (color, normal or edge)
 |		than input data types
 |	d)  only color specifiers with triplets of float values are
 |		supported
 |
 | All other cases produce undefined output
 |
 --*/

ddpex3rtn
miTransform(pddc, vinput, voutput, vert_mat, norm_mat, outtype)
	miDDContext	*pddc; 
	miListHeader	*vinput;
	miListHeader	**voutput;
	ddFLOAT		*vert_mat;
	ddFLOAT		*norm_mat;
	ddPointType	outtype;
{
		ddCoord4D	new_pt;
		miListHeader	*output;


    register	int		i, j;
    register	ddFLOAT		*f;
    register	listofddPoint	*pddilist;
    register	listofddPoint	*pddolist;

		ddPointUnion	in_pt,
				out_pt;

		ddCoord2D	tmp_pt;

		ddFLOAT		length;

		int		output_size;

    /* remember that ALL vertex types are of the form:
     *
     *   |---------------------------|---------|----------|---------|
     *             coords               color     normal      edge 
     *					(opt)	  (opt)      (opt)
     */

    /* Don't pass on normals unless valid xform */
    if (norm_mat == NULL) DD_UnSetVertNormal(outtype);

    DD_VertPointSize(outtype, output_size);


    switch (outtype) {

        case DD_RGBFLOAT_POINT4D:
        case DD_HSV_POINT4D:
        case DD_HLS_POINT4D:
        case DD_CIE_POINT4D:
        case DD_NORM_POINT4D:
        case DD_EDGE_POINT4D:
        case DD_RGBFLOAT_NORM_POINT4D:
        case DD_HSV_NORM_POINT4D:
        case DD_HLS_NORM_POINT4D:
        case DD_CIE_NORM_POINT4D:
        case DD_RGBFLOAT_EDGE_POINT4D:
        case DD_HSV_EDGE_POINT4D:
        case DD_HLS_EDGE_POINT4D:
        case DD_CIE_EDGE_POINT4D:
        case DD_NORM_EDGE_POINT4D:
        case DD_RGBFLOAT_NORM_EDGE_POINT4D:
        case DD_HSV_NORM_EDGE_POINT4D:
        case DD_HLS_NORM_EDGE_POINT4D:
        case DD_CIE_NORM_EDGE_POINT4D:
 	case DD_HOMOGENOUS_POINT:
	 {
	  switch (vinput->type) {
	    case DD_2D_POINT:
	      {

		/* Use the pre-defined 4D list for output */
		*voutput = output = MI_NEXTTEMPDATALIST(pddc);

		/* Insure sufficient room for each header */
		MI_ALLOCLISTHEADER(output, vinput->numLists)
		if (!output->ddList) return(BadAlloc);

		output->type = outtype;
		output->numLists = vinput->numLists;
		output->flags =  vinput->flags;

		pddilist = vinput->ddList;
		pddolist = output->ddList;

		/* Now, transform each list */
		for(j=0; j < vinput->numLists; j++) {

		  if ((i = pddolist->numPoints = pddilist->numPoints) <= 0) 
		    continue;

		  /*
		   * Insure sufficient room for each vertex 
		   * Add one to leave room for possible polygon close.
		   */
		  MI_ALLOCLISTOFDDPOINT(pddolist,(i+1),output_size);
 		  if(!(out_pt.p4Dpt = (pddolist->pts.p4Dpt)))
						return(BadAlloc);

		  in_pt.p2Dpt = pddilist->pts.p2Dpt;
	    
		  /* Note - just assume z = 0.0, w = 1.0 */
		  while (i--) {
		    f = vert_mat;

		    out_pt.p4Dpt->x = (*f++)*in_pt.p2Dpt->x;
		    out_pt.p4Dpt->x += (*f++)*in_pt.p2Dpt->y;
		    f++;
		    out_pt.p4Dpt->x += (*f++);

		    out_pt.p4Dpt->y =  (*f++)*in_pt.p2Dpt->x;
		    out_pt.p4Dpt->y +=  (*f++)*in_pt.p2Dpt->y;
		    f++;
		    out_pt.p4Dpt->y +=  (*f++);

		    out_pt.p4Dpt->z =  (*f++)*in_pt.p2Dpt->x;
		    out_pt.p4Dpt->z +=  (*f++)*in_pt.p2Dpt->y;
		    f++;
		    out_pt.p4Dpt->z +=  (*f++);

		    out_pt.p4Dpt->w =  (*f++)*in_pt.p2Dpt->x;
		    out_pt.p4Dpt->w +=  (*f++)*in_pt.p2Dpt->y;
		    f++;
		    out_pt.p4Dpt->w +=  (*f);

		    in_pt.p2Dpt++;
		    out_pt.p4Dpt++;

                      /* At this point we should be pointing to
                       * to the beginning of the next vertex
                       */

		  }
		  pddilist++;
		  pddolist++;
		}
		break;
	      }
            case DD_RGBFLOAT_POINT:
            case DD_HSV_POINT:
            case DD_HLS_POINT:
            case DD_CIE_POINT:
            case DD_NORM_POINT:
            case DD_EDGE_POINT:
            case DD_RGBFLOAT_NORM_POINT:
            case DD_HSV_NORM_POINT:
            case DD_HLS_NORM_POINT:
            case DD_CIE_NORM_POINT:
            case DD_RGBFLOAT_EDGE_POINT:
            case DD_HSV_EDGE_POINT:
            case DD_HLS_EDGE_POINT:
            case DD_CIE_EDGE_POINT:
            case DD_NORM_EDGE_POINT:
            case DD_RGBFLOAT_NORM_EDGE_POINT:
            case DD_HSV_NORM_EDGE_POINT:
            case DD_HLS_NORM_EDGE_POINT:
            case DD_CIE_NORM_EDGE_POINT:
	    case DD_3D_POINT:
	      {

		/* Use the pre-defined 4D list for output */
		*voutput = output = MI_NEXTTEMPDATALIST(pddc);

		/* Insure sufficient room for each header */
		MI_ALLOCLISTHEADER(output, vinput->numLists)
		if (!output->ddList) return(BadAlloc);

		output->type = outtype;
		output->numLists = vinput->numLists;
		output->flags =  vinput->flags;

		pddilist = vinput->ddList;
		pddolist = output->ddList;

		/* Now, transform each list */
		for(j=0; j < vinput->numLists; j++) {

		  if ((i = pddolist->numPoints = pddilist->numPoints) <= 0) 
		    continue;

		  /*
		   * Insure sufficient room for each vertex 
		   * Add one to leave room for possible polygon close.
		   */
		  MI_ALLOCLISTOFDDPOINT(pddolist,(i+1),output_size);
 		  if(!(out_pt.p4Dpt = (pddolist->pts.p4Dpt)))
						return(BadAlloc);

		  in_pt.p3Dpt =  pddilist->pts.p3Dpt;
	    
		  /* Note - just assume w = 1.0 */
		  while (i--) {

		    /* cast operands & transform the coordinates
		       portions    */

		    f = vert_mat;

		    out_pt.p4Dpt->x = (*f++)*in_pt.p3Dpt->x;
		    out_pt.p4Dpt->x += (*f++)*in_pt.p3Dpt->y;
		    out_pt.p4Dpt->x += (*f++)*in_pt.p3Dpt->z;
		    out_pt.p4Dpt->x += (*f++);

		    out_pt.p4Dpt->y =  (*f++)*in_pt.p3Dpt->x;
		    out_pt.p4Dpt->y +=  (*f++)*in_pt.p3Dpt->y;
		    out_pt.p4Dpt->y += (*f++)*in_pt.p3Dpt->z;
		    out_pt.p4Dpt->y +=  (*f++);

		    out_pt.p4Dpt->z =  (*f++)*in_pt.p3Dpt->x;
		    out_pt.p4Dpt->z +=  (*f++)*in_pt.p3Dpt->y;
		    out_pt.p4Dpt->z += (*f++)*in_pt.p3Dpt->z;
		    out_pt.p4Dpt->z +=  (*f++);

		    out_pt.p4Dpt->w =  (*f++)*in_pt.p3Dpt->x;
		    out_pt.p4Dpt->w +=  (*f++)*in_pt.p3Dpt->y;
		    out_pt.p4Dpt->w += (*f++)*in_pt.p3Dpt->z;
		    out_pt.p4Dpt->w +=  (*f);

		    in_pt.p3Dpt++;
		    out_pt.p4Dpt++;

		    if (!DD_IsVertCoordsOnly(outtype)) {

		      if DD_IsVertColour(outtype) { 
			*out_pt.pRgbFloatClr = 
					*in_pt.pRgbFloatClr;
			in_pt.pRgbFloatClr++;
			out_pt.pRgbFloatClr++;
		      }


		      if DD_IsVertNormal(outtype) {

                        f = norm_mat;

                        out_pt.pNormal->x = (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->z;
                        (*f++);	/* no translation */

                        out_pt.pNormal->y =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->y +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->y += (*f++)*in_pt.pNormal->z;
                        (*f++);	/* no translation */

                        out_pt.pNormal->z =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->z +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->z += (*f++)*in_pt.pNormal->z;
                        (*f++);	/* no translation */

			NORMALIZE_VECTOR (out_pt.pNormal, length);

			in_pt.pNormal++;
			out_pt.pNormal++;
 		      }

		      if (DD_IsVertEdge(outtype)) { 
			*out_pt.pEdge = *in_pt.pEdge;
			in_pt.pEdge++;
			out_pt.pEdge++;
		      }
		    }

		      /* At this point we should be pointing to
		       * to the beginning of the next vertex
		       */
		  }
		  pddilist++;
		  pddolist++;
		}
		break;
	      }
            case DD_RGBFLOAT_POINT4D:
            case DD_HSV_POINT4D:
            case DD_HLS_POINT4D:
            case DD_CIE_POINT4D:
            case DD_NORM_POINT4D:
            case DD_EDGE_POINT4D:
            case DD_RGBFLOAT_NORM_POINT4D:
            case DD_HSV_NORM_POINT4D:
            case DD_HLS_NORM_POINT4D:
            case DD_CIE_NORM_POINT4D:
            case DD_RGBFLOAT_EDGE_POINT4D:
            case DD_HSV_EDGE_POINT4D:
            case DD_HLS_EDGE_POINT4D:
            case DD_CIE_EDGE_POINT4D:
            case DD_NORM_EDGE_POINT4D:
            case DD_RGBFLOAT_NORM_EDGE_POINT4D:
            case DD_HSV_NORM_EDGE_POINT4D:
            case DD_HLS_NORM_EDGE_POINT4D:
            case DD_CIE_NORM_EDGE_POINT4D:
            case DD_HOMOGENOUS_POINT:
	      {

		/* Use the pre-defined 4D list for output */
		*voutput = output = MI_NEXTTEMPDATALIST(pddc);

		/* Insure sufficient room for each header */
		MI_ALLOCLISTHEADER(output, vinput->numLists)
		if (!output->ddList) return(BadAlloc);

		output->type = outtype;
		output->numLists = vinput->numLists;
		output->flags =  vinput->flags;

		pddilist = vinput->ddList;
		pddolist = output->ddList;

		/* Now, transform each list */
		for(j=0; j < vinput->numLists; j++) {

		  if ((i = pddolist->numPoints = pddilist->numPoints) <= 0) 
		    continue;

		  /*
		   * Insure sufficient room for each vertex 
		   * Add one to leave room for possible polygon close.
		   */
		  MI_ALLOCLISTOFDDPOINT(pddolist,(i+1),output_size);
 		  if(!(out_pt.p4Dpt = (pddolist->pts.p4Dpt)))
						return(BadAlloc);

		  in_pt.p4Dpt = pddilist->pts.p4Dpt;
	    
		  /* Note - just assume w = 1.0 */
		  while (i--) {

		    /* cast operands & transform the coordinates */
		    f = vert_mat;

		    out_pt.p4Dpt->x = (*f++)*in_pt.p4Dpt->x;
		    out_pt.p4Dpt->x += (*f++)*in_pt.p4Dpt->y;
		    out_pt.p4Dpt->x += (*f++)*in_pt.p4Dpt->z;
		    out_pt.p4Dpt->x += (*f++)*in_pt.p4Dpt->w;

		    out_pt.p4Dpt->y =  (*f++)*in_pt.p4Dpt->x;
		    out_pt.p4Dpt->y +=  (*f++)*in_pt.p4Dpt->y;
		    out_pt.p4Dpt->y += (*f++)*in_pt.p4Dpt->z;
		    out_pt.p4Dpt->y +=  (*f++)*in_pt.p4Dpt->w;

		    out_pt.p4Dpt->z =  (*f++)*in_pt.p4Dpt->x;
		    out_pt.p4Dpt->z +=  (*f++)*in_pt.p4Dpt->y;
		    out_pt.p4Dpt->z += (*f++)*in_pt.p4Dpt->z;
		    out_pt.p4Dpt->z +=  (*f++)*in_pt.p4Dpt->w;

		    out_pt.p4Dpt->w =  (*f++)*in_pt.p4Dpt->x;
		    out_pt.p4Dpt->w +=  (*f++)*in_pt.p4Dpt->y;
		    out_pt.p4Dpt->w += (*f++)*in_pt.p4Dpt->z;
		    out_pt.p4Dpt->w +=  (*f)*in_pt.p4Dpt->w;

		    in_pt.p4Dpt++;
		    out_pt.p4Dpt++;

                    if (!DD_IsVertCoordsOnly(outtype)) {
                     
                      if DD_IsVertColour(outtype) {
			*out_pt.pRgbFloatClr = 
					*in_pt.pRgbFloatClr;
			in_pt.pRgbFloatClr++;
			out_pt.pRgbFloatClr++;
                      }
 
                      if DD_IsVertNormal(outtype) {
 
                        f = norm_mat;
 
                        out_pt.pNormal->x = (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->z;
                        (*f++); /* no translation */
 
                        out_pt.pNormal->y =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->y +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->y += (*f++)*in_pt.pNormal->z;
                        (*f++); /* no translation */
 
                        out_pt.pNormal->z =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->z +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->z += (*f++)*in_pt.pNormal->z;

			NORMALIZE_VECTOR (out_pt.pNormal, length);

			in_pt.pNormal++;
			out_pt.pNormal++;
                      }

                      if (DD_IsVertEdge(outtype)) {
			*out_pt.pEdge= *in_pt.pEdge;
			in_pt.pEdge++;
			out_pt.pEdge++;
                      }
		    }

		  }

		  pddilist++;
		  pddolist++;
		}
		break;
	      }
	    default:
		*voutput = NULL;
		return(1);
	  }

	  return(0);
	 }

	/* 
	 * Next case output a 2DS point. Note that this point is
	 * normalized by w if necessary.
	 */
	case DD_2DS_POINT:
	case DD_RGBFLOAT_POINT2DS:
	case DD_HSV_POINT2DS:
	case DD_HLS_POINT2DS:
	case DD_CIE_POINT2DS:
	case DD_NORM_POINT2DS:
	case DD_EDGE_POINT2DS:
	case DD_RGBFLOAT_NORM_POINT2DS:
	case DD_HSV_NORM_POINT2DS:
	case DD_HLS_NORM_POINT2DS:
	case DD_CIE_NORM_POINT2DS:
	case DD_RGBFLOAT_EDGE_POINT2DS:
	case DD_HSV_EDGE_POINT2DS:
	case DD_HLS_EDGE_POINT2DS:
	case DD_CIE_EDGE_POINT2DS:
	case DD_NORM_EDGE_POINT2DS:
	case DD_RGBFLOAT_NORM_EDGE_POINT2DS:
	case DD_HSV_NORM_EDGE_POINT2DS:
	case DD_HLS_NORM_EDGE_POINT2DS:
	case DD_CIE_NORM_EDGE_POINT2DS:
	 {
	  ddFLOAT w;
	  switch (vinput->type) {
	     case DD_2D_POINT:
	      {

		/* Use the pre-defined 2D list for output */
		*voutput = output = &pddc->Static.misc.list2D;

		/* Insure sufficient room for each header */
		MI_ALLOCLISTHEADER(output, vinput->numLists)
		if (!output->ddList) return(BadAlloc);

		output->type = outtype;
		output->numLists = vinput->numLists;
		output->flags =  vinput->flags;

		pddilist = vinput->ddList;
		pddolist = output->ddList;

		/* Now, transform each list */
		for(j=0; j < vinput->numLists; j++) {

		  if ((i = pddolist->numPoints = pddilist->numPoints) <= 0) 
		    continue;

		  /*
		   * Insure sufficient room for each vertex 
		   * Add one to leave room for possible polygon close.
		   */
		  MI_ALLOCLISTOFDDPOINT(pddolist,(i+1),output_size);
 		  if(!(out_pt.p2DSpt = (pddolist->pts.p2DSpt)))
						return(BadAlloc);


		  in_pt.p2DSpt = pddilist->pts.p2DSpt;
	    
		  /* Note - just assume z = 0.0, w = 1.0 */
		  while (i--) {

		    /* cast operands & transform the coordinates */
		    f = vert_mat;

		    tmp_pt.x = (*f++)*in_pt.p2Dpt->x;
		    tmp_pt.x += (*f++)*in_pt.p2Dpt->y;
		    f++;
		    tmp_pt.x += (*f++);

		    tmp_pt.y =  (*f++)*in_pt.p2Dpt->x;
		    tmp_pt.y +=  (*f++)*in_pt.p2Dpt->y;
		    f++;
		    tmp_pt.y +=  (*f++);

		    /* Skip Z transformation */
		    f += 4;

		    /* The w must be computed to normalize the result */
		    w =  (*f++)*in_pt.p2Dpt->x;
		    w +=  (*f++)*in_pt.p2Dpt->y;
		    f++;
		    w +=  (*f++);

		    /* Now round and normalize the result */
		    if (w != 1.0) {
		      out_pt.p2DSpt->x = (ddSHORT)(tmp_pt.x / w);
		      out_pt.p2DSpt->y = (ddSHORT)(tmp_pt.y / w);
		    } else {
		      out_pt.p2DSpt->x = (ddSHORT)(tmp_pt.x);
		      out_pt.p2DSpt->y = (ddSHORT)(tmp_pt.y);
		    }
		    in_pt.p2Dpt++;
		    out_pt.p2DSpt++;


                      /* At this point we should be pointing to 
                       * to the beginning of the next vertex
                       */ 

		  }
		  pddilist++;
		  pddolist++;
		}
		break;
	      }
	    case DD_3D_POINT:
            case DD_RGBFLOAT_POINT:
            case DD_HSV_POINT:
            case DD_HLS_POINT:
            case DD_CIE_POINT:
            case DD_NORM_POINT:
            case DD_EDGE_POINT:
            case DD_RGBFLOAT_NORM_POINT:
            case DD_HSV_NORM_POINT:
            case DD_HLS_NORM_POINT:
            case DD_CIE_NORM_POINT:
            case DD_RGBFLOAT_EDGE_POINT:
            case DD_HSV_EDGE_POINT:
            case DD_HLS_EDGE_POINT:
            case DD_CIE_EDGE_POINT:
            case DD_NORM_EDGE_POINT:
            case DD_RGBFLOAT_NORM_EDGE_POINT:
            case DD_HSV_NORM_EDGE_POINT:
            case DD_HLS_NORM_EDGE_POINT:
            case DD_CIE_NORM_EDGE_POINT:
	      {

		/* Use the pre-defined 2D list for output */
		*voutput = output = &pddc->Static.misc.list2D;

		/* Insure sufficient room for each header */
		MI_ALLOCLISTHEADER(output, vinput->numLists)
		if (!output->ddList) return(BadAlloc);

		output->type = outtype;
		output->numLists = vinput->numLists;
		output->flags =  vinput->flags;

		pddilist = vinput->ddList;
		pddolist = output->ddList;

		/* Now, transform each list */
		for(j=0; j < vinput->numLists; j++) {

		  if ((i = pddolist->numPoints = pddilist->numPoints) <= 0) 
		    continue;

		  /*
		   * Insure sufficient room for each vertex 
		   * Add one to leave room for possible polygon close.
		   */
		  MI_ALLOCLISTOFDDPOINT(pddolist,(i+1),output_size);
 		  if(!(out_pt.p2DSpt = (pddolist->pts.p2DSpt)))
						return(BadAlloc);


		  in_pt.p3Dpt = pddilist->pts.p3Dpt;
	    
		  /* Note - just assume z = 0.0, w = 1.0 */
		  while (i--) {

		    /* cast operands & transform the coordinates */
		    f = vert_mat;

		    tmp_pt.x = (*f++)*in_pt.p3Dpt->x;
		    tmp_pt.x += (*f++)*in_pt.p3Dpt->y;
		    tmp_pt.x += (*f++)*in_pt.p3Dpt->z;
		    tmp_pt.x += (*f++);

		    tmp_pt.y =  (*f++)*in_pt.p3Dpt->x;
		    tmp_pt.y +=  (*f++)*in_pt.p3Dpt->y;
		    tmp_pt.y += (*f++)*in_pt.p3Dpt->z;
		    tmp_pt.y +=  (*f++);

		    /* Skip Z transformation */
		    f += 4;

		    /* The w must be computed to normalize the result */
		    w =  (*f++)*in_pt.p3Dpt->x;
		    w +=  (*f++)*in_pt.p3Dpt->y;
		    w +=  (*f++)*in_pt.p3Dpt->z;
		    w +=  (*f++);

		    /* Now round and normalize the result */
		    if (w != 1.0) {
		      out_pt.p2DSpt->x = IRINT(tmp_pt.x / w);
		      out_pt.p2DSpt->y = IRINT(tmp_pt.y / w);
		    } else {
		      out_pt.p2DSpt->x = IRINT(tmp_pt.x);
		      out_pt.p2DSpt->y = IRINT(tmp_pt.y);
		    }
		    in_pt.p3Dpt++;
		    out_pt.p2DSpt++;


                    if (!DD_IsVertCoordsOnly(outtype)) {

                      if DD_IsVertColour(outtype) {
                        *out_pt.pRgbFloatClr = 
					*in_pt.pRgbFloatClr;
                        in_pt.pRgbFloatClr++;
                        out_pt.pRgbFloatClr++;
                      }

                      if DD_IsVertNormal(outtype) {

                        f = norm_mat;

                        out_pt.pNormal->x = (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->z;
                        (*f++); /* no translation */

                        out_pt.pNormal->y =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->y +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->y += (*f++)*in_pt.pNormal->z;
                        (*f++); /* no translation */

                        out_pt.pNormal->z =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->z +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->z += (*f++)*in_pt.pNormal->z;

			NORMALIZE_VECTOR (out_pt.pNormal, length);

                        in_pt.pNormal++;
                        out_pt.pNormal++;
                      }

                      if (DD_IsVertEdge(outtype)) {
                        *out_pt.pEdge= *in_pt.pEdge;
                        in_pt.pEdge++;
                        out_pt.pEdge++;
                      }

                    }

		  }
		  pddilist++;
		  pddolist++;
		}
		break;
	      }
	    case DD_RGBFLOAT_POINT4D:
	    case DD_HSV_POINT4D:
	    case DD_HLS_POINT4D:
	    case DD_CIE_POINT4D:
	    case DD_NORM_POINT4D:
	    case DD_EDGE_POINT4D:
	    case DD_RGBFLOAT_NORM_POINT4D:
	    case DD_HSV_NORM_POINT4D:
	    case DD_HLS_NORM_POINT4D:
	    case DD_CIE_NORM_POINT4D:
	    case DD_RGBFLOAT_EDGE_POINT4D:
	    case DD_HSV_EDGE_POINT4D:
	    case DD_HLS_EDGE_POINT4D:
	    case DD_CIE_EDGE_POINT4D:
	    case DD_NORM_EDGE_POINT4D:
	    case DD_RGBFLOAT_NORM_EDGE_POINT4D:
	    case DD_HSV_NORM_EDGE_POINT4D:
	    case DD_HLS_NORM_EDGE_POINT4D:
	    case DD_CIE_NORM_EDGE_POINT4D:
	    case DD_HOMOGENOUS_POINT:
	      {

		/* Use the pre-defined 2D list for output */
		*voutput = output = &pddc->Static.misc.list2D;

		/* Insure sufficient room for each header */
		MI_ALLOCLISTHEADER(output, vinput->numLists)
		if (!output->ddList) return(BadAlloc);

		output->type = outtype;
		output->flags =  vinput->flags;
		output->numLists =  vinput->numLists;

		pddilist = vinput->ddList;
		pddolist = output->ddList;

		/* Now, transform each list */
		for(j = 0; j < vinput->numLists; j++) {

		  if ((i = pddolist->numPoints = pddilist->numPoints) <= 0) 
		    continue;

		  /*
		   * Insure sufficient room for each vertex 
		   * Add one to leave room for possible polygon close.
		   */
		  MI_ALLOCLISTOFDDPOINT(pddolist, (i+1), output_size);
 		  if(!(out_pt.p2DSpt = (pddolist->pts.p2DSpt)))
						return(BadAlloc);

		  in_pt.p4Dpt = pddilist->pts.p4Dpt;
	    
		  /* Note - just assume w = 1.0 */
		  while (i--) {

		    /* cast operands & transform the coordinates */
		    f = vert_mat;

		    tmp_pt.x = (*f++)*in_pt.p4Dpt->x;
		    tmp_pt.x += (*f++)*in_pt.p4Dpt->y;
		    tmp_pt.x += (*f++)*in_pt.p4Dpt->z;
		    tmp_pt.x += (*f++)*in_pt.p4Dpt->w;

		    tmp_pt.y =  (*f++)*in_pt.p4Dpt->x;
		    tmp_pt.y +=  (*f++)*in_pt.p4Dpt->y;
		    tmp_pt.y += (*f++)*in_pt.p4Dpt->z;
		    tmp_pt.y +=  (*f++)*in_pt.p4Dpt->w;

		    /* Skip Z transformation */
		    f += 4;

		    w =  (*f++)*in_pt.p4Dpt->x;
		    w +=  (*f++)*in_pt.p4Dpt->y;
		    w += (*f++)*in_pt.p4Dpt->z;
		    w +=  (*f)*in_pt.p4Dpt->w;

		    /* Now round and normal->ze the result */
		    if (w != 1.0) {
		      out_pt.p2DSpt->x = IRINT(tmp_pt.x / w);
		      out_pt.p2DSpt->y = IRINT(tmp_pt.y / w);
		    } else {
		      out_pt.p2DSpt->x = IRINT(tmp_pt.x);
		      out_pt.p2DSpt->y = IRINT(tmp_pt.y);
		    }

		    in_pt.p4Dpt++;
		    out_pt.p2DSpt++;


                    if (!DD_IsVertCoordsOnly(outtype)) {

                      if DD_IsVertColour(outtype) {
                        *out_pt.pRgbFloatClr = 
					*in_pt.pRgbFloatClr;
                        in_pt.pRgbFloatClr++;
                        out_pt.pRgbFloatClr++;
                      }

                      if DD_IsVertNormal(outtype) {

                        f = norm_mat;

                        out_pt.pNormal->x = (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->x += (*f++)*in_pt.pNormal->z;
                        (*f++); /* no translation */

                        out_pt.pNormal->y =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->y +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->y += (*f++)*in_pt.pNormal->z;
                        (*f++); /* no translation */

                        out_pt.pNormal->z =  (*f++)*in_pt.pNormal->x;
                        out_pt.pNormal->z +=  (*f++)*in_pt.pNormal->y;
                        out_pt.pNormal->z += (*f++)*in_pt.pNormal->z;

			NORMALIZE_VECTOR (out_pt.pNormal, length);

                        in_pt.pNormal++;
                        out_pt.pNormal++;
                      }

                      if (DD_IsVertEdge(outtype)) {
                        *out_pt.pEdge= *in_pt.pEdge;
                        in_pt.pEdge++;
                        out_pt.pEdge++;
                      }

                    }

		  }
		  pddilist++;
		  pddolist++;
		}
		break;
	      }

	    /* no more input types for 2DS outputs*/
	    default:
		*voutput = NULL;
		return(1);
	    break;
	  }

      return(0);
    }

    /* no more output types left */
    default:
      *voutput = NULL;
      return(1);
    break;
  }


}

/*++
 |
 | miFacetTransform(pddc, finput, foutput, norm_mat)
 |
 | Facet list transform routine.
 |
 | General transform routine for PEX.
 |
 | This routine takes in four parameters: a list of vertices,
 | an output point type, and two matrices: one for transforming
 | vertex data, the second for transforming normal data. 
 | The following assumptions are made:
 | 	a)  2DS input is never used 
 |	b)  2D input does not support normal, color or edge info
 |	c)  output types do not have more attributes (color, normal or edge)
 |		than input data types
 |	d)  only color specifiers with triplets of float values are
 |		supported
 |
 | All other cases produce undefined output
 |
 --*/

ddpex3rtn
miFacetTransform(pddc, finput, foutput, norm_mat)
	miDDContext	*pddc; 
	listofddFacet	*finput;
	listofddFacet	**foutput;
	ddFLOAT		*norm_mat;
{

    listofddFacet	*fct_list;
    ddFacetUnion	in_fct;
    ddFacetUnion	out_fct;
    ddFLOAT		*f;
    ddFLOAT		length;
    char		color_flag;
    int			j;
    int			facet_size;

    /* Some quick error checking */
    if (!DD_IsFacetNormal(finput->type)) {
      *foutput = finput;
      return(Success);
    }

    /*
     * First, allocate storage for the facet list
     */
    fct_list = MI_NEXTTEMPFACETLIST(pddc);
    fct_list->type = finput->type;

    DDFacetSIZE(finput->type, facet_size);
    MI_ALLOCLISTOFDDFACET(fct_list, finput->numFacets, facet_size);
    if (!(out_fct.pNoFacet = fct_list->facets.pNoFacet)) return(BadAlloc);

    color_flag = DD_IsFacetColour(finput->type);

    in_fct = finput->facets;



    /* Remember, facet data is of the form:
     *
     * |--------------|--------------------------|
     *   color (opt)         normal (opt)
     */

    for (j = 0; j < finput->numFacets; j++) {

	/* Copy the input facet color */
	if (color_flag)
	  *(out_fct.pFacetRgbFloat++) = *(in_fct.pFacetRgbFloat++);

	f = norm_mat;

	out_fct.pFacetN->x = (*f++)*in_fct.pFacetN->x;
	out_fct.pFacetN->x += (*f++)*in_fct.pFacetN->y;
	out_fct.pFacetN->x += (*f++)*in_fct.pFacetN->z;
	(*f++); /* no translation */

	out_fct.pFacetN->y =  (*f++)*in_fct.pFacetN->x;
	out_fct.pFacetN->y +=  (*f++)*in_fct.pFacetN->y;
	out_fct.pFacetN->y += (*f++)*in_fct.pFacetN->z;
	(*f++); /* no translation */

	out_fct.pFacetN->z =  (*f++)*in_fct.pFacetN->x;
	out_fct.pFacetN->z +=  (*f++)*in_fct.pFacetN->y;
	out_fct.pFacetN->z += (*f++)*in_fct.pFacetN->z;

	NORMALIZE_VECTOR (out_fct.pFacetN, length);

	/* Process next facet */
	in_fct.pFacetN++;
	out_fct.pFacetN++;
    }

    fct_list->numFacets = finput->numFacets;
    *foutput = fct_list;

    return(Success);

}

/*++
 |
 | miBoundsTransform(pddc, ibounds, obounds, mat)
 |
 | Transforms a ddListBounds stucture by the specified matrix.
 |
 --*/

ddpex3rtn
miBoundsTransform(pddc, ibounds, obounds, mat)
	miDDContext	*pddc; 
	ddListBounds	*ibounds;
	ddListBounds	*obounds;
	ddFLOAT		*mat;
{

    ddFLOAT	*f = mat;

    obounds->xmin =  (*f++)*ibounds->xmin;
    obounds->xmin += (*f++)*ibounds->ymin;
    obounds->xmin += (*f++)*ibounds->zmin;
    obounds->xmin += (*f++)*ibounds->wmin;

    obounds->ymin =  (*f++)*ibounds->xmin;
    obounds->ymin += (*f++)*ibounds->ymin;
    obounds->ymin += (*f++)*ibounds->zmin;
    obounds->ymin += (*f++)*ibounds->wmin;

    obounds->zmin =  (*f++)*ibounds->xmin;
    obounds->zmin += (*f++)*ibounds->ymin;
    obounds->zmin += (*f++)*ibounds->zmin;
    obounds->zmin += (*f++)*ibounds->wmin;

    obounds->wmin =  (*f++)*ibounds->xmin;
    obounds->wmin += (*f++)*ibounds->ymin;
    obounds->wmin += (*f++)*ibounds->zmin;
    obounds->wmin += (*f++)*ibounds->wmin;

    f = mat;

    obounds->xmax =  (*f++)*ibounds->xmax;
    obounds->xmax += (*f++)*ibounds->ymax;
    obounds->xmax += (*f++)*ibounds->zmax;
    obounds->xmax += (*f++)*ibounds->wmax;

    obounds->ymax =  (*f++)*ibounds->xmax;
    obounds->ymax += (*f++)*ibounds->ymax;
    obounds->ymax += (*f++)*ibounds->zmax;
    obounds->ymax += (*f++)*ibounds->wmax;

    obounds->zmax =  (*f++)*ibounds->xmax;
    obounds->zmax += (*f++)*ibounds->ymax;
    obounds->zmax += (*f++)*ibounds->zmax;
    obounds->zmax += (*f++)*ibounds->wmax;

    obounds->wmax =  (*f++)*ibounds->xmax;
    obounds->wmax += (*f++)*ibounds->ymax;
    obounds->wmax += (*f++)*ibounds->zmax;
    obounds->wmax += (*f++)*ibounds->wmax;

    return(Success);

}
