/* $XConsortium: technq.c,v 1.1 93/07/19 10:10:59 rws Exp $ */
/**** module technq.c ****/
/****************************************************************************
				NOTICE
                              
This software is being provided by AGE Logic, Inc. and MIT under the
following license.  By obtaining, using and/or copying this software,
you agree that you have read, understood, and will comply with these
terms and conditions:

     Permission to use, copy, modify, distribute and sell this
     software and its documentation for any purpose and without
     fee or royalty and to grant others any or all rights granted
     herein is hereby granted, provided that you agree to comply
     with the following copyright notice and statements, including
     the disclaimer, and that the same appears on all copies and
     derivative works of the software and documentation you make.
     
     "Copyright 1993 by AGE Logic, Inc. and the Massachusetts
     Institute of Technology"
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC AND MIT
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC OR MIT OR LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The names of AGE Logic, Inc. and MIT may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic and MIT.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
****************************************************************************
 	technq.c: Routines to handle technique protocol requests

	Dean Verheiden  AGE Logic, Inc.  April 1993
****************************************************************************/

#define _XIEC_TECHNQ

/*
 *  Include files
 */
#include <stdio.h>
/*
 *  Core X Includes
 */
#include <X.h>
#include <Xproto.h>
/*
 *  XIE Includes
 */
#include <XIE.h>
#include <XIEproto.h>
/*
 *  more X server includes.
 */
#include <misc.h>
#include <extnsionst.h>
#include <dixstruct.h>

/*
 *  Element Specific Includes
 */
#include <corex.h>
#include <macro.h>
#include <memory.h>
#include <technq.h>

/*
 *  Xie protocol proceedures called from the dispatcher
 */
int  ProcQueryTechniques();
int  SProcQueryTechniques();

/*
 *  Functions called by other Xie procedures
 */
CARD16 TechDefault();
Bool TechNeedsParams();
techVecPtr FindTechnique();

/*
 *  Used internally by this module
 */
static int return_length();
static int send_reply();
static int send_technique_replies();

/*------------------------------------------------------------------------
------------------------ QueryTechniques Procedures -----------------------
------------------------------------------------------------------------*/
int ProcQueryTechniques(client)
     ClientPtr client;
{
  xieQueryTechniquesReply rep;
  REQUEST( xieQueryTechniquesReq );
  REQUEST_SIZE_MATCH( xieQueryTechniquesReq );
  
  /*
   * Fill in the reply header
   */
  memset(&rep, 0, sz_xieQueryTechniquesReply);
  rep.type        = X_Reply;
  rep.sequenceNum = client->sequence;

  /* First, figure out how big the ListofTechniqueRecs is going to be */
  if (return_length(stuff->techniqueGroup, &rep.length, &rep.numTechniques)) 
	return(BadValue);

  if( client->swapped ) {      
    /*
     * Swap the reply header fields
     */
    register int n;
    
    swaps(&rep.sequenceNum,n);
    swapl(&rep.length,n);
    swaps(&rep.numTechniques,n);
  }

  WriteToClient(client, sz_xieQueryTechniquesReply, (char *)&rep);

  send_technique_replies(stuff->techniqueGroup,client);

  return(Success);
}                               /* end ProcQueryTechniques */

int SProcQueryTechniques(client)
     ClientPtr client;
{
  register int n;
  REQUEST( xieQueryTechniquesReq );
  swaps(&stuff->length, n);
  REQUEST_SIZE_MATCH( xieQueryTechniquesReq );
  return (ProcQueryTechniques(client));
}                               /* end SProcQueryTechniques */

/*************************************************************************
 Initialization Procedure: Fill in technique name sizes
*************************************************************************/

/* Macro to step though a group array's techniques and determine size */
#define FILL_IN_SIZE(array)						\
{									\
	register int i;							\
	for (i = 0; i < sizeof(array)/sizeof(Technique); i++)		\
		array[i].nameLength =  strlen((char *)(array[i].name));	\
}

int technique_init()
{
	FILL_IN_SIZE(Tcoloralloc);
	FILL_IN_SIZE(Tconstrain);
	FILL_IN_SIZE(Tconvertfromrgb);
	FILL_IN_SIZE(Tconverttorgb);
	FILL_IN_SIZE(Tconvolve);
	FILL_IN_SIZE(Tdecode);
	FILL_IN_SIZE(Tdither);
	FILL_IN_SIZE(Tencode);
	FILL_IN_SIZE(Tgamut);
	FILL_IN_SIZE(Tgeometry);
	FILL_IN_SIZE(Thistogram);
	FILL_IN_SIZE(Twhiteadjust);
}				/* end technique_init */


