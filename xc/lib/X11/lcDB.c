/* $XConsortium: lcDB.c,v 1.4 94/01/20 18:06:28 rws Exp $ */
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
#include	<X11/Xlib.h>
#include	<X11/Xresource.h>
#include	"Xlibint.h"
#include	"XlcPubI.h"
#include	<stdio.h>

static Bool	is_endof_category();
static int	get_token();
static char	*get_normal_word();
static char 	*cut_string_literal();
static Status	append_value_list();
static char	*new_string();
static char	*concat_name();
static void	clear_parse_info();

typedef struct _XLocaleDataBaseRec{
     XrmQuark	category;
     XrmQuark	name;
     char	**value;
     int	value_num;
     struct _XLocaleDataBaseRec	*next;
}XLocaleDataBaseRec, *XLocaleDataBase;

typedef	struct	_XLocaleDBCacheRec{
     XrmQuark		db_name_q;
     XLocaleDataBase	db;
     int		count;
     struct _XLocaleDBCacheRec	*next;
}XLocaleDBCacheRec, *XLocaleDBCache;

static enum {
     S_NULL,
     S_SEMICOLON,
     S_NEST_MARK,
     S_CATEGORY,
     S_NAME,
     S_VALUE
}state;

static enum {
     T_NL,
     T_COMMENT,
     T_SEMICOLON,
     T_DOUBLE_Q,
     T_BEGIN_NEST,
     T_END_NEST,
     T_HEX,
     T_DIGIT,
     T_OCTAL,
     T_WHITE,
     T_BACKSLASH,
     T_DEFAULT
}token;

typedef struct _KeywordRec{
     char	*name;
     int	len;
     int	value;
} KeywordRec;

static KeywordRec keyword_tbl[] = {
     {"\n",	1,	T_NL},
     {"#",	1,	T_COMMENT},
     {";",	1,	T_SEMICOLON},
     {"\"",	1,	T_DOUBLE_Q},
     {"{",	1,	T_BEGIN_NEST},
     {"}",	1,	T_END_NEST},
     {"\\x",	2,	T_HEX},
     {"\\d",	2,	T_DIGIT},
     {"\\o",	2,	T_OCTAL},
     {" ",	1,	T_WHITE},
     {"\t",	1,	T_WHITE},     
     {"\\",	1,	T_BACKSLASH},
     0
};

#define MAX_NAME_NEST	64

typedef struct {
     int	pre_state;
     char	*category;
     char	*name[MAX_NAME_NEST];
     int	nest_depth;
     char	**value;
     int	value_len;
     int	value_num;
}DBParseInfo;

static	DBParseInfo	parse_info;
static	XLocaleDBCache	db_cache = (XLocaleDBCache)NULL;

static Status store_to_database( 
#if NeedFunctionPrototypes
	XLocaleDataBase *xlocale_db 
#endif
);

/************************************************************************/
/*	_XlcGetResource(lcd,category,class,value,count)			*/
/*----------------------------------------------------------------------*/
/*	This function retrieves XLocale database information		*/
/************************************************************************/
void
_XlcGetResource(lcd, category, class, value, count)
     XLCd	lcd;
     char	*category;
     char	*class;
     char	***value;
     int	*count;
{
     XLCdPublicMethodsPart *methods = XLC_PUBLIC_METHODS(lcd);

     (*methods->get_resource)(lcd,category,class,value,count);
     return;
}

/************************************************************************/
/*	_XlcGetLocaleDataBase(lcd,category,class,value,count)		*/
/*----------------------------------------------------------------------*/
/*	This function retrieves XLocale database information		*/
/************************************************************************/
void
_XlcGetLocaleDataBase(lcd, category, name, value, count)
     XLCd	lcd;
     char	*category;
     char	*name;
     char	***value;
     int	*count;
{
     XLocaleDataBase	xlocale_db = (XLocaleDataBase)XLC_PUBLIC(lcd,xlocale_db);
     XLocaleDataBase	xldb;
     XrmQuark	category_q, name_q;

     category_q = XrmStringToQuark(category);
     name_q = XrmStringToQuark(name);
     for(xldb=xlocale_db; xldb!=NULL; xldb = xldb->next){
	  if((category_q == xldb->category) && (name_q == xldb->name)){
	       *value = xldb->value;
	       *count = xldb->value_num;
	       return;
	  }
     }
     *value = NULL;
     *count = 0;
}

