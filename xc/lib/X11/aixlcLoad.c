/* $XConsortium: aixlcLoad.c,v 1.1 93/09/17 13:25:19 rws Exp $ */
/*
 *
 * Copyright IBM Corporation 1993
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/
/************************************************************************/
/*	XaixlcLoad.c							*/
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "Xlibint.h"
#include "Xlcint.h"
#include "Xaixlcint.h"

/************************************************************************/
/*	Private defines							*/
/************************************************************************/
enum {
	LDX_INVALID_TYPE,
	LDX_DYNAMIC,
	LDX_STATIC_SIM,
	LDX_STATIC_MIM
};

enum {
	LDX_INVALID_VERSION,
	LDX_R5,
	LDX_R6
};

typedef struct _ldxDB {
	char	*lc_name;
	int	type;
	char	*option;
} ldxDBRec, *ldxDB;

static ldxDB	_ldxdb = (ldxDB)NULL;
static int	_ldxdb_size = 0;

typedef XLCd (*XLCdLoader)();

#define	LDXDBPATH_DEFAULT	"/usr/lib/nls/loc/X11/"
#define	LDXDBFILE	"ldx.dir"
#define	LDXPATH_DEFAULT	LDXDBPATH_DEFAULT
#define	LDXFILE_SUFFIX	".ldx"

#ifndef	_POSIX_MAX_PATH
#define	_POSIX_MAX_PATH	1024
#endif

#define	is_comment(s)	(*(s) == '\0' || *(s) == '#')

#define	_REALLOC(p, n)	((p) == NULL ? Xmalloc(n) : Xrealloc((p), (n)))

/************************************************************************/
/*	Private functions						*/
/************************************************************************/

static char *
read_line(line, linesize, fp)
	char	*line;
	int	linesize;
	FILE	*fp;
{
	while(fgets(line, linesize, fp) != NULL){
		if(! is_comment(line)){
			return	line;
		}
	}

	return	NULL;
}

static int
parse_line(line, argv, argsize)
	char	*line;
	char	**argv;
	int	argsize;
{
	int	argc = 0;
	char	*p = line;

	while(argc < argsize){
		while(isspace(*p)){
			++p;
		}
		if(*p == '\0'){
			break;
		}
		argv[argc++] = p;
		while(! isspace(*p)){
			++p;
		}
		if(*p == '\0'){
			break;
		}
		*p++ = '\0';
	}

	return	argc;
}

static int
set_entries(db, argc, argv)
	ldxDB	db;
	int	argc;
	char	**argv;
{
	char	*p;

	if(argc < 1){
		return	0;
	}

	/* Column 1: full locale name */
	p = Xmalloc(strlen(argv[0]) + 1);
	if(p == NULL){
		return	0;
	}
	strcpy(p, argv[0]);
	db->lc_name = p;

	/* Column 2: ldx type   (optional) */
	db->type = LDX_STATIC_SIM;	/* default type */
	if(argc < 2){
		db->option = NULL;
		return	1;
	}
	if(! strcmp(argv[1], "LDX_DYNAMIC")){
		db->type = LDX_DYNAMIC;
	}else if(! strcmp(argv[1], "LDX_STATIC_MIM")){
		db->type = LDX_STATIC_MIM;
	}

	/* Column 3: ldx option (optional) */
	if(argc < 3 || ! strcmp(argv[2], "NONE")){
		db->option = NULL;
		return	1;
	}
	p = Xmalloc(strlen(argv[2]) + 1);
	if(p == NULL){
		Xfree(db->lc_name);
		return	0;
	}
	strcpy(p, argv[2]);
	db->option = p;

	return	1;
}

