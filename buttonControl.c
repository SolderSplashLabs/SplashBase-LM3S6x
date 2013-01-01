/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Redistributions of source code must retain the above copyright notice

*/

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "datatypes.h"

#include "buttonControl.h"

ui32 Button1PressedCounter = 0;
ui32 Button2PressedCounter = 0;
ui8 ButtonsHeld = 0;
ui8 ButtonsPressed = 0;

//*****************************************************************************
//
// ButtonInit - Set up the GPIO
//
//*****************************************************************************
void Buttons_Init ( void )
{
	// Enable the peripheral
	
	// Set as input
	GPIOPinTypeGPIOInput(BUTTON1_PORT, BUTTON1_PIN);
	// No on board pull up for SW1
	GPIOPadConfigSet(BUTTON1_PORT, BUTTON1_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	GPIOPinTypeGPIOInput(BUTTON2_PORT, BUTTON2_PIN);
}

//*****************************************************************************
//
// Buttons_Task - Called via the interrupt, so we can debounce presses
// and time for long presses
//
//*****************************************************************************
void Buttons_Task( ui32 tickTimeBase )
{
	if ( GPIOPinRead(BUTTON1_PORT, BUTTON1_PIN) )
	{
		// Button Up
		Button1PressedCounter = 0;
	}
	else
	{
		// Button Down (Pressed)
		if ( Button1PressedCounter < BUTTON_PRESSED_LIMIT )
		{
			// If we are about to exceed the pressed limit
			// we only raise the pressed flag once per press
			if (( Button1PressedCounter + tickTimeBase ) >= BUTTON_PRESSED_LIMIT )
			{
				ButtonsPressed |= BIT0;
			}
		}

		// Increment counter
		Button1PressedCounter += tickTimeBase;

		// Held gets re-raised whilst it's being held
		if ( Button1PressedCounter >= BUTTON_HELD_LIMIT )
		{
			ButtonsHeld |= BIT0;
			Button1PressedCounter = BUTTON_HELD_LIMIT;
		}
	}

	// TODO : could make the above work on a loop, but there's only 2 buttons so no point

	if ( GPIOPinRead(BUTTON2_PORT, BUTTON2_PIN) )
	{
		// Button Up
		Button2PressedCounter = 0;
	}
	else
	{
		// Button Down (Pressed)
		if ( Button2PressedCounter < BUTTON_PRESSED_LIMIT )
		{
			// If we are about to exceed the pressed limit
			// we only raise the pressed flag once per press
			if (( Button2PressedCounter + tickTimeBase ) >= BUTTON_PRESSED_LIMIT )
			{
				ButtonsPressed |= BIT1;
			}
		}

		// Increment counter
		Button2PressedCounter += tickTimeBase;

		// Held gets re-raised whilst it's being held
		if ( Button2PressedCounter >= BUTTON_HELD_LIMIT )
		{
			ButtonsHeld |= BIT1;
			Button2PressedCounter = BUTTON_HELD_LIMIT;
		}
	}

}

//*****************************************************************************
//
// Buttons_CheckForPressed - Accessor function used by the application get 
// check for presses
//
//*****************************************************************************
ui8 Buttons_CheckForPressed ( void )
{
	return (ButtonsPressed);
}

//*****************************************************************************
//
// Buttons_CheckForPressed - Accessor function used by the application get
// check for presses
//
//*****************************************************************************
ui8 Buttons_CheckForHeld ( void )
{
	return (ButtonsHeld);
}

//*****************************************************************************
//
// Buttons_Acknowledge - Accessor - Used to clear button pressed flags
//
//*****************************************************************************
void Buttons_Acknowledge ( void )
{
	ButtonsPressed = 0x00;
	ButtonsHeld = 0x00;
}
