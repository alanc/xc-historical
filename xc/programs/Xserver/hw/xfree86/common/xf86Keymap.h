/* $XConsortium: xf86Keymap.h,v 1.6 95/01/10 16:00:59 kaleb Exp kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Keymap.h,v 3.1 1994/12/11 10:54:42 dawes Exp $ */
/*
 * For Scancodes see notes in atKeynames.h  !!!!
 */
static KeySym map[NUM_KEYCODES * GLYPHS_PER_KEY] = { 
    /* 0x00 */  NoSymbol,       NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x01 */  XK_Escape,      NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x02 */  XK_1,           XK_exclam,	NoSymbol,	NoSymbol,
    /* 0x03 */  XK_2,           XK_at,		NoSymbol,	NoSymbol,
    /* 0x04 */  XK_3,           XK_numbersign,	NoSymbol,	NoSymbol,
    /* 0x05 */  XK_4,           XK_dollar,	NoSymbol,	NoSymbol,
    /* 0x06 */  XK_5,           XK_percent,	NoSymbol,	NoSymbol,
    /* 0x07 */  XK_6,           XK_asciicircum,	NoSymbol,	NoSymbol,
    /* 0x08 */  XK_7,           XK_ampersand,	NoSymbol,	NoSymbol,
    /* 0x09 */  XK_8,           XK_asterisk,	NoSymbol,	NoSymbol,
    /* 0x0a */  XK_9,           XK_parenleft,	NoSymbol,	NoSymbol,
    /* 0x0b */  XK_0,           XK_parenright,	NoSymbol,	NoSymbol,
    /* 0x0c */  XK_minus,       XK_underscore,	NoSymbol,	NoSymbol,
    /* 0x0d */  XK_equal,       XK_plus,	NoSymbol,	NoSymbol,
    /* 0x0e */  XK_BackSpace,   NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x0f */  XK_Tab,         NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x10 */  XK_Q,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x11 */  XK_W,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x12 */  XK_E,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x13 */  XK_R,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x14 */  XK_T,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x15 */  XK_Y,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x16 */  XK_U,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x17 */  XK_I,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x18 */  XK_O,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x19 */  XK_P,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1a */  XK_bracketleft, XK_braceleft,	NoSymbol,	NoSymbol,
    /* 0x1b */  XK_bracketright,XK_braceright,	NoSymbol,	NoSymbol,
    /* 0x1c */  XK_Return,      NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1d */  XK_Control_L,   NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1e */  XK_A,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1f */  XK_S,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x20 */  XK_D,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x21 */  XK_F,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x22 */  XK_G,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x23 */  XK_H,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x24 */  XK_J,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x25 */  XK_K,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x26 */  XK_L,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x27 */  XK_semicolon,   XK_colon,	NoSymbol,	NoSymbol,
    /* 0x28 */  XK_quoteright,  XK_quotedbl,	NoSymbol,	NoSymbol,
    /* 0x29 */  XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,
    /* 0x2a */  XK_Shift_L,     NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2b */  XK_backslash,   XK_bar,		NoSymbol,	NoSymbol,
    /* 0x2c */  XK_Z,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2d */  XK_X,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2e */  XK_C,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2f */  XK_V,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x30 */  XK_B,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x31 */  XK_N,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x32 */  XK_M,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x33 */  XK_comma,       XK_less,	NoSymbol,	NoSymbol,
    /* 0x34 */  XK_period,      XK_greater,	NoSymbol,	NoSymbol,
    /* 0x35 */  XK_slash,       XK_question,	NoSymbol,	NoSymbol,
    /* 0x36 */  XK_Shift_R,     NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x37 */  XK_KP_Multiply, NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x38 */  XK_Alt_L,	XK_Meta_L,	NoSymbol,	NoSymbol,
    /* 0x39 */  XK_space,       NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3a */  XK_Caps_Lock,   NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3b */  XK_F1,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3c */  XK_F2,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3d */  XK_F3,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3e */  XK_F4,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3f */  XK_F5,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x40 */  XK_F6,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x41 */  XK_F7,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x42 */  XK_F8,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x43 */  XK_F9,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x44 */  XK_F10,         NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x45 */  XK_Num_Lock,    NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x46 */  XK_Scroll_Lock,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x47 */  XK_KP_Home,	XK_KP_7,	NoSymbol,	NoSymbol,
    /* 0x48 */  XK_KP_Up,	XK_KP_8,	NoSymbol,	NoSymbol,
    /* 0x49 */  XK_KP_Prior,	XK_KP_9,	NoSymbol,	NoSymbol,
    /* 0x4a */  XK_KP_Subtract, NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x4b */  XK_KP_Left,	XK_KP_4,	NoSymbol,	NoSymbol,
    /* 0x4c */  NoSymbol,	XK_KP_5,	NoSymbol,	NoSymbol,
    /* 0x4d */  XK_KP_Right,	XK_KP_6,	NoSymbol,	NoSymbol,
    /* 0x4e */  XK_KP_Add,      NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x4f */  XK_KP_End,	XK_KP_1,	NoSymbol,	NoSymbol,
    /* 0x50 */  XK_KP_Down,	XK_KP_2,	NoSymbol,	NoSymbol,
    /* 0x51 */  XK_KP_Next,	XK_KP_3,	NoSymbol,	NoSymbol,
    /* 0x52 */  XK_KP_Insert,	XK_KP_0,	NoSymbol,	NoSymbol,
    /* 0x53 */  XK_KP_Delete,	XK_KP_Decimal,	NoSymbol,	NoSymbol,
    /* 0x54 */  XK_Sys_Req,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x55 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x56 */  XK_less,	XK_greater,	NoSymbol,	NoSymbol,
    /* 0x57 */  XK_F11,		NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x58 */  XK_F12,		NoSymbol,	NoSymbol,	NoSymbol,

    /* 0x59 */  XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5a */  XK_Up,		NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5b */  XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5c */  XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5d */  XK_Begin,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5e */  XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5f */  XK_End,		NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x60 */  XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x61 */  XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x62 */  XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x63 */  XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x64 */  XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x65 */  XK_Control_R,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x66 */  XK_Pause,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x67 */  XK_Print,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x68 */  XK_KP_Divide,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x69 */  XK_Alt_R,	XK_Meta_R,	NoSymbol,	NoSymbol,
    /* 0x6a */  XK_Break,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x6b */  XK_Meta_L,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x6c */  XK_Meta_R,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x6d */  XK_Menu,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x6e */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x6f */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x70 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x71 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x72 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x73 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x74 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x75 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x76 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x77 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x78 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x79 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x7a */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x7b */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x7c */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x7d */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x7e */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x7f */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,

    /* These are for ServerNumLock handling */
    /* 0x80 */  XK_KP_7,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x81 */  XK_KP_8,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x82 */  XK_KP_9,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x83 */  XK_KP_4,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x84 */  XK_KP_5,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x85 */  XK_KP_6,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x86 */  XK_KP_1,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x87 */  XK_KP_2,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x88 */  XK_KP_3,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x89 */  XK_KP_0,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x8a */  XK_KP_Decimal,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x8b */  XK_KP_Home,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x8c */  XK_KP_Up,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x8d */  XK_KP_Prior,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x8e */  XK_KP_Left,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x8f */  XK_KP_Begin,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x90 */  XK_KP_Right,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x91 */  XK_KP_End,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x92 */  XK_KP_Down,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x93 */  XK_KP_Next,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x94 */  XK_KP_Insert,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x95 */  XK_KP_Delete,	NoSymbol,	NoSymbol,	NoSymbol,
};
