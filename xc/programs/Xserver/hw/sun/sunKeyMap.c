/* $XConsortium: sunKeyMap.c,v 4.11 93/08/06 15:09:38 kaleb Exp $ */
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include	"sun.h"
#define		XK_KATAKANA
#include	"keysym.h"
#include	"Sunkeysym.h"

/*
 * Which keyboards should we support?
 * Default is that all keyboards are supported
 * Each keyboard type occupies approx 2.5K so you are recomended
 * to remove support for all keyboards except those you are going
 * to use. To remove support for a keyboard type just remove the
 * corresponding define below.
 */
#define CANADA4
#define DENMARK4
#define DENMARK5
#define FRANCE5
#define FRANCEBELG4
#define GERMANY4
#define GERMANY5
#define ITALY4
#define ITALY5
#define JAPAN4
#define JAPAN5
#define KOREA4
#define KOREA5
#define NETHERLAND4
#define NETHERLAND5
#define NORWAY4
#define NORWAY5
#define PORTUGAL4
#define PORTUGAL5
#define SPAIN5
#define SPAINLATAM4 
#define SWEDEN5
#define SWEDENFIN4
#define SWITZER_FR4
#define SWITZER_FR5
#define SWITZER_GE4
#define SWITZER_GE5
#define TAIWAN4
#define TAIWAN5
#define UK4
#define UK5
#define US101A_SUN
#define US4
#define US5
#define US_UNIX5

/*
 *	XXX - Its not clear what to map these to for now.
 *	keysyms.h doesn't define enough function key names.
 */


#ifndef	XK_L1
#define	XK_L1	XK_Cancel
#define	XK_L2	XK_Redo
#define	XK_R1	NoSymbol
#define	XK_R2	NoSymbol
#define	XK_R3	NoSymbol
#define	XK_L3	XK_Menu
#define	XK_L4	XK_Undo
#define	XK_R4	NoSymbol
#define	XK_R5	NoSymbol
#define	XK_R6	NoSymbol
#define	XK_L5	XK_Insert
#define	XK_L6	XK_Select
#define	XK_R7	NoSymbol
#define	XK_R8	XK_Up
#define	XK_R9	NoSymbol
#define	XK_L7	XK_Execute
#define	XK_L8	XK_Print
#define	XK_R10	XK_Left
#define	XK_R11	XK_Home
#define	XK_R12	XK_Right
#define	XK_L9	XK_Find
#define	XK_L10	XK_Help
#define	XK_R13	NoSymbol
#define	XK_R14	XK_Down
#define	XK_R15	NoSymbol
#endif

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
	XK_Right,	XK_R12,			/* 0x5d */
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
	XK_Alt_R,	NoSymbol,		/* 0x13 */
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
	XK_Right,	XK_R12,			/* 0x5d */
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

#define	cT	(ControlMask)
#define	sH	(ShiftMask)
#define	lK	(LockMask)
#define	mT	(Mod1Mask)
#define	nL	(Mod4Mask)
#define	mK	(Mod5Mask)
static CARD8 type2modmap[MAP_LENGTH] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 40-4f */
    0,  0,  0,  cT, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 50-5f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  sH, 0,  0,  0,  0,  0, /* 60-6f */
    0,  0,  0,  0,  0,  sH, 0,  0,  0,  0,  0,  0,  0,  0,  0,  mT,/* 70-7f */
    0,  mT, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 80-8f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 90-9f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* a0-af */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* b0-bf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* c0-cf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* d0-df */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* e0-ef */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* f0-ff */
};

static CARD8 type3modmap[MAP_LENGTH] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 00-0f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 10-1f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 20-2f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 30-3f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 40-4f */
    0,  0,  0,  cT, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 50-5f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  sH, 0,  0,  0,  0,  0, /* 60-6f */
    0,  0,  0,  0,  0,  sH, 0,  0,  0,  0,  0,  0,  0,  0,  lK, mT,/* 70-7f */
    0,  mT, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 80-8f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 90-9f */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* a0-af */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* b0-bf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* c0-cf */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* d0-df */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* e0-ef */
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* f0-ff */
};

CARD8 *sunModMap[] = {
	NULL,
	NULL,
	type2modmap,
	type3modmap,
};

/*
 * The compose codes are coded as:
 *
 *	First char	second char	result
 */
KeySym sunType4ComposeMap[] = {
	XK_space,	XK_space,	XK_nobreakspace,
	XK_exclam,	XK_exclam,	XK_exclamdown,
	XK_C,		XK_slash,	XK_cent,
	XK_c,		XK_slash,	XK_cent,
	XK_L,		XK_minus,	XK_sterling,
	XK_l,		XK_minus,	XK_sterling,
	XK_o,		XK_x,		XK_currency,
	XK_O,		XK_X,		XK_currency,
	XK_0,		XK_x,		XK_currency,
	XK_0,		XK_X,		XK_currency,
	XK_Y,		XK_minus,	XK_yen,
	XK_y,		XK_minus,	XK_yen,
	XK_bar,		XK_bar,		XK_brokenbar,
	XK_S,		XK_O,		XK_section,
	XK_s,		XK_o,		XK_section,
	XK_quotedbl,	XK_quotedbl,	XK_diaeresis,
	XK_C,		XK_O,		XK_copyright,
	XK_c,		XK_o,		XK_copyright,
	XK_minus,	XK_A,		XK_ordfeminine,
	XK_minus,	XK_a,		XK_ordfeminine,
	XK_less,	XK_less,	XK_guillemotleft,
	XK_minus,	XK_bar,		XK_notsign,
	XK_minus,	XK_comma,	XK_notsign,
	XK_minus,	XK_minus,	XK_hyphen,
	XK_R,		XK_O,		XK_registered,
	XK_r,		XK_o,		XK_registered,
	XK_asciicircum,	XK_minus,	XK_macron,
	XK_asciicircum,	XK_asterisk,	XK_degree,
	XK_plus,	XK_minus,	XK_plusminus,
	XK_asciicircum,	XK_2,		XK_twosuperior,
	XK_asciicircum,	XK_3,		XK_threesuperior,
	XK_backslash,	XK_backslash,	XK_acute,
	XK_slash,	XK_u,		XK_mu,
	XK_P,		XK_exclam,	XK_paragraph,
	XK_asciicircum,	XK_period,	XK_periodcentered,
	XK_comma,	XK_comma,	XK_cedilla,
	XK_asciicircum,	XK_1,		XK_onesuperior,
	XK_underscore,	XK_O,		XK_masculine,
	XK_underscore,	XK_o,		XK_masculine,
	XK_greater,	XK_greater,	XK_guillemotright,
	XK_1,		XK_4,		XK_onequarter,
	XK_1,		XK_2,		XK_onehalf,
	XK_3,		XK_4,		XK_threequarters,
	XK_question,	XK_question,	XK_questiondown,
	XK_A,		XK_grave,	XK_Agrave,
	XK_A,		XK_apostrophe,	XK_Aacute,
	XK_A,		XK_asciicircum,	XK_Acircumflex,
	XK_A,		XK_asciitilde,	XK_Atilde,
	XK_A,		XK_quotedbl,	XK_Adiaeresis,
	XK_A,		XK_asterisk,	XK_Aring,
	XK_A,		XK_E,		XK_AE,
	XK_C,		XK_comma,	XK_Ccedilla,
	XK_E,		XK_grave,	XK_Egrave,
	XK_E,		XK_apostrophe,	XK_Eacute,
	XK_E,		XK_asciicircum,	XK_Ecircumflex,
	XK_E,		XK_quotedbl,	XK_Ediaeresis,
	XK_I,		XK_grave,	XK_Igrave,
	XK_I,		XK_apostrophe,	XK_Iacute,
	XK_I,		XK_asciicircum,	XK_Icircumflex,
	XK_I,		XK_quotedbl,	XK_Idiaeresis,
	XK_D,		XK_minus,	XK_Eth,
	XK_N,		XK_asciitilde,	XK_Ntilde,
	XK_O,		XK_grave,	XK_Ograve,
	XK_O,		XK_apostrophe,	XK_Oacute,
	XK_O,		XK_asciicircum,	XK_Ocircumflex,
	XK_O,		XK_asciitilde,	XK_Otilde,
	XK_O,		XK_quotedbl,	XK_Odiaeresis,
	XK_x,		XK_x,		XK_multiply,
	XK_O,		XK_slash,	XK_Ooblique,
	XK_U,		XK_grave,	XK_Ugrave,
	XK_U,		XK_apostrophe,	XK_Uacute,
	XK_U,		XK_asciicircum,	XK_Ucircumflex,
	XK_U,		XK_quotedbl,	XK_Udiaeresis,
	XK_Y,		XK_apostrophe,	XK_Yacute,
	XK_P,		XK_bar,		XK_Thorn,
	XK_T,		XK_H,		XK_Thorn,
	XK_s,		XK_s,		XK_ssharp,
	XK_a,		XK_grave,	XK_agrave,
	XK_a,		XK_apostrophe,	XK_aacute,
	XK_a,		XK_asciicircum,	XK_acircumflex,
	XK_a,		XK_asciitilde,	XK_atilde,
	XK_a,		XK_quotedbl,	XK_adiaeresis,
	XK_a,		XK_asterisk,	XK_aring,
	XK_a,		XK_e,		XK_ae,
	XK_c,		XK_comma,	XK_ccedilla,
	XK_e,		XK_grave,	XK_egrave,
	XK_e,		XK_apostrophe,	XK_eacute,
	XK_e,		XK_asciicircum,	XK_ecircumflex,
	XK_e,		XK_quotedbl,	XK_ediaeresis,
	XK_i,		XK_grave,	XK_igrave,
	XK_i,		XK_apostrophe,	XK_iacute,
	XK_i,		XK_asciicircum,	XK_icircumflex,
	XK_i,		XK_quotedbl,	XK_idiaeresis,
	XK_d,		XK_minus,	XK_eth,
	XK_n,		XK_asciitilde,	XK_ntilde,
	XK_o,		XK_grave,	XK_ograve,
	XK_o,		XK_apostrophe,	XK_oacute,
	XK_o,		XK_asciicircum,	XK_ocircumflex,
	XK_o,		XK_asciitilde,	XK_otilde,
	XK_o,		XK_quotedbl,	XK_odiaeresis,
	XK_minus,	XK_colon,	XK_division,
	XK_o,		XK_slash,	XK_oslash,
	XK_u,		XK_grave,	XK_ugrave,
	XK_u,		XK_apostrophe,	XK_uacute,
	XK_u,		XK_asciicircum,	XK_ucircumflex,
	XK_u,		XK_quotedbl,	XK_udiaeresis,
	XK_y,		XK_apostrophe,	XK_yacute,
	XK_t,		XK_h,		XK_thorn,
	XK_p,		XK_bar,		XK_thorn,
	XK_y,		XK_quotedbl,	XK_ydiaeresis,
	0,		0,		0
};

