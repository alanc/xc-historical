/* $XConsortium$ */
#ifdef TEST

#define xalloc(s)   malloc(s)
#define xfree(s)    free(s)

#endif

#define BUFFILESIZE	8192
#define BUFFILEEOF	-1

typedef unsigned char BufChar;

typedef struct _buffile {
    BufChar *bufp;
    int	    left;
    BufChar buffer[BUFFILESIZE];
    int	    (*io)(/* BufFilePtr f */);
    int	    (*skip)(/* BufFilePtr f, int count */);
    int	    (*close)(/* BufFilePtr f */);
    char    *private;
} BufFileRec, *BufFilePtr;

extern BufFilePtr   BufFileCreate ();
extern BufFilePtr   BufFileOpenRead (), BufFileOpenWrite ();
extern BufFilePtr   BufFilePushCompressed ();
extern int	    BufFileClose ();
extern int	    BufFileFlush ();
#define BufFileGet(f)	((f)->left-- ? *(f)->bufp++ : (*(f)->io) (f))
#define BufFilePut(c,f)	(--(f)->left ? *(f)->bufp++ = (c) : (*(f)->io) (c,f))
#define BufFileSkip(f,c)    ((*(f)->skip) (f, c))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
