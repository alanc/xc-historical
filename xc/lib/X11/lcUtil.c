/* $XConsortium: lcUtil.c,v 1.1 93/09/17 13:32:05 rws Exp $ */
/******************************************************************

              Copyright 1991, 1992 by TOSHIBA Corp.
              Copyright 1992 by FUJITSU LIMITED

 Permission to use, copy, modify, distribute, and sell this software
 and its documentation for any purpose is hereby granted without fee,
 provided that the above copyright notice appear in all copies and
 that both that copyright notice and this permission notice appear
 in supporting documentation, and that the name of TOSHIBA Corp. and
 FUJITSU LIMITED not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior permission.
 TOSHIBA Corp. and FUJITSU LIMITED makes no representations about the
 suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.
 
 TOSHIBA CORP. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH REGARD
 TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS, IN NO EVENT SHALL TOSHIBA CORP. AND FUJITSU LIMITED BE
 LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Author   : Katsuhisa Yano       TOSHIBA Corp.
                                 mopi@osa.ilab.toshiba.co.jp
 Modifier : Takashi Fujiwara     FUJITSU LIMITED 
                                 fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <X11/Xos.h>
#include "Xlibint.h"

#ifdef X_NOT_STDC_ENV
#ifndef toupper
#define toupper(c)      ((int)(c) - 'a' + 'A')
#endif

extern char *getenv();
#endif

int 
_XlcCompareISOLatin1(str1, str2)
    char *str1, *str2;
{
    register char ch1, ch2;

    for ( ; (ch1 = *str1) && (ch2 = *str2); str1++, str2++) {
        if (islower(ch1))
            ch1 = toupper(ch1);
        if (islower(ch2))
            ch2 = toupper(ch2);

        if (ch1 != ch2)
            break;
    }

    return *str1 - *str2;
}

int 
_XlcNCompareISOLatin1(str1, str2, len)
    char *str1, *str2;
    int len;
{
    register char ch1, ch2;

    for ( ; (ch1 = *str1) && (ch2 = *str2) && len; str1++, str2++, len--) {
        if (islower(ch1))
            ch1 = toupper(ch1);
        if (islower(ch2))
            ch2 = toupper(ch2);

        if (ch1 != ch2)
            break;
    }

    if (len == 0)
        return 0;

    return *str1 - *str2;
}

static char *
get_word(str, len)
    register char *str;
    int *len;
{
    register char ch, *strptr;

    ch = *str;
    while (ch == ' ' || ch == '\t' || ch == '\n')
        ch = *(++str);

    strptr = str;
    ch = *strptr;
    while (ch != ' ' && ch != '\t' && ch != '\n' && ch != 0)
        ch = *(++strptr);

    if (strptr == str)
        return NULL;

    *len = strptr - str;
    return str;
}

#ifndef XLOCALEDIR
#define XLOCALEDIR "/usr/lib/X11/locale"
#endif

FILE *
_XlcOpenLocaleFile(dir, locale, name)
    char *dir;
    char *locale;
    char *name;
{
    FILE *fd;
    char buf[BUFSIZE], locale_file[BUFSIZE];

    if (locale)
        sprintf(locale_file, "%s/%s", locale, name);
    else
        strcpy(locale_file, name);

    if (dir) {
        sprintf(buf, "%s/%s", dir, locale_file);
        if (fd = fopen(buf, "r"))
            return fd;
    }

    if (dir = getenv("XLOCALEDIR")) {
        sprintf(buf, "%s/%s", dir, locale_file);
        if (fd = fopen(buf, "r"))
            return fd;
    }
#ifdef sun
    if (dir = getenv("OPENWINHOME")) {
        sprintf(buf, "%s/lib/locale/%s", dir, locale_file);
        if (fd = fopen(buf, "r"))
            return fd;
    }
#endif
    sprintf(buf, "%s/%s", XLOCALEDIR, locale_file);

    return fopen(buf, "r");
}

#ifndef LOCALE_ALIAS
#define LOCALE_ALIAS    "locale.alias"
#endif

/************************************************************************/
/*	_XlcResolveLocaleName()						*/
/*----------------------------------------------------------------------*/
/*	This function resolves locale alias name with using a file	*/
/*	file database defined LOCALE_ALIAS.				*/
/*	(simplified locale name ==> full locale name)			*/
/*	Plus; this function decompose the full locale name into		*/
/*	each categories (language, territory, codeset)			*/
/************************************************************************/
char*
_XlcResolveLocaleName(name,language,territory,codeset)
	char 	*name;
	char	*language;
	char	*territory;
	char	*codeset;
{
	static	char	full_name[BUFSIZE];
	FILE	*fd;
	char	buf[BUFSIZE];
	char	*alias, *locale, *ptr, *name_p;
	int	len;

	full_name[0] = '\0';
	/*
	 * Open locale alias file
	 */
	if(fd = _XlcOpenLocaleFile((char*)NULL,(char*)NULL,LOCALE_ALIAS)) {
		while (fgets(buf,BUFSIZE,fd)) {
			/*
			 * Check if comment line
			 */
			if (buf[0] == '#' || buf[0] == '!') {
				continue;
			}
			if ((alias = get_word(buf,&len)) == (char*)NULL) {
				continue;
			}
			alias[len] = '\0';
			if (strcmp(alias,name)) {
				continue;	/* Not match */
			}
			if ((locale=get_word(buf+len+1,&len)) == (char*)NULL) {
				continue;
			}
			strncpy(full_name,locale,len);
			full_name[len] = '\0';
			goto found;
		}
	}
	strcpy(full_name, name);

 found:
	if (fd) {
		fclose(fd);
	}

	/*
	 * Decompose locale name
	 */
	if (language) *language = '\0';
	if (territory) *territory = '\0';
	if (codeset) *codeset = '\0';

	name_p = full_name;
	ptr = language;
	while (1) {
		if (*name_p == '_') {
			if (ptr)
				*ptr = '\0';
			ptr = territory;
		} else if (*name_p == '.') {
			if (ptr)
				*ptr = '\0';
			ptr = codeset;
		} else {
			if (ptr)
				*ptr++ = *name_p;
			if (*name_p == '\0')
				break;
		}
		name_p++;
	}
	return (full_name);
}

