/* $Header: sunKeyMap.c,v 1.1 87/08/08 14:23:27 toddb Locked $ */
/*
 * The Sun X drivers are a product of Sun Microsystems, Inc. and are provided
 * for unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify these drivers without charge, but are not authorized
 * to license or distribute them to anyone else except as part of a product or
 * program developed by the user.
 * 
 * THE SUN X DRIVERS ARE PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND
 * INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE
 * PRACTICE.
 *
 * The Sun X Drivers are provided with no support and without any obligation
 * on the part of Sun Microsystems, Inc. to assist in their use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THE SUN X
 * DRIVERS OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#include	"sun.h"
#include	"keysym.h"

/*
 *	XXX - Its not clear what to map these to for now.
 *	keysyms.h doesn't define enough function key names.
 */


#define	XK_L1	NoSymbol
#define	XK_L2	NoSymbol
#define	XK_R1	NoSymbol
#define	XK_R2	NoSymbol
#define	XK_R3	NoSymbol
#define	XK_L3	NoSymbol
#define	XK_L4	NoSymbol
#define	XK_R4	NoSymbol
#define	XK_R5	NoSymbol
#define	XK_R6	NoSymbol
#define	XK_L5	NoSymbol
#define	XK_L6	NoSymbol
#define	XK_R7	NoSymbol
#define	XK_R8	NoSymbol
#define	XK_R9	NoSymbol
#define	XK_L7	NoSymbol
#define	XK_L8	NoSymbol
#define	XK_R10	NoSymbol
#define	XK_R11	NoSymbol
#define	XK_R12	NoSymbol
#define	XK_L9	NoSymbol
#define	XK_L10	NoSymbol
#define	XK_R13	NoSymbol
#define	XK_R14	NoSymbol
#define	XK_R15	NoSymbol

#ifdef	notdef
static KeySym KlunkMap[] = {
};

static KeySym VT100[] = {
};
#endif

