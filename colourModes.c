/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2011 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#define COLOUR_MODES
#include "SplashBaseHeaders.h"

/*
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "boards/dk-lm3s9d96/enet_ptpd/random.h"

#include "datatypes.h"

#include "pwmControl.h"

#define COLOUR_MODES
#include "colourModes.h"
*/



ui8 ColourCurrentMode = COLOUR_MODE_BOUNCE;

// mode - change to supplied mode, see COLOUR_MODES enum
// colourOne,colourTwo - bits0-7 = Blue, bits 8-15 = Green, bits 16-23 = Red 0xRRGGBB
// mutiple of the 10ms tick rate, used to delay the next adjustment
void ColourModeSet( ui8 mode, ui32 colourOne, ui32 colourTwo, ui16 noOfSteps, ui8 ticksPerStep )
{
	ui16 pwmFreq;
	pwmFreq = pwmGetFreq() -1;
	// Bounce - alternate between colourOne and Two
	
	// Rotate, ignore colourOne + two, use timing to set the rotation rate
	
	// Flash - off to ColourOne 50% of the timing value
	
	// off
	
	if ( ColourSettings.colourMode != mode )
	{
		// Mode Change!
		ColourSettings.colourMode = mode;
		
		// clear down the current set dutys
		pwmSetDuty(pwmFreq, 0x07);
	}
	ColourSettings.maxSteps = noOfSteps;
	ColourSettings.ticksPerStep = ticksPerStep;
	
	ColourSettings.topRed = 0x000000FF & (colourOne >> 16);
	ColourSettings.topGreen = 0x000000FF & (colourOne >> 8);
	ColourSettings.topBlue = 0x000000FF & colourOne;

	ColourSettings.bottomRed = 0x000000FF & (colourTwo >> 16);
	ColourSettings.bottomGreen = 0x000000FF & (colourTwo >> 8);
	ColourSettings.bottomBlue = 0x000000FF & colourTwo;	
	
	if (ColourSettings.colourMode == COLOUR_MODE_MANUAL)
	{
		ColourModeManual();
	}
}

void ColourModeManual ( void )
{
ui16 pwmFreq;
volatile float freqScale;
volatile float tempFloat;

	pwmFreq = pwmGetFreq() -1;
 	freqScale = pwmFreq;
 	freqScale /= 255;	
 	
 	tempFloat = pwmFreq - (ColourSettings.topRed * freqScale);
	pwmSetDuty((ui16)(tempFloat), COLOUR_RED_PWM_CH);
	
	tempFloat = pwmFreq - (ColourSettings.topGreen * freqScale);
	pwmSetDuty((ui16)(tempFloat), COLOUR_GREEN_PWM_CH);
	
	tempFloat = pwmFreq - (ColourSettings.topBlue * freqScale);
	pwmSetDuty((ui16)(tempFloat), COLOUR_BLUE_PWM_CH);
}

void ColourModeTick( void )
{
static ui32 counter = 0;

	// count
	counter ++;
	
 	if (counter >= ColourSettings.ticksPerStep)
 	{
 		counter = 0;
 		
 		switch ( ColourSettings.colourMode )
 		{
 			case COLOUR_MODE_CYCLE :
 				ColourModeCycle();
 			break;
 			
 			case COLOUR_MODE_BOUNCE :
 				ColourModeBounce();
 			break;
 			
 			/*case COLOUR_MODE_FLASH :
 				ColourModeFlash();
 			break;*/
 			
 			case COLOUR_MODE_BOUNCE_RAND :
 				ColourModeRandomBounce();
 			break;
 		}

 	}
	
}

enum COLOUR_MIX
{
	RGMIX = 0,
	GBMIX,
	BRMIX
};

