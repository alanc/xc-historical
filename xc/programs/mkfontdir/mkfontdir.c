/***********************************************************
Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $XConsortium: mkfontdir.c,v 1.6 91/02/10 18:22:18 rws Exp $ */

#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <stdio.h>
#include <sys/param.h>
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

#include <X11/X.h>
#include <X11/Xproto.h
#include "fontmisc.h"
#include "fontstruct.h"
#include "fontdir.h"

#define  XK_LATIN1
#include <X11/keysymdef.h>

static void 
WriteFontDir(dirName, dir)
    char	    *dirName;
    FontFileDirPtr  dir;
{
    int		    i;
    FILE	    *file;
    char	    full_name[MAXPATHLEN];
    FontFileNamePtr fname;

    sprintf (full_name, "%s/%s", dirName, FontDirFile);
    file = fopen (full_name, "w");
    if (!file)
    {
	fprintf (stderr, "mkfontdir: can't create directory %s\n", full_name);
	exit (1);
    }
    fprintf(file, "%d\n", dir->used);
    for (i = 0; i < dir->used; i++) {
	fname = &dir->names[i];
	fprintf (file, "%s %s\n", fname->file, fname->name);
    }
    fclose (file);
}

static char *
NameForAtomOrNone (a)
    Atom    a;
{
    char    *name;

    name = NameForAtom (a);
    if (!name)
	return "";
    return name;
}

static Bool
GetFontName(file_name, font_name)
    char    *file_name;
    char    *font_name;
{
    FontInfoRec	info;
    int		i;
    char	*atom_name;
    char	*atom_value;

    if (FontInfoLoad (&info, file_name) != Success)
	return FALSE;

    for (i = 0; i < info.nprops; i++) 
    {
	atom_name = (char *) NameForAtomOrNone (info.props[i].name);
	if (atom_name && strcmp (atom_name, "FONT") == 0 && info.isStringProp[i])
	{
	    atom_value = NameForAtomOrNone (info.props[i].value);
	    if (strlen (atom_value) == 0)
		return FALSE;
	    strcpy (font_name, atom_value);
	    return TRUE;
	}
    }
    return FALSE;
}

static Bool
FontNameExists (dir, font_name)
    FontFileDirPtr  dir;
    char	    *font_name;
{
    int	    i;

    for (i = 0; i < dir->used; i++)
	if (!strcmp (dir->names[i].name, font_name))
	    return TRUE;
    return FALSE;
}

static Bool
ProcessFile (dirName, fileName, dir)
    char		*dirName;
    char		*fileName;
    FontFileDirPtr	dir;
{
    char	font_name[MAXPATHLEN];
    char	ps_font_name[MAXPATHLEN];
    char	full_name[MAXPATHLEN];

    strcpy (full_name, dirName);
    if (dirName[strlen(dirName) - 1] != '/')
	strcat (full_name, "/");
    strcat (full_name, fileName);

    if (!GetFontName (full_name, font_name))
	return FALSE;

    CopyISOLatin1Lowered (font_name, font_name, strlen(font_name));

    if (FontNameExists (dir, font_name))
    {
	fprintf (stderr, "Duplicate font names %s\n", font_name);
	return FALSE;
    }
    FontFileAddDir (dir, font_name, fileName, FALSE);
    return TRUE;
}

static Bool
Ematch(ext, name)
    char	*ext;
    char	*name;
{
    int	i,j;

    i = strlen(name);
    j = strlen(ext);
    return ((i > j) && (strcmp(ext, name + i - j) == 0));
}

Estrip(ext,name)
    char	*ext;
    char	*name;
{
    name[strlen(name) - strlen(ext)] = '\0';
}

/***====================================================================***/

typedef struct _nameBucket {
    struct _nameBucket	*next;
    char		*name;
    int			extension;
} NameBucketRec, *NameBucketPtr;
    
#define New(type,count)	((type *) malloc (count * sizeof (type)))

#define HASH_SIZE   1024

char *
MakeName(name)
    char	*name;
{
    char    *new;

    new = New(char, strlen(name) + 1);
    strcpy (new,name);
    return new;
}

int
Hash(name)
    char	*name;
{
    int	    i;
    char    c;

    i = 0;
    while (c = *name++)
	i = i << 1 ^ c;
    return i & (HASH_SIZE - 1);
}

static void 
DoDirectory(dirName)
    char	*dirName;
{
    FontFileDirPtr	dir;
    char		fileName[MAXPATHLEN];
    int			i;
    char		**names;
    char		*extension, *FontFileExtension();
    int			nnames;
    int			namesize;
    DIR			*dirp;
    struct dirent	*file;
    NameBucketPtr	*hashTable, bucket, *prev, next;
    int			hash;
    int			type;

    hashTable = New (NameBucketPtr, HASH_SIZE);
    bzero((char *)hashTable, HASH_SIZE * sizeof(NameBucketPtr));

    dir = FontFileMakeDir (dirName, 100);
    namesize = 0;
    names = 0;
    nnames = 0;
    if ((dirp = opendir (dirName)) == NULL)
	return;
    while ((file = readdir (dirp)) != NULL) {
	type = FontFileType (file->d_name);
	if (type != -1)
	{
	    extension = FontFileExtension (type);
	    Estrip (extension, file->d_name);
	    hash = Hash (file->d_name);
	    prev = &hashTable[hash];
	    bucket = *prev;
	    while (bucket && strcmp (bucket->name, file->d_name))
	    {
		prev = &bucket->next;
		bucket = *prev;
	    }
	    if (bucket)
	    {
		if (bucket->extension > type)
		    bucket->extension = type;
	    }
	    else
	    {
		bucket = New (NameBucketRec, 1);
		bucket->name = MakeName (file->d_name);
		bucket->next = 0;
		bucket->extension = type;
		*prev = bucket;
	    }
	}
    }
    for (hash = 0; hash < HASH_SIZE; hash++)
    {
	for (bucket = hashTable[hash]; bucket; bucket = next)
	{
	    next = bucket->next;
	    strcpy (fileName, bucket->name);
	    strcat (fileName, FontFileExtension(bucket->extension));
	    ProcessFile (dirName, fileName, dir);
	    free (bucket->name);
	    free (bucket);
	}
    }
    free (hashTable);
    if (dir->used > 0)
	(void)WriteFontDir(dirName, dir);
    FontFileFreeDir (dir);
}

/***====================================================================***/

main (argc, argv)
    int argc;
    char **argv;
{
    int i;

    if (argc == 1)
	DoDirectory(".");
    else
	for (i = 1; i < argc; i++) {
	    DoDirectory(argv[i]);
 	}
    exit(0);	
}
