/* $XConsortium: miscfuncs.c,v 1.0 91/01/06 21:08:48 rws Exp $ */

/*
**  ALPHASORT
**  Trivial sorting predicate for scandir; puts entries in alphabetical order.
**
**  This code is by Rich Salz (rsalz@bbn.com), and ported to SVR4
**  by David Elliott (dce@smsc.sony.com).  No copyrights were found
**  in the original.
*/
#include <X11/Xos.h>
#include <dirent.h>
#include <stdlib.h>

/* A convenient shorthand. */
typedef struct dirent	 ENTRY;

int
alphasort(d1, d2)
    ENTRY	**d1;
    ENTRY	**d2;
{
    return(strcmp(d1[0]->d_name, d2[0]->d_name));
}

/*
**  SCANDIR
**  Scan a directory, collecting all (selected) items into a an array.
**
**  This code is by Rich Salz (rsalz@bbn.com), and ported to SVR4
**  by David Elliott (dce@smsc.sony.com).  No copyrights were found
**  in the original.
*/

#ifndef NULL
#define NULL 0
#endif

#ifndef DIRSIZ
#define DIRSIZ(dp) (((dp)->d_reclen + 1 + 3) & ~3)
#endif

/* Initial guess at directory size. */
#define INITIAL_SIZE	20

int
scandir(Name, List, Selector, Sorter)
    char		  *Name;
    ENTRY		***List;
    int			 (*Selector)();
    int			 (*Sorter)();
{
    register ENTRY	 **names;
    register ENTRY	  *E;
    register DIR	  *Dp;
    register int	   i;
    register int	   size;

    /* Get initial list space and open directory. */
    size = INITIAL_SIZE;
    if ((names = (ENTRY **)malloc(size * sizeof names[0])) == NULL
     || (Dp = opendir(Name)) == NULL)
	return(-1);

    /* Read entries in the directory. */
    for (i = 0; E = readdir(Dp); )
	if (Selector == NULL || (*Selector)(E)) {
	    /* User wants them all, or he wants this one. */
	    if (++i >= size) {
		size <<= 1;
		names = (ENTRY **)realloc((char *)names, size * sizeof names[0]);
		if (names == NULL) {
		    closedir(Dp);
		    return(-1);
		}
	    }

	    /* Copy the entry. */
	    if ((names[i - 1] = (ENTRY *)malloc(DIRSIZ(E))) == NULL) {
		closedir(Dp);
		return(-1);
	    }
	    names[i - 1]->d_ino = E->d_ino;
	    names[i - 1]->d_reclen = E->d_reclen;
	    (void)strcpy(names[i - 1]->d_name, E->d_name);
	}

    /* Close things off. */
    names[i] = NULL;
    *List = names;
    closedir(Dp);

    /* Sort? */
    if (i && Sorter)
	qsort((char *)names, i, sizeof names[0], Sorter);

    return(i);
}
