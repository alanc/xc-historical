/* $XConsortium$ */
 
#include "ximager5.h"
 
main()
{
       XYspace S;
       path p;
 
       InitImager();
       S = Scale(IDENTITY, 300.0, -300.0);
       p = Join(Line(Loc(S, 0.0, 1.0)), Line(Loc(S, 1.0, 0.0)));
       Interior(ClosePath(p), EVENODDRULE);
}
 
void Trace()
{
}
 
void *DEFAULTDEVICE;