static void
read_ldxdb(path)
	char	*path;
{
	char	filename[_POSIX_PATH_MAX];
	FILE	*fp;
	ldxDB	db = _ldxdb;
	int	db_size = _ldxdb_size;
	char	line[_POSIX_PATH_MAX];
	int	cnt;

	/* Construct database file name */
	strcpy(filename, path);
	if(path[strlen(path) - 1] != '/'){
		strcat(filename, "/");
	}
	strcat(filename, LDXDBFILE);

	/* Open database file */
	fp = fopen(filename, "r");
	if(fp == (FILE *)NULL){
		return;
	}

	/* find top */
	for(cnt = 0; cnt < db_size; ++cnt){
		if(db[cnt].lc_name == NULL){
			break;
		}
	}

	/* Read database file and set entry */
	for(;;){
		int	argc;
		char	*argv[10];

		if(! read_line(line, _POSIX_PATH_MAX, fp)){
			/* No more line */
			break;
		}

		argc = parse_line(line, argv, 10);
		if(argc == 0){
			continue;
		}
		if(cnt >= db_size){
			int	size = db_size + 16;

			db = (ldxDB)_REALLOC(db, sizeof(ldxDBRec) * size);
			if(db == (ldxDB)NULL){
				goto err;
			}
			db_size = size;
			bzero(db + cnt, sizeof(ldxDBRec) * (db_size - cnt));
		}
		if(! set_entries(db + cnt, argc, argv)){
			goto err;	/* fatal error */
		}

		++cnt;
	}

	fclose(fp);

	_ldxdb = db;
	_ldxdb_size = db_size;

	return;

 err:;
	/* discard entries extracted from database file */
	if(db != (ldxDB)NULL){
		int	i;
		for(i = 0; i < db_size; ++i){
			if(db[i].lc_name == NULL){
				break;
			}
			Xfree(db[i].lc_name);
			Xfree(db[i].option);
		}
		Xfree((char *)db);
	}

	if(fp != (FILE *)NULL){
		fclose(fp);
	}

	_ldxdb = NULL;
	_ldxdb_size = 0;

	return;
}

static void
create_ldxdb()
{
	char	*pathlist, *path;
	char	buffer[_POSIX_PATH_MAX];

	LockMutex();

	if(_ldxdb != (ldxDB)NULL){
		/* ldx database is already created */
		UnlockMutex();
		return;
	}

	pathlist = getenv("LDXDBPATH");
	if(pathlist == NULL){
		pathlist = LDXDBPATH_DEFAULT;
	}

	strcpy(buffer, pathlist);
	pathlist = buffer;

	while((path = pathlist) != NULL){
		pathlist = strchr(pathlist, ':');
		if(pathlist != NULL){
			*pathlist++ = '\0';
		}
		read_ldxdb(path);
	}

	UnlockMutex();
}

static int
resolve_ldxinfo(lc_name, type, option)
	char	*lc_name;
	int	*type;
	char	**option;
{
	ldxDB	db;

	if(_ldxdb == (ldxDB)NULL){
		create_ldxdb();
	}

	for(db = _ldxdb; db->lc_name != NULL; ++db){
		if(! strcmp(lc_name, db->lc_name)){
			*type = db->type;
			*option = db->option;
			return	1;
		}
	}

	return	0;
}

static int
get_ldxversion(ldx)
	_XlcCoreObj	ldx;
{
	if(ldx == (_XlcCoreObj)NULL){
		return	LDX_INVALID_VERSION;
	}

	if(ldx->lc_object_header.type_id == _LC_LDX_R6 &&
	   ldx->lc_object_header.magic   == _LC_MAGIC &&
	   ldx->lc_object_header.version == _LC_VERSION_R6){
		return	LDX_R6;
	}

	if(ldx->lc_object_header.type_id == _LC_LDX &&
	   ldx->lc_object_header.magic   == _LC_MAGIC &&
	   ldx->lc_object_header.version == _LC_VERSION){
		return	LDX_R5;
	}

	return	LDX_INVALID_VERSION;
}

typedef struct _ldxCache{
	char	*path;
	_XlcCoreObj	(*entrypoint)();
	int	ref_count;
	struct _ldxCache	*next;
} ldxCache;

static ldxCache	*_ldxcache = (ldxCache *)NULL;

static char *
complete_path(path, lc_name)
	char	*path;
	char	*lc_name;
{
	static char	buffer[_POSIX_PATH_MAX];

	if(getuid() != geteuid() || getgid() != getegid()){
		/* The loadable module name must be restricted for
		   security issue. */
		strcpy(buffer, LDXPATH_DEFAULT);
		strcat(buffer, lc_name);
		strcat(buffer, LDXFILE_SUFFIX);
		path = buffer;
	}else if(path == NULL || *path == '\0'){
		strcpy(buffer, lc_name);
		strcat(buffer, LDXFILE_SUFFIX);
		path = buffer;
	}else{
		int	len, idx;

		len = strlen(path);
		idx = len - strlen(LDXFILE_SUFFIX);
		if(idx < 0 || strcmp(path + idx, LDXFILE_SUFFIX)){
			strcpy(buffer, path);
			if(buffer[len - 1] == '/'){
				strcat(buffer, lc_name);
			}
			strcat(buffer, LDXFILE_SUFFIX);
			path = buffer;
		}
	}

	return	path;
}

