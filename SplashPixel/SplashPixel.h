


#define SP_COLUMNS 			4								// We have this many SplashPixels Boards Across
#define SP_ROWS 			2								// by this many rows deep
#define SP_RGB_WIDTH		4								// No of RGB's across each SplashPixel
#define SP_LEDS				(SP_RGB_WIDTH * SP_RGB_WIDTH)*3	// LEDs per SplashPixel Board
#define SP_COUNT			SP_COLUMNS * SP_ROWS			// No of SplashPixel Boards, We assume we allways have a complete rectangle

#define SP_RGB_COLUMNS 		SP_COLUMNS * SP_RGB_WIDTH		// Each SplashPixel is 4x4, work out how big the matrix is base upon the size of the matrix						
#define SP_RGB_ROWS 		SP_ROWS * SP_RGB_WIDTH			// Each SplashPixel is 4x4, work out how big the matrix is base upon the size of the matrix	
#define SP_RGB_TOTAL		SP_RGB_COLUMNS * SP_RGB_ROWS	// Total No of RGB Leds in the Matrix
#define SP_LED_TOTAL		SP_RGB_TOTAL * 3				// There are 3 LEDs per RGB
#define SP_BITS_PER_LED		12								// 12bits resolution per LED

#define SPI_FIFO_SIZE		8								// Depth of the SPI Fifo

// RGB Struct, each pixel in the frame buffer is an instance of this structure
typedef struct RGB_T
{
	ui16	blue;
	ui16	red;
	ui16	green;
} RGB_T;


void SP_Task( void );
void SP_Init( void );
bool SP_FeedTheFifo( bool resetBufferPos );
void SP_LatchControl( ui8 loadLatch );
void SP_BlankControl( ui8 blankOutput );
void SP_ClearFrameBuffer( void );
void SP_PackUpdate ( ui8 BoardNo );
bool SP_SetPixel( RGB_T *newPixel, ui16 row, ui16 column );
void SP_SetAllPixels( RGB_T *newPixel );
bool SP_DrawSquare( RGB_T *lineColour, ui16 row, ui16 column, ui16 width, ui16 height, ui8 lineThickness, bool fillSq, RGB_T *fillColour );
bool SP_DrawHorizontalLine( RGB_T *lineColour, ui16 row, ui16 column, ui16 len, ui8 lineThickness );
bool SP_DrawVerticalLine( RGB_T *lineColour, ui16 row, ui16 column, ui16 len, ui8 lineThickness );
void SP_FrameSync( void );
void SP_FifoEmptyEvent ( void );
bool SP_IsFifoEmpty ( void );
bool SP_ResetFifoEmptyFlag ( void );

#ifdef _SPLASH_PIXEL_H_

// Frame buffer, used as an interface between app code and splashPixel updating
RGB_T frameBuffer[ SP_RGB_ROWS ][ SP_RGB_COLUMNS ];

// Buffer used to update a single splash pixel
// we keep this buffer unpacked, it will hold each 12bit value for each led on a single splashpixel
ui32 SplashPixelBuf[ SP_LEDS ];

bool SpFifoEmptied = false;

ui8 SpPause = false;

#else

extern RGB_T frameBuffer[ SP_RGB_ROWS ][ SP_RGB_COLUMNS ];

#endif

#define SP_FRAMEBUFFER_SIZE 	sizeof(frameBuffer)
