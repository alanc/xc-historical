


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
*     $Header: //toklas/archive/rcs/speedo/do_trns.c,v 22.1 91/01/23 17:15:57 leeann Release $                                                                       *
*                                                                                    *
*     $Log:	do_trns.c,v $
*       Revision 22.1  91/01/23  17:15:57  leeann
*       Release
*       
*       Revision 21.1  90/11/20  14:35:58  leeann
*       Release
*       
*       Revision 20.1  90/11/12  09:20:22  leeann
*       Release
*       
*       Revision 19.1  90/11/08  10:18:16  leeann
*       Release
*       
*       Revision 18.1  90/09/24  09:50:48  mark
*       Release
*       
*       Revision 17.2  90/09/14  15:01:47  leeann
*       save bounding box in orus if the INCL_ISW flag is set
*       
*       Revision 17.1  90/09/13  15:57:19  mark
*       Release name rel0913
*       
*       Revision 16.1  90/09/11  12:54:24  mark
*       Release
*       
*       Revision 15.1  90/08/29  10:02:41  mark
*       Release name rel0829
*       
*       Revision 14.1  90/07/13  10:38:19  mark
*       Release name rel071390
*       
*       Revision 13.1  90/07/02  10:37:15  mark
*       Release name REL2070290
*       
*       Revision 12.1  90/04/23  12:11:25  mark
*       Release name REL20
*       
*       Revision 11.1  90/04/23  10:11:22  mark
*       Release name REV2
*       
*       Revision 10.3  90/04/12  13:06:05  leeann
*       change compilation option for squeezing to INCL_SQUEEZING
*       
*       Revision 10.2  90/03/29  16:44:28  leeann
*       Added set_flags argument to read_bbox
*       added SQUEEZE code to save the oru bbox when
*       set_flag is TRUE
*       
*       Revision 10.1  89/07/28  18:08:02  mark
*       Release name PRODUCT
*       
*       Revision 9.1  89/07/27  10:21:11  mark
*       Release name PRODUCT
*       
*       Revision 8.1  89/07/13  18:17:51  mark
*       Release name Product
*       
*       Revision 7.1  89/07/11  08:59:24  mark
*       Release name PRODUCT
*       
*       Revision 6.2  89/07/09  14:48:14  mark
*       since get_args takes a STACKFAR pointer to the point returned,
*       transformation failed if stack and globals had different 
*       locations.  Now retrieve P0 into local and assign to global copy.
*       
*       Revision 6.1  89/06/19  08:33:23  mark
*       Release name prod
*       
*       Revision 5.4  89/06/06  17:23:13  mark
*       add curve depth to output module curve functions
*       
*       Revision 5.3  89/06/02  17:03:54  mark
*       use sp_plaid symbol for referring to plaid tables so
*       that these buffers can be conditionally allocated
*       off the stack for reentrant mode.
*       
*       Revision 5.2  89/05/17  16:26:03  john
*       Inhibited conversion of curve to vector when curves_out
*       is on and adjusted depth is zero or less.
*       
*       Revision 5.1  89/05/01  17:51:32  mark
*       Release name Beta
*       
*       Revision 4.1  89/04/27  12:12:10  mark
*       Release name Beta
*       
*       Revision 3.1  89/04/25  08:25:08  mark
*       Release name beta
*       
*       Revision 2.3  89/04/12  12:11:30  mark
*       added stuff for far stack and font
*       
*       Revision 2.2  89/04/10  17:09:00  mark
*       Modified pointer declarations that are used to refer
*       to font data to use FONTFAR symbol, which will be used
*       for Intel SS != DS memory models
*       Modified read_bbox and get_args to receive a pointer and 
*       return the resulting pointer, rather than receiving a 
*       pointer to a pointer
*       
*       Revision 2.1  89/04/04  13:31:49  mark
*       Release name EVAL
*       
*       Revision 1.9  89/04/04  13:17:24  mark
*       Update copyright text
*       
*       Revision 1.8  89/04/03  09:36:01  mark
*       added break; statement to default with null scope because
*       Microsoft C gives questionable syntax error if you don't
*       
*       Revision 1.7  89/03/31  16:58:41  john
*       Default curve support removed.
*       
*       Revision 1.6  89/03/31  14:44:20  mark
*       change speedo.h to spdo_prv.h
*       change comments from fontware to speedo
*       
*       Revision 1.5  89/03/31  12:14:52  john
*       modified to use new NEXT_WORD macro.
*       
*       Revision 1.4  89/03/30  17:47:58  john
*       read_bbox() rewritten.
*       
*       Revision 1.3  89/03/29  16:08:34  mark
*       changes for slot independence and dynamic/reentrant
*       data allocation
*       
*       Revision 1.2  89/03/21  13:26:05  mark
*       change name from oemfw.h to speedo.h
*       
*       Revision 1.1  89/03/15  12:29:02  mark
*       Initial revision
*                                                                                 *
*                                                                                    *
*************************************************************************************/

