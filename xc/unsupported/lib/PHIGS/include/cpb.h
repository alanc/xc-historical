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

#ifndef PHG_CPB_H_INCLUDED
#define PHG_CPB_H_INCLUDED

/* Types for PHIGS/PEX CPB module. */

typedef struct { /* Data internal to the CPB module */
    Phg_scratch		scratch;
    Css_handle		cssh;	/* NULL in two-process client. */
} Cp_b;

/* CPB functions used only in the monitor. */
extern void		phg_cpb_pm_destroy();
extern Ws_handle	phg_cpb_pm_open_ws();
extern void		phg_cpb_pm_close_ws();

/* CPB functions used only in the client. */
extern void		phg_cpbc_destroy();
extern Ws_handle	phg_cpbc_open_ws();
extern void		phg_cpbc_close_ws();
extern int		phg_cpbc_struct_exists();
extern void		phg_cpbc_ar_archive();

/* CPB utility functions used in both the client and the monitor. */
extern void		phg_cpb_destroy();
extern void		phg_cpb_ws_update();
extern void		phg_cpb_ws_redraw_all();
extern void		phg_cpb_set_disp_state();
extern void		phg_cpb_inq_disp_update_state();
extern void		phg_cpb_inq_colr_map_meth_st();
Ws_handle		phg_cpb_open_ws();
extern void		phg_cpb_close_ws();
extern void		phg_cpb_post_struct();
extern void		phg_cpb_unpost_struct();
extern void		phg_cpb_unpost_all();
extern void		phg_cpb_set_rep();
extern void		phg_cpb_inq_rep();
extern void		phg_cpb_inq_view_rep();
extern void		phg_cpb_set_hlhsr_mode();
extern void		phg_cpb_inq_hlhsr_mode();
extern void		phg_cpb_set_ws_win();
extern void		phg_cpb_set_ws_vp();
extern void		phg_cpb_inq_ws_xform();
extern void		phg_cpb_set_filter();
extern void		phg_cpb_inq_filter();
extern void		phg_cpb_inq_posted();
extern void		phg_cpb_set_view_input_priority();
extern void		phg_cpb_inq_indices();
extern void		phg_cpb_message();
extern void		phg_cpb_inq_win_info();
extern void		phg_cpb_ws_drawable_pick();
extern void		phg_cpb_ws_map_points();
extern void		phg_cpb_ws_redraw_regions();
extern void		phg_cpb_ws_map_points();
extern void		phg_cpb_ws_synch();

extern void		phg_cpb_add_el();
extern void		phg_cpb_inq_el_type_size();
extern void		phg_cpb_inq_el_content();
extern void		phg_cpb_open_struct();
extern void		phg_cpb_close_struct();
extern void		phg_cpb_delete_all_structs();
extern void		phg_cpb_delete_struct();
extern void		phg_cpb_delete_struct_net();
extern void		phg_cpb_set_edit_mode();
extern void		phg_cpb_set_el_ptr();
extern void		phg_cpb_copy_all_els();
extern void		phg_cpb_delete_el();
extern void		phg_cpb_inq_el_ptr();
extern void		phg_cpb_inq_struct_status();
extern void		phg_cpb_inq_struct_ids();
extern void		phg_cpb_el_search();
extern void		phg_cpb_inq_wss_posted_to();
extern void		phg_cpb_inq_hierarchy();
extern void		phg_cpb_change_struct_id();
extern void		phg_cpb_change_struct_refs();
extern void		phg_cpb_change_struct_idrefs();
extern void		phg_cpb_inc_spa_search();
extern void		phg_cpb_inq_text_extent();
extern int		phg_cpb_full_copy_b_to_a();

#endif