/*
 * The following generic maplength is used for the undefined maps
 */
static int genericmaplen = 0;

#ifdef CANADA4

static int Canada4maplen = 125 ;

static KeySym Canada4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	XK_Control_R,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_degree,	NoSymbol,	XK_notsign,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_question,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	XK_braceright,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	XK_bar, 	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_Agrave,	NoSymbol,	XK_quoteleft,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Circum,	SunXK_FA_Diaeresis,	XK_asciicircum,	NoSymbol,	/* 64*/
	XK_Ccedilla,	NoSymbol,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Egrave,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	XK_guillemotleft,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	XK_guillemotright,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_quoteright,	XK_less,	NoSymbol,	/*107*/
	XK_period,	NoSymbol,	XK_greater,	NoSymbol,	/*108*/
	XK_Eacute,	NoSymbol,	XK_slash,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_Ugrave,	NoSymbol,	XK_backslash,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Canada4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	NoSymbol,	NoSymbol
};

static SunModmapRec Canada4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	13,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	119,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Canada4Map     	NULL
#define Canada4maplen 	genericmaplen
#define Canada4KPMap   	NULL
#define Canada4modmap  	NULL

#endif /* CANADA4 */


#ifdef DENMARK4

static int Denmark4maplen = 125 ;

static KeySym Denmark4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_asciitilde,	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_currency,	XK_dollar,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_plus,	XK_question,	NoSymbol,	NoSymbol,	/* 40*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	XK_bar, 	NoSymbol,	/* 41*/
	XK_quoteright,	XK_asterisk,	XK_quoteleft,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Aring,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Circum,	SunXK_FA_Tilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_AE,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Ooblique,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_onehalf,	XK_section,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_backslash,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Denmark4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Denmark4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	67,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Denmark4Map     	NULL
#define Denmark4maplen 		genericmaplen
#define Denmark4KPMap   	NULL
#define Denmark4modmap  	NULL

#endif /* DENMARK4 */


#ifdef DENMARK5

static int Denmark5maplen = 125 ;

static KeySym Denmark5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_currency,	XK_dollar,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_asciicircum,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_plus,	XK_question,	NoSymbol,	NoSymbol,	/* 40*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	XK_bar, 	NoSymbol,	/* 41*/
	XK_onehalf,	XK_section,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Separator,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Aring,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Circum,	SunXK_FA_Tilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_AE,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Ooblique,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_quoteright,	XK_asterisk,	XK_quoteleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_backslash,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Denmark5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Separator,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Denmark5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Denmark5Map     	NULL
#define Denmark5maplen		genericmaplen
#define Denmark5KPMap   	NULL
#define Denmark5modmap  	NULL

#endif /* DENMARK5 */


#ifdef FRANCE5

static int France5maplen = 125 ;

static KeySym France5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_ampersand,	XK_1,   	NoSymbol,	NoSymbol,	/* 30*/
	XK_eacute,	XK_2,   	XK_asciitilde,	NoSymbol,	/* 31*/
	XK_quotedbl,	XK_3,   	XK_numbersign,	NoSymbol,	/* 32*/
	XK_quoteright,	XK_4,   	XK_braceleft,	NoSymbol,	/* 33*/
	XK_parenleft,	XK_5,   	XK_bracketleft,	NoSymbol,	/* 34*/
	XK_minus,	XK_6,   	XK_bar, 	NoSymbol,	/* 35*/
	XK_egrave,	XK_7,   	XK_quoteleft,	NoSymbol,	/* 36*/
	XK_underscore,	XK_8,   	XK_backslash,	NoSymbol,	/* 37*/
	XK_ccedilla,	XK_9,   	XK_asciicircum,	NoSymbol,	/* 38*/
	XK_agrave,	XK_0,   	XK_at,  	NoSymbol,	/* 39*/
	XK_parenright,	XK_degree,	XK_bracketright,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	XK_braceright,	NoSymbol,	/* 41*/
	XK_twosuperior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Circum,	SunXK_FA_Diaeresis,	NoSymbol,	NoSymbol,	/* 64*/
	XK_dollar,	XK_sterling,	XK_currency,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_ugrave,	XK_percent,	NoSymbol,	NoSymbol,	/* 87*/
	XK_asterisk,	XK_mu,  	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_comma,	XK_question,	NoSymbol,	NoSymbol,	/*106*/
	XK_semicolon,	XK_period,	NoSymbol,	NoSymbol,	/*107*/
	XK_colon,	XK_slash,	NoSymbol,	NoSymbol,	/*108*/
	XK_exclam,	XK_section,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec France5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec France5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define France5Map     	NULL
#define France5maplen	genericmaplen
#define France5KPMap   	NULL
#define France5modmap  	NULL

#endif /* FRANCE5 */


#ifdef FRANCEBELG4

static int FranceBelg4maplen = 125 ;

static KeySym FranceBelg4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_bracketright,	XK_braceright,	XK_guillemotright,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_ampersand,	XK_1,   	NoSymbol,	NoSymbol,	/* 30*/
	XK_eacute,	XK_2,   	XK_twosuperior,	NoSymbol,	/* 31*/
	XK_quotedbl,	XK_3,   	XK_threesuperior,	NoSymbol,	/* 32*/
	XK_quoteright,	XK_4,   	NoSymbol,	NoSymbol,	/* 33*/
	XK_parenleft,	XK_5,   	NoSymbol,	NoSymbol,	/* 34*/
	XK_section,	XK_6,   	XK_asciicircum,	NoSymbol,	/* 35*/
	XK_egrave,	XK_7,   	NoSymbol,	NoSymbol,	/* 36*/
	XK_exclam,	XK_8,   	XK_sterling,	NoSymbol,	/* 37*/
	XK_ccedilla,	XK_9,   	XK_backslash,	NoSymbol,	/* 38*/
	XK_agrave,	XK_0,   	NoSymbol,	NoSymbol,	/* 39*/
	XK_parenright,	XK_degree,	XK_asciitilde,	NoSymbol,	/* 40*/
	XK_minus,	XK_underscore,	XK_numbersign,	NoSymbol,	/* 41*/
	XK_asterisk,	XK_bar, 	XK_currency,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Circum,	SunXK_FA_Diaeresis,	NoSymbol,	NoSymbol,	/* 64*/
	XK_quoteleft,	XK_dollar,	XK_at,  	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_M,   	NoSymbol,	XK_mu,  	NoSymbol,	/* 86*/
	XK_ugrave,	XK_percent,	NoSymbol,	NoSymbol,	/* 87*/
	XK_bracketleft,	XK_braceleft,	XK_guillemotleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_comma,	XK_question,	NoSymbol,	NoSymbol,	/*106*/
	XK_semicolon,	XK_period,	NoSymbol,	NoSymbol,	/*107*/
	XK_colon,	XK_slash,	NoSymbol,	NoSymbol,	/*108*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec FranceBelg4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec FranceBelg4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	13,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	119,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define FranceBelg4Map     	NULL
#define FranceBelg4maplen 	genericmaplen
#define FranceBelg4KPMap   	NULL
#define FranceBelg4modmap  	NULL

#endif /* FRANCEBELG4 */