void ColourModeCycle ( void )
{
static ui16 red = 0;
static ui16 green = 0;
static ui16 blue = 0;
static ui8 mixState = 0;

static ui8 redDescending = 0;
static ui8 greenDescending = 0;
static ui8 blueDescending = 0;

ui16 pwmFreq = pwmGetFreq() -1;
volatile float freqScale;
volatile float tempFloat;

	freqScale = pwmFreq;
 	freqScale /= ColourSettings.maxSteps;
	
	switch(mixState)
	{
		case RGMIX:
	
			blue = 0;
		
			if(green < ColourSettings.maxSteps)
			{
				green++;
				redDescending = false;
			}
			else
			{
				redDescending = true;
			}
		
			if(redDescending)
			{
				if ( red > ColourSettings.maxSteps) red = ColourSettings.maxSteps; 
				
				if (red)
				{
					red--;
				}
				if(!red)
				{
					redDescending = false;
					mixState = GBMIX;
				}
			}
	
		break;

		case GBMIX:

				red = 0;

				if(blue < ColourSettings.maxSteps)
				{
					blue++;
					greenDescending = false;
				}
				else
				{
					greenDescending = true;
				}
	
				if(greenDescending)
				{
					if ( green > ColourSettings.maxSteps) green = ColourSettings.maxSteps; 
					if (green)
					{
						green--;
					}
					if(!green)
					{
						greenDescending = false;
						mixState = BRMIX;
					}
				}
			
		break;

		case BRMIX:
	
			green = 0;
	
			if(red < ColourSettings.maxSteps)
			{
				red++;
				blueDescending = false;
			}
			else
			{
				blueDescending = true;
			}
	
			if(blueDescending)
			{
				if ( blue > ColourSettings.maxSteps) blue = ColourSettings.maxSteps; 
				if (blue)
				{
					blue--;
				}
				if(!blue)
				{
					blueDescending = false;
					mixState = RGMIX;
				}
			}
			
		break;
	}
	
	tempFloat = pwmFreq - (red * freqScale);
	pwmSetDuty((ui16)(tempFloat), COLOUR_RED_PWM_CH);
	
	tempFloat = pwmFreq - (green * freqScale);
	pwmSetDuty((ui16)(tempFloat), COLOUR_GREEN_PWM_CH);
	
	tempFloat = pwmFreq - (blue * freqScale);
	pwmSetDuty((ui16)(tempFloat), COLOUR_BLUE_PWM_CH);
	
	// Update the Colours by first finding the ratio of step counts to 255
	tempFloat = ColourSettings.maxSteps / 255;
	ColourSettings.topRed = red * tempFloat;
	ColourSettings.topGreen = green * tempFloat;
	ColourSettings.topBlue = blue * tempFloat;
}

