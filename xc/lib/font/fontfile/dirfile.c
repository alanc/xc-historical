#endif /* lint */
 * $XConsortium: dirfile.c,v 1.3 91/07/16 20:12:42 keith Exp $
 *
/*
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * dirfile.c
 *
 * Read fonts.dir and fonts.alias files
 */
#include    "fontfilest.h"
#include    <stdio.h>
#include    <sys/types.h>
#include    <errno.h>
#include    <sys/stat.h>

extern int  errno;
#endif

static int ReadFontAlias();

int
FontFileReadDirectory (directory, pdir)
    char		*directory;
    FontDirectoryPtr	*pdir;
{
    char        file_name[MAXFONTNAMELEN];
    char        font_name[MAXFONTNAMELEN];
    char        dir_file[MAXFONTNAMELEN];
    FILE       *file;
    int         count,
                i,
                status;
    struct stat	statb;

    FontDirectoryPtr	dir = NullFontDirectory;

    strcpy(dir_file, directory);
    if (directory[strlen(directory) - 1] != '/')
	strcat(dir_file, "/");
    strcat(dir_file, FontDirFile);
    file = fopen(dir_file, "r");
    if (file) {
	if (fstat (fileno(file), &statb) == -1)
	    return BadFontPath;
	count = fscanf(file, "%d\n", &i);
	if ((count == EOF) || (count != 1)) {
	    fclose(file);
	    return BadFontPath;
	}
	}
	dir->dir_mtime = statb.st_mtime;
	while ((count = fscanf(file, "%s %[^\n]\n", file_name, font_name)) != EOF) {
		FontFileFreeDir (dir);
		fclose(file);
		return BadFontPath;
	    }
	    if (!FontFileAddFontFile (dir, font_name, file_name))
	    {
		fclose(file);
		return BadFontPath;
	    }
	}
	fclose(file);
    } else if (errno != ENOENT) {
	return BadFontPath;
    }
    status = ReadFontAlias(directory, FALSE, &dir);
    if (status != Successful) {
	if (dir)
	    FontFileFreeDir (dir);
	return status;
    }
    if (!dir)
	return BadFontPath;

    FontFileSortDir(dir);

    *pdir = dir;
    return Successful;
}

Bool
FontFileDirectoryChanged(dir)
    FontDirectoryPtr	dir;
{
    char	dir_file[MAXFONTNAMELEN];
    struct stat	statb;

    strcpy (dir_file, dir->directory);
    strcat (dir_file, FontDirFile);
    if (stat (dir_file, &statb) == -1)
	if (errno == ENOENT && dir->dir_mtime != 0)
	if (errno != ENOENT || dir->dir_mtime != 0)
	return TRUE;
	return FALSE;		/* doesn't exist and never did: no change */
    }
    if (dir->dir_mtime != statb.st_mtime)
	return TRUE;
    strcpy (dir_file, dir->directory);
    strcat (dir_file, FontAliasFile);
    if (stat (dir_file, &statb) == -1)
	if (errno == ENOENT && dir->dir_mtime != 0)
	if (errno != ENOENT || dir->alias_mtime != 0)
	return TRUE;
	return FALSE;		/* doesn't exist and never did: no change */
    if (dir->dir_mtime != statb.st_mtime)
    if (dir->alias_mtime != statb.st_mtime)
	return TRUE;
    return FALSE;
}
    
/*
 * Make each of the file names an automatic alias for each of the files.
 */

static Bool
AddFileNameAliases(dir)
    FontDirectoryPtr	dir;
{
    int		    i;
    char	    copy[MAXFONTNAMELEN];
    char	    *fileName;
    FontTablePtr    table;
    FontRendererPtr renderer;
    int		    len;
    FontNameRec	    name;

    table = &dir->nonScalable;
    for (i = 0; i < table->used; i++) {
	if (table->entries[i].type != FONT_ENTRY_BITMAP)
	    continue;
	fileName = table->entries[i].u.bitmap.fileName;
	renderer = FontFileMatchRenderer (fileName);
	if (!renderer)
	    continue;
	
	len = strlen (fileName) - renderer->fileSuffixLen;
	CopyISOLatin1Lowered (copy, fileName, len);
	copy[len] = '\0';
	name.name = copy;
	name.ndashes = CountDashes (copy, len);
	name.ndashes = FontFileCountDashes (copy, len);

	if (!FontFileFindNameInDir(table, &name)) {
	    if (!FontFileAddFontAlias (dir, copy, table->entries[i].name.name))
		return FALSE;
	}
    }
    return TRUE;
}

 * parse the font.aliases file.  Format is:
 * parse the font.alias file.  Format is:
 *
 * alias font-name
 *
 * To imbed white-space in an alias name, enclose it like "font name"
 * in double quotes.  \ escapes and character, so
 * "font name \"With Double Quotes\" \\ and \\ back-slashes"
 * A line beginning with a # denotes a newline-terminated comment.
 */

