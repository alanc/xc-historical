/* $XConsortium: xf86_Config.h,v 1.2 94/10/12 20:33:21 kaleb Exp kaleb $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86_Config.h,v 3.17 1994/11/30 20:41:19 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Dawes <dawes@physics.su.oz.au>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Dawes 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Thomas Roell and
 * David Dawes makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THOMAS ROELL AND DAVID DAWES DISCLAIMS ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR DAVID DAWES BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#ifndef XCONFIG_FLAGS_ONLY

#define CLOCK_TOLERANCE 2000         /* Clock matching tolerance (2MHz) */

#ifdef BLACK
#undef BLACK
#endif
#ifdef WHITE
#undef WHITE
#endif
#ifdef SCROLLLOCK
#undef SCROLLLOCK
#endif
#ifdef MONO
#undef MONO
#endif

typedef struct {
  int           num;                  /* returned number */
  char          *str;                 /* private copy of the return-string */
  double        realnum;              /* returned number as a real */
} LexRec, *LexPtr;

/* Monitor and device records. */
#define MAX_HSYNC 8
#define MAX_VREFRESH 8

typedef struct { float hi, lo; } range ;

typedef struct {
   char *id;
   char *vendor;
   char *model;
   float bandwidth;
   int n_hsync;
   range hsync[MAX_HSYNC];
   int n_vrefresh;
   range vrefresh[MAX_VREFRESH];
   DisplayModePtr Modes, Last; /* Start and end of monitor's mode list */
} MonRec, *MonPtr ;

typedef struct {
   char *identifier;
   char *vendor;
   char *board;
   char *chipset;
   char *ramdac;
   int dacSpeed;
   int clocks;
   int clock[MAXCLOCKS];
   OFlagSet options;
   OFlagSet clockOptions;
   OFlagSet xconfigFlag;
   int videoRam;
   unsigned long speedup;
   char *clockprog;
   int textClockValue;
   int BIOSbase;                 /* Base address of video BIOS */
   unsigned long MemBase;                  /* Frame buffer base address */
   int IObase;
   int DACbase;
   int COPbase;
   int POSbase;
   int instance;
   int s3Madjust;
   int s3Nadjust;
} GDevRec, *GDevPtr;

typedef struct {
   int depth;
   xrgb weight;
   int frameX0;
   int frameY0;
   int virtualX;
   int virtualY;
   DisplayModePtr modes;
   xrgb whiteColour;
   xrgb blackColour;
   int defaultVisual;
   OFlagSet options;
   OFlagSet xconfigFlag;
} DispRec, *DispPtr;

/*
 * We use the convention that tokens >= 1000 or < 0 do not need to be
 * present in a screen's list of valid tokens in order to be valid.
 * Also, each token should have a unique value regardless of the section
 * it is used in.
 */

#define LOCK_TOKEN  -3
#define ERROR_TOKEN -2
#define NUMBER		10000                  
#define STRING		10001

/* GJA -- gave those high numbers since they occur in many sections. */
#define SECTION		10002
#define SUBSECTION	10003  /* Only used at one place now. */
#define ENDSECTION	10004
#define ENDSUBSECTION	10005
#define IDENTIFIER	10006
#define VENDOR		10007
#define DASH		10008
#define COMMA		10009

#ifdef INIT_CONFIG
static SymTabRec TopLevelTab[] = {
  { SECTION,   "section" },
  { -1,         "" },
};
#endif /* INIT_CONFIG */

#define HRZ	1001	/* Silly name to avoid conflict with linux/param.h */
#define KHZ	1002
#define MHZ	1003

