


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
*                                                                                    *
*     $Header: //toklas/archive/rcs/speedo/do_char.c,v 22.1 91/01/23 17:15:40 leeann Release $                                                                       *
*                                                                                    *
*     $Log:	do_char.c,v $
*       Revision 22.1  91/01/23  17:15:40  leeann
*       Release
*       
*       Revision 21.2  91/01/21  18:04:43  leeann
*       make pointer in get_char_org a FONTFAR
*       
*       Revision 21.1  90/11/20  14:35:42  leeann
*       Release
*       
*       Revision 20.1  90/11/12  09:20:07  leeann
*       Release
*       
*       Revision 19.1  90/11/08  10:17:59  leeann
*       Release
*       
*       Revision 18.2  90/11/06  19:02:02  leeann
*       fix bugs when combining imported setwidth and squeezing
*       
*       Revision 18.1  90/09/24  09:50:30  mark
*       Release
*       
*       Revision 17.4  90/09/19  18:10:20  leeann
*       make preview_bounding_box visible when squeezing
*       
*       Revision 17.3  90/09/17  10:49:00  mark
*       Change INCL_WHITE conditional for allocation of sp_intercepts 
*       to INCL_SCREEN, so that it is used.
*       
*       Revision 17.2  90/09/14  15:01:06  leeann
*       changes for imported setwidth = 1 into src area
*       
*       Revision 17.1  90/09/13  15:56:56  mark
*       Release name rel0913
*       
*       Revision 16.2  90/09/13  15:13:55  leeann
*       allow imported widths of zero
*       
*       Revision 16.1  90/09/11  12:54:03  mark
*       Release
*       
*       Revision 15.2  90/09/05  11:29:03  leeann
*       recalculate constants when necessary for precision
*       with imported setwidths
*       
*       Revision 15.1  90/08/29  10:02:25  mark
*       Release name rel0829
*       
*       Revision 14.2  90/08/23  16:14:17  leeann
*       for imported set width, check if the imported width
*       is greater than the max in the font, and if it is,
*       reset the maximum.
*       
*       Revision 14.1  90/07/13  10:37:50  mark
*       Release name rel071390
*       
*       Revision 13.1  90/07/02  10:36:48  mark
*       Release name REL2070290
*       
*       Revision 12.3  90/06/26  08:55:59  leeann
*       compute squeezed bounding box for composite characters
*       before the characters are generated
*       
*       Revision 12.2  90/06/06  18:05:43  leeann
*       Correct parameter to sp_make_char_isw
*       
*       Revision 12.1  90/04/23  12:11:10  mark
*       Release name REL20
*       
*       Revision 11.1  90/04/23  10:11:05  mark
*       Release name REV2
*       
*       Revision 10.10  90/04/23  09:42:14  mark
*       fix declaration of do_make_char to match reentrant requirements
*       
*       Revision 10.9  90/04/21  10:47:12  mark
*       if multidevice support is enabled, make sure that the
*       device is specified before imaging characters.
*       
*       Revision 10.8  90/04/18  10:53:58  mark
*       add function sp_get_char_bbox
*       
*       Revision 10.7  90/04/17  09:32:51  leeann
*       make imported setwidth parameter 1/65536 of a pixel
*       
*       Revision 10.6  90/04/11  13:07:15  leeann
*       change squeezing compilation option to INCL_SQUEEZING,
*       include make_char_isw function
*       
*       Revision 10.5  90/04/05  15:14:24  leeann
*       set squeezing_compound flag apropriately
*       
*       Revision 10.4  90/03/29  16:43:16  leeann
*       Added set_flags argument to read_bbox
*       Added SQUEEZE code to make_simp_char to read_bbox
*       before calling plaid_tcb
*       
*       Revision 10.3  90/03/26  15:48:42  mark
*       calculate subpixel setwidth values for normal and compound characters
*       using metric_resolution and specs.xxmult
*       also change get_char_width similarly
*       
*       Revision 10.2  89/09/11  11:39:10  mark
*       correct declaration of stackfar pointer to fontfar pointer argument
*       in functions sp_get_posn_arg and sp_get_scale_arg so that code works
*       with Microsoft C when stackfar and fontfar are not equivalent.
*       *** EOF *** 
*       
*       Revision 10.1  89/07/28  18:07:36  mark
*       Release name PRODUCT
*       
*       Revision 9.1  89/07/27  10:20:41  mark
*       Release name PRODUCT
*       
*       Revision 8.1  89/07/13  18:17:24  mark
*       Release name Product
*       
*       Revision 7.1  89/07/11  08:58:56  mark
*       Release name PRODUCT
*       
*       Revision 6.2  89/07/09  11:37:08  mark
*       only allocate intercept lists if one of the bitmap output
*       modules is included
*       
*       Revision 6.1  89/06/19  08:33:00  mark
*       Release name prod
*       
*       Revision 5.3  89/06/02  16:58:09  mark
*       In the reentrant model, allocate space for intercept
*       lists and plaid tables on the stack
*       
*       Revision 5.2  89/06/01  16:56:04  mark
*       only process outline data if begin_char returns TRUE
*       
*       Revision 5.1  89/05/01  17:50:55  mark
*       Release name Beta
*       
*       Revision 4.2  89/05/01  16:17:51  john
*       bug in get_char_org() corrected:
*       16 bit offsets in char directory now cast to unsigned
*       before being cast into fix31.
*       
*       Revision 4.1  89/04/27  12:11:30  mark
*       Release name Beta
*       
*       Revision 3.1  89/04/25  08:24:27  mark
*       Release name beta
*       
*       Revision 2.5  89/04/24  15:52:48  john
*       Two pixel allowance added to compound char
*       bounding box.
*       
*       Revision 2.4  89/04/12  13:17:24  mark
*       correct far pointer declarations of get_posn_args 
*       and get_scale_args
*       
*       Revision 2.3  89/04/12  12:10:52  mark
*       added stuff for far stack and font
*       
*       Revision 2.2  89/04/10  17:06:57  mark
*       Modified pointer declarations that are used to refer
*       to font data to use FONTFAR symbol, which will be used
*       for Intel SS != DS memory models
*       Also changed use of read_bbox and plaid_tcb to return 
*       updated pointer, rather than passing a pointer to a pointer
*       
*       Revision 2.1  89/04/04  13:31:13  mark
*       Release name EVAL
*       
*       Revision 1.10  89/04/04  13:16:21  mark
*       Update copyright text
*       
*       Revision 1.9  89/03/31  14:42:23  mark
*       Change speedo.h to spdo_prv.h
*       
*       Revision 1.8  89/03/31  12:14:14  john
*       modified to use new NEXT_WORD macro.
*       
*       Revision 1.7  89/03/30  17:47:21  john
*       read_long() moved to set_spcs.c
*       
*       Revision 1.6  89/03/29  16:07:47  mark
*       changes for slot independence and dynamic/reentrant
*       data allocation
*       
*       Revision 1.5  89/03/24  16:48:01  john
*       Direct access to character directory implemented.
*       
*       Revision 1.4  89/03/23  17:51:21  john
*       Added expansion joint to character header data
*       
*       Revision 1.3  89/03/23  11:50:59  john
*       Dynamic char data loader modified to correct compound character
*       bug. New offset arg added to load_char_data()
*       
*       Revision 1.2  89/03/21  13:25:27  mark
*       change name from oemfw.h to speedo.h
*       
*       Revision 1.1  89/03/15  12:28:44  mark
*       Initial revision
*                                                                                 *
*                                                                                    *
*************************************************************************************/

