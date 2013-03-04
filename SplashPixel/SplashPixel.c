/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013.03 - www.soldersplash.co.uk - C. Matthews - R. Steel

 SplashPixel Frame buffer

*/

// SP numbering, a single SP bus that snakes rather than ordered nicely
// 0  1  2
// 5  4  3
// 6  7  8
// Assuming the start of the SPI chain is board 0 and that is in the top left

// The FrameBuffer ordered logically into columns and rows

#include "globals.h"

#ifdef SPLASHPIXEL_ENABLED

#include "datatypes.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"

#include <string.h>
#include "SpiMaster.h"

#include "inc\LM3S6432.h"

#define _SPLASH_PIXEL_H_
#include "SplashPixel.h"


// --------------------------------------------------------------------------------------
// SP_Init
// Initalise the SplashPixel control lines, bus and framebuffer
// --------------------------------------------------------------------------------------
void SP_Init( void )
{
	// TODO : Set up the latch and blank directions
	
	// Set up Latch and Blank Pins
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_7);
	//GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
	
	SP_LatchControl( false );
	SP_BlankControl(true);
	
	// Enable the output
	// TODO : might want to blank it until the first framebuffers worth has been loaded
	SP_BlankControl( false );
		
	SP_ClearFrameBuffer();
	
	// Sets up the SPI interface
	SPI_Init();
}

// --------------------------------------------------------------------------------------
// SP_Task
// Called regularly, this will make sure the SPI Fifo is topped up and load the latch
// on the TLC5947 when a full update is complete.
// --------------------------------------------------------------------------------------
void SP_Task( void )
{
	static ui8 currentBoard = SP_COUNT - 1;
	static ui8 currentState = 0;
	static ui16 delay = 0;
		
	switch ( currentState )
	{
		case 0 :
			
			SP_PackUpdate( currentBoard );
			
			SP_ResetFifoEmptyFlag();
						
			// Start feeding the fifo from the begining of the buffer
			SP_FeedTheFifo( true );
			
			// Enable the SPI TX empty interrupt
			SSI_TxFifoIntEnable();
			
			currentState = 1;
			//delay = 40;
			delay = 10;
		break;
		
		case 1 :
		
			// fifo is topped up by the interrupt on the FIFO level.
			// Keep feeding it			
			//if ( SP_FeedTheFifo( false ) )
			if ( SP_IsFifoEmpty() )
			{
				if ( currentBoard ) 
				{
					currentBoard --;
					
					// Another board to do
					//currentState = 0;
					
					SP_PackUpdate( currentBoard );
			
					SP_ResetFifoEmptyFlag();
								
					// Start feeding the fifo from the begining of the buffer
					SP_FeedTheFifo( true );
					
					// Enable the SPI TX empty interrupt
					SSI_TxFifoIntEnable();
				}
				else
				{
					// That was the last board to update
					// Lets wait for the FIFO to drain
					currentState = 2;	
				}
			}
			else
			{
				// still have data left to send ...
					
			}	
								
		break;
		
		case 2 :
			// Wait until the FIFO is empty
			if (! SPI_Busy() )
			{
				// Set latch to apply the update
				currentState = 3;
				SP_BlankControl( true );
				SP_LatchControl( true );
				SP_LatchControl( false );
				SP_BlankControl( false );
			}
			
		break;
		
		case 3 :
			// Start Again!
			if (delay)
			{
				delay --;	
			}
			else
			{				
				currentBoard = SP_COUNT - 1;
				currentState = 0;
				/*
				SP_LatchControl( true );
				SP_BlankControl( true );
				SP_BlankControl( false );
				SP_LatchControl( false );
				*/
				if (! SpPause) SP_FrameSync();
				delay = 0;
			}	
		break;
		
		case 4 :
		if (delay)
			{
				delay --;	
			}
			else
			{	
				currentState = 0;	
			}
		
		break;
	}
	
}

