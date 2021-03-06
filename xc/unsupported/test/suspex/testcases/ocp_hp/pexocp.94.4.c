/* $XConsortium: pexocp.94.4.c,v 1.0 93/11/22 12:36:27 rws Exp $ */

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
 * PEXlib FillAreaSet 3D with data:  Test Case 4 (fewer than 3 vertices)
 */

draw_image  (Display          *dpy,
	     XID              resourceID,
	     PEXOCRequestType req_type)
    {
      PEXVertexRGBNormalEdge		points_1[5], points_2[3], points_3[2];
      PEXVertexRGBNormalEdge		points_4[1], points_5[8], points_6[5];
      PEXListOfVertex			point_list_1[4], point_list_2[2];
      PEXColor				p_surface_color, p_edge_color;
      PEXTableIndex			lights_on[4];

      /* Set up some data. */

      points_1[0].point.x = 0.1;
      points_1[0].point.y = 0.1;
      points_1[0].point.z = 0.0;
      points_1[0].rgb.red = 0.0;
      points_1[0].rgb.green = 0.0;
      points_1[0].rgb.blue = 1.0;
      points_1[0].normal.x = 0.5773502;
      points_1[0].normal.y = 0.5773502;
      points_1[0].normal.z = 0.5773502;
      points_1[0].edge = 1;
      points_1[1].point.x = 0.3;
      points_1[1].point.y = 0.1;
      points_1[1].point.z = 0.0;
      points_1[1].rgb.red = 0.0;
      points_1[1].rgb.green = 0.0;
      points_1[1].rgb.blue = 1.0;
      points_1[1].normal.x = 0.5773502;
      points_1[1].normal.y = 0.5773502;
      points_1[1].normal.z = 0.5773502;
      points_1[1].edge = 1;
      points_1[2].point.x = 0.3;
      points_1[2].point.y = 0.3;
      points_1[2].point.z = 0.0;
      points_1[2].rgb.red = 0.0;
      points_1[2].rgb.green = 0.0;
      points_1[2].rgb.blue = 1.0;
      points_1[2].normal.x = 0.5773502;
      points_1[2].normal.y = 0.5773502;
      points_1[2].normal.z = 0.5773502;
      points_1[2].edge = 1;
      points_1[3].point.x = 0.1;
      points_1[3].point.y = 0.3;
      points_1[3].point.z = 0.0;
      points_1[3].rgb.red = 0.0;
      points_1[3].rgb.green = 0.0;
      points_1[3].rgb.blue = 1.0;
      points_1[3].normal.x = 0.5773502;
      points_1[3].normal.y = 0.5773502;
      points_1[3].normal.z = 0.5773502;
      points_1[3].edge = 1;
      points_1[4].point.x = 0.1;
      points_1[4].point.y = 0.1;
      points_1[4].point.z = 0.0;
      points_1[4].rgb.red = 0.0;
      points_1[4].rgb.green = 0.0;
      points_1[4].rgb.blue = 1.0;
      points_1[4].normal.x = 0.5773502;
      points_1[4].normal.y = 0.5773502;
      points_1[4].normal.z = 0.5773502;
      points_1[4].edge = 1;

      points_2[0].point.x = 0.7;
      points_2[0].point.y = 0.1;
      points_2[0].point.z = 0.0;
      points_2[0].rgb.red = 1.0;
      points_2[0].rgb.green = 1.0;
      points_2[0].rgb.blue = 1.0;
      points_2[0].normal.x = 0.5773502;
      points_2[0].normal.y = 0.5773502;
      points_2[0].normal.z = 0.5773502;
      points_2[0].edge = 1;
      points_2[1].point.x = 0.9;
      points_2[1].point.y = 0.1;
      points_2[1].point.z = 0.0;
      points_2[1].rgb.red = 1.0;
      points_2[1].rgb.green = 0.0;
      points_2[1].rgb.blue = 0.0;
      points_2[1].normal.x = 0.5773502;
      points_2[1].normal.y = 0.5773502;
      points_2[1].normal.z = 0.5773502;
      points_2[1].edge = 1;
      points_2[2].point.x = 0.9;
      points_2[2].point.y = 0.3;
      points_2[2].point.z = 0.0;
      points_2[2].rgb.red = 0.0;
      points_2[2].rgb.green = 1.0;
      points_2[2].rgb.blue = 0.0;
      points_2[2].normal.x = 0.5773502;
      points_2[2].normal.y = 0.5773502;
      points_2[2].normal.z = 0.5773502;
      points_2[2].edge = 1;

      points_3[0].point.x = 0.7;
      points_3[0].point.y = 0.7;
      points_3[0].point.z = 0.0;
      points_3[0].rgb.red = 1.0;
      points_3[0].rgb.green = 0.0;
      points_3[0].rgb.blue = 0.0;
      points_3[0].normal.x = 0.5773502;
      points_3[0].normal.y = 0.5773502;
      points_3[0].normal.z = 0.5773502;
      points_3[0].edge = 1;
      points_3[1].point.x = 0.9;
      points_3[1].point.y = 0.7;
      points_3[1].point.z = 0.0;
      points_3[1].rgb.red = 1.0;
      points_3[1].rgb.green = 0.0;
      points_3[1].rgb.blue = 0.0;
      points_3[1].normal.x = 0.5773502;
      points_3[1].normal.y = 0.5773502;
      points_3[1].normal.z = 0.5773502;
      points_3[1].edge = 1;

      points_4[0].point.x = 0.1;
      points_4[0].point.y = 0.7;
      points_4[0].point.z = 0.0;
      points_4[0].rgb.red = 0.0;
      points_4[0].rgb.green = 1.0;
      points_4[0].rgb.blue = 0.0;
      points_4[0].normal.x = -0.5773502;
      points_4[0].normal.y = 0.5773502;
      points_4[0].normal.z = 0.5773502;
      points_4[0].edge = 1;

      points_5[0].point.x = 0.45;
      points_5[0].point.y = 0.45;
      points_5[0].point.z = 0.5;
      points_5[0].rgb.red = 1.0;
      points_5[0].rgb.green = 1.0;
      points_5[0].rgb.blue = 1.0;
      points_5[0].normal.x = 0.5773502;
      points_5[0].normal.y = 0.5773502;
      points_5[0].normal.z = 0.5773502;
      points_5[0].edge = 0;
      points_5[1].point.x = 0.50;
      points_5[1].point.y = 0.25;
      points_5[1].point.z = 0.5;
      points_5[1].rgb.red = 1.0;
      points_5[1].rgb.green = 0.0;
      points_5[1].rgb.blue = 0.0;
      points_5[1].normal.x = 0.5773502;
      points_5[1].normal.y = 0.5773502;
      points_5[1].normal.z = 0.5773502;
      points_5[1].edge = 1;
      points_5[2].point.x = 0.55;
      points_5[2].point.y = 0.45;
      points_5[2].point.z = 0.5;
      points_5[2].rgb.red = 0.0;
      points_5[2].rgb.green = 1.0;
      points_5[2].rgb.blue = 0.0;
      points_5[2].normal.x = 0.5773502;
      points_5[2].normal.y = 0.5773502;
      points_5[2].normal.z = 0.5773502;
      points_5[2].edge = 0;
      points_5[3].point.x = 0.75;
      points_5[3].point.y = 0.50;
      points_5[3].point.z = 0.5;
      points_5[3].rgb.red = 0.0;
      points_5[3].rgb.green = 0.0;
      points_5[3].rgb.blue = 1.0;
      points_5[3].normal.x = 0.5773502;
      points_5[3].normal.y = 0.5773502;
      points_5[3].normal.z = 0.5773502;
      points_5[3].edge = 1;
      points_5[4].point.x = 0.55;
      points_5[4].point.y = 0.55;
      points_5[4].point.z = 0.5;
      points_5[4].rgb.red = 1.0;
      points_5[4].rgb.green = 1.0;
      points_5[4].rgb.blue = 0.0;
      points_5[4].normal.x = 0.5773502;
      points_5[4].normal.y = 0.5773502;
      points_5[4].normal.z = 0.5773502;
      points_5[4].edge = 0;
      points_5[5].point.x = 0.50;
      points_5[5].point.y = 0.75;
      points_5[5].point.z = 0.5;
      points_5[5].rgb.red = 1.0;
      points_5[5].rgb.green = 0.0;
      points_5[5].rgb.blue = 1.0;
      points_5[5].normal.x = 0.5773502;
      points_5[5].normal.y = 0.5773502;
      points_5[5].normal.z = 0.5773502;
      points_5[5].edge = 1;
      points_5[6].point.x = 0.45;
      points_5[6].point.y = 0.55;
      points_5[6].point.z = 0.5;
      points_5[6].rgb.red = 0.0;
      points_5[6].rgb.green = 1.0;
      points_5[6].rgb.blue = 1.0;
      points_5[6].normal.x = 0.5773502;
      points_5[6].normal.y = 0.5773502;
      points_5[6].normal.z = 0.5773502;
      points_5[6].edge = 0;
      points_5[7].point.x = 0.25;
      points_5[7].point.y = 0.50;
      points_5[7].point.z = 0.5;
      points_5[7].rgb.red = 1.0;
      points_5[7].rgb.green = 1.0;
      points_5[7].rgb.blue = 1.0;
      points_5[7].normal.x = 0.5773502;
      points_5[7].normal.y = 0.5773502;
      points_5[7].normal.z = 0.5773502;
      points_5[7].edge = 1;

      /* Now, hook up the points into lists. */
      point_list_1[0].count = 5;
      point_list_1[0].vertices.rgb_normal_edge = points_1;
      point_list_1[1].count = 3;
      point_list_1[1].vertices.rgb_normal_edge = points_2;
      point_list_1[2].count = 2;
      point_list_1[2].vertices.rgb_normal_edge = points_3;
      point_list_1[3].count = 1;
      point_list_1[3].vertices.rgb_normal_edge = points_4;

      point_list_2[0].count = 8;
      point_list_2[0].vertices.rgb_normal_edge = points_5;
      point_list_2[1].count = 0;
      point_list_2[1].vertices.rgb_normal_edge = NULL;

      PEXSetSurfaceEdgeFlag(dpy, resourceID, req_type, PEXOn);
      PEXSetInteriorStyle(dpy, resourceID, req_type, PEXInteriorStyleSolid);
      PEXSetSurfaceInterpMethod(dpy, resourceID, req_type, 
				PEXSurfaceInterpColor);

      PEXSetReflectionModel( dpy, resourceID, req_type, PEXReflectionDiffuse);
      /* Turn the lights on. */
      lights_on[0] = 1;
      lights_on[1] = 2;
      lights_on[2] = 3;
      PEXSetLightSourceState( dpy, resourceID, req_type, 
			      3, lights_on, 0, ((PEXTableIndex*) NULL));

      p_edge_color.rgb.red = 1.0;
      p_edge_color.rgb.green = 0.0;
      p_edge_color.rgb.blue = 0.0;
      PEXSetSurfaceEdgeColor(dpy, resourceID, req_type, 
			     PEXColorTypeRGB, &p_edge_color);
      p_surface_color.rgb.red = 1.0;
      p_surface_color.rgb.green = 1.0;
      p_surface_color.rgb.blue = 1.0;
      PEXSetSurfaceColor(dpy, resourceID, req_type, 
			 PEXColorTypeRGB, &p_surface_color);

      PEXFillAreaSetWithData(dpy, resourceID, req_type, 
			     PEXShapeNonConvex, False, PEXContourDisjoint,
			     PEXGANone, 
			     (PEXGAColor | PEXGANormal | PEXGAEdges), 
			     PEXColorTypeRGB, 4, NULL, point_list_1);

      PEXFillAreaSetWithData(dpy, resourceID, req_type, 
			     PEXShapeNonConvex, False, PEXContourIntersecting,
			     PEXGANone, 
			     (PEXGAColor | PEXGANormal | PEXGAEdges), 
			     PEXColorTypeRGB, 2, NULL, point_list_2);
    }

