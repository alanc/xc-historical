/* $XConsortium$ */

/*
 * This is a very simple emulation for BSD with the standard compiler
 * It is limited to 6 args.
 * Do NOT use if the system posesses vsprintf()
 * NB. This is not even necessarily portable even to all BSD systems.
 * This is only required temporarily until the tet_infoline() issue
 * is resolved.
 */

#include <varargs.h>

vsprintf(buf, fmt, args)
char	*buf;
char	*fmt;
va_list	args;
{
int 	a1, a2, a3, a4, a5, a6;

	a1  = va_arg(args, int);
	a2  = va_arg(args, int);
	a3  = va_arg(args, int);
	a4  = va_arg(args, int);
	a5  = va_arg(args, int);
	a6  = va_arg(args, int);

	sprintf(buf, fmt, a1, a2, a3, a4, a5, a6);

}

