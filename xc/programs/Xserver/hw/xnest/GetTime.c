/* $XConsortium: GetTime.c,v 1.1 93/07/19 17:49:05 rws Exp $ */
#include "Xos.h"
#include <time.h>

long
GetTimeInMillis()
{
    struct timeval  tp;

    GETTIMEOFDAY(&tp);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
