/* $XConsortium: xieperf.h,v 1.5 93/07/27 14:31:47 rws Exp $ */

/**** module xieperf.h ****/
/******************************************************************************
				NOTICE
                              
This software is being provided by AGE Logic, Inc. and MIT under the
following license.  By obtaining, using and/or copying this software,
you agree that you have read, understood, and will comply with these
terms and conditions:

     Permission to use, copy, modify, distribute and sell this
     software and its documentation for any purpose and without
     fee or royalty and to grant others any or all rights granted
     herein is hereby granted, provided that you agree to comply
     with the following copyright notice and statements, including
     the disclaimer, and that the same appears on all copies and
     derivative works of the software and documentation you make.
     
     "Copyright 1993 by AGE Logic, Inc. and the Massachusetts
     Institute of Technology"
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC AND MIT
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC OR MIT OR LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The names of AGE Logic, Inc. and MIT may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic and MIT.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
*****************************************************************************
  
	xieperf.h -- xieperf header file

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#ifndef VMS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#else
#include <decw$include/Xlib.h>
#include <decw$include/Xutil.h>
#endif
#include <X11/Xfuncs.h>
#include <X11/extensions/XIElib.h>

#ifndef NULL
#define NULL 0
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *malloc();
#endif
#include <fcntl.h>

#define WIDTH         600	/* Size of large window to work within  */
#define HEIGHT        600

typedef Bool (*InitProc)    (/* XParms xp; Parms p */);
typedef void (*Proc)	    (/* XParms xp; Parms p */);

extern void NullProc	    (/* XParms xp; Parms p */);
extern Bool NullInitProc    (/* XParms xp; Parms p */);

#define VALL	   	 1		/* future use - see x11perf.h */

typedef unsigned char Version;		/* ditto */

static struct stat _Stat_Buffer;
#define file_size(path) ( stat(path,&_Stat_Buffer)== 0 ? \
                _Stat_Buffer.st_size :  -1)

#define	ClampInputs	1
#define	ClampOutputs	2

#define	Drawable	1
#define	DrawablePlane 	2

#define NoObscure 	0
#define Obscured	1
#define	Obscuring	2

#define Overlap		1
#define	NoOverlap	2

/* geometry stuff */

#define	GEO_TYPE_CROP		1
#define	GEO_TYPE_SCALE		2
#define	GEO_TYPE_MIRRORX	3
#define	GEO_TYPE_MIRRORY	4
#define	GEO_TYPE_MIRRORXY	5
#define GEO_TYPE_ROTATE		6
#define	GEO_TYPE_DEFAULT	7
#define GEO_TYPE_SCALEDROTATE   8

/* capabilities masks */

/* the low 8 bits of the short are for test requirements. Bit positions not
   defined below are reserved for future expansion */

#define	CAPA_COLOR_8	( 1 << 0 )	
#define	CAPA_COLOR_16	( 1 << 1 )	
#define	CAPA_COLOR_24	( 1 << 2 )	
#define	CAPA_TRIPLEBAND	( 1 << 3 )	

#define	CAPA_MASK	0x00ff

#define	IsColor8( x ) ( x & CAPA_COLOR_8 ? 1 : 0 )
#define	IsColor16( x ) ( x & CAPA_COLOR_16 ? 1 : 0 )
#define	IsColor24( x ) ( x & CAPA_COLOR_24 ? 1 : 0 )
#define IsTripleBand( x ) ( x & CAPA_TRIPLE_BAND ) ? 1 : 0 )

#define IsFaxImage( x ) ( x == xieValDecodeG42D   ||		\
			  x == xieValDecodeG32D   ||		\
			  x == xieValDecodeG31D )

/* protocol subset masks */

/* the high 8 bits of the short are for XIE subsets. Again, those bits not
   defined are reserved for future use */

#define SUBSET_MASK	0xff00
#define	SUBSET_FULL	( xieValFull << 8 )
#define SUBSET_DIS	( xieValDIS << 8 )

#define	IsFull( x ) ( x & SUBSET_FULL ? 1 : 0 )
#define IsDIS( x ) ( x & SUBSET_DIS ? 1 : 0 )
#define IsDISAndFull( x ) ( IsFullTest( x ) && IsDISTest( x ) )

/*
 * configuration shared by all tests 
 */
 
/* image configuration */

typedef struct _Image {
    char	*fname;		/* filename */
    int		fsize;	      	/* size in bytes. set in init function */
    int		data_class;    	/* singleband or tripleband */
    int		width;		/* width of image */
    int         height;         /* height of image */
    int         depth;		/* pixel depth */
    int         levels;		/* 1 << depth */
    int		decode;	     	/* decode method */
    int         fill_order;
    int         pixel_order;
    int         pixel_stride;
    int         scanline_pad;
    int         left_pad;
    unsigned int chksum;
} XIEimage;

