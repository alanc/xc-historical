main ()
{
    f ();
}

f ()
{
    g ();
}


g ()
{
    unsigned long   returnStack[16];
    int		    i;

    getStackTrace (returnStack, 16);
    for (i = 0; i < 16 && returnStack[i]; i++)
	printf ("%2d: 0x%x\n", i, returnStack[i]);
}
