#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>

/*
 * This is the DNET implementation of the X Transport service layer
 */

static XtransConnInfo
TRANS(DNETOpenCOTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
PRMSG(1,"TRANS(DNETOpenCOTSClient)(%s,%s,%s) - Not Implemented\n",
			protocol, host, port);
return NULL;
}

static XtransConnInfo
TRANS(DNETOpenCOTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
return NULL;
}

static XtransConnInfo
TRANS(DNETOpenCLTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
return NULL;
}

static XtransConnInfo
TRANS(DNETOpenCLTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
return NULL;
}

static int
TRANS(DNETSetOption)(ciptr, option, arg)
XtransConnInfo	ciptr;
int		option;
int		arg;
{
}

static int
TRANS(DNETCreateListener)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
}

static XtransConnInfo
TRANS(DNETAccept)(ciptr)
XtransConnInfo	ciptr;
{
return NULL;
}

static int
TRANS(DNETConnect)(ciptr, host, port)
XtransConnInfo	ciptr;
char		*host;
char		*port;
{
}

static int
TRANS(DNETBytesReadable)(ciptr, pend)
XtransConnInfo	ciptr;
BytesReadable_t	*pend;
{
return -1;
}

static int
TRANS(DNETRead)(ciptr, buf, size)
XtransConnInfo	ciptr;
char		*buf;
int		size;
{
return read(ciptr->fd,buf,size);
}

static int
TRANS(DNETWrite)(ciptr, buf, size)
XtransConnInfo	ciptr;
char		*buf;
int		size;
{
return write(ciptr->fd,buf,size);
}

static int
TRANS(DNETReadv)(ciptr, buf, size)
XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;
{
return READV(ciptr,buf,size);
}

static int
TRANS(DNETWritev)(ciptr, buf, size)
XtransConnInfo	ciptr;
struct iovec	*buf;
int		size;
{
return WRITEV(ciptr,buf,size);
}

static int
TRANS(DNETDisconnect)(ciptr)
XtransConnInfo	ciptr;
{
}

static int
TRANS(DNETClose)(ciptr)
XtransConnInfo	ciptr;
{
}

static
TRANS(DNETNameToAddr)(ciptr)
XtransConnInfo	ciptr;
{
}

static
TRANS(DNETAddrToName)(ciptr)
XtransConnInfo	ciptr;
{
}

Xtransport	TRANS(DNETFuncs) = {
	/* DNET Interface */
	"decnet",
	0,
	TRANS(DNETOpenCOTSClient),
	TRANS(DNETOpenCOTSServer),
	TRANS(DNETOpenCLTSClient),
	TRANS(DNETOpenCLTSServer),
	TRANS(DNETSetOption),
	TRANS(DNETCreateListener),
	TRANS(DNETAccept),
	TRANS(DNETConnect),
	TRANS(DNETBytesReadable),
	TRANS(DNETRead),
	TRANS(DNETWrite),
	TRANS(DNETReadv),
	TRANS(DNETWritev),
	TRANS(DNETDisconnect),
	TRANS(DNETClose),
	TRANS(DNETNameToAddr),
	TRANS(DNETAddrToName),
};
