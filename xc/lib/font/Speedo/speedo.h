#ifndef _SPEEDO_H_
#define _SPEEDO_H_

/*****************************************************************************
*                                                                            *
*  Copyright 1989, as an unpublished work by Bitstream Inc., Cambridge, MA   *
*                         U.S. Patent No 4,785,391                           *
*                           Other Patent Pending                             *
*                                                                            *
*         These programs are the sole property of Bitstream Inc. and         *
*           contain its proprietary and confidential information.            *
*                                                                            *
*****************************************************************************/

/********************* Revision Control Information **********************************
*                                                                         
*     $Header: //toklas/archive/rcs/speedo/speedo.h,v 22.1 91/01/23 17:22:07 leeann Release $                                                                       *
*                                                                                    
*     $Log:	speedo.h,v $
*       Revision 22.1  91/01/23  17:22:07  leeann
*       Release
*       
*       Revision 21.2  91/01/17  09:45:31  leeann
*       modify declaration of sp_calculate functions to be NOT static
*       
*       Revision 21.1  90/11/20  14:41:26  leeann
*       Release
*       
*       Revision 20.1  90/11/12  09:37:03  leeann
*       Release
*       
*       Revision 19.1  90/11/08  10:26:38  leeann
*       Release
*       
*       Revision 18.2  90/11/06  19:00:11  leeann
*       make global clipping coordinates, add new
*       function compute_isw_scale
*       
*       Revision 18.1  90/09/24  10:18:07  mark
*       Release
*       
*       Revision 17.3  90/09/24  09:32:48  mark
*       change #ifdef #if on squeezing declarations for proper compilation
*       
*       Revision 17.2  90/09/19  18:09:34  leeann
*       make preview_bounding_box visible when squeezing
*       
*       Revision 17.1  90/09/13  15:58:52  mark
*       Release name rel0913
*       
*       Revision 16.1  90/09/11  13:23:55  mark
*       Release
*       
*       Revision 15.2  90/09/05  11:26:46  leeann
*       added two new functions: sp_reset_xmax and sp_preview_bounding_box
*       
*       Revision 15.1  90/08/29  10:03:35  mark
*       Release name rel0829
*       
*       Revision 14.3  90/08/23  16:11:46  leeann
*       make setup_const take min and max as arguments
*       
*       Revision 14.2  90/08/02  10:12:26  mark
*       make type declarations conditional on STDEF
*       
*       Revision 12.3  90/06/20  15:56:39  leeann
*       Add parameter of number of y control zones to function
*       sp_calculate_y_zone
*       
*       Revision 12.2  90/06/01  15:23:49  mark
*       add mirror flag to tcb_t as a flag of mirro
*       image transformations.
*       
*       Revision 12.1  90/04/23  12:12:15  mark
*       Release name REL20
*       
*       Revision 11.1  90/04/23  10:12:26  mark
*       Release name REV2
*       
*       Revision 10.24  90/04/23  09:41:03  mark
*       locate declaration of sp_get_cust_no so it is 
*       available in all configurations
*       add prototypes to bitmap/outline point function 
*       declarations in multi device support
*       
*       Revision 10.23  90/04/21  10:44:16  mark
*       add declaration of structure and functions for
*       multiple output device handling (bitmap_t, outline_t
*       sp_set_bitmap_device() and sp_set_outline_device()
*       
*       include useropt.h for overriding default configuration
*       without needing to modify speedo.h
*       
*       added structure tags to all structure declarations to
*       kill warnings on certain compilers
*       
*       Revision 10.22  90/04/18  16:06:39  judy
*       new global variable, rnd_xmin, added for inter-character
*       spacing fix. Stores rounded out part of xmin.
*       
*       Revision 10.21  90/04/18  09:55:44  mark
*       define init_userout
*       
*       Revision 10.20  90/04/18  09:43:22  mark
*       add default definition of user defined output modules
*       
*       Revision 10.19  90/04/17  09:32:11  leeann
*       use same setwidth variable for squeeze and imported setwidth
*       
*       Revision 10.18  90/04/12  12:59:00  mark
*       add argument of type buff_t to get_cust_no, since
*       valids specs cannot be provided via set_specs until
*       the encryption is set, which requires customer number
*       
*       Revision 10.17  90/04/12  12:22:31  mark
*       added definition of bbox_t for bounding boxes
*       added declaration of sp_get_char_bbox and sp_get_cust_no
*       
*       Revision 10.16  90/04/12  09:10:53  leeann
*       default INCL_CLIPPING to be false
*       
*       Revision 10.15  90/04/11  13:03:25  leeann
*       make conditional compilation flag for squeezing be
*       INCL_SQUEEZING, add imported setwidth data
*       
*       Revision 10.14  90/04/10  12:06:20  mark
*       include symbolic constants for squeezing or clipping
*       
*       Revision 10.13  90/04/06  13:41:30  mark
*       put definition of real back
*       
*       Revision 10.12  90/04/06  13:11:30  mark
*       make fix7 definition conditional on SPD_BMAP
*       
*       Revision 10.11  90/04/06  13:08:29  mark
*       remove definition of real, make definition of boolean
*       and ufix8 conditional on SPD_BMAP
*       
*       Revision 10.10  90/04/06  12:31:41  mark
*       declare curve handling functions in out_scrn
*       
*       Revision 10.9  90/04/05  15:13:35  leeann
*       add new SQUEEZE fields setwidth_orus and squeezing_compound
*       
*       Revision 10.8  90/03/30  14:57:29  mark
*        remove out_wht and add out_scrn and out_util
*       
*       Revision 10.7  90/03/29  16:42:42  leeann
*       Added set_flags argument to read_bbox
*       
*       Revision 10.6  90/03/28  13:49:15  leeann
*       new global variables added for squeezing, 
*       new function skip_orus added
*       
*       Revision 10.5  90/03/27  14:48:57  leeann
*       Include new functions skip_control_zone, skip_interpolation_zone
*       
*       Revision 10.4  90/03/26  15:47:37  mark
*       add definitions for optional metric resolution in font header
*       add metric_resolution to sp_globals
*       
*       Revision 10.3  89/10/25  13:40:35  mark
*       change default to extended font support, change maximum to 750 constraints
*       
*       Revision 10.2  89/09/11  11:36:40  mark
*       correct declaration of stackfar pointer to fontfar pointer argument
*       in functions sp_get_posn_arg and sp_get_scale_arg so that code works
*       with Microsoft C when stackfar and fontfar are not equivalent.
*       
*       Revision 10.1  89/07/28  18:09:31  mark
*       Release name PRODUCT
*       
*       Revision 9.3  89/07/28  18:05:41  mark
*       fix function prototype for sp_open_outline
*       
*       Revision 9.2  89/07/28  16:40:36  mark
*       Modified prototype of sp_draw_vector_to_2d to use a GLOBALFAR
*       pointer to the band structure.  Change out_bl2d.c as well.
*       
*       Revision 9.1  89/07/27  10:22:49  mark
*       Release name PRODUCT
*       
*       Revision 8.2  89/07/27  10:05:34  mark
*       change pfont element of speedo_globals to
*       be GLOBALFAR since it now points to copy save
*       in speedo_globals.  Also change function 
*       prototype of (*init_out) to take a GLOBALFAR 
*       pointer to a specs_t
*       
*       Revision 8.1  89/07/13  18:19:12  mark
*       Release name Product
*       
*       Revision 7.1  89/07/11  09:00:58  mark
*       Release name PRODUCT
*       
*       Revision 6.4  89/07/09  14:59:34  mark
*       change stuff to handle GLOBALFAR option
*       
*       Revision 6.3  89/07/09  13:16:58  mark
*       changed prototypes for sp_open_bitmap to contain
*       new high resolution positioning information
*       
*       Revision 6.2  89/07/09  12:37:34  mark
*       For dynamic allocation, pointers to intercepts and plaid
*       must be modified with STACKFAR
*       Also, add specs structure to sp_globals and copy structure
*       passed to set_specs, then set pspecs to point to this copy
*       in case user allocates it off the stack
*       
*       Revision 6.1  89/06/19  08:34:42  mark
*       Release name prod
*       
*       Revision 5.7  89/06/16  17:44:32  mark
*       fix output module curve prototypes to include new
*       parameter for splitting depth
*       
*       Revision 5.6  89/06/16  16:51:27  mark
*       boost MAX_CONSTR to 300 for reasonable safety margin
*       
*       Revision 5.5  89/06/06  17:22:37  mark
*       add curve depth to output module curve functions
*       
*       Revision 5.4  89/06/05  17:23:00  mark
*       define symbols for sp_plaid and sp_intercepts.  These symbols are
*       equated to sp_globals for static and dynamic models, but are indirect
*       references via new pointers for reentrant mode.  Pointers are set to
*       stack allocated arrays in make_char
*       
*       Revision 5.3  89/06/02  08:29:16  mark
*       move xmin,xmax,ymin,ymax out of conditional so that they
*       are available to outline output module as well.
*       
*       Revision 5.2  89/06/01  16:52:18  mark
*       changed declaration of begin_char functions to boolean
*       
*       Revision 5.1  89/05/01  17:53:27  mark
*       Release name Beta
*       
*       Revision 4.2  89/05/01  17:16:48  mark
*       set PROTO_DECL1 to void in non-reentrant case for
*       functions with no parameters
*       
*       Revision 4.1  89/04/27  12:14:36  mark
*       Release name Beta
*       
*       Revision 3.2  89/04/26  16:58:46  mark
*       remove redefinitions of private header
*       don't use #undef extern, because some compilers get upset
*       
*       Revision 3.1  89/04/25  08:27:39  mark
*       Release name beta
*       
*       Revision 2.8  89/04/24  10:37:19  mark
*       Straighten out declaration of static functions in out_bl2d.c
*       
*       Revision 2.7  89/04/18  18:22:39  john
*       setup_mult(), setup_offset() function definitions added
*       
*       Revision 2.6  89/04/13  10:45:44  mark
*       make flags in specs_t unsigned (agrees with documentation)
*       
*       Revision 2.5  89/04/12  13:18:25  mark
*       correct far pointer declarations of get_posn_args 
*       and get_scale_args
*       
*       Revision 2.4  89/04/12  12:09:59  mark
*       added stuff for far stack and font
*       
*       Revision 2.3  89/04/10  17:03:56  mark
*       updated function prototypes to use FONTFAR symbol
*       which will establish independent data segment for
*       Font buffers under MicroSoft C
*       
*       Revision 2.2  89/04/10  12:37:22  mark
*       Changed COMPACT to SHORT_LISTS
*       added PROTOS_AVAIL flags, which enables use of function prototypes
*       
*       Revision 2.1  89/04/04  13:34:08  mark
*       Release name EVAL
*       
*       Revision 1.13  89/04/04  13:20:48  mark
*       Update copyright text
*       
*       Revision 1.12  89/04/03  15:06:16  mark
*       defined COMPACT and set up new default configuration
*       
*       Revision 1.11  89/03/31  17:35:55  john
*       Added read_word_u() function def.
*       
*       Revision 1.10  89/03/31  15:06:52  mark
*       split into speedo.h and spdo_prv.h
*       
*       Revision 1.9  89/03/31  12:15:46  john
*       modified to use new NEXT_WORD macro.
*       
*       Revision 1.8  89/03/30  17:57:14  john
*       normal moved from output module data to set_spcs area of 
*       global data structure.
*       
*       Revision 1.7  89/03/29  16:05:17  mark
*       Added data declarations for output module slot independence
*       also defined SPEEDO_GLOBALS structure to contain all data.
*       also moved function declarations into here, and defined
*       macros to redefine all functions for reentrant code
*       
*       Revision 1.6  89/03/24  16:23:13  john
*       Character directory definitions removed
*       
*       Revision 1.5  89/03/23  11:49:58  john
*       new entried added to font header
*       
*       Revision 1.4  89/03/22  18:13:03  csdf
*       cdf 3/22/88 Added compile constant so that this works with VFONT
*       
*       Revision 1.3  89/03/22  13:34:21  mark
*       added conditional definition of conditional compile flags to allow
*       overriding from command line.
*       
*       Revision 1.2  89/03/21  13:23:10  mark
*       change name from oemfw.h to speedo.h
*       
*       Revision 1.1  89/03/15  12:33:02  mark
*       Initial revision
*                                                                                
*                                                                                    
*************************************************************************************/




