#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: AsciiSrc.c,v 1.9 89/07/16 16:08:10 converse Exp $";
#endif /* lint && SABER */

/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

/***********************************************************************
 *
 * Ascii Source
 *
 ***********************************************************************/

/*
 * AsciiSrc.c - Source code for Ascii Text Source.
 *
 * This is the source code for the Ascii Text Source.
 * It is intended to be used with the Text widget, the simplest way to use
 * this text source is to use the AsciiText Widget.
 *
 * Date:    June 29, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xmu/Misc.h>

#include <X11/Xaw/AsciiTextP.h>
#include <X11/Xaw/AsciiSrcP.h>

static XtResource resources[] = {
    {XtNstring, XtCString, XtRString, sizeof (char *),
       XtOffset(AsciiSourcePtr, string), XtRString, NULL},
    {XtNtype, XtCType, XtRAsciiType, sizeof (XawAsciiType),
       XtOffset(AsciiSourcePtr, type), XtRImmediate, (caddr_t)XawAsciiString},
    {XtNdataCompression, XtCDataCompression, XtRBoolean, sizeof (Boolean),
       XtOffset(AsciiSourcePtr, data_compression), 
       XtRImmediate, (caddr_t) TRUE},
    {XtNpieceSize, XtCPieceSize, XtRLong, sizeof (long),
       XtOffset(AsciiSourcePtr, piece_size),
       XtRImmediate, (caddr_t)DEFAULT_PIECE_SIZE},
    {XtNcallback, XtCCallback, XtRCallback, sizeof(caddr_t), 
       XtOffset(AsciiSourcePtr, callback), XtRCallback, (caddr_t)NULL},
#ifdef ASCII_STRING
    {XtNasciiString, XtCAsciiString, XtRBoolean, sizeof (Boolean),
       XtOffset(AsciiSourcePtr, ascii_string), 
       XtRImmediate, (caddr_t) FALSE},
    {XtNlength, XtCLength, XtRInt, sizeof (int),
       XtOffset(AsciiSourcePtr, ascii_length), 
       XtRImmediate, (caddr_t) MAGIC_VALUE},
#endif /* ASCII_STRING */

#ifdef ASCII_DISK
    {XtNfile, XtCFile, XtRString, sizeof (String),
       XtOffset(AsciiSourcePtr, filename), XtRString, NULL},
#endif /* ASCII_DISK */
};

static XtResource sourceResources[] = {
    {XtNeditType, XtCEditType, XtREditMode, sizeof(int), 
        XtOffset(XawTextSource, edit_mode), XtRString, "read"},
};

static Piece * FindPiece(), * AllocNewPiece();
static FILE * InitStringOrFile();
static void FreeAllPieces(), RemovePiece(), BreakPiece(), LoadPieces();
static String MyStrncpy(), StorePiecesInString();
static Boolean WriteToFile(), SetValuesHook();
static void GetValuesHook(), RemoveOldStringOrFile(),  CvtStringToAsciiType();

extern char *tmpnam();
void bcopy();
extern int errno, sys_nerr;
extern char* sys_errlist[];

/************************************************************
 *
 * Semi-Public Interfaces.
 *
 ************************************************************/

/*	Function Name: ReadText
 *	Description: This function reads the source.
 *	Arguments: src - the AsciiSource.
 *                 pos - position of the text to retreive.
 * RETURNED        text - text block that will contain returned text.
 *                 length - maximum number of characters to read.
 *	Returns: The number of characters read into the buffer.
 */

static XawTextPosition
ReadText(src, pos, text, length)
XawTextSource src;
XawTextPosition pos;
XawTextBlock *text;	
int length;		
{
  XawTextPosition count, start;
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;
  Piece * piece;
  
  piece = FindPiece(data, pos, &start);
    
  text->firstPos = pos;
  text->ptr = piece->text + (pos - start);
  count = piece->used - (pos - start);
  text->length = (length > count) ? count : length;
  return(pos + text->length);
}

/*	Function Name: ReplaceText.
 *	Description: Replaces a block of text with new text.
 *	Arguments: src - the AsciiSource.
 *                 startPos, endPos - ends of text that will be removed.
 *                 text - new text to be inserted into buffer at startPos.
 *	Returns: XawEditError or XawEditDone.
 */

