#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Xauth.h>

#ifndef True
typedef int Bool;
#define False 0
#define True 1
#endif

extern char *ProgramName;
extern Bool okay_to_use_stdin;
int process_command_list(), auth_initialize();
extern void print_help();
extern Bool print_numeric;