#ifdef RCSSTATUS
static char rcsid[] = "$Header: //toklas/archive/rcs/speedo/do_trns.c,v 22.1 91/01/23 17:15:57 leeann Release $";
#endif





/**************************** D O _ T R N S . C ******************************
 *                                                                           *
 * This module is responsible for executing all intelligent transformation   *
 * for bounding box and outline data                                         *
 *                                                                           *
 ********************** R E V I S I O N   H I S T O R Y **********************
 *                                                                           *
 *  1) 16 Dec 88  jsc  Created                                               *
 *                                                                           *
 *  2) 28 Feb 89  jsc  Plaid data monitoring functions added.                *
 *                                                                           *
 ****************************************************************************/


#include "spdo_prv.h"               /* General definitions for Speedo    */

#define   DEBUG      0

#if DEBUG
#include <stdio.h>
#define SHOW(X) printf("X = %d\n", X)
#else
#define SHOW(X)
#endif

/***** GLOBAL VARIABLES *****/

/***** GLOBAL FUNCTIONS *****/

/***** EXTERNAL VARIABLES *****/

/***** EXTERNAL FUNCTIONS *****/

/***** STATIC VARIABLES *****/

/***** STATIC FUNCTIONS *****/



FUNCTION ufix8 FONTFAR *read_bbox(pointer, pPmin, pPmax, set_flag)
GDECL
ufix8 FONTFAR *pointer;    /* Pointer to next byte in char data */
point_t STACKFAR *pPmin;      /* Lower left corner of bounding box */
point_t STACKFAR *pPmax;      /* Upper right corner of bounding box */
boolean set_flag; /* flag to indicate whether global oru bbox should be saved */
/*
 * Called by make_simp_char() and make_comp_char() to read the 
 * bounding box data from the font.
 * Sets Pmin and Pmax to the bottom left and top right corners
 * of the bounding box after transformation into device space.
 * The units of Pmin and Pmax are sub-pixels.
 * Updates *ppointer to point to the byte following the
 * bounding box data.
 */
{
ufix8    format1;
ufix8    format;
fix15    i;
point_t  P;

sp_globals.x_int = 0;
sp_globals.y_int = sp_globals.Y_int_org;
sp_globals.x_orus = sp_globals.y_orus = 0;
format1 = NEXT_BYTE(pointer);
pointer = get_args(pointer, format1, pPmin);
#if INCL_SQUEEZING || INCL_ISW
if (set_flag)
    {
    sp_globals.bbox_xmin_orus = sp_globals.x_orus;
    sp_globals.bbox_ymin_orus = sp_globals.y_orus;
    }
#endif
*pPmax = *pPmin;
for (i = 1; i < 4; i++)
    {
    switch(i)
        {
    case 1:
        if (format1 & BIT6)            /* Xmax requires X int zone 1? */
            sp_globals.x_int++;
        format = (format1 >> 4) | 0x0c;
        break;

    case 2:
        if (format1 & BIT7)            /* Ymax requires Y int zone 1? */
            sp_globals.y_int++;
        format = NEXT_BYTE(pointer);
        break;

    case 3:
        sp_globals.x_int = 0; 
        format >>= 4;
        break;

    default:
		break;
        }

    pointer = get_args(pointer, format, &P);
#if INCL_SQUEEZING || INCL_ISW
    if (set_flag && (i==2))
	{
	sp_globals.bbox_xmax_orus = sp_globals.x_orus;
	sp_globals.bbox_ymax_orus = sp_globals.y_orus;
	}
#endif
    if ((i == 2) || (!sp_globals.normal)) 
        {
        if (P.x < pPmin->x)
            pPmin->x = P.x;
        if (P.y < pPmin->y)
            pPmin->y = P.y;
        if (P.x > pPmax->x)
            pPmax->x = P.x;
        if (P.y > pPmax->y)
            pPmax->y = P.y;
        }
    }

#if DEBUG
printf("BBOX %6.1f(Xint 0), %6.1f(Yint 0), %6.1f(Xint %d), %6.1f(Yint %d)\n",
    (real)pPmin->x / (real)sp_globals.onepix, 
    (real)pPmin->y / (real)sp_globals.onepix, 
    (real)pPmax->x / (real)sp_globals.onepix, 
    (format1 >> 6) & 0x01,
    (real)pPmax->y / (real)sp_globals.onepix,
    (format1 >> 7) & 0x01);

#endif
return pointer;
}

