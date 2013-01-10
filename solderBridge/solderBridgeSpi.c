/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013-01-01 - www.soldersplash.co.uk - C. Matthews - R. Steel

 Redistributions of source code must retain the above copyright notice
*/

#include "datatypes.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"

#include "servoSolderBridge.h"
#include "solderBridgeSpi.h"

bool SB_Scanning = false;

const char *BRIDGE_NAMES[SB_BRIDGE_MAX] = 	{	"SB_NONE",
												"SB_SERVO_24",
												"SB_DMX_MASTER",
												"SB_NAUGHT_TO_TEN",
												"SB_DATALOGGER",
												"SB_NINE_DOF" };

ui8 SolderBridgeList[ SB_SPI_CS_COUNT ];
ui8 SolderBridgeListVer[ SB_SPI_CS_COUNT ];

// --------------------------------------------------------------------------------------
// SolderBridge_Task
//
// --------------------------------------------------------------------------------------
void SolderBridge_Task ( void )
{
	if (SB_Scanning)
	{
		SB_Scan( SB_Scanning );
	}
}

// --------------------------------------------------------------------------------------
// SolderBridge_StartScan
// Start looking for SolderBridges
// --------------------------------------------------------------------------------------
bool SolderBridge_StartScan ( void )
{
bool result = false;

	if (SB_Scanning)
	{
		// Do nothing!
	}
	else
	{
		result = true;
		SB_Scan( false );
	}

	return (result);
}

// --------------------------------------------------------------------------------------
// SB_Scan
// Internal Function, used to scan the SPI bus for Bridges
// --------------------------------------------------------------------------------------
static void SB_Scan ( bool getResponse )
{
ui32 tempLong = 0;
ui8 i = 0;

	if (! getResponse )
	{
		// CS them all!!
		SB_CS_Select( SB_SPI_SLAVE0 | SB_SPI_SLAVE1 | SB_SPI_SLAVE2 | SB_SPI_SLAVE3 | SB_SPI_SLAVE4 );

		SSIDataPut(SSI0_BASE, (ui16)('#'<<8 | SB_WHOS_THERE));

		while ( SSIBusy(SSI0_BASE) )
		{
			// TODO : Time out
		}

		// Clear data read in
		while(SSIDataGetNonBlocking(SSI0_BASE, &tempLong))
		{

		}

		SB_CS_DeSelect(SB_SPI_CS_PINS);

		SB_Scanning = true;
	}
	else
	{
		for (i=0; i<SB_SPI_CS_COUNT; i++)
		{
			SB_CS_Select( 0x01 << i );
			// Clock out zeros
			SSIDataPut(SSI0_BASE, (ui16)0x0000);

			while ( SSIBusy(SSI0_BASE) )
			{
				// TODO : Time out
			}

			// Read the data read in
			SSIDataGetNonBlocking(SSI0_BASE, &tempLong);

			SolderBridgeList[i] = (ui8)(tempLong >> 8);
			SolderBridgeListVer[i] = (ui8)(0x00FF & tempLong);

			SB_CS_DeSelect(SB_SPI_CS_PINS);
		}

		SB_Scanning = false;
	}

}


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
// SB_GetBridgeType
// returns the bridge type at that index
// --------------------------------------------------------------------------------------
ui8 SB_GetBridgeType ( ui8 position )
{
	return( SolderBridgeList[ position ] );
}

// --------------------------------------------------------------------------------------
// SB_GetBridgeAtPos
// returns the bridge name at that index
// --------------------------------------------------------------------------------------
const char * SB_GetBridgeName ( ui8 position )
{
ui8 bridgeType = SolderBridgeList[position];

	if (bridgeType >= SB_BRIDGE_MAX-1) bridgeType = 0;
	return( BRIDGE_NAMES[ bridgeType ] );
}

// --------------------------------------------------------------------------------------
// SB_CS_Select
//
// --------------------------------------------------------------------------------------
void SB_CS_Select ( ui8 csMask )
{
ui8 pinMask = 0;

	// Only 5 CS lines available BIT0-BIT4
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
// SB_ServoSet
// Set 12 Servo positions
// --------------------------------------------------------------------------------------
void SB_ServoSet (ui8 slaveMask, ui8 *positions, ui8 servoOffset, ui8 servoCnt )
{
ui32 tempLong = 0;
ui16 *tempShort ;

	// TODO : Finish this function
	// CS the Slave
	SB_CS_Select( slaveMask );

	SSIDataPutNonBlocking(SSI0_BASE, (ui16)('#'<<8 | SB_SERVO_MOVE));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(servoOffset<<8 | servoCnt));
	tempShort = positions;

	SSIDataPutNonBlocking(SSI0_BASE, tempShort[0]);//(ui16)(positions[1]<<8 | positions[0]));
	SSIDataPutNonBlocking(SSI0_BASE, tempShort[1]);//(ui16)(positions[3]<<8 | positions[2]));
	SSIDataPutNonBlocking(SSI0_BASE, tempShort[2]);//(ui16)(positions[5]<<8 | positions[4]));
	SSIDataPutNonBlocking(SSI0_BASE, tempShort[3]);//(ui16)(positions[7]<<8 | positions[6]));
	SSIDataPutNonBlocking(SSI0_BASE, tempShort[4]);//(ui16)(positions[9]<<8 | positions[8]));
	SSIDataPut(SSI0_BASE, tempShort[5]);//(ui16)(positions[11]<<8 | positions[10]));

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
ui16 tempLong = 0;

	// CS the Slave
	SB_CS_Select( slaveMask );

	SSIDataPutNonBlocking(SSI0_BASE, (ui16)('#'<<8 | SB_ZEROTEN_UPDATE));

	// Pack the update
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(outputLevels[0]<<8 | outputLevels[1]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(outputLevels[2]<<8 | outputLevels[3]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(outputLevels[4]<<8 | outputLevels[5]));
	SSIDataPutNonBlocking(SSI0_BASE, (ui16)(outputLevels[6]<<8 | 0x00));

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
// SB_DmxUpdate
//
// --------------------------------------------------------------------------------------
bool SB_DmxUpdate(ui8 slaveMask, ui16 offset, ui16 channelCnt, ui8 *updatedVal)
{
	ui16 tempLong = 0;
	ui16 i = 0;

	// TODO : Buffer check, channelCnt Limit
	// TODO : offset check with channelcnt cant exceed 512

	// CS the Slave
	SB_CS_Select( slaveMask );

	SSIDataPut(SSI0_BASE, (ui16)('#'<<8 | SB_DMX_UPDATE));

	SSIDataPut(SSI0_BASE, offset);
	SSIDataPut(SSI0_BASE, channelCnt);

	// pack 2 bytes into each 16bit value
	// accounting for an odd number to send
	for (i=0; i<(channelCnt/2); i++)
	{
		tempLong = updatedVal[i*2];
		//tempLong <<= 8;

		if ( (i*2)+1 >= channelCnt)
		{
			// clear the bottom byte out
			tempLong &= ~0xFF00;
		}
		else
		{
			tempLong |= (updatedVal[(i*2)+1] << 8) & 0xFF00;
		}
		SSIDataPut(SSI0_BASE, tempLong);
	}

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

	// Rising Edge, 5mhz, 16bits per
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 5000000, 16);

	SSIEnable(SSI0_BASE);

	// Empty the fifo incase there is any unprocessed received data, we don't want/need it
	while(SSIDataGetNonBlocking(SSI0_BASE, &tempLong))
	{
		// TODO : Time out
	}
}
