#include <X11/ICE/ICElib.h>
#include <X11/Intrinsic.h>


/*
 * This stuff goes in Xt
 */

Status
XtInitializeICE (appContext)

XtAppContext appContext;

{
    void _XtIceWatchProc ();

    return (IceAddConnectionWatch (_XtIceWatchProc, (IcePointer) appContext));
}


void
_XtIceWatchProc (iceConn, clientData, opening, watchData)

IceConn 	iceConn;
IcePointer	clientData;
Bool		opening;
IcePointer	*watchData;

{
    if (opening)
    {
	XtAppContext appContext = (XtAppContext) clientData;
	void _XtProcessIceMsgProc ();

	*watchData = (IcePointer) XtAppAddInput (
	    appContext,
	    IceConnectionNumber (iceConn),
            (XtPointer) XtInputReadMask,
	    _XtProcessIceMsgProc,
	    (XtPointer) iceConn);
    }
    else
    {
	XtRemoveInput ((XtInputId) *watchData);
    }
}


void
_XtProcessIceMsgProc (clientData, source, id)

XtPointer	clientData;
int 		*source;
XtInputId	*id;

{
    IceConn	iceConn = (IceConn) clientData;

    IceProcessMessage (iceConn, NULL);
}
