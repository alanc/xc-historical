/* $XConsortium: pexocp.95.3.c,v 1.0 93/11/22 12:36:45 rws Exp $ */

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
#include <misc.h>
#include <lut.h>

#define		NUM_TRIANGLES		51
#define     NUM_FACETS          NUM_TRIANGLES
#define		NUM_VERTICES		(NUM_TRIANGLES+2)
#define		X_START				0.01
#define		X_END				0.99
#define		TWIST_Y0			0.9
#define		FLAT_Y0				(TWIST_Y0-((STRIP_HALF_WIDTH*2+GAP)*3))
#define		Z0					0.5
#define		STRIP_HALF_WIDTH	0.06
#define		TWIST_ANGLE			(M_PI*1)
#define		GAP					0.015
#define		DOWN				((STRIP_HALF_WIDTH*2)+GAP)

/*
 *	draw_image prototype
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
	x_axis[1].rgb.blue		= 1.0;

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
	y_axis[1].rgb.blue		= 1.0;
    
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
	z_axis[1].rgb.blue		= 1.0;

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

/* 
 * PEXlib Triangle Strip: Mainline test for all permutations of
 *		data formats.
 */
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
	int						i;
	float					dx, delta, angle, norm_angle;
	PEXColor				color;
	PEXTableIndex			lights_on[4];
	PEXCoord				origin;

	PEXColorRGB				facet_rgb[NUM_FACETS];
	PEXVector				facet_normal[NUM_FACETS];
	PEXColorRGBNormal		facet_rgb_normal[NUM_FACETS];

	PEXCoord				twist_pts[NUM_VERTICES];
	PEXCoord				flat_pts[NUM_VERTICES];

	PEXVertexRGB			pts_rgb[NUM_VERTICES];
	PEXVertexRGBNormal		pts_rgb_normal[NUM_VERTICES];

	PEXArrayOfVertex		vertices;

	unsigned int			facet_attributes, vertex_attributes, count;
	int						color_type;

	PEXArrayOfFacetData		facet_data;

	PEXReflectionAttributes	refl_attributes;

	/*
	 *	Generate the points for the band.
	 */
	dx		= (X_END - X_START)/(NUM_VERTICES - 1);
	delta	= TWIST_ANGLE / (NUM_VERTICES - 1);

	for( i = 0; i < NUM_VERTICES; i += 1 )
	{
		angle			= i * delta;
		twist_pts[i].x	= X_START + i * dx;
		flat_pts[i].x	= twist_pts[i].x;
		flat_pts[i].z	= Z0;

		/*
		 *	Alternate between odd and even for vertices
		 */
		if( !(i & 0x1) ) /* odd or even? */
		{
			twist_pts[i].y	= TWIST_Y0 + STRIP_HALF_WIDTH * cos(angle);
			flat_pts[i].y	= FLAT_Y0  + STRIP_HALF_WIDTH;
			twist_pts[i].z	= Z0       + STRIP_HALF_WIDTH * sin(angle);
		}
		else
		{
			twist_pts[i].y	= TWIST_Y0 - STRIP_HALF_WIDTH * cos(angle);
			flat_pts[i].y	= FLAT_Y0  - STRIP_HALF_WIDTH;
			twist_pts[i].z	= Z0       - STRIP_HALF_WIDTH * sin(angle);
		}
	}

	set_view( dpy, view_lut );

	/*
	 *	Set the view index.
	 */