#ifdef GERMANY4

static int Germany4maplen = 125 ;

static KeySym Germany4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	XK_Alt_R,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_bracketright,	XK_braceright,	XK_guillemotright,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_twosuperior,	NoSymbol,	/* 31*/
	XK_3,   	XK_section,	XK_threesuperior,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_degree,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_quoteleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_quoteright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_bar, 	NoSymbol,	/* 39*/
	XK_ssharp,	XK_question,	XK_backslash,	NoSymbol,	/* 40*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 41*/
	XK_numbersign,	XK_asciicircum,	XK_at,  	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Udiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	XK_plus,	XK_asterisk,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Odiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Adiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_bracketleft,	XK_braceleft,	XK_guillemotleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	XK_mu,  	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Germany4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Germany4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	19,	(Mod2Mask),
	13,	(Mod3Mask),
	0,	0
};

#else

#define Germany4Map     	NULL
#define Germany4maplen 		genericmaplen
#define Germany4KPMap   	NULL
#define Germany4modmap  	NULL

#endif /* GERMANY4 */


#ifdef GERMANY5

static int Germany5maplen = 125 ;

static KeySym Germany5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_twosuperior,	NoSymbol,	/* 31*/
	XK_3,   	XK_section,	XK_threesuperior,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_ssharp,	XK_question,	XK_backslash,	NoSymbol,	/* 40*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 41*/
	XK_asciicircum,	XK_degree,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Separator,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_q,   	XK_Q,   	XK_at,  	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Udiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	XK_plus,	XK_asterisk,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Odiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Adiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_numbersign,	XK_quoteright,	XK_quoteleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_m,   	XK_M,   	XK_mu,  	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_bar, 	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Germany5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Separator,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Germany5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Germany5Map     	NULL
#define Germany5maplen 		genericmaplen
#define Germany5KPMap   	NULL
#define Germany5modmap  	NULL

#endif /* GERMANY5 */


#ifdef ITALY4

static int Italy4maplen = 125 ;

static KeySym Italy4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_bracketright,	XK_braceright,	XK_guillemotright,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_twosuperior,	NoSymbol,	/* 31*/
	XK_3,   	XK_sterling,	XK_threesuperior,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_notsign,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_backslash,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_bar, 	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	XK_igrave,	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 41*/
	XK_ugrave,	XK_section,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_egrave,	XK_eacute,	NoSymbol,	NoSymbol,	/* 64*/
	XK_plus,	XK_asterisk,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_ograve,	XK_ccedilla,	XK_at,  	NoSymbol,	/* 86*/
	XK_agrave,	XK_degree,	XK_numbersign,	NoSymbol,	/* 87*/
	XK_bracketleft,	XK_braceleft,	XK_guillemotleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Italy4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Italy4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Italy4Map     	NULL
#define Italy4maplen 	genericmaplen
#define Italy4KPMap   	NULL
#define Italy4modmap  	NULL

#endif /* ITALY4 */


#ifdef ITALY5

static int Italy5maplen = 125 ;

static KeySym Italy5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_sterling,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_braceleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_braceright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	NoSymbol,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	XK_igrave,	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 41*/
	XK_backslash,	XK_bar, 	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_egrave,	XK_eacute,	XK_bracketleft,	NoSymbol,	/* 64*/
	XK_plus,	XK_asterisk,	XK_bracketright,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_ograve,	XK_ccedilla,	XK_at,  	NoSymbol,	/* 86*/
	XK_agrave,	XK_degree,	XK_numbersign,	NoSymbol,	/* 87*/
	XK_ugrave,	XK_section,	XK_asciitilde,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Italy5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Italy5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Italy5Map     	NULL
#define Italy5maplen 	genericmaplen
#define Italy5KPMap   	NULL
#define Italy5modmap  	NULL

#endif /* ITALY5 */


#ifdef JAPAN4

static int Japan4maplen = 125 ;

static KeySym Japan4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	XK_kana_NU,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_kana_FU,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_kana_A,	XK_kana_a,	/* 32*/
	XK_4,   	XK_dollar,	XK_kana_U,	XK_kana_u,	/* 33*/
	XK_5,   	XK_percent,	XK_kana_E,	XK_kana_e,	/* 34*/
	XK_6,   	XK_ampersand,	XK_kana_O,	XK_kana_o,	/* 35*/
	XK_7,   	XK_quoteright,	XK_kana_YA,	XK_kana_ya,	/* 36*/
	XK_8,   	XK_parenleft,	XK_kana_YU,	XK_kana_yu,	/* 37*/
	XK_9,   	XK_parenright,	XK_kana_YO,	XK_kana_yo,	/* 38*/
	XK_0,   	XK_0,   	XK_kana_WA,	XK_kana_WO,	/* 39*/
	XK_minus,	XK_equal,	XK_kana_HO,	NoSymbol,	/* 40*/
	XK_asciicircum,	XK_asciitilde,	XK_kana_HE,	NoSymbol,	/* 41*/
	XK_bracketright,	XK_braceright,	XK_kana_MU,	XK_kana_closingbracket,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_q,   	XK_Q,   	XK_kana_TA,	NoSymbol,	/* 54*/
	XK_w,   	XK_W,   	XK_kana_TE,	NoSymbol,	/* 55*/
	XK_e,   	XK_E,   	XK_kana_I,	XK_kana_i,	/* 56*/
	XK_r,   	XK_R,   	XK_kana_SU,	NoSymbol,	/* 57*/
	XK_t,   	XK_T,   	XK_kana_KA,	NoSymbol,	/* 58*/
	XK_y,   	XK_Y,   	XK_kana_N,	NoSymbol,	/* 59*/
	XK_u,   	XK_U,   	XK_kana_NA,	NoSymbol,	/* 60*/
	XK_i,   	XK_I,   	XK_kana_NI,	NoSymbol,	/* 61*/
	XK_o,   	XK_O,   	XK_kana_RA,	NoSymbol,	/* 62*/
	XK_p,   	XK_P,   	XK_kana_SE,	NoSymbol,	/* 63*/
	XK_at,  	XK_quoteleft,	XK_voicedsound,	NoSymbol,	/* 64*/
	XK_bracketleft,	XK_braceleft,	XK_semivoicedsound,	XK_kana_openingbracket,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_a,   	XK_A,   	XK_kana_CHI,	NoSymbol,	/* 77*/
	XK_s,   	XK_S,   	XK_kana_TO,	NoSymbol,	/* 78*/
	XK_d,   	XK_D,   	XK_kana_SHI,	NoSymbol,	/* 79*/
	XK_f,   	XK_F,   	XK_kana_HA,	NoSymbol,	/* 80*/
	XK_g,   	XK_G,   	XK_kana_KI,	NoSymbol,	/* 81*/
	XK_h,   	XK_H,   	XK_kana_KU,	NoSymbol,	/* 82*/
	XK_j,   	XK_J,   	XK_kana_MA,	NoSymbol,	/* 83*/
	XK_k,   	XK_K,   	XK_kana_NO,	NoSymbol,	/* 84*/
	XK_l,   	XK_L,   	XK_kana_RI,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_plus,	XK_kana_RE,	NoSymbol,	/* 86*/
	XK_colon,	XK_asterisk,	XK_kana_KE,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_prolongedsound,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_z,   	XK_Z,   	XK_kana_TSU,	XK_kana_tsu,	/*100*/
	XK_x,   	XK_X,   	XK_kana_SA,	NoSymbol,	/*101*/
	XK_c,   	XK_C,   	XK_kana_SO,	NoSymbol,	/*102*/
	XK_v,   	XK_V,   	XK_kana_HI,	NoSymbol,	/*103*/
	XK_b,   	XK_B,   	XK_kana_KO,	NoSymbol,	/*104*/
	XK_n,   	XK_N,   	XK_kana_MI,	NoSymbol,	/*105*/
	XK_m,   	XK_M,   	XK_kana_MO,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	XK_kana_NE,	XK_kana_comma,	/*107*/
	XK_period,	XK_greater,	XK_kana_RU,	XK_kana_fullstop,	/*108*/
	XK_slash,	XK_question,	XK_kana_ME,	XK_kana_conjunctive,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_backslash,	XK_underscore,	XK_kana_RO,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	XK_Execute,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	XK_Kanji,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	XK_Henkan_Mode,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Japan4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Japan4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Japan4Map     	NULL
#define Japan4maplen 	genericmaplen
#define Japan4KPMap   	NULL
#define Japan4modmap  	NULL

#endif /* JAPAN4 */


#ifdef JAPAN5

static int Japan5maplen = 125 ;