/***************************** S P E E D O . H *******************************
 ********************** R E V I S I O N   H I S T O R Y **********************
 *                                                                           *
 *  1) 14 Dec 88  jsc  Created                                               *
 *                                                                           *
 *  2) 23 Jan 89  jsc  normal removed from tcb structure                     *
 *                                                                           *
 *  3)  2 Feb 89  jsc  Inhibit constraint option (Bit 5) added to flags      *
 *                     element of specs bundle.                              *
 *                                                                           *
 *  4)  6 Feb 89  jsc  COMPACT compile time flag renamed INCL_EXT            *
 *                                                                           *
 *  5)  9 Feb 89  jsc  Font header field FH_CMPCT changed to FH_FLAGS and    *
 *                     redefined to contain 8 flags.                         *
 *                                                                           *
 *                     Position of customer number changed in font header.   *
 *                                                                           *
 *  6) 17 Feb 89  jsc  Support for 8 possible output modules.                *                          
 *                                                                           *
 *  7) 24 Feb 89  jsc  New fields added to font header.                      *
 *                                                                           *
 ****************************************************************************/

/*****  USER OPTIONS OVERRIDE DEFAULTS ******/
#include "useropt.h"

/*****  CONFIGURATION DEFINITIONS *****/

#ifndef INCL_CLIPPING
#define INCL_CLIPPING 0		/* 0 indicates CLIPPING code is not compiled in*/
#endif

#ifndef INCL_SQUEEZING
#define INCL_SQUEEZING 0		/* 0 indicates SQUEEZE code is not compiled in*/
#endif

#ifndef INCL_EXT
#define  INCL_EXT       1          /* 1 to include extended font support */
#endif                             /* 0 to omit extended font support */

#ifndef INCL_RULES
#define  INCL_RULES     1          /* 1 to include intelligent scaling support */
#endif                             /* 0 to omit intelligent scaling support */

#ifndef INCL_BLACK                                                    
#define  INCL_BLACK     1          /* 1 to include blackwriter output support */
#endif                             /* 0 to omit output mode 0 support */

#ifndef INCL_SCREEN
#define  INCL_SCREEN     0          /* 1 to include screen writeroutput support */
#endif                             /* 0 to omit support */

#ifndef INCL_OUTLINE
#define  INCL_OUTLINE     0          /* 1 to include outline output support */
#endif                             /* 0 to omit output mode 2 support */

#ifndef INCL_2D
#define  INCL_2D          0          /* 1 to include 2d blackwriter output support */
#endif                             /* 0 to omit output mode 3 support */

#ifndef INCL_USEROUT
#define INCL_USEROUT      0          /* 1 to include user defined output module support */
#endif                               /* 0 to omit user defined output module support */

#ifndef INCL_LCD
#define  INCL_LCD       1          /* 1 to include load char data support*/
#endif                             /* 0 to omit load char data support */
#ifndef INCL_ISW
#define  INCL_ISW       0          /* 1 to include imported width support */
#endif                             /* 0 to omit imported width support */

#ifndef INCL_METRICS
#define  INCL_METRICS   1          /* 1 to include metrics support */
#endif                             /* 0 to omit metrics support */

#ifndef INCL_KEYS
#define  INCL_KEYS      0          /* 1 to include multi key support */
#endif                             /* 0 to omit multi key support */

#ifndef INCL_MULTIDEV
#define  INCL_MULTIDEV  0          /* 1 to include multiple output device support */
#endif                             /* 0 to omit multi device support */

#ifndef SHORT_LISTS
#define SHORT_LISTS 1                  /* 1 to allocate small intercept lists */
#endif

#ifndef PROTOS_AVAIL                /* 1 to use function prototyping */
#define PROTOS_AVAIL 0   			/* 0 to suppress it */
#endif

#ifndef FONTFAR						/* if Intel mixed memory model implementation */
#define FONTFAR						/* pointer type modifier for font buffer */
#endif

#ifndef STACKFAR					/* if Intel mixed memory model implementation */
#define STACKFAR					/* pointer type modifier for font buffer */
#endif

