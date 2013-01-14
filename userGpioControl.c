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

#define GPIO_MAX_MCU		8
#define GPIO_MAX_EXT		8
#define GPIO_TOTAL			GPIO_MAX_MCU + GPIO_MAX_EXT

ui32 GPIO_REGISTERS[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTH_BASE };

//*****************************************************************************
//
// UserCheckGpioReservedPins - based on the the mode modifies the mask to remove
// GPIOs that are assigned a function
//
//*****************************************************************************
ui32 UserCheckGpioReservedPins( ui8 portNo, ui32 mask )
{
	return ( mask );
}

//*****************************************************************************
//
// UserSetGpioInput - Set GPIO as an Input
//
//*****************************************************************************
bool UserSetGpioOutput ( ui8 portNo, ui32 pins )
{
bool result = false;

	if (portNo < GPIO_MAX_MCU)
	{
		// If we have a relay add-on stop it's GPIO being changed
		pins = UserCheckGpioReservedPins(portNo, pins);

		GPIOPinTypeGPIOInput(GPIO_REGISTERS[portNo], pins);

		result = true;
	}
	else if (portNo < GPIO_TOTAL)
	{
		// TODO : Tell solderbridge module to set supplied pins on the port as inputs
		result = true;
	}
	else
	{
		// Out of range!
	}
}

//*****************************************************************************
//
// UserSetGpioOutput - Set GPIO as an output
//
//*****************************************************************************
bool UserSetGpioOutput ( ui8 portNo, ui32 pins )
{
bool result = false;

	if (portNo < GPIO_MAX_MCU)
	{
		// If we have a relay add-on stop it's GPIO being changed
		pins = UserCheckGpioReservedPins(portNo, pins);

		GPIOPinTypeGPIOOutput(GPIO_REGISTERS[portNo], pins);

		result = true;
	}
	else if (portNo < GPIO_TOTAL)
	{
		// TODO : Tell solderbridge module to set supplied pins on the port as outputs
		result = true;
	}
	else
	{
		// Out of range!
	}
}


//*****************************************************************************
//
// UserSetGpioOutputs - Set/Clear GPIO outputs
//
//*****************************************************************************
bool UserSetGpioOutputs ( ui8 portNo, ui32 mask, ui32 newVal)
{
ui32 portRegister = 0;
bool result = false;

	if (portNo < GPIO_MAX_MCU)
	{
		// Stop any output thats being used from being user controlled
		mask = UserCheckGpioReservedPins(portNo, mask);

		// Apply the change to the microcontrollers ports
		portRegister = GPIO_REGISTERS[portNo];

		// If we have a relay add-on stop it's GPIO being changed via this function
		mask = UserCheckGpioReservedPins(portNo, mask);

		GPIOPinWrite(portRegister, mask, newVal);
		result = true;
	}
	else if ( portNo < (GPIO_MAX_MCU +GPIO_MAX_EXT))
	{
		// This change effects external I/O

		result = true;
	}
	else
	{
		// Out of range!
	}
}