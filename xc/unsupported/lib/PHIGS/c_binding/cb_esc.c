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

/* Escape functions for the PHIGS C binding */

#include "phg.h"
#include "cp.h"
#include "cb_priv.h"

static void
esc_u1( cph, in )
    Cp_handle		cph;
    Pescape_in_data	*in;
{
    Phg_args		cp_args;

    /* Error synchronization */
    if ( CB_ENTRY_CHECK( cph, ERR2, Pfn_escape)) {
	cp_args.data.idata = (int)in->escape_in_u1.sync_on;
	CP_FUNC( cph, CP_FUNC_OP_ERROR_SYNC, &cp_args, (Phg_ret *)NULL );
    }
}

static void
esc_u2( cph, in, store, out )
    Cp_handle		cph;
    Pescape_in_data	*in;
    Pstore		store;
    Pescape_out_data	*out;
{
    Phg_args		args;
    Phg_ret		ret;
    int			size;
    
    if (phg_cb_ws_open(cph, in->escape_in_u2.ws_id, Pfn_escape)) {
	args.data.idata = in->escape_in_u2.ws_id;
	CP_FUNC( cph, CP_FUNC_OP_XX_INQ_DPY_AND_DRAWABLE, &args, &ret );
	if ( !(out->escape_out_u2.err_ind = ret.err) ) {
	    out->escape_out_u2.display = ret.data.dpy_and_drawable.display;
	    out->escape_out_u2.drawable_id =
		ret.data.dpy_and_drawable.drawable_id;
	    out->escape_out_u2.input_overlay_id =
		ret.data.dpy_and_drawable.overlay_id;
	    size = strlen( ret.data.dpy_and_drawable.display_name ) + 1;
	    if ( CB_STORE_SPACE( store, size,
		    &out->escape_out_u2.err_ind ) ) {
		out->escape_out_u2.display_name = store->buf;
		strcpy( out->escape_out_u2.display_name,
		    ret.data.dpy_and_drawable.display_name );
	    }
	}
    }
    
}

static void
esc_u3( cph, in )
    Cp_handle		cph;
    Pescape_in_data	*in;
{
    Phg_args		cp_args;

    /* Disable/Enable detection of DC errors. */
    if ( CB_ENTRY_CHECK( cph, ERR2, Pfn_escape)) {
	cph->flags.ignore_DC_errors = in->escape_in_u3.ignore_DC_errors ? 1 : 0;
    }
}

static void
esc_u4( cph, in, out )
    Cp_handle		cph;
    Pescape_in_data	*in;
    Pescape_out_data	*out;
{
    Phg_args		cp_args;
    Phg_ret		ret;
    Psl_ws_info		*wsinfo;
    Wst_phigs_dt	*dt;

    register Phg_args_drawable_pick	*args = &cp_args.data.drawable_pick;

    out->escape_out_u4.status = PIN_STATUS_NO_IN;
    out->escape_out_u4.pick.depth = 0;

    if ( !(wsinfo = phg_cb_ws_open(cph, in->escape_in_u2.ws_id, Pfn_escape)) )
	return;

    dt = &wsinfo->wstype->desc_tbl.phigs_dt;
    if ( !phg_cb_echo_limits_valid( cph, Pfn_escape, wsinfo->wsid,
	    &in->escape_in_u4.echo_volume, dt ) ) {
	/* Error has been reported, just fall out of the if. */
	return;
    }

    args->wsid = in->escape_in_u4.ws_id;
    args->point = in->escape_in_u4.point;
    args->ap_size = in->escape_in_u4.ap_size;
    args->order = in->escape_in_u4.order;
    args->pet = in->escape_in_u4.pet;
    args->esw = in->escape_in_u4.echo_switch;
    args->echo_volume = in->escape_in_u4.echo_volume;
    args->filter = in->escape_in_u4.filter;
    CP_FUNC( cph, CP_FUNC_OP_DRAWABLE_PICK, &cp_args, &ret );

    if ( ret.err ) {
	/* Report errors immediately so user doesn't try to read garbage.  */
	ERR_FLUSH( phg_cur_cph->erh);

    } else {
	out->escape_out_u4.status = ret.data.drawable_pick.status;
	if ( out->escape_out_u4.status == PIN_STATUS_OK ) {
	    out->escape_out_u4.pick.depth =
		MIN(in->escape_in_u4.depth, ret.data.drawable_pick.pick.depth);
	    /* Assumes the output array is allocated by the application. */
	    bcopy( ret.data.drawable_pick.pick.path_list,
		out->escape_out_u4.pick.path_list,
		out->escape_out_u4.pick.depth * sizeof(Ppick_path_elem) );
	}
    }
}

