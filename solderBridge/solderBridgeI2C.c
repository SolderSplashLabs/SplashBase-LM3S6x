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

#define _BRIDGE_I2C_
#include "solderBridgeI2c.h"

// 32bit I/O Card, There are 8 possible addresses :
// 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27

// *****************************************************************************
// SB_I2C_Init
//
// *****************************************************************************
void SB_I2C_Init( void )
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

	// Port B will already be powered in main, if not you must enable power to it before using it

    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    // Setup the I2C
    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), true);

}

// *****************************************************************************
// SB_I2C_Init
//
// *****************************************************************************
void SB_I2C_Scan ( void )
{
ui8 i = 0;

	// loop through each I2C address looking for a reply.
	for (i=0; i<PCA_MAX; i++)
	{
		if ( PCA9555_ReadPorts( PCA_ADDR_BASE+i, &IoExpanders.input[i]) )
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
// SB_I2C_Init
//
// *****************************************************************************
bool PCA9555_ReadPorts ( ui8 address, ui32 *portValue )
{
bool result = false;
ui32 error = 0;

	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, address, false);

	I2CMasterDataPut(I2C0_MASTER_BASE, 0);

	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);

	//
    // Wait until master module is done transferring.
    //
	while(I2CMasterBusy(I2C0_MASTER_BASE))
	{

	}

	error = I2CMasterErr( I2C0_MASTER_BASE );

	if ( I2C_MASTER_ERR_NONE == error )
	{
		// Success, now read the result
		I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, address, true);

		// We need more than 1 byte
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

		// Wait until master module is done transferring.
		while(I2CMasterBusy(I2C0_MASTER_BASE))
		{

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
