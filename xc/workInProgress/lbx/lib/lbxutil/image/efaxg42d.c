/* $XConsortium$ */
/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992 Sam Leffler
 * Copyright (c) 1991, 1992 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include "lbxfax.h"
#include "lbximage.h"

/*
 * Note that these tables are ordered such that the
 * index into the table is known to be either the
 * run length, or (run length / 64) + a fixed offset.
 *
 * NB: The G3CODE_INVALID entries are only used
 *     during state generation (see mkg3states.c).
 */

tableentry TIFFFaxWhiteCodes[] = {
    { 8, 0x35, 0 },	/* 0011 0101 */
    { 6, 0x7, 1 },	/* 0001 11 */
    { 4, 0x7, 2 },	/* 0111 */
    { 4, 0x8, 3 },	/* 1000 */
    { 4, 0xB, 4 },	/* 1011 */
    { 4, 0xC, 5 },	/* 1100 */
    { 4, 0xE, 6 },	/* 1110 */
    { 4, 0xF, 7 },	/* 1111 */
    { 5, 0x13, 8 },	/* 1001 1 */
    { 5, 0x14, 9 },	/* 1010 0 */
    { 5, 0x7, 10 },	/* 0011 1 */
    { 5, 0x8, 11 },	/* 0100 0 */
    { 6, 0x8, 12 },	/* 0010 00 */
    { 6, 0x3, 13 },	/* 0000 11 */
    { 6, 0x34, 14 },	/* 1101 00 */
    { 6, 0x35, 15 },	/* 1101 01 */
    { 6, 0x2A, 16 },	/* 1010 10 */
    { 6, 0x2B, 17 },	/* 1010 11 */
    { 7, 0x27, 18 },	/* 0100 111 */
    { 7, 0xC, 19 },	/* 0001 100 */
    { 7, 0x8, 20 },	/* 0001 000 */
    { 7, 0x17, 21 },	/* 0010 111 */
    { 7, 0x3, 22 },	/* 0000 011 */
    { 7, 0x4, 23 },	/* 0000 100 */
    { 7, 0x28, 24 },	/* 0101 000 */
    { 7, 0x2B, 25 },	/* 0101 011 */
    { 7, 0x13, 26 },	/* 0010 011 */
    { 7, 0x24, 27 },	/* 0100 100 */
    { 7, 0x18, 28 },	/* 0011 000 */
    { 8, 0x2, 29 },	/* 0000 0010 */
    { 8, 0x3, 30 },	/* 0000 0011 */
    { 8, 0x1A, 31 },	/* 0001 1010 */
    { 8, 0x1B, 32 },	/* 0001 1011 */
    { 8, 0x12, 33 },	/* 0001 0010 */
    { 8, 0x13, 34 },	/* 0001 0011 */
    { 8, 0x14, 35 },	/* 0001 0100 */
    { 8, 0x15, 36 },	/* 0001 0101 */
    { 8, 0x16, 37 },	/* 0001 0110 */
    { 8, 0x17, 38 },	/* 0001 0111 */
    { 8, 0x28, 39 },	/* 0010 1000 */
    { 8, 0x29, 40 },	/* 0010 1001 */
    { 8, 0x2A, 41 },	/* 0010 1010 */
    { 8, 0x2B, 42 },	/* 0010 1011 */
    { 8, 0x2C, 43 },	/* 0010 1100 */
    { 8, 0x2D, 44 },	/* 0010 1101 */
    { 8, 0x4, 45 },	/* 0000 0100 */
    { 8, 0x5, 46 },	/* 0000 0101 */
    { 8, 0xA, 47 },	/* 0000 1010 */
    { 8, 0xB, 48 },	/* 0000 1011 */
    { 8, 0x52, 49 },	/* 0101 0010 */
    { 8, 0x53, 50 },	/* 0101 0011 */
    { 8, 0x54, 51 },	/* 0101 0100 */
    { 8, 0x55, 52 },	/* 0101 0101 */
    { 8, 0x24, 53 },	/* 0010 0100 */
    { 8, 0x25, 54 },	/* 0010 0101 */
    { 8, 0x58, 55 },	/* 0101 1000 */
    { 8, 0x59, 56 },	/* 0101 1001 */
    { 8, 0x5A, 57 },	/* 0101 1010 */
    { 8, 0x5B, 58 },	/* 0101 1011 */
    { 8, 0x4A, 59 },	/* 0100 1010 */
    { 8, 0x4B, 60 },	/* 0100 1011 */
    { 8, 0x32, 61 },	/* 0011 0010 */
    { 8, 0x33, 62 },	/* 0011 0011 */
    { 8, 0x34, 63 },	/* 0011 0100 */
    { 5, 0x1B, 64 },	/* 1101 1 */
    { 5, 0x12, 128 },	/* 1001 0 */
    { 6, 0x17, 192 },	/* 0101 11 */
    { 7, 0x37, 256 },	/* 0110 111 */
    { 8, 0x36, 320 },	/* 0011 0110 */
    { 8, 0x37, 384 },	/* 0011 0111 */
    { 8, 0x64, 448 },	/* 0110 0100 */
    { 8, 0x65, 512 },	/* 0110 0101 */
    { 8, 0x68, 576 },	/* 0110 1000 */
    { 8, 0x67, 640 },	/* 0110 0111 */
    { 9, 0xCC, 704 },	/* 0110 0110 0 */
    { 9, 0xCD, 768 },	/* 0110 0110 1 */
    { 9, 0xD2, 832 },	/* 0110 1001 0 */
    { 9, 0xD3, 896 },	/* 0110 1001 1 */
    { 9, 0xD4, 960 },	/* 0110 1010 0 */
    { 9, 0xD5, 1024 },	/* 0110 1010 1 */
    { 9, 0xD6, 1088 },	/* 0110 1011 0 */
    { 9, 0xD7, 1152 },	/* 0110 1011 1 */
    { 9, 0xD8, 1216 },	/* 0110 1100 0 */
    { 9, 0xD9, 1280 },	/* 0110 1100 1 */
    { 9, 0xDA, 1344 },	/* 0110 1101 0 */
    { 9, 0xDB, 1408 },	/* 0110 1101 1 */
    { 9, 0x98, 1472 },	/* 0100 1100 0 */
    { 9, 0x99, 1536 },	/* 0100 1100 1 */
    { 9, 0x9A, 1600 },	/* 0100 1101 0 */
    { 6, 0x18, 1664 },	/* 0110 00 */
    { 9, 0x9B, 1728 },	/* 0100 1101 1 */
    { 11, 0x8, 1792 },	/* 0000 0001 000 */
    { 11, 0xC, 1856 },	/* 0000 0001 100 */
    { 11, 0xD, 1920 },	/* 0000 0001 101 */
    { 12, 0x12, 1984 },	/* 0000 0001 0010 */
    { 12, 0x13, 2048 },	/* 0000 0001 0011 */
    { 12, 0x14, 2112 },	/* 0000 0001 0100 */
    { 12, 0x15, 2176 },	/* 0000 0001 0101 */
    { 12, 0x16, 2240 },	/* 0000 0001 0110 */
    { 12, 0x17, 2304 },	/* 0000 0001 0111 */
    { 12, 0x1C, 2368 },	/* 0000 0001 1100 */
    { 12, 0x1D, 2432 },	/* 0000 0001 1101 */
    { 12, 0x1E, 2496 },	/* 0000 0001 1110 */
    { 12, 0x1F, 2560 },	/* 0000 0001 1111 */
    { 12, 0x1, G3CODE_EOL },	/* 0000 0000 0001 */
    { 9, 0x1, G3CODE_INVALID },	/* 0000 0000 1 */
    { 10, 0x1, G3CODE_INVALID },	/* 0000 0000 01 */
    { 11, 0x1, G3CODE_INVALID },	/* 0000 0000 001 */
    { 12, 0x0, G3CODE_INVALID },	/* 0000 0000 0000 */
};

