/*
 * $XConsortium: parse.c,v 1.6 88/09/06 17:53:40 jim Exp $
 */
#include "def.h"
#include	<sys/signal.h>

extern char	*directives[];
extern struct symtab	deflist[];

find_includes(filep, file, file_red, recursion)
	struct filepointer	*filep;
	struct inclist		*file, *file_red;
	int			recursion;
{
	register char	*line;
	register int	type;

	while (line = getline(filep)) {
		switch(type = deftype(line, filep, file_red, file, TRUE)) {
		case IF:
			type = find_includes(filep, file,
				file_red, recursion+1);
			if (type == ELSE)
				gobble(filep, file, file_red);
			break;
		case IFFALSE:
			type = gobble(filep, file, file_red);
			if (type == ELSE)
				find_includes(filep, file,
					file_red, recursion+1);
			break;
		case IFDEF:
		case IFNDEF:
			if ((type == IFDEF && defined(line, file_red))
			 || (type == IFNDEF && !defined(line, file_red))) {
				debug1(type == IFNDEF ?
				    "line %d: %s !def'd in %s via %s%s\n" : "",
				    filep->f_line, line,
				    file->i_file, file_red->i_file, ": doit");
				type = find_includes(filep, file,
					file_red, recursion+1);
				if (type == ELSE)
					gobble(filep, file, file_red);
			}
			else {
				debug1(type == IFDEF ?
				    "line %d: %s !def'd in %s via %s%s\n" : "",
				    filep->f_line, line,
				    file->i_file, file_red->i_file, ": gobble");
				type = gobble(filep, file, file_red);
				if (type == ELSE)
					find_includes(filep, file,
						file_red, recursion+1);
			}
			break;
		case ELSE:
			if (!recursion)
				gobble(filep, file, file_red);
		case ENDIF:
			if (recursion)
				return(type);
		case DEFINE:
			define(line, file);
			break;
		case UNDEF:
			break;
		case INCLUDE:
			add_include(file, file_red, line, FALSE);
			break;
		case INCLUDEDOT:
			add_include(file, file_red, line, TRUE);
			break;
		case PRAGMA:
		case EJECT:
			break;
		case -1:
			log("%s, line %d: unknown directive == \"%s\"\n",
				file_red->i_file, filep->f_line, line);
			break;
		case -2:
			log("%s, line %d: incomplete include == \"%s\"\n",
				file_red->i_file, filep->f_line, line);
			break;
		}
	}
	return(-1);
}

gobble(filep, file, file_red)
	register struct filepointer *filep;
	struct inclist		*file, *file_red;
{
	register char	*line;
	register int	type;

	while (line = getline(filep)) {
		switch(type = deftype(line, filep, file_red, file, FALSE)) {
		case IF:
		case IFFALSE:
		case IFDEF:
		case IFNDEF:
			type = gobble(filep, file, file_red);
			if (type == ELSE)
				type = gobble(filep, file, file_red);
			break;
		case ELSE:
		case ENDIF:
			debug0("%s, line %d: #%s\n",
				file->i_file, filep->f_line,
				directives[type]);
			return(type);
		case DEFINE:
		case UNDEF:
		case INCLUDE:
		case INCLUDEDOT:
		case PRAGMA:
		case EJECT:
			break;
		case -1:
			log("%s, line %d: unknown directive == \"%s\"\n",
				file_red->i_file, filep->f_line, line);
			break;
		}
	}
	return(-1);
}

/*
 * Decide what type of # directive this line is.
 */
