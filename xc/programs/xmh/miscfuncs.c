/* $XConsortium: miscfuncs.c,v 1.3 91/02/08 10:48:11 rws Exp $ */

#include <X11/Xos.h>
#ifdef SYSV
#include <dirent.h>
#else
#ifdef SVR4
#include <dirent.h>
#else
#ifdef USG
#include <dirent.h>
#else
#ifdef _POSIX_SOURCE
#include <dirent.h>
#else
#include <sys/dir.h>
#ifndef dirent
#define dirent direct
#endif
#endif
#endif
#endif
#endif
char *malloc();
char *realloc();

/* A convenient shorthand. */
typedef struct dirent	 ENTRY;

/*
**  This code is by Rich Salz (rsalz@bbn.com), and ported to SVR4
**  by David Elliott (dce@smsc.sony.com).  No copyrights were found
**  in the original.  Subsequently modified by Bob Scheifler.
*/

/* Initial guess at directory size. */
#define INITIAL_SIZE	20

static int StrCmp(a, b)
    char **a, **b;
{
    return strcmp(*a, *b);
}

int
ScanDir(Name, List, Selector)
    char		  *Name;
    char		***List;
    int			 (*Selector)();
{
    register char	 **names;
    register ENTRY	  *E;
    register DIR	  *Dp;
    register int	   i;
    register int	   size;

    /* Get initial list space and open directory. */
    size = INITIAL_SIZE;
    if (!(names = (char **)malloc(size * sizeof(char *))) ||
	!(Dp = opendir(Name)))
	return(-1);

    /* Read entries in the directory. */
    for (i = 0; E = readdir(Dp); )
	if (!Selector || (*Selector)(E->d_name)) {
	    /* User wants them all, or he wants this one. */
	    if (++i >= size) {
		size <<= 1;
		names = (char**)realloc((char *)names, size * sizeof(char*));
		if (!names) {
		    closedir(Dp);
		    return(-1);
		}
	    }

	    /* Copy the entry. */
	    if (!(names[i - 1] = (char *)malloc(strlen(E->d_name) + 1))) {
		closedir(Dp);
		return(-1);
	    }
	    (void)strcpy(names[i - 1], E->d_name);
	}

    /* Close things off. */
    names[i] = (char *)0;
    *List = names;
    closedir(Dp);

    /* Sort? */
    if (i)
	qsort((char *)names, i, sizeof(char *), StrCmp);

    return(i);
}
