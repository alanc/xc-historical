/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)config.c	4.1	91/05/02
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<X11/Xos.h>
#include	<strings.h>
#include	"misc.h"
#include	"configstr.h"
#include	"osdep.h"
#include	"globals.h"
#include	"access.h"

static char *font_catalogue = NULL;

static char *config_set_int(),
           *config_set_bool(),
           *config_set_list(),
           *config_set_file(),
           *config_set_resolutions();

/* these need to be in lower case and alphabetical order so a
 * binary search lookup can be used
 */
static ConfigOptionRec config_options[] = {
    {"alternate-servers", config_set_list},
    {"cache-size", config_set_int},
    {"catalogue", config_set_list},
    {"client-limit", config_set_int},
    {"clone-self", config_set_bool},
    {"default-point-size", config_set_int},
    {"default-resolutions", config_set_resolutions},
    {"error-file", config_set_file},
    {"server-number", config_set_int},
    {"trusted-clients", config_set_list},
    {"use-syslog", config_set_bool},
    {(char *) 0, 0},
};

char       *ConfigErrors[] = {
    "",
    "CONFIG: insufficient memory to load configuration file \"%s\"\n",
    "CONFIG: can't open configuration file \"%s\"\n",
    "CONFIG: error reading configuration file \"%s\"\n",
    "CONFIG: bad value \"%s\" for parameter \"%s\"\n",
    "CONFIG: unknown parameter \"%s\"\n",
    "CONFIG: missing '=' after parameter \"%s\"\n",
    "CONFIG: value out of range for parameter \"%s\"\n",
    "CONFIG: syntax error near parameter \"%s\"\n",
    "CONFIG: missing value for parameter \"%s\"\n",
    "CONFIG: extra value for parameter \"%s\"\n",
};

#define	iseol(c)	((c) == '\n' || (c) == '\r' || (c) == '\f')
#define	skip_whitespace(c)	while(isspace(*(c)) || *(c) == ',') (c)++;
#define	skip_val(c)	while(!isspace(*(c)) && *(c) != ',' && *(c) != '\0')\
						(c) ++;
#define	skip_list_val(c)	while(!isspace(*(c)) && *(c) != '\0')\
						(c) ++;
#define	blank_comment(c)	while (!iseol(*(c)) && *(c) != '\0')	\
						*(c)++= ' ';

static char *
next_assign(c)
    char       *c;
{
    int         nesting = 0;

    while (*c != '\0') {
	if (*c == '(')
	    nesting++;
	else if (*c == ')')
	    nesting--;
	else if (*c == '=' && nesting == 0)
	    return c;
	c++;
    }
    return (char *) 0;
}

static void
strip_comments(data)
    char       *data;
{
    char       *c;

    c = data;
    while ((c = index(c, '#')) != NULL) {
	if (c == data || *(c - 1) != '\\') {
	    blank_comment(c);
	} else {
	    c++;
	}
    }
}

static      ConfigOptionPtr
match_param_name(name)
    char       *name;
{
    int         pos,
                rc,
                low,
                high;

    low = 0;
    high = sizeof(config_options) / sizeof(ConfigOptionRec) - 2;
    pos = high >> 1;

    while (low <= high) {
	rc = strcmp(name, config_options[pos].parm_name);
	if (rc == 0) {
	    return &config_options[pos];
	} else if (rc < 0) {
	    high = pos - 1;
	} else {
	    low = pos + 1;
	}
	pos = ((high + low) >> 1);
    }
}

