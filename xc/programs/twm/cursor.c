/***********************************************************************
 *
 * $XConsortium: cursor.c,v 1.2 89/04/07 11:57:30 toml Exp $
 *
 * cursor creation code
 *
 * 05-Apr-89 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#include <stdio.h>
#include "twm.h"
#include "screen.h"
#include "util.h"

static struct _CursorName {
    char		*name;
    unsigned int	shape;
    Cursor		cursor;
} cursor_names[] = {

{"X_cursor",		XC_X_cursor,		NULL},
{"arrow",		XC_arrow,		NULL},
{"based_arrow_down",	XC_based_arrow_down,    NULL},
{"based_arrow_up",	XC_based_arrow_up,      NULL},
{"boat",		XC_boat,		NULL},
{"bogosity",		XC_bogosity,		NULL},
{"bottom_left_corner",	XC_bottom_left_corner,  NULL},
{"bottom_right_corner",	XC_bottom_right_corner, NULL},
{"bottom_side",		XC_bottom_side,		NULL},
{"bottom_tee",		XC_bottom_tee,		NULL},
{"box_spiral",		XC_box_spiral,		NULL},
{"center_ptr",		XC_center_ptr,		NULL},
{"circle",		XC_circle,		NULL},
{"clock",		XC_clock,		NULL},
{"coffee_mug",		XC_coffee_mug,		NULL},
{"cross",		XC_cross,		NULL},
{"cross_reverse",	XC_cross_reverse,       NULL},
{"crosshair",		XC_crosshair,		NULL},
{"diamond_cross",	XC_diamond_cross,       NULL},
{"dot",			XC_dot,			NULL},
{"dotbox",		XC_dotbox,		NULL},
{"double_arrow",	XC_double_arrow,	NULL},
{"draft_large",		XC_draft_large,		NULL},
{"draft_small",		XC_draft_small,		NULL},
{"draped_box",		XC_draped_box,		NULL},
{"exchange",		XC_exchange,		NULL},
{"fleur",		XC_fleur,		NULL},
{"gobbler",		XC_gobbler,		NULL},
{"gumby",		XC_gumby,		NULL},
{"hand1",		XC_hand1,		NULL},
{"hand2",		XC_hand2,		NULL},
{"heart",		XC_heart,		NULL},
{"icon",		XC_icon,		NULL},
{"iron_cross",		XC_iron_cross,		NULL},
{"left_ptr",		XC_left_ptr,		NULL},
{"left_side",		XC_left_side,		NULL},
{"left_tee",		XC_left_tee,		NULL},
{"leftbutton",		XC_leftbutton,		NULL},
{"ll_angle",		XC_ll_angle,		NULL},
{"lr_angle",		XC_lr_angle,		NULL},
{"man",			XC_man,			NULL},
{"middlebutton",	XC_middlebutton,	NULL},
{"mouse",		XC_mouse,		NULL},
{"pencil",		XC_pencil,		NULL},
{"pirate",		XC_pirate,		NULL},
{"plus",		XC_plus,		NULL},
{"question_arrow",	XC_question_arrow,	NULL},
{"right_ptr",		XC_right_ptr,		NULL},
{"right_side",		XC_right_side,		NULL},
{"right_tee",		XC_right_tee,		NULL},
{"rightbutton",		XC_rightbutton,		NULL},
{"rtl_logo",		XC_rtl_logo,		NULL},
{"sailboat",		XC_sailboat,		NULL},
{"sb_down_arrow",	XC_sb_down_arrow,       NULL},
{"sb_h_double_arrow",	XC_sb_h_double_arrow,   NULL},
{"sb_left_arrow",	XC_sb_left_arrow,       NULL},
{"sb_right_arrow",	XC_sb_right_arrow,      NULL},
{"sb_up_arrow",		XC_sb_up_arrow,		NULL},
{"sb_v_double_arrow",	XC_sb_v_double_arrow,   NULL},
{"shuttle",		XC_shuttle,		NULL},
{"sizing",		XC_sizing,		NULL},
{"spider",		XC_spider,		NULL},
{"spraycan",		XC_spraycan,		NULL},
{"star",		XC_star,		NULL},
{"target",		XC_target,		NULL},
{"tcross",		XC_tcross,		NULL},
{"top_left_arrow",	XC_top_left_arrow,      NULL},
{"top_left_corner",	XC_top_left_corner,	NULL},
{"top_right_corner",	XC_top_right_corner,    NULL},
{"top_side",		XC_top_side,		NULL},
{"top_tee",		XC_top_tee,		NULL},
{"trek",		XC_trek,		NULL},
{"ul_angle",		XC_ul_angle,		NULL},
{"umbrella",		XC_umbrella,		NULL},
{"ur_angle",		XC_ur_angle,		NULL},
{"watch",		XC_watch,		NULL},
{"xterm",		XC_xterm,		NULL},
};

NewFontCursor(cp, str)
Cursor *cp, str;
{
    int i;

    for (i = 0; i < sizeof(cursor_names); i++)
    {
	if (strcmp(str, cursor_names[i].name) == 0)
	{
	    if (cursor_names[i].cursor == NULL)
		cursor_names[i].cursor = XCreateFontCursor(dpy,
			cursor_names[i].shape);
	    *cp = cursor_names[i].cursor;
	    return;
	}
    }
    fprintf(stderr, "twm: couldn't find font cursor \"%s\"\n", str);
}

NewBitmapCursor(cp, source, mask)
Cursor *cp;
char *source, *mask;
{
    XColor fore, back;
    int hotx, hoty;
    int sx, sy, sw, sh;
    int mx, my, mw, mh;
    Pixmap spm, mpm;

    fore.pixel = Scr->Black;
    XQueryColor(dpy, Scr->CMap, &fore);
    back.pixel = Scr->White;
    XQueryColor(dpy, Scr->CMap, &back);

    spm = GetBitmap(source);
    if ((hotx = HotX) < 0) hotx = 0;
    if ((hoty = HotY) < 0) hoty = 0;
    mpm = GetBitmap(mask);

    /* make sure they are the same size */

    XGetGeometry(dpy, spm, &JunkRoot, &sx, &sy, &sw, &sh, &JunkBW, &JunkDepth);
    XGetGeometry(dpy, mpm, &JunkRoot, &mx, &my, &mw, &mh, &JunkBW, &JunkDepth);
    if (sw != mw || sh != mh)
    {
	fprintf(stderr, "twm: cursor bitmaps \"%s\" and \"%s\" are\n",
	    source, mask);
	fprintf(stderr, "     not the same size\n");
	return;
    }
    *cp = XCreatePixmapCursor(dpy, spm, mpm, &fore, &back, hotx,hoty);
}