void inquire_test_params(char *test_name, 
			 int  *num_images, 
			 int  *supported_modes, 
			 char *win_title)
    {
    strcpy(test_name, "pexocp.94.4");
    *num_images      = 1;
    *supported_modes = IMM_MODE | STR_MODE | WKS_MODE;

    strcpy(win_title, "pexocp.94.4");
    }

void misc_setup(Display *dpy, Window window, PEXRenderer renderer,
		int cur_image)
    {
    unsigned long         mask = PEXRAHLHSRMode;
    PEXRendererAttributes attrs;
    PEXLightEntry	  lights[2];

    attrs.hlhsr_mode = PEXHLHSRZBuffer;
    PEXChangeRenderer(dpy, renderer, mask, &attrs);

    /* Define the ambient light. */
    lights[0].type = PEXLightAmbient;
    lights[0].color.type = PEXColorTypeRGB;
    lights[0].color.value.rgb.red = .3;
    lights[0].color.value.rgb.green = .3;
    lights[0].color.value.rgb.blue = .3;

    /* Define the point light. */
    lights[1].type = PEXLightWCPoint;
    lights[1].point.x = 0.2;
    lights[1].point.y = 0.2;
    lights[1].point.z = 0.5;
    lights[1].attenuation1 = 1.0;
    lights[1].attenuation2 = 1.0;
    lights[1].color.type = PEXColorTypeRGB;
    lights[1].color.value.rgb.red = 1.0;
    lights[1].color.value.rgb.green = 1.0;
    lights[1].color.value.rgb.blue = 1.0;

    PEXSetTableEntries(dpy, lightLUT, 1, 2, PEXLUTLight, lights);
    }

void execute_test(Display *dpy, XID resourceID, 
	       PEXOCRequestType req_type, int cur_image)
    {
    describe_test(" \nPEXFillAreaSetWithData (fewer than 3 vertices)\n");
    describe_test(" The image from this test should be blank\n\n");

    draw_image(dpy, resourceID, req_type);
    }

void testproc(Display *dpy, Window window, int cur_image)
    {
     image_testproc("pexocp.94.4", "pexocp.94.4", img_desc,
		      dpy,window, 0, 0, 
		      glob_window_width, glob_window_height);
    }
