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

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "datatypes.h"

#define _USER_GPIO_
#include "userGpioControl.h"

// Mask of Outputs controlled by the application that the user should not be allowed to control
static ui32 UserGpioMask[ GPIO_PORT_TOTAL ];


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
// UserSetGpioInput - Set GPIO as an Input
//
//*****************************************************************************
bool UserGpioDirInput ( ui8 portNo, ui32 pins )
{
bool result = false;

	if (portNo < GPIO_PORT_MAX_MCU)
	{
		// If we have a relay add-on stop it's GPIO being changed
		pins = UserGpio_AppGetMask(portNo, pins);

		GPIOPinTypeGPIOInput(GPIO_REGISTERS[portNo], pins);

		result = true;
	}
	else if (portNo < GPIO_PORT_TOTAL)
	{
		// TODO : Tell solderbridge module to set supplied pins on the port as inputs
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
// UserSetGpioOutput - Set GPIO as an output
//
//*****************************************************************************
bool UserGpioDirOutput ( ui8 portNo,  ui32 mask, ui32 pins )
{
bool result = false;

	if (portNo < GPIO_PORT_MAX_MCU)
	{
		// If we have a relay add-on stop it's GPIO being changed
		pins = UserGpio_AppGetMask(portNo, pins);

		GPIOPinTypeGPIOOutput(GPIO_REGISTERS[portNo], pins);

		result = true;
	}
	else if (portNo < GPIO_PORT_TOTAL)
	{
		// TODO : Tell solderbridge module to set supplied pins on the port as outputs
		ExtGpio_SetDirection( portNo-GPIO_PORT_MAX_MCU, mask, pins );
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

		// If we have a relay add-on stop it's GPIO being changed via this function
		mask = UserGpio_AppGetMask(portNo, mask);

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
