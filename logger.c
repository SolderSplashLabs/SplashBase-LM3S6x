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
//#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/lm3s6432.h"
#include "datatypes.h"

#define RELAYCONTROL
#include "relayControl.h"

// NOTE : Could have log entries sent out over UDP as well?

// *****************************************************************************
// LogEvent
// Log the event in to the ring buffer
// *****************************************************************************
void LogEvent ( ui8 eventId, ui32 eventData, ui32 eventData2 )
{
	// Time stamp it
	//Time_StampNow();
}

// *****************************************************************************
// LogInit
//
// *****************************************************************************
void LogInit ( void )
{

}