static KeySym Japan5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	XK_kana_NU,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_kana_FU,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_kana_A,	XK_kana_a,	/* 32*/
	XK_4,   	XK_dollar,	XK_kana_U,	XK_kana_u,	/* 33*/
	XK_5,   	XK_percent,	XK_kana_E,	XK_kana_e,	/* 34*/
	XK_6,   	XK_ampersand,	XK_kana_O,	XK_kana_o,	/* 35*/
	XK_7,   	XK_quoteright,	XK_kana_YA,	XK_kana_ya,	/* 36*/
	XK_8,   	XK_parenleft,	XK_kana_YU,	XK_kana_yu,	/* 37*/
	XK_9,   	XK_parenright,	XK_kana_YO,	XK_kana_yo,	/* 38*/
	XK_0,   	XK_0,   	XK_kana_WA,	XK_kana_WO,	/* 39*/
	XK_minus,	XK_equal,	XK_kana_HO,	NoSymbol,	/* 40*/
	XK_asciicircum,	XK_asciitilde,	XK_kana_HE,	NoSymbol,	/* 41*/
	XK_backslash,	XK_bar, 	XK_prolongedsound,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_q,   	XK_Q,   	XK_kana_TA,	NoSymbol,	/* 54*/
	XK_w,   	XK_W,   	XK_kana_TE,	NoSymbol,	/* 55*/
	XK_e,   	XK_E,   	XK_kana_I,	XK_kana_i,	/* 56*/
	XK_r,   	XK_R,   	XK_kana_SU,	NoSymbol,	/* 57*/
	XK_t,   	XK_T,   	XK_kana_KA,	NoSymbol,	/* 58*/
	XK_y,   	XK_Y,   	XK_kana_N,	NoSymbol,	/* 59*/
	XK_u,   	XK_U,   	XK_kana_NA,	NoSymbol,	/* 60*/
	XK_i,   	XK_I,   	XK_kana_NI,	NoSymbol,	/* 61*/
	XK_o,   	XK_O,   	XK_kana_RA,	NoSymbol,	/* 62*/
	XK_p,   	XK_P,   	XK_kana_SE,	NoSymbol,	/* 63*/
	XK_at,  	XK_quoteleft,	XK_voicedsound,	NoSymbol,	/* 64*/
	XK_bracketleft,	XK_braceleft,	XK_semivoicedsound,	XK_kana_openingbracket,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_a,   	XK_A,   	XK_kana_CHI,	NoSymbol,	/* 77*/
	XK_s,   	XK_S,   	XK_kana_TO,	NoSymbol,	/* 78*/
	XK_d,   	XK_D,   	XK_kana_SHI,	NoSymbol,	/* 79*/
	XK_f,   	XK_F,   	XK_kana_HA,	NoSymbol,	/* 80*/
	XK_g,   	XK_G,   	XK_kana_KI,	NoSymbol,	/* 81*/
	XK_h,   	XK_H,   	XK_kana_KU,	NoSymbol,	/* 82*/
	XK_j,   	XK_J,   	XK_kana_MA,	NoSymbol,	/* 83*/
	XK_k,   	XK_K,   	XK_kana_NO,	NoSymbol,	/* 84*/
	XK_l,   	XK_L,   	XK_kana_RI,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_plus,	XK_kana_RE,	NoSymbol,	/* 86*/
	XK_colon,	XK_asterisk,	XK_kana_KE,	NoSymbol,	/* 87*/
	XK_bracketright,	XK_braceright,	XK_kana_MU,	XK_kana_closingbracket,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_z,   	XK_Z,   	XK_kana_TSU,	XK_kana_tsu,	/*100*/
	XK_x,   	XK_X,   	XK_kana_SA,	NoSymbol,	/*101*/
	XK_c,   	XK_C,   	XK_kana_SO,	NoSymbol,	/*102*/
	XK_v,   	XK_V,   	XK_kana_HI,	NoSymbol,	/*103*/
	XK_b,   	XK_B,   	XK_kana_KO,	NoSymbol,	/*104*/
	XK_n,   	XK_N,   	XK_kana_MI,	NoSymbol,	/*105*/
	XK_m,   	XK_M,   	XK_kana_MO,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	XK_kana_NE,	XK_kana_comma,	/*107*/
	XK_period,	XK_greater,	XK_kana_RU,	XK_kana_fullstop,	/*108*/
	XK_slash,	XK_question,	XK_kana_ME,	XK_kana_conjunctive,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_backslash,	XK_underscore,	XK_kana_RO,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	XK_Execute,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	XK_Kanji,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	XK_Henkan_Mode,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Japan5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Japan5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Japan5Map     	NULL
#define Japan5maplen 	genericmaplen
#define Japan5KPMap   	NULL
#define Japan5modmap  	NULL

#endif /* JAPAN5 */


#ifdef KOREA4

static int Korea4maplen = 125 ;

static KeySym Korea4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Korea4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Korea4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	111,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Korea4Map     	NULL
#define Korea4maplen 	genericmaplen
#define Korea4KPMap   	NULL
#define Korea4modmap  	NULL

#endif /* KOREA4 */


#ifdef KOREA5

static int Korea5maplen = 125 ;

static KeySym Korea5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Korea5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Korea5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Korea5Map     	NULL
#define Korea5maplen 	genericmaplen
#define Korea5KPMap   	NULL
#define Korea5modmap  	NULL

#endif /* KOREA5 */


#ifdef NETHERLAND4

static int Netherland4maplen = 125 ;

static KeySym Netherland4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_backslash,	XK_bar, 	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	XK_onesuperior,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_twosuperior,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_threesuperior,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	XK_onequarter,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_onehalf,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_threequarters,	NoSymbol,	/* 35*/
	XK_7,   	XK_underscore,	XK_sterling,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_braceleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_braceright,	NoSymbol,	/* 38*/
	XK_0,   	XK_quoteright,	XK_quoteleft,	NoSymbol,	/* 39*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/* 40*/
	XK_degree,	SunXK_FA_Tilde,	SunXK_FA_Cedilla,	NoSymbol,	/* 41*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Diaeresis,	XK_asciicircum,	SunXK_FA_Circum,	NoSymbol,	/* 64*/
	XK_asterisk,	XK_brokenbar,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	XK_ssharp,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_plus,	XK_plusminus,	NoSymbol,	NoSymbol,	/* 86*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 87*/
	XK_at,  	XK_section,	XK_notsign,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	XK_guillemotleft,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	XK_guillemotright,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	XK_cent,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	XK_mu,  	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_equal,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_bracketright,	XK_bracketleft,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Netherland4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Netherland4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	13,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	119,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Netherland4Map     	NULL
#define Netherland4maplen 	genericmaplen
#define Netherland4KPMap   	NULL
#define Netherland4modmap  	NULL

#endif /* NETHERLAND4 */


#ifdef NETHERLAND5

static int Netherland5maplen = 125 ;

static KeySym Netherland5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	XK_onesuperior,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_twosuperior,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_threesuperior,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	XK_onequarter,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_onehalf,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_threequarters,	NoSymbol,	/* 35*/
	XK_7,   	XK_underscore,	XK_sterling,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_braceleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_braceright,	NoSymbol,	/* 38*/
	XK_0,   	XK_quoteright,	XK_quoteleft,	NoSymbol,	/* 39*/
	XK_slash,	XK_question,	XK_backslash,	NoSymbol,	/* 40*/
	XK_degree,	SunXK_FA_Tilde,	SunXK_FA_Cedilla,	NoSymbol,	/* 41*/
	XK_at,  	XK_section,	XK_notsign,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Separator,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Diaeresis,	SunXK_FA_Circum,	NoSymbol,	NoSymbol,	/* 64*/
	XK_asterisk,	XK_bar, 	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,   	XK_ssharp,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_plus,	XK_plusminus,	NoSymbol,	NoSymbol,	/* 86*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 87*/
	XK_less,	XK_greater,	XK_asciicircum,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_z,   	XK_Z,   	XK_guillemotleft,	NoSymbol,	/*100*/
	XK_x,   	XK_X,   	XK_guillemotright,	NoSymbol,	/*101*/
	XK_c,   	XK_C,   	XK_cent,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_m,   	XK_M,   	XK_mu,  	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	XK_periodcentered,	NoSymbol,	/*108*/
	XK_minus,	XK_equal,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_bracketright,	XK_bracketleft,	XK_brokenbar,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Netherland5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Separator,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Netherland5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Netherland5Map     	NULL
#define Netherland5maplen 	genericmaplen
#define Netherland5KPMap   	NULL
#define Netherland5modmap  	NULL

#endif /* NETHERLAND5 */


#ifdef NORWAY4

static int Norway4maplen = 125 ;

static KeySym Norway4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_asciitilde,	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_currency,	XK_dollar,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_plus,	XK_question,	NoSymbol,	NoSymbol,	/* 40*/
	XK_backslash,	SunXK_FA_Grave,	SunXK_FA_Acute,	NoSymbol,	/* 41*/
	XK_quoteright,	XK_asterisk,	XK_quoteleft,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Aring,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Circum,	SunXK_FA_Tilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Ooblique,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_AE,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_bar, 	XK_section,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Norway4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Norway4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Norway4Map     	NULL