tableentry TIFFFaxBlackCodes[] = {
    { 10, 0x37, 0 },	/* 0000 1101 11 */
    { 3, 0x2, 1 },	/* 010 */
    { 2, 0x3, 2 },	/* 11 */
    { 2, 0x2, 3 },	/* 10 */
    { 3, 0x3, 4 },	/* 011 */
    { 4, 0x3, 5 },	/* 0011 */
    { 4, 0x2, 6 },	/* 0010 */
    { 5, 0x3, 7 },	/* 0001 1 */
    { 6, 0x5, 8 },	/* 0001 01 */
    { 6, 0x4, 9 },	/* 0001 00 */
    { 7, 0x4, 10 },	/* 0000 100 */
    { 7, 0x5, 11 },	/* 0000 101 */
    { 7, 0x7, 12 },	/* 0000 111 */
    { 8, 0x4, 13 },	/* 0000 0100 */
    { 8, 0x7, 14 },	/* 0000 0111 */
    { 9, 0x18, 15 },	/* 0000 1100 0 */
    { 10, 0x17, 16 },	/* 0000 0101 11 */
    { 10, 0x18, 17 },	/* 0000 0110 00 */
    { 10, 0x8, 18 },	/* 0000 0010 00 */
    { 11, 0x67, 19 },	/* 0000 1100 111 */
    { 11, 0x68, 20 },	/* 0000 1101 000 */
    { 11, 0x6C, 21 },	/* 0000 1101 100 */
    { 11, 0x37, 22 },	/* 0000 0110 111 */
    { 11, 0x28, 23 },	/* 0000 0101 000 */
    { 11, 0x17, 24 },	/* 0000 0010 111 */
    { 11, 0x18, 25 },	/* 0000 0011 000 */
    { 12, 0xCA, 26 },	/* 0000 1100 1010 */
    { 12, 0xCB, 27 },	/* 0000 1100 1011 */
    { 12, 0xCC, 28 },	/* 0000 1100 1100 */
    { 12, 0xCD, 29 },	/* 0000 1100 1101 */
    { 12, 0x68, 30 },	/* 0000 0110 1000 */
    { 12, 0x69, 31 },	/* 0000 0110 1001 */
    { 12, 0x6A, 32 },	/* 0000 0110 1010 */
    { 12, 0x6B, 33 },	/* 0000 0110 1011 */
    { 12, 0xD2, 34 },	/* 0000 1101 0010 */
    { 12, 0xD3, 35 },	/* 0000 1101 0011 */
    { 12, 0xD4, 36 },	/* 0000 1101 0100 */
    { 12, 0xD5, 37 },	/* 0000 1101 0101 */
    { 12, 0xD6, 38 },	/* 0000 1101 0110 */
    { 12, 0xD7, 39 },	/* 0000 1101 0111 */
    { 12, 0x6C, 40 },	/* 0000 0110 1100 */
    { 12, 0x6D, 41 },	/* 0000 0110 1101 */
    { 12, 0xDA, 42 },	/* 0000 1101 1010 */
    { 12, 0xDB, 43 },	/* 0000 1101 1011 */
    { 12, 0x54, 44 },	/* 0000 0101 0100 */
    { 12, 0x55, 45 },	/* 0000 0101 0101 */
    { 12, 0x56, 46 },	/* 0000 0101 0110 */
    { 12, 0x57, 47 },	/* 0000 0101 0111 */
    { 12, 0x64, 48 },	/* 0000 0110 0100 */
    { 12, 0x65, 49 },	/* 0000 0110 0101 */
    { 12, 0x52, 50 },	/* 0000 0101 0010 */
    { 12, 0x53, 51 },	/* 0000 0101 0011 */
    { 12, 0x24, 52 },	/* 0000 0010 0100 */
    { 12, 0x37, 53 },	/* 0000 0011 0111 */
    { 12, 0x38, 54 },	/* 0000 0011 1000 */
    { 12, 0x27, 55 },	/* 0000 0010 0111 */
    { 12, 0x28, 56 },	/* 0000 0010 1000 */
    { 12, 0x58, 57 },	/* 0000 0101 1000 */
    { 12, 0x59, 58 },	/* 0000 0101 1001 */
    { 12, 0x2B, 59 },	/* 0000 0010 1011 */
    { 12, 0x2C, 60 },	/* 0000 0010 1100 */
    { 12, 0x5A, 61 },	/* 0000 0101 1010 */
    { 12, 0x66, 62 },	/* 0000 0110 0110 */
    { 12, 0x67, 63 },	/* 0000 0110 0111 */
    { 10, 0xF, 64 },	/* 0000 0011 11 */
    { 12, 0xC8, 128 },	/* 0000 1100 1000 */
    { 12, 0xC9, 192 },	/* 0000 1100 1001 */
    { 12, 0x5B, 256 },	/* 0000 0101 1011 */
    { 12, 0x33, 320 },	/* 0000 0011 0011 */
    { 12, 0x34, 384 },	/* 0000 0011 0100 */
    { 12, 0x35, 448 },	/* 0000 0011 0101 */
    { 13, 0x6C, 512 },	/* 0000 0011 0110 0 */
    { 13, 0x6D, 576 },	/* 0000 0011 0110 1 */
    { 13, 0x4A, 640 },	/* 0000 0010 0101 0 */
    { 13, 0x4B, 704 },	/* 0000 0010 0101 1 */
    { 13, 0x4C, 768 },	/* 0000 0010 0110 0 */
    { 13, 0x4D, 832 },	/* 0000 0010 0110 1 */
    { 13, 0x72, 896 },	/* 0000 0011 1001 0 */
    { 13, 0x73, 960 },	/* 0000 0011 1001 1 */
    { 13, 0x74, 1024 },	/* 0000 0011 1010 0 */
    { 13, 0x75, 1088 },	/* 0000 0011 1010 1 */
    { 13, 0x76, 1152 },	/* 0000 0011 1011 0 */
    { 13, 0x77, 1216 },	/* 0000 0011 1011 1 */
    { 13, 0x52, 1280 },	/* 0000 0010 1001 0 */
    { 13, 0x53, 1344 },	/* 0000 0010 1001 1 */
    { 13, 0x54, 1408 },	/* 0000 0010 1010 0 */
    { 13, 0x55, 1472 },	/* 0000 0010 1010 1 */
    { 13, 0x5A, 1536 },	/* 0000 0010 1101 0 */
    { 13, 0x5B, 1600 },	/* 0000 0010 1101 1 */
    { 13, 0x64, 1664 },	/* 0000 0011 0010 0 */
    { 13, 0x65, 1728 },	/* 0000 0011 0010 1 */
    { 11, 0x8, 1792 },	/* 0000 0001 000 */
    { 11, 0xC, 1856 },	/* 0000 0001 100 */
    { 11, 0xD, 1920 },	/* 0000 0001 101 */
    { 12, 0x12, 1984 },	/* 0000 0001 0010 */
    { 12, 0x13, 2048 },	/* 0000 0001 0011 */
    { 12, 0x14, 2112 },	/* 0000 0001 0100 */
    { 12, 0x15, 2176 },	/* 0000 0001 0101 */
    { 12, 0x16, 2240 },	/* 0000 0001 0110 */
    { 12, 0x17, 2304 },	/* 0000 0001 0111 */
    { 12, 0x1C, 2368 },	/* 0000 0001 1100 */
    { 12, 0x1D, 2432 },	/* 0000 0001 1101 */
    { 12, 0x1E, 2496 },	/* 0000 0001 1110 */
    { 12, 0x1F, 2560 },	/* 0000 0001 1111 */
    { 12, 0x1, G3CODE_EOL },	/* 0000 0000 0001 */
    { 9, 0x1, G3CODE_INVALID },	/* 0000 0000 1 */
    { 10, 0x1, G3CODE_INVALID },	/* 0000 0000 01 */
    { 11, 0x1, G3CODE_INVALID },	/* 0000 0000 001 */
    { 12, 0x0, G3CODE_INVALID },	/* 0000 0000 0000 */
};

