/*
 * (c) Copyright 1990 Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		SCCDFile.h
 *
 *	DESCRIPTION
 *		Include file for TekCMS Color Extension when using the
 *		X Device Color Characterization Convention (XDCCC).
 *
 *	REVISION
 *		$Header: SCCDFile.h,v 1.2 90/12/13 07:44:21 adamsc Exp $
 */
#ifndef SCCDFILE_H
#define SCCDFILE_H

#include "XcmsLRGBSD.h"
#include "XcmsGRAYSD.h"

/*
 *	DEFINES
 */

#define	READABLE_SD_SUFFIX		".txt"
#define TXT_FORMAT_VERSION		"0.2"

#define	DATA_DELIMS			" "	/* space */

#define SC_BEGIN_KEYWORD		"SCREENDATA_BEGIN"
#define SC_END_KEYWORD			"SCREENDATA_END"
#define COMMENT_KEYWORD			"COMMENT"
#define NAME_KEYWORD			"NAME"
#define MODEL_KEYWORD			"MODEL"
#define PART_NUMBER_KEYWORD		"PART_NUMBER"
#define SERIAL_NUMBER_KEYWORD		"SERIAL_NUMBER"
#define REVISION_KEYWORD		"REVISION"
#define SCREEN_CLASS_KEYWORD		"SCREEN_CLASS"
#define COLORIMETRIC_BEGIN_KEYWORD	"COLORIMETRIC_BEGIN"
#define COLORIMETRIC_END_KEYWORD	"COLORIMETRIC_END"
#define XYZTORGBMAT_BEGIN_KEYWORD	"XYZtoRGB_MATRIX_BEGIN"
#define XYZTORGBMAT_END_KEYWORD		"XYZtoRGB_MATRIX_END"
#define RGBTOXYZMAT_BEGIN_KEYWORD	"RGBtoXYZ_MATRIX_BEGIN"
#define RGBTOXYZMAT_END_KEYWORD		"RGBtoXYZ_MATRIX_END"
#define IPROFILE_BEGIN_KEYWORD		"INTENSITY_PROFILE_BEGIN"
#define IPROFILE_END_KEYWORD		"INTENSITY_PROFILE_END"
#define ITBL_BEGIN_KEYWORD		"INTENSITY_TBL_BEGIN"
#define ITBL_END_KEYWORD		"INTENSITY_TBL_END"

#define WHITEPT_XYZ_BEGIN_KEYWORD	"WHITEPT_XYZ_BEGIN"
#define WHITEPT_XYZ_END_KEYWORD		"WHITEPT_XYZ_END"

#define VIDEO_RGB_KEYWORD		"VIDEO_RGB"
#define VIDEO_GRAY_KEYWORD		"VIDEO_GRAY"

#define DATA				-1
#define SC_BEGIN			1
#define SC_END				2
#define COMMENT				3
#define NAME				4
#define MODEL				5
#define PART_NUMBER			6
#define SERIAL_NUMBER			7
#define REVISION			8
#define SCREEN_CLASS			9
#define COLORIMETRIC_BEGIN		10
#define COLORIMETRIC_END		11
#define XYZTORGBMAT_BEGIN		12
#define XYZTORGBMAT_END			13
#define RGBTOXYZMAT_BEGIN		14
#define RGBTOXYZMAT_END			15
#define IPROFILE_BEGIN			16
#define IPROFILE_END			17
#define ITBL_BEGIN			18
#define ITBL_END			19
#define WHITEPT_XYZ_BEGIN		20
#define WHITEPT_XYZ_END			21

#define CORR_TYPE_NONE  -1
#define CORR_TYPE_0	0
#define CORR_TYPE_1	1
#define CORR_TABLE_1	0
#define CORR_TABLE_3	2

#define VIDEO_RGB	0
#define VIDEO_GRAY	1
#endif /* SCCDFILE_H */