#ifdef RCSSTATUS
static char rcsid[] = "$Header: //toklas/archive/rcs/speedo/do_char.c,v 22.1 91/01/23 17:15:40 leeann Release $";
#endif



/***************************** D O - C H A R . C *****************************
 *                                                                           *
 * This is the top level module for processing one simple or composite       *
 * character.
 *                                                                           *
 ********************** R E V I S I O N   H I S T O R Y **********************
 *                                                                           *
 *  1) 15 Dec 88  jsc  Created                                               *
 *                                                                           *
 *  2) 27 Jan 89  jsc  fw_get_char_id() added.                               *
 *                     fw_get_char_width() added.                            *
 *                     make_simple_char() and make_comp_char() changed from  *
 *                     void to boolean to return FALSE if error              *
 *                                                                           *
 *  3) 30 Jan 89  jsc  fw_get_char_width() changed from ufix16 to fix31      *
 *                                                                           *
 *  4)  6 Feb 89  jsc  Conditional compilation of metrics functions.         *
 *                                                                           *
 *                     Conditional compilation of dynamic character data     *
 *                     loading.                                              *
 *                                                                           *
 *  5)  9 Feb 89  jsc  Kerning access functions added.                       *
 *                                                                           *
 *  6)  2 Mar 89  jsc  Corrected char index bounds checking bug.             *
 *                                                                           *
 ****************************************************************************/

#include "spdo_prv.h"               /* General definitions for Speedo    */

#define   DEBUG   0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

/***** GLOBAL VARIABLES *****/

/*****  GLOBAL FUNCTIONS *****/

/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/

/***** STATIC FUNCTIONS *****/


FUNCTION ufix16 get_char_id(char_index)
GDECL
ufix16 char_index;     /* Index to character in char directory */
/*
 * Returns character id for specified character index in currently
 * selected font.
 * Reports Error 10 and returns 0 if no font selected.
 * Reports Error 12 and returns 0 if character data not available.
 */
{
ufix8 FONTFAR  *pointer;      /* Pointer to character data */

if (!sp_globals.specs_valid)     /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return (ufix16)0;            /* Return zero character id */
    }

pointer = get_char_org(char_index, TRUE); /* Get pointer to character data */
if (pointer == NULL)             /* Character data not available? */
    {
    report_error(12);            /* Report character data not avail */
    return (ufix16)0;            /* Return zero character id */
    }

return (ufix16)NEXT_WORD(pointer); /* Return character id */
}


#if INCL_METRICS
FUNCTION fix31 get_char_width(char_index)
GDECL
ufix16 char_index;     /* Index to character in char directory */
/*
 * Returns character set width for specified character index in currently
 * selected font in units of 1/65536 em.
 * Reports Error 10 and returns 0 if no font selected.
 * Reports Error 12 and returns 0 if character data not available.
 */
{
ufix8 FONTFAR  *pointer;      /* Pointer to character data */
fix31    set_width;    /* Set width of character */

if (!sp_globals.specs_valid)                /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return (fix31)0;             /* Return zero character width */
    }

pointer = get_char_org(char_index, TRUE); /* Get pointer to character data */
if (pointer == NULL)             /* Character data not available? */
    {
    report_error(12);            /* Report character data not avail */
    return (fix31)0;             /* Return zero character width */
    }

pointer += 2;                    /* Skip over character id */
set_width = (fix31)NEXT_WORD(pointer); /* Read set width  and Convert units */
set_width = ((set_width << 16) + (sp_globals.metric_resolution >> 1)) / sp_globals.metric_resolution;
return set_width;                /* Return in 1/65536 em units */
}
#endif