static int
parse_config(data)
    char       *data;
{
    char       *c,
               *val,
               *next_eq,
               *consumed,
               *p;
    char        param_name[64];
    Bool        equals_missing;
    ConfigOptionPtr param;

    c = data;
    skip_whitespace(c);

    while (*c != '\0') {
	equals_missing = FALSE;

	/* get parm name in lower case */
	p = c;
	while (isalnum(*c) || *c == '-') {
	    if (isupper(*c))
		*c = tolower(*c);
	    c++;
	}
	bcopy(p, param_name, min(sizeof(param_name), (int) (c - p)));
	param_name[(int) (c - p)] = '\0';

	/* check for junk */
	if (!isspace(*c) && *c != '=') {
	    fprintf(stderr, ConfigErrors[CONFIG_ERR_SYNTAX], param_name);
	    /* eat garbage */
	    while (!isspace(*c) && *c != '=' && *c != '\0')
		c++;
	}
	skip_whitespace(c);
	if (*c != '=') {
	    fprintf(stderr, ConfigErrors[CONFIG_ERR_NOEQUALS], param_name);
	    equals_missing = TRUE;
	} else {
	    c++;
	}

	skip_whitespace(c);

	/* find next assignment to guess where the value ends */
	if ((next_eq = next_assign(c)) != NULL) {
	    /* back up over whitespace */
	    for (val = next_eq - 1; val >= c &&
		    (isspace(*val) || *val == ',');
		    val--);

	    /* back over parm name */
	    for (; val >= c && (isalnum(*val) || *val == '-'); val--);

	    if (val <= c) {
		/* no value, ignore */
		fprintf(stderr, ConfigErrors[CONFIG_ERR_NOVALUE], param_name);
		continue;
	    }
	    *val = '\0';
	} else if (*c == '\0') {
	    /* no value, ignore */
	    fprintf(stderr, ConfigErrors[CONFIG_ERR_NOVALUE], param_name);
	    continue;
	}
	/* match parm name */
	if (equals_missing) {
	    equals_missing = FALSE;
	} else if ((param = match_param_name(param_name)) == NULL) {
	    fprintf(stderr, ConfigErrors[CONFIG_ERR_UNKNOWN], param_name);
	} else {
	    consumed = (param->set_func) (param, c);

	    skip_whitespace(consumed);
	    if (*consumed != '\0') {
		fprintf(stderr, ConfigErrors[CONFIG_ERR_EXTRAVALUE],
			param_name);
	    }
	}

	if (next_eq != NULL)
	    c = val + 1;
	else			/* last setting */
	    break;
    }
    return FSSuccess;
}

/*
 * handles anything that should be set once the file is parsed
 */
static void
set_config_values()
{
    int         err,
                num;

    err = SetFontCatalogue(font_catalogue, &num);
    if (err != FSSuccess) {
	FatalError("Element #%d of font path \"%s\" is bad\n", 
		num, font_catalogue);
    }
    fsfree((char *)font_catalogue);
    font_catalogue = NULL;
}

int
ReadConfigFile(filename)
    char       *filename;
{
    FILE       *fp;
    int         ret;
    int         len;
    char       *data;

    data = (char *) fsalloc(CONFIG_MAX_FILESIZE);
    if (!data) {
	fprintf(stderr, ConfigErrors[CONFIG_ERR_MEMORY], filename);
	return -1;
    }
    if ((fp = fopen(filename, "r")) == NULL) {
	fprintf(stderr, ConfigErrors[CONFIG_ERR_OPEN], filename);
	return -1;
    }
    ret = fread(data, sizeof(char), CONFIG_MAX_FILESIZE, fp);
    if (ret <= 0) {
	fprintf(stderr, ConfigErrors[CONFIG_ERR_READ], filename);
	return -1;
    }
    len = ftell(fp);
    len = min(len, CONFIG_MAX_FILESIZE);
    data[len] = '\0';		/* NULL terminate the data */

    (void) fclose(fp);

    strip_comments(data);
    ret = parse_config(data);

    fsfree(data);

    /* set the values that wait till the end */
    set_config_values();

    return ret;
}

struct {
    char       *name;
    int         val;
}           name_val[] = {

    "yes", TRUE,
    "on", TRUE,
    "1", TRUE,
    "true", TRUE,
    "no", FALSE,
    "off", FALSE,
    "0", FALSE,
    "false", FALSE,
    (char *) 0, 0,
};

