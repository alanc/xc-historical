#define XK_LATIN1
#include    <X11/keysymdef.h>
/* #include    <X11/Xmu/CharSet.h> */

/* make sure everything initializes themselves at least once */

long serverGeneration = 1;

unsigned long *
Xalloc (m)
{
    return (unsigned long *) malloc (m);
}

unsigned long *
Xrealloc (n,m)
    unsigned long   *n;
{
    if (!n)
	return (unsigned long *) malloc (m);
    else
	return (unsigned long *) realloc ((char *) n, m);
}

Xfree (n)
    unsigned long   *n;
{
    if (n)
	free ((char *) n);
}

CopyISOLatin1Lowered (dst, src, len)
    char    *dst, *src;
    int	    len;
{
    register unsigned char *dest, *source;

    for (dest = (unsigned char *)dst, source = (unsigned char *)src;
	 *source && len > 0;
	 source++, dest++, len--)
    {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}

register_fpe_functions ()
{
}