static void
esc_u5( cph, in, out )
    Cp_handle		cph;
    Pescape_in_data	*in;
    Pescape_out_data	*out;
{
    Phg_args		cp_args;
    Phg_ret		ret;

    register Phg_args_map_points	*args = &cp_args.data.map_points;

    if ( !phg_cb_ws_open(cph, in->escape_in_u5.ws_id, Pfn_escape) )
	return;

    args->wsid = in->escape_in_u5.ws_id;
    args->points = in->escape_in_u5.points;
    CP_FUNC( cph, CP_FUNC_OP_MAP_POINTS, &cp_args, &ret );

    if ( ret.err ) {
	/* Report errors immediately so user doesn't try to read garbage.  */
	ERR_FLUSH( phg_cur_cph->erh);
	out->escape_out_u5.points.num_points = 0;
    } else {
	out->escape_out_u5.view_index = ret.data.map_points.view_index;
	out->escape_out_u5.points.num_points =
	    ret.data.map_points.points.num_points;
	if ( out->escape_out_u5.points.num_points > 0 ) {
	    /* Assumes the output array is allocated by the application. */
	    bcopy( ret.data.map_points.points.points,
		out->escape_out_u5.points.points,
		ret.data.map_points.points.num_points * sizeof(Ppoint3) );
	}
    }
}

static void
esc_u6( cph, in )
    Cp_handle		cph;
    Pescape_in_data	*in;
{
    Phg_args		cp_args;

    register Phg_args_redraw_regions	*args = &cp_args.data.redraw_regions;

    if ( !phg_cb_ws_open(cph, in->escape_in_u6.ws_id, Pfn_escape) )
	return;

    args->wsid = in->escape_in_u6.ws_id;
    args->num_regions = in->escape_in_u6.num_regions;
    args->regions = in->escape_in_u6.regions;
    CP_FUNC( cph, CP_FUNC_OP_REDRAW_REGIONS, &cp_args, (Phg_ret *)NULL );
}

static void
esc_u7( cph, in )
    Cp_handle		cph;
    Pescape_in_data	*in;
{
    Phg_args		cp_args;

    if ( !phg_cb_ws_open(cph, in->escape_in_u7.ws_id, Pfn_escape) )
	return;

    cp_args.data.idata = in->escape_in_u7.ws_id;
    CP_FUNC( cph, CP_FUNC_OP_WS_SYNCH, &cp_args, (Phg_ret *)NULL );
}

void
pescape( func_id, in, store, out )
    Pint        	func_id;	/* escape function identifier	*/
    Pescape_in_data	*in;		/* input data for the function	*/
    Pstore		store;
    Pescape_out_data	*out;		/* OUT output data of the function */
{
    switch ( func_id) {
	case PUESC_ERRSYNC:
	    esc_u1( phg_cur_cph, in);
	    break;

	case PUESC_DPYINFO:
	    esc_u2( phg_cur_cph, in, store, out );
	    break;

	case PUESC_IGNORE_DC_ERRORS:
	    esc_u3( phg_cur_cph, in );
	    break;

	case PUESC_DRAWABLE_POINT_TO_PICK:
	    esc_u4( phg_cur_cph, in, out );
	    break;

	case PUESC_DRAWABLE_POINTS_TO_WC:
	    esc_u5( phg_cur_cph, in, out );
	    break;

	case PUESC_REDRAW_REGIONS:
	    esc_u6( phg_cur_cph, in );
	    break;

	case PUESC_WS_SYNCH:
	    esc_u7( phg_cur_cph, in );
	    break;

	default:
	    ERR_REPORT( phg_cur_cph->erh, ERR350);
	    break;
    }
}