void SP_FrameSync( void )
{
	static RGB_T lineColour;	
	static ui8 row = 0;
	static ui8 column = 0;
	static ui8 width = 0;
	
	static ui8 mode = 0;
	static volatile ui8 delayLevel = 1;
	static ui8 delay = 1;
	
	delay --;
	
	if ( delay > 0 ) return;
		
	delay = delayLevel;
	// A single frame has gone ...
	
	/*
	if (mode == 0)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		SP_SetAllPixels(&lineColour);
		
		lineColour.blue = 255;
		SP_DrawHorizontalLine( &lineColour, row, 0, SP_RGB_COLUMNS, 1 );
		
		row ++;
	   	if (row >= SP_RGB_ROWS)
	   	{
	   		row = 0;
	   		mode = 1;
	   	}
	}
	else if (mode == 1)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		SP_SetAllPixels(&lineColour);
		
		lineColour.green = 255;
		SP_DrawHorizontalLine( &lineColour, row, 0, SP_RGB_COLUMNS, 1 );
		
		row ++;
	   	if (row >= SP_RGB_ROWS)
	   	{
	   		row = 0;
	   		mode = 2;
	   	}	
	}
	else if (mode == 2)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		SP_SetAllPixels(&lineColour);
		
		lineColour.blue = 255;
		SP_DrawHorizontalLine( &lineColour, row, 0, SP_RGB_COLUMNS, 1 );
		
		row ++;
	   	if (row >= SP_RGB_ROWS)
	   	{
	   		row = 0;
	   		mode = 3;
	   		column = 0;
	   	}
	}
	else if (mode == 3)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		SP_SetAllPixels(&lineColour);
		
		lineColour.red = 255;
		SP_DrawVerticalLine( &lineColour, 0, column, SP_RGB_ROWS, 1 );
		
		column ++;
	   	if (column > SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		mode = 4;
	   	}
	}
	else if (mode == 4)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		SP_SetAllPixels(&lineColour);
		
		lineColour.green = 255;
		SP_DrawVerticalLine( &lineColour, 0, column, SP_RGB_ROWS, 1 );
		
		column ++;
	   	if (column > SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		mode = 5;
	   	}
	}
	else if (mode == 5)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		SP_SetAllPixels(&lineColour);
		
		lineColour.blue = 255;
		SP_DrawVerticalLine( &lineColour, 0, column, SP_RGB_ROWS, 1 );
		
		column ++;
	   	if (column > SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		mode = 6;
	   	}
	}
	else if (mode == 6)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		//SP_SetAllPixels(&lineColour);
		
		lineColour.red = 255;
		SP_DrawVerticalLine( &lineColour, 0, column, SP_RGB_ROWS, 1 );
		
		column ++;
	   	if (column > SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		mode = 7;
	   	}
	}
	else if (mode == 7)
	{
		lineColour.red = 0;
		lineColour.green = 0;
		lineColour.blue = 0;
		
		//SP_SetAllPixels(&lineColour);
		
		lineColour.green = 255;
		SP_DrawVerticalLine( &lineColour, 0, column, SP_RGB_ROWS, 1 );
		
		column ++;
	   	if (column > SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		mode = 8;
	   	}
	}
	else if (mode == 8)
	{	
		lineColour.blue = 255;
		SP_DrawVerticalLine( &lineColour, 0, column, SP_RGB_ROWS, 1 );
		
		column ++;
	   	if (column > SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		mode = 9;
	   		lineColour.red = 0;
			lineColour.green = 0;
			lineColour.blue = 0;
			
			SP_SetAllPixels(&lineColour);
	   	}
	}
	if (mode == 9)
	{	
		lineColour.blue = 255;
		SP_DrawHorizontalLine( &lineColour, row, 0, SP_RGB_COLUMNS, 1 );
		
		row ++;
	   	if (row >= SP_RGB_ROWS)
	   	{
	   		row = 0;
	   		mode = 10;
	   	}
	}
	else if (mode == 10)
	{		
		lineColour.green = 255;
		SP_DrawHorizontalLine( &lineColour, row, 0, SP_RGB_COLUMNS, 1 );
		
		row ++;
	   	if (row >= SP_RGB_ROWS)
	   	{
	   		row = 0;
	   		mode = 11;
	   	}	
	}
	else if (mode == 11)
	{
		lineColour.red = 255;
		SP_DrawHorizontalLine( &lineColour, row, 0, SP_RGB_COLUMNS, 1 );
		
		row ++;
	   	if (row >= SP_RGB_ROWS)
	   	{
	   		row = 0;
	   		mode = 12;
	   		column = 0;
	   		
	   		lineColour.red = 0;
			lineColour.green = 0;
			lineColour.blue = 0;
		
			SP_SetAllPixels(&lineColour);
	   	}
	}
	else if (mode == 12)
	{
		lineColour.red = 0x1FF;
		SP_SetPixel(&lineColour, row, column);	
		
		column ++;
		if (column >= SP_RGB_COLUMNS)
	   	{
	   		column = 0;
	   		row ++;
	   		if (row >= SP_RGB_ROWS)
	   		{
	   			mode = 13;
	   			lineColour.red = 0;
				lineColour.green = 0;
				lineColour.blue = 0;
	   		}
	   	}
	}
	else if (mode == 13)
	{
		lineColour.blue = 0x1FF;
		SP_SetPixel(&lineColour, row, column);	
		
		if (column) 
		{	
			column --;
		}
		else
		{
			column = SP_RGB_COLUMNS -1;
			
			if (row)
			{
				row--;
			}
			else
			{
				row = 0;
	   			mode = 14;
	   			column = 0;
	   			width = 12;
	   			lineColour.red = 0;
				lineColour.green = 0;
				lineColour.blue = 0;
	   			SP_SetAllPixels(&lineColour);
	   			lineColour.red = 0x1FF;
			}
		}
	}
	else if (mode == 14)
	{
		SP_DrawSquare(&lineColour, row, column, width, width, 1, false, &lineColour);
		
		width = width - 2;	
		row ++;
		column ++;
		delay = 10;
		
		if (column > 5)
		{
			row = 0;
	   		mode = 15;
	   		column = 0;
	   		
	   		lineColour.red = 255;
			lineColour.green = 255;
			lineColour.blue = 255;
	   		SP_SetAllPixels(&lineColour);
		}
		else if (column == 1)
		{
			lineColour.red = 0;
			lineColour.green = 0x1FF;
			lineColour.blue = 0;
		}
		else if (column == 2)
		{
			lineColour.red = 0;
			lineColour.green = 0;
			lineColour.blue = 0x1FF;
		}
		else if (column == 3)
		{
			lineColour.red = 0x1FF;
			lineColour.green = 0;
			lineColour.blue = 0x1FF;
		}
		else if (column == 4)
		{
			lineColour.red = 0;
			lineColour.green = 0x1FF;
			lineColour.blue = 0x1FF;
		}
		else if (column == 5)
		{
			lineColour.red = 0x1FF;
			lineColour.green = 0x1FF;
			lineColour.blue = 0x1FF;
		}
	}
	else if (mode == 15)
	{
		row = 0;
	    mode = 0;
	   	column = 0;	
	   	
	   	lineColour.red = 255;
		lineColour.green = 255;
		lineColour.blue = 255;
	   	SP_SetAllPixels(&lineColour);
	   	
	   	delay = 50;
	}
	*/
	
	/*
    lineColour.red = 0;
	lineColour.green = 0;
	lineColour.blue = 0;
    
   	SP_SetPixel( &lineColour, row, column );
   	
   	row ++;
   	if (row > 3)
   	{
   		row = 0;
   		column ++;
   		if (column>7) column = 0;
   	}
   	
   	lineColour.red = 255;
	lineColour.green = 0;
	lineColour.blue = 0;
    
   	SP_SetPixel( &lineColour, row, column );	
   	*/
}

