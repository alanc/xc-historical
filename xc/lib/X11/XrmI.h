
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

static unsigned short _xrmtypes[NUM_CHARS] = {
    _EOF,0,0,0,0,0,0,0,0,SPACE,					/*   0. */
    EOL,0,0,0,0,0,0,0,0,0,					/*  10. */
    0,0,0,0,0,0,0,0,0,0,					/*  20. */
    0,0,SPACE,COMMENT,0,0,0,0,0,0,				/*  30. */
    0,0,LOOSE,0,0,0,TIGHT,0,ODIGIT,ODIGIT,
    ODIGIT,ODIGIT,ODIGIT,ODIGIT,ODIGIT,  			/*  50. */
    ODIGIT,0,0,SEP,0,						/*  55. */
    0,0,0,0,0,0,0,0,0,0,					/*  60. */
    0,0,0,0,0,0,0,0,0,0,					/*  70. */
    0,0,0,0,0,0,0,0,0,0,					/*  80. */
    0,0,BSLASH,0,0,0,0,0,0,0,				        /*  90. */
    0,0,0,0,0,0,0,0,0,0,					/*  100. */
    N,0,0,0,0,0,0,0,0,0,					/*  110. */
    /* The rest will be automatically initialized to zero. */
};

#define get_next_char(ch, str)		(_xrmtypes[(ch) = *((str)++)])

#define xrm_is_space(bits) 		((bits) & SPACE)
#define xrm_is_end_of_quark(bits)     ((bits)&(LOOSE|TIGHT|SEP|SPACE|_EOF|EOL))
#define xrm_is_EOL(bits) 		((bits) & (EOL|_EOF))
#define xrm_is_EOF(bits) 		((bits) & _EOF)
#define xrm_is_tight(bits)		((bits) & TIGHT)
#define xrm_is_loose(bits)		((bits) & LOOSE)
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