#define Norway4maplen 	genericmaplen
#define Norway4KPMap   	NULL
#define Norway4modmap  	NULL

#endif /* NORWAY4 */


#ifdef NORWAY5

static int Norway5maplen = 125 ;

static KeySym Norway5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_currency,	XK_dollar,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_asciicircum,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_plus,	XK_question,	NoSymbol,	NoSymbol,	/* 40*/
	XK_backslash,	SunXK_FA_Grave,	SunXK_FA_Acute,	NoSymbol,	/* 41*/
	XK_bar, 	XK_section,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Separator,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Aring,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Circum,	SunXK_FA_Tilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Ooblique,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_AE,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_quoteright,	XK_asterisk,	XK_quoteleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Norway5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Separator,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Norway5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Norway5Map     	NULL
#define Norway5maplen 	genericmaplen
#define Norway5KPMap   	NULL
#define Norway5modmap  	NULL

#endif /* NORWAY5 */


#ifdef PORTUGAL4

static int Portugal4maplen = 125 ;

static KeySym Portugal4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_bracketright,	XK_braceright,	XK_guillemotright,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	XK_section,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_notsign,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_backslash,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_bar, 	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	XK_exclamdown,	XK_questiondown,	NoSymbol,	NoSymbol,	/* 41*/
	SunXK_FA_Tilde,	SunXK_FA_Circum,	XK_asciicircum,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Diaeresis,	XK_asterisk,	XK_plus,	NoSymbol,	/* 64*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Ccedilla,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_masculine,	XK_ordfeminine,	NoSymbol,	NoSymbol,	/* 87*/
	XK_bracketleft,	XK_braceleft,	XK_guillemotleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Portugal4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Portugal4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Portugal4Map     	NULL
#define Portugal4maplen 	genericmaplen
#define Portugal4KPMap   	NULL
#define Portugal4modmap  	NULL

#endif /* PORTUGAL4 */


#ifdef PORTUGAL5

static int Portugal5maplen = 125 ;

static KeySym Portugal5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	XK_section,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_asciicircum,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	XK_guillemotleft,	XK_guillemotright,	NoSymbol,	NoSymbol,	/* 41*/
	XK_backslash,	XK_bar, 	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_plus,	XK_asterisk,	SunXK_FA_Diaeresis,	NoSymbol,	/* 64*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Ccedilla,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_masculine,	XK_ordfeminine,	NoSymbol,	NoSymbol,	/* 87*/
	SunXK_FA_Tilde,	SunXK_FA_Circum,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Portugal5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Portugal5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Portugal5Map     	NULL
#define Portugal5maplen 	genericmaplen
#define Portugal5KPMap   	NULL
#define Portugal5modmap  	NULL

#endif /* PORTUGAL5 */


#ifdef SPAIN5

static int Spain5maplen = 125 ;

static KeySym Spain5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	XK_bar, 	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_periodcentered,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	XK_asciicircum,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_notsign,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	NoSymbol,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	XK_exclamdown,	XK_questiondown,	NoSymbol,	NoSymbol,	/* 41*/
	XK_masculine,	XK_ordfeminine,	XK_backslash,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Grave,	SunXK_FA_Circum,	XK_bracketleft,	NoSymbol,	/* 64*/
	XK_plus,	XK_asterisk,	XK_bracketright,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Ntilde,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	SunXK_FA_Acute,	SunXK_FA_Diaeresis,	XK_braceleft,	NoSymbol,	/* 87*/
	XK_ccedilla,	XK_Ccedilla,	XK_braceright,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Spain5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Spain5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Spain5Map     	NULL
#define Spain5maplen 	genericmaplen
#define Spain5KPMap   	NULL
#define Spain5modmap  	NULL

#endif /* SPAIN5 */


#ifdef SPAINLATAM4

static int SpainLatAm4maplen = 125 ;

static KeySym SpainLatAm4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_bracketright,	XK_braceright,	XK_guillemotright,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_periodcentered,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_degree,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_notsign,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_backslash,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_bar, 	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	XK_exclamdown,	XK_questiondown,	NoSymbol,	NoSymbol,	/* 41*/
	XK_Ccedilla,	NoSymbol,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	XK_masculine,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	SunXK_FA_Grave,	SunXK_FA_Circum,	XK_asciicircum,	NoSymbol,	/* 64*/
	XK_plus,	XK_asterisk,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	XK_ordfeminine,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Ntilde,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	SunXK_FA_Acute,	SunXK_FA_Diaeresis,	NoSymbol,	NoSymbol,	/* 87*/
	XK_bracketleft,	XK_braceleft,	XK_guillemotleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec SpainLatAm4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec SpainLatAm4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define SpainLatAm4Map     	NULL
#define SpainLatAm4maplen 	genericmaplen
#define SpainLatAm4KPMap   	NULL
#define SpainLatAm4modmap  	NULL

#endif /* SPAINLATAM4 */


#ifdef SWEDEN5

static int Sweden5maplen = 125 ;

static KeySym Sweden5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_currency,	XK_dollar,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_plus,	XK_question,	XK_backslash,	NoSymbol,	/* 40*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 41*/
	XK_section,	XK_onehalf,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Separator,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Aring,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	XK_asciicircum,	XK_asciitilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Odiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Adiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_quoteright,	XK_asterisk,	XK_quoteleft,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_bar, 	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Sweden5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Separator,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Sweden5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Sweden5Map     	NULL
#define Sweden5maplen 	genericmaplen
#define Sweden5KPMap   	NULL
#define Sweden5modmap  	NULL

#endif /* SWEDEN5 */


#ifdef SWEDENFIN4

static int SwedenFin4maplen = 125 ;

static KeySym SwedenFin4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_asciitilde,	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	XK_sterling,	NoSymbol,	/* 32*/
	XK_4,   	XK_currency,	XK_dollar,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_braceleft,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_bracketleft,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_bracketright,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_braceright,	NoSymbol,	/* 39*/
	XK_plus,	XK_question,	XK_backslash,	NoSymbol,	/* 40*/
	SunXK_FA_Acute,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteright,	XK_asterisk,	XK_quoteleft,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_Aring,	NoSymbol,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Circum,	SunXK_FA_Tilde,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_Odiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 86*/
	XK_Adiaeresis,	NoSymbol,	NoSymbol,	NoSymbol,	/* 87*/
	XK_section,	XK_onehalf,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_bar, 	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec SwedenFin4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec SwedenFin4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	67,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define SwedenFin4Map     	NULL
#define SwedenFin4maplen 	genericmaplen
#define SwedenFin4KPMap   	NULL
#define SwedenFin4modmap  	NULL

#endif /* SWEDENFIN4 */


#ifdef SWITZER_FR4

static int Switzer_Fr4maplen = 125 ;

static KeySym Switzer_Fr4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_greater,	XK_braceright,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_plus,	XK_exclam,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_asterisk,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_ccedilla,	XK_cent,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_section,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_bar, 	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_degree,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_backslash,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_asciicircum,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	SunXK_FA_Circum,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 41*/
	XK_dollar,	SunXK_FA_Tilde,	XK_sterling,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_egrave,	XK_udiaeresis,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Acute,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_eacute,	XK_odiaeresis,	NoSymbol,	NoSymbol,	/* 86*/
	XK_agrave,	XK_adiaeresis,	NoSymbol,	NoSymbol,	/* 87*/
	XK_less,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	XK_mu,  	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_bracketright,	XK_bracketleft,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Switzer_Fr4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Switzer_Fr4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	67,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Switzer_Fr4Map     	NULL
#define Switzer_Fr4maplen 	genericmaplen
#define Switzer_Fr4KPMap   	NULL
#define Switzer_Fr4modmap  	NULL

#endif /* SWITZER_FR4 */


#ifdef SWITZER_FR5

static int Switzer_Fr5maplen = 125 ;

static KeySym Switzer_Fr5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_plus,	XK_bar, 	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_asterisk,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_ccedilla,	XK_asciicircum,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_quoteleft,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	SunXK_FA_Acute,	NoSymbol,	/* 40*/
	SunXK_FA_Circum,	SunXK_FA_Grave,	SunXK_FA_Tilde,	NoSymbol,	/* 41*/
	XK_section,	XK_degree,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_egrave,	XK_udiaeresis,	XK_bracketleft,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	XK_exclam,	XK_bracketright,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_eacute,	XK_odiaeresis,	NoSymbol,	NoSymbol,	/* 86*/
	XK_agrave,	XK_adiaeresis,	XK_braceleft,	NoSymbol,	/* 87*/
	XK_dollar,	XK_sterling,	XK_braceright,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_backslash,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Switzer_Fr5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Switzer_Fr5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Switzer_Fr5Map     	NULL