#ifndef GLOBALFAR
#define GLOBALFAR
#endif
 
#define MODE_BLACK 0
#define MODE_SCREEN MODE_BLACK + INCL_BLACK
#define MODE_OUTLINE MODE_SCREEN + INCL_SCREEN
#define MODE_2D MODE_OUTLINE + INCL_OUTLINE

#ifdef DYNAMIC_ALLOC
#if DYNAMIC_ALLOC 
#define STATIC_ALLOC 0
#endif
#endif

#ifdef REENTRANT_ALLOC
#if REENTRANT_ALLOC 
#define STATIC_ALLOC 0
#endif
#endif

#ifndef STATIC_ALLOC
#define STATIC_ALLOC 1
#endif

#ifndef DYNAMIC_ALLOC
#define DYNAMIC_ALLOC 0
#endif

#ifndef REENTRANT_ALLOC
#define REENTRANT_ALLOC 0
#endif

/*****  TYPE  DEFINITIONS *****/

#ifndef STDEF
#ifndef SPD_BMAP

#if defined(APOLLO) || defined(sun)
typedef   char     fix7;
#else
typedef signed char fix7;
#endif

typedef   double   real;

#ifdef VFONT
typedef   unsigned char
                   ufix8;
#else 
typedef   unsigned char
                   ufix8,
                   boolean;
#endif
#endif

typedef   short    fix15;

typedef   unsigned short
                   ufix16;

typedef   long     fix31;

typedef   unsigned long
                   ufix32;
#endif

/***** GENERAL CONSTANTS *****/

#ifndef FALSE
#define  FALSE     0
#define  TRUE      1
#endif

#ifndef NULL
#define NULL       0
#endif

#define  FUNCTION

#define  BIT0           0x01
#define  BIT1           0x02
#define  BIT2           0x04
#define  BIT3           0x08
#define  BIT4           0x10
#define  BIT5           0x20
#define  BIT6           0x40
#define  BIT7           0x80

#if INCL_EXT                       /* Extended fonts supported? */

#define  MAX_CONSTR     750       /* Max constraints (incl 4 dummies) */
#define  MAX_CTRL_ZONES  256       /* Max number of controlled orus */
#define  MAX_INT_ZONES   256       /* Max number of interpolation zones */

#else                              /* Compact fonts only supported */

#define  MAX_CONSTR      512       /* Max constraints (incl 4 dummies) */
#define  MAX_CTRL_ZONES   64       /* Max number of controlled orus */
#define  MAX_INT_ZONES    64       /* Max number of interpolation zones */

#endif

#define  SCALE_SHIFT   12   /* Binary point positiion for scale values */
#define  SCALE_RND   2048   /* Rounding bit for scaling transformation */
#define  ONE_SCALE   4096   /* Unity scale value */
    
#ifdef INCL_SCREEN   /* constants used by Screenwriter module */
#define LEFT_INT 1   /* left intercept */
#define END_INT 2    /* last intercept */
#define FRACTION 0xFC  /* fractional portion of intercept type list */
#endif

#if INCL_SQUEEZING || INCL_CLIPPING          /* constants used by SQUEEZEing code */
#define EM_TOP 764
#define EM_BOT -236
#endif

/*****  STRUCTURE DEFINITIONS *****/
#if PROTOS_AVAIL

#if REENTRANT_ALLOC
#define PROTO_DECL1 struct speedo_global_data GLOBALFAR *sp_global_ptr
#define PROTO_DECL2 PROTO_DECL1 ,
#else
#define PROTO_DECL1 void
#define PROTO_DECL2
#endif
#endif

typedef
struct buff_tag
    {
    ufix8 FONTFAR *org;                   /* Pointer to start of buffer */
    ufix32  no_bytes;              /* Size of buffer in bytes */
    } 
buff_t;                            /* Buffer descriptor */

typedef  struct constr_tag
    {
    ufix8 FONTFAR *org;                   /* Pointer to first byte in constr data  */
    ufix16  font_id;               /* Font id for calculated data           */
    fix15   xppo;                  /* X pixels per oru for calculated data  */
    fix15   yppo;                  /* Y pixels per oru for calculated data  */
    boolean font_id_valid;         /* TRUE if font id valid                 */
    boolean data_valid;            /* TRUE if calculated data valid         */
    boolean active;                /* TRUE if constraints enabled           */
    }                  
constr_t;                          /* Constraint data state                 */

typedef  struct kern_tag
    {
    ufix8 FONTFAR *tkorg;                 /* First byte of track kerning data      */
    ufix8 FONTFAR *pkorg;                 /* First byte of pair kerning data       */
    fix15   no_tracks;             /* Number of kerning tracks              */
    fix15   no_pairs;              /* Number of kerning pairs               */
    }                  
kern_t;                            /* Kerning control block                 */

typedef struct specs_tag
    {
    buff_t STACKFAR *pfont;                 /* Pointer to font data                  */
    fix31   xxmult;                /* Coeff of X orus to compute X pix      */
    fix31   xymult;                /* Coeff of Y orus to compute X pix      */
    fix31   xoffset;               /* Constant to compute X pix             */
    fix31   yxmult;                /* Coeff of X orus to compute Y pix      */
    fix31   yymult;                /* Coeff of Y orus to compute Y pix      */
    fix31   yoffset;               /* Constant to compute Y pix             */
    ufix32  flags;                 /* Mode flags:                           */
                                   /*   Bit  0 - 2: Output module selector: */
                                   /*   Bit  3: Send curves to output module*/
                                   /*   Bit  4: Use linear scaling if set   */
                                   /*   Bit  5: Inhibit constraint table    */
                                   /*   Bit  6: Import set width if set     */
                                   /*   Bit  7:   not used                  */
                                   /*   Bit  8: Squeeze left if set         */
                                   /*   Bit  9: Squeeze right if set        */
                                   /*   Bit 10: Squeeze top if set          */
                                   /*   Bit 11: Squeeze bottom if set       */
                                   /*   Bit 12: Clip left if set            */
                                   /*   Bit 13: Clip right if set           */
                                   /*   Bit 14: Clip top if set             */
                                   /*   Bit 15: Clip bottom if set          */
                                   /*   Bits 16-31   not used               */
    void *out_info;                /* information for output module         */
    }
specs_t;                           /* Specs structure for fw_set_specs      */

typedef struct tcb_tag
    {
    fix15   xxmult;                /* Linear coeff of Xorus to compute Xpix */
    fix15   xymult;                /* Linear coeff of Yorus to compute Xpix */
    fix31   xoffset;               /* Linear constant to compute Xpix       */
    fix15   yxmult;                /* Linear coeff of Xorus to compute Ypix */
    fix15   yymult;                /* Linear coeff of Yorus to compute Ypix */
    fix31   yoffset;               /* Linear constant to compute Ypix       */
    fix15   xppo;                  /* Pixels per oru in X dimension of char */
    fix15   yppo;                  /* Pixels per oru in Y dimension of char */
    fix15   xpos;                  /* Origin in X dimension of character    */
    fix15   ypos;                  /* Origin in Y dimension of character    */
    ufix16  xtype;                 /* Transformation type for X oru coords  */
    ufix16  ytype;                 /* Transformation type for Y oru coords  */
    ufix16  xmode;                 /* Transformation mode for X oru coords  */
    ufix16  ymode;                 /* Transformation mode for Y oru coords  */
	fix15  mirror;                /* Transformation creates mirror image   */
    }
tcb_t;                             /* Transformation control block          */

typedef struct point_tag
    {
    fix15   x;                     /* X coord of point (shifted pixels)     */
    fix15   y;                     /* Y coord of point (shifted pixels)     */
    }
point_t;                           /* Point in device space                 */

typedef struct band_tag
    {
    fix15   band_max;
    fix15   band_min;
    fix15   band_array_offset;
    fix15   band_floor;
    fix15   band_ceiling;
    } band_t;

typedef struct bbox_tag
    {
    fix31   xmin;
    fix31   xmax;
    fix31   ymin;
    fix31   ymax;
    } bbox_t;

