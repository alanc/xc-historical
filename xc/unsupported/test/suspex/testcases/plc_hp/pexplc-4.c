/* $XConsortium$ */

/******************************************************************************/
/*                                                                            */
/*  (c) Copyright Hewlett-Packard Company, 1993, Fort Collins, Colorado       */
/*                                                                            */
/*                            All Rights Reserved                             */
/*                                                                            */
/*  Permission to use, copy, modify, and distribute this software and its     */
/*  documentation for any purpose and without fee is hereby granted,          */
/*  provided that the above copyright notices appear in all copies and that   */
/*  both the copyright notices and this permission notice appear in           */
/*  supporting documentation, and that the name of Hewlett-Packard not be     */
/*  used in advertising or publicity pertaining to distribution of the        */
/*  software without specific, written prior permission.                      */
/*                                                                            */
/*  HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD TO THIS         */
/*  SOFTWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        */
/*  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Hewlett-Packard    */
/*  shall not be liable for errors contained herein or direct, indirect,      */
/*  special, incidental or consequential damages in connection with the       */
/*  furnishing, performance or use of this software.                          */
/*                                                                            */
/******************************************************************************/

#include <X11/PEX5/PEXlib.h>
#include <misc.h>
#include <lut.h>


/* 
 * PEXlib Pipeline Context Module:  Test Case 4  --  PEXChangePipelineContext
 *                                  (using Index colors)
 */

test_image(Display         *dpy,
	   XID              resourceID,
           PEXOCRequestType req_type)
    {
      PEXCoord				points[5], mpoints[5], fpoints[5];

      /* Set up some data. */

      points[0].x = 0.1; points[0].y = 0.1; points[0].z = 0.0;
      points[1].x = 0.9; points[1].y = 0.1; points[1].z = 0.0;
      points[2].x = 0.9; points[2].y = 0.9; points[2].z = 0.0;
      points[3].x = 0.1; points[3].y = 0.9; points[3].z = 0.0;
      points[4].x = 0.1; points[4].y = 0.1; points[4].z = 0.0;

      mpoints[0].x = 0.3; mpoints[0].y = 0.3; mpoints[0].z = 0.0;
      mpoints[1].x = 0.7; mpoints[1].y = 0.3; mpoints[1].z = 0.0;
      mpoints[2].x = 0.7; mpoints[2].y = 0.7; mpoints[2].z = 0.0;
      mpoints[3].x = 0.3; mpoints[3].y = 0.7; mpoints[3].z = 0.0;
      mpoints[4].x = 0.3; mpoints[4].y = 0.3; mpoints[4].z = 0.0;

      fpoints[0].x = 0.4; fpoints[0].y = 0.4; fpoints[0].z = 0.0;
      fpoints[1].x = 0.6; fpoints[1].y = 0.4; fpoints[1].z = 0.0;
      fpoints[2].x = 0.6; fpoints[2].y = 0.6; fpoints[2].z = 0.0;
      fpoints[3].x = 0.4; fpoints[3].y = 0.6; fpoints[3].z = 0.0;
      fpoints[4].x = 0.4; fpoints[4].y = 0.4; fpoints[4].z = 0.0;

      PEXPolyline(dpy, resourceID, req_type, 5, points);
      /* PEXMarkers(dpy, resourceID, req_type, 5, mpoints); */
      PEXFillArea(dpy, resourceID, req_type, PEXShapeNonConvex, False, 5, fpoints);
    }


void inquire_test_params(char *test_name, 
			 int  *num_images, 
			 int  *supported_modes, 
			 char *win_title)
    {
    strcpy(test_name, "pexplc-4");
    *num_images      = 1;
    *supported_modes = IMM_MODE | STR_MODE | WKS_MODE;

    strcpy(win_title, "pexplc-4");
    }

void misc_setup(Display *dpy, Window window, PEXRenderer renderer,
		int cur_image)
    {
    PEXRendererAttributes		rndAttrs, *rndAttrs2;
    unsigned long			item_mask = 0;
    unsigned long			valuemask[3];
    PEXPipelineContext                  plc;
    PEXPCAttributes			pc_values, *pc_values2;

    _hppex_set_stderr("pexplc-4.stderr", "pexplc-4");

    valuemask[0] = 0;
    valuemask[1] = 0;
    valuemask[2] = 0;
    plc = PEXCreatePipelineContext(dpy, valuemask, &pc_values);

    valuemask[0] = 0x0A200000;

    pc_values.line_color.type = PEXColorTypeIndexed;
    pc_values.line_color.value.indexed.index = 2;

    pc_values.surface_color.type = PEXColorTypeIndexed;
    pc_values.surface_color.value.indexed.index = 4;

    pc_values.interior_style = PEXInteriorStyleSolid;

    PEXChangePipelineContext(dpy, plc, valuemask, &pc_values);
    pc_values2 = PEXGetPipelineContext(dpy, plc, valuemask);
    if (pc_values2->line_color.type != PEXColorTypeIndexed)
      _hppex_stderr_print("ERROR 1 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");
    if (pc_values2->line_color.value.indexed.index != 2)
      _hppex_stderr_print("ERROR 2 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");
    if (pc_values2->surface_color.type != PEXColorTypeIndexed)
      _hppex_stderr_print("ERROR 3 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");
    if (pc_values2->surface_color.value.indexed.index != 4)
      _hppex_stderr_print("ERROR 4 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");
    if (pc_values2->interior_style != PEXInteriorStyleSolid)
      _hppex_stderr_print("ERROR 5 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");


    item_mask = (PEXRAPipelineContext | PEXRAColorTable | PEXRABackgroundColor | PEXRAClearImage);
    rndAttrs.pipeline_context = plc; 
    rndAttrs.color_table = colorLUT;
    rndAttrs.background_color.type = PEXColorTypeRGB;
    rndAttrs.background_color.value.rgb.red = 0.0;
    rndAttrs.background_color.value.rgb.green = 0.0;
    rndAttrs.background_color.value.rgb.blue = 0.0;
    rndAttrs.clear_image = True;

    PEXChangeRenderer(dpy, renderer, item_mask, &rndAttrs);
    rndAttrs2 = PEXGetRendererAttributes(dpy, renderer, item_mask);
    if (plc != rndAttrs2->pipeline_context)
      _hppex_stderr_print("ERROR 6 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");
    if (colorLUT != rndAttrs2->color_table)
      _hppex_stderr_print("ERROR 7 in PEXlib test pexplc-4:  PEXChangePipelineContext()\n");
    }

void execute_test(Display *dpy, XID resourceID, 
	       PEXOCRequestType req_type, int cur_image)
    {
    describe_test("\nPipeline Context Test #4 (PEXChangePipelineContext with indexed colors)\n\n");
    describe_test(" This tests produces 1 image consisting of an outer red solid\n");
    describe_test(" polyline square and an inner green solid fill area square.\n");
    describe_test(" If correct, an empty stderr file is also produced.\n\n");

    test_image(dpy, resourceID, req_type);
    }

void testproc(Display *dpy, Window window, int cur_image)
    {
    file_testproc("pexplc-4.stderr", "pexplc-4");
     image_testproc("pexplc-4", "pexplc-4", img_desc,
		      dpy,window, 0, 0, 
		      glob_window_width, glob_window_height);
    }