#define Switzer_Fr5maplen 	genericmaplen
#define Switzer_Fr5KPMap   	NULL
#define Switzer_Fr5modmap  	NULL

#endif /* SWITZER_FR5 */


#ifdef SWITZER_GE4

static int Switzer_Ge4maplen = 125 ;

static KeySym Switzer_Ge4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	XK_greater,	XK_braceright,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_plus,	XK_exclam,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_asterisk,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_ccedilla,	XK_cent,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	XK_section,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	XK_bar, 	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	XK_degree,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	XK_backslash,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_asciicircum,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	XK_quoteleft,	NoSymbol,	/* 40*/
	SunXK_FA_Circum,	SunXK_FA_Grave,	NoSymbol,	NoSymbol,	/* 41*/
	XK_dollar,	SunXK_FA_Tilde,	XK_sterling,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_udiaeresis,	XK_egrave,	NoSymbol,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	SunXK_FA_Acute,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_odiaeresis,	XK_eacute,	NoSymbol,	NoSymbol,	/* 86*/
	XK_adiaeresis,	XK_agrave,	NoSymbol,	NoSymbol,	/* 87*/
	XK_less,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	XK_mu,  	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_bracketright,	XK_bracketleft,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Switzer_Ge4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Switzer_Ge4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	76,	(LockMask),
	119,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	67,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Switzer_Ge4Map     	NULL
#define Switzer_Ge4maplen 	genericmaplen
#define Switzer_Ge4KPMap   	NULL
#define Switzer_Ge4modmap  	NULL

#endif /* SWITZER_GE4 */


#ifdef SWITZER_GE5

static int Switzer_Ge5maplen = 125 ;

static KeySym Switzer_Ge5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_plus,	XK_bar, 	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	XK_at,  	NoSymbol,	/* 31*/
	XK_3,   	XK_asterisk,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_ccedilla,	XK_asciicircum,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	XK_asciitilde,	NoSymbol,	/* 34*/
	XK_6,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_slash,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_equal,	XK_quoteleft,	NoSymbol,	/* 39*/
	XK_quoteright,	XK_question,	SunXK_FA_Acute,	NoSymbol,	/* 40*/
	SunXK_FA_Circum,	SunXK_FA_Grave,	SunXK_FA_Tilde,	NoSymbol,	/* 41*/
	XK_section,	XK_degree,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_udiaeresis,	XK_egrave,	XK_bracketleft,	NoSymbol,	/* 64*/
	SunXK_FA_Diaeresis,	XK_exclam,	XK_bracketright,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_odiaeresis,	XK_eacute,	NoSymbol,	NoSymbol,	/* 86*/
	XK_adiaeresis,	XK_agrave,	XK_braceleft,	NoSymbol,	/* 87*/
	XK_dollar,	XK_sterling,	XK_braceright,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_semicolon,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_colon,	NoSymbol,	NoSymbol,	/*108*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_less,	XK_greater,	XK_backslash,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Switzer_Ge5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Switzer_Ge5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Switzer_Ge5Map     	NULL
#define Switzer_Ge5maplen 	genericmaplen
#define Switzer_Ge5KPMap   	NULL
#define Switzer_Ge5modmap  	NULL

#endif /* SWITZER_GE5 */


#ifdef TAIWAN4

static int Taiwan4maplen = 125 ;

static KeySym Taiwan4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Taiwan4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Taiwan4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Taiwan4Map     	NULL
#define Taiwan4maplen 	genericmaplen
#define Taiwan4KPMap   	NULL
#define Taiwan4modmap  	NULL

#endif /* TAIWAN4 */


#ifdef TAIWAN5

static int Taiwan5maplen = 125 ;

static KeySym Taiwan5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec Taiwan5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec Taiwan5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define Taiwan5Map     	NULL
#define Taiwan5maplen 	genericmaplen
#define Taiwan5KPMap   	NULL
#define Taiwan5modmap  	NULL

#endif /* TAIWAN5 */


#ifdef UK4

static int UK4maplen = 125 ;

static KeySym UK4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	XK_brokenbar,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_sterling,	XK_numbersign,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	XK_notsign,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec UK4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec UK4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define UK4Map     	NULL
#define UK4maplen 	genericmaplen
#define UK4KPMap   	NULL
#define UK4modmap  	NULL

#endif /* UK4 */


#ifdef UK5

static int UK5maplen = 125 ;

static KeySym UK5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_sterling,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_notsign,	XK_brokenbar,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_at,  	NoSymbol,	NoSymbol,	/* 87*/
	XK_numbersign,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	XK_backslash,	XK_bar, 	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec UK5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec UK5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define UK5Map     	NULL
#define UK5maplen 	genericmaplen
#define UK5KPMap   	NULL
#define UK5modmap  	NULL

#endif /* UK5 */


#ifdef US101A_SUN

static int US101A_Sunmaplen = 125 ;

static KeySym US101A_SunMap[] = {
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 20*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 24*/
	XK_Insert,	XK_L1,  	SunXK_Stop,	NoSymbol,	/* 25*/
	XK_End, 	XK_L5,  	SunXK_Front,	NoSymbol,	/* 26*/
	XK_Down,	XK_L9,  	SunXK_Find,	NoSymbol,	/* 27*/
	XK_Right,	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_Prior,	XK_L3,  	SunXK_Props,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 72*/
	XK_Next,	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	NoSymbol,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_Home,	XK_L2,  	SunXK_Again,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Delete,	XK_L4,  	SunXK_Undo,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec US101A_SunKPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec US101A_Sunmodmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	122,	(Mod1Mask),
	120,	(Mod3Mask),
	0,	0
};

#else

#define US101A_SunMap     	NULL
#define US101A_Sunmaplen 	genericmaplen
#define US101A_SunKPMap   	NULL
#define US101A_Sunmodmap  	NULL

#endif /* US101A_SUN */


#ifdef US4

static int US4maplen = 125 ;

static KeySym US4Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	NoSymbol,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	NoSymbol,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	XK_Break,	/* 23*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	XK_R4,  	XK_R4,  	XK_KP_Equal,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_KP_Subtract,	NoSymbol,	NoSymbol,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	XK_Linefeed,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec US4KPMap[] = {
	45,	XK_KP_Equal,	NoSymbol,
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec US4modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define US4Map     	NULL
#define US4maplen 	genericmaplen
#define US4KPMap   	NULL
#define US4modmap  	NULL

#endif /* US4 */


#ifdef US5

static int US5maplen = 125 ;

static KeySym US5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec US5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec US5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define US5Map     	NULL
#define US5maplen 	genericmaplen
#define US5KPMap   	NULL
#define US5modmap  	NULL

#endif /* US5 */


#ifdef US_UNIX5

static int US_UNIX5maplen = 125 ;