#if INCL_METRICS
FUNCTION fix15 get_track_kern(track, point_size)
GDECL
fix15  track;          /* Track required (0 - 3) */
fix15  point_size;     /* Point size (units of whole points) */
/*
 * Returns inter-character spacing adjustment in units of 1/256
 * points for the specified kerning track and point size.
 * If the specified point size is larger than the maximum point
 * size for the specified track, the adjustment for the maximum
 * point size is used.
 * If the specified point size is smaller than the minimum point
 * size for the specified track, the adjustment for the minimum
 * point size is used.
 * If the specified point size is between the minimum point size
 * and the maximum point size for the specified track, the 
 * adjustment is interpolated linearly between the minimum and
 * maximum adjustments.
 * Reports Error 10 and returns 0 if no font selected.
 * Reports Error 13 and returns 0 if track kerning data not in font.
 */
{
ufix8 FONTFAR   *pointer;      /* Pointer to character data */
fix15    no_tracks;    /* Number of kerning tracks in font */
ufix8    format;       /* Track kerning format byte */
fix15    i;            /* Track counter */
fix15    min_pt_size;  /* Minimum point size for track */
fix15    max_pt_size;  /* Maximum point size for track */
fix15    min_adj;      /* Adjustment for min point size */
fix15    max_adj;      /* Adjustment for max point size */
fix31    delta_pt_size;/* Max point size - min point size */
fix31    delta_adj;    /* Min adjustment - max adjustment */
fix15    adj = 0;      /* Interpolated adjustment */

if (track == 0)                  /* Track zero selected? */
    {
    return adj;                  /* Return zero track kerning adjustment */
    }

if (!sp_globals.specs_valid)                /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return adj;                  /* Return zero track kerning adjustment */
    }

no_tracks = sp_globals.kern.no_tracks;      /* Number of kerning tracks */
if (track > no_tracks)           /* Required track not available? */
    {
    report_error(13);            /* Report track kerning data not avail */
    return adj;                  /* Return zero track kerning adjustment */
    }

pointer =  sp_globals.kern.tkorg;            /* Point to start of track kern data */
for (i = 0; i < track; i++)      /* Read until track required is read */
    {
    format = NEXT_BYTE(pointer); /* Read track kerning format byte */
    min_pt_size = (format & BIT0)? 
        NEXT_WORD(pointer):
        (fix15)NEXT_BYTE(pointer);
    min_adj = (format & BIT1)? 
        NEXT_WORD(pointer):
        (fix15)NEXT_BYTE(pointer);
    max_pt_size = (format & BIT2)? 
        NEXT_WORD(pointer):
        (fix15)NEXT_BYTE(pointer);
    max_adj = (format & BIT3)? 
        NEXT_WORD(pointer):
        (fix15)NEXT_BYTE(pointer);
    }

if (point_size <= min_pt_size)   /* Smaller than minimum point size? */
    {
    return min_adj;              /* Return minimum adjustment (1/256 points) */
    }

if (point_size >= max_pt_size)   /* Larger than maximum point size? */
    {
    return max_adj;              /* Return maximum adjustment (1/256 points) */
    }

delta_pt_size = (fix31)(max_pt_size - min_pt_size);
delta_adj = (fix31)(min_adj - max_adj);
adj = (fix15)(min_adj - 
       (((fix31)(point_size - min_pt_size) * delta_adj + 
         (delta_pt_size >> 1)) / delta_pt_size));
return adj;                      /* Return interpolated adjustment (1/256 points) */
}
#endif

#if INCL_METRICS
FUNCTION fix31 get_pair_kern(char_index1, char_index2)
GDECL
ufix16 char_index1;    /* Index to first character in char directory */
ufix16 char_index2;    /* Index to second character in char directory */
/*
 * Returns inter-character spacing adjustment in units of 1/65536 em
 * for the specified pair of characters.
 * Reports Error 10 and returns 0 if no font selected.
 * Reports Error 14 and returns 0 if pair kerning data not in font.
 */
{
ufix8 FONTFAR  *origin;       /* Pointer to first kerning pair record */
ufix8 FONTFAR  *pointer;      /* Pointer to character data */
ufix16   tmpufix16;    /* Temporary workspace */
fix15    no_pairs;     /* Number of kerning pairs in font */
ufix8    format;       /* Track kerning format byte */
boolean  long_id;      /* TRUE if 2-byte character ids */
fix15    rec_size;     /* Number of bytes in kern pair record */
fix15    n;            /* Number of remaining kern pairs */
fix15    nn;           /* Number of kern pairs in first partition */
fix15    base;         /* Index to first record in rem kern pairs */
fix15    i;            /* Index to kern pair being tested */
fix31    adj = 0;      /* Returned value of adjustment */
fix15    adj_base;     /* Adjustment base for relative adjustments */

if (!sp_globals.specs_valid)                /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return adj;                  /* Return zero pair kerning adjustment */
    }

no_pairs = sp_globals.kern.no_pairs;        /* Number of kerning pairs */
if (no_pairs == 0)               /* Pair kerning data not available? */
    {
    report_error(14);            /* Report pair kerning data not avail */
    return adj;                  /* Return zero pair kerning adjustment */
    }

pointer = sp_globals.kern.pkorg;            /* Point to start of pair kern data */
format = NEXT_BYTE(pointer);     /* Read pair kerning format byte */
if (!(format & BIT0))            /* One-byte adjustment values? */
    adj_base = NEXT_WORD(pointer); /* Read base adjustment */
origin = pointer;                /* First byte of kerning pair data */
rec_size = format + 3;           /* Compute kerning pair record size */
long_id = format & BIT1;         /* Set flag for 2-byte char index */

