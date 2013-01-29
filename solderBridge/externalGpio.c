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

#include "datatypes.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"

#define _EXT_GPIO_
#include "externalGpio.h"

// 32bit I/O Card, There are 8 possible addresses :
// 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27

// For simplicity we shall imagine each PCA chip is just another GPIO port
// The mapping of port names to I2C addresses is as follows

// Address 0x20 - PORTI
// Address 0x21 - PORTJ
// Address 0x22 - PORTK
// Address 0x23 - PORTL
// Address 0x24 - PORTM
// Address 0x25 - PORTN
// Address 0x26 - PORTO
// Address 0x27 - PORTP


#define EXTIO_PORT_OFFSET	9

// We need to give the external ports numbers
enum EXIO_PORTNO
{
	EXTIO_PORTI = 0,
	EXTIO_PORTJ,
	EXTIO_PORTK,
	EXTIO_PORTL,
	EXTIO_PORTM,
	EXTIO_PORTN,
	EXTIO_PORTO,
	EXTIO_PORTP,
	EXTIO_MAX
};


// *****************************************************************************
//
// ExtGpio_Task
//
// *****************************************************************************
void ExtGpio_Task ( void )
{
	// A PCA Will pull down on an interrupt line if its input has changed
	// They are all on the Same pin so we have to go find it!
	if ( GPIOPinRead(EXT_GPIO_INTTERUPT_PORT, EXT_GPIO_INTTERUPT_PIN) )
	{
		// Do Nothing
	}
	else
	{
		// An interrupt has occurred, or we are just refreshing, go read our external GPIO
		ExtGpio_UpdateInputs();
	}

	if ( IoExpanders.updateDirection )
	{
		// We have some updating to do
		ExtGpio_UpdateDirection();
	}

	if ( IoExpanders.updateOutput )
	{
		// We have some updating to do
		ExtGpio_UpdateOutput();
	}
}

// *****************************************************************************
//
// ExtGpio_Init
//
// *****************************************************************************
void ExtGpio_Init( void )
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

	// Port B will already be powered in main, if not you must enable power to it before using it

    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    // Setup the I2C
    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), true);

    // Configure the interrupt line, this goes low when when an input changes
    GPIOPinTypeGPIOInput(EXT_GPIO_INTTERUPT_PORT, EXT_GPIO_INTTERUPT_PIN);
}

// *****************************************************************************
//
// ExtGpio_UpdateDirection - (Internal) called by the task to update a number of
// PCA control GPIO direction pins
//
// *****************************************************************************
static void ExtGpio_UpdateDirection( void )
{
ui8 i = 0;
ui8 selectedMask = 0;

	// loop through each I2C address looking for a reply.
	for (i=0; i<PCA_MAX; i++)
	{
		selectedMask = (0x01 << i);
		if ( IoExpanders.pcaAvailible & selectedMask )
		{
			if ( IoExpanders.updateDirection & selectedMask )
			{
				if ( ExtGpio_WriteReg ( PCA_ADDR_BASE+i, PCA9555_REG_DIR, IoExpanders.direction[i] ) )
				{
					IoExpanders.updateDirection &= ~selectedMask;
				}
				else
				{
					// The update failed
					// TODO : Log
				}
			}
		}
	}
}

// *****************************************************************************
//
// ExtGpio_UpdateOutput - (Internal) called by the task to update a number of
// PCA controlled outputs
//
// *****************************************************************************
static void ExtGpio_UpdateOutput( void )
{
ui8 i = 0;
ui8 selectedMask = 0;

	// loop through each I2C address looking for a reply.
	for (i=0; i<PCA_MAX; i++)
	{
		selectedMask = (0x01 << i);
		if ( IoExpanders.pcaAvailible & selectedMask )
		{
			if ( IoExpanders.updateOutput & selectedMask )
			{
				if ( ExtGpio_WriteReg ( PCA_ADDR_BASE+i, PCA9555_REG_OUPUT, IoExpanders.output[i] ) )
				{
					IoExpanders.updateOutput &= ~selectedMask;
				}
				else
				{
					// The update failed
					// TODO : Log
				}
			}
		}
	}
}


// *****************************************************************************
//
// ExtGpio_UpdateInputs - Update the inputs of the known ExtGPIO
//
// *****************************************************************************
void ExtGpio_UpdateInputs ( void )
{
ui8 i = 0;

	// loop through each I2C address looking for a reply.
	for (i=0; i<PCA_MAX; i++)
	{
		if ( IoExpanders.pcaAvailible & (0x01 << i) )
		{
			if ( ExtGpio_ReadPort( PCA_ADDR_BASE+i, &IoExpanders.input[i] ) )
			{
				// Read the External GPIO
			}
			else
			{
				// The Ext GPIO isn't answering
				// TODO : Log that we have been unable to talk to the ext gpio
			}
		}
	}
}

// *****************************************************************************
//
// ExtGpio_Scan - Scans the I2C bus for available ExtGPIO
//
// *****************************************************************************
void ExtGpio_Scan ( void )
{
ui8 i = 0;

	// loop through each I2C address looking for a reply.
	for (i=0; i<PCA_MAX; i++)
	{
		if ( ExtGpio_ReadPort( PCA_ADDR_BASE+i, &IoExpanders.input[i]) )
		{
			IoExpanders.pcaAvailible |= (0x01 << i);
		}
		else
		{
			IoExpanders.pcaAvailible &= ~(0x01 << i);
		}
	}
}

