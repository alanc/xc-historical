/* $XConsortium$ */
/***	jpegd.h - common include file for jpeg decoders

	Ben Fahy, Oct 1993
***/

#define JPEG_DECODE_GOAL_Startup		0
#define JPEG_DECODE_GOAL_Done			1
#define JPEG_DECODE_GOAL_ReadDataForInit	2
#define JPEG_DECODE_GOAL_TryToInit		3
#define JPEG_DECODE_GOAL_InitDone		4
#define JPEG_DECODE_GOAL_ProcessData		5
#define JPEG_DECODE_GOAL_ReadDataForProcess	6
#define JPEG_DECODE_GOAL_WriteDataForProcess	7
#define JPEG_DECODE_GOAL_EndOfInput		8
#define JPEG_DECODE_GOAL_FlushData		9

#define JPEG_DECODE_ERROR_BadGoal		1
#define JPEG_DECODE_ERROR_DecoderIsFreakingOut	2
#define JPEG_DECODE_ERROR_CouldNotStart		3
#define JPEG_DECODE_ERROR_CouldNotInit		4
#define JPEG_DECODE_ERROR_BadInitRetCode	5
#define JPEG_DECODE_ERROR_BadOutputAlloc	6
#define JPEG_DECODE_ERROR_DecodeError		7
#define JPEG_DECODE_ERROR_NoMoreProcessData	8
#define JPEG_DECODE_ERROR_ColorMismatch		9
#define JPEG_DECODE_ERROR_WidthMismatch		10
#define JPEG_DECODE_ERROR_HeightMismatch	11

typedef struct _jpeg_decode_state {
	int goal;		/* what to do next			*/
	int error_code;		/* ooops				*/

	int color;		/* color image or grayscale?		*/
	int width,height;	/* available elsewhere, but I'm lazy 	*/

	int o_line;	 	/* absolute output line position	*/
	int nl_flushed;	 	/* how many lines we have flusehd	*/
	int nl_found;	 	/* how many lines we have, this round	*/
	unsigned char **o_lines[3];   
				/* pointers to output lines, 3 bands	*/

	decompress_info_ptr cinfo;	
				/* decoder's private state		*/
	decompress_methods_ptr dc_methods;
	external_methods_ptr    e_methods;
				/* decoder's private methods		*/

	unsigned char *jpeg_input_buffer;
	unsigned char *jpeg_output_buffer;
				/* names self-explanatory?		*/

	int  strip_size;	/* size  of currently available strip	*/
	unsigned char *strip;	/* start of currently available strip	*/
	unsigned char *sptr;	/* position within the strip		*/
	int needs_input_strip;	/* need a new strip			*/
	int i_strip;		/*  input strip #. nice for debugging	*/
	int o_strip;		/* output strip #. nice for debugging	*/
	int final;		/* this is the last strip		*/
	int no_more_strips;	/* flag saying you can't have any more	*/

} JpegDecodeState;

extern int decode_jpeg_lossless_color(
#ifdef NEED_PROTOTYPES
	JPegDecodeState *	 /* state */
#endif
);

extern int decode_jpeg_lossy_color(
#ifdef NEED_PROTOTYPES
	JPegDecodetSate *	 /* state */
#endif
);

extern int decode_jpeg_lossless_gray(
#ifdef NEED_PROTOTYPES
	JPegDecodeState *	 /* state */
#endif
);

extern int decode_jpeg_lossy_gray(
#ifdef NEED_PROTOTYPES
	JPegDecodeState *	 /* state */
#endif
);


