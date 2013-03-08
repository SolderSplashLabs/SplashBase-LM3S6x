/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

 This function filters user control of GPIO, stopping them from interfering with IO
 Control by the application. It also diverts IO control to external add ons
*/
#define _USER_GPIO_
#include "SplashBaseHeaders.h"
#include "inc/hw_gpio.h"

// Mask of Outputs controlled by the application that the user should not be allowed to control
static ui32 UserGpioMask[ GPIO_PORT_TOTAL ];


// *****************************************************************************
//
// UserGpioInit - On boot set the GPIO up to the user specification
//
// *****************************************************************************
void UserGpioInit ( void )
{
ui8 i = 0;
ui32 mask = 0;

	if ( SystemConfig.flags & CONFIG_USER_GPIO_INIT )
	{
		for (i=0; i<GPIO_PORT_TOTAL; i++)
		{
			// Get the port pins I should not touch cleared from the mask
			mask = UserGpio_AppGetMask ( i, 0xFFFFFFFF );

			// Now apply initialisation to the port pins I can touch
			UserGpioDirection( i, mask, SystemConfig.UserGpioInit[i][0] );
			UserGpioSetOutputs(i, mask, SystemConfig.UserGpioInit[i][1] );
		}
	}
}


// *****************************************************************************
//
// UserGpio_AppGetMask - Return the mask to use on the supplied GPIO port
// To avoid modifying Outputs Controlled by the Application
// mask - mask intended to be used on the port
// return - mask modified removing protected bits
//
// *****************************************************************************
ui32 UserGpio_AppGetMask( ui8 portNo, ui32 mask )
{
	return (mask &= ~UserGpioMask[portNo]);
}

// *****************************************************************************
//
// UserGpio_AppGetMask - Set the mask to use on the supplied GPIO port
// To avoid modifying GPIO Controlled by the Application.
// mask - bits set are added to the protected list
//
// *****************************************************************************
void UserGpio_AppSetMask( ui8 portNo, ui32 mask )
{
	UserGpioMask[portNo] |= mask;
}

// *****************************************************************************
//
// UserGpio_AppClrMask - Clear bits in the on the supplied GPIO port
// bits set in supplied mask are cleared from the application mask
// mask - bits set are removed from the protected list
//
// *****************************************************************************
void UserGpio_AppClrMask( ui8 portNo, ui32 mask )
{
	UserGpioMask[portNo] &= ~mask;
}

//*****************************************************************************
//
// UserGpioDirection - Set GPIO direction
//
// pins - each bit represents a GPIO pin 1=output bit0=pin0
//
//*****************************************************************************
bool UserGpioDirection ( ui8 portNo, ui32 mask, ui32 pins )
{
bool result = false;

	if (portNo < GPIO_PORT_MAX_MCU)
	{
		// If we have a relay add-on stop it's GPIO being changed
		mask = UserGpio_AppGetMask(portNo, mask);
		pins = pins & mask;

		GPIOPinTypeGPIOOutput(GPIO_REGISTERS[portNo], pins);

		result = true;
	}
	else if (portNo < GPIO_PORT_TOTAL)
	{
		ExtGpio_SetDirection( (portNo-GPIO_PORT_MAX_MCU), mask, pins );
		result = true;
	}
	else
	{
		// Out of range!
	}

	return( result );
}


//*****************************************************************************
//
// UserSetGpioOutputs - Set/Clear GPIO outputs
//
//*****************************************************************************
bool UserGpioSetOutputs ( ui8 portNo, ui32 mask, ui32 newVal)
{
ui32 portRegister = 0;
bool result = false;

	if (portNo < GPIO_PORT_MAX_MCU)
	{
		// Stop any output thats being used from being user controlled
		mask = UserGpio_AppGetMask(portNo, mask);

		// Apply the change to the microcontrollers ports
		portRegister = GPIO_REGISTERS[portNo];

		GPIOPinWrite(portRegister, mask, newVal);
		result = true;
	}
	else if ( portNo < (GPIO_PORT_TOTAL))
	{
		// This change effects external I/O
		ExtGpio_SetPort( portNo-GPIO_PORT_MAX_MCU, mask, newVal );
		result = true;
	}
	else
	{
		// Out of range!
	}

	return( result );
}

//*****************************************************************************
//
// UserGpioGet - Retrieves the selected port pin status
//
//*****************************************************************************
bool UserGpioGet ( ui8 portNo, ui32 *buffer )
{
bool result = false;

	if (buffer != 0)
	{
		if (portNo < GPIO_PORT_MAX_MCU)
		{
			// Get MCU port
			*buffer = GPIOPinRead(GPIO_REGISTERS[portNo], 0xFF);
			result = true;
		}
		else if ( portNo < (GPIO_PORT_TOTAL))
		{
			// Get External Port
			// This change effects external I/O
			ExtGpio_GetPort( (portNo-GPIO_PORT_MAX_MCU), (ui16 *)buffer );
			result = true;
		}
		else
		{
			// Out of range!
		}
	}

	return ( result );
}

//*****************************************************************************
//
// UserGpioDirGet - Retrieves the selected port direction, redirecting it to
//					IO expanding bridges if needed
//
//*****************************************************************************
bool UserGpioDirGet ( ui8 portNo, ui32 *buffer )
{
bool result = false;

	if (buffer != 0)
	{
		if (portNo < GPIO_PORT_MAX_MCU)
		{
			// Get MCU port direction - There's no driver lib function for this so we do it ourselves
			*buffer = HWREG(GPIO_REGISTERS[portNo] + GPIO_O_DIR);
			result = true;
		}
		else if ( portNo < (GPIO_PORT_TOTAL))
		{
			// Get External Port Direction
			*buffer = ExtGpio_GetDirection( (portNo-GPIO_PORT_MAX_MCU) );
			result = true;
		}
		else
		{
			// Out of range!
		}
	}

	return ( result );
}
