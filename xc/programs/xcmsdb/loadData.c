/* $XConsortium: loadData.c,v 1.5 91/05/14 15:00:33 dave Exp $ */

/*
 * (c) Copyright 1990 Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		LoadSCCData.c
 *
 *	DESCRIPTION
 *		TekCMS API routine that reads screen data from a file
 *	        and then loads the data on the root window of the screen.
 *		
 *
 *
 */

/*
 *      INCLUDES
 */

#include <X11/Xos.h>
#include <sys/stat.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "Xcmsint.h"
#include "SCCDFile.h"


/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */

#ifdef X_NOT_STDC_ENV
extern char *strtok();
extern char *strchr();
#endif
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
char *calloc();
#endif
#if defined(macII) && !defined(__STDC__)  /* stdlib.h fails to define these */
char *calloc();
#endif /* macII */

/* extern int LINEAR_RGB_FreeSCCData(); /* /* XXX Xlib internal */

/*
 *      LOCAL TYPEDEFS
 *              typedefs local to this package (for use with local vars).
 *
 */

typedef struct _DefineEntry {
    char	*pString;
    int		define;
} DefineEntry;


/*
 *      LOCAL VARIABLES
 */
static int linenum = 0;

static DefineEntry KeyTbl[] = {
    SC_BEGIN_KEYWORD,		SC_BEGIN,
    SC_END_KEYWORD,		SC_END,
    COMMENT_KEYWORD,		COMMENT,
    NAME_KEYWORD,		NAME,
    MODEL_KEYWORD,		MODEL,
    PART_NUMBER_KEYWORD,	PART_NUMBER,
    SERIAL_NUMBER_KEYWORD,	SERIAL_NUMBER,
    REVISION_KEYWORD,		REVISION,
    SCREEN_CLASS_KEYWORD,	SCREEN_CLASS,
    COLORIMETRIC_BEGIN_KEYWORD,	COLORIMETRIC_BEGIN,
    COLORIMETRIC_END_KEYWORD,	COLORIMETRIC_END,
    XYZTORGBMAT_BEGIN_KEYWORD,	XYZTORGBMAT_BEGIN,
    XYZTORGBMAT_END_KEYWORD,	XYZTORGBMAT_END,
    WHITEPT_XYZ_BEGIN_KEYWORD,	WHITEPT_XYZ_BEGIN,
    WHITEPT_XYZ_END_KEYWORD,	WHITEPT_XYZ_END,
    RGBTOXYZMAT_BEGIN_KEYWORD,	RGBTOXYZMAT_BEGIN,
    RGBTOXYZMAT_END_KEYWORD,	RGBTOXYZMAT_END,
    IPROFILE_BEGIN_KEYWORD,	IPROFILE_BEGIN,
    IPROFILE_END_KEYWORD,	IPROFILE_END,
    ITBL_BEGIN_KEYWORD,		ITBL_BEGIN,
    ITBL_END_KEYWORD,		ITBL_END,
    "",				-1
};

static DefineEntry ScrnClassTbl[] = {
    VIDEO_RGB_KEYWORD,		VIDEO_RGB,
#ifdef GRAY
    VIDEO_GRAY_KEYWORD,		VIDEO_GRAY,
#endif /* GRAY */
    "",				-1
};


/************************************************************************
 *									*
 *			 PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		StrToDefine - convert a string to a define
 *
 *	SYNOPSIS
 */
static int
StrToDefine(pde,pstring)
    DefineEntry	pde[];	/* IN: table of X string-define pairs		*/
			/*     last entry must contain pair "", 0	*/
    char *pstring;	/* IN: string to be looked up in that table	*/
/*
 *	DESCRIPTION
 *		Converts a string to an integer define.
 *
 *		Looks up the string in the table and returns the integer
 *		associated with the string.
 *
 *		Later may need similar function for unsigned long define.
 *
 *
 *
 *	RETURNS
 *		The int equivalent of the defined string.
 *		-1 if the string is not found in table
 *
 */
{
    while( strcmp(pde->pString,"") != 0 ){
	if( strcmp(pde->pString,pstring) == 0){
	    return(pde->define);
	}
	pde++;
    }
    return(-1);
}

/*
 *	NAME
 *		DefineToStr
 *
 *	SYNOPSIS
 */
static char *
DefineToStr(pde,id)
    DefineEntry	pde[];	/* IN: table of X string-define pairs		*/
			/*     last entry must contain pair "", 0	*/
    int		id;	/* IN: id to be looked up in that table	*/
/*
 *	DESCRIPTION
 *		Converts a string to an integer define.
 *
 *		Looks up the string in the table and returns the integer
 *		associated with the string.
 *
 *		Later may need similar function for unsigned long define.
 *
 *
 *
 *	RETURNS
 *		The int equivalent of the defined string.
 *		-1 if the string is not found in table
 *
 */
{
    while(pde->define != -1) {
	if (pde->define == id) {
	    return(pde->pString);
	}
	pde++;
    }
    return(NULL);
}

/*
 *	NAME
 *		SCKeyOf - convert keyword into key ID
 *
 *	SYNOPSIS
 */
static int
SCKeyOf(string)
    char *string;
/*
 *	DESCRIPTION
 *		Converts a string to an integer define.
 *
 *		Looks up the string in the table and returns the integer
 *		associated with the string.
 *
 *		Later may need similar function for unsigned long define.
 *
 *
 *
 *	RETURNS
 *		The int equivalent of the defined string.
 *		-1 if the string is not found in table
 *
 */
{
    return(StrToDefine(KeyTbl, string));
}


