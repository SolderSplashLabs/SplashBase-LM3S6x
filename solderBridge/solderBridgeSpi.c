

#include "datatypes.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"

#include "solderBridgeSpi.h"

ui8 SolderBridgeList[ SB_SPI_CS_COUNT ];

// --------------------------------------------------------------------------------------
// SB_SetupList
//
// --------------------------------------------------------------------------------------
void SB_SetupList ( ui8 *bridgeList )
{
	if (bridgeList != 0)
	{
		// TODO : would a memcpy be less work...?
		SolderBridgeList[0] = bridgeList[0];
		SolderBridgeList[1] = bridgeList[1];
		SolderBridgeList[2] = bridgeList[2];
		SolderBridgeList[3] = bridgeList[3];
		SolderBridgeList[4] = bridgeList[4];
	}
}


// --------------------------------------------------------------------------------------
// SB_CS_Select
//
// --------------------------------------------------------------------------------------
void SB_CS_Select ( ui8 csMask )
{
ui8 pinMask = 0;

	// Only 5 CS lines availible
	pinMask = csMask & 0x1F;

	// Clear pins
	GPIOPinWrite(SB_SPI_CS_PORT, pinMask, 0);
}

// --------------------------------------------------------------------------------------
// SB_CS_DeSelect
//
// --------------------------------------------------------------------------------------
void SB_CS_DeSelect ( ui8 csMask )
{
ui8 pinMask = 0;

	// Only 5 CS lines availible
	pinMask = csMask & 0x1F;

	// Set pins
	GPIOPinWrite(SB_SPI_CS_PORT, (SB_SPI_SLAVE0 | SB_SPI_SLAVE1 | SB_SPI_SLAVE2 | SB_SPI_SLAVE3 | SB_SPI_SLAVE4), pinMask);
}


// --------------------------------------------------------------------------------------
// SB_CS_DeSelect
//
// --------------------------------------------------------------------------------------
void SB_Ping ( ui8 slaveMask )
{
	// Set CS
	SSIDataPutNonBlocking(SSI0_BASE, '#');
	SSIDataPutNonBlocking(SSI0_BASE, '0x01');
}


// --------------------------------------------------------------------------------------
// SB_ServoSet
// Set 12 Servo positions
// --------------------------------------------------------------------------------------
void SB_ServoSet (ui8 slaveMask, ui8 *positions, ui8 servoOffset, ui8 servoCnt )
{
ui32 tempLong = 0;

	// CS the Slave
	SB_CS_Select( slaveMask );

	SSIDataPutNonBlocking(SSI0_BASE, (ui16)('#'<<8 | 0x10));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(servoOffset<<8 | servoCnt));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(positions[0]<<8 | positions[1]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(positions[2]<<8 | positions[3]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(positions[4]<<8 | positions[5]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(positions[6]<<8 | positions[7]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(positions[8]<<8 | positions[9]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(positions[10]<<8 | positions[11]));

	// Make sure it's all sent
	while ( SSIBusy(SSI0_BASE) )
	{
		// TODO : Time out
	}

	// Clear data read in
	while(SSIDataGetNonBlocking(SSI0_BASE, &tempLong))
	{

	}

	// Deselect the Slave
	SB_CS_DeSelect( slaveMask );
}


// --------------------------------------------------------------------------------------
// SB_ServoSet
//
// --------------------------------------------------------------------------------------
void SB_ZeroToTenOutput (ui8 slaveMask, ui8 *outputLevels )
{
	// CS the Slave
	SSIDataPutNonBlocking(SSI0_BASE, '#');
	SSIDataPutNonBlocking(SSI0_BASE, '0x20');
	SSIDataPutNonBlocking(SSI0_BASE, outputLevels[0]);
	SSIDataPutNonBlocking(SSI0_BASE, outputLevels[1]);
	SSIDataPutNonBlocking(SSI0_BASE, outputLevels[2]);
	SSIDataPutNonBlocking(SSI0_BASE, outputLevels[3]);
	SSIDataPutNonBlocking(SSI0_BASE, outputLevels[4]);
	SSIDataPutNonBlocking(SSI0_BASE, outputLevels[5]);
}

// --------------------------------------------------------------------------------------
// SB_Init
//
// --------------------------------------------------------------------------------------
void SB_Init ( void )
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

	// Set CS lines as outputs
	GPIOPinTypeGPIOOutput(SB_SPI_CS_PORT, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4);

	// Deselect them all!
	SB_CS_DeSelect( 0xFF );

	// Rising Edge, 1mhz, 16bits per
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 16);

	SSIEnable(SSI0_BASE);

	// Empty the fifo incase there is any unprocessed received data, we don't want/need it
	while(SSIDataGetNonBlocking(SSI0_BASE, &tempLong))
	{
		// TODO : Time out
	}
}