n = no_pairs;                    /* Consider all kerning pairs */
base = 0;                        /* Set base at first kern pair record */
while (n != 0)                   /* While 1 or more kern pairs remain ... */
    {
    nn = n >> 1;                 /* Size of first partition */
    i = base + nn;               /* Index to record to be tested */
    pointer = origin + (i * rec_size);
    tmpufix16 = NEXT_CHNDX(pointer, long_id);
    if (char_index1 < tmpufix16)
        {
        n = nn;                  /* Number remaining in first partition */
        continue;
        }
    if (char_index1 > tmpufix16)
        {
        n -= nn + 1;             /* Number remaining in second partition */
        base = i + 1;            /* Base index for second partition */
        continue;
        }
    tmpufix16 = NEXT_CHNDX(pointer, long_id);
    if (char_index2 < tmpufix16)
        {
        n = nn;                  /* Number remaining in first partition */
        continue;
        }
    if (char_index2 > tmpufix16)
        {
        n -= nn + 1;             /* Number remaining in second partition */
        base = i + 1;            /* Base index for second partition */
        continue;
        }
    adj = (format & BIT0)? 
        (fix31)NEXT_WORD(pointer):
        (fix31)(adj_base + (fix15)NEXT_BYTE(pointer));
    adj = ((adj << 16) + (sp_globals.orus_per_em >> 1)) / sp_globals.orus_per_em; /* Convert units */
    n = 0;                       /* No more to consider */
    }
return adj;                      /* Return pair kerning adjustment */
}
#endif


#if INCL_METRICS
#ifdef old
FUNCTION boolean get_char_bbox(char_index, bbox)
GDECL
ufix16 char_index;
bbox_t *bbox;
{
/*
 *	returns true if character exists, false if it doesn't
 *	provides transformed character bounding box in 1/65536 pixels
 *	in the provided bbox_t structure.  Bounding box may be
 *	conservative in the event that the transformation is not
 *	normal or the character is compound.
 */

ufix8 FONTFAR *pointer;
fix15 tmp;
point_t Pmin, Pmax;

#if REENTRANT_ALLOC
plaid_t plaid;
sp_globals.plaid = &plaid;
#endif

if (!sp_globals.specs_valid)                /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return FALSE;                /* Error return */
    }

init_tcb();                      /* Initialize transformation control block */

pointer = get_char_org(char_index, TRUE); /* Point to start of character data */
if (pointer == NULL)             /* Character data not available? */
    {
    report_error(12);            /* Report character data not avail */
    return FALSE;                /* Error return */
    }

pointer += 2;                    /* Skip over character id */
tmp = NEXT_WORD(pointer); /* Read set width */
               
tmp = NEXT_BYTE(pointer);
if (tmp & BIT1)               /* Optional data in header? */
    {
    tmp = (ufix8)NEXT_BYTE(pointer); /* Read size of optional data */
    pointer += tmp;         /* Skip optional data */
    }

pointer = plaid_tcb(pointer, tmp);              /* Process plaid data */
pointer = read_bbox(pointer, &Pmin, &Pmax,(boolean)FALSE);        /* Read bounding box */
bbox->xmin  = (fix31)Pmin.x << sp_globals.poshift;
bbox->xmax  = (fix31)Pmax.x << sp_globals.poshift;
bbox->ymin  = (fix31)Pmin.y << sp_globals.poshift;
bbox->ymax  = (fix31)Pmax.y << sp_globals.poshift;
return TRUE;
}

#else /* new code, 4/25/91 */

FUNCTION boolean get_char_bbox(char_index, bbox)
GDECL
ufix16 char_index;
bbox_t *bbox;
{
/*
 *	returns true if character exists, false if it doesn't
 *	provides transformed character bounding box in 1/65536 pixels
 *	in the provided bbox_t structure.  Bounding box may be
 *	conservative in the event that the transformation is not
 *	normal or the character is compound.
 */

ufix8 FONTFAR *pointer;
fix15 tmp;
fix15 format;
ufix16 pix_adj;
point_t Pmin, Pmax;

#if REENTRANT_ALLOC
plaid_t plaid;
sp_globals.plaid = &plaid;
#endif

if (!sp_globals.specs_valid)                /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return FALSE;                /* Error return */
    }

init_tcb();                      /* Initialize transformation control block */

pointer = get_char_org(char_index, TRUE); /* Point to start of character data */
if (pointer == NULL)             /* Character data not available? */
    {
    report_error(12);            /* Report character data not avail */
    return FALSE;                /* Error return */
    }

pointer += 2;                    /* Skip over character id */
tmp = NEXT_WORD(pointer); /* Read set width */
               
format = NEXT_BYTE(pointer);
if (format & BIT1)               /* Optional data in header? */
    {
    tmp = (ufix8)NEXT_BYTE(pointer); /* Read size of optional data */
    pointer += tmp;         /* Skip optional data */
    }

if (format & BIT0)
    {
    pix_adj = sp_globals.onepix << 1;          /* Allow 2 pixel expansion ... */
    }
else
    {
    pix_adj = 0;
    }

pointer = plaid_tcb(pointer, format);              /* Process plaid data */
pointer = read_bbox(pointer, &Pmin, &Pmax,(boolean)FALSE);        /* Read bounding box */

Pmin.x -= pix_adj;                         /* ... of components of ... */
Pmin.y -= pix_adj;                         /* ... compound ... */
Pmax.x += pix_adj;                         /* ... character ... */
Pmax.y += pix_adj;                         /* ... bounding box. */


bbox->xmin  = (fix31)Pmin.x << sp_globals.poshift;
bbox->xmax  = (fix31)Pmax.x << sp_globals.poshift;
bbox->ymin  = (fix31)Pmin.y << sp_globals.poshift;
bbox->ymax  = (fix31)Pmax.y << sp_globals.poshift;
return TRUE;
}
#endif	/* new code */

#endif