/*
 *	NAME
 *		SCScrnClassOf - convert screen class string into class ID
 *
 *	SYNOPSIS
 */
static int
SCScrnClassOf(string)
    char *string;
/*
 *	DESCRIPTION
 *		Converts a string to an integer define.
 *
 *		Looks up the string in the table and returns the integer
 *		associated with the string.
 *
 *		Later may need similar function for unsigned long define.
 *
 *
 *
 *	RETURNS
 *		The int equivalent of the defined string.
 *		-1 if the string is not found in table
 *
 */
{
    return(StrToDefine(ScrnClassTbl, string));
}


/*
 *	NAME
 *		SCScrnClassStringOf - convert screen class id into class string
 *
 *	SYNOPSIS
 */
static char *
SCScrnClassStringOf(id)
    int id;
/*
 *	DESCRIPTION
 *		Converts a id to astring
 *
 *	RETURNS
 *		Pointer to string if found; otherwise NULL.
 *
 */
{
    return(DefineToStr(ScrnClassTbl, id));
}

/* close the stream and return any memory allocated. */
static void 
closeS(stream, pScreenData) 
    FILE *stream;
    LINEAR_RGB_SCCData *pScreenData;
{
    if (stream) {
        fclose (stream);
    }
    /* (void) LINEAR_RGB_FreeSCCData(pScreenData); */ /* Xlib internal */
}

/*
 *  Get a line of text from the stream.
 */
static char *
nextline(buf, maxch, stream)
    char *buf;
    int	maxch;
    FILE *stream;
{
    linenum++;
    return (fgets(buf, maxch, stream));
}


static int
ProcessColorimetric(stream, pScreenData, filename, VisualFlag)
    FILE *stream;
    LINEAR_RGB_SCCData *pScreenData;
    char *filename;
    int VisualFlag;
{
    char buf[BUFSIZ];
    char keyword[BUFSIZ];
    char token[BUFSIZ], *ptoken;
    int  ntok;
    int	 state = 0;
		 /* 0 -- looking for XYZTORGBMAT */
		 /* 1 -- processing data for XYZTORGBMAT */
		 /* 2 -- completed processing XYZTORGBMAT and */
		 /*      looking for RGBTOXYZMat */
		 /* 3 -- processing data fro RGBTOXYZMat */
		 /* 4 -- completed processing RGBTOXYZMat */
		 /* Note: the order of the matrices is important. */
		 /*       The XYZTORGBMAT must preceed the RGBTOXYZMat. */
    int	 count;
    XcmsFloat *pElement;
    float fNum;

    while ((nextline(buf, BUFSIZ, stream)) != NULL) {
	if ((ntok = sscanf(buf, "%s %s", keyword, token)) > 0) {
	    switch (SCKeyOf(keyword)) {
	      case XYZTORGBMAT_BEGIN :
		if (VisualFlag != VIDEO_RGB) {
		  fprintf(stderr, 
			 "Keyword XYZTORGBMAT_BEGIN mismatch for visual %s.\n",
			  SCScrnClassStringOf(VisualFlag));
		  return (0);
		}
		if (state != 0) {
		  fprintf(stderr,
			  "Extraneous keyword %s in file %s.\n", 
			  keyword, filename);
		  return (0);
		}
		state = 1;
		count = 0;
		pElement = (XcmsFloat *) pScreenData->XYZtoRGBmatrix;
		break;
	      case XYZTORGBMAT_END :
		if (VisualFlag != VIDEO_RGB) {
		  fprintf(stderr, 
			  "Keyword XYZTORGBMAT_END mismatch for visual %s.\n",
			  SCScrnClassStringOf(VisualFlag));
		  return (0);
		}
		if ((state != 1) || (count != 9)) {
		  fprintf(stderr,
			  "Incomplete A matrix in file %s -- Premature %s\n", 
			  keyword, filename);
		  return (0);
		}
		state = 2;
		break;
	      case RGBTOXYZMAT_BEGIN :
		if (VisualFlag != VIDEO_RGB) {
		  fprintf(stderr,
			 "Keyword RGBTOXYZMAT_BEGIN mismatch for visual %s.\n",
			  SCScrnClassStringOf(VisualFlag));
		  return (0);
		}
		if (state != 0 && state != 2) {
		    fprintf(stderr, "Extraneous keyword %s in file %s.\n", 
			    keyword, filename);
		    return (0);
		}
		state = 3;
		count = 0;
		pElement = (XcmsFloat *) pScreenData->RGBtoXYZmatrix;
		break;
	      case RGBTOXYZMAT_END :
		if (VisualFlag != VIDEO_RGB) {
		    fprintf(stderr, 
			   "Keyword RGBTOXYZMAT_END mismatch for visual %s.\n",
			    SCScrnClassStringOf(VisualFlag));
		    return (0);
		}
		if ((state != 3) || (count != 9)) {
		    fprintf(stderr, 
			   "Incomplete A matrix in file %s -- Premature %s\n", 
			    keyword, filename);
		    return (0);
		}
		state = 4;
		break;
#ifdef GRAY
	      case WHITEPT_XYZ_BEGIN :
		if (VisualFlag != VIDEO_GRAY) {
		    fprintf(stderr,
			 "Keyword WHITEPT_XYZ_BEGIN mismatch for visual %s.\n",
			    SCScrnClassStringOf(VisualFlag));
		    return (0);
		}
		if (state != 0) {
		  fprintf(stderr,
			  "Extraneous keyword %s in file %s.\n", 
			  keyword, filename);
		  return (0);
		}
		state = 1;
		count = 0;
		pElement = (XcmsFloat *) pScreenData->XYZtoRGBmatrix;
		break;
	      case WHITEPT_XYZ_END :
		if (VisualFlag != VIDEO_GRAY) {
		    fprintf(stderr,
			   "Keyword WHITEPT_XYZ_END mismatch for visual %s.\n",
			    SCScrnClassStringOf(VisualFlag));
		    return (0);
		}
		if ((state != 1) || (count != 3)) {
		    fprintf(stderr,
			"Incomplete white point in file %s -- Premature %s\n", 
			    keyword, filename);
		    return (0);
		}
		state = 4;
		break;
#endif /* GRAY */
	      case DATA :
		for (ptoken = strtok(buf, DATA_DELIMS); ptoken != NULL;
			ptoken = strtok(NULL, DATA_DELIMS)) {
		    /*******************************************************
		     *  This magic about fNum converted to XcmsFloat is 
		     *  because our stupid sscanf does not understand that 
		     *  %f also applies to doubles.
		     *******************************************************/
		    if (sscanf(ptoken, "%f", &fNum) != 1) {
			if (VisualFlag == VIDEO_RGB) {
			    fprintf(stderr,
				    "Invalid A Matrix value %s in file %s.", 
				    ptoken, filename);
			} else {
			    fprintf(stderr,
				    "Invalid CIEXYZ value %s in file %s.\n",
				    ptoken, filename);
			}
			return (0);
		    }
		    *pElement++ = (XcmsFloat) fNum;
		    if (VisualFlag == VIDEO_RGB) {
			if (++count > 9) {
			    fprintf(stderr,
				   "Extra A Matrix value %s in filename %s\n", 
				    ptoken, filename);
			    return (0);
			}
		    } else {
			if (++count > 3) {
			    fprintf(stderr,
				    "Extra CIEXYZ value %s in file %s.\n",
				    ptoken, filename);
			    return (0);
			  }
		    }
		}
		break;
	      case COLORIMETRIC_BEGIN :
		fprintf(stderr, 
			"Extraneous keyword %s in file %s.\n", 
			keyword, filename);
		return (0);
/* NOTREACHED */break;	
	      case COLORIMETRIC_END :
		if (state != 4) {
		    fprintf(stderr,
		   "Incomplete Colorimetric data in file %s -- Premature %s\n",
			    filename, keyword);
		    return (0);
		}
		return (1);
	      case COMMENT :
		/* Currently, do nothing. */
		break;
	      default :
		fprintf(stderr,
			"Unexpected keyword %s in file %s\n",
			keyword, filename);
		return (0);
/* NOTREACHED */break;		
	    }
	} else if (ntok < 0) {
	    /* mismatch */
	    fprintf(stderr, "Unrecognized keyword\n");
	    return (0);
/* NOTREACHED */break;		
	}
    }
    return (0);
}