deftype(line, filep, file_red, file, parse_it)
	register char	*line;
	register struct filepointer *filep;
	register struct inclist *file_red, *file;
	int	parse_it;
{
	register char	*p;
	char	*directive, savechar;
	register int	ret;

	/*
	 * Parse the directive...
	 */
	directive=line+1;
	while (*directive == ' ' || *directive == '\t')
		directive++;

	p = directive;
	while (*p >= 'a' && *p <= 'z')
		p++;
	savechar = *p;
	*p = '\0';
	ret = match(directive, directives);
	*p = savechar;

	/*
	 * If we don't recognize this compiler directive or we happen
	 * to just be gobbling up text while waiting for an #endif or
	 * a #else, then we don't need to parse this line.
	 */
	if (ret < 0 || ! parse_it)
		return(ret);

	/*
	 * now decide how to parse the directive, and do it.
	 */
	while (*p == ' ' || *p == '\t')
		p++;
	switch (ret) {
	case IF:
		/*
		 * parse an expression.
		 */
		debug0("%s, line %d: #if %s\n",
			file->i_file, filep->f_line, p);
		if (zero_value(p, filep, file_red))
			ret = IFFALSE;
		break;
	case IFDEF:
	case IFNDEF:
		debug0("%s, line %d: #%s %s\n",
			file->i_file, filep->f_line, directives[ret], p);
	case UNDEF:
		/*
		 * separate the name of a single symbol.
		 */
		while (isalnum(*p) || *p == '_')
			*line++ = *p++;
		*line = '\0';
		break;
	case INCLUDE:
		debug2("%s, line %d: #include %s\n",
			file->i_file, filep->f_line, p);
		/* Support ANSI macro substitution */
		{
		    struct symtab *sym = defined(p, file_red);
		    if (sym) {
			p = sym->s_value;
		    }
		}
		/*
		 * Separate the name of the include file.
		 */
		while (*p && *p != '"' && *p != '<')
			p++;
		if (! *p)
			return(-2);
		if (*p++ == '"') {
			ret = INCLUDEDOT;
			while (*p && *p != '"')
				*line++ = *p++;
		} else
			while (*p && *p != '>')
				*line++ = *p++;
		*line = '\0';
		break;
	case DEFINE:
		/*
		 * copy the definition back to the beginning of the line.
		 */
		strcpy (line, p);
		break;
	case ELSE:
	case ENDIF:
	case PRAGMA:
	case EJECT:
		debug0("%s, line %d: #%s\n",
			file->i_file, filep->f_line, directives[ret]);
		/*
		 * nothing to do.
		 */
		break;
	}
	return(ret);
}

struct symtab *defined(symbol, file)
	register char	*symbol;
	struct inclist	*file;
{
	register struct symtab	*val;

	if (val = slookup(symbol, deflist)) {
		debug1("%s defined on command line\n", symbol);
		return(val);
	}
	if (val = fdefined(symbol, file))
		return(val);
	debug1("%s not defined in %s\n", symbol, file->i_file);
	return(NULL);
}

struct symtab *fdefined(symbol, file)
	register char	*symbol;
	struct inclist	*file;
{
	register struct inclist	**ip;
	register struct symtab	*val;
	register int	i;
	static int	recurse_lvl = 0;

	if (file->i_defchecked)
		return(NULL);
	file->i_defchecked = TRUE;
	if (val = slookup(symbol, file->i_defs))
		debug1("%s defined in %s\n", symbol, file->i_file);
	if (val == NULL && file->i_list)
		for (ip = file->i_list, i=0; i < file->i_listlen; i++, ip++)
			if (val = fdefined(symbol, *ip)) {
				debug1("%s defined in %s\n",
					symbol, (*ip)->i_file);
				break;
			}
	recurse_lvl--;
	file->i_defchecked = FALSE;

	return(val);
}

struct symtab *slookup(symbol, stab)
	register char	*symbol;
	register struct symtab	*stab;
{
	if (stab)
		for (; stab->s_name; stab++)
			if (strcmp(symbol, stab->s_name) == 0)
				return(stab);
	return(NULL);
}

/*
 * Return true if the #if expression evaluates to 0
 */
zero_value(exp, filep, file_red)
	register char	*exp;
	register struct filepointer *filep;
	register struct inclist *file_red;
{
#ifdef	CPP
	return (cppsetup(exp, filep, file_red) == 0);
#else	CPP
	return(TRUE);
#endif	CPP
}

define(def, file)
	register char	*def;
	register struct inclist	*file;
{
	register char	*p;
	struct symtab	*sp = file->i_lastdef++;
	register int	i;

	/*
	 * If we are out of space, allocate some more.
	 */
	if (file->i_defs == NULL
	|| file->i_lastdef == file->i_defs + file->i_deflen) {
		if (file->i_defs)
			file->i_defs = (struct symtab *) realloc(file->i_defs,
			    sizeof(struct symtab)*(file->i_deflen+SYMTABINC));
		else
			file->i_defs = (struct symtab *)
				malloc(sizeof (struct symtab) * SYMTABINC);
		i=file->i_deflen;
		file->i_deflen += SYMTABINC;
		while (i < file->i_deflen)
			file->i_defs[ i++ ].s_name = NULL;
		file->i_lastdef = file->i_defs + file->i_deflen - SYMTABINC;
		if (sp) /* be sure we use last cell in previous group */
			file->i_lastdef--;
		sp = file->i_lastdef++;
	}
	else if (file->i_lastdef > file->i_defs + file->i_deflen)
		log_fatal("define() botch\n");

	/*
	 * copy the symbol being defined.
	 */
	p = def;
	while (isalnum(*p) || *p == '_')
		p++;
	if (*p)
		*p++ = '\0';
	sp->s_name = copy(def);

	/*
	 * And copy its value.
	 */
	while (*p == ' ' && *p == '\t')
		p++;
	sp->s_value = copy(p);
}
