/* $XConsortium$ */
/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"Xatom.h"

static XCharStruct perchar[] = {
	{0, 128, 129, 66, -3, 0},	/* 0 */
	{0, 127, 128, 63, 1, 0},	/* 1 */
	{-1, 125, 63, 64, 0, 0},	/* 2 */
	{1, 126, 63, 64, 0, 0},	/* 3 */
};
static XFontProp props[] = {
	{XA_COPYRIGHT, 0},
};

char	*xtfont5cpright = "These glyphs are unencumbered";

XFontStruct xtfont5 = {
	(XExtData*)0,
	(Font)0,
	FontLeftToRight,	/* direction */
	0,	/* min_byte2 */
	3,	/* max_byte2 */
	0,	/* min_byte1 */
	0,	/* max_byte1 */
	1,	/* all chars exist */
	1234,	/* default char */
	1,	/* n_properties */
	props,
	{-1, 125, 63, 63, -3, 0},
	{1, 128, 129, 66, 1, 0},
	perchar,
	66,	/* font ascent */
	1,	/* font descent */
};