#if INCL_ISW
FUNCTION boolean make_char_isw(char_index,imported_setwidth)
GDECL
ufix16 char_index;
ufix32 imported_setwidth;
{
fix15   xmin;          /* Minimum X ORU value in font */
fix15   xmax;          /* Maximum X ORU value in font */
fix15   ymin;          /* Minimum Y ORU value in font */
fix15   ymax;          /* Maximum Y ORU value in font */
ufix16  return_value;

sp_globals.import_setwidth_act = TRUE;
/* convert imported width to orus */
sp_globals.imported_width = (sp_globals.metric_resolution * 
			    imported_setwidth) >> 16;
return_value = do_make_char(char_index);

if (sp_globals.isw_modified_constants)
    {
    /* reset fixed point constants */
    xmin = read_word_u(sp_globals.font_org + FH_FXMIN);
    ymin = read_word_u(sp_globals.font_org + FH_FYMIN);
    ymax = read_word_u(sp_globals.font_org + FH_FYMAX);
    sp_globals.constr.data_valid = FALSE;
    xmax = read_word_u(sp_globals.font_org + FH_FXMAX);
    if (!setup_consts(xmin,xmax,ymin,ymax))
        {
        report_error(3);           /* Requested specs out of range */
        return FALSE;
        }
    }    
return (return_value);
}

FUNCTION boolean make_char(char_index)
GDECL
ufix16 char_index;     /* Index to character in char directory */
{
sp_globals.import_setwidth_act = FALSE;
return (do_make_char(char_index));
}

FUNCTION static boolean do_make_char(char_index)
#else
FUNCTION boolean make_char(char_index)
#endif
/*
 * Outputs specified character using the currently selected font and
 * scaling and output specifications.
 * Reports Error 10 and returns FALSE if no font specifications 
 * previously set.
 * Reports Error 12 and returns FALSE if character data not available.
 */
GDECL
ufix16 char_index;
{
ufix8 FONTFAR  *pointer;      /* Pointer to character data */
fix15    x_orus;
fix15    tmpfix15;
ufix8    format;

#if INCL_ISW
sp_globals.isw_modified_constants = FALSE;
#endif

#if REENTRANT_ALLOC

plaid_t plaid;

#if INCL_BLACK || INCL_SCREEN || INCL_2D
intercepts_t intercepts;
sp_globals.intercepts = &intercepts;
#endif

sp_globals.plaid = &plaid;
#endif

if (!sp_globals.specs_valid)                /* Font specs not defined? */
    {
    report_error(10);            /* Report font not specified */
    return FALSE;                /* Error return */
    }

#if INCL_MULTIDEV
#if INCL_OUTLINE
if (sp_globals.output_mode == MODE_OUTLINE && !sp_globals.outline_device_set)
	{
	report_error(2);
	return FALSE;
	}
else
#endif
	if (!sp_globals.bitmap_device_set)
		{
		report_error(2);
		return FALSE;
		}
#endif


init_tcb();                      /* Initialize transformation control block */

pointer = get_char_org(char_index, TRUE); /* Point to start of character data */
SHOW(pointer);
if (pointer == NULL)             /* Character data not available? */
    {
    report_error(12);            /* Report character data not avail */
    return FALSE;                /* Error return */
    }

pointer += 2;                    /* Skip over character id */
x_orus = NEXT_WORD(pointer); /* Read set width */
#if INCL_SQUEEZING || INCL_ISW
sp_globals.setwidth_orus = x_orus;
#endif

#if INCL_ISW
if (sp_globals.import_setwidth_act)
    x_orus = sp_globals.imported_width;
#endif
sp_globals.Psw.x = (fix15)((fix31)
                   (((fix31)x_orus * (sp_globals.specs.xxmult>>16) + 
                  ( ((fix31)x_orus * (sp_globals.specs.xxmult&0xffffL) )>>16) 
                  ) << sp_globals.pixshift) / sp_globals.metric_resolution);

sp_globals.Psw.y = (fix15)(   
		  (fix31)( 
                 ((fix31)x_orus * (sp_globals.specs.yxmult>>16) + 
                ( ((fix31)x_orus * (sp_globals.specs.yxmult&0xffffL) )>>16) 
                  ) << sp_globals.pixshift) / sp_globals.metric_resolution);
               
format = NEXT_BYTE(pointer);
if (format & BIT1)               /* Optional data in header? */
    {
    tmpfix15 = (ufix8)NEXT_BYTE(pointer); /* Read size of optional data */
    pointer += tmpfix15;         /* Skip optional data */
    }
if (format & BIT0)
    {
    return make_comp_char(pointer); /* Output compound character */
    }
else
    {
    return make_simp_char(pointer, format); /* Output simple character */
    }
}