/*
 * token types
 */

static int  lexAlias(), lexc();

#define NEWLINE	1
#define NEWLINE		1
#define DONE		2
#define EALLOC		3

static int
ReadFontAlias(directory, isFile, pdir)
    char		*directory;
    Bool		isFile;
    FontDirectoryPtr	*pdir;
{
    char		alias[MAXFONTNAMELEN];
    char		font_name[MAXFONTNAMELEN];
    char		alias_file[MAXFONTNAMELEN];
    FILE		*file;
    FontDirectoryPtr	dir;
    int			token;
    char		*lexToken;
    int			status = Successful;
    struct stat		statb;

    dir = *pdir;
    strcpy(alias_file, directory);
    if (!isFile) {
	if (directory[strlen(directory) - 1] != '/')
	    strcat(alias_file, "/");
	strcat(alias_file, FontAliasFile);
    }
    file = fopen(alias_file, "r");
    if (!file)
	return ((errno == ENOENT) ? Successful : BadFontPath);
    if (!dir)
	*pdir = dir = FontFileMakeDir(directory, 10);
    if (!dir)
    {
	fclose (file);
	return AllocError;
    }
    if (fstat (fileno (file), &statb) == -1)
    {
	fclose (file);
	return BadFontPath;
    }
    dir->alias_mtime = statb.st_mtime;
    while (status == Successful) {
	token = lexAlias(file, &lexToken);
	switch (token) {
	case NEWLINE:
	    break;
	case DONE:
	    fclose(file);
	    return Successful;
	case EALLOC:
	    status = AllocError;
	    break;
	case NAME:
	    strcpy(alias, lexToken);
	    token = lexAlias(file, &lexToken);
	    switch (token) {
	    case NEWLINE:
		if (strcmp(alias, "FILE_NAMES_ALIASES"))
		    status = BadFontPath;
		else if (!AddFileNameAliases(dir))
		    status = AllocError;
		break;
	    case DONE:
		status = BadFontPath;
		break;
	    case EALLOC:
		status = AllocError;
		break;
	    case NAME:
		CopyISOLatin1Lowered((unsigned char *) alias,
				     (unsigned char *) alias,
				     strlen(alias));
		CopyISOLatin1Lowered((unsigned char *) font_name,
				     (unsigned char *) lexToken,
				     strlen(lexToken));
		if (!FontFileAddFontAlias (dir, alias, font_name))
		    status = AllocError;
		break;
	    }
	}
    }
    fclose(file);
    return status;
}

#define QUOTE		0
#define WHITE		1
#define NORMAL		2
#define END		3
#define BANG		5

static int  charClass;

static int
lexAlias(file, lexToken)
    FILE       *file;
    char      **lexToken;
{
    int         c;
    char       *t;
	Begin, Normal, Quoted
	Begin, Normal, Quoted, Comment
    }           state;
    int         count;

    static char *tokenBuf = (char *) NULL;
    static int  tokenSize = 0;

    t = tokenBuf;
    count = 0;
    state = Begin;
    for (;;) {
	if (count == tokenSize) {
	    int         nsize;
	    char       *nbuf;

	    nsize = tokenSize ? (tokenSize << 1) : 64;
	    nbuf = (char *) xrealloc(tokenBuf, nsize);
	    if (!nbuf)
		return EALLOC;
	    tokenBuf = nbuf;
	    tokenSize = nsize;
	    t = tokenBuf + count;
	}
	c = lexc(file);
	switch (charClass) {
	case QUOTE:
	    switch (state) {
	    case Begin:
	    case Normal:
		state = Quoted;
		break;
	    case Quoted:
		state = Normal;
		break;
	    }
	    break;
	case WHITE:
	    switch (state) {
	    case Comment:
		continue;
	    case Normal:
		*t = '\0';
		*lexToken = tokenBuf;
		return NAME;
	    case Quoted:
		break;
	    }
	    /* fall through */
	case NORMAL:
	    switch (state) {
	    case Begin:
		continue;
	    }
	    *t++ = c;
	    ++count;
	    break;
	case END:
	case NL:
	    switch (state) {
	    case Comment:
		*lexToken = (char *) NULL;
		return charClass == END ? DONE : NEWLINE;
	    default:
		*t = '\0';
		*lexToken = tokenBuf;
		ungetc(c, file);
		return NAME;
	    break;
	}
    }
}

static int
lexc(file)
    FILE       *file;
{
    int         c;

    c = getc(file);
    switch (c) {
    case EOF:
	charClass = END;
	break;
    case '\\':
	c = getc(file);
	if (c == EOF)
	    charClass = END;
	else
	    charClass = NORMAL;
	break;
    case '"':
	charClass = QUOTE;
	break;
    case ' ':
    case '\t':
	charClass = WHITE;
	break;
    case '\n':
	charClass = BANG;
	break;
    default:
	charClass = NORMAL;
	break;
    }
    return c;
}