static KeySym Sun2Map[] = {
	XK_L1,		NoSymbol,		/* 0x01 */
	NoSymbol,	NoSymbol,		/* 0x02 */
	XK_L2,		NoSymbol,		/* 0x03 */
	NoSymbol,	NoSymbol,		/* 0x4 */
	XK_F1,		NoSymbol,		/* 0x5 */
	XK_F2,		NoSymbol,		/* 0x6 */
	NoSymbol,	NoSymbol,		/* 0x7 */
	XK_F3,		NoSymbol,		/* 0x8 */
	NoSymbol,	NoSymbol,		/* 0x9 */
	XK_F4,		NoSymbol,		/* 0xa */
	NoSymbol,	NoSymbol,		/* 0xb */
	XK_F5,		NoSymbol,		/* 0xc */
	NoSymbol,	NoSymbol,		/* 0xd */
	XK_F6,		NoSymbol,		/* 0xe */
	NoSymbol,	NoSymbol,		/* 0xf */
	XK_F7,		NoSymbol,		/* 0x10 */
	XK_F8,		NoSymbol,		/* 0x11 */
	XK_F9,		NoSymbol,		/* 0x12 */
	XK_Break,	NoSymbol,		/* 0x13 */
	NoSymbol,	NoSymbol,		/* 0x14 */
	XK_R1,		NoSymbol,		/* 0x15 */
	XK_R2,		NoSymbol,		/* 0x16 */
	XK_R3,		NoSymbol,		/* 0x17 */
	NoSymbol,	NoSymbol,		/* 0x18 */
	XK_L3,		NoSymbol,		/* 0x19 */
	XK_L4,		NoSymbol,		/* 0x1a */
	NoSymbol,	NoSymbol,		/* 0x1b */
	NoSymbol,	NoSymbol,		/* 0x1c */
	XK_Escape,	NoSymbol,		/* 0x1d */
	XK_1,		XK_exclam,		/* 0x1e */
	XK_2,		XK_at,			/* 0x1f */
	XK_3,		XK_numbersign,		/* 0x20 */
	XK_4,		XK_dollar,		/* 0x21 */
	XK_5,		XK_percent,		/* 0x22 */
	XK_6,		XK_asciicircum,		/* 0x23 */
	XK_7,		XK_ampersand,		/* 0x24 */
	XK_8,		XK_asterisk,		/* 0x25 */
	XK_9,		XK_parenleft,		/* 0x26 */
	XK_0,		XK_parenright,		/* 0x27 */
	XK_minus,	XK_underscore,		/* 0x28 */
	XK_equal,	XK_plus,		/* 0x29 */
	XK_quoteleft,	XK_asciitilde,		/* 0x2a */
	XK_BackSpace,	NoSymbol,		/* 0x2b */
	NoSymbol,	NoSymbol,		/* 0x2c */
	XK_R4,		NoSymbol,		/* 0x2d */
	XK_R5,		NoSymbol,		/* 0x2e */
	XK_R6,		NoSymbol,		/* 0x2f */
	NoSymbol,	NoSymbol,		/* 0x30 */
	XK_L5,		NoSymbol,		/* 0x31 */
	NoSymbol,	NoSymbol,		/* 0x32 */
	XK_L6,		NoSymbol,		/* 0x33 */
	NoSymbol,	NoSymbol,		/* 0x34 */
	XK_Tab,		NoSymbol,		/* 0x35 */
	XK_Q,		NoSymbol,		/* 0x36 */
	XK_W,		NoSymbol,		/* 0x37 */
	XK_E,		NoSymbol,		/* 0x38 */
	XK_R,		NoSymbol,		/* 0x39 */
	XK_T,		NoSymbol,		/* 0x3a */
	XK_Y,		NoSymbol,		/* 0x3b */
	XK_U,		NoSymbol,		/* 0x3c */
	XK_I,		NoSymbol,		/* 0x3d */
	XK_O,		NoSymbol,		/* 0x3e */
	XK_P,		NoSymbol,		/* 0x3f */
	XK_bracketleft,	XK_braceleft,		/* 0x40 */
	XK_bracketright,	XK_braceright,	/* 0x41 */
	XK_Delete,	NoSymbol,		/* 0x42 */
	NoSymbol,	NoSymbol,		/* 0x43 */
	XK_R7,		NoSymbol,		/* 0x44 */
	XK_Up,		XK_R8,			/* 0x45 */
	XK_R9,		NoSymbol,		/* 0x46 */
	NoSymbol,	NoSymbol,		/* 0x47 */
	XK_L7,		NoSymbol,		/* 0x48 */
	XK_L8,		NoSymbol,		/* 0x49 */
	NoSymbol,	NoSymbol,		/* 0x4a */
	NoSymbol,	NoSymbol,		/* 0x4b */
	XK_Control_L,	NoSymbol,		/* 0x4c */
	XK_A,		NoSymbol,		/* 0x4d */
	XK_S,		NoSymbol,		/* 0x4e */
	XK_D,		NoSymbol,		/* 0x4f */
	XK_F,		NoSymbol,		/* 0x50 */
	XK_G,		NoSymbol,		/* 0x51 */
	XK_H,		NoSymbol,		/* 0x52 */
	XK_J,		NoSymbol,		/* 0x53 */
	XK_K,		NoSymbol,		/* 0x54 */
	XK_L,		NoSymbol,		/* 0x55 */
	XK_semicolon,	XK_colon,		/* 0x56 */
	XK_quoteright,	XK_quotedbl,		/* 0x57 */
	XK_backslash,	XK_bar,			/* 0x58 */
	XK_Return,	NoSymbol,		/* 0x59 */
	NoSymbol,	NoSymbol,		/* 0x5a */
	XK_Left,	XK_R10,			/* 0x5b */
	XK_R11,		NoSymbol,		/* 0x5c */
	XK_Right,	NoSymbol,		/* 0x5d */
	NoSymbol,	NoSymbol,		/* 0x5e */
	XK_L9,		NoSymbol,		/* 0x5f */
	NoSymbol,	NoSymbol,		/* 0x60 */
	XK_L10,		NoSymbol,		/* 0x61 */
	NoSymbol,	NoSymbol,		/* 0x62 */
	XK_Shift_L,	NoSymbol,		/* 0x63 */
	XK_Z,		NoSymbol,		/* 0x64 */
	XK_X,		NoSymbol,		/* 0x65 */
	XK_C,		NoSymbol,		/* 0x66 */
	XK_V,		NoSymbol,		/* 0x67 */
	XK_B,		NoSymbol,		/* 0x68 */
	XK_N,		NoSymbol,		/* 0x69 */
	XK_M,		NoSymbol,		/* 0x6a */
	XK_comma,	XK_less,		/* 0x6b */
	XK_period,	XK_greater,		/* 0x6c */
	XK_slash,	XK_question,		/* 0x6d */
	XK_Shift_R,	NoSymbol,		/* 0x6e */
	XK_Linefeed,	NoSymbol,		/* 0x6f */
	XK_R13,		NoSymbol,		/* 0x70 */
	XK_Down,	XK_R14,			/* 0x71 */
	XK_R15,		NoSymbol,		/* 0x72 */
	NoSymbol,	NoSymbol,		/* 0x73 */
	NoSymbol,	NoSymbol,		/* 0x74 */
	NoSymbol,	NoSymbol,		/* 0x75 */
	NoSymbol,	NoSymbol,		/* 0x76 */
	NoSymbol,	NoSymbol,		/* 0x77 */
	XK_Meta_L,	NoSymbol,		/* 0x78 */
	XK_space,	NoSymbol,		/* 0x79 */
	XK_Meta_R,	NoSymbol,		/* 0x7a */
	NoSymbol,	NoSymbol,		/* 0x7b */
	NoSymbol,	NoSymbol,		/* 0x7c */
	NoSymbol,	NoSymbol,		/* 0x7d */
	NoSymbol,	NoSymbol,		/* 0x7e */
	NoSymbol,	NoSymbol,		/* 0x7f */
};