/************************************************************************/
/*	_XlcDestroyLocaleDataBase(lcd)					*/
/*----------------------------------------------------------------------*/
/*	This function destroy the specified XLocale database. 		*/
/*	If the XLocale database is referd from other locale,		*/
/*	this function just decrease reference count of the database.	*/
/*	If no locale refers the database, this function relete it	*/
/*	from the cache list and free work area.				*/
/************************************************************************/
void
_XlcDestroyLocaleDataBase(lcd)
     XLCd	lcd;
{
     XLocaleDataBase 	xlocale_db = (XLocaleDataBase)XLC_PUBLIC(lcd,xlocale_db);
     XLocaleDataBase	xldb, next;
     XLocaleDBCache	cache, prev_cache;

     /* Seach in the cache list */
     for (cache=prev_cache=db_cache; cache; cache=cache->next) {
          if (cache->db == xlocale_db) { /* found */
	       if (cache->count > 1) { /* Someone refer this db */
	            cache->count--;
		    xlocale_db = (XLocaleDataBase)NULL;
		    return;
	       }
	       /* Destroy XLocale database instance */
               for (xldb=xlocale_db; xldb!=NULL; xldb = next) {
	            next = xldb->next;
	            if (xldb->value != NULL) {
	                 if (xldb->value[0] != NULL) Xfree(xldb->value[0]);
	                 Xfree(xldb->value);	  
	            }
	            Xfree(xldb);
               }
	       /* Remove it from cache list */
	       if (cache == db_cache) {
		    db_cache = cache->next;
	       } else { 
	            prev_cache->next = cache->next;
	       }
	       Xfree((char*)cache);
	       xlocale_db = (XLocaleDataBase)NULL;
	       return;
          }
	  prev_cache = cache;
     }
     xlocale_db = (XLocaleDataBase)NULL;
     return;
}

