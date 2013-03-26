/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#include "SplashBaseHeaders.h"

ui32 Uptime = 0;
ui32 UnixTime = 0;
ui32 MsTimer = 0;

// *****************************************************************************
//
// Time_Task
//
// *****************************************************************************
void Time_Task ( void )
{
	static ui16 msCounter = 0;
	static ui32 ntpCounter = 0;

	MsTimer += SYSTICKMS;
	msCounter += SYSTICKMS;

	if (msCounter > 1000)
	{
		msCounter = msCounter - 1000;
		UnixTime++;
		Uptime++;
		ntpCounter ++;

		// Refresh twice a day, is this too much?
		if ( ntpCounter >= SECONDS_IN_HALF_DAY )
		{
			// Each day update the clock
			if ( SystemConfig.flags.NtpEnabled )
			{
				SntpGetTime();
			}
			ntpCounter = 0;
		}
	}

}


// *****************************************************************************
//
// Time_SetUnix
//
// *****************************************************************************
void Time_SetUnix( ui32 timeStamp )
{
	UnixTime = timeStamp;
}

// *****************************************************************************
//
// Time_StampNow
//
// *****************************************************************************
ui32 Time_StampNow ( si16 minOffset )
{
ui32 secOffset = 0;

	// Convert to seconds
	secOffset = minOffset * 60;

	return (UnixTime+secOffset);
}

// *****************************************************************************
//
// Time_Uptime
//
// *****************************************************************************
ui32 Time_Uptime ( void )
{
	return (Uptime);
}

// *****************************************************************************
// Time_Get
// the calling function supplies a structure and it is populated with the current time
// *****************************************************************************
void Time_Get ( TIME_STRUCT *time )
{
	// Make sure it's not a null pointer
	if (time != 0)
	{
		// get the remainder after taking the days out, then calc the hours
		time->hour = (UnixTime % SECONDS_IN_AN_DAY) / SECONDS_IN_AN_HOUR;
		// next do the same to get minutes
		time->min = (UnixTime % SECONDS_IN_AN_HOUR) / SECONDS_IN_AN_MIN;
		time->second = UnixTime % SECONDS_IN_AN_MIN;
	}
}
