/* $XConsortium$ */
/*
 * This file is an incredible crock to get the normally-inline functions
 * built into the server so that things can be debugged properly.
 */


#define static /**/
#define __inline__ /**/
#undef NO_INLINE
#include "compiler.h"