/*
	PEXSetViewIndex( dpy, res_id, req_type, 1 );
*/

	/*
	 *	Build the axes.
	 */
	origin.x = origin.y = origin.z = 0;
	build_axes( dpy, res_id, req_type, &origin, 1.0 );


	/*
	 *	Turn on the lights
	 */
	lights_on[0] = 1;
	lights_on[1] = 2;
	lights_on[2] = 3;
	PEXSetLightSourceState( dpy, res_id, req_type, 3, lights_on, 0,
		((PEXTableIndex *) NULL ));

	/*
	 *	Set the reflection model
	 */
	PEXSetReflectionModel( dpy, res_id, req_type, PEXReflectionSpecular );

	/*
	 *	Set the reflection attributes
	 */
	refl_attributes.ambient							= 1.0;
	refl_attributes.diffuse							= 1.0;
	refl_attributes.specular						= 1.0;
	refl_attributes.specular_conc					= 5.0;
	refl_attributes.transmission					= 0.0;
	refl_attributes.specular_color.type				= PEXColorTypeRGB;
	refl_attributes.specular_color.value.rgb.red	= 0.0;
	refl_attributes.specular_color.value.rgb.green	= 0.0;
	refl_attributes.specular_color.value.rgb.blue	= 1.0;
	PEXSetReflectionAttributes( dpy, res_id, req_type, &refl_attributes );

	/*
	 *	No front/back facet distinguish
	 */
	PEXSetFacetDistinguishFlag( dpy, res_id, req_type, PEXOff );

	/*
	 *	No facet culling
	 */
	PEXSetFacetCullingMode( dpy, res_id, req_type, PEXNone );

	/*
	 *	Hollow interiors for front/back facets
	 */
	PEXSetInteriorStyle( dpy, res_id, req_type, PEXInteriorStyleHollow );
	PEXSetBFInteriorStyle( dpy, res_id, req_type, PEXInteriorStyleHollow );

	/*
	 *	White/Yellow interior color for front/back facets
	 */
	color.rgb.red	= 1.0;
	color.rgb.green	= 1.0;
	color.rgb.blue	= 1.0;
	PEXSetSurfaceColor( dpy, res_id, req_type, PEXColorTypeRGB, &color );
	color.rgb.red	= 1.0;
	color.rgb.green	= 1.0;
	color.rgb.blue	= 0.0;
	PEXSetBFSurfaceColor( dpy, res_id, req_type, PEXColorTypeRGB, &color );

	/*
	 *	Edges off but set to be magenta colored dashed lines
	 */
	color.rgb.green = 0;
	color.rgb.red = color.rgb.blue = 1.0;
	PEXSetSurfaceEdgeFlag( dpy, res_id, req_type, PEXOff );
	PEXSetSurfaceEdgeType( dpy, res_id, req_type, PEXSurfaceEdgeDashed );
	PEXSetSurfaceEdgeColor( dpy, res_id, req_type, PEXColorTypeRGB, &color );

	/*
	 *	Goraud interior shading
	 */
	PEXSetSurfaceInterpMethod( dpy, res_id, req_type, PEXSurfaceInterpColor );

	/*
	 *	Render the twisted triangle strip with NO optional data.
	 *	Since no optional data is provided, the currently installed
	 *	attributes will be used.  The image should be a white twisted
	 *	triangle strip with hollow interiors and lighting/shading
	 *	effects.
	 *
	 *		FACET DATA:
	 *			color   - None
	 *			normals - None
	 *
	 *		VERTEX DATA:
	 *			color   - None
	 *			normals - None
	 *
	 */
	facet_attributes	= PEXGANone;
	vertex_attributes	= PEXGANone;
	color_type			= PEXColorTypeRGB;
	count				= NUM_VERTICES;
	vertices.no_data	= twist_pts;
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );


	/*
	 *	Render the flat triangle strip with NO optional data.
	 *	Again, the current pipeline context will be used for
	 *	all attributes, so the image should be just a white flat
	 *	triangle strip with no lighting/shading effects.
	 *	
	 *
	 *		FACET DATA:
	 *			color   - None
	 *			normals - None
	 *
	 *		VERTEX DATA:
	 *			color   - None
	 *			normals - None
	 *
	 */
	vertices.no_data	= flat_pts;
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );

	/*
	 *	Bump the y's down a bit
	 */
	for( i = 0; i < NUM_VERTICES; i++ )
	{
		twist_pts[i].y		-= DOWN;
		pts_rgb[i].point	= twist_pts[i];
	}
	vertices.rgb	= pts_rgb;

	/*
	 *	Add pure red and green color per vertex
	 *	as well as magenta and yellow color per facet.
	 */
	for( i = 0; i < NUM_VERTICES; i += 2 )
	{
		pts_rgb[i].rgb.red		= 1.0;
		pts_rgb[i].rgb.green	= 0.0;
		pts_rgb[i].rgb.blue		= 0.0;

		facet_rgb[i].red		= 1.0;
		facet_rgb[i].green		= 0.0;
		facet_rgb[i].blue		= 1.0;
	}
	for( i = 1; i < NUM_VERTICES; i += 2 )
	{
		pts_rgb[i].rgb.red		= 0.0;
		pts_rgb[i].rgb.green	= 1.0;
		pts_rgb[i].rgb.blue		= 0.0;

		facet_rgb[i].red		= 1.0;
		facet_rgb[i].green		= 1.0;
		facet_rgb[i].blue		= 0.0;
	}
	facet_data.rgb	= facet_rgb;

	/*
	 *	Render the triangle strip with optional color
	 *	data per vertex and facet.  The interior is hollow
	 *	and if both facet and vertex color data is included,
	 *	the vertex data should override the facet data.  For
	 *	this case, the lines between the top and bottom
	 *	vertices should be interpolated and non-alternating.
	 *	The lighting/shading effects should be the same as
	 *	for the first white twisted strip.
	 *
	 *		FACET DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - None
	 *
	 *		VERTEX DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - None
	 *
	 */
	facet_attributes	= PEXGAColor;
	vertex_attributes	= PEXGAColor;
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );

	/*
	 *	Bump the y's down
	 */
	for( i = 0; i < NUM_VERTICES; i++ )
	{
		twist_pts[i].y		-= DOWN;
		pts_rgb[i].point	= twist_pts[i];
	}
	vertices.rgb	= pts_rgb;

	/*
	 *	Solid interiors for front/back facets
	 */
	PEXSetInteriorStyle( dpy, res_id, req_type, PEXInteriorStyleSolid );
	PEXSetBFInteriorStyle( dpy, res_id, req_type, PEXInteriorStyleSolid );

	/*
	 *	Render the triangle strip with optional color
	 *	data per vertex and facet.  The interior is now solid
	 *	and if both facet and vertex color data is included,
	 *	the vertex data should override the facet data.  For
	 *	this case, the color between the top and bottom
	 *	vertices should be interpolated and non-alternating.
	 *	The lighting/shading effects should be the same as
	 *	for the first white twisted strip.
	 *
	 *		FACET DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - None
	 *
	 *		VERTEX DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - None
	 */
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );

	/*
	 *	Bump the y's down
	 */
	for( i = 0; i < NUM_VERTICES; i++ )
	{
		flat_pts[i].y		-= DOWN;
		pts_rgb[i].point	= flat_pts[i];
	}

	/*
	 *	Render the flat triangle strip with optional color
	 *	data per vertex and facet.  The interior is solid
	 *	and if both facet and vertex color data is included,
	 *	the vertex data should override the facet data.  For
	 *	this case, the color between the top and bottom
	 *	vertices should be interpolated and non-alternating.
	 *	The lighting/shading effects should be the same as
	 *	for the first white twisted strip.
	 *	
	 *		FACET DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - None
	 *
	 *		VERTEX DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - None
	 */
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );

	/*
	 *	Bump the y's down a bit
	 */
	for( i = 0; i < NUM_VERTICES; i++ )
	{
		flat_pts[i].y		-= DOWN;
		pts_rgb[i].point	= flat_pts[i];
	}
	vertices.rgb	= pts_rgb;

	/*
	 *	Now include normals per facet along with
	 *	color per facet.
	 */
	for( i = 0; i < NUM_FACETS; i += 2 )
	{
		facet_rgb_normal[i].rgb.red		= 1.0;
		facet_rgb_normal[i].rgb.green	= 0.0;
		facet_rgb_normal[i].rgb.blue	= 1.0;
	}
	for( i = 1; i < NUM_FACETS; i += 2 )
	{
		facet_rgb_normal[i].rgb.red		= 1.0;
		facet_rgb_normal[i].rgb.green	= 1.0;
		facet_rgb_normal[i].rgb.blue	= 0.0;
	}
	norm_angle	= M_PI*2/NUM_FACETS;
	for( i = 0; i < NUM_FACETS; i++ )
	{
		facet_rgb_normal[i].normal.x = -(cos( norm_angle * i ));
		facet_rgb_normal[i].normal.y = 0.0;
		facet_rgb_normal[i].normal.z = (sin( norm_angle * i ));
		if( facet_rgb_normal[i].normal.z < 0 )
		{
			/*
			 *	Reverse the sign of the x component if the z
			 *	component is "back facing"
			 */
			facet_rgb_normal[i].normal.x = -facet_rgb_normal[i].normal.x;
		}
	}
	facet_data.rgb_normal	= facet_rgb_normal;

	/*
	 *	Render the flat triangle strip with optional color
	 *	data per vertex and per facet as well as normals per
	 *	facet.  The interior is hollow and if both facet and
	 *	vertex color data is included, the vertex data should
	 *	override the facet data.  For this case, the lines
	 *	between the top and bottom vertices should be
	 *	interpolated and non-alternating.  The lighting/shading
	 *	effects should be the same as for the first white
	 *	twisted strip.
	 *
	 *		FACET DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - Yes
	 *
	 *		VERTEX DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - No
	 *
	 */
	facet_attributes	= PEXGAColor | PEXGANormal;
	vertex_attributes	= PEXGAColor;
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );

	/*
	 *	Bump the y's down a bit
	 */
	for( i = 0; i < NUM_VERTICES; i++ )
	{
		flat_pts[i].y				-= DOWN;
		pts_rgb_normal[i].point		= flat_pts[i];
		pts_rgb_normal[i].rgb		= pts_rgb[i].rgb;
	}
	
	/*
	 *	Add the normal per vertex data.
	 */
	norm_angle  = M_PI*2/NUM_VERTICES;
	for( i = 0; i < NUM_VERTICES; i++ )
	{
		pts_rgb_normal[i].normal.x = -(cos( norm_angle * i ));
		pts_rgb_normal[i].normal.y = 0.0;
		pts_rgb_normal[i].normal.z = (sin( norm_angle * i ));
		if( pts_rgb_normal[i].normal.z < 0 )
		{
			/*
			 *	Reverse the sign of the x component if the z
			 *	component is back facing.
			 */
			pts_rgb_normal[i].normal.x = -pts_rgb_normal[i].normal.x;
		}
	}
	vertices.rgb_normal	= pts_rgb_normal;

	/*
	 *	Render the flat triangle strip with optional color
	 *	data per vertex and per facet as well as normals per
	 *	vertex and per facet.  The interior is solid and if
	 *	both facet and vertex optional data is included,
	 *	the vertex data should 	override the facet data.
	 *	This triangle strip, (the last one) should have the
	 *	"S" shaded appearance as the one just above it, but
	 *	the individual shading of each facet should not be
	 *	as dominant as in the case with only facet normals.
	 *
	 *		FACET DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - Yes
	 *
	 *		VERTEX DATA:
	 *			color   - Yes, direct RGBs
	 *			normals - Yes
	 *
	 */
	vertex_attributes	= PEXGAColor | PEXGANormal;
	PEXTriangleStrip(
		dpy, res_id, req_type,
		facet_attributes,
		vertex_attributes,
		color_type,
		facet_data,
		count,
		vertices );

} /* draw_image */