/*************************************************************************
 Return Length Procedure: from the requested group, calculate size of
			  data to be returned
			  A non-zero return value indicates a Value error
*************************************************************************/

/* Return next highest number that is evenly divisible by 4 */ 
#define QUAD_SIZE(val)	(val + 3 & ~3)

/* Macro to step though a group array's techniques and determine size */
#define	GROUP_SIZE(array)					\
{								\
	register int i;						\
	for (i = 0; i < sizeof(array)/sizeof(Technique); i++) {	\
		*plen += sizeof(xieTypTechniqueRec) + 		\
			QUAD_SIZE(array[i].nameLength);		\
		(*pnt)++;					\
	}							\
}

static int return_length(group, plen, pnt)
xieTypTechniqueGroup	group;
CARD32	*plen;
CARD16	*pnt;
{
	switch	(group) {
	case xieValDefault:
		*pnt = NUM_TECH_DEFAULTS;
		*plen = sizeof(xieTypTechniqueRec) * NUM_TECH_DEFAULTS +
			QUAD_SIZE(Tcoloralloc[0].nameLength) + 
			QUAD_SIZE(Tconvolve[0].nameLength) +
			QUAD_SIZE(Tdither[0].nameLength) +
			QUAD_SIZE(Tgamut[0].nameLength) +
			QUAD_SIZE(Tgeometry[0].nameLength) +
			QUAD_SIZE(Twhiteadjust[0].nameLength);
		break;
	case xieValAll:
		GROUP_SIZE(Tcoloralloc);
		GROUP_SIZE(Tconstrain);
		GROUP_SIZE(Tconvertfromrgb);
		GROUP_SIZE(Tconverttorgb);
		GROUP_SIZE(Tconvolve);
		GROUP_SIZE(Tdecode);
		GROUP_SIZE(Tdither);
		GROUP_SIZE(Tencode);
		GROUP_SIZE(Tgamut);
		GROUP_SIZE(Tgeometry);
		GROUP_SIZE(Thistogram);
		GROUP_SIZE(Twhiteadjust);
		break;
	case xieValColorAlloc:
		GROUP_SIZE(Tcoloralloc);
		break;
	case xieValConstrain:
		GROUP_SIZE(Tconstrain);
		break;
	case xieValConvertFromRGB:
		GROUP_SIZE(Tconvertfromrgb);
		break;
	case xieValConvertToRGB:
		GROUP_SIZE(Tconverttorgb);
		break;
	case xieValConvolve:
		GROUP_SIZE(Tconvolve);
		break;
	case xieValDecode:
		GROUP_SIZE(Tdecode);
		break;
	case xieValDither:
		GROUP_SIZE(Tdither);
		break;
	case xieValEncode:
		GROUP_SIZE(Tencode);
		break;
	case xieValGamut:
		GROUP_SIZE(Tgamut);
		break;
	case xieValGeometry:
		GROUP_SIZE(Tgeometry);
		break;
	case xieValHistogram:
		GROUP_SIZE(Thistogram);
		break;
	case xieValWhiteAdjust:
		GROUP_SIZE(Twhiteadjust);
		break;
	default:
		return(1);	/* Illegal group */
		break;
	}

	/* Scale size to reflect 32 bit words */
	*plen /= 4;

	return(0);
		
}				/* end return_length */


/*************************************************************************
 Send Technique Reply: send technique structures to client, one at a time,
			swapping them first if necessary
*************************************************************************/

/* Macro to step though a group array's techniques and determine size */
#define	SEND_TECHNIQUE(array)					\
{								\
	register int i;						\
	for (i = 0; i < sizeof(array)/sizeof(Technique); 	\
			send_reply(array[i++],client));		\
}

static int send_reply(tech, client)
Technique	tech;
ClientPtr client;
{
	xieTypTechniqueRec rep;

	rep.needsParam = tech.needsParam;
	rep.group = tech.group;
	rep.number = (client->swapped) ? lswaps(tech.number) : tech.number;
	rep.speed = tech.speed;
	rep.nameLength = tech.nameLength;

	/* Send everything except the name */
  	WriteToClient(client, sz_xieTypTechniqueRec, (char *)&rep);
	/* Send the name */
  	WriteToClient(client, tech.nameLength, (char *)tech.name);

}

