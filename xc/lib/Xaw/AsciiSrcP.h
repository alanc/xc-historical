/*
 * $XConsortium: AsciiSrcP.h,v 1.2 89/07/06 16:00:56 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

/***********************************************************************
 *
 * Ascii Source
 *
 ***********************************************************************/

/*
 * AsciiSrcP.h - Private Header for Ascii Text Source.
 *
 * This is the private header file for the Ascii Text Source.
 * It is intended to be used with the Text widget, the simplest way to use
 * this text source is to use the AsciiText Widget.
 *
 * Date:    June 29, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#ifndef _XawAsciiSrcP_h
#define _XawAsciiSrcP_h

#include <X11/Xaw/TextP.h>
#include <X11/Xaw/AsciiSrc.h>

#define TMPSIZ 32		/* bytes to allocate for tmpnam */
#define DEFAULT_PIECE_SIZE BUFSIZ

#ifdef ASCII_STRING
#  define MAGIC_VALUE -1		/* ascii_length magic value. */
#endif

#define streq(a, b)        ( strcmp((a), (b)) == 0 )

typedef struct _Piece {		/* Piece of the text file of BUFSIZ allocated 
				   characters. */
  char * text;			/* The text in this buffer. */
  XawTextPosition used;		/* The number of characters of this buffer 
				   that have been used. */
  struct _Piece *prev, *next;	/* linked list pointers. */
} Piece;

typedef struct _AsciiSourceData {

/*
 * Resources.
 */

  char       *string;		/* either the string, or the
				   file name, depending upon the type. */
  XawAsciiType type;		/* either string or disk. */
  long piece_size;		/* Size of text buffer for each piece. */
  Boolean data_compression;	/* compress to minimum memory automatically
				   on save? */
  XtCallbackList callback;	/* A callback list to call when the source is
				   changed. */
#ifdef ASCII_STRING
  Boolean ascii_string;		/* Emulate the R3 ascii string widget? */
  int     ascii_length;		/* length field for ascii string emulation. */
#endif /* ASCII_STRING */

#ifdef ASCII_DISK
  String filename;		/* name of file for Compatability. */
#endif /* ASCII_DISK */

/*
 * Private data.
 */

  Boolean	is_tempfile;	  /* Is this a temporary file? */
  Boolean       changes;	  /* Has this file been edited? */
  Boolean       allocated_string; /* Have I allocated the
				     string in data->string? */
  XawTextPosition length; 	/* length of file */
  Piece * first_piece;		/* first piece of the text. */

} AsciiSourceData, *AsciiSourcePtr;

#endif /* _XawAsciiSrcP_h  - Don't add anything after this line. */
