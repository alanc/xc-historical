/* $XConsortium: saveutil.c,v 1.19 94/08/10 14:59:28 mor Exp mor $ */
/******************************************************************************

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
******************************************************************************/

#include "xsm.h"

char 		 session_save_file[PATH_MAX];
Bool	 	 getline();



void
set_session_save_file_name (session_name)

char *session_name;

{
    char *p;

    p = (char *) getenv ("SM_SAVE_DIR");
    if (!p)
    {
	p = (char *) getenv ("HOME");
	if (!p)
	    p = ".";
    }

    strcpy (session_save_file, p);
    strcat (session_save_file, "/.XSM-");
    strcat (session_save_file, session_name);
}



int
ReadSave(session_name, sm_id)

char *session_name;
char **sm_id;

{
    char		*buf;
    int			buflen;
    char		*p;
    PendingClient	*c;
    Prop		*prop;
    PropValue		*val;
    FILE		*f;
    int			state;
    int			version_number;

    f = fopen(session_save_file, "r");
    if(!f) {
	if (verbose)
	    printf("No session save file.\n");
	*sm_id = NULL;
	return 0;
    }
    if (verbose)
	printf("Reading session save file...\n");

    buf = NULL;
    buflen = 0;

    /* Read version # */
    getline(&buf, &buflen, f);
    if(p = strchr(buf, '\n')) *p = '\0';
    version_number = atoi (buf);
    if (version_number != SAVEFILE_VERSION)
    {
	if (verbose)
	    printf("Incompatible version of session save file.\n");
	*sm_id = NULL;
	return 0;
    }

    /* Read SM's id */
    getline(&buf, &buflen, f);
    if(p = strchr(buf, '\n')) *p = '\0';
    *sm_id = XtNewString(buf);

    state = 0;
    while(getline(&buf, &buflen, f)) {
	if(p = strchr(buf, '\n')) *p = '\0';
	for(p = buf; *p && isspace(*p); p++) /* LOOP */;
	if(*p == '#') continue;

	if(!*p) {
	    state = 0;
	    continue;
	}

	if(!isspace(buf[0])) {
	    switch(state) {
		case 0:
		    c = (PendingClient *)XtMalloc(sizeof *c);
		    if(!c) nomem();

		    c->clientId = XtNewString(p);
		    c->clientHostname = NULL;  /* set in next state */

		    c->props = ListInit();
		    if(!c->props) nomem();

		    if(!ListAddLast(PendingList, (void *)c)) nomem();

		    state = 1;
		    break;

		case 1:
		    c->clientHostname = XtNewString(p);
                    state = 2;
                    break;

		case 2:
		case 4:
		    prop = (Prop *)XtMalloc(sizeof *prop);
		    if(!prop) nomem();

		    prop->name = XtNewString(p);
		    prop->values = ListInit();
		    if(!prop->values) nomem();

		    prop->type = NULL;

		    if(!ListAddLast(c->props, (void *)prop)) nomem();

		    state = 3;
		    break;

		case 3:
		    prop->type = XtNewString(p);
		    state = 4;
		    break;

		default:
		    fprintf(stderr, "state %d\n", state);
		    fprintf(stderr,
			    "Corrupt save file line ignored:\n%s\n", buf);
		    continue;
	    }
	} else {
	    if (state != 4) {
		fprintf(stderr, "Corrupt save file line ignored:\n%s\n", buf);
		continue;
	    }
	    val = (PropValue *)XtMalloc(sizeof *val);
	    if(!val) nomem();

	    if (strcmp (prop->type, SmCARD8) == 0)
	    {
		val->length = 1;
		val->value = (XtPointer) XtMalloc (1);
		*((char *)(val->value)) = atoi (p);
	    }
	    else
	    {
		val->length = strlen(p);
		val->value = XtNewString(p);
	    }

	    if(!ListAddLast(prop->values, (void *)val)) nomem(); 
	}
    }
    fclose(f);

    return 1;
}



static void
SaveClient (f, client)

FILE	  *f;
ClientRec *client;

