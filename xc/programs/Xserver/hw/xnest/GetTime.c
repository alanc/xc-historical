/* $XConsortium: GetTime.c,v 1.2 94/03/28 14:58:29 gildea Exp $ */
#include "Xos.h"
#include <time.h>

long
GetTimeInMillis()
{
    struct timeval  tp;

    X_GETTIMEOFDAY(&tp);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