FUNCTION static boolean make_simp_char(pointer, format)
GDECL
ufix8 FONTFAR  *pointer;      /* Pointer to first byte of position argument */
ufix8    format;       /* Character format byte */
/*
 * Called by sp_make_char() to output a simple (non-compound) character.
 * Returns TRUE on completion.
 */
{
point_t Pmin, Pmax;    /* Transformed corners of bounding box */
#if INCL_SQUEEZING || INCL_ISW
ufix8 FONTFAR *save_pointer;
#endif
#if INCL_ISW
fix31   char_width;
fix31   isw_scale;
#endif

#if INCL_SQUEEZING
sp_globals.squeezing_compound = FALSE;
if ((sp_globals.pspecs->flags & SQUEEZE_LEFT) ||
    (sp_globals.pspecs->flags & SQUEEZE_RIGHT) ||
    (sp_globals.pspecs->flags & SQUEEZE_TOP) ||
    (sp_globals.pspecs->flags & SQUEEZE_BOTTOM) )
    {
	/* get the bounding box data before processing the character */
    save_pointer = pointer;
    preview_bounding_box (pointer, format);
    pointer = save_pointer;
    }
#endif
#if (INCL_ISW)
if (sp_globals.import_setwidth_act)
    {
    save_pointer = pointer;
    preview_bounding_box (pointer, format);
    pointer = save_pointer;
        /* make sure I'm not going to get fixed point overflow */
    isw_scale = compute_isw_scale();
    if (sp_globals.bbox_xmin_orus < 0)
        char_width = SQUEEZE_MULT((sp_globals.bbox_xmax_orus - sp_globals.bbox_xmin_orus), isw_scale);
    else
	char_width = SQUEEZE_MULT(sp_globals.bbox_xmax_orus, isw_scale);
    if (char_width >= sp_globals.isw_xmax)
        if (!reset_xmax(char_width))
              return FALSE;
    }
#endif
pointer = plaid_tcb(pointer, format);              /* Process plaid data */
pointer = read_bbox(pointer, &Pmin, &Pmax, FALSE);      /* Read bounding box */
if (fn_begin_char(sp_globals.Psw, Pmin, Pmax))     /* Signal start of character output */
	{
	do
    	{
	    proc_outl_data(pointer);              /* Process outline data */
    	}
	while (!fn_end_char());                      /* Repeat if not done */
	}
return TRUE;
}

FUNCTION static boolean make_comp_char(pointer)
GDECL
ufix8 FONTFAR  *pointer;      /* Pointer to first byte of position argument */
/*
 * Called by sp_make_char() to output a compound character.
 * Returns FALSE if data for any sub-character is not available.
 * Returns TRUE if output completed with no error.
 */
{
point_t  Pmin, Pmax;   /* Transformed corners of bounding box */
point_t  Pssw;         /* Transformed escapement vector */
ufix8 FONTFAR  *pointer_sav;  /* Saved pointer to compound character data */
ufix8 FONTFAR  *sub_pointer;  /* Pointer to sub-character data */
ufix8    format;       /* Format of DOCH instruction */
ufix16   sub_char_index; /* Index to sub-character in character directory */
fix15    x_posn;       /* X position of sub-character (outline res units) */
fix15    y_posn;       /* Y position of sub-character (outline res units) */
fix15    x_scale;      /* X scale factor of sub-character (scale units) */
fix15    y_scale;      /* Y scale factor of sub-character (scale units) */
fix15    tmpfix15;     /* Temporary workspace */
fix15    x_orus;       /* Set width in outline resolution units */
fix15    pix_adj;      /* Pixel adjustment to compound char bounding box */
#if INCL_SQUEEZING
fix31    x_factor, x_offset, top_scale, bottom_scale;
boolean  squeezed_x, squeezed_y;
#endif
#if INCL_SQUEEZING || INCL_ISW
fix15    x_offset_pix;
#endif
#if INCL_ISW
fix31   char_width;
fix31   isw_scale;
#endif


#if INCL_SQUEEZING
sp_globals.squeezing_compound = TRUE;
#endif
pointer = read_bbox(pointer, &Pmin, &Pmax, TRUE); /* Read bounding box data */
pix_adj = sp_globals.onepix << 1;          /* Allow 2 pixel expansion ... */
Pmin.x -= pix_adj;                         /* ... of components of ... */
Pmin.y -= pix_adj;                         /* ... compound ... */
Pmax.x += pix_adj;                         /* ... character ... */
Pmax.y += pix_adj;                         /* ... bounding box. */

#if INCL_SQUEEZING
/* scale the bounding box if necessary before calling begin_char */
squeezed_x = calculate_x_scale(&x_factor, &x_offset, 0);
squeezed_y = calculate_y_scale(&top_scale, &bottom_scale,0,0);

if (squeezed_x)
    { /* scale the x coordinates of the bbox */
    x_offset_pix = (fix15)(((x_offset >> 16) * sp_globals.tcb0.xppo)
                    >> sp_globals.mpshift);
    if ((x_offset_pix >0) && (x_offset_pix < sp_globals.onepix))
        x_offset_pix = sp_globals.onepix;
    Pmin.x = SQUEEZE_MULT (x_factor, Pmin.x) + x_offset_pix - pix_adj;
    Pmax.x = SQUEEZE_MULT (x_factor, Pmax.x) + x_offset_pix + pix_adj;
    }
if (squeezed_y)
    { /* scale the y coordinates of the bbox */
    if ((Pmin.y) < 0)
        Pmin.y = SQUEEZE_MULT (bottom_scale, Pmin.y) - pix_adj;
    else
        Pmin.y = SQUEEZE_MULT (top_scale, Pmin.y) - pix_adj;
    if ((Pmax.y) < 0)
        Pmax.y = SQUEEZE_MULT (bottom_scale, Pmax.y) + pix_adj;
    else
        Pmax.y = SQUEEZE_MULT (top_scale, Pmax.y) + pix_adj;
    }
#endif
#if (INCL_ISW)
if (sp_globals.import_setwidth_act)
    {
        /* make sure I'm not going to get fixed point overflow */
    isw_scale = ((fix31)sp_globals.imported_width << 16)/
                 (fix31)sp_globals.setwidth_orus;
    char_width = SQUEEZE_MULT((sp_globals.bbox_xmax_orus - 
                               sp_globals.bbox_xmin_orus),
isw_scale);
    if (char_width >= sp_globals.isw_xmax)
        if (!reset_xmax(char_width))
              return FALSE;
    }
#endif

if (fn_begin_char(sp_globals.Psw, Pmin, Pmax)) /* Signal start of character data */
	{
	pointer_sav = pointer;
	do
	    {
	    pointer = pointer_sav;                 /* Point to next DOCH or END instruction */
	    while (format = NEXT_BYTE(pointer))    /* DOCH instruction? */
	        {
	        init_tcb();                        /* Initialize transformation control block */
	        x_posn = get_posn_arg(&pointer, format);
	        y_posn = get_posn_arg(&pointer, (ufix8)(format >> 2));
	        x_scale = get_scale_arg(&pointer, (ufix8)(format & BIT4));
	        y_scale = get_scale_arg(&pointer, (ufix8)(format & BIT5));
	        scale_tcb(&sp_globals.tcb, x_posn, y_posn, x_scale, y_scale); /* Scale for sub-char */
	        sub_char_index = (format & BIT6)?  /* Read sub-char index */
	            (ufix16)NEXT_WORD(pointer):
	            (ufix16)NEXT_BYTE(pointer);          
	        sub_pointer = get_char_org(sub_char_index, FALSE); /* Point to start of sub-char */
	        if (sub_pointer == NULL)           /* Character data not available? */
	            {
	            return FALSE;                  /* Abort character output */
	            }
	        sub_pointer += 2;                  /* Skip over character id */
	        x_orus = NEXT_WORD(sub_pointer);   /* Read set_width of sub-character */

			Pssw.x = (fix15)(   
					  (fix31)( 
                              ((fix31)x_orus * (sp_globals.specs.xxmult>>16) + 
                              ( ((fix31)x_orus * (sp_globals.specs.xxmult&0xffffL) )>>16) 
                             ) << sp_globals.pixshift) / sp_globals.metric_resolution);
			Pssw.y = (fix15)(   
					  (fix31)( 
                              ((fix31)x_orus * (sp_globals.specs.yxmult>>16) + 
                              ( ((fix31)x_orus * (sp_globals.specs.yxmult&0xffffL) )>>16) 
                             ) << sp_globals.pixshift) / sp_globals.metric_resolution);
               
	        format = NEXT_BYTE(sub_pointer);   /* Read sub-character format */
	        if (format & BIT1)                 /* Optional data in header? */
	            {
	            tmpfix15 = (ufix8)NEXT_BYTE(sub_pointer); /* Read size of optional data */
	            sub_pointer += tmpfix15;           /* Skip optional data */
	            }
	        sub_pointer = plaid_tcb(sub_pointer, format);   /* Process sub-character plaid data */
	        sub_pointer = read_bbox(sub_pointer, &Pmin, &Pmax, FALSE); /* Read bounding box */
	        fn_begin_sub_char(Pssw, Pmin, Pmax);  /* Signal start of sub-character data */
	        proc_outl_data(sub_pointer);       /* Process sub-character data */
	        fn_end_sub_char();                    /* Signal end of sub-character data */
	        }
	    }
	while (!fn_end_char());                       /* Signal end of character; repeat if required */
	}
return TRUE;
}

