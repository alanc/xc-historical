/*
 * $XConsortium: sunGX.h,v 1.1 91/06/28 16:17:27 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

typedef unsigned int	Uint;

/* modes */
#define GX_INDEX(n)	    ((n) << 4)
#define GX_INDEX_ALL	    0x00000030
#define GX_INDEX_MOD	    0x00000040
#define GX_BDISP_0	    0x00000080
#define GX_BDISP_1	    0x00000100
#define GX_BDISP_ALL	    0x00000180
#define GX_BREAD_0	    0x00000200
#define GX_BREAD_1	    0x00000400
#define GX_BREAD_ALL	    0x00000600
#define GX_BWRITE1_ENABLE   0x00000800
#define GX_BWRITE1_DISABLE  0x00001000
#define GX_BWRITE1_ALL	    0x00001800
#define GX_BWRITE0_ENABLE   0x00002000
#define GX_BWRITE0_DISABLE  0x00004000
#define GX_BWRITE0_ALL	    0x00006000
#define GX_DRAW_RENDER	    0x00008000
#define GX_DRAW_PICK	    0x00010000
#define GX_DRAW_ALL	    0x00018000
#define GX_MODE_COLOR8	    0x00020000
#define GX_MODE_COLOR1	    0x00040000
#define GX_MODE_HRMONO	    0x00060000
#define GX_MODE_ALL	    0x00060000
#define GX_VBLANK	    0x00080000
#define GX_BLIT_NOSRC	    0x00100000
#define GX_BLIT_SRC	    0x00200000
#define GX_BLIT_ALL	    0x00300000

/* rasterops */
#define GX_ROP_CLEAR	    0x0
#define GX_ROP_INVERT	    0x1
#define GX_ROP_NOOP	    0x2
#define GX_ROP_SET	    0x3

#define GX_ROP_00_0(rop)    ((rop) << 0)
#define GX_ROP_00_1(rop)    ((rop) << 2)
#define GX_ROP_01_0(rop)    ((rop) << 4)
#define GX_ROP_01_1(rop)    ((rop) << 6)
#define GX_ROP_10_0(rop)    ((rop) << 8)
#define GX_ROP_10_1(rop)    ((rop) << 10)
#define GX_ROP_11_0(rop)    ((rop) << 12)
#define GX_ROP_11_1(rop)    ((rop) << 14)
#define GX_PLOT_PLOT	    0x00000000
#define GX_PLOT_UNPLOT	    0x00020000
#define GX_RAST_BOOL	    0x00000000
#define GX_RAST_LINEAR	    0x00040000
#define GX_ATTR_UNSUPP	    0x00400000
#define GX_ATTR_SUPP	    0x00800000
#define GX_POLYG_OVERLAP    0x01000000
#define GX_POLYG_NONOVERLAP 0x02000000
#define GX_PATTERN_ZEROS    0x04000000
#define GX_PATTERN_ONES	    0x08000000
#define GX_PATTERN_MASK	    0x0c000000
#define GX_PIXEL_ZEROS	    0x10000000
#define GX_PIXEL_ONES	    0x20000000
#define GX_PIXEL_MASK	    0x30000000
#define GX_PLANE_ZEROS	    0x40000000
#define GX_PLANE_ONES	    0x80000000
#define GX_PLANE_MASK	    0xc0000000