/* a file represents an image. 3 files per test are supported */

typedef struct _XIEfile {
    XIEimage	*image1;	
    XIEimage	*image2;	
    XIEimage	*image3;	
} XIEifile;

/* test parameters */

typedef struct _Parms {
    /* Required fields */
    int  	objects;    /* Number of objects to process in one X call */
    /* Optional fields */
    int		description; /* server requirements flags */
    int         buffer_size; /* size when sending/reading data from xie */
    char        *data;       /* image data */
    XIEifile 	finfo;      /* image file info */	
    XPointer	ts;	    /* test specifics */		
} ParmRec, *Parms;

/*
 * test specific configuration. One structure per C source file.
 */

typedef struct _abortParms {
    int 	lutSize; 
    int		lutLevels;
} AbortParms;

typedef struct _awaitParms {
    int		lutSize;
    int		lutLevels;
} AwaitParms;

typedef struct _blendParms {
    XieConstant constant;
    XieFloat 	alphaConstant;
    int		bandMask;
} BlendParms;

typedef struct _constrainParms {
    int		constrain;
    int		clamp;
} ConstrainParms;

typedef struct _creatDstryParms {
    int		dummy;
} CreateDestroyParms;

typedef struct _cvtToIndexParms {
    int		dither;
} CvtToIndexParms;

typedef struct _ditherParms {
    int		dither;
    int		drawable;
} DitherParms;

typedef struct _exportClParms {
    int		dummy;
} ExportClParms;

typedef struct _floParms {
    int		dummy;
} FloParms;

typedef struct _floMapParms {
    int		dummy;
} FloMapParms;

typedef struct _geometryParms {
    int		geoType;
    int		geoHeight;
    int		geoWidth;
    int		geoXOffset;
    int		geoYOffset;
    XieGeometryTechnique geoTech;
    int		geoAngle;
} GeometryParms;

typedef struct _logicalParms {
    XieConstant	logicalConstant;
    unsigned long logicalOp;
    int		logicalBandMask;
} LogicalParms;

typedef struct _importParms {
    int		obscure;
} ImportParms;

typedef struct _importClParms {
    int		dummy;
} ImportClParms;

typedef struct _pasteUpParms {
    int		overlap;
} PasteUpParms;

typedef struct _redefineParms {
    XieConstant	constant;
    int		bandMask;
    unsigned long op1;
    unsigned long op2;
} RedefineParms;

typedef struct _modifyParms {
    XieConstant	constant;
    int		bandMask;
    unsigned long op;
} ModifyParms;

typedef struct _pointParms {
    int		levelsIn;
    int		levelsOut;
} PointParms;

typedef struct _unconstrainParms {
    int		constrain;
    int		clamp;
} UnconstrainParms;

typedef struct _purgeColStParms {
    int		dummy;
} PurgeColStParms;

typedef struct _queryParms {
    int		lutSize;
    int		lutLevels;
    XieTechniqueGroup techGroup;
} QueryParms;

typedef struct _XParms {
    Display	    *d;
    char	    *displayName;	/* see do_await.c */
    Window	    w;
    GC		    fggc;
    GC		    bggc;
    unsigned long   foreground;
    unsigned long   background;
    XVisualInfo     vinfo;
    Bool	    pack;
    Version	    version;
} XParmRec, *XParms;

typedef int TestType;

typedef struct _Test {
    char	*option;    /* Name to use in prompt line		    */
    char	*label;     /* Fuller description of test		    */
    InitProc    init;       /* Initialization procedure			    */
    Proc	proc;       /* Timed benchmark procedure		    */
    Proc	passCleanup;/* Cleanup between repetitions of same test     */
    Proc	cleanup;    /* Cleanup after test			    */
    Version     versions;   /* future expansion	    */
    TestType    testType;   /* future expansion     */
    int		clips;      /* Number of obscuring windows to force clipping*/
    ParmRec     parms;      /* Parameters passed to test procedures	    */
} Test;

extern Test test[];

#define ForEachTest(x) for (x = 0; test[x].option != NULL; x++)

XiePhotomap GetXIEPhotomap();
XieRoi GetXIERoi();
XieLut GetXIELut();

/*****************************************************************************

For repeatable results, XIEperf should be run using a local connection on a
freshly-started server.  The default configuration runs each test 5 times, in
order to see if each trial takes approximately the same amount of time.
Strange glitches should be examined; if non-repeatable I chalk them up to
daemons and network traffic.  Each trial is run for 5 seconds, in order to
reduce random time differences.  The number of objects processed per second is
displayed to 3 significant digits, but you'll be lucky on most UNIX system if
the numbers are actually consistent to 2 digits.

******************************************************************************/