static int
ProcessIProfile(stream, pScreenData, tableType, nTables, filename)
    FILE *stream;
    LINEAR_RGB_SCCData *pScreenData;
    int  tableType, nTables;
    char *filename;
{
    char buf[BUFSIZ];
    char keyword[BUFSIZ];
    char tableStr[BUFSIZ], *ptoken;
    int  ntok;
    int  size;
    int	 state = 0;
	 /************************************************
	  * 0 -- Looking for Intensity Table(s)          *
	  * 1 -- Processing Intensity Table(s)           *
          ************************************************/
    int	 nTbl = 0;
    int	 count = 0;
    IntensityRec *pIRec = NULL;
    float fNum;
    int dNum;

    while ((nextline(buf, BUFSIZ, stream)) != NULL) {
	if ((ntok = sscanf(buf, "%s %s %d", keyword, tableStr, &size)) > 0) {
	    switch (SCKeyOf(keyword)) {
	      case ITBL_BEGIN :
		if (state != 0) {
		    fprintf(stderr,"File %s has an unexpected keyword %s\n", 
			   filename, keyword);
		    return (0);
		}
		if (size < 0) {
		    fprintf(stderr,
			    "File %s has count %d < 0 for Intensity Table.\n",
			      filename, size);
		    return (0);
		}
		if (strcmp(tableStr, "GREEN") == 0) {
		    if (nTables != 3) {
			fprintf(stderr,"File %s incorrect number of tables\n", 
			filename);
			return (0);
		    }
		    if (pScreenData->pGreenTbl->pBase != NULL) {
			fprintf(stderr,
			     "File %s has multiple GREEN Intensity Profiles\n",
			           filename);
			return (0);
		    }
		    pScreenData->pGreenTbl->nEntries = size;
		    pScreenData->pGreenTbl->pBase =
			 (IntensityRec *) calloc (size, sizeof(IntensityRec));
		    if (!pScreenData->pGreenTbl->pBase) {
			fprintf(stderr,
		     "Unable to allocate space for GREEN Intensity Profile\n");
			return (0);
		    }
		    pIRec = pScreenData->pGreenTbl->pBase;
		} else if (strcmp(tableStr, "BLUE") == 0) {
		    if (nTables != 3) {
			fprintf(stderr,
				"File %s incorrect number of tables\n",
				filename);
			return (0);
		    }
		    if (pScreenData->pBlueTbl->pBase != NULL) {
			fprintf(stderr,
			      "File %s has multiple BLUE Intensity Profiles\n",
			           filename);
			return (0);
		    }
		    pScreenData->pBlueTbl->nEntries = size;
		    pScreenData->pBlueTbl->pBase =
			 (IntensityRec *) calloc (size, sizeof(IntensityRec));
		    if (!pScreenData->pBlueTbl->pBase) {
			fprintf(stderr,
		      "Unable to allocate space for BLUE Intensity Profile\n");
			return (0);
		    }
		    pIRec = pScreenData->pBlueTbl->pBase;
		} else {
		    if (!strcmp(tableStr, "RGB") && nTables != 1) {
			fprintf(stderr,"File %s has multiple RGB Intensity Tables",
			      filename);
			return (0);
		    }
		    if (pScreenData->pRedTbl->pBase != NULL) {
			fprintf(stderr,
		  "File %s has multiple RED or GRAY or RGB Intensity Tables\n",
				   filename);
			return (0);
		    }
		    pScreenData->pRedTbl->nEntries = size;
		    pScreenData->pRedTbl->pBase = 
			 (IntensityRec *) calloc (size, sizeof(IntensityRec));
		    if (!pScreenData->pRedTbl->pBase) {
			fprintf(stderr,
			     "Unable to allocate space for intensity table\n");
			return (0);
		    }
		    pIRec = pScreenData->pRedTbl->pBase;
		}
		state = 1;
		count = 0;
		break;
	      case ITBL_END :
		if ((state != 1) || (count != size)) {
		    fprintf(stderr,
		    "File %s has incomplete Intensity Table -- Premature %s\n",
			  filename, keyword);
		    return (0);
		}
		nTbl++;
		state = 0;
		break;
	      case DATA :
		for (ptoken = strtok(buf, DATA_DELIMS); ptoken != NULL;
			ptoken = strtok(NULL, DATA_DELIMS)) {
		    /********************************************************
		     * Note: tableType should only be 0 or 1 at this point. 
		     *       0 indicates value and intensity stored.
		     *       1 indicates only intensity stored. 
		     ********************************************************/
		    if (tableType) {
			if (sscanf(ptoken, "%f", &fNum) != 1) {
			    fprintf(stderr,
			   "File %s has invalid Intensity Profile value %s\n", 
				  filename, ptoken);
			    return (0);
			}
			/* With tableType 1 only store the intensity. */
			pIRec->intensity = (XcmsFloat) fNum;
			pIRec++;
		    } else {
#ifndef ANSI_C		/* Note ansi C can handle 0x preceeding hex number */
			if (*ptoken == '0' && *(ptoken+1) == 'x') 
			    ptoken += 2;
#endif /* ANSI_C */
			/****************************************************
		         *  This magic about dNum is required because of 
			 *  a stupid compiler that does not know that %x
			 *  also applies to shorts.
			 ****************************************************/
			if (sscanf(ptoken, "%x", &dNum) != 1) {
			    fprintf(stderr,
			    "File %s has invalid Intensity Profile value %s\n",
				  filename, ptoken);
			    return (0);
			}
			if ((ptoken = strtok(NULL, DATA_DELIMS)) == NULL) {
			    fprintf(stderr,
				  "File %s is missing Intensity Profile value",
				  filename);
			    return (0);
			}
			/****************************************************
		         *  This magic about fNum is required because of 
			 *  a stupid compiler that does not know that %f
			 *  also applies to doubles.
			 ****************************************************/
			if (sscanf(ptoken, "%f", &fNum) != 1) {
			    fprintf(stderr,
			"File %s has invalid Intensity Profile intensity %s\n",
				  filename, ptoken);
			    return (0);
			}
			/* With tableType 0 only store both value & intensity*/
			pIRec->value = (unsigned short) dNum;
			pIRec->intensity = (XcmsFloat) fNum;
			pIRec++;
		    }
		    if (++count > size) {
			fprintf(stderr,
				"File %s has extra Intensity value %s\n",
				filename, ptoken);
			return (0);
		    }
		}
		break;
	      case IPROFILE_BEGIN :
		fprintf(stderr,"File %s has extraneous keyword %s\n", 
			  filename, keyword);
		return (0);
/* NOTREACHED */break;
	      case IPROFILE_END :
		if ((state != 0) || (nTbl != nTables)) {
		    fprintf(stderr,
	     "File %s has incomplete Intensity Profile data -- Premature %s\n",
			   filename, keyword);
		    return (0);
		}
		return (1);
	      case COMMENT :
		/* ignore line */
		break;
	      default :
		fprintf(stderr,"File %s has unexpected keyword %s\n",
		      filename, keyword);
		return (0);
/* NOTREACHED */break;
	    }
	} else if (ntok < 0) {
	    /* mismatch */
	    fprintf(stderr,"Unrecognized keyword");
	    return (0);
/* NOTREACHED */break;
	}
    }
    return (0);
}