// Will stop incrementing the pattern but continue to refresh the sp's
void SP_Pause( ui8 pause )
{
	SpPause = pause;
}

void SP_FifoEmptyEvent ( void )
{
	SpFifoEmptied = true;
}

bool SP_IsFifoEmpty ( void )
{
	return( SpFifoEmptied );
}

bool SP_ResetFifoEmptyFlag ( void )
{
	SpFifoEmptied = false;
}

// Accessor function used by the interrupt to top up the fifo.
bool SP_FeedTheFifo( bool resetBufferPos )
{
ui8 i;
static ui8 pixelBufPos = SP_LEDS;
bool result = true;

	// reset the buffer pos if asked too
	if ( resetBufferPos ) pixelBufPos = 0;

	// if everything's been sent theres nothing to do
	if ( pixelBufPos >= SP_LEDS )
	{
		// Everything has been sent
		result = false;
		SP_FifoEmptyEvent();
		
		// Disable TX Interrupt if needed?
	}
	else
	{
		// Load up the SPI FIFO
		for (i=0; i<SPI_FIFO_SIZE; i++)
		{
			if ( SPI_AddToFifo( SplashPixelBuf[pixelBufPos] ) )
			{
				// We have added one to the FIFO
				pixelBufPos ++;
				
				// Anthing Left to send?
				if ( pixelBufPos >= SP_LEDS )
				{
					// Inform the calling theres nothing to do
					result = false;
					SP_FifoEmptyEvent();
					break;
				}
				else
				{
					// More to send
				}
			}	
			else
			{
				// no room in the FIFO, lets stop the for loop
				break;	
			}
		}	
	}
	
	return( result );
}

