/* $XConsortium$ */
/* POSIX dirent.h compatibility. */

#include <sys/dir.h>


/* This is GROSS! BSD uses direct, POSIX uses dirent. */
#define dirent direct