unsigned char NoBitRevTable[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 
};


/*
 * -------------------------------------------------------------------------
 *              FAX G42D encoding for 1 bit images
 * -------------------------------------------------------------------------
 */

static short sp_data, sp_bit;

static tableentry horizcode =
    { 3, 0x1 };		/* 001 */

static tableentry passcode =
    { 4, 0x1 };		/* 0001 */

static tableentry vcodes[7] = {
    { 7, 0x03 },	/* 0000 011 */
    { 6, 0x03 },	/* 0000 11 */
    { 3, 0x03 },	/* 011 */
    { 1, 0x1 },		/* 1 */
    { 3, 0x2 },		/* 010 */
    { 6, 0x02 },	/* 0000 10 */
    { 7, 0x02 }		/* 0000 010 */
};

typedef struct {
    unsigned char *bufStart;
    unsigned char *bufPtr;
    int bufSize;
    int bytesLeft;
} Buffer;



/*
 * Flush bits to output buffer.
 */

static int
flushbits (outbuf)

Buffer *outbuf;

{
    if (outbuf->bytesLeft > 0)
    {
	*(outbuf->bufPtr++) = NoBitRevTable[sp_data];
	outbuf->bytesLeft--;

	sp_data = 0;
	sp_bit = 8;

	return (1);
    }
    else
	return (0);
}