// --------------------------------------------------------------------------------------
// SP_PackUpdate - Pack an update for a single SplashPixel Board
// Board no is 0 based
// --------------------------------------------------------------------------------------
void SP_CopyFrameBuffer ( ui16 offset, ui8 *buffer, ui16 length )
{
volatile ui8 *framePtr;

	framePtr = (ui8 *)frameBuffer;

	memcpy( &framePtr[offset], buffer, length);
}

// --------------------------------------------------------------------------------------
// SP_PackUpdate - Pack an update for a single SplashPixel Board
// Board no is 0 based
// --------------------------------------------------------------------------------------
void SP_PackUpdate ( ui8 BoardNo )
{
ui8 i = 0;	
ui8 packType = 0;	
ui8 fbRowPos = 0;
ui8 fbColPos = 0;
ui8 row = 0;
ui8 column = 0;
ui8 rowOffset = 0;
ui8 colOffset = 0;

#define NORTH	0
#define EAST	1
#define SOUTH	2
	
	// Use SP_WIDTH & SP_HEIGHT to calulate the position of the SplashPixel in the Framebuffer

	// Calculate the Row Offset for the selected BoardNo
	rowOffset = (BoardNo / SP_COLUMNS) * SP_RGB_WIDTH;

	
	// Column Offset varries as the spi snakes between rows of SplashPixels
	// if its an odd row it's right to left, even left to right
	if (( BoardNo / SP_COLUMNS ) % 2)
	{
		// odd
		colOffset = ((SP_COLUMNS - 1) - (BoardNo % SP_COLUMNS)) * SP_RGB_WIDTH;
	}
	else
	{
		// even
		colOffset = (BoardNo % SP_COLUMNS) * SP_RGB_WIDTH;
	} 
			  
	// Loop around 4 rows and 4 columns
	
	// Should not be needed
	i = 0;

	//board 0 is special case - its always north
	if(BoardNo == 0 )
	{
		packType = NORTH;
	}

	//else if its an east facing
	else if((BoardNo % SP_COLUMNS) == 0 )
	{
		packType = EAST;
	}
	//else if its odd row then its north
	else if (( BoardNo / SP_COLUMNS ) % 2)
	{
		packType = SOUTH;
	}
	//else an even row is south
	else
	{
		packType = NORTH;
	}
	
	switch (packType)
	{
		case NORTH:

			for(row = 0; row < SP_RGB_WIDTH; row++)
			{
				for(column = SP_RGB_WIDTH; column > 0; column--)
				{
					fbRowPos = rowOffset + row;
					fbColPos = colOffset + (column-1);
					
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].blue;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].red;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].green;
				}
			}
			break;


		case EAST:

			for(column = SP_RGB_WIDTH; column > 0; column--)
			{
				for(row = SP_RGB_WIDTH; row > 0; row--)
				{
					fbRowPos = rowOffset + (row-1);
					fbColPos = colOffset + (column-1);
					
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].blue;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].red;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].green;
				}
			}
			break;

		case SOUTH:

			for(row = SP_RGB_WIDTH; row > 0; row--)
			{
				for(column = 0; column < SP_RGB_WIDTH; column++)
				{
					fbRowPos = rowOffset + (row-1);
					fbColPos = colOffset + column;
					
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].blue;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].red;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].green;
				}
			}
			break;
	}
		
	
	// SplashPixelBuf can now be sent via the bus, this buffer contains the values for one board
}

