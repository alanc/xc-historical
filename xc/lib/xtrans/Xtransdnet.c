
/*
 * This is the DNET implementation of the X Transport service layer
 */

static XtransConnInfo *
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

static XtransConnInfo *
TRANS(DNETOpenCOTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
return NULL;
}

static XtransConnInfo *
TRANS(DNETOpenCLTSClient)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
return NULL;
}

static XtransConnInfo *
TRANS(DNETOpenCLTSServer)(thistrans, protocol, host, port)
Xtransport	*thistrans;
char		*protocol;
char		*host;
char		*port;
{
return NULL;
}

static
TRANS(DNETSetOption)(thistrans, fd, option, arg)
Xtransport	*thistrans;
int		fd;
int		option;
int		arg;
{
}

static
TRANS(DNETCreateListener)(thistrans, fd, port)
Xtransport	*thistrans;
int		fd;
char		*port;
{
}

static XtransConnInfo *
TRANS(DNETAccept)(thistrans, fd)
Xtransport	*thistrans;
int		fd;
{
return NULL;
}

static
TRANS(DNETConnect)(thistrans, fd, host, port)
Xtransport	*thistrans;
int		fd;
char		*host;
char		*port;
{
}

static int
TRANS(DNETBytesReadable)(thistrans, fd, pend)
Xtransport	*thistrans;
int		fd;
BytesReadable_t	*pend;
{
return -1;
}

static int
TRANS(DNETRead)(thistrans, fd, buf, size)
Xtransport	*thistrans;
int		fd;
char		*buf;
int		size;
{
return read(fd,buf,size);
}

static int
TRANS(DNETWrite)(thistrans, fd, buf, size)
Xtransport	*thistrans;
int		fd;
char		*buf;
int		size;
{
return write(fd,buf,size);
}

static int
TRANS(DNETReadv)(thistrans, fd, buf, size)
Xtransport	*thistrans;
int		fd;
struct iovec	*buf;
int		size;
{
return READV(fd,buf,size);
}

static int
TRANS(DNETWritev)(thistrans, fd, buf, size)
Xtransport	*thistrans;
int		fd;
struct iovec	*buf;
int		size;
{
return WRITEV(fd,buf,size);
}

static
TRANS(DNETDisconnect)(thistrans, fd)
Xtransport	*thistrans;
int		fd;
{
}

static
TRANS(DNETClose)(thistrans, fd)
Xtransport	*thistrans;
int		fd;
{
}

static
TRANS(DNETNameToAddr)(thistrans, fd)
Xtransport	*thistrans;
int		fd;
{
}

static
TRANS(DNETAddrToName)(thistrans, fd)
Xtransport	*thistrans;
int		fd;
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
