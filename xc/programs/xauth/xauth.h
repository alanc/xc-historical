#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xauth.h>

#ifndef True
typedef int Bool;
#define False 0
#define True 1
#endif

extern char *ProgramName;

extern char *copystring(), *malloc(), *realloc();
int process_command(), auth_initialize(), auth_finalize();
extern void print_help();
extern Bool format_numeric;
extern Bool verbose;