/*

// --------------------------------------------------------------------------------------
// SP_PackUpdate - Pack an update for a single SplashPixel Board
// Board no is 0 based
// --------------------------------------------------------------------------------------
void SP_PackUpdate ( ui8 BoardNo )
{
ui8 i = 0;	
ui8 fbRowPos = 0;
ui8 fbColPos = 0;
ui8 row = 0;
ui8 column = 0;
ui8 rowOffset = 0;
ui8 colOffset = 0;
	
	// Use SP_WIDTH & SP_HEIGHT to calulate the position of the SplashPixel in the Framebuffer

	// Calculate the Row Offset for the selected BoardNo
	// TODO : type cast the divide result
	rowOffset = (BoardNo / SP_COLUMNS) * SP_RGB_WIDTH;

	
	// Column Offset varries as the spi snakes between rows of SplashPixels
	// if its an odd row it's right to left, even left to right
	if (( BoardNo / SP_COLUMNS ) % 2)
	{
		// odd
		colOffset = ((SP_COLUMNS - 1) - (BoardNo % SP_COLUMNS)) * SP_RGB_WIDTH;
	}
	else
	{
		// even
		colOffset = (BoardNo % SP_COLUMNS) * SP_RGB_WIDTH;
	} 
			  
	// Loop around 4 rows and 4 columns
	
	// Should not be needed
	i = 0;
	
	if (( BoardNo / SP_COLUMNS ) % 2)
	{
		// Odd Row
		
		// For a board facing east, on a odd row at the end..
		for( column=SP_RGB_WIDTH; column>0; column--)
		{
			for( row=SP_RGB_WIDTH; row>0; row-- )
			{
				// Calc framebuffer pos
				fbRowPos = rowOffset + (row-1);
				fbColPos = colOffset + (column-1);
				
				SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].blue;
				SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].red;
				SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].green;
			}
		}
	}
	else
	{
		// Even Row
				
		// On Even rows the first board of the row points east
		// Board 0, the entry board, is a special case that is exempt from this rule
//			
//			if ((0 != BoardNo) && (first board in the row))
//			{
//				
//			}
//			else
//			{
//				
			// TLC5947's expect the last channel first, each splash pixel board's RGBs are organised like so :
			// 5 6 7 8
			// 1 2 3 4
			// 5 6 7 8
			// 1 2 3 4
			// you start from the top row updating RGB no 8 of the second TLC5947 
			for( row=0; row<SP_RGB_WIDTH; row++ )
			{
				for( column=SP_RGB_WIDTH; column>0; column--)
				{
					fbRowPos = rowOffset + row;
					fbColPos = colOffset + (column-1);
					
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].blue;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].red;
					SplashPixelBuf[i++] = frameBuffer[fbRowPos][fbColPos].green;
				}
			}
		//}
	}
	
	
	// SplashPixelBuf can now be sent via the bus, this buffer contains the values for one board
}
*/

// --------------------------------------------------------------------------------------
// SP_Init
// Set a single pixel to a colour
// --------------------------------------------------------------------------------------
bool SP_SetPixel( RGB_T *newPixel, ui16 row, ui16 column )
{
bool result = false;

	// bound check the supplied row & column
	if (( row < SP_RGB_ROWS ) && ( column < SP_RGB_COLUMNS ))
	{
		// Edit the frame buffer
		frameBuffer[row][column].red = newPixel->red;
		frameBuffer[row][column].green = newPixel->green;
		frameBuffer[row][column].blue = newPixel->blue;
		
		result = true;
	}
	
	return( result );
}

// --------------------------------------------------------------------------------------
// SP_SetAllPixels
// Sets all pixels the same colour
// --------------------------------------------------------------------------------------
void SP_SetAllPixels( RGB_T *newPixel )
{
ui16 row;
ui16 column;

	for ( row=0; row< SP_RGB_ROWS; row++ )
	{
		for ( column=0; column<SP_RGB_COLUMNS; column++ )
		{	
			frameBuffer[row][column].red = newPixel->red;
			frameBuffer[row][column].green = newPixel->green;
			frameBuffer[row][column].blue = newPixel->blue;
		}
	}
}

// --------------------------------------------------------------------------------------
// SP_DrawVerticalLine
// lineColour - RGB colour of the lane, pointer to RGB_T
// start row & column for the line
// lineThickness, thickness in pixels-1 for the line, thickness of 0 is 1 pixel wide
// --------------------------------------------------------------------------------------
bool SP_DrawVerticalLine( RGB_T *lineColour, ui16 row, ui16 column, ui16 len, ui8 lineThickness )
{
bool result = false;
ui8 x = 0;
ui8 y = 0;
	
	// bound check, line can't be too long
	if (( row+len <= SP_RGB_ROWS ) && ( column+lineThickness <= SP_RGB_COLUMNS ))
	{
		for ( y = row; y < row+len; y++ )
		{
			for ( x = column; x < column + lineThickness; x++ )
			{
				frameBuffer[y][x].red = lineColour->red;
				frameBuffer[y][x].green = lineColour->green;
				frameBuffer[y][x].blue = lineColour->blue;	
			}
		}
		
		result = true;
	}
	
	return( result );
}

