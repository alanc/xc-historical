#include "copyright.h"

/* $XConsortium: XFont.c,v 11.31 89/10/08 14:32:31 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/
#define NEED_REPLIES
#include "Xlibint.h"

int _XQueryFont();

XFontStruct *XLoadQueryFont(dpy, name)
   register Display *dpy;
   char *name;
{
    XFontStruct *font_result;
    register long nbytes;
    Font fid;
    xOpenFontReq *req;
    int seqadj = 1;
    int	error_status;

    LockDisplay(dpy);
    GetReq(OpenFont, req);
    nbytes = req->nbytes  = name ? strlen(name) : 0;
    req->fid = fid = XAllocID(dpy);
    req->length += (nbytes+3)>>2;
    Data (dpy, name, nbytes);
#ifdef WORD64
    /* 
     *  If a NoOp is generated, the sequence number will be off
     *  by one, so this temporarily adjusts the sequence number.
     */
    if ((long)dpy->bufptr >> 61) seqadj = 2;
#endif
    dpy->request -= seqadj;
    error_status = _XQueryFont(dpy, fid, &font_result);
    dpy->request += seqadj;
    if (error_status) {
	font_result = (XFontStruct *) NULL;
	if (error_status == 1) {
	    /* if _XQueryFont returned 1, then the OpenFont request got
	       a BadName error.  This means that the following QueryFont
	       request is guaranteed to get a BadFont error, since the id
	       passed to QueryFont wasn't really a valid font id.  To read
	       and discard this second error, we call _XReply again. */
	    xReply reply;
	    (void) _XReply (dpy, &reply, 0, xFalse);
        }
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return font_result;
}

XFreeFont(dpy, fs)
    register Display *dpy;
    XFontStruct *fs;
{ 
    register xResourceReq *req;
    register _XExtension *ext = dpy->ext_procs;

    LockDisplay(dpy);
    while (ext) {		/* call out to any extensions interested */
	if (ext->free_Font != NULL) (*ext->free_Font)(dpy, fs, &ext->codes);
	ext = ext->next;
	}    
    GetResReq (CloseFont, fs->fid, req);
    _XFreeExtData(fs->ext_data);
    if (fs->per_char)
       Xfree ((char *) fs->per_char);
    if (fs->properties)
       Xfree ((char *) fs->properties);
    Xfree ((char *) fs);
    UnlockDisplay(dpy);
    SyncHandle();
}

/*
 * Returns:	0	success
 *		1	protocol error
 *		2	Xlib memory allocation failed
 */