#if SHORT_LISTS 
#define  MAX_INTERCEPTS  256      /* Max storage for intercepts */
typedef  ufix8   cdr_t;           /* 8 bit links in intercept chains */
#else
#define  MAX_INTERCEPTS 1000      /* Max storage for intercepts */
typedef  ufix16   cdr_t;          /* 16 bit links in intercept chains */
#endif

#if REENTRANT_ALLOC

typedef struct intercepts_tag
    {
	fix15 car[MAX_INTERCEPTS];
	fix15 cdr[MAX_INTERCEPTS];
#if INCL_SCREEN
	ufix8 inttype[MAX_INTERCEPTS];
	ufix8 leftedge;
	ufix16 fracpix;
#endif
	} intercepts_t;

typedef struct plaid_tag
	{
	fix15    orus[MAX_CTRL_ZONES];   /* Controlled coordinate table (orus) */
#if INCL_RULES
	fix15    pix[MAX_CTRL_ZONES];    /* Controlled coordinate table (sub-pixels) */
	fix15    mult[MAX_INT_ZONES];    /* Interpolation multiplier table */
	fix31    offset[MAX_INT_ZONES];  /* Interpolation offset table */
#endif
	} plaid_t;
#endif

#if INCL_MULTIDEV
#if PROTOS_AVAIL
typedef struct bitmap_tag 
	{
	void (*p_open_bitmap)(PROTO_DECL2 fix31 x_set_width, fix31 y_set_width, fix31 xorg, fix31 yorg, fix15 xsize,fix15 ysize);
	void (*p_set_bits)(PROTO_DECL2 fix15 y, fix15 xbit1, fix15 xbit2);
	void (*p_close_bitmap)(PROTO_DECL1);
	} bitmap_t;

typedef struct outline_tag 
	{
	void (*p_open_outline)(PROTO_DECL2 fix31 x_set_width, fix31 y_set_width, fix31 xmin, fix31 xmax, fix31 ymin,fix31 ymax);
	void (*p_start_char)(PROTO_DECL1);
	void (*p_start_contour)(PROTO_DECL2 fix31 x,fix31 y,boolean outside);
	void (*p_curve)(PROTO_DECL2 fix31 x1, fix31 y1, fix31 x2, fix31 y2, fix31 x3, fix31 y3);
	void (*p_line)(PROTO_DECL2 fix31 x, fix31 y);
	void (*p_close_contour)(PROTO_DECL1);
	void (*p_close_outline)(PROTO_DECL1);
	} outline_t;
#else
typedef struct bitmap_tag 
	{
	void (*p_open_bitmap)();
	void (*p_set_bits)();
	void (*p_close_bitmap)();
	} bitmap_t;

typedef struct outline_tag 
	{
	void (*p_open_outline)();
	void (*p_start_char)();
	void (*p_start_contour)();
	void (*p_curve)();
	void (*p_line)();
	void (*p_close_contour)();
	void (*p_close_outline)();
	} outline_t;
#endif
#endif

/* ---------------------------------------------------*/
/****  MAIN GLOBAL DATA STRUCTURE, SPEEDO_GLOBALS *****/

typedef struct speedo_global_data 
	{
/*  do_char.c data definitions */
#if INCL_METRICS                    /* Metrics functions supported? */
         kern_t  kern;              /* Kerning control block */
#endif                                                               /* endif incl_metrics */
	 point_t   Psw;             /* End of escapement vector (1/65536 pixel units) */

#if INCL_LCD                        /* Dynamic load character data supported? */
         fix15  cb_offset;          /* Offset to sub-char data in char buffer */
#endif                                                               /* endif incl_lcd */

/* do_trns.c data definitions */
	 point_t  P0;               /* Current point (sub-pixels) */
	 fix15    x_orus;           /* Current X argument (orus) */
	 fix15    y_orus;           /* Current Y argument (orus) */
	 fix15    x_pix;            /* Current X argument (sub-pixels) */
	 fix15    y_pix;            /* Current Y argument (sub-pixels) */
	 ufix8    x_int;            /* Current X interpolation zone */
	 ufix8    y_int;            /* Current Y interpolation zone */

#if INCL_MULTIDEV && INCL_OUTLINE
     outline_t outline_device;
     boolean   outline_device_set;
#endif

#if INCL_BLACK || INCL_SCREEN || INCL_2D
#if INCL_MULTIDEV
     bitmap_t bitmap_device;
     boolean  bitmap_device_set;
#endif
     band_t   y_band;           /* Y current band(whole pixels) */

	 struct set_width_tag
        {
        fix31 x;
        fix31 y;
        } set_width; /* Character escapement vector */

	 boolean  first_pass;       /* TRUE during first pass thru outline data */
	 boolean  extents_running;  /* T if extent accumulation for each vector */
	 fix15    x0_spxl;          /* X coord of current point (sub pixels) */
	 fix15    y0_spxl;          /* Y coord of current point (sub pixels) */
	 fix15    y_pxl;            /* Y coord of current point (whole pixels) */
#if REENTRANT_ALLOC
     intercepts_t STACKFAR *intercepts;
#else                                                                /* else if not reentrant */
	 fix15    car[MAX_INTERCEPTS]; /* Data field of intercept storage */
	 cdr_t    cdr[MAX_INTERCEPTS]; /* Link field of intercept storage */
#if INCL_SCREEN
         ufix8    inttype[MAX_INTERCEPTS];
         ufix8    leftedge;
         ufix16   fracpix;
#endif                                                               /* endif incl_screen */
#endif                                                               /* endif reentrant */
	 fix15    bmap_xmin;        /* Min X value (sub-pixel units) */
	 fix15    bmap_xmax;        /* Max X value (sub-pixel units) */
	 fix15    bmap_ymin;        /* Min Y value (sub-pixel units) */
	 fix15    bmap_ymax;        /* Max Y value (sub-pixel units) */
	 fix15    no_y_lists;       /* Number of active intercept lists */
	 fix15    first_offset;     /* Index of first active list cell */
	 fix15    next_offset;      /* Index of next free list cell */
	 boolean  intercept_oflo;   /* TRUE if intercepts data lost */
#endif                                                               /* endif incl_black, incl_screen, incl_2d */

/* bounding box now used by all output modules, including outline */
	 fix15    xmin;             /* Min X value in whole character */
	 fix15    xmax;             /* Max X value in whole character */
	 fix15    ymin;             /* Min Y value in whole character */
	 fix15    ymax;             /* Max Y value in whole character */

#if INCL_2D
         fix15    no_x_lists;       /* Number of active x intercept lists */
         band_t   x_band;           /* X current band(whole pixels) */
         boolean  x_scan_active;    /* X scan flag during scan conversion */
#endif

/* reset.c data definitions */
         ufix16   key32;            /* Decryption keys 3,2 combined */
         ufix8    key4;             /* Decryption key 4 */
         ufix8    key6;             /* Decryption key 6 */
         ufix8    key7;             /* Decryption key 7 */
         ufix8    key8;             /* Decryption key 8 */

/* set_spcs.c data definitions */
         buff_t   font;
         buff_t GLOBALFAR *pfont; /* Pointer to font buffer structure */
         fix31    font_buff_size; /* Number of bytes loaded in font buffer */
         ufix8 FONTFAR *pchar_dir; /* Pointer to character directory */
         fix15    first_char_idx; /* Index to first character in font */
         fix15    no_chars_avail; /* Total characters in font layout */
         fix15    orus_per_em;    /* Outline resolution */
         fix15    metric_resolution; /* metric resolution for setwidths, kerning pairs
							(defaults to orus_per_em) */
         tcb_t    tcb0;           /* Top level transformation control block */
	
         boolean  specs_valid;    /* TRUE if fw_set_specs() successful */
	
         fix15    depth_adj;      /* Curve splitting depth adjustment */
         boolean  curves_out;     /* Allow curves to output module */
         fix15    output_mode;    /* Output module selector */
         fix15    thresh;         /* Scan conversion threshold (sub-pixels) */
         boolean  normal;         /* TRUE if 0 obl and mult of 90 deg rot  */
	
         fix15    multshift;      /* Fixed point shift for multipliers */
         fix15    pixshift;       /* Fixed point shift for sub-pixels */
         fix15    poshift;        /* Left shift from pixel to output format */
         fix15    mpshift;        /* Fixed point shift for mult to sub-pixels */
         fix31    multrnd;        /* 0.5 in multiplier units */
         fix15    pixrnd;         /* 0.5 in sub-pixel units */
         fix31    mprnd;          /* 0.5 sub-pixels in multiplier units */
         fix15    pixfix;         /* Mask to remove fractional pixels */
         fix15    onepix;         /* 1.0 pixels in sub-pixel units */

#if PROTOS_AVAIL
         boolean (*init_out)(PROTO_DECL2 specs_t GLOBALFAR *specsarg);
         boolean (*begin_char)(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax); 
         void    (*begin_sub_char)(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax);
         void    (*begin_contour)(PROTO_DECL2 point_t P1,boolean outside); 
         void    (*curve)(PROTO_DECL2 point_t P1, point_t P2, point_t P3, fix15 depth);  
         void    (*line)(PROTO_DECL2 point_t P1);               
         void    (*end_contour)(PROTO_DECL1); 
         void    (*end_sub_char)(PROTO_DECL1);
         boolean (*end_char)(PROTO_DECL1);    
#else                                                                /* if not protos */
         boolean (*init_out)();
         boolean (*begin_char)(); 
         void    (*begin_sub_char)();
         void    (*begin_contour)(); 
         void    (*curve)();         
         void    (*line)();               
         void    (*end_contour)(); 
         void    (*end_sub_char)();
         boolean (*end_char)();
#endif                                                               /* end not protos */

         specs_t GLOBALFAR *pspecs;    /* Pointer to specifications bundle */
         specs_t specs;                /* copy specs onto stack */
         ufix8 FONTFAR  *font_org;     /* Pointer to start of font data */
         ufix8 FONTFAR  *hdr2_org;     /* Pointer to start of private header data */

/* set_trns.c data definitions */
         tcb_t    tcb;                 /* Current transformation control block */
         ufix8    Y_edge_org;          /* Index to first Y controlled coordinate */
         ufix8    Y_int_org;           /* Index to first Y interpolation zone */
         fix31    rnd_xmin;            /* rounded out value of xmin for int-char spac. fix */

#if REENTRANT_ALLOC
         plaid_t STACKFAR  *plaid;
#else                                                                /* if not reentrant */
         fix15    orus[MAX_CTRL_ZONES];   /* Controlled coordinate table (orus) */
#if INCL_RULES
         fix15    pix[MAX_CTRL_ZONES];    /* Controlled coordinate table (sub-pixels) */
         fix15    mult[MAX_INT_ZONES];    /* Interpolation multiplier table */
         fix31    offset[MAX_INT_ZONES];  /* Interpolation offset table */
#endif                                                               /* endif incl_rules */
#endif                                                               /* endif not reentrant */

         fix15    no_X_orus;              /* Number of X controlled coordinates */
         fix15    no_Y_orus;              /* Number of Y controlled coordinates */
         ufix16   Y_constr_org;           /* Origin of constraint table in font data */

#if INCL_RULES
         constr_t constr;                 /* Constraint data state */
         boolean  c_act[MAX_CONSTR];      /* TRUE if constraint currently active */
         fix15    c_pix[MAX_CONSTR];      /* Size of constrained zone if active */
#endif                                                            
#if  INCL_ISW       
         boolean import_setwidth_act;     /* boolean to indicate imported setwidth */
	 boolean isw_modified_constants;
         ufix32 imported_width;		  /* value of imported setwidth */	
	 fix15 isw_xmax;		  /* maximum oru value for constants*/
#endif
#if INCL_SQUEEZING || INCL_ISW
         fix15 setwidth_orus;             /* setwidth value in orus */
	/* bounding box in orus for squeezing */
         fix15 bbox_xmin_orus;	          /* X minimum in orus */
         fix15 bbox_xmax_orus;            /* X maximum in orus */
         fix15 bbox_ymin_orus;            /* Y minimum in orus */
         fix15 bbox_ymax_orus;            /* Y maximum in orus */
#endif
#ifdef INCL_SQUEEZING
         boolean squeezing_compound;       /* flag to indicate a compound character*/
#endif
#ifdef INCL_CLIPPING
	fix31 clip_xmax;
	fix31 clip_ymax;
	fix31 clip_xmin;
	fix31 clip_ymin;
#endif
	} SPEEDO_GLOBALS;

