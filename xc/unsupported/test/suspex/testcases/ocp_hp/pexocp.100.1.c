/* $XConsortium: pexocp.100.1.c,v 1.0 93/11/22 12:32:39 rws Exp $ */

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
#include <math.h>
#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif
#include <misc.h>
#include <lut.h>

#define NUM_ROWS			9
#define NUM_COLS			15
#define	NUM_CELL_ARRAYS		1

/*
 *	prototypes
 */
void draw_image(
		Display				*dpy,
		XID					 rid,
		PEXOCRequestType	 req_type,
		PEXLookupTable		 light_lut,
		PEXLookupTable		 view_lut );

void set_view(
	Display			*dpy,
	PEXLookupTable	view_table );

void build_axes(
	Display				*dpy,
	XID					 res_id,
	PEXOCRequestType	 req_type,
	PEXCoord			*origin,
	double				 length );


/* 
 * PEXlib Cell Array 2D : Mainline test 
 */

void build_axes(
	Display				*dpy,
	XID					 res_id,
	PEXOCRequestType	 req_type,
	PEXCoord			*origin,
	double				 length )
{
    PEXVertexRGB	x_axis[2], y_axis[2], z_axis[2];
    PEXListOfVertex	axes[3];

    /*
	 *	Set the vertex coordinates and colors.
	 */
    axes[0].count			= 2;
    axes[0].vertices.rgb	= x_axis;

    x_axis[0].point			= *origin;

    x_axis[1].point.x		= origin->x + length;
    x_axis[1].point.y		= origin->y;
    x_axis[1].point.z		= origin->z;

	x_axis[0].rgb.red		= 1.0;
	x_axis[0].rgb.green		= 1.0;
	x_axis[0].rgb.blue		= 1.0;

	x_axis[1].rgb.red		= 1.0;
	x_axis[1].rgb.green		= 1.0;
	x_axis[1].rgb.blue		= 0.0;

    
    axes[1].count			= 2;
    axes[1].vertices.rgb	= y_axis;
    y_axis[0].point			= *origin;
    y_axis[1].point.x		= origin->x;
    y_axis[1].point.y		= origin->y + length;
    y_axis[1].point.z		= origin->z;

	y_axis[0].rgb.red		= 1.0;
	y_axis[0].rgb.green		= 1.0;
	y_axis[0].rgb.blue		= 1.0;

	y_axis[1].rgb.red		= 1.0;
	y_axis[1].rgb.green		= 1.0;
	y_axis[1].rgb.blue		= 0.0;
    
    axes[2].count			= 2;
    axes[2].vertices.rgb	= z_axis;
    z_axis[0].point			= *origin;
    z_axis[1].point.x		= origin->x;
    z_axis[1].point.y		= origin->y;
    z_axis[1].point.z		= origin->z + length;

	z_axis[0].rgb.red		= 1.0;
	z_axis[0].rgb.green		= 1.0;
	z_axis[0].rgb.blue		= 1.0;

	z_axis[1].rgb.red		= 1.0;
	z_axis[1].rgb.green		= 1.0;
	z_axis[1].rgb.blue		= 0.0;

	PEXSetPolylineInterpMethod( dpy, res_id, req_type, PEXPolylineInterpColor );
    PEXPolylineSetWithData( dpy, res_id, req_type, PEXGAColor,
			    PEXColorTypeRGB, 3, axes );
}


void set_view(
	Display			*dpy,
	PEXLookupTable	view_table )
{
    PEXViewEntry	view;
    PEXCoord2D		window[2];
    double			view_plane, front_plane, back_plane;
    PEXCoord		prp;
    PEXNPCSubVolume	viewport;

    /*
	 *	The view orientation parameters.
	 */
    static PEXCoord		view_ref_pt			= {  0.5, 0.5, 0.5 };
    static PEXVector	view_plane_normal	= {  0.0, 0.0,   1 };
    static PEXVector	view_up_vec			= {    0,   1,   0 };

    /*
	 *	Compute the view orientation transform.
	 */
    PEXViewOrientationMatrix( &view_ref_pt, &view_plane_normal,
				    &view_up_vec, view.orientation );
    
    /*
	 *	The view mapping parameters.
	 */
    prp.x = 0.0;
	prp.y = 0.0;
	prp.z =   5;

    window[0].x = -0.8; window[1].x = 0.8;
    window[0].y = -0.9; window[1].y = 0.7;

    front_plane = 3; view_plane = 0; back_plane = -1;

    viewport.min.x = 0; viewport.max.x = 1;
    viewport.min.y = 0; viewport.max.y = 1;
    viewport.min.z = 0; viewport.max.z = 1;
    
    /*
	 *	Compute the view mapping transform.
	 */
    PEXViewMappingMatrix( window, &viewport, True, &prp,
			        view_plane, back_plane, front_plane,
			        view.mapping );
    
    /*
	 *	The view clipping parameters.
	 */
    view.clip_flags		= PEXClipXY;
    view.clip_limits	= viewport;
    
    /*
	 *	Set view 1.
	 */
    PEXSetTableEntries( dpy, view_table, 1, 1, PEXLUTView,
		       (PEXPointer) &view );
}