static KeySym Sun3Map[] = {
	XK_L1,		NoSymbol,		/* 0x01 */
	NoSymbol,	NoSymbol,		/* 0x02 */
	XK_L2,		NoSymbol,		/* 0x03 */
	NoSymbol,	NoSymbol,		/* 0x4 */
	XK_F1,		NoSymbol,		/* 0x5 */
	XK_F2,		NoSymbol,		/* 0x6 */
	NoSymbol,	NoSymbol,		/* 0x7 */
	XK_F3,		NoSymbol,		/* 0x8 */
	NoSymbol,	NoSymbol,		/* 0x9 */
	XK_F4,		NoSymbol,		/* 0xa */
	NoSymbol,	NoSymbol,		/* 0xb */
	XK_F5,		NoSymbol,		/* 0xc */
	NoSymbol,	NoSymbol,		/* 0xd */
	XK_F6,		NoSymbol,		/* 0xe */
	NoSymbol,	NoSymbol,		/* 0xf */
	XK_F7,		NoSymbol,		/* 0x10 */
	XK_F8,		NoSymbol,		/* 0x11 */
	XK_F9,		NoSymbol,		/* 0x12 */
	XK_Break,	NoSymbol,		/* 0x13 */
	NoSymbol,	NoSymbol,		/* 0x14 */
	XK_R1,		NoSymbol,		/* 0x15 */
	XK_R2,		NoSymbol,		/* 0x16 */
	XK_R3,		NoSymbol,		/* 0x17 */
	NoSymbol,	NoSymbol,		/* 0x18 */
	XK_L3,		NoSymbol,		/* 0x19 */
	XK_L4,		NoSymbol,		/* 0x1a */
	NoSymbol,	NoSymbol,		/* 0x1b */
	NoSymbol,	NoSymbol,		/* 0x1c */
	XK_Escape,	NoSymbol,		/* 0x1d */
	XK_1,		XK_exclam,		/* 0x1e */
	XK_2,		XK_at,			/* 0x1f */
	XK_3,		XK_numbersign,		/* 0x20 */
	XK_4,		XK_dollar,		/* 0x21 */
	XK_5,		XK_percent,		/* 0x22 */
	XK_6,		XK_asciicircum,		/* 0x23 */
	XK_7,		XK_ampersand,		/* 0x24 */
	XK_8,		XK_asterisk,		/* 0x25 */
	XK_9,		XK_parenleft,		/* 0x26 */
	XK_0,		XK_parenright,		/* 0x27 */
	XK_minus,	XK_underscore,		/* 0x28 */
	XK_equal,	XK_plus,		/* 0x29 */
	XK_quoteleft,	XK_asciitilde,		/* 0x2a */
	XK_BackSpace,	NoSymbol,		/* 0x2b */
	NoSymbol,	NoSymbol,		/* 0x2c */
	XK_R4,		NoSymbol,		/* 0x2d */
	XK_R5,		NoSymbol,		/* 0x2e */
	XK_R6,		NoSymbol,		/* 0x2f */
	NoSymbol,	NoSymbol,		/* 0x30 */
	XK_L5,		NoSymbol,		/* 0x31 */
	NoSymbol,	NoSymbol,		/* 0x32 */
	XK_L6,		NoSymbol,		/* 0x33 */
	NoSymbol,	NoSymbol,		/* 0x34 */
	XK_Tab,		NoSymbol,		/* 0x35 */
	XK_Q,		NoSymbol,		/* 0x36 */
	XK_W,		NoSymbol,		/* 0x37 */
	XK_E,		NoSymbol,		/* 0x38 */
	XK_R,		NoSymbol,		/* 0x39 */
	XK_T,		NoSymbol,		/* 0x3a */
	XK_Y,		NoSymbol,		/* 0x3b */
	XK_U,		NoSymbol,		/* 0x3c */
	XK_I,		NoSymbol,		/* 0x3d */
	XK_O,		NoSymbol,		/* 0x3e */
	XK_P,		NoSymbol,		/* 0x3f */
	XK_bracketleft,	XK_braceleft,		/* 0x40 */
	XK_bracketright,	XK_braceright,	/* 0x41 */
	XK_Delete,	NoSymbol,		/* 0x42 */
	NoSymbol,	NoSymbol,		/* 0x43 */
	XK_R7,		NoSymbol,		/* 0x44 */
	XK_Up,		XK_R8,			/* 0x45 */
	XK_R9,		NoSymbol,		/* 0x46 */
	NoSymbol,	NoSymbol,		/* 0x47 */
	XK_L7,		NoSymbol,		/* 0x48 */
	XK_L8,		NoSymbol,		/* 0x49 */
	NoSymbol,	NoSymbol,		/* 0x4a */
	NoSymbol,	NoSymbol,		/* 0x4b */
	XK_Control_L,	NoSymbol,		/* 0x4c */
	XK_A,		NoSymbol,		/* 0x4d */
	XK_S,		NoSymbol,		/* 0x4e */
	XK_D,		NoSymbol,		/* 0x4f */
	XK_F,		NoSymbol,		/* 0x50 */
	XK_G,		NoSymbol,		/* 0x51 */
	XK_H,		NoSymbol,		/* 0x52 */
	XK_J,		NoSymbol,		/* 0x53 */
	XK_K,		NoSymbol,		/* 0x54 */
	XK_L,		NoSymbol,		/* 0x55 */
	XK_semicolon,	XK_colon,		/* 0x56 */
	XK_quoteright,	XK_quotedbl,		/* 0x57 */
	XK_backslash,	XK_bar,			/* 0x58 */
	XK_Return,	NoSymbol,		/* 0x59 */
	NoSymbol,	NoSymbol,		/* 0x5a */
	XK_Left,	XK_R10,			/* 0x5b */
	XK_R11,		NoSymbol,		/* 0x5c */
	XK_Right,	NoSymbol,		/* 0x5d */
	NoSymbol,	NoSymbol,		/* 0x5e */
	XK_L9,		NoSymbol,		/* 0x5f */
	NoSymbol,	NoSymbol,		/* 0x60 */
	XK_L10,		NoSymbol,		/* 0x61 */
	NoSymbol,	NoSymbol,		/* 0x62 */
	XK_Shift_L,	NoSymbol,		/* 0x63 */
	XK_Z,		NoSymbol,		/* 0x64 */
	XK_X,		NoSymbol,		/* 0x65 */
	XK_C,		NoSymbol,		/* 0x66 */
	XK_V,		NoSymbol,		/* 0x67 */
	XK_B,		NoSymbol,		/* 0x68 */
	XK_N,		NoSymbol,		/* 0x69 */
	XK_M,		NoSymbol,		/* 0x6a */
	XK_comma,	XK_less,		/* 0x6b */
	XK_period,	XK_greater,		/* 0x6c */
	XK_slash,	XK_question,		/* 0x6d */
	XK_Shift_R,	NoSymbol,		/* 0x6e */
	XK_Linefeed,	NoSymbol,		/* 0x6f */
	XK_R13,		NoSymbol,		/* 0x70 */
	XK_Down,	XK_R14,			/* 0x71 */
	XK_R15,		NoSymbol,		/* 0x72 */
	NoSymbol,	NoSymbol,		/* 0x73 */
	NoSymbol,	NoSymbol,		/* 0x74 */
	NoSymbol,	NoSymbol,		/* 0x75 */
	NoSymbol,	NoSymbol,		/* 0x76 */
	XK_Caps_Lock,	NoSymbol,		/* 0x77 */
	XK_Meta_L,	NoSymbol,		/* 0x78 */
	XK_space,	NoSymbol,		/* 0x79 */
	XK_Meta_R,	NoSymbol,		/* 0x7a */
	NoSymbol,	NoSymbol,		/* 0x7b */
	NoSymbol,	NoSymbol,		/* 0x7c */
	NoSymbol,	NoSymbol,		/* 0x7d */
	NoSymbol,	NoSymbol,		/* 0x7e */
	NoSymbol,	NoSymbol,		/* 0x7f */
};

KeySymsRec sunKeySyms[] = {
    /*	map	   minKeyCode	maxKC	width */
#ifdef	notdef
    KlunkMap,		1,	0x7a,	2,
    VT100Map,		1,	0x7a,	2,
#else
    (KeySym *)NULL,	0,	0,	0,
    (KeySym *)NULL,	0,	0,	0,
#endif
    Sun2Map,		1,	0x7a,	2,
    Sun3Map,		1,	0x7a,	2,
};

ModifierMapRec sunMapRec[] = {
/*	lock shiftA shiftB ctrlA ctrlB m1a m1b m2a m2b m3a m3b m4a m4b m5a m5b */
    {    0,    0,     0,     0,   0,   0,    0,  0, 0,  0,  0,  0,  0,  0,  0,},
    {    0,    0,     0,     0,   0,   0,    0,  0, 0,  0,  0,  0,  0,  0,  0,},
    {	0x77, 0x63,  0x6e, 0x4c,  0, 0x78, 0x7a, 0, 0,  0,  0,  0,  0,  0,  0,},
    {	0x77, 0x63,  0x6e, 0x4c,  0, 0x78, 0x7a, 0, 0,  0,  0,  0,  0,  0,  0,},
};