/***********************************************************************************
 *
 *  Speedo global data structure allocation 
 *
 ***********************************************************************************/

#ifdef SET_SPCS
#define EXTERN 
#else
#define EXTERN extern
#endif
#if STATIC_ALLOC
EXTERN SPEEDO_GLOBALS GLOBALFAR sp_globals;
#define sp_intercepts sp_globals
#define sp_plaid sp_globals
#else
#if DYNAMIC_ALLOC
EXTERN SPEEDO_GLOBALS GLOBALFAR *sp_global_ptr;
#define sp_globals (*sp_global_ptr)
#define sp_intercepts (*sp_global_ptr)
#define sp_plaid (*sp_global_ptr)
#else
#if REENTRANT_ALLOC
#define sp_globals (*sp_global_ptr)
#define sp_intercepts (*(*sp_global_ptr).intercepts)
#define sp_plaid (*(*sp_global_ptr).plaid)
#endif
#endif
#endif
#ifdef EXTERN
#undef EXTERN
#endif


/***** PUBLIC FONT HEADER OFFSET CONSTANTS  *****/
#define  FH_FMVER    0      /* U   D4.0 CR LF NULL NULL  8 bytes            */
#define  FH_FNTSZ    8      /* U   Font size (bytes) 4 bytes                */
#define  FH_FBFSZ   12      /* U   Min font buffer size (bytes) 4 bytes     */
#define  FH_CBFSZ   16      /* U   Min char buffer size (bytes) 2 bytes     */
#define  FH_HEDSZ   18      /* U   Header size (bytes) 2 bytes              */
#define  FH_FNTID   20      /* U   Source Font ID  2 bytes                  */
#define  FH_SFVNR   22      /* U   Source Font Version Number  2 bytes      */
#define  FH_FNTNM   24      /* U   Source Font Name  70 bytes               */
#define  FH_MDATE   94      /* U   Manufacturing Date  10 bytes             */
#define  FH_LAYNM  104      /* U   Layout Name  70 bytes                    */
#define  FH_CPYRT  174      /* U   Copyright Notice  78 bytes               */
#define  FH_NCHRL  252      /* U   Number of Chars in Layout  2 bytes       */
#define  FH_NCHRF  254      /* U   Total Number of Chars in Font  2 bytes   */
#define  FH_FCHRF  256      /* U   Index of first char in Font  2 bytes     */
#define  FH_NKTKS  258      /* U   Number of kerning tracks in font 2 bytes */
#define  FH_NKPRS  260      /* U   Number of kerning pairs in font 2 bytes  */
#define  FH_FLAGS  262      /* U   Font flags 1 byte:                       */
                            /*       Bit 0: Extended font                   */
                            /*       Bit 1: not used                        */
                            /*       Bit 2: not used                        */
                            /*       Bit 3: not used                        */
                            /*       Bit 4: not used                        */
                            /*       Bit 5: not used                        */
                            /*       Bit 6: not used                        */
                            /*       Bit 7: not used                        */
#define  FH_CLFGS  263      /* U   Classification flags 1 byte:             */
                            /*       Bit 0: Italic                          */
                            /*       Bit 1: Monospace                       */
                            /*       Bit 2: Serif                           */
                            /*       Bit 3: Display                         */
                            /*       Bit 4: not used                        */
                            /*       Bit 5: not used                        */
                            /*       Bit 6: not used                        */
                            /*       Bit 7: not used                        */
#define  FH_FAMCL  264      /* U   Family Classification 1 byte:            */
                            /*       0:  Don't care                         */
                            /*       1:  Serif                              */
                            /*       2:  Sans serif                         */
                            /*       3:  Monospace                          */
                            /*       4:  Script or calligraphic             */
                            /*       5:  Decorative                         */
                            /*       6-255: not used                        */
