/*
 * $XConsortium: cppsetup.c,v 1.11 93/07/13 18:03:01 gildea Exp $
 *
 * Copyright 1993 Massachusetts Institute of Technology
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
 */
#include "def.h"

#ifdef	CPP
/*
 * This file is strictly for the sake of cpy.y and yylex.c (if
 * you indeed have the source for cpp).
 */
#define IB 1
#define SB 2
#define NB 4
#define CB 8
#define QB 16
#define WB 32
#define SALT '#'
#if pdp11 | vax | ns16000 | mc68000 | ibm032
#define COFF 128
#else
#define COFF 0
#endif
/*
 * These variables used by cpy.y and yylex.c
 */
extern char	*outp, *inp, *newp, *pend;
extern char	*ptrtab;
extern char	fastab[];
extern char	slotab[];

/*
 * cppsetup
 */
struct filepointer	*currentfile;
struct inclist		*currentinc;

cppsetup(line, filep, inc)
	register char	*line;
	register struct filepointer	*filep;
	register struct inclist		*inc;
{
	register char *p, savec;
	static boolean setupdone = FALSE;
	boolean	value;

	if (!setupdone) {
		cpp_varsetup();
		setupdone = TRUE;
	}

	currentfile = filep;
	currentinc = inc;
	inp = newp = line;
	for (p=newp; *p; p++)
		;

	/*
	 * put a newline back on the end, and set up pend, etc.
	 */
	*p++ = '\n';
	savec = *p;
	*p = '\0';
	pend = p;

	ptrtab = slotab+COFF;
	*--inp = SALT; 
	outp=inp; 
	value = yyparse();
	*p = savec;
	return(value);
}

struct symtab *lookup(symbol)
	char	*symbol;
{
	static struct symtab    undefined;
	struct symtab   *sp;

	sp = isdefined(symbol, currentinc, NULL);
	if (sp == NULL) {
		sp = &undefined;
		sp->s_value = NULL;
	}
	return (sp);
}

pperror(tag, x0,x1,x2,x3,x4)
	int	tag,x0,x1,x2,x3,x4;
{
	warning("\"%s\", line %d: ", currentinc->i_file, currentfile->f_line);
	warning(x0,x1,x2,x3,x4);
}


yyerror(s)
	register char	*s;
{
	fatalerr("Fatal error: %s\n", s);
}
#else /* not CPP */

#include "ifparser.h"
struct _parse_data {
    struct filepointer *filep;
    struct inclist *inc;
    const char *line;
};

static const char *
_my_if_errors (ip, cp, expecting)
    IfParser *ip;
    const char *cp;
    const char *expecting;
{
    struct _parse_data *pd = (struct _parse_data *) ip->data;
    int lineno = pd->filep->f_line;
    char *filename = pd->inc->i_file;
    char prefix[300];
    int prefixlen;
    int i;

    sprintf (prefix, "\"%s\":%d", filename, lineno);
    prefixlen = strlen(prefix);
    fprintf (stderr, "%s:  %s", prefix, pd->line);
    i = cp - pd->line;
    if (i > 0 && pd->line[i-1] != '\n') {
	putc ('\n', stderr);
    }
    for (i += prefixlen + 3; i > 0; i--) {
	putc (' ', stderr);
    }
    fprintf (stderr, "^--- expecting %s\n", expecting);
    return NULL;
}


#define MAXNAMELEN 256

static struct symtab *
_lookup_variable (ip, var, len)
    IfParser *ip;
    const char *var;
    int len;
{
    char tmpbuf[MAXNAMELEN + 1];
    struct _parse_data *pd = (struct _parse_data *) ip->data;

    if (len > MAXNAMELEN)
	return 0;

    strncpy (tmpbuf, var, len);
    tmpbuf[len] = '\0';
    return isdefined (tmpbuf, pd->inc, NULL);
}


static int
_my_eval_defined (ip, var, len)
    IfParser *ip;
    const char *var;
    int len;
{
    if (_lookup_variable (ip, var, len))
	return 1;
    else
	return 0;
}

#define isvarfirstletter(ccc) (isalpha(ccc) || (ccc) == '_')

static int
_my_eval_variable (ip, var, len)
    IfParser *ip;
    const char *var;
    int len;
{
    struct symtab *s;

    s = _lookup_variable (ip, var, len);
    if (!s)
	return 0;
    do {
	var = s->s_value;
	if (!isvarfirstletter(*var))
	    break;
	s = _lookup_variable (ip, var, strlen(var));
    } while (s);

    return atoi(var);
}


cppsetup(line, filep, inc)
	register char	*line;
	register struct filepointer	*filep;
	register struct inclist		*inc;
{
    IfParser ip;
    struct _parse_data pd;
    int val = 0;

    pd.filep = filep;
    pd.inc = inc;
    pd.line = line;
    ip.funcs.handle_error = _my_if_errors;
    ip.funcs.eval_defined = _my_eval_defined;
    ip.funcs.eval_variable = _my_eval_variable;
    ip.data = (char *) &pd;

    (void) ParseIfExpression (&ip, line, &val);
    if (val)
	return IF;
    else
	return IFFALSE;
}
#endif /* CPP */