void inquire_test_params(char *test_name, 
			 int  *num_images, 
			 int  *supported_modes, 
			 char *win_title)
{
    strcpy(test_name, "pexocp.95.3");
    *num_images      = 1;
    *supported_modes = IMM_MODE | STR_MODE | WKS_MODE;

    strcpy(win_title, "pexocp.95.3");
}

void misc_setup(Display *dpy, Window window, PEXRenderer renderer,
		int cur_image)
{
    unsigned long         mask = PEXRAHLHSRMode;
    PEXRendererAttributes attrs;
    PEXLightEntry			lights[3];

    attrs.hlhsr_mode = PEXHLHSRZBuffer;

    PEXChangeRenderer(dpy, renderer, mask, &attrs);
    set_view( dpy, viewLUT );
	/*
	 *	Define the Ambient light (weak)
	 */
	lights[0].type					= PEXLightAmbient;
	lights[0].color.type			= PEXColorTypeRGB;
	lights[0].color.value.rgb.red	= 0.5;
	lights[0].color.value.rgb.green	= 0.5;
	lights[0].color.value.rgb.blue	= 0.5;

	/*
	 *	Define the Directional light
	 */
	lights[1].type					= PEXLightWCVector;
	lights[1].direction.x			=  0.0;
	lights[1].direction.y			=  0.0;
	lights[1].direction.z			= -1.0;
	lights[1].color.type			= PEXColorTypeRGB;
	lights[1].color.value.rgb.red	= 1.0;
	lights[1].color.value.rgb.green	= 1.0;
	lights[1].color.value.rgb.blue	= 1.0;

	/*
	 *	Define the Point light
	 */
	lights[2].type					= PEXLightWCPoint;
	lights[2].point.x				= 0.5;
	lights[2].point.y				= 0.5;
	lights[2].point.z				= 1.0;
	lights[2].attenuation1			= 1.0;
	lights[2].attenuation2			= 1.0;
	lights[2].concentration			= 1.0;
	lights[2].spread_angle			= 0.01;
	lights[2].color.type			= PEXColorTypeRGB;
	lights[2].color.value.rgb.red	= 1.0;
	lights[2].color.value.rgb.green	= 1.0;
	lights[2].color.value.rgb.blue	= 1.0;


	PEXSetTableEntries( dpy, lightLUT, 1, 3, PEXLUTLight, lights );
}

