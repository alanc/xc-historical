#ifdef DEBUG
#include	<stdio.h>
#ifdef SABER
#define assert(ex)	{if (!(ex)){(void)fprintf(stderr,"Assertion \"ex\" failed: file \"%s\", line %d\n", __FILE__, __LINE__);saber_stop();}}
#else
#define assert(ex)	{if (!(ex)){(void)fprintf(stderr,"Assertion \"ex\" failed: file \"%s\", line %d\n", __FILE__, __LINE__);abort();}}
#endif
#else
#define assert(ex)
#endif