void draw_image(
		Display				*dpy,
		XID					 res_id,
		PEXOCRequestType	 req_type,
		PEXLookupTable		 light_lut,
		PEXLookupTable		 view_lut )
{	
	/*
	 *	Data declarations
	 */
	unsigned int	indx;
	PEXCoord		origin;
	PEXColorRGB		magenta,
					yellow;
	float			x[NUM_CELL_ARRAYS][4],
					y[NUM_CELL_ARRAYS][4];
	PEXCoord2D		pts1,
					pts2,
					pts3,
					pts4;
	unsigned int	col_count,
					row_count;
	PEXTableIndex	color_indices[NUM_ROWS*NUM_COLS];


	/*
	 *	Set the view index.  Commented out to mitigate pixelitis
	 */
/*
    PEXSetViewIndex( dpy, res_id, req_type, 1 );
*/

	/*
	 *	Build the axes. Again, commented out to simplify the
	 *	archived image.
	 */
	origin.x = origin.y = origin.z = 0;
/*
	build_axes( dpy, res_id, req_type, &origin, 1.0 );
*/

	/*
	 *	Set up the columns and row counts as well as the
	 *	color index array (the array will be ignored)
	 */
	col_count = NUM_COLS;
	row_count = NUM_ROWS;
	for( indx = 0; indx < NUM_ROWS * NUM_COLS; indx++ )
		color_indices[indx] = indx % 8;

	/*
	 *	In the x-y plane.
	 */
	x[0][0] = 0.1;	y[0][0] = 0.1;
	x[0][1] = 0.9;	y[0][1] = 0.1;
	x[0][2] = 0.9;	y[0][2] = 0.9;
	x[0][3] = 0.1;	y[0][3] = 0.9;

	yellow.red		= 1.0;
	yellow.green	= 1.0;
	yellow.blue		= 0.0;

	magenta.red		= 1.0;
	magenta.green	= 0.0;
	magenta.blue	= 1.0;

	PEXSetMarkerScale( dpy, res_id, req_type, 3.0 );

	for( indx = 0; indx < NUM_CELL_ARRAYS; indx++ )
	{
		/*
		 *	Set the points for the Cell Array.
		 */
		pts1.x = x[indx][0]; pts1.y = y[indx][0];
		pts2.x = x[indx][1]; pts2.y = y[indx][1];
		pts3.x = x[indx][2]; pts3.y = y[indx][2];
		pts4.x = x[indx][3]; pts4.y = y[indx][3];

		/*
		 *	Use yellow Polymarkers to show the two cell
		 *	array points.
		 */
		PEXSetMarkerColor( dpy, res_id, req_type, PEXColorTypeRGB, 
				   (PEXColor *) &yellow );
		PEXMarkers2D( dpy, res_id, req_type, 1, &pts1 );
		PEXMarkers2D( dpy, res_id, req_type, 1, &pts3 );

		/*
		 *	Use a magenta marker to show the computed points.
		 */
		PEXSetMarkerColor( dpy, res_id, req_type, PEXColorTypeRGB, 
				 (PEXColor *) &magenta );
		PEXMarkers2D( dpy, res_id, req_type, 1, &pts2 );
		PEXMarkers2D( dpy, res_id, req_type, 1, &pts4 );

		/*
		 *	Render the cell array in the x-y plane
		 */
		PEXCellArray2D(
			dpy, res_id, req_type,
			&pts1,
			&pts3,
			col_count,
			row_count,
			color_indices );
	}

} /* draw_image */


void inquire_test_params(char *test_name, 
			 int  *num_images, 
			 int  *supported_modes, 
			 char *win_title)
    {
    strcpy(test_name, "pexocp.100.1");
    *num_images      = 1;
    *supported_modes = IMM_MODE | STR_MODE | WKS_MODE;

    strcpy(win_title, "pexocp.100.1");
    }

void misc_setup(Display *dpy, Window window, PEXRenderer renderer,
		int cur_image)
    {
    unsigned long         mask = PEXRAHLHSRMode;
    PEXRendererAttributes attrs;

    attrs.hlhsr_mode = PEXHLHSRZBuffer;

    PEXChangeRenderer(dpy, renderer, mask, &attrs);
    set_view( dpy, viewLUT );
    }

void execute_test(Display *dpy, XID resourceID, 
	       PEXOCRequestType req_type, int cur_image)
    {
    describe_test("\nPEXCellArray2D Mainline Test\n\n");
    describe_test("A Cell array is drawn with markers at the corners.\n");
    describe_test("\n");

    draw_image( dpy, resourceID, req_type, lightLUT, viewLUT );
    }

void testproc(Display *dpy, Window window, int cur_image)
    {
     image_testproc("pexocp.100.1", "pexocp.100.1", img_desc,
		      dpy,window, 0, 0, 
		      glob_window_width, glob_window_height);
    }