static int
LoadDataRGB(pDpy, root, tableType, nTables, pScreenData)
    Display *pDpy;
    Window root;
    int tableType, nTables;
    LINEAR_RGB_SCCData *pScreenData;
{
    unsigned char *ret_prop;
    int  count;
    int  nLevels;
    int  IntArray[18], *pInt;
    int  *pArray = (int *)NULL;
    int  ret_format;
    unsigned long ret_len, ret_after;
    Atom MatricesAtom, CorrectAtom, ret_atom;
    IntensityRec *pIRec = NULL;
    XcmsFloat *pValue;
    int	total;

#ifdef PDEBUG
    printf ("XYZtoRGB Matrix values:\n");
    printf ("       %f %f %f\n       %f %f %f\n       %f %f %f\n", 
			    pScreenData->XYZtoRGBmatrix[0][0],
			    pScreenData->XYZtoRGBmatrix[0][1],
			    pScreenData->XYZtoRGBmatrix[0][2],
			    pScreenData->XYZtoRGBmatrix[1][0],
			    pScreenData->XYZtoRGBmatrix[1][1],
			    pScreenData->XYZtoRGBmatrix[1][2],
			    pScreenData->XYZtoRGBmatrix[2][0],
			    pScreenData->XYZtoRGBmatrix[2][1],
			    pScreenData->XYZtoRGBmatrix[2][2]);
    printf ("RGBtoXYZ Matrix values:\n");
    printf ("       %f %f %f\n       %f %f %f\n       %f %f %f\n", 
			    pScreenData->RGBtoXYZmatrix[0][0],
			    pScreenData->RGBtoXYZmatrix[0][1],
			    pScreenData->RGBtoXYZmatrix[0][2],
			    pScreenData->RGBtoXYZmatrix[1][0],
			    pScreenData->RGBtoXYZmatrix[1][1],
			    pScreenData->RGBtoXYZmatrix[1][2],
			    pScreenData->RGBtoXYZmatrix[2][0],
			    pScreenData->RGBtoXYZmatrix[2][1],
			    pScreenData->RGBtoXYZmatrix[2][2]);
    if (tableType != CORR_TYPE_NONE) {
	printf ("Intensity-RGB value conversion type and number of tables:\n");
	printf ("       type = %d, number of tables = %d\n", 
		tableType, nTables));
    } else {
	printf ("Intensity-RGB Conversion Not Specified");
	return (1);
    }