static KeySym US_UNIX5Map[] = {
	XK_L1,  	XK_L1,  	SunXK_Stop,	NoSymbol,	/*  1*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  2*/
	XK_L2,  	XK_L2,  	SunXK_Again,	NoSymbol,	/*  3*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*  4*/
	XK_F1,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  5*/
	XK_F2,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  6*/
	XK_F10, 	NoSymbol,	NoSymbol,	NoSymbol,	/*  7*/
	XK_F3,  	NoSymbol,	NoSymbol,	NoSymbol,	/*  8*/
	SunXK_F36,	NoSymbol,	NoSymbol,	NoSymbol,	/*  9*/
	XK_F4,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 10*/
	SunXK_F37,	NoSymbol,	NoSymbol,	NoSymbol,	/* 11*/
	XK_F5,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 12*/
	SunXK_AltGraph,	NoSymbol,	NoSymbol,	NoSymbol,	/* 13*/
	XK_F6,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 14*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 15*/
	XK_F7,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 16*/
	XK_F8,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 17*/
	XK_F9,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 18*/
	XK_Alt_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 19*/
	XK_Up,  	NoSymbol,	NoSymbol,	NoSymbol,	/* 20*/
	XK_R1,  	XK_R1,  	XK_Pause,	XK_Break,	/* 21*/
	XK_R2,  	XK_R2,  	XK_Print,	SunXK_Sys_Req,	/* 22*/
	XK_R3,  	XK_R3,  	XK_Scroll_Lock,	NoSymbol,	/* 23*/
	XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,	/* 24*/
	XK_L3,  	XK_L3,  	SunXK_Props,	NoSymbol,	/* 25*/
	XK_L4,  	XK_L4,  	SunXK_Undo,	NoSymbol,	/* 26*/
	XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,	/* 27*/
	XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,	/* 28*/
	XK_Escape,	NoSymbol,	NoSymbol,	NoSymbol,	/* 29*/
	XK_1,   	XK_exclam,	NoSymbol,	NoSymbol,	/* 30*/
	XK_2,   	XK_at,  	NoSymbol,	NoSymbol,	/* 31*/
	XK_3,   	XK_numbersign,	NoSymbol,	NoSymbol,	/* 32*/
	XK_4,   	XK_dollar,	NoSymbol,	NoSymbol,	/* 33*/
	XK_5,   	XK_percent,	NoSymbol,	NoSymbol,	/* 34*/
	XK_6,   	XK_asciicircum,	NoSymbol,	NoSymbol,	/* 35*/
	XK_7,   	XK_ampersand,	NoSymbol,	NoSymbol,	/* 36*/
	XK_8,   	XK_asterisk,	NoSymbol,	NoSymbol,	/* 37*/
	XK_9,   	XK_parenleft,	NoSymbol,	NoSymbol,	/* 38*/
	XK_0,   	XK_parenright,	NoSymbol,	NoSymbol,	/* 39*/
	XK_minus,	XK_underscore,	NoSymbol,	NoSymbol,	/* 40*/
	XK_equal,	XK_plus,	NoSymbol,	NoSymbol,	/* 41*/
	XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,	/* 42*/
	XK_BackSpace,	NoSymbol,	NoSymbol,	NoSymbol,	/* 43*/
	XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,	/* 44*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 45*/
	XK_R5,  	XK_R5,  	XK_KP_Divide,	NoSymbol,	/* 46*/
	XK_R6,  	XK_R6,  	XK_KP_Multiply,	NoSymbol,	/* 47*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 48*/
	XK_L5,  	XK_L5,  	SunXK_Front,	NoSymbol,	/* 49*/
	XK_Delete,	XK_Delete,	XK_KP_Decimal,	NoSymbol,	/* 50*/
	XK_L6,  	XK_L6,  	SunXK_Copy,	NoSymbol,	/* 51*/
	XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,	/* 52*/
	XK_Tab, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 53*/
	XK_Q,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 54*/
	XK_W,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 55*/
	XK_E,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 56*/
	XK_R,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 57*/
	XK_T,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 58*/
	XK_Y,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 59*/
	XK_U,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 60*/
	XK_I,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 61*/
	XK_O,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 62*/
	XK_P,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 63*/
	XK_bracketleft,	XK_braceleft,	NoSymbol,	NoSymbol,	/* 64*/
	XK_bracketright,	XK_braceright,	NoSymbol,	NoSymbol,	/* 65*/
	XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,	/* 66*/
	SunXK_Compose,	NoSymbol,	NoSymbol,	NoSymbol,	/* 67*/
	XK_R7,  	XK_R7,  	XK_KP_7,	XK_Home,	/* 68*/
	XK_Up,  	XK_R8,  	XK_KP_8,	NoSymbol,	/* 69*/
	XK_R9,  	XK_R9,  	XK_KP_9,	XK_Prior,	/* 70*/
	XK_R4,  	XK_R4,  	XK_KP_Subtract,	NoSymbol,	/* 71*/
	XK_L7,  	XK_L7,  	SunXK_Open,	NoSymbol,	/* 72*/
	XK_L8,  	XK_L8,  	SunXK_Paste,	NoSymbol,	/* 73*/
	XK_End, 	NoSymbol,	NoSymbol,	NoSymbol,	/* 74*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/* 75*/
	XK_Control_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 76*/
	XK_A,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 77*/
	XK_S,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 78*/
	XK_D,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 79*/
	XK_F,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 80*/
	XK_G,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 81*/
	XK_H,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 82*/
	XK_J,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 83*/
	XK_K,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 84*/
	XK_L,   	NoSymbol,	NoSymbol,	NoSymbol,	/* 85*/
	XK_semicolon,	XK_colon,	NoSymbol,	NoSymbol,	/* 86*/
	XK_quoteright,	XK_quotedbl,	NoSymbol,	NoSymbol,	/* 87*/
	XK_backslash,	XK_bar, 	XK_brokenbar,	NoSymbol,	/* 88*/
	XK_Return,	NoSymbol,	NoSymbol,	NoSymbol,	/* 89*/
	XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,	/* 90*/
	XK_Left,	XK_R10, 	XK_KP_4,	NoSymbol,	/* 91*/
	XK_R11, 	XK_R11, 	XK_KP_5,	NoSymbol,	/* 92*/
	XK_Right,	XK_R12, 	XK_KP_6,	NoSymbol,	/* 93*/
	XK_Insert,	XK_Insert,	XK_KP_0,	NoSymbol,	/* 94*/
	XK_L9,  	XK_L9,  	SunXK_Find,	NoSymbol,	/* 95*/
	XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,	/* 96*/
	XK_L10, 	XK_L10, 	SunXK_Cut,	NoSymbol,	/* 97*/
	XK_Num_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/* 98*/
	XK_Shift_L,	NoSymbol,	NoSymbol,	NoSymbol,	/* 99*/
	XK_Z,   	NoSymbol,	NoSymbol,	NoSymbol,	/*100*/
	XK_X,   	NoSymbol,	NoSymbol,	NoSymbol,	/*101*/
	XK_C,   	NoSymbol,	NoSymbol,	NoSymbol,	/*102*/
	XK_V,   	NoSymbol,	NoSymbol,	NoSymbol,	/*103*/
	XK_B,   	NoSymbol,	NoSymbol,	NoSymbol,	/*104*/
	XK_N,   	NoSymbol,	NoSymbol,	NoSymbol,	/*105*/
	XK_M,   	NoSymbol,	NoSymbol,	NoSymbol,	/*106*/
	XK_comma,	XK_less,	NoSymbol,	NoSymbol,	/*107*/
	XK_period,	XK_greater,	NoSymbol,	NoSymbol,	/*108*/
	XK_slash,	XK_question,	NoSymbol,	NoSymbol,	/*109*/
	XK_Shift_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*110*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*111*/
	XK_R13, 	XK_R13, 	XK_KP_1,	XK_End, 	/*112*/
	XK_Down,	XK_R14, 	XK_KP_2,	NoSymbol,	/*113*/
	XK_R15, 	XK_R15, 	XK_KP_3,	XK_Next,	/*114*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*115*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*116*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*117*/
	XK_Help,	NoSymbol,	NoSymbol,	NoSymbol,	/*118*/
	XK_Caps_Lock,	NoSymbol,	NoSymbol,	NoSymbol,	/*119*/
	XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,	/*120*/
	XK_space,	NoSymbol,	NoSymbol,	NoSymbol,	/*121*/
	XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,	/*122*/
	XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,	/*123*/
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,	/*124*/
	XK_KP_Add,	NoSymbol,	NoSymbol,	NoSymbol,	/*125*/
};

static SunKPmapRec US_UNIX5KPMap[] = {
	46,	XK_KP_Divide,	NoSymbol,
	47,	XK_KP_Multiply,	NoSymbol,
	50,	XK_KP_Decimal,	NoSymbol,
	68,	XK_KP_7,	XK_Home,
	69,	XK_KP_8,	NoSymbol,
	70,	XK_KP_9,	XK_Prior,
	71,	XK_KP_Subtract,	NoSymbol,
	90,	XK_KP_Enter,	NoSymbol,
	91,	XK_KP_4,	NoSymbol,
	92,	XK_KP_5,	NoSymbol,
	93,	XK_KP_6,	NoSymbol,
	94,	XK_KP_0,	NoSymbol,
	112,	XK_KP_1,	XK_End,
	113,	XK_KP_2,	NoSymbol,
	114,	XK_KP_3,	XK_Next,
	125,	XK_KP_Add,	NoSymbol,
	0,	0,		0
};

static SunModmapRec US_UNIX5modmap[] = {
	99,	(ShiftMask),
	110,	(ShiftMask),
	119,	(LockMask),
	76,	(ControlMask),
	120,	(Mod1Mask),
	122,	(Mod1Mask),
	13,	(Mod2Mask),
	19,	(Mod3Mask),
	0,	0
};

#else

#define US_UNIX5Map     	NULL
#define US_UNIX5maplen 		genericmaplen
#define US_UNIX5KPMap   	NULL
#define US_UNIX5modmap  	NULL

#endif /* US_UNIX5 */

