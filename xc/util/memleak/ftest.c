static char    *foo, *bar, *bletch;
static char    *glorf[100];

extern char *malloc ();

main ()
{
    int	    i;

    foo = malloc (1000);
    bar = malloc (2000);
    bletch = malloc (3000);
    for (i = 0; i < 100; i++)
	glorf[i] = malloc (i * 200);
    for (i = 0; i < 100; i++) {
	free (glorf[i]);
	glorf[i] = 0;
    }
    free (foo);
    free (bletch);
    bletch = 0;
    *foo = 'a';
    bar = 0;
    CheckMemory ();
}
