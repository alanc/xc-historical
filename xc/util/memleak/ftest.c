#include    "fmalloc.h"

static char    *foo, *bar, *bletch;
static char    *glorf[100];

main ()
{
    int	    i;

    foo = fmalloc (1000);
    bar = fmalloc (2000);
    bletch = fmalloc (3000);
    for (i = 0; i < 100; i++)
	glorf[i] = fmalloc (i * 200);
    for (i = 0; i < 100; i++) {
	ffree (glorf[i]);
	glorf[i] = 0;
    }
    ffree (foo);
    ffree (bletch);
    bletch = 0;
    *foo = 'a';
    bar = 0;
    CheckMemory ();
}