/*ARGSUSED*/
static int 
ReplaceText (src, startPos, endPos, text)
XawTextSource src;
XawTextPosition startPos, endPos;
XawTextBlock *text;
{
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;
  Piece *start_piece, *end_piece, *temp_piece;
  XawTextPosition start_first, end_first;
  int length, firstPos;

/*
 * Editing a read only source is not allowed.
 */

  if (src->edit_mode == XawtextRead) 
    return(XawEditError);

  start_piece = FindPiece(data, startPos, &start_first);
  end_piece = FindPiece(data, endPos, &end_first);

  data->changes = TRUE;		/* We have changed the buffer. */

#ifndef notdef			/* This code only will call the first
				   function in the callback list, I plan
				   to fix this when the correct R4 Xt
				   function comes along. CDP 7-5-89. */

   if ( (data->callback != NULL) && (data->callback[0].callback != NULL)) 
     (data->callback[0].callback) (src->widget, data->callback[0].closure, 
				   NULL);
#endif

/* 
 * Remove Old Stuff. 
 */

  if (start_piece != end_piece) {
    temp_piece = start_piece->next;

/*
 * If empty and not the only piece then remove it. 
 */

    if ( ((start_piece->used = startPos - start_first) == 0) &&
	 !((start_piece->next == NULL) && (start_piece->prev == NULL)) )
      RemovePiece(data, start_piece);

    while (temp_piece != end_piece) {
      temp_piece = temp_piece->next;
      RemovePiece(data, temp_piece->prev);
    }
    end_piece->used -= endPos - end_first;
    if (end_piece->used != 0)
      MyStrncpy(end_piece->text, (end_piece->text + endPos - end_first),
		(int) end_piece->used);
  }
  else {			/* We are fully in one piece. */
    if ( (start_piece->used -= endPos - startPos) == 0) {
      if ( !((start_piece->next == NULL) && (start_piece->prev == NULL)) )
	RemovePiece(data, start_piece);
    }
    else {
      MyStrncpy(start_piece->text + (startPos - start_first),
		start_piece->text + (endPos - start_first),
		(int) (start_piece->used - (startPos - start_first)) );
#ifdef ASCII_STRING
      if (data->ascii_string) 
	start_piece->text[data->length - (endPos - startPos)] = '\0';
#endif
    }
  }

  data->length += -(endPos - startPos) + text->length;

  if ( text->length != 0) {

    /* 
     * Put in the New Stuff.
     */
    
    start_piece = FindPiece(data, startPos, &start_first);
    
    length = text->length;
    firstPos = text->firstPos;
    
    while (length > 0) {
      char * ptr;
      int fill;
      
      if (start_piece->used == data->piece_size) {
#ifdef ASCII_STRING
	if (data->ascii_string) {
	  /*
	   * If we are in ascii string emulation mode. Then the string is not 
	   * allowed to grow.
	   */
	  start_piece->used = data->length = data->piece_size;
	  start_piece->text[data->length] = '\0';
	  return(XawEditError);
	}
#endif
	BreakPiece(data, start_piece);
	start_piece = FindPiece(data, startPos, &start_first);
      }
      fill = Min((int) (data->piece_size - start_piece->used), length);
      
      ptr = start_piece->text + (startPos - start_first);
      MyStrncpy(ptr + fill, ptr, 
		(int) start_piece->used - (startPos - start_first));
      strncpy(ptr, text->ptr + firstPos, fill);
      
      startPos += fill;
      firstPos += fill;
      start_piece->used += fill;
      length -= fill;
    }
  }

#ifdef ASCII_STRING
  if (data->ascii_string) 
    start_piece->text[start_piece->used] = '\0';
#endif

  return(XawEditDone);
}

/* ARGSUSED */
static int 
DiskSetLastPos (src, lastPos)
XawTextSource src;
XawTextPosition lastPos;
{
#ifdef ASCII_STRING
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;

  if (data->ascii_string) 
    return(data->first_piece->used = lastPos);
#endif

  XtErrorMsg("DiskSetLastPos", "XawTextSetLastPos", "XawError",
	     "The Ascii source does not support this operation",
	     NULL,  (Cardinal) 0);
  return(0);
}

