/* $XConsortium$ */

extern Bool s3BlockCursor;
extern Bool s3ReloadCursor;

#define BLOCK_CURSOR	s3BlockCursor = TRUE;

#define UNBLOCK_CURSOR	{ \
			   if (s3ReloadCursor) \
			      s3RestoreCursor(s3savepScreen); \
			   s3BlockCursor = FALSE; \
			}
