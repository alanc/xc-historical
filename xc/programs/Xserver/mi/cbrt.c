/* $XConsortium: cbrt.c,v 1.0 90/08/20 19:10:14 rws Exp $ */

/* simple cbrt, in case your math library doesn't have a good one */

double pow();

double
cbrt(x)
    double x;
{
    return pow(x, 1.0/3.0);
}