/*	Function Name: Scan
 *	Description: Scans the text source for the number and type
 *                   of item specified.
 *	Arguments: src - the AsciiSource.
 *                 position - the position to start scanning.
 *                 type - type of thing to scan for.
 *                 dir - direction to scan.
 *                 count - which occurance if this thing to search for.
 *                 include - whether or not to include the character found in
 *                           the position that is returned. 
 *	Returns: the position of the item found.
 */

static 
XawTextPosition 
Scan (src, position, type, dir, count, include)
XawTextSource         src;
XawTextPosition       position;
XawTextScanType       type;
XawTextScanDirection  dir;
int     	      count;
Boolean	              include;
{
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;
  register int inc;
  Piece * piece;
  XawTextPosition first;
  register char * ptr;

  if (type == XawstAll) {	/* Optomize this common case. */
    if (dir == XawsdRight)
      return(data->length);
    return(0);			/* else. */
  }

  if ( dir == XawsdRight )
    inc = 1;
  else {
    inc = -1;
    if (position == 0)
      return(0);		/* scanning left from 0??? */
    position--;
  }

  if (position >= data->length)
    position = data->length - 1;

  piece = FindPiece(data, position, &first);

/*
 * If the buffer is empty then return 0. 
 */

  if ( piece->used == 0 ) return(0); 

  ptr = (position - first) + piece->text;

  switch (type) {
  case XawstWhiteSpace: 
  case XawstEOL: 
    for ( ; count > 0 ; count-- ) {
      Boolean non_space = FALSE;
      while (TRUE) {
	register char c = *ptr;

	ptr += inc;
	position += inc;
	
	if (type == XawstWhiteSpace) {
	  if (isspace(c)) {
	    if (non_space) 
	      break;
	  }
	  else
	    non_space = TRUE;
	}
	else			/* type == XawstEOL */
	  if (c == '\n') break;

	if ( ptr < piece->text ) {
	  piece = piece->prev;
	  if (piece == NULL)	/* Begining of text. */
	    return(0);
	  ptr = piece->text + piece->used - 1;
	}
	else if ( ptr >= (piece->text + piece->used) ) {
	  piece = piece->next;
	  if (piece == NULL)	/* End of text. */
	    return(data->length);
	  ptr = piece->text;
	}
      }
    }
    if (!include)
      position -= inc;
    break;
  case XawstPositions: 
    position += count * inc;
    break;
/*  case XawstAll:		---- handled in special code above */
  }

  if ( dir == XawsdLeft )
    position++;

  if (position < 0)
    return(0);
  if (position > data->length)
    return(data->length);
  return(position);
}

/*	Function Name: Search
 *	Description: Searchs the text source for the text block passed
 *	Arguments: src - the AsciiSource.
 *                 position - the position to start scanning.
 *                 dir - direction to scan.
 *                 text - the text block to search for.
 *	Returns: the position of the item found.
 */

static XawTextPosition 
Search(src, position, dir, text)
XawTextSource         src;
XawTextPosition       position;
XawTextScanDirection  dir;
XawTextBlock *        text;
{
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;
  register int inc, count = 0;
  register char * ptr;
  Piece * piece;
  char * buf;
  XawTextPosition first;

  if ( dir == XawsdRight )
    inc = 1;
  else {
    inc = -1;
    if (position == 0)
      return(XawTextSearchError);	/* scanning left from 0??? */
    position--;
  }

  buf = XtMalloc(sizeof(char) * text->length);
  strncpy(buf, (text->ptr + text->firstPos), text->length);
  piece = FindPiece(data, position, &first);
  ptr = (position - first) + piece->text;

  while (TRUE) {
    if (*ptr == ((dir == XawsdRight) ? *(buf + count) 
		                     : *(buf + text->length - count - 1)) ) {
      if (count == (text->length - 1))
	break;
      else
	count++;
    }
    else {
      if (count != 0) {
	position -=inc * count;
	ptr -= inc * count;
      }
      count = 0;
    }

    ptr += inc;
    position += inc;
    
    while ( ptr < piece->text ) {
      piece = piece->prev;
      if (piece == NULL) {	/* Begining of text. */
	XtFree(buf);
	return(XawTextSearchError);
      }
      ptr = piece->text + piece->used - 1;
    }
   
    while ( ptr >= (piece->text + piece->used) ) {
      piece = piece->next;
      if (piece == NULL) {	/* End of text. */
	XtFree(buf);
	return(XawTextSearchError);
      }
      ptr = piece->text;
    }
  }

  XtFree(buf);
  if (dir == XawsdLeft)
    return(position);
  return(position - (text->length - 1));
}