KeySym *sunType4Maps[] = {
	US4Map,    		/* 0x00 */
	US4Map,    		/* 0x01 */
	FranceBelg4Map,		/* 0x02 */
	Canada4Map,		/* 0x03 */
	Denmark4Map,		/* 0x04 */
	Germany4Map,		/* 0x05 */
	Italy4Map,    		/* 0x06 */
	Netherland4Map,		/* 0x07 */
	Norway4Map,		/* 0x08 */
	Portugal4Map,		/* 0x09 */
	SpainLatAm4Map,		/* 0x0a */
	SwedenFin4Map,		/* 0x0b */
	Switzer_Fr4Map,		/* 0x0c */
	Switzer_Ge4Map,		/* 0x0d */
	UK4Map,    		/* 0x0e */
	NULL,    		/* 0x0f */
	Korea4Map,    		/* 0x10 */
	Taiwan4Map,		/* 0x11 */
	NULL,    		/* 0x02 */
	US101A_SunMap,		/* 0x13 */
	NULL,    		/* 0x14 */
	NULL,    		/* 0x15 */
	NULL,    		/* 0x16 */
	NULL,    		/* 0x17 */
	NULL,    		/* 0x18 */
	NULL,    		/* 0x19 */
	NULL,    		/* 0x1a */
	NULL,    		/* 0x1b */
	NULL,    		/* 0x1c */
	NULL,    		/* 0x1d */
	NULL,    		/* 0x1e */
	NULL,    		/* 0x1f */
	Japan4Map,    		/* 0x20 */
	US5Map,    		/* 0x21 */
	US_UNIX5Map,		/* 0x22 */
	France5Map,		/* 0x23 */
	Denmark5Map,		/* 0x24 */
	Germany5Map,		/* 0x25 */
	Italy5Map,    		/* 0x26 */
	Netherland5Map,		/* 0x27 */
	Norway5Map,		/* 0x28 */
	Portugal5Map,		/* 0x29 */
	Spain5Map,    		/* 0x2a */
	Sweden5Map,		/* 0x2b */
	Switzer_Fr5Map,		/* 0x2c */
	Switzer_Ge5Map,		/* 0x2d */
	UK5Map,    		/* 0x2e */
	Korea5Map,    		/* 0x2f */
	Taiwan5Map,		/* 0x30 */
	Japan5Map,    		/* 0x31 */
};

int sunType4MaxLayout = 0x31;

int *sunType4MapLen[] = {
	&US4maplen,    		/* 0x00 */
	&US4maplen,    		/* 0x01 */
	&FranceBelg4maplen,	/* 0x02 */
	&Canada4maplen,		/* 0x03 */
	&Denmark4maplen,	/* 0x04 */
	&Germany4maplen,	/* 0x05 */
	&Italy4maplen,    	/* 0x06 */
	&Netherland4maplen,	/* 0x07 */
	&Norway4maplen,		/* 0x08 */
	&Portugal4maplen,	/* 0x09 */
	&SpainLatAm4maplen,	/* 0x0a */
	&SwedenFin4maplen,	/* 0x0b */
	&Switzer_Fr4maplen,	/* 0x0c */
	&Switzer_Ge4maplen,	/* 0x0d */
	&UK4maplen,    		/* 0x0e */
	NULL,    		/* 0x0f */
	&Korea4maplen,    	/* 0x10 */
	&Taiwan4maplen,		/* 0x11 */
	NULL,    		/* 0x02 */
	&US101A_Sunmaplen,	/* 0x13 */
	NULL,    		/* 0x14 */
	NULL,    		/* 0x15 */
	NULL,    		/* 0x16 */
	NULL,    		/* 0x17 */
	NULL,    		/* 0x18 */
	NULL,    		/* 0x19 */
	NULL,    		/* 0x1a */
	NULL,    		/* 0x1b */
	NULL,    		/* 0x1c */
	NULL,    		/* 0x1d */
	NULL,    		/* 0x1e */
	NULL,    		/* 0x1f */
	&Japan4maplen,    	/* 0x20 */
	&US5maplen,    		/* 0x21 */
	&US_UNIX5maplen,	/* 0x22 */
	&France5maplen,		/* 0x23 */
	&Denmark5maplen,	/* 0x24 */
	&Germany5maplen,	/* 0x25 */
	&Italy5maplen,    	/* 0x26 */
	&Netherland5maplen,	/* 0x27 */
	&Norway5maplen,		/* 0x28 */
	&Portugal5maplen,	/* 0x29 */
	&Spain5maplen,    	/* 0x2a */
	&Sweden5maplen,		/* 0x2b */
	&Switzer_Fr5maplen,	/* 0x2c */
	&Switzer_Ge5maplen,	/* 0x2d */
	&UK5maplen,    		/* 0x2e */
	&Korea5maplen,    	/* 0x2f */
	&Taiwan5maplen,		/* 0x30 */
	&Japan5maplen,    	/* 0x31 */
};

SunKPmapRec *sunType4KPMaps[] = {
	US4KPMap,    		/* 0x00 */
	US4KPMap,    		/* 0x01 */
	FranceBelg4KPMap,	/* 0x02 */
	Canada4KPMap,		/* 0x03 */
	Denmark4KPMap,		/* 0x04 */
	Germany4KPMap,		/* 0x05 */
	Italy4KPMap,    	/* 0x06 */
	Netherland4KPMap,	/* 0x07 */
	Norway4KPMap,		/* 0x08 */
	Portugal4KPMap,		/* 0x09 */
	SpainLatAm4KPMap,	/* 0x0a */
	SwedenFin4KPMap,	/* 0x0b */
	Switzer_Fr4KPMap,	/* 0x0c */
	Switzer_Ge4KPMap,	/* 0x0d */
	UK4KPMap,    		/* 0x0e */
	NULL,    		/* 0x0f */
	Korea4KPMap,    	/* 0x10 */
	Taiwan4KPMap,		/* 0x11 */
	NULL,    		/* 0x02 */
	US101A_SunKPMap,	/* 0x13 */
	NULL,    		/* 0x14 */
	NULL,    		/* 0x15 */
	NULL,    		/* 0x16 */
	NULL,    		/* 0x17 */
	NULL,    		/* 0x18 */
	NULL,    		/* 0x19 */
	NULL,    		/* 0x1a */
	NULL,    		/* 0x1b */
	NULL,    		/* 0x1c */
	NULL,    		/* 0x1d */
	NULL,    		/* 0x1e */
	NULL,    		/* 0x1f */
	Japan4KPMap,    	/* 0x20 */
	US5KPMap,    		/* 0x21 */
	US_UNIX5KPMap,		/* 0x22 */
	France5KPMap,		/* 0x23 */
	Denmark5KPMap,		/* 0x24 */
	Germany5KPMap,		/* 0x25 */
	Italy5KPMap,    	/* 0x26 */
	Netherland5KPMap,	/* 0x27 */
	Norway5KPMap,		/* 0x28 */
	Portugal5KPMap,		/* 0x29 */
	Spain5KPMap,    	/* 0x2a */
	Sweden5KPMap,		/* 0x2b */
	Switzer_Fr5KPMap,	/* 0x2c */
	Switzer_Ge5KPMap,	/* 0x2d */
	UK5KPMap,    		/* 0x2e */
	Korea5KPMap,    	/* 0x2f */
	Taiwan5KPMap,		/* 0x30 */
	Japan5KPMap,    	/* 0x31 */
};

SunModmapRec *sunType4ModMaps[] = {
	US4modmap,    		/* 0x00 */
	US4modmap,    		/* 0x01 */
	FranceBelg4modmap,	/* 0x02 */
	Canada4modmap,		/* 0x03 */
	Denmark4modmap,		/* 0x04 */
	Germany4modmap,		/* 0x05 */
	Italy4modmap,    	/* 0x06 */
	Netherland4modmap,	/* 0x07 */
	Norway4modmap,		/* 0x08 */
	Portugal4modmap,	/* 0x09 */
	SpainLatAm4modmap,	/* 0x0a */
	SwedenFin4modmap,	/* 0x0b */
	Switzer_Fr4modmap,	/* 0x0c */
	Switzer_Ge4modmap,	/* 0x0d */
	UK4modmap,    		/* 0x0e */
	NULL,    		/* 0x0f */
	Korea4modmap,    	/* 0x10 */
	Taiwan4modmap,		/* 0x11 */
	NULL,    		/* 0x02 */
	US101A_Sunmodmap,	/* 0x13 */
	NULL,    		/* 0x14 */
	NULL,    		/* 0x15 */
	NULL,    		/* 0x16 */
	NULL,    		/* 0x17 */
	NULL,    		/* 0x18 */
	NULL,    		/* 0x19 */
	NULL,    		/* 0x1a */
	NULL,    		/* 0x1b */
	NULL,    		/* 0x1c */
	NULL,    		/* 0x1d */
	NULL,    		/* 0x1e */
	NULL,    		/* 0x1f */
	Japan4modmap,    	/* 0x20 */
	US5modmap,    		/* 0x21 */
	US_UNIX5modmap,		/* 0x22 */
	France5modmap,		/* 0x23 */
	Denmark5modmap,		/* 0x24 */
	Germany5modmap,		/* 0x25 */
	Italy5modmap,    	/* 0x26 */
	Netherland5modmap,	/* 0x27 */
	Norway5modmap,		/* 0x28 */
	Portugal5modmap,	/* 0x29 */
	Spain5modmap,    	/* 0x2a */
	Sweden5modmap,		/* 0x2b */
	Switzer_Fr5modmap,	/* 0x2c */
	Switzer_Ge5modmap,	/* 0x2d */
	UK5modmap,    		/* 0x2e */
	Korea5modmap,    	/* 0x2f */
	Taiwan5modmap,		/* 0x30 */
	Japan5modmap,    	/* 0x31 */
};
