/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include "x11perf.h"

static XRectangle   *rects;
static GC	    pgc;

static unsigned char bitmap8x8[] = {
    0xCC, 0x66, 0x33, 0x99, 0xCC, 0x66, 0x33, 0x99
/*
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA
*/
};

static unsigned char bitmap4x4[] = {
   0x03, 0x06, 0x0c, 0x09
};


#define mensetmanus_width 161
#define mensetmanus_height 145

static char mensetmanus_bits[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff,
 0x01, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x07, 0x00, 0x00,
 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x3f, 0xdb, 0x0f, 0x00, 0x00, 0x00, 0x80, 0x07,
 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xf7, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x80, 0x61, 0x00, 0x00, 0xc6,
 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc3, 0xff,
 0x1f, 0x00, 0x00, 0x00, 0xc0, 0xe1, 0xff, 0xff, 0x87, 0x01, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc3, 0xf6, 0x1f, 0x00, 0x00,
 0x00, 0xc0, 0xc0, 0xff, 0xff, 0x87, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x80, 0x9f, 0x65, 0x1f, 0x00, 0x00, 0x00, 0xc0, 0x00,
 0x07, 0x80, 0x87, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x80, 0x9f, 0x6d, 0x1f, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xff, 0xff, 0xc7,
 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x89, 0x6d,
 0x1b, 0x00, 0x00, 0x00, 0xc0, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x81, 0x6d, 0x1b, 0x00, 0x00,
 0x00, 0xc0, 0xe1, 0x5b, 0xdb, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0xc0, 0x81, 0x6d, 0x1b, 0x00, 0x00, 0x00, 0x80, 0x83,
 0x5b, 0xdb, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0xc0, 0x81, 0x6d, 0x1b, 0x00, 0x00, 0x00, 0x00, 0xcf, 0x5b, 0x1b, 0x0c,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x6d,
 0x1b, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x5b, 0x1b, 0x0c, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x80, 0x6d, 0x1b, 0x00, 0x00,
 0x00, 0x00, 0xf8, 0x5b, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0xe0, 0x83, 0x6d, 0x19, 0x00, 0x00, 0x00, 0x00, 0x80,
 0x5b, 0x0b, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0xe0, 0x83, 0x6d, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x80, 0x5b, 0x0f, 0x0e,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x6d,
 0x1b, 0x00, 0x00, 0x00, 0x00, 0x80, 0x4b, 0x0f, 0x07, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x79, 0x1b, 0x00, 0x00,
 0x00, 0x00, 0x80, 0x6b, 0xbf, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x03, 0xfb, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x80,
 0x6d, 0xb7, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x03, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xed, 0xf7, 0x07,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xfe,
 0x7f, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xed, 0xd7, 0x07, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0xf8, 0xff, 0x07, 0x00,
 0x00, 0x00, 0xe0, 0xfe, 0xd7, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x80, 0xff, 0x7f, 0xe0, 0xff, 0x3f, 0x00, 0x00, 0x00, 0xf0,
 0xff, 0xd6, 0xfe, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0,
 0xbf, 0x73, 0x80, 0x3b, 0x7c, 0x00, 0x00, 0x00, 0xf8, 0xe7, 0xd6, 0xef,
 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xb9, 0x73, 0xc0,
 0x3b, 0xf0, 0x01, 0x00, 0x00, 0xff, 0xc3, 0xbf, 0xe7, 0x3f, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xb8, 0x7f, 0xe0, 0x3b, 0xc0, 0x03,
 0x00, 0xc0, 0xbf, 0xc7, 0xbf, 0xe7, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x1c, 0xb8, 0xff, 0xf8, 0x3b, 0x80, 0x07, 0x00, 0xe0, 0x61,
 0x87, 0xfd, 0xe7, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e,
 0xb8, 0xf3, 0xff, 0x3b, 0x00, 0x0f, 0x00, 0x78, 0x60, 0x8e, 0xf1, 0x67,
 0xc7, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xb8, 0xc3, 0x8f,
 0x7b, 0x00, 0x0e, 0x00, 0x38, 0xe0, 0x8c, 0x03, 0x66, 0x87, 0x03, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xb8, 0x03, 0x83, 0x7b, 0x00, 0x1e,
 0x00, 0x1c, 0xe0, 0x1d, 0x03, 0x76, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xc0, 0x07, 0xb8, 0xe3, 0x8f, 0xbb, 0x01, 0x39, 0x00, 0x1e, 0xe0,
 0x3b, 0x03, 0xf6, 0x87, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f,
 0xb8, 0xe3, 0x8f, 0x3b, 0xc6, 0x38, 0x00, 0x17, 0xe0, 0x73, 0x07, 0xfe,
 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0b, 0xb8, 0xe7, 0x8e,
 0x3b, 0x38, 0x78, 0x00, 0x13, 0xe0, 0x77, 0x06, 0xfe, 0xff, 0xff, 0xff,
 0x0f, 0x00, 0x00, 0x00, 0xe0, 0x12, 0x3c, 0xff, 0xce, 0x3b, 0x00, 0x78,
 0x80, 0x13, 0x60, 0xef, 0x0e, 0xfe, 0x00, 0x00, 0x60, 0x08, 0x00, 0x00,
 0x00, 0x70, 0x22, 0x3e, 0xfc, 0xfe, 0x39, 0x00, 0xe8, 0x80, 0x61, 0x60,
 0xce, 0x0f, 0xfe, 0x00, 0x00, 0x60, 0xf8, 0x00, 0x00, 0x00, 0x70, 0xc2,
 0x39, 0xf8, 0x7e, 0x38, 0x00, 0xc8, 0xc0, 0x81, 0x71, 0xde, 0x0f, 0xfe,
 0x00, 0x00, 0x60, 0x88, 0x00, 0x00, 0x00, 0x70, 0x04, 0x38, 0xe0, 0x0e,
 0x38, 0x00, 0xc4, 0xc0, 0x01, 0x7e, 0xbc, 0x1f, 0xfe, 0x00, 0x00, 0x60,
 0x88, 0x00, 0x00, 0x00, 0x78, 0x04, 0x38, 0xe0, 0x0e, 0x38, 0x00, 0xc4,
 0xc1, 0x01, 0x60, 0x3c, 0x1f, 0xfe, 0x00, 0x00, 0x60, 0x84, 0x0f, 0x00,
 0x00, 0x78, 0x08, 0x38, 0xe0, 0x0f, 0x38, 0x00, 0x82, 0xe1, 0x01, 0x60,
 0x74, 0x3e, 0xfe, 0x00, 0x00, 0x60, 0x44, 0x08, 0x00, 0x00, 0xb8, 0x10,
 0x38, 0xe0, 0x0f, 0x38, 0x00, 0x81, 0xe1, 0x00, 0x60, 0xe4, 0x3e, 0xfe,
 0x00, 0x00, 0x60, 0x44, 0x08, 0x00, 0x00, 0x98, 0x20, 0x38, 0xf8, 0x3f,
 0x38, 0x80, 0x80, 0xe1, 0x01, 0x60, 0xc4, 0x3c, 0xfe, 0x00, 0x00, 0x60,
 0x24, 0x0c, 0x00, 0x00, 0x1c, 0xc1, 0x39, 0x3c, 0x78, 0x38, 0x40, 0x80,
 0x61, 0x01, 0x60, 0xc4, 0x79, 0xfe, 0x00, 0x00, 0x60, 0x22, 0x32, 0x00,
 0x00, 0x1c, 0x01, 0x3e, 0x1c, 0xf0, 0x38, 0xf0, 0x9f, 0x61, 0x01, 0x60,
 0x84, 0x7b, 0xee, 0x00, 0x00, 0x60, 0x12, 0x21, 0x00, 0x00, 0x1c, 0x02,
 0x38, 0x0e, 0xe7, 0xf8, 0x3f, 0xe0, 0x61, 0x00, 0x60, 0x04, 0xf7, 0xce,
 0x00, 0x00, 0x60, 0x92, 0x10, 0x00, 0x00, 0x1c, 0x04, 0x38, 0x06, 0xff,
 0x38, 0x40, 0x80, 0x61, 0x02, 0x60, 0x04, 0xe7, 0xcf, 0x00, 0x00, 0x60,
 0x8a, 0x18, 0x00, 0x00, 0x1c, 0x18, 0x38, 0x07, 0xf0, 0x38, 0x00, 0x81,
 0x61, 0x02, 0x60, 0x04, 0xee, 0xcf, 0xff, 0x03, 0x60, 0x49, 0xe4, 0x00,
 0x00, 0x1c, 0x70, 0x38, 0x07, 0xe7, 0x38, 0x00, 0x82, 0x61, 0x04, 0x60,
 0x04, 0xdc, 0xcf, 0x01, 0x04, 0x60, 0x25, 0x42, 0x00, 0x00, 0x1c, 0xfe,
 0x3f, 0x03, 0xff, 0x38, 0x00, 0x84, 0x61, 0x08, 0x60, 0x04, 0xfc, 0xcf,
 0xf8, 0xff, 0x60, 0x95, 0x31, 0x00, 0x00, 0xfc, 0x01, 0xff, 0x03, 0xf0,
 0xf8, 0x1f, 0x88, 0x61, 0x10, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x00, 0xe1,
 0x53, 0x0c, 0x00, 0x00, 0x38, 0xf0, 0xff, 0x03, 0xef, 0xf8, 0x7f, 0x90,
 0x61, 0x40, 0xe0, 0xff, 0xff, 0xff, 0x00, 0xff, 0xe3, 0x2b, 0xfe, 0x00,
 0x00, 0x18, 0xf8, 0xff, 0x03, 0xff, 0x7c, 0xf0, 0x90, 0x61, 0x00, 0x3e,
 0x36, 0xe3, 0xe1, 0x00, 0x00, 0xe3, 0x9f, 0x60, 0x00, 0x00, 0x38, 0x3c,
 0x00, 0x03, 0xf0, 0x1f, 0xe0, 0xa0, 0xe1, 0x00, 0x30, 0x22, 0xe2, 0xc0,
 0x00, 0xff, 0xe3, 0x7f, 0x18, 0x00, 0x00, 0x38, 0x0e, 0x00, 0x07, 0xe7,
 0x07, 0xc0, 0xe1, 0xc1, 0x00, 0x18, 0x22, 0xe2, 0xc0, 0x00, 0x00, 0xe1,
 0x1f, 0x07, 0x00, 0x00, 0x38, 0x06, 0x00, 0x07, 0xef, 0x01, 0xc0, 0xc1,
 0xc1, 0x01, 0x16, 0x24, 0xe2, 0xc0, 0x01, 0xff, 0xe1, 0xff, 0x00, 0x00,
 0x00, 0x70, 0x07, 0x00, 0x0e, 0xf8, 0x00, 0x80, 0xc3, 0xc0, 0xf1, 0x11,
 0x24, 0xe4, 0xc0, 0x03, 0xe0, 0xe1, 0x7f, 0x00, 0x00, 0x00, 0x70, 0x07,
 0x00, 0x1e, 0x3c, 0x00, 0x80, 0xe3, 0x80, 0x0f, 0x10, 0x24, 0xe4, 0xc0,
 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0xe0, 0x03, 0x00, 0xfc, 0x0f,
 0x00, 0x00, 0x63, 0x80, 0x01, 0x08, 0x44, 0xe4, 0xc0, 0xff, 0xff, 0xff,
 0xff, 0xff, 0x01, 0x00, 0xe0, 0x03, 0x00, 0xf8, 0x07, 0x00, 0x00, 0x73,
 0x80, 0x03, 0x08, 0x44, 0xe4, 0xc0, 0xfc, 0x00, 0x07, 0x0e, 0x00, 0x00,
 0x00, 0xc0, 0x03, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x33, 0x00, 0x03, 0x08,
 0x44, 0xe4, 0xe0, 0xcc, 0x00, 0x07, 0x0e, 0x00, 0x00, 0x00, 0xc0, 0x03,
 0x00, 0x78, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x07, 0x04, 0x44, 0xe8, 0x70,
 0xcc, 0x01, 0x87, 0x0f, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00, 0x3e, 0x00,
 0x00, 0x80, 0x1f, 0x00, 0x0e, 0x04, 0x44, 0xe8, 0x38, 0xcc, 0x81, 0xe7,
 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0f, 0x00, 0x00, 0x80, 0x0f,
 0x00, 0x1c, 0x02, 0x44, 0xf8, 0x1e, 0xcc, 0xe1, 0xff, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x0f, 0xc0, 0x23, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x38, 0x01,
 0x42, 0xfc, 0x0f, 0xcc, 0xf1, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e,
 0xf0, 0x39, 0x00, 0x00, 0xe0, 0x03, 0x00, 0xf0, 0x00, 0xc2, 0xff, 0x07,
 0xcc, 0x7f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x7c, 0x0e, 0x00,
 0x00, 0xf0, 0x01, 0x00, 0xe0, 0x03, 0xe2, 0xe7, 0x07, 0xcc, 0x1f, 0x07,
 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xbf, 0x23, 0xe0, 0xff, 0xff, 0x00,
 0x00, 0xc0, 0x1f, 0xff, 0x64, 0x06, 0xcc, 0x07, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0xf0, 0xff, 0x3c, 0xf0, 0xff, 0x7f, 0x00, 0x00, 0x00, 0xff,
 0x7f, 0x64, 0x06, 0xcc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70,
 0xf8, 0x1f, 0x78, 0x7c, 0x7f, 0x00, 0x00, 0x00, 0xfc, 0x20, 0x64, 0x06,
 0xcc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0xf8, 0x33, 0x1c,
 0x38, 0x66, 0x00, 0x00, 0x00, 0x0c, 0x20, 0x6c, 0x06, 0xcc, 0x03, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x78, 0x3e, 0x0e, 0x38, 0x76, 0x00,
 0x00, 0x00, 0x0c, 0x20, 0x68, 0x06, 0xcc, 0x03, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x38, 0xf8, 0x87, 0x0f, 0x38, 0x76, 0x00, 0x00, 0x00, 0x0c,
 0x20, 0x68, 0x06, 0xcc, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38,
 0x78, 0xf8, 0x0f, 0x38, 0x76, 0x00, 0x00, 0x00, 0x0c, 0x10, 0x68, 0x06,
 0xcc, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xf8, 0xff, 0x0f,
 0x38, 0x76, 0x00, 0x00, 0x00, 0x0c, 0x10, 0x68, 0x06, 0xcc, 0x07, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xf8, 0xff, 0x0e, 0x38, 0x76, 0x00,
 0x00, 0x00, 0x0c, 0x10, 0x68, 0x06, 0xcc, 0x07, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x1c, 0xf8, 0xe1, 0x0e, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x0c,
 0x08, 0x68, 0x06, 0xcc, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x0c, 0x08, 0x68, 0x06,
 0xcc, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x38, 0xe0, 0x0e,
 0x38, 0x3e, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x68, 0x06, 0xcc, 0x0e, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00,
 0x00, 0x00, 0x1c, 0x04, 0x68, 0x06, 0xcc, 0x0e, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x06, 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x1c,
 0x02, 0x68, 0x06, 0xcc, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06,
 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x1c, 0x02, 0x68, 0x06,
 0xcc, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x38, 0xe0, 0x0e,
 0x38, 0x3e, 0x00, 0x00, 0x00, 0x1c, 0x01, 0x68, 0x06, 0xcc, 0x1c, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00,
 0x00, 0x00, 0x18, 0x01, 0x68, 0x06, 0xcc, 0x1c, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x03, 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x98,
 0x00, 0x68, 0x06, 0xcc, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03,
 0x38, 0xe0, 0x0e, 0x38, 0x3e, 0x00, 0x00, 0x00, 0x98, 0x00, 0x68, 0x06,
 0xcc, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x38, 0xe0, 0x0e,
 0x38, 0x1e, 0x00, 0x00, 0x00, 0x58, 0x00, 0x6c, 0x0e, 0xcc, 0x18, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x30, 0xe0, 0x0e, 0x38, 0x1e, 0x00,
 0x00, 0x00, 0x38, 0x00, 0x64, 0x0e, 0xcc, 0x18, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xc0, 0x01, 0x70, 0xe0, 0x0e, 0x1c, 0x1e, 0x00, 0x00, 0x00, 0x18,
 0x00, 0x64, 0x1e, 0xcc, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01,
 0xe0, 0xe1, 0x0e, 0x0f, 0x1e, 0x00, 0x00, 0x00, 0x18, 0x00, 0x64, 0x3e,
 0xcc, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xc0, 0xef, 0xce,
 0x07, 0x1e, 0x00, 0x00, 0x00, 0x18, 0x00, 0x64, 0x3e, 0xcc, 0x30, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0xff, 0xfe, 0x03, 0x1e, 0x00,
 0x00, 0x00, 0x18, 0x00, 0x64, 0x7e, 0xcc, 0x30, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0x00, 0xfc, 0xfe, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x18,
 0x00, 0x64, 0x66, 0xcc, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0x00, 0xe0, 0x1e, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x18, 0x00, 0x66, 0xe6,
 0xcc, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xe0, 0x0e,
 0x00, 0x1e, 0x00, 0x00, 0x00, 0x18, 0x00, 0x62, 0xc6, 0xcd, 0x70, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0xfe, 0xff, 0x0f, 0x1e, 0x00,
 0x00, 0x00, 0x18, 0x00, 0x62, 0xc6, 0xcd, 0x70, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x70, 0x00, 0xc0, 0xff, 0xff, 0x0f, 0x0e, 0x00, 0x00, 0x00, 0x38,
 0x00, 0x62, 0x86, 0xcf, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
 0xf0, 0xff, 0xff, 0x0f, 0x0e, 0x00, 0x00, 0x00, 0x38, 0x00, 0x61, 0x86,
 0xcf, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0xfc, 0x00, 0x00,
 0x0e, 0x0e, 0x00, 0x00, 0x00, 0x38, 0x00, 0x61, 0x8e, 0xcf, 0xe0, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x78, 0x00, 0x00, 0x0e, 0x0e, 0x00,
 0x00, 0x00, 0xf8, 0xff, 0x7f, 0xfe, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x18, 0x00, 0xf0, 0x00, 0x00, 0x0e, 0x0e, 0x00, 0x00, 0x00, 0xf0,
 0xff, 0x3f, 0xff, 0xdf, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00,
 0xe0, 0x01, 0x00, 0x0e, 0x0e, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x8f, 0x07,
 0x9e, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0xe0, 0x07, 0x00,
 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x30, 0x00, 0xc0, 0xff, 0x3f, 0xc0, 0x01,
 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x0f, 0x00,
 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00,
 0x00, 0x1c, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0xf0,
 0xff, 0xff, 0x00, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff,
 0x3f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x38, 0x00,
 0x38, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x7f, 0x00, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x38, 0x00, 0x38, 0xe0, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0x06, 0x18, 0x00, 0x38, 0xe0, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
 0x06, 0x18, 0x00, 0x38, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0xdc, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1c, 0x00,
 0x38, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x9c, 0x03, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1c, 0x00, 0x18, 0x70, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1c, 0x07, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0x07, 0x0c, 0x00, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0x1c, 0xfe, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00,
 0x07, 0x0c, 0x00, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0x1c, 0xfc, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0c, 0x00,
 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1c, 0x1c, 0x00,
 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0e, 0x00, 0x18, 0x70, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1c, 0x1c, 0x00, 0x80, 0x03, 0x00,
 0x00, 0x00, 0x00, 0x07, 0x0e, 0x00, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0x1c, 0x1c, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
 0x07, 0x0e, 0x00, 0x18, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0x1c, 0x1c, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0e, 0x00,
 0x18, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1c, 0x1c, 0x00,
 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0x06, 0x00, 0x18, 0x38, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x07, 0x00,
 0x00, 0x00, 0x00, 0x07, 0x06, 0x00, 0x18, 0x38, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00,
 0x07, 0x0e, 0x00, 0x18, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0x1c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0e, 0x00,
 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1c, 0x00, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0e, 0x00, 0x18, 0x30, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0x07, 0x0e, 0x00, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
 0x07, 0x0e, 0x00, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0x1e, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0c, 0x00,
 0x38, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1e, 0x00, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1c, 0x00, 0x38, 0x70, 0x00,
 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0x06, 0x1c, 0x00, 0x38, 0xe0, 0x00, 0x00, 0x00, 0x00,
 0x00, 0xe0, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
 0x06, 0x18, 0x00, 0x38, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
 0x1e, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x18, 0x00,
 0x30, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x1e, 0x00, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x38, 0x00, 0x30, 0xe0, 0x01,
 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x1f, 0x00, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0x0c, 0x3c, 0x00, 0x30, 0xf0, 0x01, 0x00, 0x00, 0x00,
 0x00, 0xf8, 0x03, 0x1f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
 0x0c, 0x7e, 0x00, 0x70, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x0f,
 0x1f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0c, 0xe7, 0x00,
 0x70, 0xbc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x1e, 0x1f, 0x00, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xdc, 0xe3, 0x00, 0x60, 0x1e, 0x07,
 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x1f, 0x00, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0xfc, 0xc1, 0x01, 0xe0, 0x0f, 0x0f, 0x00, 0x00, 0x00,
 0x80, 0x07, 0xf0, 0x1f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
 0xf8, 0x80, 0x03, 0xe0, 0x03, 0x1e, 0x00, 0x00, 0x00, 0xc0, 0x03, 0xe0,
 0x1b, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x78, 0x80, 0x07,
 0xe0, 0x01, 0x1c, 0x00, 0x00, 0x00, 0xe0, 0x01, 0xc0, 0x1b, 0x00, 0x00,
 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x0f, 0xe0, 0x01, 0x38,
 0x00, 0x00, 0x00, 0xf0, 0x00, 0xc0, 0x19, 0x00, 0x00, 0x00, 0x06, 0x00,
 0x00, 0x00, 0x00, 0x30, 0x0e, 0x1e, 0xc0, 0x71, 0xf8, 0x00, 0x00, 0x00,
 0x78, 0x00, 0xdc, 0x19, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
 0x30, 0x1f, 0x1e, 0xc0, 0xf9, 0xf8, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff,
 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff,
 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff,
 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


int InitRectangles(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i;
    int size = p->special;
    int x, y;
    int rows;

    pgc = xp->fggc;

    rects = (XRectangle *)malloc(p->objects * sizeof(XRectangle));
    x = 0;
    y = 0;
    rows = 0;

    for (i = 0; i != p->objects; i++) {
	rects[i].x = x;
        rects[i].y = y;
	rects[i].width = rects[i].height = size;

	y += size;
	rows++;
	if (y + size > HEIGHT || rows == MAXROWS) {
	    rows = 0;
	    y = 0;
	    x += size;
	    if (x + size > WIDTH) {
		x = 0;
	    }
	}
    }

    if (p->fillStyle == FillStippled || p->fillStyle == FillOpaqueStippled) {
	Pixmap      stipple;
	XGCValues   gcv;

	stipple = XCreateBitmapFromData(xp->d, xp->w, bitmap8x8, 8, 8);
	gcv.stipple = stipple;
	gcv.fill_style = p->fillStyle;
	XChangeGC(xp->d, xp->fggc, GCFillStyle | GCStipple, &gcv);
	XChangeGC(xp->d, xp->bggc, GCFillStyle | GCStipple, &gcv);
	XFreePixmap(xp->d, stipple);

    } else if (p->fillStyle == FillTiled) {
	Pixmap      tile;
	XGCValues   gcv;

	gcv.fill_style = FillTiled;
	if (!p->font) {
	    /* Little 4x4 tile */
	    tile = XCreatePixmapFromBitmapData(xp->d, xp->w, bitmap4x4, 4, 4,
		    xp->foreground, xp->background,
		    DefaultDepth(xp->d, DefaultScreen(xp->d)));
	} else {
	    /* Enormous tile.  Well, pretty big. */
	    tile = XCreatePixmapFromBitmapData(xp->d, xp->w,
		    mensetmanus_bits, mensetmanus_width, mensetmanus_height,
		    xp->foreground, xp->background,
		    DefaultDepth(xp->d, DefaultScreen(xp->d)));
	}
	gcv.tile = tile;
	XChangeGC(xp->d, xp->fggc, GCFillStyle | GCTile, &gcv);
	XFreePixmap(xp->d, tile);
	if (!p->font) {
	    tile = XCreatePixmapFromBitmapData(xp->d, xp->w, bitmap4x4, 4, 4,
		    xp->background, xp->foreground,
		    DefaultDepth(xp->d, DefaultScreen(xp->d)));
	} else {
	    tile = XCreatePixmapFromBitmapData(xp->d, xp->w,
		    mensetmanus_bits, mensetmanus_width, mensetmanus_height,
		    xp->background, xp->foreground,
		    DefaultDepth(xp->d, DefaultScreen(xp->d)));
	}
	gcv.fill_style = FillTiled;
	gcv.tile = tile;
	XChangeGC(xp->d, xp->bggc, GCFillStyle | GCTile, &gcv);
	XFreePixmap(xp->d, tile);
    }

    return reps;
}

void DoRectangles(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int i;

    for (i = 0; i != reps; i++) {
        XFillRectangles(xp->d, xp->w, pgc, rects, p->objects);
        if (pgc == xp->bggc)
            pgc = xp->fggc;
        else
            pgc = xp->bggc;
    }
}

void EndRectangles(xp, p)
    XParms  xp;
    Parms p;
{
    free(rects);
}