// --------------------------------------------------------------------------------------
// SP_DrawHorizontalLine
// lineColour - RGB colour of the lane, pointer to RGB_T
// start row & column for the line
// lineThickness, thickness in pixels-1 for the line, thickness of 0 is 1 pixel wide
// --------------------------------------------------------------------------------------
bool SP_DrawHorizontalLine( RGB_T *lineColour, ui16 row, ui16 column, ui16 len, ui8 lineThickness )
{
bool result = false;
ui8 x = 0;
ui8 y = 0;
	
	// bound check, line can't be too long
	if (( column+len <= SP_RGB_COLUMNS ) && ( row+lineThickness <= SP_RGB_ROWS ))
	{
		
		for ( x = column; x < column + len; x++ )
		{
			for ( y = row; y < row + lineThickness; y++ )
			{
				frameBuffer[y][x].red = lineColour->red;
				frameBuffer[y][x].green = lineColour->green;
				frameBuffer[y][x].blue = lineColour->blue;	
			}
		}
		
		result = true;
	}
	
	return( result );
}

// --------------------------------------------------------------------------------------
// SP_DrawSquare
// Draw a square into the frame buffer, specify the top left coordinates and the bottom right.
// lineThickness is specified in terms of number of pixel's (RBGS leds wide)
// if fillSq is true, the inside of the square is set to the supplied colour
// --------------------------------------------------------------------------------------
bool SP_DrawSquare( RGB_T *lineColour, ui16 row, ui16 column, ui16 width, ui16 height, ui8 lineThickness, bool fillSq, RGB_T *fillColour )
{
bool result = false;
bool drawLine = false;
ui8 x = 0;
ui8 y = 0;

	// bound check, line can't be too long
	if (( column + width <= SP_RGB_COLUMNS ) && ( row + height <= SP_RGB_ROWS ))
	{
		for ( x = column; x < column + height; x++ )
		{
			for ( y = row; y < row + width; y++ )
			{
				if (fillSq)
				{
					frameBuffer[y][x].red = fillColour->red;
					frameBuffer[y][x].green = fillColour->green;
					frameBuffer[y][x].blue = fillColour->blue;
				}
				
				if (( y >= row ) && (y <= row + lineThickness ))
				{
					// top line
					drawLine = true;
				}
				else if (( y >= (row + width) - lineThickness ) && ( y <= (row + width) ))
				{
					// bottom line
					drawLine = true;
				}
				else if (( x >= column ) && (x <= column + lineThickness ))
				{
					// top line
					drawLine = true;
				}
				else if (( x >= (column + height) - lineThickness ) && ( y <= (column + height) ))
				{
					// bottom line
					drawLine = true;
				}
								
				if ( drawLine ) 
				{
					frameBuffer[y][x].red = lineColour->red;
					frameBuffer[y][x].green = lineColour->green;
					frameBuffer[y][x].blue = lineColour->blue;
				}
			}
			
		}
		
		result = true;
	}
	
	return( result );
}

// --------------------------------------------------------------------------------------
// SP_ClearFrameBuffer - Clear the entire framebuffer down
// --------------------------------------------------------------------------------------
void SP_ClearFrameBuffer( void )
{
	// TODO : does size of work here ..
	memset(&frameBuffer, 0, sizeof(frameBuffer));
}

// --------------------------------------------------------------------------------------
// SP_LatchControl - Control the latch line to the TLC5946
// --------------------------------------------------------------------------------------
void SP_LatchControl( ui8 loadLatch )
{
	// Latch rising from low to high applys the data in the shift register to the latch/output	
	
	if ( loadLatch )
	{
		// Go from a low to a high	
		GPIO_PORTC_DATA_R |= BIT5;
	}
	else
	{
		// pull line low
		GPIO_PORTC_DATA_R &= ~BIT5;
	}
}

// --------------------------------------------------------------------------------------
// SP_LatchControl - Control the Blank line to the TLC5946
// --------------------------------------------------------------------------------------
void SP_BlankControl( ui8 blankOutput )
{
	// While BLANK is high all outputs are switched off, on transition to low the values in the latch registers are applied	
	
	if ( blankOutput )
	{
		// Set blank high
		GPIO_PORTC_DATA_R |= BIT7;
	}
	else
	{
		// clear the blank line
		GPIO_PORTC_DATA_R &= ~BIT7;
	}
}

#endif

