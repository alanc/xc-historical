/*
 * $XConsortium: AsciiSrc.c,v 1.1 89/06/21 17:24:30 kit Exp $
 */

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

/*
 * Private data.
 */

  Boolean	is_tempfile;	  /* Is this a temporary file? */
  Boolean       changes;	  /* Has this file been edited? */
  Boolean       allocated_string; /* Have I allocated the
				     string in data->string? */
  FILE *file;		
  
  XawTextPosition length; 	/* length of file */
  Piece * first_piece;		/* first piece of the text. */

} AsciiSourceData, *AsciiSourcePtr;