ui8 ColourModeBounce ( void )
{
static ui8 goingUp = 1;
static ui16 stepCnt;

volatile float red = 0;
volatile float green = 0;
volatile float blue = 0;

volatile ui16 temp;
volatile float freqScale;
volatile ui16 pwmFreq;

bool result = 0;
 	
 	if (goingUp)
 	{
 		stepCnt ++;

 		if ( stepCnt >= ColourSettings.maxSteps )
 		{
 			goingUp = 0;
 			stepCnt = ColourSettings.maxSteps;
 			
 			// let a calling function know we have completed a cycle
 			result = 2;
 		}
 	}
 	else
 	{
 		// if a change occurs to the max steps while counting down, bring the counts back into range
 		if ( stepCnt >= ColourSettings.maxSteps )
 		{
 			stepCnt = ColourSettings.maxSteps;
 		}
 		
 		stepCnt --;
 		
 		if ( 0 == stepCnt )
 		{
 			goingUp = 1;
 			// let a calling function know we have completed a cycle
 			result = 1;
 		}
 	}	
 	
 	pwmFreq = pwmGetFreq() -1;
 	freqScale = pwmFreq;
 	freqScale /= 255;
 	
 	// RED -------------------------------------------
 	// Work out the direction of travel
 	if (ColourSettings.topRed > ColourSettings.bottomRed)
 	{		
 		// work counts per step in our range, then multiple it by the current step
 		red = (float)((float)(ColourSettings.topRed - ColourSettings.bottomRed) / (float)ColourSettings.maxSteps ) * stepCnt;
 		red += ColourSettings.bottomRed;
 		red *= freqScale;
 		
 		temp = pwmFreq - red;
 		pwmSetDuty(temp, COLOUR_RED_PWM_CH);
 	}
 	else
 	{
 		// work counts per step in our range, then multiple it by the current step
 		red = (float)((float)(ColourSettings.bottomRed - ColourSettings.topRed) / (float)ColourSettings.maxSteps ) * stepCnt;
 		red = ColourSettings.bottomRed - red;
 		red *= freqScale;
 		
 		temp = pwmFreq - red;
 		pwmSetDuty(temp, COLOUR_RED_PWM_CH);
 	}
 	// RED ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 	
 	// Green -------------------------------------------
 	// Work out the direction of travel
 	if (ColourSettings.topGreen > ColourSettings.bottomGreen)
 	{		
 		// work counts per step in our range, then multiple it by the current step
 		green = (float)((float)(ColourSettings.topGreen - ColourSettings.bottomGreen) / (float)ColourSettings.maxSteps ) * stepCnt;
 		green += ColourSettings.bottomGreen;
 		green *= freqScale;
 		
 		temp = pwmFreq - green;
 		pwmSetDuty(temp, COLOUR_GREEN_PWM_CH);
 	}
 	else
 	{
 		// work counts per step in our range, then multiple it by the current step
 		green = (float)((float)(ColourSettings.bottomGreen - ColourSettings.topGreen) / (float)ColourSettings.maxSteps ) * stepCnt;
 		green = ColourSettings.bottomGreen - green;
 		green *= freqScale;
 		
 		temp = pwmFreq - green;
 		pwmSetDuty(temp, COLOUR_GREEN_PWM_CH);
 	}
 	
 	// Blue -------------------------------------------
 	// Work out the direction of travel
 	if (ColourSettings.topBlue > ColourSettings.bottomBlue)
 	{		
 		// work counts per step in our range, then multiple it by the current step
 		blue = (float)((float)(ColourSettings.topBlue - ColourSettings.bottomBlue) / (float)ColourSettings.maxSteps ) * stepCnt;
 		blue += ColourSettings.bottomBlue;
 		blue *= freqScale;
 		
 		temp = pwmFreq - blue;
 		pwmSetDuty(temp, COLOUR_BLUE_PWM_CH);
 	}
 	else
 	{
 		// work counts per step in our range, then multiple it by the current step
 		blue = (float)((float)(ColourSettings.bottomBlue - ColourSettings.topBlue) / (float)ColourSettings.maxSteps ) * stepCnt;
 		blue = ColourSettings.bottomBlue - blue;
 		blue *= freqScale;
 		
 		temp = pwmFreq - blue;
 		pwmSetDuty(temp, COLOUR_BLUE_PWM_CH);
 	}
	
	return( result );
}

void ColourModeFlash ( void )
{
	
}

void ColourModeRandomBounce ( void )
{
	ui32 random;
	ui8 bounceCycle;
	
	bounceCycle = ColourModeBounce();
	
	// Same as bounce but we randomise the value after each cycle
	if ( 1 == bounceCycle )
	{
		random = RandomNumber();
		
		ColourSettings.topRed = 0x000000FF & (random >> 16);
		ColourSettings.topGreen = 0x000000FF & (random >> 8);
		ColourSettings.topBlue = 0x000000FF & random;
	}
	else if ( 2 == bounceCycle )
	{
		random = RandomNumber();
		
		ColourSettings.bottomRed = 0x000000FF & (random >> 16);
		ColourSettings.bottomGreen = 0x000000FF & (random >> 8);
		ColourSettings.bottomBlue = 0x000000FF & random;
	}	
	else
	{
		// It's still busy	
	}
}