#if INCL_LCD           /* Dynamic load character data supported? */
FUNCTION static ufix8 FONTFAR *get_char_org(char_index, top_level)
GDECL
ufix16   char_index;   /* Index of character to be accessed */
boolean  top_level;    /* Not a compound character element
/*
 * Called by sp_get_char_id(), sp_get_char_width(), sp_make_char() and
 * make_comp_char() to get a pointer to the start of the character data
 * for the specified character index.
 * Version for configuration supporting dynamic character data loading.
 * Calls load_char_data() to load character data if not already loaded
 * as part of the original font buffer.
 * Returns NULL if character data not available
 */
{
buff_t  *pchar_data;   /* Buffer descriptor requested */
ufix8 FONTFAR  *pointer;      /* Pointer into character directory */
ufix8    format;       /* Character directory format byte */
fix31    char_offset;  /* Offset of char data from start of font file */
fix31    next_char_offset; /* Offset of char data from start of font file */
fix15    no_bytes;     /* Number of bytes required for char data */

if (top_level)                        /* Not element of compound char? */
    {
    if (char_index < sp_globals.first_char_idx)  /* Before start of character set? */
        return NULL;
    char_index -= sp_globals.first_char_idx;
    if (char_index >= sp_globals.no_chars_avail) /* Beyond end of character set? */
        return NULL;
    sp_globals.cb_offset = 0;                    /* Reset char buffer offset */
    }

pointer = sp_globals.pchar_dir;
format = NEXT_BYTE(pointer);          /* Read character directory format byte */
pointer += char_index << 1;           /* Point to indexed character entry */
if (format)                           /* 3-byte entries in char directory? */
    {
    pointer += char_index;            /* Adjust for 3-byte entries */
    char_offset = read_long(pointer); /* Read file offset to char data */
    next_char_offset = read_long(pointer + 3); /* Read offset to next char */
    }
else
    {
    char_offset = (fix31)((ufix16)NEXT_WORD(pointer)); /* Read file offset to char data */
    next_char_offset = (fix31)((ufix16)NEXT_WORD(pointer)); /* Read offset to next char */
    }

no_bytes = next_char_offset - char_offset;
if (no_bytes == 0)                    /* Character not in directory? */
    return NULL;

if (next_char_offset <= sp_globals.font_buff_size)/* Character data already in font buffer? */
    return sp_globals.pfont->org + char_offset;  /* Return pointer into font buffer */

pchar_data = load_char_data(char_offset, no_bytes, sp_globals.cb_offset); /* Request char data load */
if (pchar_data->no_bytes < no_bytes)  /* Correct number of bytes loaded? */
    return NULL;

if (top_level)                        /* Not element of compound char? */
    {
    sp_globals.cb_offset = no_bytes;
    }

return pchar_data->org;               /* Return pointer into character data buffer */
}
#endif