/*	Function Name: SetValuesHook
 *	Description: Sets the values for the AsciiSource.
 *	Arguments: src - the text source.
 *                 args - the arg list.
 *                 num_args - the number of args in the list.
 *	Returns: True if redisplay is needed.
 */

static Boolean
SetValuesHook(src, args, num_args)
XawTextSource src;
ArgList args;
Cardinal * num_args;
{
  Boolean total_reset = FALSE;
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;
  AsciiSourcePtr old_data = XtNew(AsciiSourceData);
  FILE * file;

  bcopy( (char *) data, (char *) old_data, sizeof(AsciiSourceData));
  
  XtSetSubvalues( (caddr_t) data, resources, XtNumber(resources),
		  args, * num_args);

  if ( (old_data->string != data->string) || (old_data->type != data->type) ) {

    if (old_data->string == data->string) {
      data->allocated_string = FALSE; /* fool it into not freeing the string */
      RemoveOldStringOrFile(old_data);        /* remove old info. */
      data->allocated_string = TRUE;
    }
    else {
      RemoveOldStringOrFile(old_data);        /* remove old info. */
      data->allocated_string = FALSE;
    }

    file = InitStringOrFile(src);    /* Init new info. */
    LoadPieces(data, file, NULL);    /* load new info into internal buffers. */
    if (file != NULL) fclose(file);
    XawTextSetSource(src->widget, src, 0);   /* tell text widget 
						what happened. */
    total_reset = TRUE;
  }

  if ( !total_reset && (old_data->piece_size != data->piece_size) ) {
    String string = StorePiecesInString(old_data);
    FreeAllPieces(old_data);
    LoadPieces(data, NULL, string);
    XtFree(string);
  }

  XtFree(old_data);

  XtSetSubvalues((caddr_t) src, sourceResources, XtNumber(sourceResources),
		 args, * num_args);

  return(FALSE);
}

/*	Function Name: GetValuesHook
 *	Description: This is a get values hook routine that sets the
 *                   values specific to the ascii source.
 *	Arguments: src - the text source.
 *                 args - the argument list.
 *                 num_args - the number of args.
 *	Returns: none.
 */

static void
GetValuesHook(src, args, num_args)
XawTextSource src;
ArgList args;
Cardinal * num_args;
{
  register int i;
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;

#ifdef ASCII_STRING
  if (!data->ascii_string)
#endif
    if (data->type == XawAsciiString) {
      for (i = 0; i < *num_args ; i++ ) 
	if (streq(args[i].name, XtNstring)) {
	  XawAsciiSave(src->widget);
	  break;
	}
    }

  XtGetSubvalues((caddr_t) data, resources, XtNumber(resources),
		 args, *num_args);

  XtGetSubvalues((caddr_t) src, sourceResources, XtNumber(sourceResources),
		 args, *num_args);
}

/************************************************************
 *
 * Public routines 
 *
 ************************************************************/

/*	Function Name: XawAsciiSourceFreeString
 *	Description: Frees the string returned by a get values call
 *                   on the string when the source is of type string.
 *	Arguments: w - the widget.
 *	Returns: none.
 */

void
XawAsciiSourceFreeString(w)
Widget w;
{
  TextWidget tw = (TextWidget) w;
  AsciiSourcePtr data = (AsciiSourcePtr) tw->text.source->data;  

  if (data->allocated_string) {
    data->allocated_string = FALSE;
    XtFree(data->string);
  }
}
    
/*	Function Name: XawAsciiSourceCreate
 *	Description: Creates the AsciiSource.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */

