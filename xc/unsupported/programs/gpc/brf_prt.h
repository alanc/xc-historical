/* $XConsortium: brf_prt.h,v 5.2 91/02/16 10:07:16 rws Exp $ */

/*
 */

/***********************************************************
Copyright 1989, 1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

						All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * Copyright (c) 1989, 1990, 1991 by M.I.T. and Sun Microsystems, Inc.
 */

/*--------------------------------------------------------------------*\
|  Copyright (C) 1989, 1990, 1991, National Computer Graphics Association
|
|  Permission is granted to any individual or institution to use, copy, or
|  redistribute this software so long as it is not sold for profit, provided
|  this copyright notice is retained.
|
|                         Developed for the
|                National Computer Graphics Association
|                         2722 Merrilee Drive
|                         Fairfax, VA  22031
|                           (703) 698-9600
|
|                                by
|                 SimGraphics Engineering Corporation
|                    1137 Huntington Drive  Unit A
|                      South Pasadena, CA  91030
|                           (213) 255-0900
|---------------------------------------------------------------------
|
| Author        :	Norman D. Evangelista
|
| File          :	brf_prt.h
| Date          :	Tue Jul  4 14:08:02 PDT 1989
| Project       :	BIF Benchmark Report Format
| Description   :	Definitions and constants for report generation
| Status        :	Version 1.0
|
| Revisions     :
|
|       2/90            MFC Tektronix, Inc.: PEX-SI API implementation.
|
|      12/90            MFC Tektronix, Inc.: PEX-SI PEX5R1 Release.
|
\*--------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>

#define N_TOCOLON	25
#define N_TITLELINES	5
#define N_HEADFILES	4

char *brf_title[] = 
{
"------------------------------------------------------------------------\n",
"|          Graphics Performance Characterization Committee             |\n",
"|                Standard Graphics System Benchmarks                   |\n",
"|----------------------------------------------------------------------|\n",
"| Date                    :                                            |\n",
NULL
};

char *brf_beninfo[] = 
{
"|                                                                      |\n",
"|----------------------------------------------------------------------|\n",
"|   Benchmark Information                                              |\n",
"|----------------------------------------------------------------------|\n",
"| Benchmark Title         :   Level 2: Picture Level Benchmark         |\n",
"| Benchmark Version       :   1.00                                     |\n",
"| Benchmark Date          :   Mon Jul  3 18:43:54 PDT 1989             |\n",
"| Benchmark Authors       :   SimGraphics Engineering Corporation      |\n",
"|                                                                      |\n",
NULL
};

char *brf_impinfo[] = 
{
"|----------------------------------------------------------------------|\n",
"|   Implementation Information                                         |\n",
"|----------------------------------------------------------------------|\n",
"| Implementation Title    :   PEX-SI API Port                          |\n",
"| Implementation Version  :   1.20                                     |\n",
"| Implementation Date     :   Mon Nov  5 08:45:00 PDT 1990             |\n",
"| Implementation Authors  :   Tektronix, Inc                           |\n",
"|                                                                      |\n",
"| Graphics Library        :   PEX c-binding PHIGS/PHIGS+               |\n",
"| Graphics Library Version:   PEX5R1                                   |\n",
"|                                                                      |\n",
NULL
};

#ifdef EXTERNALNOTE
	/* the figure for window size is hard wired into this
	next block. In the case of the Alliant, the window always
	opens to this size. To prevent a possible error, the window
	should be locked against a user resize using the pesc() function.
	The ultimate answer is to be able to read the window size and insert
	it into this report. Size should be read just before
	and just after the test loop. If there was any change in 
	size or position (unlikely), then someone moved the window
	and the test timing is invalid. */
#endif

char *brf_sysinfo[] = 
{
"|----------------------------------------------------------------------|\n",
"|   System Configuration                                               |\n",
"|----------------------------------------------------------------------|\n",
"|  System Make and Model  :   Unknown                                  |\n",
"|  Hardware Configuration :   Unknown                                  |\n",
"|                                                                      |\n",
"|                                                                      |\n",
"|  Operating System       :   Unknown                                  |\n",
"|  Windowing System       :   Unknown                                  |\n",
"|  PLB Window Size        :   Unknown                                  |\n",
"|  Stopwatch Accuracy     :   Unknown                                  |\n",
"|                                                                      |\n",
"------------------------------------------------------------------------\n",
NULL
};

char *brf_timeinfo[] = 
{
"|----------------------------------------------------------------------|\n",
"|   Test Loop Timing Information                                       |\n",
"|----------------------------------------------------------------------|\n",
"|  Number of Frames        :                                           |\n",
"|  Elapsed Time (sec)      :                                           |\n",
"|  Transport Delay         :                                           |\n",
"|  Avg. Frames per Second  :                                           |\n",
"|  Avg. Time per Frame     :                                           |\n",
"|  Timing Merit Mthd 1     :                                           |\n",
"|  Timing Merit Mthd 2     :                                           |\n"
,NULL
};


char *brf_exceptioninfo[] = 
{
"|----------------------------------------------------------------------|\n",
"|   Global Exceptions Encountered\n",
"|----------------------------------------------------------------------|\n"
,NULL
};

#if 0 /* WORKING : to be deleted */
char	*BRF_hedrFiles[] =
{
	"brf_title.b",
	"brf_beninf.b",
	"brf_impinf.b",
	"brf_sysinf.b"
};
#endif

char	**BRF_hedrData[] =
{
	brf_title,
	brf_beninfo,
	brf_impinfo,
	brf_sysinfo
};

char	**BRF_timeData[] =
{
	brf_timeinfo
};

char	*BRF_timeFiles[] =
{
	"brf_tmeinf.b"
};