#if INCL_LCD
#else                  /* Dynamic load character data not supported? */
FUNCTION static ufix8 FONTFAR *get_char_org(char_index, top_level)
GDECL
ufix16   char_index;   /* Index of character to be accessed */
boolean  top_level;    /* Not a compound character element
/*
 * Called by sp_get_char_id(), sp_get_char_width(), sp_make_char() and
 * make_comp_char() to get a pointer to the start of the character data
 * for the specified character index.
 * Version for configuration not supporting dynamic character data loading.
 * Returns NULL if character data not available
 */
{
ufix8   FONTFAR *pointer;      /* Pointer into character directory */
ufix8    format;       /* Character directory format byte */
fix31    char_offset;  /* Offset of char data from start of font file */
fix31    next_char_offset; /* Offset of char data from start of font file */
fix15    no_bytes;     /* Number of bytes required for char data */

if (top_level)                        /* Not element of compound char? */
    {
    if (char_index < sp_globals.first_char_idx)  /* Before start of character set? */
        return NULL;
    char_index -= sp_globals.first_char_idx;
    if (char_index >= sp_globals.no_chars_avail) /* Beyond end of character set? */
        return NULL;
    }

pointer = sp_globals.pchar_dir;
format = NEXT_BYTE(pointer);          /* Read character directory format byte */
pointer += char_index << 1;           /* Point to indexed character entry */
if (format)                           /* 3-byte entries in char directory? */
    {
    pointer += char_index;            /* Adjust for 3-byte entries */
    char_offset = read_long(pointer); /* Read file offset to char data */
    next_char_offset = read_long(pointer + 3); /* Read offset to next char */
    }
else
    {
    char_offset = (fix31)((ufix16)NEXT_WORD(pointer)); /* Read file offset to char data */
    next_char_offset = (fix31)((ufix16)NEXT_WORD(pointer)); /* Read offset to next char */
    }

no_bytes = next_char_offset - char_offset;
if (no_bytes == 0)                    /* Character not in directory? */
    return NULL;

return sp_globals.pfont->org + char_offset;      /* Return pointer into font buffer */
}
#endif


FUNCTION static fix15 get_posn_arg(ppointer, format)
GDECL
ufix8 FONTFAR * STACKFAR *ppointer;     /* Pointer to first byte of position argument */
ufix8    format;       /* Format of DOCH arguments */
/*
 * Called by make_comp_char() to read a position argument from the
 * specified point in the font/char buffer.
 * Updates pointer to byte following position argument.
 * Returns value of position argument in outline resolution units
 */
{
switch (format & 0x03)
    {
case 1:
    return NEXT_WORD(*ppointer);

case 2:
    return (fix15)((fix7)NEXT_BYTE(*ppointer));

default:
    return (fix15)0;
    }
}

FUNCTION static fix15 get_scale_arg(ppointer, format)
GDECL
ufix8 FONTFAR *STACKFAR *ppointer;     /* Pointer to first byte of position argument */
ufix8    format;       /* Format of DOCH arguments */
/*
 * Called by make_comp_char() to read a scale argument from the
 * specified point in the font/char buffer.
 * Updates pointer to byte following scale argument.
 * Returns value of scale argument in scale units (normally 1/4096)
 */
{
if (format)
    return NEXT_WORD(*ppointer);
else
    return (fix15)ONE_SCALE;
}
#if INCL_ISW || INCL_SQUEEZING
FUNCTION static void preview_bounding_box(pointer,format)
GDECL
ufix8 FONTFAR  *pointer;      /* Pointer to first byte of position argument */
ufix8    format;       /* Character format byte */
{
point_t  Pmin, Pmax;   /* Transformed corners of bounding box */

    sp_globals.no_X_orus = (format & BIT2)?
        (fix15)NEXT_BYTE(pointer):
        0;
    sp_globals.no_Y_orus = (format & BIT3)?
        (fix15)NEXT_BYTE(pointer):
        0;
    pointer = read_oru_table(pointer);

    /* Skip over control zone table */
    pointer = skip_control_zone(pointer,format);

    /* Skip over interpolation table */
    pointer = skip_interpolation_table(pointer,format);
    /* get_args has a pathological need for this value to be set */
    sp_globals.Y_edge_org = sp_globals.no_X_orus;
    pointer = read_bbox(pointer, &Pmin, &Pmax, TRUE);        /* Read bounding bo
x */

}
#endif
#if INCL_ISW
FUNCTION static boolean reset_xmax(xmax)
GDECL
fix31   xmax;

{
fix15   xmin;          /* Minimum X ORU value in font */
fix15   ymin;          /* Minimum Y ORU value in font */
fix15   ymax;          /* Maximum Y ORU value in font */


sp_globals.isw_modified_constants = TRUE;
xmin = read_word_u(sp_globals.font_org + FH_FXMIN);
ymin = read_word_u(sp_globals.font_org + FH_FYMIN);
ymax = read_word_u(sp_globals.font_org + FH_FYMAX);

if (!setup_consts(xmin,xmax,ymin,ymax))
    {
    report_error(3);           /* Requested specs out of range */
    return FALSE;
    }
sp_globals.constr.data_valid = FALSE;
/* recompute setwidth */
sp_globals.Psw.x = (fix15)((fix31)(
     ((fix31)sp_globals.imported_width * (sp_globals.specs.xxmult>>16) +
     ( ((fix31)sp_globals.imported_width *
          (sp_globals.specs.xxmult&0xffffL) )>>16)
     ) << sp_globals.pixshift) / sp_globals.metric_resolution);
sp_globals.Psw.y = (fix15)(   
		  (fix31)( 
                 ((fix31)sp_globals.imported_width * (sp_globals.specs.yxmult>>16) + 
                ( ((fix31)sp_globals.imported_width * (sp_globals.specs.yxmult&0xffffL) )>>16) 
                  ) << sp_globals.pixshift) / sp_globals.metric_resolution);
               
return TRUE;
}
#endif
