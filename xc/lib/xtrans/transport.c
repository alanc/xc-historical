#include "Xtransint.h"

#ifdef DNETCONN
#include "Xtransdnet.c"
#endif
#ifdef LOCALCONN
#include "Xtranslocal.c"
#endif
#if defined(TCPCONN) || defined(UNIXCONN)
#include "Xtranssock.c"
#endif
#ifdef TLICONN
#include "Xtranstli.c"
#endif
#include "Xtrans.c"
#include "Xtransutil.c"