/************************************************************************/
/*	_XlcCreateLocaleDataBase(lcd)					*/
/*----------------------------------------------------------------------*/
/*	This function create a XLocale database which correspond to	*/
/*	a specified XLCd.						*/
/************************************************************************/
XPointer
_XlcCreateLocaleDataBase(lcd)
     XLCd	lcd;
{
     XLocaleDataBase	xlocale_db = (XLocaleDataBase) 0;
     XLocaleDBCache	db, new_db;
     FILE	*fd;
     int	line_count = 0;
     char	buffer[BUFSIZ];
     char	*buf_ptr;
     int	tok;
     int	token_len;
     Bool	error = False;
     char	value[BUFSIZ];
     int	value_len = 0;
     XrmQuark	db_name_q;
     char	pathname[256], *name;

     name = _XlcFileName(lcd, "locale");
     if(name == NULL){
	 return (XPointer)NULL;
     }
     strcpy(pathname, name);
     Xfree(name);

     /* Find cached XLocale database */
     db_name_q = XrmStringToQuark(pathname);
     for (db = db_cache; db; db = db->next) {
          if (db_name_q == db->db_name_q) {
	       db->count++;
	       XLC_PUBLIC(lcd,xlocale_db) = (XPointer)(db->db);
	       return (XPointer)(db->db);
          }
     }

     /* Creating new XLocale database */
     fd = fopen(pathname,"r");

     if (fd == NULL) return (XPointer)NULL;

     bzero(&parse_info, sizeof(DBParseInfo));
     while (fgets(buffer, BUFSIZ, fd) && !error) {
	  line_count++;
	  buf_ptr = buffer;
	  get_token(buf_ptr, &tok);
	  if(tok == T_COMMENT) continue;
	  value_len = 0;
	  parse_info.pre_state = S_NULL;
	  while(*buf_ptr && !error){
	       token_len = get_token(buf_ptr, &tok);
	       switch(tok){
	       case T_NL:
		    if(parse_info.pre_state == S_VALUE){
			 if(((value_len != 0) &&
			     (append_value_list(value, value_len)!=Success)) ||
			    (store_to_database(&xlocale_db) != Success)){
			      error = True;
			      break;
			 }
			 parse_info.value = NULL;
			 parse_info.value_len = 0;
			 parse_info.value_num = 0;
			 Xfree(parse_info.name[parse_info.nest_depth]);
			 parse_info.name[parse_info.nest_depth] = NULL;
		    }
		    value_len = 0;
	       case T_WHITE:
		    buf_ptr+=token_len;
		    break;
	       case T_SEMICOLON:
		    if((parse_info.pre_state != S_VALUE) ||
		       (append_value_list(value, value_len)!=Success)){
			 error = True;
			 break;	
		    }
		    value_len=0;
		    parse_info.pre_state = S_SEMICOLON;
		    buf_ptr+=token_len;
		    break;
	       case T_BEGIN_NEST:
		    if((parse_info.pre_state != S_NAME) ||
		       (parse_info.name[parse_info.nest_depth++] == NULL) ||
		       (parse_info.nest_depth > MAX_NAME_NEST)){
			 error = True;
			 break;
		    }
		    parse_info.pre_state = S_NEST_MARK;
		    buf_ptr+=token_len;
		    break;
	       case T_END_NEST:
	       {
		    if((parse_info.pre_state != S_NULL) ||
		       (--parse_info.nest_depth < 0)){
			 error = True;
			 break;
		    }
		    if(parse_info.name[parse_info.nest_depth] != NULL){
			 Xfree(parse_info.name[parse_info.nest_depth]);
		    }
		    parse_info.name[parse_info.nest_depth] = NULL;
		    parse_info.pre_state = S_NEST_MARK;
		    buf_ptr+=token_len;
		    break;
	       }
	       case T_DOUBLE_Q:
	       {
		    char	*string;
		    int		len;

		    if(((parse_info.pre_state == S_NAME) ||
			(parse_info.pre_state == S_SEMICOLON)) &&
		       (string = cut_string_literal(&buf_ptr, &len)) != NULL &&
		       (append_value_list(string, len) == Success)){
			 parse_info.pre_state = S_VALUE;
		    }else{
			 error = True;
		    }
		    break;
	       }
	       case T_HEX:
	       case T_DIGIT:
	       case T_OCTAL:
		    if((parse_info.pre_state == S_NAME) ||
		       (parse_info.pre_state == S_SEMICOLON) ||
		       (parse_info.pre_state == S_VALUE)){
			 char	*numeric_str, *tmp_buf_ptr, *end_ptr;
			 int	len;
			 tmp_buf_ptr = buf_ptr;
			 buf_ptr += token_len;
			 if(((numeric_str = 
			      get_normal_word(buf_ptr, &end_ptr, &len))
			     == NULL) ||
			    (value_len + token_len + len > BUFSIZ)){
			      if(numeric_str) Xfree(numeric_str);
			      error = True;
			      break;
			 }
			 strncpy(&(value[value_len]), tmp_buf_ptr, token_len);
			 strcpy(&(value[value_len+token_len]), numeric_str);
			 value_len += (token_len + len);
			 parse_info.pre_state = S_VALUE;
			 Xfree(numeric_str);
			 buf_ptr = end_ptr;
		    }else{
			 error = True;
		    }
		    break;
	       case T_BACKSLASH:
		    buf_ptr += token_len;
		    if(*buf_ptr == '\n'){
			 do{
			      if(fgets(buffer, BUFSIZ, fd) == NULL){
				   error=True;
				   break;
			      }
			      line_count++;
			      buf_ptr = buffer;
			      get_token(buf_ptr, &tok);
			 }while(tok == T_COMMENT);
			 break;
		    }
		    if(*buf_ptr == '\0'){
			 error = True;
			 break;
		    }
	       default:
	       {
		    char	*word, *end_ptr;
		    int		len;

		    if((word = get_normal_word(buf_ptr, &end_ptr, &len))
		       == NULL){
			 error = True;
			 break;
		    }
			 
		    switch(parse_info.pre_state){
		    case S_NULL:
		    {
			 int	cat_len;

			 if(parse_info.category == NULL) {
			      parse_info.category = word;
			      parse_info.pre_state = S_CATEGORY;
			      buf_ptr = end_ptr;
			 }else if((parse_info.nest_depth==0) &&
				  (is_endof_category(buf_ptr, &cat_len))){
			      clear_parse_info();
			      parse_info.pre_state = S_CATEGORY;
			      buf_ptr += cat_len;
			      Xfree(word);
			 }else{
			      if(parse_info.name[parse_info.nest_depth] 
				 != NULL){
				   Xfree(parse_info.name[parse_info.nest_depth]);
			      }
			      parse_info.name[parse_info.nest_depth] = word;
			      parse_info.pre_state = S_NAME;
			      buf_ptr = end_ptr;
			 }
			 break;
		    }
		    case S_NAME:
		    case S_SEMICOLON:
		    case S_VALUE:
		    {
			 if(value_len+len > BUFSIZ){
			      Xfree(word);
			      error=True;
			      break;
			 }
			 strcpy(&(value[value_len]), word);
			 value_len += len;
			 parse_info.pre_state = S_VALUE;
			 Xfree(word);
			 buf_ptr = end_ptr;
			 break;
		    }
		    default:
		         Xfree(word);
			 error=True;
			 break;
		    }/* end of switch(parse_info.pre_state) */
	       }/* end of default */
	       }/* end of switch(tok) */
	  }/* end of while(*buf_ptr && !error) */
     }/* end of while(fgets()) */	
     if(parse_info.category != NULL){
	  clear_parse_info();
	  error = True;
     }
     fclose(fd);
     if(error) {
	  fprintf(stderr,"syntax error: line %d.\n", line_count);
	  _XlcDestroyLocaleDataBase(lcd);
	  return ((XPointer)NULL);
     }

     /* Re-chain db_list */
     new_db = (XLocaleDBCache)Xmalloc(sizeof(XLocaleDBCacheRec));
     if (new_db == (XLocaleDBCache)NULL) {
	  return ((XPointer)NULL);
     }
     new_db->db_name_q = db_name_q;
     new_db->db = xlocale_db;
     new_db->count = 1;
     new_db->next = db_cache;
     db_cache = new_db;

     XLC_PUBLIC(lcd,xlocale_db) = (XPointer)xlocale_db;
     return (XPointer)xlocale_db;
}