XawTextSource 
XawAsciiSourceCreate(parent, args, num_args)
Widget	parent;
ArgList	args;
Cardinal num_args;
{
  XawTextSource src;
  AsciiSourcePtr data;
  static Boolean src_init = FALSE;
  FILE * file;

  if (!src_init) {		/* Call this only once. */
    src_init = TRUE;
    XtAddConverter( XtRString, XtRAsciiType, CvtStringToAsciiType,
		   NULL, (Cardinal) 0);
  }

  src = XtNew(XawTextSourceRec);

  XtGetSubresources (parent, (caddr_t)src, XtNtextSource, XtCTextSource,
		     sourceResources, XtNumber(sourceResources),
		     args, num_args);

  src->widget = parent;
  src->Read = ReadText;
  src->Replace = ReplaceText;
  src->SetLastPos = DiskSetLastPos;
  src->Scan = Scan;
  src->Search = Search;
  src->SetSelection = NULL;
  src->ConvertSelection = NULL;
  src->SetValuesHook = SetValuesHook;
  src->GetValuesHook = GetValuesHook;
  src->data = (caddr_t) (data = XtNew(AsciiSourceData));

  XtGetSubresources (parent, (caddr_t)data, XtNtextSource, XtCTextSource,
		     resources, XtNumber(resources),
		     args, num_args);

/*
 * Set correct flags (override resources) depending upon widget class.
 */

#ifdef ASCII_DISK
  if (XtIsSubclass(parent, asciiDiskWidgetClass)) {
    data->type = XawAsciiFile;
    data->string = data->filename;
  }
#endif

#ifdef ASCII_STRING
  if (XtIsSubclass(parent, asciiStringWidgetClass)) {
    data->ascii_string = TRUE;
    data->type = XawAsciiString;
  }
#endif

/* end */

  data->changes = FALSE;
  data->allocated_string = FALSE;

  file = InitStringOrFile(src);
  LoadPieces(data, file, NULL);
  if (file != NULL) fclose(file);
  return src;
}

/*	Function Name: XawAsciiSourceDestroy
 *	Description: Destroys an ascii source (frees all data)
 *	Arguments: src - ths source to free.
 *	Returns: none.
 */

void 
XawAsciiSourceDestroy (src)
XawTextSource src;
{
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;

  RemoveOldStringOrFile(data);

  XtFree((char *) src->data);
  XtFree((char *) src);
}

/*	Function Name: XawAsciiSave
 *	Description: Saves all the pieces into a file or string as required.
 *	Arguments: w - the asciiText Widget.
 *	Returns: TRUE if the save was successful.
 */

Boolean
XawAsciiSave(w)
Widget w;
{
  AsciiWidget tw = (AsciiWidget) w;
  AsciiSourcePtr data = (AsciiSourcePtr) tw->text.source->data;
  char * string;

#ifdef ASCII_STRING
  if (data->ascii_string) {	/* It did not support this functionality. */
    XtAppWarning(XtWidgetToApplicationContext(w), 
	        "XawAsciiSave is not supported in by the AsciiString Widget.");
    return(FALSE);
  }
#endif

  if (!data->changes)		/* No changes to save. */
    return(TRUE);

  string = StorePiecesInString(data);

  if (data->type == XawAsciiFile) {
    if (WriteToFile(w, string, data->string) == FALSE) {
      XtFree(string);
      return(FALSE);
    }
    XtFree(string);
  }
  else {			/* This is a string widget. */
    if (data->allocated_string == TRUE) 
      XtFree(data->string);
    else
      data->allocated_string = TRUE;
    
    data->string = string;
  }
  data->changes = FALSE;
  return(TRUE);
}

/*	Function Name: XawAsciiSaveAsFile
 *	Description: Save the current buffer as a file.
 *	Arguments: w - the ascii text widget.
 *                 name - name of the file to save this file into.
 *	Returns: True if the save was sucessful.
 */

Boolean
XawAsciiSaveAsFile(w, name)
Widget w;
String name;
{
  AsciiSourcePtr data = (AsciiSourcePtr) ((TextWidget) w)->text.source->data;
  String string;
  Boolean ret;

  string = StorePiecesInString(data); 

  ret = WriteToFile(w, string, name);
  XtFree(string);
  return(ret);
}

/*	Function Name: XawAsciiSourceChanged
 *	Description: Returns true if the source has changed since last saved.
 *	Arguments: w - the ascii source widget.
 *	Returns: a Boolean (see description).
 */

Boolean 
XawAsciiSourceChanged(w)
Widget w;
{
  return ( ((AsciiSourcePtr) ((TextWidget) w)->text.source->data)->changes);
}
  
/************************************************************
 *
 * Private Functions.
 *
 ************************************************************/

static void
RemoveOldStringOrFile(data) 
AsciiSourcePtr data;
{
  FreeAllPieces(data);

  if (data->allocated_string) 
    XtFree(data->string);
}