/************************************************************************/
/*	_XlcResolveDBName()						*/
/*----------------------------------------------------------------------*/
/*	This function resolves locale db file name with using a file	*/
/*	file database defined LOCALE_DIR.				*/
/*	(full locale name ==> locale database file name)		*/
/*	The resolved name is owned by this function, a caller should	*/
/*	not modify or free it.						*/
/************************************************************************/
#define	LOCALE_DIR	"locale.dir"
char*
_XlcResolveDBName(name)
	char 	*name;
{
	static	char	file_name[BUFSIZE];
	FILE	*fd;
	char	buf[BUFSIZE];
	char	*file, *locale;
	int	len;

	file_name[0] = '\0';
	/*
	 * Open locale alias file
	 */
	if(fd = _XlcOpenLocaleFile((char*)NULL,(char*)NULL,LOCALE_DIR)) {
		while (fgets(buf,BUFSIZE,fd)) {
			/* Skip comment line */
			if (buf[0] == '#' || buf[0] == '!') {
				continue;
			}
			if ((file = get_word(buf,&len)) == (char*)NULL) {
				continue;
			}
			file[len] = '\0';
			if ((locale=get_word(buf+len+1,&len)) == (char*)NULL) {
				continue;
			}
			locale[len] = '\0';
			if (strcmp(locale,name)) {
				continue;	/* Not match */
			}
			strcpy(file_name,file);
			goto found;
		}
	}
	if (fd) {
		fclose(fd);
	}
	return (char*)NULL;

 found:
	fclose(fd);
	return (file_name);
}

/************************************************************************/
/*	_XlcResolveI18NPath()						*/
/*----------------------------------------------------------------------*/
/*	This function resolves locale db path name with using 		*/
/*	vendor dependent environment variable.				*/
/************************************************************************/
char*
_XlcResolveI18NPath(name)
    char *name;
{
	static	char	path_name[BUFSIZE];
	char	*dir;

#ifdef notdef
#ifdef	sun
	if (dir = getenv("OPENWINHOME")) {
		sprintf(path_name,"%s/lib/locale/%s",dir,name);
#else
	if (dir = getenv("XLOCALEDIR")) {
		sprintf(path_name,"%s/%s",dir,name);
#endif
	} else {
		sprintf(path_name,"%s/%s",XLOCALEDIR,name);
	}
#endif

	if (dir = getenv("XLOCALEDIR"))
		sprintf(path_name,"%s/%s",dir,name);
	else
		sprintf(path_name,"%s/%s",XLOCALEDIR,name);

	return (path_name);
}