/************************************************************************/
/*	Internal Subfunctions						*/
/************************************************************************/
static Status
store_to_database(xlocale_db)
XLocaleDataBase	*xlocale_db;
{
     XLocaleDataBase	new;
     if(parse_info.name[parse_info.nest_depth] == NULL){
	  parse_info.value = NULL;
	  return(! Success);
     }else{
	  char	*name;
	  if(((new = (XLocaleDataBase)Xmalloc(sizeof(XLocaleDataBaseRec)))
	      == NULL) ||
	     ((name = concat_name()) == NULL)){
	       if(new != NULL) Xfree(new);
	       return(! Success);
	  }
	  new->category = XrmStringToQuark(parse_info.category);
	  new->name = XrmStringToQuark(name);
	  new->next = *xlocale_db;
	  new->value = parse_info.value;
	  new->value_num = parse_info.value_num;
	  Xfree(name);
	  *xlocale_db = new;
     }
     return(Success);
}

/*****************************************/
#define END_MARK	"END "

static Bool
is_endof_category(str, len)
char	*str;
int	*len;
{
     int	cat_len = strlen(parse_info.category);
     int	end_len = strlen(END_MARK);
     *len = 0;
     if(strncmp(str, END_MARK, end_len) ||
	strncmp(str+end_len, parse_info.category, cat_len)){
	  return(False);
     }
     *len = end_len + cat_len;
     return(True);
}

/*****************************************/
static int
get_token(str, token)
char	*str;
int	*token;
{
    KeywordRec *keyword = keyword_tbl;
    
    for ( ; keyword->name; keyword++) {
        if (! strncmp(str, keyword->name, keyword->len)) {
	     *token = keyword->value;
	     return(keyword->len);
        }
    }
    *token = T_DEFAULT;
    return(1);
}