/*	Function Name: WriteToFile
 *	Description: Write the string specified to the begining of the file
 *                   specified.
 *	Arguments: w - the widget.          (for error messages only)
 *                 string - string to write.
 *                 name - the name of the file
 *                 file - file to write it to.
 *	Returns: returns TRUE if sucessful, FALSE otherwise.
 */

static Boolean
WriteToFile(w, string, name)
Widget w;
String string, name;
{
  char buf[BUFSIZ];
  int fd;
  
  if ( ((fd = creat(name, 0666)) == -1 ) ||
       (write(fd, string, sizeof(char) * strlen(string)) == -1) ) {
    sprintf(buf, "Error, while attempting to write to the file %s.", name);
    XtAppWarning(XtWidgetToApplicationContext(w), buf); 
    return(FALSE);
  }

  if ( close(fd) == -1 ) {
    sprintf(buf, "Error, while attempting to close the file %s.", name);
    XtAppWarning(XtWidgetToApplicationContext(w), buf); 
    return(FALSE);
  }
  return(TRUE);
}

/*	Function Name: StorePiecesInString
 *	Description: store the pieces in memory into a standard ascii string.
 *	Arguments: data - the ascii pointer data.
 *	Returns: none.
 */

static String
StorePiecesInString(data)
AsciiSourcePtr data;
{
  String string;
  XawTextPosition first;
  Piece * piece;

  string = XtMalloc(sizeof(char) * data->length + 1);
  
  for (first = 0, piece = data->first_piece ; piece != NULL; 
       first += piece->used, piece = piece->next) 
    strncpy(string + first, piece->text, piece->used);

  string[data->length] = '\0';	/* NULL terminate this sucker. */

  if (data->data_compression) {	/* This will refill all pieces to capacity. */
    FreeAllPieces(data);
    LoadPieces(data, NULL, string);
  }

  return(string);
}

/*	Function Name: InitStringOrFile.
 *	Description: Initializes the string or file.
 *	Arguments: src - the AsciiSource.
 *	Returns: none - May exit though.
 */

static FILE *
InitStringOrFile(src)
XawTextSource src;
{
  AsciiSourcePtr data = (AsciiSourcePtr) src->data;
  char * open_mode;
  FILE * file;

  if (data->type == XawAsciiString) {

    if (data->string == NULL)
      data->length = 0;
    else 
      data->length = strlen(data->string);

#ifdef ASCII_STRING
    if (data->ascii_string) {
      if (data->ascii_length == MAGIC_VALUE) 
	data->piece_size = data->length; /* leave space for the NULL. */
      else
	data->piece_size = data->ascii_length - 1;
    }
#endif

    return(NULL);
  }

/*
 * type is XawAsciiFile.
 */

  data->is_tempfile = FALSE;

  switch (src->edit_mode) {
  case XawtextRead:
    if (data->string == NULL)
      XtErrorMsg("NoFile", "asciiSourceCreate", "XawError",
		 "Creating a read only disk widget and no file specified.",
		 NULL, 0);
    open_mode = "r";
    break;
  case XawtextAppend:
  case XawtextEdit:
    if (data->string == NULL) {
      data->string = tmpnam (XtMalloc((unsigned)TMPSIZ));
      data->is_tempfile = TRUE;
    } 
    else {
      if (!data->allocated_string) {
	data->allocated_string = TRUE;
	data->string = XtNewString(data->string);
      }
      open_mode = "r+";
    }

    break;
  default:
    XtErrorMsg("badMode", "asciiSourceCreate", "XawError",
	       "Bad editMode for ascii source; must be Read, Append or Edit.",
	       NULL, NULL);
  }

  if (!data->is_tempfile) {
    if ((file = fopen(data->string, open_mode)) == 0) {
      String params[2];
      Cardinal num_params = 2;
      
      params[0] = data->string;
      if (errno <= sys_nerr)
	params[1] = sys_errlist[errno];
      else {
	char msg[11];
	sprintf(msg, "errno=%.4d", errno);
	params[1] = msg;
      }
      XtErrorMsg("openError", "asciiSourceCreate", "XawError",
		 "Cannot open source file %s; %s", params, &num_params);
    }
    (void) fseek(file, 0L, 2);
    data->length = ftell (file); 
  } 
  else {
    data->length = 0;
    return(NULL);
  }

  return(file);
}