/* do not use this debuggin unless you want to see the world go by */
#ifdef ALLDEBUG

    if (!tableType && nTables != 3) {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	printf ("Intensity Table  RGB  %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
    } else if (tableType && nTables != 3) {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	printf ("Intensity Table  RGB  %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t%f\n", pIRec->intensity);
	}
    } else if (!tableType) {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	printf ("Intensity Table  RED    %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
	nLevels = pScreenData->pGreenTbl->nEntries;
	pIRec = pScreenData->pGreenTbl->pBase;
	printf ("Intensity Table  GREEN  %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
	nLevels = pScreenData->pBlueTbl->nEntries;
	pIRec = pScreenData->pBlueTbl->pBase;
	printf ("Intensity Table  BLUE   %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
    } else {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	printf ("Intensity Table  RED    %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t%f\n", pIRec->intensity);
	}
	nLevels = pScreenData->pGreenTbl->nEntries;
	pIRec = pScreenData->pGreenTbl->pBase;
	printf ("Intensity Table  GREEN  %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t%f\n", pIRec->intensity);
	}
	nLevels = pScreenData->pBlueTbl->nEntries;
	pIRec = pScreenData->pBlueTbl->pBase;
	printf ("Intensity Table  BLUE   %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t%f\n", pIRec->intensity);
	}
    }
#endif /* ALLDEBUG */
#endif /* PDEBUG */

    /*
     * Store the XDCCC_LINEAR_RGB_MATRICES
     */
    pInt = IntArray;
    pValue = (XcmsFloat *)pScreenData->XYZtoRGBmatrix;
    for (count = 0; count < 9; count++) {
	*pInt++ = (int) (*pValue++ * (XcmsFloat) XDCCC_NUMBER);
    }
    pValue = (XcmsFloat *)pScreenData->RGBtoXYZmatrix;
    for (count = 0; count < 9; count++) {
	*pInt++ = (int) (*pValue++ * (XcmsFloat) XDCCC_NUMBER);
    }
    MatricesAtom = XInternAtom (pDpy, XDCCC_MATRIX_ATOM_NAME, False);
    XChangeProperty (pDpy, root, MatricesAtom, XA_INTEGER, 32, 
		     PropModeReplace, (unsigned char *)IntArray, 18);
#ifdef DEBUG
    XSync (pDpy, 0);