static int _XQueryFont (dpy, fid, xfs)	/* Internal-only entry point */
    register Display *dpy;
    Font fid;
    XFontStruct **xfs;	/* RETURN */
{
    register XFontStruct *fs;
    register long nbytes;
    xQueryFontReply reply;
    register xResourceReq *req;
    register _XExtension *ext;

    GetResReq(QueryFont, fid, req);
    if (!_XReply (dpy, (xReply *) &reply,
       ((SIZEOF(xQueryFontReply) - SIZEOF(xReply)) >> 2), xFalse))
	return 1;
    if (! (fs = (XFontStruct *) Xmalloc (sizeof (XFontStruct))))
	return 2;
    fs->ext_data 		= NULL;
    fs->fid 			= fid;
    fs->direction 		= reply.drawDirection;
    fs->min_char_or_byte2	= reply.minCharOrByte2;
    fs->max_char_or_byte2 	= reply.maxCharOrByte2;
    fs->min_byte1 		= reply.minByte1;
    fs->max_byte1 		= reply.maxByte1;
    fs->default_char 		= reply.defaultChar;
    fs->all_chars_exist 	= reply.allCharsExist;
    fs->ascent 			= cvtINT16toInt (reply.fontAscent);
    fs->descent 		= cvtINT16toInt (reply.fontDescent);
    
#ifdef MUSTCOPY
    {
	xCharInfo *xcip;

	xcip = (xCharInfo *) &reply.minBounds;
	fs->min_bounds.lbearing = cvtINT16toInt(xcip->leftSideBearing);
	fs->min_bounds.rbearing = cvtINT16toInt(xcip->rightSideBearing);
	fs->min_bounds.width = cvtINT16toInt(xcip->characterWidth);
	fs->min_bounds.ascent = cvtINT16toInt(xcip->ascent);
	fs->min_bounds.descent = cvtINT16toInt(xcip->descent);
	fs->min_bounds.attributes = xcip->attributes;

	xcip = (xCharInfo *) &reply.maxBounds;
	fs->max_bounds.lbearing = cvtINT16toInt(xcip->leftSideBearing);
	fs->max_bounds.rbearing =  cvtINT16toInt(xcip->rightSideBearing);
	fs->max_bounds.width =  cvtINT16toInt(xcip->characterWidth);
	fs->max_bounds.ascent =  cvtINT16toInt(xcip->ascent);
	fs->max_bounds.descent =  cvtINT16toInt(xcip->descent);
	fs->max_bounds.attributes = xcip->attributes;
    }
#else
    /* XXX the next two statements won't work if short isn't 16 bits */
    fs->min_bounds = * (XCharStruct *) &reply.minBounds;
    fs->max_bounds = * (XCharStruct *) &reply.maxBounds;
#endif /* MUSTCOPY */

    fs->n_properties = reply.nFontProps;
    /* 
     * if no properties defined for the font, then it is bad
     * font, but shouldn't try to read nothing.
     */
    fs->properties = NULL;
    if (fs->n_properties > 0) {
	    nbytes = reply.nFontProps * sizeof(XFontProp);
	    fs->properties = (XFontProp *) Xmalloc ((unsigned) nbytes);
	    nbytes = reply.nFontProps * SIZEOF(xFontProp);
	    if (! fs->properties) {
		Xfree((char *) fs);
		_XEatData(dpy, (unsigned long)
			  (nbytes + reply.nCharInfos * SIZEOF(xCharInfo)));
		return 2;
	    }
	    _XRead32 (dpy, (char *)fs->properties, nbytes);
    }
    /*
     * If no characters in font, then it is a bad font, but
     * shouldn't try to read nothing.
     */
    /* have to unpack charinfos on some machines (CRAY) */
    fs->per_char = NULL;
    if (reply.nCharInfos > 0){
	nbytes = reply.nCharInfos * sizeof(XCharStruct);
	if (! (fs->per_char = (XCharStruct *) Xmalloc ((unsigned) nbytes))) {
	    if (fs->n_properties) Xfree((char *) fs->n_properties);
	    Xfree((char *) fs);
	    _XEatData(dpy, (unsigned long)
			    (reply.nCharInfos * SIZEOF(xCharInfo)));
	    return 2;
	}
	    
#ifdef MUSTCOPY
	{
	    register XCharStruct *cs = fs->per_char;
	    register int i;

	    for (i = 0; i < reply.nCharInfos; i++, cs++) {
		xCharInfo xcip;

		_XRead(dpy, &xcip, SIZEOF(xCharInfo));
		cs->lbearing = cvtINT16toInt(xcip.leftSideBearing);
		cs->rbearing = cvtINT16toInt(xcip.rightSideBearing);
		cs->width =  cvtINT16toInt(xcip.characterWidth);
		cs->ascent =  cvtINT16toInt(xcip.ascent);
		cs->descent =  cvtINT16toInt(xcip.descent);
		cs->attributes = xcip.attributes;
	    }
	}
#else
	nbytes = reply.nCharInfos * SIZEOF(xCharInfo);
	_XRead16 (dpy, (char *)fs->per_char, nbytes);
#endif
    }

    ext = dpy->ext_procs;
    while (ext) {		/* call out to any extensions interested */
	if (ext->create_Font != NULL) 
		(*ext->create_Font)(dpy, fs, &ext->codes);
	ext = ext->next;
	}    
    *xfs = fs;
    return 0;
}


XFontStruct *XQueryFont (dpy, fid)
    register Display *dpy;
    Font fid;
{
    XFontStruct *font_result;

    LockDisplay(dpy);
    if (_XQueryFont(dpy, fid, &font_result) > 0)
	font_result = (XFontStruct *) NULL;
    UnlockDisplay(dpy);
    SyncHandle();
    return font_result;
}

   
   