static _XlcCoreObj
load_ldx(path)
	char	*path;
{
	ldxCache	*ptr;
	char	*searchpath = NULL;
	_XlcCoreObj	ldx, (*entry)();

	for(ptr = _ldxcache; ptr != (ldxCache *)NULL; ptr = ptr->next){
		if(! strcmp(path, ptr->path)){
			++ptr->ref_count;
			return	(*ptr->entrypoint)();
		}
	}

	/* Load ldx module dynamically */
	if(strchr(path, '/') == NULL){
		searchpath = getenv("LDXPATH");
		if(searchpath == NULL){
			searchpath = LDXPATH_DEFAULT;
		}
	}
	entry = (_XlcCoreObj (*)())load(path, 1, searchpath);
	if(entry == (_XlcCoreObj (*)())NULL){
		return	(_XlcCoreObj)NULL;
	}

	/* Create cache */
	ptr = (ldxCache *)Xmalloc(sizeof(ldxCache));
	if(ptr == (ldxCache *)NULL){
		goto err;
	}
	ptr->path = Xmalloc(strlen(path) + 1);
	if(ptr->path == NULL){
		Xfree((char *)ptr);
		goto err;
	}
	strcpy(ptr->path, path);
	ptr->entrypoint = entry;
	ptr->ref_count = 1;
	ptr->next = _ldxcache;
	_ldxcache = ptr;

	return	(*entry)();

 err:;
	/* Unload ldx */
	if(entry != (_XlcCoreObj (*)())NULL){
		unload((void *)entry);
	}
	return	(_XlcCoreObj)NULL;
}

static void
unload_ldx(path)
	char	*path;
{
	ldxCache	*pre, *ptr;

	for(pre = (ldxCache *)NULL, ptr = _ldxcache; ptr != (ldxCache *)NULL;
	    pre = ptr, ptr = ptr->next){
		if(! strcmp(path, ptr->path)){
			if((--ptr->ref_count) < 1){
				/* Free and rechain cache list */
				Xfree(ptr->path);
				unload((void *)ptr->entrypoint);
				if(pre != (ldxCache *)NULL){
					pre->next = ptr->next;
				}else{
					_ldxcache = ptr->next;
				}
				Xfree((char *)ptr);
			}
			return;
		}
	}
}

static XLCd
instantiate_xlcd(lc_name)
	char	*lc_name;
{
	int	type;
	char	*option;
	char	*path;
	_XlcCoreObj	ldx;
	XLCdLoader	_XlcInstantiate = (XLCdLoader)NULL;

	if(! resolve_ldxinfo(lc_name, &type, &option)){
		return	(XLCd)NULL;
	}

	switch(type){
	case LDX_STATIC_SIM:
		/* Use default(hardcoded) LDX which has
		   NO preedit feedback. TBD. */
		break;
	case LDX_STATIC_MIM:
		/* Use default(hardcoded) LDX which has
		   preedit feedback. TBD. */
		break;
	case LDX_DYNAMIC:
		path = complete_path(option, lc_name);
		if(path == NULL){
			break;
		}

		ldx = load_ldx(path);
		if(ldx == (_XlcCoreObj)NULL){
			break;
		}

		switch(get_ldxversion(ldx)){
		case LDX_R5:
			/* R5LDX is not supported yet. TBD. */
			break;
		case LDX_R6:
			_XlcInstantiate = (XLCdLoader)ldx->default_loader;
			break;
		default:
			/* Invalid version */
			break;
		}

		if(_XlcInstantiate == (XLCdLoader)NULL){
			unload_ldx(path);
		}
		break;
	}

	if(_XlcInstantiate == (XLCdLoader)NULL){
		/* Fallback to LDX_STATIC_SIM. TBD. */
		return	(XLCd)NULL;
	}

	return	(*_XlcInstantiate)(lc_name);
}

/************************************************************************/
/*	_XaixOSDynamicLoad()						*/
/*----------------------------------------------------------------------*/
/*	This loader creates XLCd with using AIX dynamic loading 	*/
/*	feature.							*/
/************************************************************************/
XLCd
_XaixOsDynamicLoad(name)
	char	*name;
{
	return	instantiate_xlcd(name);
}