typedef struct _sunGX {
	Uint	junk0[1];
	Uint	mode;
	Uint	clip;
	Uint	junk1[1];	    
	Uint	s;
	Uint	draw;
	Uint	blit;
	Uint	font;
	Uint	junk2[24];
	Uint	x0, y0, z0, color0;
	Uint	x1, y1, z1, color1;
	Uint	x2, y2, z2, color2;
	Uint	x3, y3, z3, color3;
	Uint	offx, offy;
	Uint	junk3[2];
	Uint	incx, incy;
	Uint	junk4[2];
	Uint	clipminx, clipminy;
	Uint	junk5[2];
	Uint	clipmaxx, clipmaxy;
	Uint	junk6[2];
	Uint	fg;
	Uint	bg;
	Uint	alu;
	Uint	pm;
	Uint	pixelm;
	Uint	junk7[2];
	Uint	patalign;
	Uint	pattern[8];
	Uint	junk8[432];
	Uint	apointx, apointy, apointz;
	Uint	junk9[1];
	Uint	rpointx, rpointy, rpointz;
	Uint	junk10[5];
	Uint	pointr, pointg, pointb, pointa;
	Uint	alinex, aliney, alinez;
	Uint	junk11[1];
	Uint	rlinex, rliney, rlinez;
	Uint	junk12[5];
	Uint	liner, lineg, lineb, linea;
	Uint	atrix, atriy, atriz;
	Uint	junk13[1];
	Uint	rtrix, rtriy, rtriz;
	Uint	junk14[5];
	Uint	trir, trig, trib, tria;
	Uint	aquadx, aquady, aquadz;
	Uint	junk15[1];
	Uint	rquadx, rquady, rquadz;
	Uint	junk16[5];
	Uint	quadr, quadg, quadb, quada;
	Uint	arectx, arecty, arectz;
	Uint	junk17[1];
	Uint	rrectx, rrecty, rrectz;
	Uint	junk18[5];
	Uint	rectr, rectg, rectb, recta;
} sunGX, *sunGXPtr;

/* Macros */

#define GX_ROP_USE_PIXELMASK	0x30000000

#define GX_BLT_INPROGRESS	0x20000000

#define GX_INPROGRESS		0x10000000
#define GX_FULL			0x20000000

#define GXWait(gx,r)\
    do\
	(r) = (int) (gx)->s; \
    while ((r) & GX_INPROGRESS)

#define GXDrawDone(gx,r) \
    do \
	(r) = (int) (gx)->draw; \
    while ((r) < 0 && ((r) & GX_FULL))

#define GXBlitDone(gx,r)\
    do\
	(r)= (int) (gx)->blit; \
    while ((r) < 0 && ((r) & GX_BLT_INPROGRESS))

#define GXBlitInit(gx,rop,pmsk) {\
    gx->fg = 0xff;\
    gx->bg = 0x00;\
    gx->pixelm = ~0;\
    gx->s = 0;\
    gx->alu = rop;\
    gx->pm = pmsk;\
    gx->clip = 0;\
}

#define GXDrawInit(gx,fore,rop,pmsk) {\
    gx->fg = fore;\
    gx->bg = 0x00; \
    gx->pixelm = ~0; \
    gx->s = 0; \
    gx->alu = rop; \
    gx->pm = pmsk; \
    gx->clip = 0;\
}

#define GXStippleInit(gx,stipple) {\
    int		_i; \
    Uint	*sp, *dp; \
    _i = 8;  \
    sp = stipple->bits; \
    dp = gx->pattern; \
    while (_i--) {  \
	dp[_i] =  sp[_i]; \
    } \
    gx->fg = stipple->fore; \
    gx->bg = stipple->back; \
    gx->patalign = stipple->patalign; \
    gx->alu = stipple->alu; \
}

extern int  sunGXScreenPrivateIndex;
extern int  sunGXGCPrivateIndex;
extern int  sunGXWindowPrivateIndex;

#define sunGXGetScreenPrivate(s)    ((sunGXPtr) \
			    (s)->devPrivates[sunGXScreenPrivateIndex].ptr)

typedef struct _sunGXStipple {
    Uint	fore, back;
    Uint	patalign;
    Uint	alu;
    Uint	bits[8];	/* actually 16 shorts */
} sunGXStippleRec, *sunGXStipplePtr;

typedef struct _sunGXPrivGC {
    int		    type;
    sunGXStipplePtr stipple;
} sunGXPrivGCRec, *sunGXPrivGCPtr;

#define sunGXGetGCPrivate(g)	    ((sunGXPrivGCPtr) \
			    (g)->devPrivates[sunGXGCPrivateIndex].ptr)

#define sunGXGetWindowPrivate(w)    ((sunGXStipplePtr) \
			    (w)->devPrivates[sunGXWindowPrivateIndex].ptr)

#define sunGXSetWindowPrivate(w,p) (\
	    (w)->devPrivates[sunGXWindowPrivateIndex].ptr = (pointer) p)