// *****************************************************************************
//
// ExtGpio_ReadPort : Each PCA9555D as 16 GPIO, internally in 2 8bit wide ports
// This function will query the selected PCA chip and update the supplied
// portValue.
//
// Note : this is a blocking function
//
// Return : boolean indicates communication success
//
// *****************************************************************************
bool ExtGpio_ReadPort ( ui8 address, ui16 *portValue )
{
bool result = false;
ui32 error = 0;

	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, address, false);

	I2CMasterDataPut(I2C0_MASTER_BASE, PCA9555_REG_INPUT);

	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);

	//
    // Wait until master module is done transferring.
    //
	while(I2CMasterBusy(I2C0_MASTER_BASE))
	{
		// TODO : Bad, Don't want a hang up!
	}

	error = I2CMasterErr( I2C0_MASTER_BASE );

	if ( I2C_MASTER_ERR_NONE == error )
	{
		// Success, now read the result
		I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, address, true);

		// We need more than 2 bytes for the 16bit port
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

		// Wait until master module is done transferring.
		while(I2CMasterBusy(I2C0_MASTER_BASE))
		{
			// TODO : Bad, Don't want a hang up!
		}

		error = I2CMasterErr( I2C0_MASTER_BASE );

		if ( I2C_MASTER_ERR_NONE == error )
		{
			*portValue = I2CMasterDataGet(I2C0_MASTER_BASE);

			I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

			//
			// Wait until master module is done transferring.
			//
			while(I2CMasterBusy(I2C0_MASTER_BASE))
			{
				// TODO : Bad, Don't want a hang up!
			}

			*portValue <<= 8;
			*portValue |= I2CMasterDataGet(I2C0_MASTER_BASE);

			result = true;
		}
	}
	else if ( I2C_MASTER_ERR_ADDR_ACK & error )
	{
		// No one there!
	}

	return (result);

}

// *****************************************************************************
//
// ExtGpio_WriteReg : Update a register on the select PCA9555
//
// *****************************************************************************
static bool ExtGpio_WriteReg ( ui8 address, ui8 reg, ui16 value )
{
bool result = false;
ui32 error = 0;

	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, address, false);

	I2CMasterDataPut(I2C0_MASTER_BASE, PCA9555_REG_DIR);

	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

	//
	// Wait until master module is done transferring.
	//
	while(I2CMasterBusy(I2C0_MASTER_BASE))
	{
		// TODO : Bad, Don't want a hang up!
	}

	error = I2CMasterErr( I2C0_MASTER_BASE );

	if ( I2C_MASTER_ERR_NONE == error )
	{
		// No error and we got acked, send the MSB First
		I2CMasterDataPut(I2C0_MASTER_BASE, (ui8)(value>>8));
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

		while(I2CMasterBusy(I2C0_MASTER_BASE))
		{
			// TODO : Bad, Don't want a hang up!
		}

		error = I2CMasterErr( I2C0_MASTER_BASE );

		if ( I2C_MASTER_ERR_NONE == error )
		{
			// No error and we got acked, send the last byte
			I2CMasterDataPut(I2C0_MASTER_BASE, (ui8)(value));
			I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

			while(I2CMasterBusy(I2C0_MASTER_BASE))
			{
				// TODO : Bad, Don't want a hang up!
			}

			error = I2CMasterErr( I2C0_MASTER_BASE );

			if ( I2C_MASTER_ERR_NONE == error ) result = true;
		}
	}

	return ( result );
}

// *****************************************************************************
//
// ExtGpio_SetDirection : Set the direction of each bit of the selected PCA9555D
// A set bit indicates an output
//
// *****************************************************************************
void ExtGpio_SetDirection ( ui8 port, ui16 mask, ui16 dir )
{
	if ( port < PCA_MAX )
	{
		// Remove any set bits not required from the supplied value
		dir &= mask;

		// Clear any outputs that are about to be changed
		IoExpanders.direction[ port ] &= ~mask;

		// Update the direction register
		IoExpanders.direction[ port ] |= dir;
		IoExpanders.updateDirection |= (1<<port);
	}
}

// *****************************************************************************
//
// ExtGpio_SetPort : Change port output
//
// *****************************************************************************
void ExtGpio_SetPort ( ui8 port, ui16 mask, ui16 value )
{
	if ( port < PCA_MAX )
	{
		// Clear affected bits & update
		IoExpanders.output[ port ] &= ~mask;
		IoExpanders.output[ port ] |= value;
		IoExpanders.updateOutput |= (1<<port);
	}
}

// *****************************************************************************
//
// ExtGpio_SetPort : Change port output
//
// *****************************************************************************
void ExtGpio_GetPort ( ui8 port, ui16 *buffer )
{
	if ( buffer != 0 )
	{
		if ( port < PCA_MAX )
		{
			*buffer = IoExpanders.input[port];
		}
	}
}


// *****************************************************************************
//
// ExtGpio_SetPortPolarity : Configure the PCA9555 Input Polarity
//
// *****************************************************************************
void ExtGpio_SetPortPolarity ( ui8 address, ui16 value )
{
	// Not implemented
}