#define  FH_FRMCL  265      /* U   Font form Classification 1 byte:         */
                            /*       Bits 0-3 (width type):                 */
                            /*         0-3:   not used                      */
                            /*         4:     Condensed                     */
                            /*         5:     not used                      */
                            /*         6:     Semi-condensed                */
                            /*         7:     not used                      */
                            /*         8:     Normal                        */
                            /*         9:     not used                      */
                            /*        10:     Semi-expanded                 */
                            /*        11:     not used                      */
                            /*        12:     Expanded                      */
                            /*        13-15:  not used                      */
                            /*       Bits 4-7 (Weight):                     */
                            /*         0:   not used                        */
                            /*         1:   Thin                            */
                            /*         2:   Ultralight                      */
                            /*         3:   Extralight                      */
                            /*         4:   Light                           */
                            /*         5:   Book                            */
                            /*         6:   Normal                          */
                            /*         7:   Medium                          */
                            /*         8:   Semibold                        */
                            /*         9:   Demibold                        */
                            /*         10:  Bold                            */
                            /*         11:  Extrabold                       */
                            /*         12:  Ultrabold                       */
                            /*         13:  Heavy                           */
                            /*         14:  Black                           */
                            /*         15-16: not used                      */
#define  FH_SFNTN  266      /* U   Short Font Name  32 bytes                */
#define  FH_SFACN  298      /* U   Short Face Name  16 bytes                */
#define  FH_FNTFM  314      /* U   Font form 14 bytes                       */
#define  FH_ITANG  328      /* U   Italic angle 2 bytes (1/256th deg)       */
#define  FH_ORUPM  330      /* U   Number of ORUs per em  2 bytes           */
#define  FH_WDWTH  332      /* U   Width of Wordspace  2 bytes              */
#define  FH_EMWTH  334      /* U   Width of Emspace  2 bytes                */
#define  FH_ENWTH  336      /* U   Width of Enspace  2 bytes                */
#define  FH_TNWTH  338      /* U   Width of Thinspace  2 bytes              */
#define  FH_FGWTH  340      /* U   Width of Figspace  2 bytes               */
#define  FH_FXMIN  342      /* U   Font-wide min X value  2 bytes           */
#define  FH_FYMIN  344      /* U   Font-wide min Y value  2 bytes           */
#define  FH_FXMAX  346      /* U   Font-wide max X value  2 bytes           */
#define  FH_FYMAX  348      /* U   Font-wide max Y value  2 bytes           */
#define  FH_ULPOS  350      /* U   Underline position 2 bytes               */
#define  FH_ULTHK  352      /* U   Underline thickness 2 bytes              */
#define  FH_SMCTR  354      /* U   Small caps transformation 6 bytes        */
#define  FH_DPSTR  360      /* U   Display sups transformation 6 bytes      */
#define  FH_FNSTR  366      /* U   Footnote sups transformation 6 bytes     */
#define  FH_ALSTR  372      /* U   Alpha sups transformation 6 bytes        */
#define  FH_CMITR  378      /* U   Chemical infs transformation 6 bytes     */
#define  FH_SNMTR  384      /* U   Small nums transformation 6 bytes        */
#define  FH_SDNTR  390      /* U   Small denoms transformation 6 bytes      */
#define  FH_MNMTR  396      /* U   Medium nums transformation 6 bytes       */
#define  FH_MDNTR  402      /* U   Medium denoms transformation 6 bytes     */
#define  FH_LNMTR  408      /* U   Large nums transformation 6 bytes        */
#define  FH_LDNTR  414      /* U   Large denoms transformation 6 bytes      */
                            /*     Transformation data format:              */
                            /*       Y position 2 bytes                     */
                            /*       X scale 2 bytes (1/4096ths)            */
                            /*       Y scale 2 bytes (1/4096ths)            */
#define  SIZE_FW FH_LDNTR + 6  /* size of nominal font header */
#define  EXP_FH_METRES SIZE_FW /* offset to expansion field metric resolution (optional) */



/***** MODE FLAGS CONSTANTS *****/
#define CURVES_OUT     0X0008  /* Output module accepts curves              */
#define BOGUS_MODE     0X0010  /* Linear scaling mode                       */
#define CONSTR_OFF     0X0020  /* Inhibit constraint table                  */
#define IMPORT_WIDTHS  0X0040  /* Imported width mode                       */
#define SQUEEZE_LEFT   0X0100  /* Squeeze left mode                         */
#define SQUEEZE_RIGHT  0X0200  /* Squeeze right mode                        */
#define SQUEEZE_TOP    0X0400  /* Squeeze top mode                          */
#define SQUEEZE_BOTTOM 0X0800  /* Squeeze bottom mode                       */
#define CLIP_LEFT      0X1000  /* Clip left mode                            */
#define CLIP_RIGHT     0X2000  /* Clip right mode                           */
#define CLIP_TOP       0X4000  /* Clip top mode                             */
#define CLIP_BOTTOM    0X8000  /* Clip bottom mode                          */

/***********************************************************************************
 *
 *  Speedo function declarations - use prototypes if available
 *
 ***********************************************************************************/

#if PROTOS_AVAIL
/*  do_char.c functions */
ufix16 sp_get_char_id(PROTO_DECL2 ufix16 char_index);
boolean sp_make_char(PROTO_DECL2 ufix16 char_index);
#if  INCL_ISW       
fix31 sp_compute_isw_scale(PROTO_DECL2);
static boolean sp_do_make_char(PROTO_DECL2 ufix16 char_index);
boolean sp_make_char_isw(PROTO_DECL2 ufix16 char_index, ufix32 imported_width);
static boolean sp_reset_xmax(PROTO_DECL2 fix31 xmax);
#endif
#if INCL_ISW || INCL_SQUEEZING
static void sp_preview_bounding_box(PROTO_DECL2 ufix8 FONTFAR  *pointer,ufix8    format);
#endif

#if INCL_METRICS                 /* Metrics functions supported? */
fix31 sp_get_char_width(PROTO_DECL2 ufix16 char_index);
fix15 sp_get_track_kern(PROTO_DECL2 fix15 track,fix15 point_size);
fix31 sp_get_pair_kern(PROTO_DECL2 ufix16 char_index1,ufix16 char_index2);
boolean sp_get_char_bbox(PROTO_DECL2 ufix16 char_index, bbox_t *bbox);
#endif

static boolean sp_make_simp_char(PROTO_DECL2 ufix8 FONTFAR *pointer,ufix8 format);
static boolean sp_make_comp_char(PROTO_DECL2 ufix8 FONTFAR *pointer);
static ufix8 FONTFAR *sp_get_char_org(PROTO_DECL2 ufix16 char_index,boolean top_level);
static fix15 sp_get_posn_arg(PROTO_DECL2 ufix8 FONTFAR *STACKFAR *ppointer,ufix8 format);
static fix15 sp_get_scale_arg(PROTO_DECL2 ufix8 FONTFAR *STACKFAR *ppointer,ufix8 format);

/* do_trns.c functions */
ufix8 FONTFAR *sp_read_bbox(PROTO_DECL2 ufix8 FONTFAR *pointer,point_t STACKFAR *pPmin,point_t STACKFAR *pPmax,boolean set_flag);
void sp_proc_outl_data(PROTO_DECL2 ufix8 FONTFAR *pointer);
static void sp_split_curve(PROTO_DECL2 point_t P1,point_t P2,point_t P3,fix15 depth);
static ufix8 FONTFAR *sp_get_args(PROTO_DECL2 ufix8 FONTFAR *pointer,ufix8  format,point_t STACKFAR *pP);

/* out_blk.c functions */
#if INCL_BLACK
boolean sp_init_black(PROTO_DECL2 specs_t GLOBALFAR *specsarg);
boolean sp_begin_char_black(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax);
void sp_begin_contour_black(PROTO_DECL2 point_t P1,boolean outside);
void sp_line_black(PROTO_DECL2 point_t P1);
boolean sp_end_char_black(PROTO_DECL1);

static void sp_add_intercept_black(PROTO_DECL2 fix15 y, fix15 x);
static void sp_proc_intercepts_black(PROTO_DECL1);
#endif

