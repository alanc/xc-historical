#include <X/mit-copyright.h>

/* Copyright 1985, 1986, Massachusetts Institute of Technology */

/* $Header: XWDFile.h,v 1.1 87/05/18 09:41:58 dkk Locked $ */
/*
 * XWDFile.h	MIT Project Athena, X Window system window raster
 *		image dumper, dump file format header file.
 *
 *  Author:	Tony Della Fera, DEC
 *		27-Jun-85
 * 
 * Modifier:    William F. Wyatt, SAO
 *              18-Nov-86  - version 6 for saving/restoring color maps
 */

#define XWD_FILE_VERSION 7

typedef struct _xwd_file_header {
	int header_size;	/* Size of the entire file header (bytes). */
	int file_version;	/* XWD_FILE_VERSION */
	int display_type;	/* Display type. */
	int display_planes;	/* Number of display planes. */
	int pixmap_format;	/* Pixmap format. */
	int pixmap_width;	/* Pixmap width. */
	int pixmap_height;	/* Pixmap height. */
	short window_width;	/* Window width. */
	short window_height;	/* Window height. */
	short window_x;		/* Window upper left X coordinate. */
	short window_y;		/* Window upper left Y coordinate. */
	short window_bdrwidth;	/* Window border width. */
	short window_ncolors;   /* number of Color entries in this window */
} XWDFileHeader;

