/* $XConsortium$ */
/***	jpege.h - common include file for jpeg encoder

	Ben Fahy, Oct 1993
***/

#define JPEG_ENCODE_GOAL_Startup		0
#define JPEG_ENCODE_GOAL_Done			1
#define JPEG_ENCODE_GOAL_TryToBeginFrame	2
#define JPEG_ENCODE_GOAL_WriteHdrData		3
#define JPEG_ENCODE_GOAL_BeginFrameDone		4
#define JPEG_ENCODE_GOAL_ProcessData		5
#define JPEG_ENCODE_GOAL_ReadDataForProcess	6
#define JPEG_ENCODE_GOAL_WriteDataForProcess	7
#define JPEG_ENCODE_GOAL_EndOfInput		8
#define JPEG_ENCODE_GOAL_FlushData		9
#define JPEG_ENCODE_GOAL_EncodeRGB		10
#define JPEG_ENCODE_GOAL_EncodeGray		11
#define JPEG_ENCODE_GOAL_CheckStatus		12
#define JPEG_ENCODE_GOAL_EndFrame		13

#define JPEG_ENCODE_ERROR_BadGoal		1
#define JPEG_ENCODE_ERROR_EncoderIsFreakingOut	2
#define JPEG_ENCODE_ERROR_CouldNotStart		3
#define JPEG_ENCODE_ERROR_CouldNotBeginFrame	4
#define JPEG_ENCODE_ERROR_BadBeginFrameRetCode	5
#define JPEG_ENCODE_ERROR_BadOutputAlloc	6
#define JPEG_ENCODE_ERROR_EncodeError		7
#define JPEG_ENCODE_ERROR_NoMoreProcessData	8
#define JPEG_ENCODE_ERROR_ColorMismatch		9
#define JPEG_ENCODE_ERROR_EndFrameError		10
#define JPEG_ENCODE_ERROR_EndFrameFreakOut	11

typedef struct _jpeg_encode_state {
	int goal;		/* what to do next			*/
	int error_code;		/* ooops				*/

	int n_bands;		/* color image or grayscale?		*/
	int width,height;	/* available elsewhere, but I'm lazy 	*/

	int flush_output;	/* how many bytes left to flush		*/
	int nl_coded;	 	/* how many lines we have, this round	*/
	int nl_tocode;	 	/* how many lines we want, this round	*/
	int i_line;	 	/* absolute input line position	*/
	unsigned char **i_lines[3];   
				/* pointers to input lines, 3 bands	*/

	compress_info_ptr cinfo;	
				/* decoder's private state		*/
	compress_methods_ptr    c_methods;
	external_methods_ptr    e_methods;
				/* decoder's private methods		*/

	unsigned char *jpeg_input_buffer;
	unsigned char *jpeg_output_buffer;
				/* names self-explanatory?		*/
	unsigned char *jpeg_output_bpos;
				/* output buffer position during flush	*/

	int  strip_size;	/* size  of currently available strip	*/
	unsigned char *strip;	/* start of currently available strip	*/
	unsigned char *sptr;	/* position within the strip		*/
	int needs_input_strip;	/* need a new strip			*/
	int i_strip;		/*  input strip #. nice for debugging	*/
	int o_strip;		/* output strip #. nice for debugging	*/
	int final;		/* this is the last strip		*/
	int no_more_strips;	/* flag saying you can't have any more	*/

  	int  strip_req_newbytes;/* number of destination bytes we want  */

	int lenQtable;
	int lenACtable;
	int lenDCtable;

	unsigned char *Qtable;
	unsigned char *ACtable;
	unsigned char *DCtable;

} JpegEncodeState;

extern int encode_jpeg_lossless_color(
#ifdef NEED_PROTOTYPES
	JPegEncodeState *	 /* state */
#endif
);

extern int encode_jpeg_lossy_color(
#ifdef NEED_PROTOTYPES
	JPegEncodeState *	 /* state */
#endif
);

extern int encode_jpeg_lossless_gray(
#ifdef NEED_PROTOTYPES
	JPegEncodeState *	 /* state */
#endif
);

extern int encode_jpeg_lossy_gray(
#ifdef NEED_PROTOTYPES
	JPegEncodeState *	 /* state */
#endif
);
