/* $XConsortium: XStrKeysym.c,v 11.9 90/10/30 08:51:40 rws Exp $ */
/* Copyright 1985, 1987, 1990 Massachusetts Institute of Technology */

#include "Xlibint.h"
#include <X11/Xresource.h>
#include <X11/keysymdef.h>

extern XrmQuark _XrmInternalStringToQuark();

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

typedef unsigned long Signature;

#define NEEDKTABLE
#include "ks_tables.h"

#ifndef KEYSYMDB
#define KEYSYMDB "/usr/lib/X11/XKeysymDB"
#endif

static Bool initialized;
static XrmDatabase keysymdb;
static XrmQuark Qkeysym[2];

XrmDatabase
_XInitKeysymDB()
{
    if (!initialized)
    {
	XrmInitialize();
	keysymdb = XrmGetFileDatabase(KEYSYMDB);
	if (keysymdb)
	    Qkeysym[0] = XrmStringToQuark("Keysym");
	initialized = True;
    }
    return keysymdb;
}

#if NeedFunctionPrototypes
KeySym XStringToKeysym(s)
    const char *s;
#else
KeySym XStringToKeysym(s)
    char *s;
#endif
{
    register int i, n;
    int h;
    register Signature sig = 0;
    register Const char *p = s;
    register int c;
    register int idx;
    Const unsigned char *entry;
    unsigned char sig1, sig2;
    KeySym val;

    while (c = *p++)
	sig = (sig << 1) + c;
    i = sig % KTABLESIZE;
    h = i + 1;
    sig1 = (sig >> 8) & 0xff;
    sig2 = sig & 0xff;
    n = KMAXHASH;
    while (idx = hashString[i])
    {
	entry = &__XkeyTable[idx];
	if ((entry[0] == sig1) && (entry[1] == sig2) &&
	    !strcmp(s, (char *)entry + 4))
	{
	    val = (entry[2] << 8) | entry[3];
	    if (!val)
		val = XK_VoidSymbol;
	    return val;
	}
	if (!--n)
	    break;
	i += h;
	if (i >= KTABLESIZE)
	    i -= KTABLESIZE;
    }

    if (!initialized)
	(void)_XInitKeysymDB();
    if (keysymdb)
    {
	XrmValue result;
	XrmRepresentation from_type;
	char c;
	KeySym val;
	XrmQuark names[2];

	names[0] = _XrmInternalStringToQuark(s, p - s - 1, sig, False);
	names[1] = NULLQUARK;
	(void)XrmQGetResource(keysymdb, names, Qkeysym, &from_type, &result);
	if (result.addr && (result.size > 1))
	{
	    val = 0;
	    for (i = 0; i < result.size - 1; i++)
	    {
		c = ((char *)result.addr)[i];
		if ('0' <= c && c <= '9') val = (val<<4)+c-'0';
		else if ('a' <= c && c <= 'z') val = (val<<4)+c-'a'+10;
		else if ('A' <= c && c <= 'Z') val = (val<<4)+c-'A'+10;
		else return NoSymbol;
	    }
	    return val;
	}
    }
    return (NoSymbol);
}
