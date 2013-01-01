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
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "datatypes.h"

#include "solderBridgeSpi.h"

#define SERVOS_PER_BRIDGE 24

//*****************************************************************************
//
// ServoMoveToPos
// TODO : Multi servo bridge config needs testing
//
//*****************************************************************************
bool ServoMoveToPos ( ui8 servoNo, ui8 position )
{
ui8 i = 0;
ui8 bridgeNo = servoNo / SERVOS_PER_BRIDGE;
ui8 result = false;

	// find the right servo bridge
	// TODO : Range check servoNo and Position

	for (i=0; i<SB_SPI_CS_COUNT-1; i++)
	{
		if ( SB_SERVO_24 == SolderBridgeList[i] )
		{
			if (bridgeNo)
			{
				bridgeNo --;
			}
			else
			{
				// found it! create the correct mask, and work out the correct offset for the
				// select bridge
				SB_ServoSet ((0x01<<i), &position, (position % SERVOS_PER_BRIDGE), 1);
				result = true;
			}
		}
	}

	return (result);
}