static char *
config_parse_bool(c, ret, pval)
    char       *c;
    int        *ret;
    Bool       *pval;
{
    char       *start,
                t;
    int         i,
                len;

    start = c;
    skip_val(c);
    t = *c;
    *c = '\0';
    len = c - start;

    for (i = 0; name_val[i].name; i++) {
	if (!strncmpnocase(start, name_val[i].name, len)) {
	    *pval = name_val[i].val;
	    *ret = 0;
	    *c = t;
	    return c;
	}
    }
    fprintf(stderr, ConfigErrors[CONFIG_ERR_VALUE], start);
    *c = t;
    *ret = -1;
    return c;
}

static char *
config_parse_int(c, ret, pval)
    char       *c;
    int        *ret;
    int        *pval;
{
    char       *start,
                t;

    start = c;
    while (*c != '\0' && !isspace(*c) && *c != ',') {
	if (!isdigit(*c)) {	/* error */
	    skip_val(c);
	    t = *c;
	    *c = '\0';
	    fprintf(stderr, ConfigErrors[CONFIG_ERR_VALUE], start);
	    *ret = -1;
	    *c = t;
	    return c;
	}
	c++;
    }
    t = *c;
    *c = '\0';
    *ret = 0;
    *pval = atoi(start);
    *c = t;
    return c;
}


/* config option sets */
/* these have to know how to do the real work and tweak the proper things */
static char *
config_set_int(parm, val)
    ConfigOptionPtr parm;
    char       *val;
{
    int         ival,
                ret;
    extern int  serverNum;

    val = config_parse_int(val, &ret, &ival);
    if (ret == -1)
	return val;

    /* now do individual attribute checks */
    if (!strcmp(parm->parm_name, "cache-size")) {
    } else if (!strcmp(parm->parm_name, "server-number")) {
	serverNum = ival;
    } else if (!strcmp(parm->parm_name, "client-limit")) {
	AccessSetConnectionLimit(ival);
    } else if (!strcmp(parm->parm_name, "default-point-size")) {
	DefaultPointSize = ival;
    }
    return val;
}

static char *
config_set_bool(parm, val)
    ConfigOptionPtr parm;
    char       *val;
{
    int
                ret;
    Bool        bval;
    extern int  ClientLimit;
    extern Bool UseSyslog,
                CloneSelf;

    val = config_parse_bool(val, &ret, &bval);
    if (ret == -1)
	return val;

    /* now do individual attribute checks */
    if (!strcmp(parm->parm_name, "use-syslog")) {
	UseSyslog = bval;
    } else if (!strcmp(parm->parm_name, "clone-self")) {
	CloneSelf = bval;
    }
    return val;
}

static char *
config_set_file(parm, val)
    ConfigOptionPtr parm;
    char       *val;
{
    extern char ErrorFile[];
    char       *start = val,
                t;

    skip_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "error-file")) {
	bcopy(start, ErrorFile, val - start + 1);
    }
    *val = t;
    return val;
}

static char *
config_set_list(parm, val)
    ConfigOptionPtr parm;
    char       *val;
{
    char       *start = val,
                t;

    skip_list_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "catalogue")) {
	/* stash it for later */
	fsfree((char *)font_catalogue);	/* dump any previous one */
	font_catalogue = (char *) fsalloc((val - start) + 1);
	if (!font_catalogue)
	    FatalError("Insufficent memory for font catalogue\n");
	bcopy(start, font_catalogue, val - start);
	font_catalogue[val - start] = '\0';
    } else if (!strcmp(parm->parm_name, "alternate-servers")) {
	SetAlternateServers(start);
    }
    *val = t;
    return val;
}

static char *
config_set_resolutions(parm, val)
    ConfigOptionPtr parm;
    char       *val;
{
    char       *start = val,
                t;
    int         err;

    skip_list_val(val);
    t = *val;
    *val = '\0';
    if (!strcmp(parm->parm_name, "default-resolutions")) {
	err = SetDefaultResolutions(start);
	if (err != FSSuccess) {
	    FatalError("Bogus resolution list \"%s\"\n", start);
	}
    }
    *val = t;
    return val;
}
