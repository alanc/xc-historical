/* $XConsortium: XrmI.h,v 1.5 89/12/22 16:11:28 rws Exp $ */
/*

Copyright 1990 by the Massachusetts Institute of Technology

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#define NUM_CHARS 256

#define _EOF   	((unsigned short) (1 << 0))
#define SPACE 	((unsigned short) (1 << 1))
#define SEP 	((unsigned short) (1 << 2))
#define TIGHT 	((unsigned short) (1 << 3))
#define LOOSE 	((unsigned short) (1 << 4))
#define EOL	((unsigned short) (1 << 5))
#define COMMENT	((unsigned short) (1 << 6))
#define ODIGIT	((unsigned short) (1 << 7))
#define BSLASH  ((unsigned short) (1 << 8))
#define N	((unsigned short) (1 << 9))

#define A_NEW_LINE 	('\n')
#define BACKSLASH 	('\\')
#define STRING_TERMINATOR ('\0')
#define ZERO		('0')

#define get_next_char(ch, str)		(_xrmtypes[(ch) = *((str)++)])

#define xrm_is_space(bits) 		((bits) & SPACE)
#define xrm_is_end_of_quark(bits)     ((bits)&(LOOSE|TIGHT|SEP|SPACE|_EOF|EOL))
#define xrm_is_EOL(bits) 		((bits) & (EOL|_EOF))
#define xrm_is_EOF(bits) 		((bits) & _EOF)
#define xrm_is_tight(bits)		((bits) & TIGHT)
#define xrm_is_loose(bits)		((bits) & LOOSE)
#define xrm_is_tight_or_loose(bits)     ((bits) & (TIGHT|LOOSE))
#define xrm_is_comment(bits)		((bits) & COMMENT)
#define xrm_is_odigit(bits)		((bits) & ODIGIT)
#define xrm_is_separator(bits) 		((bits) & (SEP|SPACE|_EOF|EOL))
#define xrm_is_real_separator(bits) 	((bits) & SEP)
#define xrm_is_end_of_value(bits) 	((bits) & (EOL|_EOF))
#define xrm_is_n(bits)			((bits) & N)
#define xrm_is_backslash(bits)		((bits) & BSLASH)
#define xrm_is_backslash_or_EOV(bits)   ((bits) & (EOL|_EOF|BSLASH))

/*
 * Macros to abstract out reading the file, and getting its size.
 *
 * You may need to redefine these for various other operating systems. 
 */

#include        <sys/stat.h>                        
#include 	<fcntl.h>

#define OpenFile(name) 		open((name), O_RDONLY)
#define CloseFile(fd)           close((fd))
#define ReadFile(fd, buf, size) read((fd), (buf), (size))
#define GetSizeOfFile(name, size)                   \
{                                                   \
    struct stat status_buffer;                      \
    if ( (stat((name), &status_buffer)) == -1 )     \
	size = -1;                                  \
    else                                            \
	size = status_buffer.st_size;               \
}