#endif /* DEBUG */

    /*
     * Now store the XDCCC_LINEAR_RGB_CORRECTION
     */
    CorrectAtom = XInternAtom (pDpy, XDCCC_CORRECT_ATOM_NAME, False);

    switch (tableType) {
      case CORR_TYPE_NONE :
	XGetWindowProperty (pDpy, root, CorrectAtom, 
			    0, 5, False, XA_INTEGER, 
			    &ret_atom, &ret_format, &ret_len, &ret_after,
			    &ret_prop);
	if (ret_format != 0) {
	    XDeleteProperty (pDpy, root, CorrectAtom);
	    XFree ((char *)ret_prop);
	}
	break;
      case CORR_TYPE_0 :
	if (nTables == 1) {
	    if ((nLevels = pScreenData->pRedTbl->nEntries) < 0) {
		fprintf(stderr,"Illegal number of entries in table\n");
		return (0);
	    }
	    total = 3 + (nLevels * 2);
	    pArray = (int *) calloc (3 + (nLevels * 2), sizeof (int));
	    if ((pInt = pArray) == NULL) {
		fprintf(stderr,"Unable allocate array of ints\n");
		return (0);
	    }
	    *pInt++ = CORR_TYPE_0;		/* type = 0 */
	    *pInt++ = 1;		/* number of tables = 1 */
	    pIRec = pScreenData->pRedTbl->pBase;
	    *pInt++ = count = nLevels;	/* number of pairs in this table */
	    for (; count; count--, pIRec++) {
		*pInt++ =(int) (pIRec->value);
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }
	    XChangeProperty (pDpy, root, CorrectAtom, XA_INTEGER, 32, 
			     PropModeReplace, (unsigned char *)pArray, total);
	} else {
	    total = 2 +	/* type and number of tables */
		    (1 + pScreenData->pRedTbl->nEntries * 2) +
		    (1 + pScreenData->pGreenTbl->nEntries * 2) +
		    (1 + pScreenData->pBlueTbl->nEntries * 2);
	    pArray = (int *) calloc(total, sizeof(int));
	    if ((pInt = pArray) == NULL) {
		fprintf(stderr,"Unable allocate array of ints");
		return (0);
	    }

	    *pInt++ = CORR_TYPE_0;		/* type = 0 */
	    *pInt++ = 3;		/* number of tables = 3 */

	    pIRec = pScreenData->pRedTbl->pBase;
	    *pInt++ = count = pScreenData->pRedTbl->nEntries;
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->value);
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }

	    pIRec = pScreenData->pGreenTbl->pBase;
	    *pInt++ = count = pScreenData->pGreenTbl->nEntries;
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->value);
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }

	    pIRec = pScreenData->pBlueTbl->pBase;
	    *pInt++ = count = pScreenData->pBlueTbl->nEntries;
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->value);
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }
	    XChangeProperty (pDpy, root, CorrectAtom, XA_INTEGER, 32, 
			    PropModeReplace, (unsigned char *)pArray, total);
	}
	break;
      case CORR_TYPE_1 :
	if (nTables == 1) {
	    if ((nLevels = pScreenData->pRedTbl->nEntries) < 0) {
		fprintf(stderr,"Illegal number of entries in table\n");
		return (0);
	    }
	    total = 3 + nLevels;
	    pArray = (int *) calloc (total, sizeof (int));
	    if ((pInt = pArray) == NULL) {
		fprintf(stderr,"Unable allocate array of ints\n");
		return (0);
	    }
	    *pInt++ = CORR_TYPE_1;	/* type = 1 */
	    *pInt++ = 1;		/* number of tables = 1 */
	    pIRec = pScreenData->pRedTbl->pBase;
	    *pInt++ = count = nLevels;	/* number of pairs in this table */
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }
	    XChangeProperty (pDpy, root, CorrectAtom, XA_INTEGER, 32, 
			     PropModeReplace, (unsigned char *)pArray, total);
	} else {
	    total = 2 +	/* slots for type and number of tables */
		    (1 + pScreenData->pRedTbl->nEntries) +
		    (1 + pScreenData->pGreenTbl->nEntries) +
		    (1 + pScreenData->pBlueTbl->nEntries);
	    pArray = (int *) calloc(total, sizeof(int));
	    if ((pInt = pArray) == NULL) {
		fprintf(stderr,"Unable allocate array of ints");
		return (0);
	    }
	    *pInt++ = CORR_TYPE_1;	/* type = 1 */
	    *pInt++ = 3;		/* number of tables = 3 */

	    pIRec = pScreenData->pRedTbl->pBase;
	    *pInt++ = count = pScreenData->pRedTbl->nEntries;
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }

	    pIRec = pScreenData->pGreenTbl->pBase;
	    *pInt++ = count = pScreenData->pGreenTbl->nEntries;
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }

	    pIRec = pScreenData->pBlueTbl->pBase;
	    *pInt++ = count = pScreenData->pBlueTbl->nEntries;
	    for (; count; count--, pIRec++) {
		*pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	    }

	    XChangeProperty (pDpy, root, CorrectAtom, XA_INTEGER, 32, 
		    PropModeReplace, (unsigned char *)pArray, total);
	}
	break;
      default :
	return (0);
    }

#ifdef DEBUG
    XSync (pDpy, 0);
#endif /* DEBUG */

    if (pArray) 
	free (pArray);
    return (1);
}

#ifdef GRAY

static int
LoadDataGray(pDpy, root, tableType, pScreenData)
    Display *pDpy;
    Window root;
    LINEAR_RGB_SCCData *pScreenData;
{
    unsigned char *ret_prop;
    int  count;
    int  nLevels;
    int  IntArray[18], *pInt, *pArray;
    int  ret_format;
    unsigned long ret_len, ret_after;
    Atom MatricesAtom, CorrectAtom, ret_atom;
    IntensityRec *pIRec = NULL;
    XcmsFloat *pValue;

#ifdef DEBUG
    printf ("WhitePt XYZ values:\n");
    printf ("       %f %f %f\n", 
			    pScreenData->XYZtoRGBmatrix[0][0],
			    pScreenData->XYZtoRGBmatrix[0][1],
			    pScreenData->XYZtoRGBmatrix[0][2]);
    if (tableType != CORR_TYPE_NONE) {
	printf ("Intensity-RGB value conversion type:\n");
	printf ("       type = %d\n", tableType);
    } else {
	printf ("Intensity-Gray Conversion Not Specified\n");
	return (1);
    }
    /* do not use this debuggin unless you want to see the world go by */

    if (!tableType) {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	printf ("Intensity Table  GRAY  %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t0x%4x\t%f\n", pIRec->value, pIRec->intensity);
	}
    } else {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	printf ("Intensity Table  GRAY  %d\n", nLevels);
	for (count = 0; count < nLevels; count++, pIRec++) {
	    printf ("\t%f\n", pIRec->intensity);
	}
    }
#endif /* DEBUG */

    /* Now store the XDCCC_SCREENWHITEPT */
    pInt = IntArray;
    pValue = (XcmsFloat *)pScreenData->XYZtoRGBmatrix;
    for (count = 0; count < 3; count++) {
	*pInt++ = (int) (*pValue++ * (XcmsFloat) XDCCC_NUMBER);
    }
    MatricesAtom = XInternAtom (pDpy,XDCCC_SCREENWHITEPT_ATOM_NAME,False);
    XChangeProperty (pDpy, root, MatricesAtom, XA_INTEGER, 32, 
		     PropModeReplace, (unsigned char *)IntArray, 3);