static int send_technique_replies(group,client)
xieTypTechniqueGroup	group;
ClientPtr client;
{
	switch	(group) {
	case xieValDefault:
		send_reply(Tcoloralloc[0],client);
		send_reply(Tconvolve[0],client);
		send_reply(Tdither[0],client);
		send_reply(Tgamut[0],client);
		send_reply(Tgeometry[0],client);
		send_reply(Twhiteadjust[0],client);
		break;
	case xieValAll:
		SEND_TECHNIQUE(Tcoloralloc);
		SEND_TECHNIQUE(Tconstrain);
		SEND_TECHNIQUE(Tconvertfromrgb);
		SEND_TECHNIQUE(Tconverttorgb);
		SEND_TECHNIQUE(Tconvolve);
		SEND_TECHNIQUE(Tdecode);
		SEND_TECHNIQUE(Tdither);
		SEND_TECHNIQUE(Tencode);
		SEND_TECHNIQUE(Tgamut);
		SEND_TECHNIQUE(Tgeometry);
		SEND_TECHNIQUE(Thistogram);
		SEND_TECHNIQUE(Twhiteadjust);
		break;
	case xieValColorAlloc:
		SEND_TECHNIQUE(Tcoloralloc);
		break;
	case xieValConstrain:
		SEND_TECHNIQUE(Tconstrain);
		break;
	case xieValConvertFromRGB:
		SEND_TECHNIQUE(Tconvertfromrgb);
		break;
	case xieValConvertToRGB:
		SEND_TECHNIQUE(Tconverttorgb);
		break;
	case xieValConvolve:
		SEND_TECHNIQUE(Tconvolve);
		break;
	case xieValDecode:
		SEND_TECHNIQUE(Tdecode);
		break;
	case xieValDither:
		SEND_TECHNIQUE(Tdither);
		break;
	case xieValEncode:
		SEND_TECHNIQUE(Tencode);
		break;
	case xieValGamut:
		SEND_TECHNIQUE(Tgamut);
		break;
	case xieValGeometry:
		SEND_TECHNIQUE(Tgeometry);
		break;
	case xieValHistogram:
		SEND_TECHNIQUE(Thistogram);
		break;
	case xieValWhiteAdjust:
		SEND_TECHNIQUE(Twhiteadjust);
		break;
	default:
		return(1);	/* Illegal group */
		break;
	}

	return(0);
		
}				/* end return_technique_replies */

/*************************************************************************
TechNeedsParams: return boolean indicating if the named technique needs
		 parameters
*************************************************************************/

/* Macro to step though a group array's techniques and determine size */
#define	CHECK_PARM(array)					\
{								\
	register int i;						\
	for (i = 0; i < sizeof(array)/sizeof(Technique); i++) {	\
		if (array[i].number == number)			\
			return (array[i].needsParam);		\
	}							\
	return(0);						\
}

Bool TechNeedsParams(group, number)
xieTypTechniqueGroup	group;
CARD16 number;
{
	switch	(group) {
		case xieValColorAlloc:
			CHECK_PARM(Tcoloralloc);
			break;
		case xieValConstrain:
			CHECK_PARM(Tconstrain);
			break;
		case xieValConvertFromRGB:
			CHECK_PARM(Tconvertfromrgb);
			break;
		case xieValConvertToRGB:
			CHECK_PARM(Tconverttorgb);
			break;
		case xieValConvolve:
			CHECK_PARM(Tconvolve);
			break;
		case xieValDecode:
			CHECK_PARM(Tdecode);
			break;
		case xieValDither:
			CHECK_PARM(Tdither);
			break;
		case xieValEncode:
			CHECK_PARM(Tencode);
			break;
		case xieValGamut:
			CHECK_PARM(Tgamut);
			break;
		case xieValGeometry:
			CHECK_PARM(Tgeometry);
			break;
		case xieValHistogram:
			CHECK_PARM(Thistogram);
			break;
		case xieValWhiteAdjust:
			CHECK_PARM(Twhiteadjust);
			break;
		default:
			return(0);	/* Illegal group */
			break;
	}
}				/* end TechNeedsParams */
/*************************************************************************
TechDefault: return group's default technique, if any
*************************************************************************/
#define	RETURN_DEFAULT(array) (array[0].tech_default ? array[0].number : 0)