/* out_scrn.c functions */
#if INCL_SCREEN
boolean sp_init_screen(PROTO_DECL2 specs_t GLOBALFAR *specsarg);
boolean sp_begin_char_screen(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax);
void sp_begin_contour_screen(PROTO_DECL2 point_t P1,boolean outside);
void sp_curve_screen(PROTO_DECL2 point_t P1,point_t P2,point_t P3, fix15 depth);
void sp_scan_curve_screen(PROTO_DECL2 fix31 X0,fix31 Y0,fix31 X1,fix31 Y1,fix31 X2,fix31 Y2,fix31 X3,fix31 Y3);
void sp_vert_line_screen(PROTO_DECL2   fix31 x, fix15 y1, fix15 y2);
void sp_line_screen(PROTO_DECL2 point_t P1);
void sp_end_contour_screen(PROTO_DECL1);
boolean sp_end_char_screen(PROTO_DECL1);

static void sp_add_intercept_screen(PROTO_DECL2 fix15 y,fix31 x);
static void sp_proc_intercepts_screen(PROTO_DECL1);
#endif

/* out_outl.c functions */
#if INCL_OUTLINE
#if INCL_MULTIDEV
boolean sp_set_outline_device(PROTO_DECL2 outline_t *ofuncs, ufix16 size);
#endif


boolean sp_init_outline(PROTO_DECL2 specs_t GLOBALFAR *specsarg);
boolean sp_begin_char_outline(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax);
void sp_begin_sub_char_outline(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax);
void sp_begin_contour_outline(PROTO_DECL2 point_t P1,boolean outside);
void sp_curve_outline(PROTO_DECL2 point_t P1,point_t P2,point_t P3, fix15 depth);
void sp_line_outline(PROTO_DECL2 point_t P1);
void sp_end_contour_outline(PROTO_DECL1);
void sp_end_sub_char_outline(PROTO_DECL1);
boolean sp_end_char_outline(PROTO_DECL1);
#endif

/* out_bl2d.c functions */
#if INCL_2D
boolean sp_init_2d(PROTO_DECL2 specs_t GLOBALFAR *specsarg);
boolean sp_begin_char_2d(PROTO_DECL2 point_t Psw,point_t Pmin,point_t Pmax);
void sp_begin_contour_2d(PROTO_DECL2 point_t P1,boolean outside);
void sp_line_2d(PROTO_DECL2 point_t P1);
boolean sp_end_char_2d(PROTO_DECL1);

static void sp_draw_vector_to_2d(PROTO_DECL2 fix15 x0,fix15 y0,fix15 x1,fix15 y1,band_t GLOBALFAR *band);
static void sp_add_intercept_2d(PROTO_DECL2 fix15 y,fix15 x);
static void sp_proc_intercepts_2d(PROTO_DECL1);
#endif

/* out_util.c functions */
#if INCL_BLACK || INCL_SCREEN || INCL_2D
        
#if INCL_MULTIDEV
boolean sp_set_bitmap_device(PROTO_DECL2 bitmap_t *bfuncs, ufix16 size);
#endif

void sp_init_char_out(PROTO_DECL2 point_t Psw, point_t Pmin, point_t Pmax);
void sp_begin_sub_char_out(PROTO_DECL2 point_t Psw, point_t Pmin, point_t Pmax);
void sp_curve_out(PROTO_DECL2 point_t P1, point_t P2, point_t P3, fix15 depth);
void sp_end_contour_out(PROTO_DECL1);
void sp_end_sub_char_out(PROTO_DECL1);
void sp_init_intercepts_out(PROTO_DECL1);
void sp_restart_intercepts_out(PROTO_DECL1);
void sp_set_first_band_out(PROTO_DECL2 point_t Pmin, point_t Pmax);
void sp_reduce_band_size_out(PROTO_DECL1);
boolean sp_next_band_out(PROTO_DECL1);
#endif

#if INCL_USEROUT
boolean sp_init_userout(specs_t *specsarg);
#endif


/* reset.c functions */
void sp_reset(PROTO_DECL1);
#if INCL_KEYS
void sp_set_key(PROTO_DECL2 ufix8 key[]);
#endif
ufix16 sp_get_cust_no(PROTO_DECL2 buff_t font_buff);

/* set_spcs.c functions */
boolean sp_set_specs(PROTO_DECL2 specs_t STACKFAR *specsarg);
void sp_type_tcb(PROTO_DECL2 tcb_t GLOBALFAR *ptcb);

static boolean sp_setup_consts(PROTO_DECL2 fix15 xmin, fix15 xmax,
	fix15 ymin, fix15 ymax);
static void sp_setup_tcb(PROTO_DECL2 tcb_t GLOBALFAR *ptcb);
static fix15 sp_setup_mult(PROTO_DECL2 fix31 input_mult);
static fix31 sp_setup_offset(PROTO_DECL2 fix31 input_offset);
fix31 sp_read_long(PROTO_DECL2 ufix8 FONTFAR *pointer);
fix15 sp_read_word_u(PROTO_DECL2 ufix8 FONTFAR *pointer);

/* set_trns.c functions */
void sp_init_tcb(PROTO_DECL1);
void sp_scale_tcb(PROTO_DECL2 tcb_t GLOBALFAR *ptcb,fix15 x_pos,fix15 y_pos,fix15 x_scale,fix15 y_scale);
ufix8 FONTFAR *sp_plaid_tcb(PROTO_DECL2 ufix8 FONTFAR *pointer,ufix8 format);
ufix8 FONTFAR *sp_skip_interpolation_table(PROTO_DECL2 ufix8 FONTFAR *pointer, ufix8 format);
ufix8 FONTFAR *sp_skip_control_zone(PROTO_DECL2 ufix8 FONTFAR *pointer, ufix8 format);

static void sp_constr_update(PROTO_DECL1);
ufix8 FONTFAR *sp_read_oru_table(PROTO_DECL2 ufix8 FONTFAR *pointer);
#if INCL_SQUEEZING || INCL_ISW
static void sp_calculate_x_pix(PROTO_DECL2 ufix8 start_edge,ufix8 end_edge,ufix16 constr_nr,fix31 x_scale,fix31 x_offset,fix31 ppo,fix15 setwidth_pix);
#endif
#if INCL_SQUEEZING
static void sp_calculate_y_pix(PROTO_DECL2 ufix8 start_edge,ufix8 end_edge,ufix16 constr_nr,fix31 top_scale,fix31 bottom_scale,fix31 ppo,fix15 emtop_pix,fix15 embot_pix);
boolean sp_calculate_x_scale(PROTO_DECL2 fix31 *x_factor,fix31 *x_offset,fix15 no_x_ctrl_zones);
boolean sp_calculate_y_scale(PROTO_DECL2 fix31 *top_scale,fix31 *bottom_scale,fix15 first_y_zone, fix15 no_Y_ctrl_zones);
#endif
static ufix8 FONTFAR *sp_setup_pix_table(PROTO_DECL2 ufix8 FONTFAR *pointer,boolean short_form,fix15 no_X_ctrl_zones,fix15 no_Y_ctrl_zones);
static ufix8 FONTFAR *sp_setup_int_table(PROTO_DECL2 ufix8 FONTFAR *pointer,fix15 no_X_int_zones,fix15 no_Y_int_zones);
                  

/* user defined functions */

void sp_report_error(PROTO_DECL2 fix15 n);

#if INCL_BLACK || INCL_SCREEN || INCL_2D
void sp_open_bitmap(PROTO_DECL2 fix31 x_set_width, fix31 y_set_width, fix31 xorg, fix31 yorg, fix15 xsize,fix15 ysize);
void sp_set_bitmap_bits(PROTO_DECL2 fix15 y, fix15 xbit1, fix15 xbit2);
void sp_close_bitmap(PROTO_DECL1);
#endif

#if INCL_OUTLINE
void sp_open_outline(PROTO_DECL2 fix31 x_set_width, fix31 y_set_width, fix31 xmin, fix31 xmax, fix31 ymin,fix31 ymax);
void sp_start_new_char(PROTO_DECL1);
void sp_start_contour(PROTO_DECL2 fix31 x,fix31 y,boolean outside);
void sp_curve_to(PROTO_DECL2 fix31 x1, fix31 y1, fix31 x2, fix31 y2, fix31 x3, fix31 y3);
void sp_line_to(PROTO_DECL2 fix31 x, fix31 y);
void sp_close_contour(PROTO_DECL1);
void sp_close_outline(PROTO_DECL1);
#endif