#ifdef DEBUG
    XSync (pDpy, 0);
#endif /* DEBUG */

    /* Now store the XDCCC_GRAY_CORRECTION */
    CorrectAtom = XInternAtom (pDpy, XDCCC_GRAY_CORRECT_ATOM_NAME, False);

    if (tableType == CORR_TYPE_NONE) {
	XGetWindowProperty (pDpy, root, CorrectAtom, 
			    0, 5, False, XA_INTEGER, 
			    &ret_atom, &ret_format, &ret_len, &ret_after,
			    &ret_prop);
	if (ret_format != 0) {
	    XDeleteProperty (pDpy, root, CorrectAtom);
	    XFree ((char *)ret_prop);
	}
	return (1);
    }
    if (!tableType) {
	nLevels = pScreenData->pRedTbl->nEntries;
	if (nLevels < 0) {
	    fprintf(stderr,"Illegal number of entries in table\n");
	    return (0);
	}
	pIRec = pScreenData->pRedTbl->pBase;
	pArray = (int *) calloc (3 + (nLevels * 2), sizeof (int));
	if ((pInt = pArray) == NULL) {
	    fprintf(stderr,"Unable allocate array of ints\n");
	    return (0);
	}
	*pInt++ = 0;			/* type = 0 */
	*pInt++ = nLevels;		/* number of pairs in this table */
	for (count = 0; count < nLevels; count++, pIRec++) {
	    *pInt++ =(int) (pIRec->value);
	    *pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	}
	XChangeProperty (pDpy, root, CorrectAtom, XA_INTEGER, 32, 
			 PropModeReplace, (unsigned char *)pArray,
			 3 + (nLevels * 2));
    } else {
	nLevels = pScreenData->pRedTbl->nEntries;
	pIRec = pScreenData->pRedTbl->pBase;
	pArray = (int *) calloc (3 + nLevels, sizeof (int));
	if ((pInt = pArray) == NULL) {
	    fprintf(stderr,"Unable allocate array of ints\n");
	    return (0);
	}
	*pInt++ = 1;			/* type = 1 */
	*pInt++ = nLevels;		/* number of pairs in this table */
	for (count = 0; count < nLevels; count++, pIRec++) {
	    *pInt++ = (int) (pIRec->intensity * (XcmsFloat) XDCCC_NUMBER);
	}
	XChangeProperty (pDpy, root, CorrectAtom, XA_INTEGER, 32, 
			 PropModeReplace, (unsigned char *)pArray,
			 3 + nLevels);
    }
#ifdef DEBUG
    XSync (pDpy, 0);
#endif /* DEBUG */
    if (pArray) 
	free (pArray);
    return (1);
}
#endif /* GRAY */


/************************************************************************
 *									*
 *			 PUBLIC ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XLoadSCCData - Read and store the screen data
 *
 *	SYNOPSIS
 */
int
LoadSCCData(pDpy, screenNumber, filename)
    Display *pDpy;
    int screenNumber;
    char *filename;