void execute_test(Display *dpy, XID resourceID, 
	       PEXOCRequestType req_type, int cur_image)
{
    describe_test( "\nPEXTriangleStrip Mainline Test:\n" );
    describe_test( "\n" );
    describe_test( "You should see 7 Triangle Strips, the top three twisted\n" );
    describe_test( "and the bottom four flat.  Three lights are on, one ambient\n" );
    describe_test( "white light, one white point light, and one white directional\n" );
    describe_test( "light.  The specular reflection color is blue.  The Triangle\n" );
    describe_test( "Strips appearances should be as follows:\n" );
    describe_test( "\n" );
    describe_test( "#1 - White with hollow interior and the left side brighter\n" );
    describe_test( "     than the right side.  (No optional data here)\n" );
    describe_test( "\n" );
    describe_test( "#2 - Red/Green with hollow interior and the left side brighter\n" );
    describe_test( "     than the right side.  Some of the blue specular reflection\n" );
    describe_test( "     should be apparent as magenta/cyan on the leftmost part\n" );
    describe_test( "     of the strip.  (Color per vertex and color per facet is included\n" );
    describe_test( "     as optional data, with the vertex color overriding the facet color)\n" );
    describe_test( "\n" );
    describe_test( "#3 - Same as above but with solid interior.  The specular coloring\n" );
    describe_test( "     should be more evident.\n" );
    describe_test( "\n" );
    describe_test( "#4 - Same as #1 but with no twist, the whole strip is front facing.\n" );
    describe_test( "     (No optional data here either)\n" );
    describe_test( "\n" );
    describe_test( "#5 - Same as #4 but with red/green color per vertex with the specular\n" );
    describe_test( "     color (blue) blending with the red/green color per vertex.\n" );
    describe_test( "     (Color per vertex and color per facet is included as optional data)\n" );
    describe_test( "\n" );
    describe_test( "#6 - Same as #4 but with normals per facet to make it look like\n" );
    describe_test( "     a sideways S to the lighting calculations, which  \n" );
    describe_test( "     creates the duller colored part in the middle.\n" );
    describe_test( "     (Normals per facet are included as optional data)\n" );
    describe_test( "\n" );
    describe_test( "#7 - Same as above but with both normals and color per vertex and\n" );
    describe_test( "     per facet included. Note the specular reflections\n" );
    
    draw_image( dpy, resourceID, req_type, lightLUT, viewLUT );
}
        

void testproc(Display *dpy, Window window, int cur_image)
{
     image_testproc("pexocp.95.3", "pexocp.95.3", img_desc,
		      dpy,window, 0, 0, 
		      glob_window_width, glob_window_height);
}
