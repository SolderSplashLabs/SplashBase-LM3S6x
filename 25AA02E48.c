/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012-02-26 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Redistributions of source code must retain the above copyright notice

*/

/*
 * The 25AA02E48 is a 2Kbit SPI controlled Eeprom with a Pre-Flashed Unique EUI48 Address (MAC)
 * http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en538613
 *
 * Written/Tested against StellarisWare 8555 - http://www.ti.com/stellarisware
 *
 * Functions are blocking and return when completed
 *
 */

#include "datatypes.h"				// SolderSplash Labs - Custom Data Types
#include "inc/hw_memmap.h"			// Stellaris memory map macros
#include "inc/hw_ssi.h"				// SSI Macros
#include "inc/hw_types.h"			// TI HW macros
#include "driverlib/ssi.h"			// SSI StellarisWare Lib
#include "driverlib/gpio.h"			// GPIO StellarisWare Lib
#include "driverlib/sysctl.h"		// System Control StellarisWare Lib

#define _25AA02E48_
#include "25AA02E48.h"

//*****************************************************************************
//
//! Eeprom_SpiInit - Initialises the SPI bus before communicating to the Eeprom
//!
//! \param None
//!
//! \return None.
//
//*****************************************************************************
void Eeprom_SpiInit( void )
{
ui32 tempLong = 0;
	
	// Configure the SPI, 
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	
	// Enable SPI 0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);	
	SSIEnable(SSI0_BASE);

	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2);

	// Increase pull up strength to 4mA
	//GPIOPadConfigSet(GPIO_PORTA_BASE, (GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5);

	// Set them high
	GPIOPinWrite(GPIO_PORTA_BASE, BIT3, BIT3);
	GPIOPinWrite(GPIO_PORTC_BASE, BIT5, BIT5);

	// Rising Edge, 100khz, 8bits per
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 100000, 8);
                       
	SSIEnable(SSI0_BASE);
	
	// Empty the fifo incase there is any unprocessed received data, we don't want/need it
    while(SSIDataGetNonBlocking(SSI0_BASE, &tempLong))
    {
    	// TODO : Time out
    }

}

//*****************************************************************************
//
//! Eeprom_GetStatus - Returns the Eeproms Status Byte
//!
//! Refer to Page 10 of the datasheet for status bits
//!
//! \param None
//!
//! \return Status value.
//
//*****************************************************************************
ui8 Eeprom_GetStatus (void)
{
volatile ui32 temp;

	Eeprom_SpiInit();

	GPIOPinWrite(GPIO_PORTA_BASE, BIT3, 0);
	GPIOPinWrite(GPIO_PORTC_BASE, BIT5, 0);

	SSIDataPut(SSI0_BASE, EEPROM_CMD_READ_ST);
	SSIDataGet(SSI0_BASE, (ui32 *)&temp);
	SSIDataPut(SSI0_BASE, 0x00);
	SSIDataGet(SSI0_BASE, (ui32 *)&temp);

	// CS Line PA3 Low
	while ( SSIBusy(SSI0_BASE) )
	{
		// TODO : Time out
	}

	GPIOPinWrite(GPIO_PORTA_BASE, BIT3, BIT3);
	GPIOPinWrite(GPIO_PORTC_BASE, BIT5, BIT5);

	return( (ui8)temp );
}



//*****************************************************************************
//
//! Eeprom_ReadMacAddr - Returns the MAC address flashed in to the eeprom
//!
//! \param macAddressBuffer - buffer to load new MAC address in to
//!
//! \return Bool, True if the MAC address read out matches the first part of
//! microchips range
//
//*****************************************************************************
bool Eeprom_ReadMacAddr( ui8 *macAddressBuffer )
{
volatile ui32 temp;
bool result = false;
ui8 i = 0;

	// Check it's not a NULL pointer
	if (macAddressBuffer)
	{
		// Sets up the SPI interface, emptys anything from the fifo
		Eeprom_SpiInit();

		GPIOPinWrite(GPIO_PORTA_BASE, BIT3, 0);
		GPIOPinWrite(GPIO_PORTC_BASE, BIT5, 0);

		SSIDataPut(SSI0_BASE, EEPROM_CMD_READ);
		SSIDataPut(SSI0_BASE, 0xFA);
		// Throw away the data the collected whilst we were sending the command
		SSIDataGet(SSI0_BASE, (ui32 *)&temp);
		SSIDataGet(SSI0_BASE, (ui32 *)&temp);
	
		// Now grab the MAC address, we clock out 6 dummy bytes
		SSIDataPut(SSI0_BASE, 0x00);
		SSIDataPut(SSI0_BASE, 0x00);
		SSIDataPut(SSI0_BASE, 0x00);
		SSIDataPut(SSI0_BASE, 0x00);
		SSIDataPut(SSI0_BASE, 0x00);
		SSIDataPut(SSI0_BASE, 0x00);

		// Make sure it's all sent
		while ( SSIBusy(SSI0_BASE) )
		{
			// TODO : Time out
		}

		// Which clocks in 6 data bytes
		for (i=0; i<6; i++)
		{
			SSIDataGet(SSI0_BASE, (ui32 *)&temp);
			macAddressBuffer[i] = (ui8)temp;
		}
	
		GPIOPinWrite(GPIO_PORTA_BASE, BIT3, BIT3);
		GPIOPinWrite(GPIO_PORTC_BASE, BIT5, BIT5);
	
		// Check the first 3 bytes match the microchip OUI
		if ((MICROCHIP_OUI_0 == macAddressBuffer[0]) && (MICROCHIP_OUI_1 == macAddressBuffer[1]) && (MICROCHIP_OUI_2 == macAddressBuffer[2]))
		{
			result = true;
		}
	}
	
	return (result);
}