/*
 *	DESCRIPTION
 *		Using the X Device Color Characterization Convention (XDCCC)
 *		read the screen data and store it on the root window of the
 *		screen.
 *
 *	RETURNS
 *		Returns 0 if failed; otherwise 1.
 *
 */
{    
    FILE *stream;
    char *pStr;
    char buf[BUFSIZ];
    char keyword[BUFSIZ];
    char token1[BUFSIZ], token2[BUFSIZ];
    int  state = 0, tableType = -1, nTables = 0;
    int VisualFlag = -2;
    Window root;
    LINEAR_RGB_SCCData *pScreenData;

    if (screenNumber < 0) {
	fprintf(stderr,"Invalid Screen Number %d\n", screenNumber);
	return(0);
    }
    root = RootWindow(pDpy, screenNumber);

    if (!root) {
	/* if no root window is available then return an error */
	fprintf(stderr,"Could not open root window supplied.\n ");
	return (0);
    }
    /*
     * Open the file, determine its size, then read it into memory.
     */
    if (filename == NULL) {
	stream = stdin;
    } else if ((int)(stream = fopen(filename, "r")) == NULL) {
	fprintf(stderr,"Could not open file %s.\n", filename);
	return (0);
    }

    if ((pScreenData = 
	 (LINEAR_RGB_SCCData *)calloc(1, sizeof(LINEAR_RGB_SCCData))) ==NULL) {
	fprintf(stderr,"Could not allocate buffer for screen data.");
	closeS (stream, pScreenData);
	return (0);
    }

    /* need to read screen data file and fill up a LINEAR_RGB_SCCData struct */

    /*
     * Advance to starting keyword 
     * Anything before this keyword is just treated as comments.
     */

    while((pStr = nextline(buf, BUFSIZ, stream)) != NULL) {
	if ((sscanf(buf, "%s %s", keyword, token1) > 0)
	    && (strcmp(keyword, SC_BEGIN_KEYWORD) == 0)) {
	    break;
	}  /* else ignore the line */
    }

    if (pStr == NULL) {
	fprintf(stderr,"File %s is missing %s\n", filename, SC_BEGIN_KEYWORD);
	closeS (stream, pScreenData);
	return (0);
    }

    if (strcmp(token1, TXT_FORMAT_VERSION) != 0) {
	fprintf(stderr,
    "Screen data format version mismatch in file %s-- expected %s, found %s\n",
		filename, TXT_FORMAT_VERSION, token1);
	closeS (stream, pScreenData);
	return (0);
    }

    while ((pStr = nextline(buf, BUFSIZ, stream)) != NULL) {
	token1[0] = NULL; token2[0] = NULL;
	if (sscanf(buf, "%s %s %s", keyword, token1, token2) > 0) {
	    switch (SCKeyOf(keyword)) {
	      case COMMENT :
	      case NAME :
	      case PART_NUMBER :
	      case MODEL :
	      case SERIAL_NUMBER :
	      case REVISION :
		/* Do nothing */
		break;
	      case SCREEN_CLASS :
		VisualFlag = SCScrnClassOf(token1);
		if (VisualFlag == -1) {
		    closeS (stream, pScreenData);
		    return (0);
		}
		break;
	      case COLORIMETRIC_BEGIN :
		if (VisualFlag == -2) {
		    closeS (stream, pScreenData);
		    return (0);
		}
		if (!ProcessColorimetric(stream, 
					 pScreenData, filename, VisualFlag)) {
		    closeS (stream, pScreenData);
		    return (0);
		}
		state |= 0x02;
		break;
	      case IPROFILE_BEGIN :
		if (VisualFlag == -2) {
		    closeS (stream, pScreenData);
		    return (0);
		}
		if (!token1[0]) {
		    state |= 0x04;
		    break;
		}
		if (sscanf(token1, "%d", &tableType) != 1 ||
		    (tableType < -1 || tableType > 1)) {
		    fprintf(stderr,
			    "File %s has invalid type specified %s\n",
			    filename, buf);
		    closeS (stream, pScreenData);
		    return (0);
		} 
		if (tableType == CORR_TYPE_NONE) {
		    /* a CORR_TYPE_NONE (-1) : no conversion is used*/
		    state |= 0x04;
		    break;
		}
		if (!token2[0] && (VisualFlag == VIDEO_RGB)) {
		    fprintf(stderr,
			"File %s has invalid number of tables specified\n",
			    filename, buf);
		    closeS (stream, pScreenData);
		    return (0);
		}
		if (VisualFlag == VIDEO_RGB) {
		    if (sscanf(token2, "%d", &nTables) != 1 ||
			(nTables != 0 && nTables != 1 && nTables != 3)) {
			fprintf(stderr,
				"File %s has invalid number of tables (must be 0, 1, or 3)\n",
				filename, buf);
			closeS (stream, pScreenData);
			return (0);
		    }
		} else {
		    nTables = 0;
		}
		switch (nTables) {
		  case 3 :
		    if (!(pScreenData->pRedTbl = (IntensityTbl *)
			calloc (1, sizeof (IntensityTbl)))) {
			fprintf(stderr,
			       "Could not allocate Red Intensity Table\n");
			closeS (stream, pScreenData);
			return (0);
		    }
		    if (!(pScreenData->pGreenTbl = (IntensityTbl *)
			calloc (1, sizeof (IntensityTbl)))) {
			fprintf(stderr,
			     "Could not allocate Green Intensity Table\n");
			closeS (stream, pScreenData);
			return (0);
		    }
		    if (!(pScreenData->pBlueTbl = (IntensityTbl *)
			calloc (1, sizeof (IntensityTbl)))) {
			fprintf(stderr,
				"Could not allocate Blue Intensity Table");
			closeS (stream, pScreenData);
			return (0);
		    }
		    if (!ProcessIProfile(stream, pScreenData, 
					 tableType, nTables, filename)) {
			closeS (stream, pScreenData);
			return (0);
		    }
		    break;
		  case 1 :
		    if (!(pScreenData->pRedTbl = (IntensityTbl *)
			  calloc (1, sizeof (IntensityTbl)))) {
			fprintf(stderr,
				"Could not allocate Red Intensity Table");
			closeS (stream, pScreenData);
			return (0);
		    }
		    pScreenData->pGreenTbl = pScreenData->pRedTbl;
		    pScreenData->pBlueTbl = pScreenData->pRedTbl;
		    if (!ProcessIProfile(stream, pScreenData, 
					 tableType, nTables, filename)) {
			closeS (stream, pScreenData);
			return (0);
		    }
		    break;
		  default :
		    /* do nothing */
		    break;
		}
		state |= 0x04;
		break;
	      case SC_END :
		if (!(state & 0x02)) {
		    fprintf(stderr,
			    "File %s is missing Colormetric data.\n", 
			    filename);
		    closeS (stream, pScreenData);
		    return (0);
		}
		if (!(state & 0x04)) {
		    fprintf(stderr,
			    "File %s is missing Intensity Profile Data.\n",
			    filename);
		}
		if (VisualFlag == VIDEO_RGB) {
		    if (!LoadDataRGB(pDpy, root,
				     tableType, nTables, pScreenData)) {
			closeS (stream, pScreenData);
			return (0);
		    }
#ifdef GRAY
		} else if (VisualFlag == VIDEO_GRAY) {
		    if (!LoadDataGray(pDpy, root,
				      tableType, pScreenData)) {
			closeS (stream, pScreenData);
			return (0);
		    }
#endif /* GRAY */
		} else {
		    fprintf(stderr,"File %s Visual missing.", filename);
		}
		closeS (stream, pScreenData);
		return (1);
/* NOTREACHED */    break;
	      default :
		fprintf(stderr,"File %s has extraneous keyword %s\n", 
			filename, keyword);
		closeS (stream, pScreenData);
		return (0);

	    }
	}   /* else it was just a blank line */
    }
    closeS (stream, pScreenData);
    return (1);
}