FUNCTION void proc_outl_data(pointer)
GDECL
ufix8 FONTFAR *pointer;      /* Pointer to next byte in char data */
/*
 * Called by make_simp_char() and make_comp_char() to read the 
 * outline data from the font.
 * The outline data is parsed, transformed into device coordinates
 * and passed to an output module for further processing.
 * Note that pointer is not updated to facilitate repeated
 * processing of the outline data when banding mode is in effect.
 */
{
ufix8    format1, format2;
point_t  P0, P1, P2, P3;
fix15    depth;
fix15    curve_count;

sp_globals.x_int = 0;
sp_globals.y_int = sp_globals.Y_int_org;
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_xint((fix15)sp_globals.x_int);         /* Record xint data */
record_yint((fix15)(sp_globals.y_int - sp_globals.Y_int_org)); /* Record yint data */
#endif

sp_globals.x_orus = sp_globals.y_orus = 0;
curve_count = 0;
while(TRUE)
    {
    format1 = NEXT_BYTE(pointer);
    switch(format1 >> 4)
        {
    case 0:                        /* LINE */
        pointer = get_args(pointer, format1, &P1);
#if DEBUG
        printf("LINE %6.1f, %6.1f\n",
            (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix);
#endif
        fn_line(P1);
        sp_globals.P0 = P1;
        continue;

    case 1:                        /* Short XINT */
        sp_globals.x_int = format1 & 0x0f;
#if DEBUG
        printf("XINT %d\n", sp_globals.x_int);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_xint((fix15)sp_globals.x_int);         /* Record xint data */
#endif
        continue;

    case 2:                        /* Short YINT */
        sp_globals.y_int = sp_globals.Y_int_org + (format1 & 0x0f);
#if DEBUG
        printf("YINT %d\n", sp_globals.y_int - sp_globals.Y_int_org);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_yint((fix15)(sp_globals.y_int - sp_globals.Y_int_org)); /* Record yint data */
#endif
        continue;
         
    case 3:                        /* Miscellaneous */
        switch(format1 & 0x0f)
            {
        case 0:                    /* END */
            if (curve_count)
                {
                fn_end_contour();
                }
            return;

        case 1:                     /* Long XINT */
            sp_globals.x_int = NEXT_BYTE(pointer);
#if DEBUG
            printf("XINT %d\n", sp_globals.x_int);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_xint((fix15)sp_globals.x_int);         /* Record xint data */
#endif
            continue;

        case 2:                     /* Long YINT */
            sp_globals.y_int = sp_globals.Y_int_org + NEXT_BYTE(pointer);
#if DEBUG
            printf("YINT %d\n", sp_globals.y_int - sp_globals.Y_int_org);
#endif
#if INCL_PLAID_OUT                 /* Plaid data monitoring included? */
record_yint((fix15)(sp_globals.y_int - sp_globals.Y_int_org)); /* Record yint data */
#endif
            continue;

        default:                    /* Not used */
            continue;
            }    

    case 4:                         /* MOVE Inside */
    case 5:                         /* MOVE Outside */
        if (curve_count++)
            {
            fn_end_contour();
            }                                
		
        pointer = get_args(pointer, format1, &P0);
		sp_globals.P0 = P0;
#if DEBUG
        printf("MOVE %6.1f, %6.1f\n",
            (real)sp_globals.P0.x / (real)sp_globals.onepix, (real)sp_globals.P0.y / (real)sp_globals.onepix);
#endif
        fn_begin_contour(sp_globals.P0, (boolean)(format1 & BIT4));
        continue;

    case 6:                         /* Undefined */
#if DEBUG
        printf("*** Undefined instruction (Hex %4x)\n", format1);
#endif
        continue;

    case 7:                         /* Undefined */
#if DEBUG
        printf("*** Undefined instruction (Hex %4x)\n", format1);
#endif
        continue;

    default:                        /* CRVE */
        format2 = NEXT_BYTE(pointer);
        pointer = get_args(pointer, format1, &P1);
        pointer = get_args(pointer, format2, &P2);
        pointer = get_args(pointer, (ufix8)(format2 >> 4), &P3);
        depth = (format1 >> 4) & 0x07;
#if DEBUG
        printf("CRVE %6.1f, %6.1f, %6.1f, %6.1f, %6.1f, %6.1f, %d\n",
            (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix, 
            (real)P2.x / (real)sp_globals.onepix, (real)P2.y / (real)sp_globals.onepix,
            (real)P3.x / (real)sp_globals.onepix, (real)P3.y / (real)sp_globals.onepix,
            depth);
#endif
        depth += sp_globals.depth_adj;
        if (sp_globals.curves_out)
            {
            fn_curve(P1, P2, P3, depth);
            sp_globals.P0 = P3;
            continue;
            }
        if (depth <= 0)
            {
            fn_line(P3);
            sp_globals.P0 = P3;
            continue;
            }   
        split_curve(P1, P2, P3, depth);
        continue;
        }
    }
}

FUNCTION static void split_curve(P1, P2, P3, depth)
GDECL
point_t P1;    /* First control point of Bezier curve */
point_t P2;    /* Second  control point of Bezier curve */
point_t P3;    /* End point of Bezier curve */
fix15   depth; /* Levels of recursive subdivision required */
/*
 * Called by proc_outl_data() to subdivide Bezier curves into an
 * appropriate number of vectors, whenever curves are not enabled
 * for output to the currently selected output module.
 * split_curve() calls itself recursively to the depth specified
 * at which point it calls line() to deliver each vector resulting
 * from the spliting process.
 */
{
fix31   X0 = (fix31)sp_globals.P0.x;
fix31   Y0 = (fix31)sp_globals.P0.y;
fix31   X1 = (fix31)P1.x;
fix31   Y1 = (fix31)P1.y;
fix31   X2 = (fix31)P2.x;
fix31   Y2 = (fix31)P2.y;
fix31   X3 = (fix31)P3.x;
fix31   Y3 = (fix31)P3.y;
point_t Pmid;
point_t Pctrl1;
point_t Pctrl2;

#if DEBUG
printf("CRVE(%3.1f, %3.1f, %3.1f, %3.1f, %3.1f, %3.1f)\n",
    (real)P1.x / (real)sp_globals.onepix, (real)P1.y / (real)sp_globals.onepix,
    (real)P2.x / (real)sp_globals.onepix, (real)P2.y / (real)sp_globals.onepix,
    (real)P3.x / (real)sp_globals.onepix, (real)P3.y / (real)sp_globals.onepix);
#endif


Pmid.x = (X0 + (X1 + X2) * 3 + X3 + 4) >> 3;
Pmid.y = (Y0 + (Y1 + Y2) * 3 + Y3 + 4) >> 3;
if ((--depth) <= 0)
    {
    fn_line(Pmid);
    sp_globals.P0 = Pmid;
    fn_line(P3);
    sp_globals.P0 = P3;
    }
else
    {
    Pctrl1.x = (X0 + X1 + 1) >> 1;
    Pctrl1.y = (Y0 + Y1 + 1) >> 1;
    Pctrl2.x = (X0 + (X1 << 1) + X2 + 2) >> 2;
    Pctrl2.y = (Y0 + (Y1 << 1) + Y2 + 2) >> 2;
    split_curve(Pctrl1, Pctrl2, Pmid, depth);
    Pctrl1.x = (X1 + (X2 << 1) + X3 + 2) >> 2;
    Pctrl1.y = (Y1 + (Y2 << 1) + Y3 + 2) >> 2;
    Pctrl2.x = (X2 + X3 + 1) >> 1;
    Pctrl2.y = (Y2 + Y3 + 1) >> 1;
    split_curve(Pctrl1, Pctrl2, P3, depth);
    }
}

FUNCTION static ufix8 FONTFAR *get_args(pointer, format, pP)
GDECL
ufix8 FONTFAR  *pointer;  /* Pointer to next byte in char data */
ufix8     format;    /* Format specifiaction of argument pair */
point_t STACKFAR *pP;        /* Resulting transformed point */
/*
 * Called by read_bbox() and proc_outl_data() to read an X Y argument
 * pair from the font.
 * The format is specified as follows:
 *     Bits 0-1: Type of X argument.
 *     Bits 2-3: Type of Y argument.
 * where the 4 possible argument types are:
 *     Type 0:   Controlled coordinate represented by one byte
 *               index into the X or Y controlled coordinate table.
 *     Type 1:   Interpolated coordinate represented by a two-byte
 *               signed integer.
 *     Type 2:   Interpolated coordinate represented by a one-byte
 *               signed increment/decrement relative to the 
 *               proceding X or Y coordinate.
 *     Type 3:   Repeat of preceding X or Y argument value and type.
 * The units of P are sub-pixels.
 * Updates *ppointer to point to the byte following the
 * argument pair.
 */
{
ufix8   edge;

/* Read X argument */
switch(format & 0x03)
    {
case 0:                           /* Index to controlled oru */
    edge = NEXT_BYTE(pointer);
    sp_globals.x_orus = sp_plaid.orus[edge];
#if INCL_RULES
    sp_globals.x_pix = sp_plaid.pix[edge];
#endif
    break;

case 1:                           /* 2 byte interpolated oru value */
    sp_globals.x_orus = NEXT_WORD(pointer);
    goto L1;

case 2:                           /* 1 byte signed oru increment */
    sp_globals.x_orus += (fix15)((fix7)NEXT_BYTE(pointer));
L1: 
#if INCL_RULES
    sp_globals.x_pix = TRANS(sp_globals.x_orus, sp_plaid.mult[sp_globals.x_int], sp_plaid.offset[sp_globals.x_int], sp_globals.mpshift);
#endif
    break;

default:                          /* No change in X value */
    break;
    }

/* Read Y argument */
switch((format >> 2) & 0x03)
    {
case 0:                           /* Index to controlled oru */
    edge = sp_globals.Y_edge_org + NEXT_BYTE(pointer);
    sp_globals.y_orus = sp_plaid.orus[edge];
#if INCL_RULES
    sp_globals.y_pix = sp_plaid.pix[edge];
#endif
    break;

case 1:                           /* 2 byte interpolated oru value */
    sp_globals.y_orus = NEXT_WORD(pointer);
    goto L2;

case 2:                           /* 1 byte signed oru increment */
    sp_globals.y_orus += (fix15)((fix7)NEXT_BYTE(pointer));
L2: 
#if INCL_RULES
    sp_globals.y_pix = TRANS(sp_globals.y_orus, sp_plaid.mult[sp_globals.y_int], sp_plaid.offset[sp_globals.y_int], sp_globals.mpshift);
#endif
    break;

default:                          /* No change in X value */
    break;
    }

#if INCL_RULES
switch(sp_globals.tcb.xmode)
    {
case 0:                           /* X mode 0 */
    pP->x = sp_globals.x_pix;
    break;

case 1:                           /* X mode 1 */
    pP->x = -sp_globals.x_pix;
    break;

case 2:                           /* X mode 2 */
    pP->x = sp_globals.y_pix;
    break;

case 3:                           /* X mode 3 */
    pP->x = -sp_globals.y_pix;
    break;

default:                          /* X mode 4 */
#endif
    pP->x = (MULT16(sp_globals.x_orus, sp_globals.tcb.xxmult) + 
             MULT16(sp_globals.y_orus, sp_globals.tcb.xymult) + 
             sp_globals.tcb.xoffset) >> sp_globals.mpshift;
#if INCL_RULES
    break;
    }

switch(sp_globals.tcb.ymode)
    {
case 0:                           /* Y mode 0 */
    pP->y = sp_globals.y_pix;
    break;

case 1:                           /* Y mode 1 */
    pP->y = -sp_globals.y_pix;
    break;

case 2:                           /* Y mode 2 */
    pP->y = sp_globals.x_pix;
    break;

case 3:                           /* Y mode 3 */
    pP->y = -sp_globals.x_pix;
    break;

default:                          /* Y mode 4 */
#endif
    pP->y = (MULT16(sp_globals.x_orus, sp_globals.tcb.yxmult) + 
             MULT16(sp_globals.y_orus, sp_globals.tcb.yymult) + 
             sp_globals.tcb.yoffset) >> sp_globals.mpshift;
#if INCL_RULES
    break;
    }
#endif

return pointer;
}