/*
 * Write a variable-length bit-value to the output stream.  Values are
 * assumed to be at most 16 bits.
 */

static int
putbits (bits, length, outbuf)

unsigned int bits;
unsigned int length;
Buffer *outbuf;

{
    static int mask[9] =
        {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

    while (length > sp_bit)
    {
	sp_data |= bits >> (length - sp_bit);
	length -= sp_bit;
	if (!flushbits (outbuf))
	    return (0);
    }

    sp_data |= (bits & mask[length]) << (sp_bit - length);
    sp_bit -= length;

    if (sp_bit == 0)
    {
	if (!flushbits (outbuf))
	    return (0);
    }

    return (1);
}


/*
 * Write a code to the output stream.
 */

static int
putcode (te, outbuf)

tableentry *te;
Buffer *outbuf;

{
    return (putbits (te->code, te->length, outbuf));
}


/*
 * Write the sequence of codes that describes the specified span of
 * zero's or one's.  The appropriate table that holds the make-up and
 * terminating codes is supplied.
 */

static int
putspan (span, tab, outbuf)

int span;
tableentry *tab;
Buffer *outbuf;

{
    while (span >= 2624)
    {
	tableentry *te = &tab[63 + (2560 >> 6)];
	if (!putcode (te, outbuf))
	    return (0);
	span -= te->runlen;
    }

    if (span >= 64)
    {
	tableentry *te = &tab[63 + (span >> 6)];
	if (!putcode (te, outbuf))
	    return (0);
	span -= te->runlen;
    }

    if (!putcode (&tab[span], outbuf))
	return (0);

    return (1);
}



#define	PIXEL(buf,ix)	((((buf)[(ix)>>3]) >> (7-((ix)&7))) & 1)

static int
EncodeFaxG42D (inbuf, refline, bits, outbuf)

unsigned char *inbuf;
unsigned char *refline;
int bits;
Buffer *outbuf;

{
    short white = 1;
    int a0 = 0;
    int a1 = (PIXEL (inbuf, 0) != white ?
	0 : LbxImageFindDiff (inbuf, 0, bits, white));
    int b1 = (PIXEL (refline, 0) != white ?
	0 : LbxImageFindDiff (refline, 0, bits, white));
    int a2, b2;

    for (;;)
    {
	b2 = LbxImageFindDiff (refline, b1, bits, PIXEL (refline, b1));
	if (b2 >= a1)
	{
	    int d = b1 - a1;
	    if (!(-3 <= d && d <= 3))
	    {
		/* horizontal mode */

		a2 = LbxImageFindDiff (inbuf, a1, bits, PIXEL (inbuf, a1));
		if (!putcode (&horizcode, outbuf))
		    return (0);

		if (a0 + a1 == 0 || PIXEL (inbuf, a0) == white)
		{
		    if (!putspan(a1 - a0, TIFFFaxWhiteCodes, outbuf))
			return (0);
		    if (!putspan(a2 - a1, TIFFFaxBlackCodes, outbuf))
			return (0);
		}
		else
		{
		    if (!putspan (a1 - a0, TIFFFaxBlackCodes, outbuf))
			return (0);
		    if (!putspan (a2 - a1, TIFFFaxWhiteCodes, outbuf))
			return (0);
		}

		a0 = a2;
	    }
	    else
	    {
		/* vertical mode */

		if (!putcode (&vcodes[d+3], outbuf))
		    return (0);
		a0 = a1;
	    }
	}
	else
	{
	    /* pass mode */

	    if (!putcode (&passcode, outbuf))
		return (0);
	    a0 = b2;
	}

	if (a0 >= bits)
	    break;

	a1 = LbxImageFindDiff (inbuf, a0, bits, PIXEL (inbuf, a0));
	b1 = LbxImageFindDiff (refline, a0, bits, !PIXEL (inbuf, a0));
	b1 = LbxImageFindDiff (refline, b1, bits, PIXEL (inbuf, a0));
    }

    return (1);
}


int
LbxImageEncodeFaxG42D (inbuf, outbuf, outbufSize,
    image_bytes, pixels_per_line, bytesCompressed)

unsigned char *inbuf;
unsigned char *outbuf;
int outbufSize;
int image_bytes;
int pixels_per_line;
int *bytesCompressed;

{
    int bytes_per_scanline = (int) (0.5 + (float) pixels_per_line / 8.0);
    int padded_bytes_per_scanline = (bytes_per_scanline % 4) ?
	bytes_per_scanline + (4 - bytes_per_scanline % 4) : bytes_per_scanline;
    unsigned char *refline, *refptr;
    Buffer OutBuf;
    int status, i;

    OutBuf.bufStart = OutBuf.bufPtr = outbuf;
    OutBuf.bufSize = OutBuf.bytesLeft = outbufSize;

    if (!(refline = (unsigned char *) malloc (bytes_per_scanline + 1)))
	return (LBX_IMAGE_COMPRESS_BAD_MALLOC);

    refptr = refline + 1;

    for (i = 0; i < bytes_per_scanline + 1; i++)
	refline[i] = 0xff;

    sp_bit = 8;
    sp_data = 0;

    while (image_bytes > 0)
    {
	if (!(status = EncodeFaxG42D (inbuf, refptr,
	    pixels_per_line, &OutBuf)))
	{
	    goto bad;
	}

	memcpy (refptr, inbuf, bytes_per_scanline);

	inbuf += padded_bytes_per_scanline;
	image_bytes -= padded_bytes_per_scanline;
    }

    status = putbits (EOL, 12, &OutBuf);
    if (status)
	status = putbits (EOL, 12, &OutBuf);
    if (status && sp_bit != 8)
    {
	status = flushbits (&OutBuf);
    }

 bad:

    free ((char *) refline);

    if (status)
    {
	*bytesCompressed = OutBuf.bufPtr - OutBuf.bufStart;
	return (LBX_IMAGE_COMPRESS_SUCCESS);
    }
    else
	return (LBX_IMAGE_COMPRESS_NOT_WORTH_IT);
}
