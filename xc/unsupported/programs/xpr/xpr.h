/* 3812 PagePrinter macros */
#define PPI	240
#define inch2pel(inches)	((int) ((inches) * PPI))
#define DEFAULT_WIDTH	8.5
#define X_MAX_PELS	inch2pel(DEFAULT_WIDTH)
#define DEFAULT_LENGTH	11
#define Y_MAX_PELS	inch2pel(DEFAULT_LENGTH)

#define INTENSITY(color) (30L*(color)->red + \
			  59L*(color)->green + \
			  11L*(color)->blue)

#define HALFINTENSITY (((1<<16)-1)*50L)

enum orientation {
    UNSPECIFIED = -1,
    PORTRAIT = 0,
    LANDSCAPE = 1,
    UPSIDE_DOWN = 2,
    LANDSCAPE_LEFT = 3
  };