static void
LoadPieces(data, file, string)
AsciiSourcePtr data;
FILE * file;
char * string;
{
  char *local_str, *ptr;
  register Piece * piece = NULL;
  XawTextPosition left;

  if (string == NULL) {
    if (data->type == XawAsciiFile) {
      local_str = XtMalloc((data->length + 1) * sizeof(char));
      if (data->length != 0) {
	fseek(file, 0L, 0);
	if (fread(local_str, sizeof(char), data->length, file)!=data->length) {
	  XtErrorMsg("readError", "asciiSourceCreate", "XawError",
		     "fread returned error.", NULL, NULL);
	}
      }
      local_str[data->length] = '\0';
    }
    else
      local_str = data->string;
  }
  else
    local_str = string;

#ifdef ASCII_STRING
/*
 * In Ascii String Emulation we will use the string in place, and
 * set the other fields as follows:
 *
 * piece_size = length;
 * piece->used = data->length;
 */
  
  if (data->ascii_string) {
    piece = AllocNewPiece(data, piece);
    piece->used = data->length;
    piece->text = data->string;
    return;
  }
#endif

  ptr = local_str;
  left = data->length;

  do {
    piece = AllocNewPiece(data, piece);

    piece->text = XtMalloc(data->piece_size * sizeof(char));
    piece->used = Min(left, data->piece_size);
    if (piece->used != 0)
      strncpy(piece->text, ptr, piece->used);

    left -= piece->used;
    ptr += piece->used;
  } while (left > 0);

  if ( (data->type == XawAsciiFile) && (string == NULL) )
    XtFree(local_str);
}

/*	Function Name: AllocNewPiece
 *	Description: Allocates a new piece of memory.
 *	Arguments: data - the source data information.
 *                 prev - the piece just before this one, or NULL.
 *	Returns: the allocated piece.
 */

static Piece *
AllocNewPiece(data, prev)
AsciiSourcePtr data;
Piece * prev;
{
  Piece * piece = XtNew(Piece);

  if (prev == NULL) {
    data->first_piece = piece;
    piece->next = NULL;
  }
  else {
    if (prev->next != NULL)
      (prev->next)->prev = piece;
    piece->next = prev->next;
    prev->next = piece;
  }
  
  piece->prev = prev;

  return(piece);
}

/*	Function Name: FreeAllPieces
 *	Description: Frees all the pieces
 *	Arguments: first - the first piece of data.
 *	Returns: none.
 */

static void 
FreeAllPieces(data)
AsciiSourcePtr data;
{
  Piece * next, * first = data->first_piece;

  if (first->prev != NULL)
    printf("Programmer Botch in FreeAllPieces, there may be a memory leak.\n");

  for ( ; first != NULL ; first = next ) {
    next = first->next;
    RemovePiece(data, first);
  }
}
  
/*	Function Name: RemovePiece
 *	Description: Removes a piece from the list.
 *	Arguments: data - the source data.
 *                 piece - the piece to remove.
 *	Returns: none.
 */

static void
RemovePiece(data, piece)
AsciiSourcePtr data;
Piece * piece;
{
  if (piece->prev == NULL)
    data->first_piece = piece->next;
  else
    (piece->prev)->next = piece->next;

  if (piece->next != NULL)
    (piece->next)->prev = piece->prev;

#ifdef ASCII_STRING
  if (data->allocated_string)
#endif
  XtFree(piece->text);
  XtFree(piece);
}

/*	Function Name: FindPiece
 *	Description: Finds the piece containing the position indicated.
 *	Arguments: data - the source data.
 *                 position - the position that we are searching for.
 * RETURNED        first - the position of the first character in this piece.
 *	Returns: piece - the piece that contains this position.
 */

static Piece *
FindPiece(data, position, first)
AsciiSourcePtr data;
XawTextPosition position, *first;
{
  Piece * old_piece, * piece = data->first_piece;
  XawTextPosition temp;

  for ( temp = 0 ; piece != NULL ; temp += piece->used, piece = piece->next ) {
    *first = temp;
    old_piece = piece;

    if ((temp + piece->used) > position) 
      return(piece);
  }
  return(old_piece);	  /* if we run off the end the return the last piece */
}
    