#ifdef INIT_CONFIG
static SymTabRec UnitTab[] = {
  { HRZ,	"hz" },
  { KHZ,	"khz" },
  { MHZ,	"mhz" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define SVGA	1010
#define VGA2	1011
#define MONO	1012
#define VGA16	1013
#define ACCEL	1014

#ifdef INIT_CONFIG
static SymTabRec DriverTab[] = {
  { SVGA,	"svga" },
  { SVGA,	"vga256" },
  { VGA2,	"vga2" },
  { MONO,	"mono" },
  { VGA16,	"vga16" },
  { ACCEL,	"accel" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define MICROSOFT	1020
#define MOUSESYS	1021
#define MMSERIES	1022
#define LOGITECH	1023
#define BUSMOUSE	1024
#define LOGIMAN		1025
#define PS_2		1026
#define MMHITTAB	1027
#define XQUE      	1030
#define OSMOUSE   	1031

#ifdef INIT_CONFIG
static SymTabRec MouseTab[] = {
  { MICROSOFT,	"microsoft" },
  { MOUSESYS,	"mousesystems" },
  { MMSERIES,	"mmseries" },
  { LOGITECH,	"logitech" },
  { BUSMOUSE,	"busmouse" },
  { LOGIMAN,	"mouseman" },
  { PS_2,	"ps/2" },
  { MMHITTAB,	"mmhittab" },
  { XQUE,	"xqueue" },
  { OSMOUSE,	"osmouse" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define FONTPATH	1040
#define RGBPATH		1041

#ifdef INIT_CONFIG
static SymTabRec FilesTab[] = {
  { ENDSECTION,	"endsection"},
  { FONTPATH,	"fontpath" },
  { RGBPATH,	"rgbpath" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define NOTRAPSIGNALS	1050
#define DONTZAP		1051

#ifdef INIT_CONFIG
static SymTabRec ServerFlagsTab[] = {
  { ENDSECTION, "endsection"},
  { NOTRAPSIGNALS, "notrapsignals" },
  { DONTZAP,	"dontzap" },
  { -1,         "" },
};
#endif /* INIT_CONFIG */

#define DISPLAYSIZE	1060
#define MODELINE	1061
#define MODEL		1062
#define BANDWIDTH	1063
#define HORIZSYNC	1064
#define VERTREFRESH	1065
#define MODE		1066
#define GAMMA		1067

#ifdef INIT_CONFIG
static SymTabRec MonitorTab[] = {
  { ENDSECTION,	"endsection"},
  { IDENTIFIER,	"identifier"}, 
  { VENDOR,	"vendorname"},
  { MODEL,	"modelname"}, 
  { MODELINE,	"modeline"},
  { DISPLAYSIZE,"displaysize" },
  { BANDWIDTH,	"bandwidth" },
  { HORIZSYNC,	"horizsync" },
  { VERTREFRESH,"vertrefresh" },
  { MODE,	"mode" },
  { GAMMA,	"gamma" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define DOTCLOCK	1070
#define HTIMINGS	1071
#define VTIMINGS	1072
#define FLAGS		1073
#define ENDMODE		1074

#ifdef INIT_CONFIG
static SymTabRec ModeTab[] = {
  { DOTCLOCK,	"dotclock"},
  { HTIMINGS,	"htimings"}, 
  { VTIMINGS,	"vtimings"},
  { FLAGS,	"flags"}, 
  { ENDMODE,	"endmode"},
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define DRIVER		1080
#define MDEVICE		1081
#define MONITOR		1082
#define SCREENNO	1083

#ifdef INIT_CONFIG
static SymTabRec ScreenTab[] = {
  { ENDSECTION, "endsection"},
  { DRIVER,	"driver" },
  { MDEVICE,	"device" },
  { MONITOR,	"monitor" },
  { SCREENNO,	"screenno" },
  { SUBSECTION,	"subsection" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

/* Mode timing keywords */
#define TT_INTERLACE	1090
#define TT_PHSYNC	1091
#define TT_NHSYNC	1092
#define TT_PVSYNC	1093
#define TT_NVSYNC	1094
#define TT_CSYNC	1095
#define TT_PCSYNC	1096
#define TT_NCSYNC	1097
#define TT_DBLSCAN	1098

#ifdef INIT_CONFIG
SymTabRec TimingTab[] = {
  { TT_INTERLACE,	"interlace"},
  { TT_PHSYNC,		"+hsync"},
  { TT_NHSYNC,		"-hsync"},
  { TT_PVSYNC,		"+vsync"},
  { TT_NVSYNC,		"-vsync"},
  { TT_CSYNC,		"composite"},
  { TT_PCSYNC,		"+csync"},
  { TT_NCSYNC,		"-csync"},
  { TT_DBLSCAN,		"doublescan"},
  { -1,			"" },
};
#else  /* defined(INIT_CONFIG) */

extern SymTabRec TimingTab[];

#endif /* !defined(INIT_CONFIG) */

/* Indexes for the specialKeyMap array */
#define K_INDEX_LEFTALT		0
#define K_INDEX_RIGHTALT	1
#define K_INDEX_SCROLLLOCK	2
#define K_INDEX_RIGHTCTL	3

/* Values for the specialKeyMap array */
#define KM_META		0
#define KM_COMPOSE	1
#define KM_MODESHIFT	2
#define KM_MODELOCK	3
#define KM_SCROLLLOCK	4
#define KM_CONTROL	5

#ifdef INIT_CONFIG
static SymTabRec KeyMapTab[] = {
  { KM_META,		"meta" },
  { KM_COMPOSE,		"compose" },
  { KM_MODESHIFT,	"modeshift" },
  { KM_MODELOCK,	"modelock" },
  { KM_SCROLLLOCK,	"scrolllock" },
  { KM_CONTROL,		"control" },
  { -1,			"" },
};
#endif /* INIT_CONFIG */

#define CHIPSET		10
#define CLOCKS		11
#define OPTION		12
#define VIDEORAM	13
#define BOARD		14
#define IOBASE		15
#define DACBASE		16
#define COPBASE		17
#define POSBASE		18
#define INSTANCE	19
#define RAMDAC		20
#define DACSPEED	21
#define SPEEDUP		22
#define NOSPEEDUP	23
#define CLOCKPROG	24
#define BIOSBASE	25
#define MEMBASE		26
#define CLOCKCHIP	27
#define S3MNADJUST	28

#ifdef INIT_CONFIG
static SymTabRec DeviceTab[] = {
  { ENDSECTION, "endsection"},
  { IDENTIFIER, "identifier"},
  { VENDOR, 	"vendorname"},
  { BOARD, 	"boardname"},
  { CHIPSET,	"chipset" },
  { RAMDAC,	"ramdac" },
  { DACSPEED,	"dacspeed"},
  { CLOCKS,	"clocks" },
  { OPTION,	"option" },
  { VIDEORAM,	"videoram" },
  { SPEEDUP,	"speedup" },
  { NOSPEEDUP,	"nospeedup" },
  { CLOCKPROG,	"clockprog" },
  { BIOSBASE,	"biosbase" },
  { MEMBASE,	"membase" },
  { IOBASE,	"iobase" },
  { DACBASE,	"dacbase" },
  { COPBASE,	"copbase" },
  { POSBASE,	"posbase" },
  { INSTANCE,	"instance" },
  { CLOCKCHIP,	"clockchip" },
  { S3MNADJUST,	"s3mnadjust" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

/* Keyboard keywords */
#define AUTOREPEAT	30
#define SERVERNUM	31
#define XLEDS		32
#define VTINIT		33
#define LEFTALT		34
#define RIGHTALT	35
#define SCROLLLOCK	36
#define RIGHTCTL	37
#define VTSYSREQ	38
#define KPROTOCOL	39

#ifdef INIT_CONFIG
static SymTabRec KeyboardTab[] = {
  { ENDSECTION,	"endsection"},
  { KPROTOCOL,	"protocol" },
  { AUTOREPEAT,	"autorepeat" },
  { SERVERNUM,	"servernumlock" },
  { XLEDS,	"xleds" },
  { VTINIT,	"vtinit" },
  { LEFTALT,	"leftalt" },
  { RIGHTALT,	"rightalt" },
  { RIGHTALT,	"altgr" },
  { SCROLLLOCK,	"scrolllock" },
  { RIGHTCTL,	"rightctl" },
  { VTSYSREQ,	"vtsysreq" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

#define P_MS		0			/* Microsoft */
#define P_MSC		1			/* Mouse Systems Corp */
#define P_MM		2			/* MMseries */
#define P_LOGI		3			/* Logitech */
#define P_BM		4			/* BusMouse ??? */
#define P_LOGIMAN	5			/* MouseMan / TrackMan
						   [CHRIS-211092] */
#define P_PS2		6			/* PS/2 mouse */
#define P_MMHIT		7			/* MM_HitTab */

#define EMULATE3	50
#define BAUDRATE	51
#define SAMPLERATE	52
#define CLEARDTR	53
#define CLEARRTS	54
#define CHORDMIDDLE	55
#define PROTOCOL	56
#define PDEVICE		57

#ifdef INIT_CONFIG
static SymTabRec PointerTab[] = {
  { PDEVICE,	"device"},
  { ENDSECTION,	"endsection"},
  { PROTOCOL,	"protocol" },
  { BAUDRATE,	"baudrate" },
  { EMULATE3,	"emulate3buttons" },
  { SAMPLERATE,	"samplerate" },
  { CLEARDTR,	"cleardtr" },
  { CLEARRTS,	"clearrts" },
  { CHORDMIDDLE,"chordmiddle" },
  { -1,		"" },
};
#endif /* INIT_CONFIG */

/* OPTION is defined to 12 above */
#define MODES		70
#define VIRTUAL		71
#define VIEWPORT	72
#define VISUAL		73
#define BLACK		74
#define WHITE		75
#define DEPTH		76
#define WEIGHT		77

#ifdef INIT_CONFIG
static SymTabRec DisplayTab[] = {
  { ENDSUBSECTION,	"endsubsection" },
  { MODES,		"modes" },
  { VIEWPORT,		"viewport" },
  { VIRTUAL,		"virtual" },
  { VISUAL,		"visual" },
  { BLACK,		"black" },
  { WHITE,		"white" },
  { DEPTH,		"depth" },
  { WEIGHT,		"weight" },
  { OPTION,		"option" },
  { -1,			"" },
};
#endif /* INIT_CONFIG */

/* Graphics keywords */
#define STATICGRAY	90
#define GRAYSCALE	91
#define STATICCOLOR	92
#define PSEUDOCOLOR	93
#define TRUECOLOR	94
#define DIRECTCOLOR	95

#ifdef INIT_CONFIG
static SymTabRec VisualTab[] = {
  { STATICGRAY,	"staticgray" },
  { GRAYSCALE,	"grayscale" },
  { STATICCOLOR,"staticcolor" },
  { PSEUDOCOLOR,"pseudocolor" },
  { TRUECOLOR,	"truecolor" },
  { DIRECTCOLOR,"directcolor" },
  { -1,         "" },
};
#endif /* INIT_CONFIG */

#endif /* XCONFIG_FLAGS_ONLY */

/* 
 * XF86Config flags to record which options were defined in the XF86Config file
 */
#define XCONFIG_FONTPATH        1       /* Commandline/XF86Config or default  */
#define XCONFIG_RGBPATH         2       /* XF86Config or default */
#define XCONFIG_CHIPSET         3       /* XF86Config or probed */
#define XCONFIG_CLOCKS          4       /* XF86Config or probed */
#define XCONFIG_DISPLAYSIZE     5       /* XF86Config or default/calculated */
#define XCONFIG_VIDEORAM        6       /* XF86Config or probed */
#define XCONFIG_VIEWPORT        7       /* XF86Config or default */
#define XCONFIG_VIRTUAL         8       /* XF86Config or default/calculated */
#define XCONFIG_SPEEDUP         9       /* XF86Config or default/calculated */
#define XCONFIG_NOMEMACCESS     10      /* set if forced on */
#define XCONFIG_INSTANCE        11      /* XF86Config or default */
#define XCONFIG_RAMDAC          12      /* XF86Config or default */
#define XCONFIG_DACSPEED        13      /* XF86Config or default */
#define XCONFIG_BIOSBASE        14      /* XF86Config or default */
#define XCONFIG_MEMBASE         15      /* XF86Config or default */
#define XCONFIG_IOBASE          16      /* XF86Config or default */
#define XCONFIG_DACBASE         17      /* XF86Config or default */
#define XCONFIG_COPBASE         18      /* XF86Config or default */
#define XCONFIG_POSBASE         19      /* XF86Config or default */

#define XCONFIG_GIVEN		"(**)"
#define XCONFIG_PROBED		"(--)"

#ifdef INIT_CONFIG

OFlagSet  GenericXF86ConfigFlag;

#else

extern OFlagSet  GenericXF86ConfigFlag;

#endif  /* INIT_CONFIG */
