/* $XConsortium: cfbpolypnt.c,v 5.16 94/03/16 16:53:04 dpw Exp $ */

/*
 * Copyright 1988 Massachusetts Institute of Technology
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

#include <X11/Xauth.h>

main (argc, argv)
char	**argv;
{
    Xauth   test_data;
    char    *name, *data, *file;
    int	    state = 0;
    FILE    *output;

    while (*++argv) {
	if (!strcmp (*argv, "-file"))
	    file = *++argv;
	else if (state == 0) {
	    name = *argv;
	    ++state;
	} else if (state == 1) {
	    data = *argv;
	    ++state;
	}
    }
    if(!file) {
	fprintf (stderr, "No file\n");
	exit (1);
    }
    test_data.family = 0;
    test_data.address_length = 0;
    test_data.address = "";
    test_data.number_length = 0;
    test_data.number = "";
    test_data.name_length = strlen (name);
    test_data.name = name;
    test_data.data_length = strlen (data);
    test_data.data = data;
    output = fopen (file, "w");
    if (output) {
	XauWriteAuth (output, &test_data);
	fclose (output);
    }
}