CARD16 TechDefault(group)
xieTypTechniqueGroup	group;
{
	switch	(group) {
		case xieValColorAlloc:
			RETURN_DEFAULT(Tcoloralloc);
			break;
		case xieValConstrain:
			RETURN_DEFAULT(Tconstrain);
			break;
		case xieValConvertFromRGB:
			RETURN_DEFAULT(Tconvertfromrgb);
			break;
		case xieValConvertToRGB:
			RETURN_DEFAULT(Tconverttorgb);
			break;
		case xieValConvolve:
			RETURN_DEFAULT(Tconvolve);
			break;
		case xieValDecode:
			RETURN_DEFAULT(Tdecode);
			break;
		case xieValDither:
			RETURN_DEFAULT(Tdither);
			break;
		case xieValEncode:
			RETURN_DEFAULT(Tencode);
			break;
		case xieValGamut:
			RETURN_DEFAULT(Tgamut);
			break;
		case xieValGeometry:
			RETURN_DEFAULT(Tgeometry);
			break;
		case xieValHistogram:
			RETURN_DEFAULT(Thistogram);
			break;
		case xieValWhiteAdjust:
			RETURN_DEFAULT(Twhiteadjust);
			break;
		default:
			return(0);	/* Illegal group */
			break;
	}
}				/* end TechDefault */
/*************************************************************************
FindTechnique: return group's technique entry point structure
*************************************************************************/
/* Macro to step though a group array's techniques and determine size 
   If 0 is specified, make sure a default technique exists
   If non zero, make sure that technique number exists
   If anything fails, return a NULL structure 
*/
#define	CHECK_TECHNIQUE(array)						\
{									\
	register int i;							\
	if (number == 0) { 						\
		if (array[0].tech_default) 				\
			return (&(array[0].tip));			\
		else							\
			return((techVecPtr)NULL);			\
	} else {							\
		for (i = 0; i < sizeof(array)/sizeof(Technique); i++) {	\
			if (array[i].number == number)			\
				return (&(array[i].tip));		\
		}							\
		return((techVecPtr)NULL);				\
	}								\
}

techVecPtr FindTechnique(group,number)
xieTypTechniqueGroup	group;
CARD16	number;
{
	switch	(group) {
		case xieValColorAlloc:
			CHECK_TECHNIQUE(Tcoloralloc);
			break;
		case xieValConstrain:
			CHECK_TECHNIQUE(Tconstrain);
			break;
		case xieValConvertFromRGB:
			CHECK_TECHNIQUE(Tconvertfromrgb);
			break;
		case xieValConvertToRGB:
			CHECK_TECHNIQUE(Tconverttorgb);
			break;
		case xieValConvolve:
			CHECK_TECHNIQUE(Tconvolve);
			break;
		case xieValDecode:
			CHECK_TECHNIQUE(Tdecode);
			break;
		case xieValDither:
			CHECK_TECHNIQUE(Tdither);
			break;
		case xieValEncode:
			CHECK_TECHNIQUE(Tencode);
			break;
		case xieValGamut:
			CHECK_TECHNIQUE(Tgamut);
			break;
		case xieValGeometry:
			CHECK_TECHNIQUE(Tgeometry);
			break;
		case xieValHistogram:
			CHECK_TECHNIQUE(Thistogram);
			break;
		case xieValWhiteAdjust:
			CHECK_TECHNIQUE(Twhiteadjust);
			break;
		default:
			return((techVecPtr)NULL);	/* Illegal group */
			break;
	}
}				/* end FindTechnique */

/*************************************************************************
NoParamCheck: Used as copyfnc for techniques that do not have parameters.
	      Checks to make sure that no parameters have been passed in.
	      A zero (false) return value indicates an error.
*************************************************************************/
static Bool NoParamCheck(flo, rparms, cparms, tsize) 
     floDefPtr flo;
     void  *rparms, *cparms;
     CARD16 tsize;
{
  return(!tsize);
}			/* end NoParamCheck */

/*************************************************************************
NoTechYet: error stub for unimplemented technique routines
*************************************************************************/
static Bool NoTechYet(flo, ped, parm, tech) 
     floDefPtr flo;
     peDefPtr  ped;
     void    *parm;
     void    *tech;
{
  return(FALSE);
}			/* end NoTechYet */

/* end module technq.c */