/*	Function Name: MyStrncpy
 *	Description: Just like string copy, but slower and will always
 *                   work on overlapping strings.
 *	Arguments: (same as strncpy) - s1, s2 - strings to copy (2->1).
 *                  n - the number of chars to copy.
 *	Returns: s1.
 */

static String
MyStrncpy(s1, s2, n)
char * s1, * s2;
int n;
{
  char * temp = XtMalloc(sizeof(char) * n);

  strncpy(temp, s2, n);		/* Saber has a bug that causes it to generate*/
  strncpy(s1, temp, n);		/* a bogus warning message here (CDP 6/32/89)*/
  XtFree(temp);
  return(s1);
}
  
/*	Function Name: BreakPiece
 *	Description: Breaks a full piece into two new pieces.
 *	Arguments: data - the source data.
 *                 piece - the piece to break.
 *	Returns: none.
 */

#define HALF_PIECE (data->piece_size/2)

static void
BreakPiece(data, piece)
AsciiSourcePtr data;
Piece * piece;
{
  Piece * new = AllocNewPiece(data, piece);
  
  new->text = XtMalloc(data->piece_size * sizeof(char));
  strncpy(new->text, piece->text + HALF_PIECE, data->piece_size - HALF_PIECE);
  piece->used = HALF_PIECE;
  new->used = data->piece_size - HALF_PIECE; 
}

/* ARGSUSED */
static void
CvtStringToAsciiType(args, num_args, fromVal, toVal)
XrmValuePtr *args;		/* unused */
Cardinal	*num_args;	/* unused */
XrmValuePtr	fromVal;
XrmValuePtr	toVal;
{
  static XawAsciiType type;
  static XrmQuark  XtQEstring;
  static XrmQuark  XtQEfile;
  static int	  haveQuarks = FALSE;
  XrmQuark q;
  char lowerName[BUFSIZ];

  if (!haveQuarks) {
    XtQEstring = XrmStringToQuark(XtEstring);
    XtQEfile   = XrmStringToQuark(XtEfile);
    haveQuarks = TRUE;
  }

  XmuCopyISOLatin1Lowered(lowerName, (char *) fromVal->addr);
  q = XrmStringToQuark(lowerName);

  if (q == XtQEstring) type = XawAsciiString;
  if (q == XtQEfile)  type = XawAsciiFile;

  (*toVal).size = sizeof(XawAsciiType);
  (*toVal).addr = (caddr_t) &type;
  return;
}

#if (defined(ASCII_STRING) || defined(ASCII_DISK))
#  include <X11/Xaw/Cardinals.h>
#endif

#ifdef ASCII_STRING
/************************************************************
 *
 * Compatability functions.
 *
 ************************************************************/
 
/*	Function Name: AsciiStringSourceCreate
 *	Description: Creates a string source.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */

XawTextSource
XawStringSourceCreate(parent, args, num_args)
Widget parent;
ArgList args;
Cardinal num_args;
{
  XawTextSource src;
  ArgList ascii_args;
  Arg temp[2];

  XtSetArg(temp[0], XtNtype, XawAsciiString);
  XtSetArg(temp[1], XtNasciiString, TRUE);
  ascii_args = XtMergeArgLists(temp, TWO, args, num_args);

  src = XawAsciiSourceCreate(parent, ascii_args, num_args + TWO);
  XtFree(ascii_args);
  return(src);
}
#endif /* ASCII_STRING */

#ifdef ASCII_DISK
/*	Function Name: AsciiDiskSourceCreate
 *	Description: Creates a disk source.
 *	Arguments: parent - the widget that will own this source.
 *                 args, num_args - the argument list.
 *	Returns: a pointer to the new text source.
 */

XawTextSource
XawDiskSourceCreate(parent, args, num_args)
Widget parent;
ArgList args;
Cardinal num_args;
{
  XawTextSource src;
  ArgList ascii_args;
  Arg temp[1];
  register int i;

  XtSetArg(temp[0], XtNtype, XawAsciiFile);
  ascii_args = XtMergeArgLists(temp, ONE, args, num_args);

  for (i = 0; i < num_args; i++) 
    if (streq(args[i].name, XtNfile) || streq(args[i].name, XtCFile)) 
      args[i].name = XtNstring;

  src = XawAsciiSourceCreate(parent, ascii_args, num_args + ONE);
  XtFree(ascii_args);
  return(src);
}
#endif /* ASCII_DISK */