/*****************************************/
static char *
get_normal_word(str,dest,len)
char	*str, **dest;
int	*len;
{
     int	tok;
     int	tok_len, word_len=0;
     char	*strptr,word[BUFSIZ],*word_ptr; 
     char	c;

     for (strptr = str; *strptr != (char)EOF && word_len < BUFSIZ; ){
	  tok_len = get_token(strptr, &tok);
	  if(tok == T_BACKSLASH){
	       c = *(strptr+tok_len);
	       if(c == '\n' || c == '\0'){
		    break;
	       }
	       strptr += tok_len;
	  }else if(tok != T_DEFAULT){
	       break;
	  }
	  strncpy(&(word[word_len]), strptr, tok_len);
	  strptr+=tok_len; word_len+=tok_len;
     }
     *len = word_len;
     word_ptr = new_string(word, word_len);
     *dest = strptr;
     return word_ptr;
}

/*****************************************/
static char *
cut_string_literal(str, len)
char **str;
int *len;
{
    char ch, *strptr, *word;

    *len = 0;
    word = strptr = ++(*str);
    ch = *strptr;
    for(ch = *strptr; ch != '"'; ch = *(++strptr)){
	 if(ch == '\0'){
	      *len = 0;
	      return(NULL);
	 }
	 if(ch == '\\'){
	      ++strptr;
	 }
    }
    if (strptr == *str) return NULL;
    *len = strptr - *str;
    *str = ++strptr;
    return word;
}

/*****************************************/
static Status
append_value_list(str,str_len)
char	*str;
int	str_len;
{
     char	**value_list = parse_info.value;
     char	*value;
     int	value_num = parse_info.value_num;
     int	length;
     char	*top;

     if(str == NULL) return(! Success);

     if(value_list == NULL){
	  value_list = (char **)Xmalloc(sizeof(char *)*2);
	  value_list[0]=NULL;
     }else{
	  value_list = 
	       (char **)Xrealloc(value_list,sizeof(char *)*(value_num+2));
     }
     if(value_list == NULL) return(! Success);

     length = parse_info.value_len + str_len + 1;
     value = value_list[0];
     if(value == NULL){
	  value = (char *)Xmalloc(length);
     }else{
	  value = (char *)Xrealloc(value, length);
     }
     if(value == NULL){
	  Xfree(value_list);
	  return(! Success);
     }
     if(value != value_list[0]){
	  int	delta, i;
	  delta = value - value_list[0];
	  value_list[0] = value;
	  for (i = 1; i < value_num; i++) {
	       value_list[i] += delta;
	  }
     }
     value_list[value_num] = top = &(value[parse_info.value_len]);
     strncpy(top, str, str_len);
     top[str_len] = 0;
     value_list[++value_num] = NULL;
     parse_info.value = value_list;
     parse_info.value_len = length;
     parse_info.value_num = value_num;
     return(Success);
}


/*****************************************/
static char	*
new_string(str, len)
char	*str;	
int	len;	/* string length */
{
     char	*new;
     /* 
      *	returned string will be null teminated. 
      * so, need Xmalloc(string length + 1).
      */
     if((new = (char *)Xmalloc(len+1)) == NULL){
	  return(NULL);
     }
     strncpy(new, str, len);
     new[len] = '\0';
     return new;
}

/*****************************************/
static char	*
concat_name()
{
     register int	i, len;
     char		*name;

     for(i=0, len=0; i<parse_info.nest_depth+1; i++){
	  len += strlen(parse_info.name[i]) + 1;
     }
     if((name = (char *)Xmalloc(len)) == NULL){
	  return(NULL);
     }
     strcpy(name, parse_info.name[0]);
     for(i=1; i<parse_info.nest_depth+1; i++){
	  strcat(name, ".");
	  strcat(name, parse_info.name[i]);
     }
     return(name);
}

/*****************************************/
static void
clear_parse_info()
{
     int	i;

     if(parse_info.category != NULL){
	  Xfree(parse_info.category);
     }

     for(i=0; i<parse_info.nest_depth; i++){
	  if(parse_info.name[i] != NULL){
	       Xfree(parse_info.name[i]);
	  }
     }

     if(parse_info.value != NULL){
	  if(parse_info.value[0] != NULL){
	       Xfree(parse_info.value[0]);
	  }
	  Xfree(parse_info.value);
     }
     bzero(&parse_info, sizeof(DBParseInfo));
}
