/* $XConsortium$ */

#include <stdio.h>
#include <string.h>

char	*optarg;
int 	optind = 1;
int 	opterr = 1;

getopt(argc, argv, optstring)
int 	argc;
char	**argv;
char	*optstring;
{
static int 	avplace;
char	*ap;
char	*cp;
int 	c;

	if (optind >= argc)
		return(EOF);

	ap = argv[optind] + avplace;

	/* At begining of arg but not an option */
	if (avplace == 0) {
		if (ap[0] != '-')
			return(EOF);
		else if (ap[1] == '-') {
			/* Special end of options option */
			optind++;
			return(EOF);
		} else if (ap[1] == '\0')
			return(EOF);	/* single '-' is not allowed */
	}

	/* Get next letter */
	avplace++;
	c = *++ap;

	cp = strchr(optstring, c);
	if (cp == NULL || c == ':') {
		if (opterr)
			fprintf(stderr, "Unrecognised option -- %c\n", c);
		return('?');
	}

	if (cp[1] == ':') {
		/* There should be an option arg */
		avplace = 0;
		if (ap[1] == '\0') {
			/* It is a separate arg */
			if (++optind >= argc) {
				if (opterr)
					fprintf(stderr, "Option requires an argument\n");
				return('?');
			}
			optarg = argv[optind++];
		} else {
			/* is attached to option letter */
			optarg = ap + 1;
			++optind;
		}
	} else {

		/* If we are out of letters then go to next arg */
		if (ap[1] == '\0') {
			++optind;
			avplace = 0;
		}

		optarg = NULL;
	}

	return(c);
}

#ifdef test

main(argc, argv)
int 	argc;
char	**argv;
{
extern char *optarg;
extern int 	optind;
int 	c;

	while ((c = getopt(argc, argv, "ab:cd:")) != EOF) {
		switch (c) {
		case 'a':
		case 'c':
			printf("opt -%c\n", c);
			break;
		case 'b':
		case 'd':
			printf("opt -%c %s\n", c, optarg);
			break;
		case '?':
			printf("Bad option %c\n", c);
			break;
		default:
			printf("unexpected default\n");
			break;
		}
	}
}
#endif