{
    List *pl;

    fprintf (f, "%s\n", client->clientId);
    fprintf (f, "%s\n", client->clientHostname);

    for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
    {
	Prop *pprop = (Prop *) pl->thing;
	List *pj, *vl;
	PropValue *pval;

	fprintf (f, "%s\n", pprop->name);
	fprintf (f, "%s\n", pprop->type);

	if (strcmp (pprop->type, SmCARD8) == 0)
	{
	    char *card8;
	    int value;

	    vl = ListFirst (pprop->values);
	    pval = (PropValue *) vl->thing;

	    card8 = pval->value;
	    value = *card8;
	    fprintf(f, "\t%d\n", value);
	}
	else
	{
	    for (pj = ListFirst (pprop->values); pj; pj = ListNext (pj))
	    {
		pval = (PropValue *) pj->thing;
		fprintf (f, "\t%s\n", pval->value);
	    }
	}
    }

    fprintf (f, "\n");
}



void
WriteSave (sm_id)

char *sm_id;

{
    ClientRec *client;
    FILE *f;
    List *cl;

    f = fopen (session_save_file, "w");

    if (!f)
    {
	perror ("open session save file for write");
    }
    else
    {
	fprintf (f, "%d\n", SAVEFILE_VERSION);
	fprintf (f, "%s\n", sm_id);

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    if (client->restartHint == SmRestartNever)
		continue;

	    SaveClient (f, client);
	}

	for (cl = ListFirst (RestartAnywayList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    SaveClient (f, client);
	}

	fclose(f);
    }
}



void
ExecuteOldDiscardCommands(session_name)

char *session_name;

{
    char	*buf;
    int		buflen;
    char	*p, *dir;
    FILE	*f;
    int		state;
    int		foundDiscard;
    char	filename[256];
    int		version_number;

    dir = (char *) getenv ("SM_SAVE_DIR");
    if (!dir)
    {
	dir = (char *) getenv ("HOME");
	if (!dir)
	    dir = ".";
    }

    sprintf (filename, "%s/.XSM-%s", dir, session_name);

    f = fopen(filename, "r");
    if(!f) {
	return;
    }

    buf = NULL;
    buflen = 0;

    /* Read version # */
    getline(&buf, &buflen, f);
    if(p = strchr(buf, '\n')) *p = '\0';
    version_number = atoi (buf);
    if (version_number != SAVEFILE_VERSION)
    {
	if (verbose)
	    printf("Can't delete session save file - incompatible version.\n");
	return;
    }

    /* Read SM's id */
    getline(&buf, &buflen, f);
    if(p = strchr(buf, '\n')) *p = '\0';

    state = 0;
    foundDiscard = 0;
    while(getline(&buf, &buflen, f)) {
	if(p = strchr(buf, '\n')) *p = '\0';
	for(p = buf; *p && isspace(*p); p++) /* LOOP */;
	if(*p == '#') continue;

	if(!*p) {
	    state = 0;
	    foundDiscard = 0;
	    continue;
	}

	if(!isspace(buf[0])) {
	    switch(state) {
		case 0:
		    state = 1;
		    break;

		case 1:
                    state = 2;
                    break;

		case 2:
		case 4:
		    if (strcmp (p, SmDiscardCommand) == 0)
			foundDiscard = 1;
		    state = 3;
		    break;

		case 3:
		    state = 4;
		    break;

		default:
		    continue;
	    }
	} else {
	    if (state != 4) {
		continue;
	    }
	    if (foundDiscard)
	    {
		system (p);	/* Discard Command */
		foundDiscard = 0;
	    }
	}
    }

    fclose(f);
}



Bool
getline(pbuf, plen, f)
char	**pbuf;
int	*plen;
FILE	*f;
{
	int c;
	int i;

	i = 0;
	while(1) {
	    if(i+2 > *plen) {
		if(*plen) *plen *= 2;
		else *plen = BUFSIZ;
		if(*pbuf) *pbuf = (char *) realloc(*pbuf, *plen + 1);
		else *pbuf = (char *) XtMalloc(*plen + 1);
	    }
	    c = getc(f);
	    if(c == EOF) break;
	    (*pbuf)[i++] = c;
	    if(c == '\n') break;
	}
	(*pbuf)[i] = '\0';
	return i;
}