#if INCL_LCD                     /* Dynamic load character data supported? */
buff_t *sp_load_char_data(PROTO_DECL2 fix31 file_offset,fix15 no_bytes,fix15 cb_offset);        /* Load character data from font file */
#endif

#if INCL_PLAID_OUT               /* Plaid data monitoring included? */
void   sp_record_xint(PROTO_DECL2 fix15 int_num);            /* Record xint data */
void   sp_record_yint(PROTO_DECL2 fix15 int_num);            /* Record yint data */
void sp_begin_plaid_data(PROTO_DECL1);         /* Signal start of plaid data */
void sp_begin_ctrl_zones(PROTO_DECL2 fix15, no_X_zones, fix15 no_Y_zones);         /* Signal start of control zones */
void sp_record_ctrl_zone(PROTO_DECL2 fix31 start, fix31 end, fix15 constr);         /* Record control zone data */
void sp_begin_int_zones(PROTO_DECL2 fix15 no_X_int_zones, fix15 no_Y_int_zones);          /* Signal start of interpolation zones */
void sp_record_int_zone(PROTO_DECL2 fix31 start, fix31 end);          /* Record interpolation zone data */
void sp_end_plaid_data(PROTO_DECL1);           /* Signal end of plaid data */
#endif

#else   /* NO PROTOTYPES AVAILABLE */



/*  do_char.c functions */
boolean sp_make_char();          /* Make specified character */
#if  INCL_ISW       
fix31 sp_compute_isw_scale();
static boolean sp_do_make_char();
boolean sp_make_char_isw();     /* Make specified character with */
                                /* imported set widths.*/
static boolean sp_reset_xmax();
#endif
#if INCL_ISW || INCL_SQUEEZING
static void sp_preview_bounding_box();
#endif
ufix16  sp_get_char_id();        /* Get character id for specified char */

#if INCL_METRICS                 /* Metrics functions supported? */
fix31   sp_get_char_width();     /* Get character width for specified char */
fix15   sp_get_track_kern();     /* Get track kerning for specified size */
fix31   sp_get_pair_kern();      /* Get kerning for specified char pair */
boolean sp_get_char_bbox();
#endif

static boolean sp_make_simp_char(); /* Process simple character data */
static boolean sp_make_comp_char(); /* Process compound character data */
static ufix8 FONTFAR *sp_get_char_org();   /* Look up char in character directory */
static fix15   sp_get_posn_arg();   /* Read Xpos Ypos args in DOCH instruction */
static fix15   sp_get_scale_arg();  /* read Xscale Yscale args in DOCH instruction */

/* do_trns.c functions */
ufix8 FONTFAR *sp_read_bbox();              /* Read bounding box */
void   sp_proc_outl_data();         /* Process outline data */
static void   sp_split_curve();            /* Split Bezier curve into vectors */
static ufix8 FONTFAR *sp_get_args();      /* Read X Y argument pair */

/* out_0c.c functions */
boolean sp_init_black();
boolean sp_begin_char_black();
void sp_begin_contour_black();
void sp_line_black();
boolean sp_end_char_black();

static void    sp_add_intercept_black();
static void    sp_proc_intercepts_black();

/* out_util.c functions */
#if INCL_BLACK || INCL_SCREEN || INCL_2D

void sp_init_char_out();
void sp_begin_sub_char_out();
void sp_curve_out();
void sp_end_contour_out();
void sp_end_sub_char_out();
void sp_init_intercepts_out();
void sp_restart_intercepts_out();
void sp_set_first_band_out();
void sp_reduce_band_size_out();
boolean sp_next_band_out();
#endif

#if INCL_USEROUT
boolean sp_init_userout();
#endif

/* reset.c functions */
void sp_reset();                 /* Initialize Fontware mechanism */

#if INCL_KEYS
void sp_set_key();
#endif
ufix16 sp_get_cust_no();

/* set_spcs.c functions */
boolean    sp_set_specs();       /* Set specifications */
void       sp_type_tcb();           /* Update transformation class in tcb */

static void    sp_setup_tcb();      /* Set up transformation control block */
static fix15 sp_setup_mult();       /* Convert mult to internal form */
static fix31 sp_setup_offset();     /* Convert offset to internal form */
static boolean sp_setup_consts();   /* Set up scaling constants */
fix31   sp_read_long();      /* Read long as 3 bytes encrypted */
fix15   sp_read_word_u();    /* Read word as 2 bytes unencrypted */

/* set_trns.c functions */
void    sp_init_tcb();              /* Initialize current trans control block */
void    sp_scale_tcb();             /* Transform trans control block */
ufix8 FONTFAR *sp_plaid_tcb();             /* Enable intelligent transformation */
ufix8 FONTFAR *sp_skip_interpolation_table();
ufix8 FONTFAR *sp_skip_control_zone();

static void sp_constr_update();     /* Update constraint table */
ufix8 FONTFAR *sp_read_oru_table();    /* Read controlled coord table */
#if INCL_SQUEEZING || INCL_ISW
static void sp_calculate_x_pix();
#endif
#if INCL_SQUEEZING
static void sp_calculate_y_pix();
boolean sp_calculate_x_scale();
boolean sp_calculate_y_scale() ;
#endif
static ufix8 FONTFAR *sp_setup_pix_table();   /* Read control zone table */
static ufix8 FONTFAR *sp_setup_int_table();   /* Read interpolation zone table */

/* user defined functions */

#if INCL_BLACK || INCL_SCREEN || INCL_2D
void sp_open_bitmap();
void sp_set_bitmap_bits();
void sp_close_bitmap();
#endif

#if INCL_OUTLINE
void sp_open_outline();
void sp_start_new_char();
void sp_start_contour();
void sp_curve_to();
void sp_line_to();
void sp_close_contour();
void sp_close_outline();
#endif

#if INCL_LCD                     /* Dynamic load character data supported? */
buff_t *sp_load_char_data();        /* Load character data from font file */
#endif

#if INCL_PLAID_OUT               /* Plaid data monitoring included? */
void   sp_record_xint();            /* Record xint data */
void   sp_record_yint();            /* Record yint data */
void sp_begin_plaid_data();         /* Signal start of plaid data */
void sp_begin_ctrl_zones();         /* Signal start of control zones */
void sp_record_ctrl_zone();         /* Record control zone data */
void sp_begin_int_zones();          /* Signal start of interpolation zones */
void sp_record_int_zone();          /* Record interpolation zone data */
void sp_end_plaid_data();           /* Signal end of plaid data */
#endif

boolean sp_init_screen();        /* If only screen-writer mode supported */
boolean sp_begin_char_screen();      /* If screenwriter mode supported */
void    sp_begin_contour_screen();   /* If screenwriter mode supported */
void    sp_line_screen();            /* If screenwriter mode supported */
void    sp_curve_screen();           /* If screenwriter mode supported */
void sp_scan_curve_screen();
void sp_vert_line_screen();
void sp_end_contour_screen();
boolean sp_end_char_screen();        /* If screenwriter mode supported */

static void    sp_add_intercept_screen();
static void    sp_proc_intercepts_screen();


boolean sp_init_outline();        /* If only vector output mode supported */
boolean sp_begin_char_outline();      /* If only vector output mode supported */
void    sp_begin_sub_char_outline();  /* If only vector output mode supported */
void    sp_begin_contour_outline();   /* If only vector output mode supported */
void    sp_curve_outline();           /* If only vector output mode supported */
void    sp_line_outline();            /* If only vector output mode supported */
void    sp_end_contour_outline();     /* If only vector output mode supported */
void    sp_end_sub_char_outline();    /* If only vector output mode supported */
boolean sp_end_char_outline();        /* If only vector output mode supported */

boolean sp_init_2d();       /* If screen-writer and other modes supported */
boolean sp_begin_char_2d();     /* If screen-writer and other modes supported */
void    sp_begin_contour_2d();  /* If screen-writer and other modes supported */
void    sp_line_2d();           /* If screen-writer and other modes supported */
boolean sp_end_char_2d();       /* If screen-writer and other modes supported */

static void    sp_add_intercept_2d();
static void    sp_proc_intercepts_2d();
static void    sp_draw_vector_to_2d();

#endif



#endif /* _SPEEDO_H_ */
