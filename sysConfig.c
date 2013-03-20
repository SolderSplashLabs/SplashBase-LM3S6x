/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013-03-01 - www.soldersplash.co.uk - C. Matthews - R. Steel

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

*/

#define _CONFIG_
#include "SplashBaseHeaders.h"
#include <string.h>

// *****************************************************************************
//
// ConfigFactoryDefault - Copy the factory defaults over to the current settings
//
// *****************************************************************************
void SysConfigFactoryDefault(void)
{
	memcpy( &SystemConfig, &CONFIG_FACTORY_DEFAULTS, sizeof(SystemConfig));
}

// *****************************************************************************
//
// SysConfigLoad - Load config from flash
//
// *****************************************************************************
void SysConfigLoad(void)
{
    unsigned char *pucBuffer;

    //
    // Get a pointer to the latest parameter block in flash.
    //
    pucBuffer = FlashPBGet();

    //
    // See if a parameter block was found in flash.
    //
    if(pucBuffer)
    {
        //
        // A parameter block was found so copy the contents to both our
        // active parameter set and the working default set.
        //
        SystemConfig = *(tConfigParameters *)pucBuffer;
    }
}

// *****************************************************************************
//
// ConfigSave - Save config to flash
//
// *****************************************************************************
void SysConfigSave(void)
{
	// Save to flash
    FlashPBSave((unsigned char *)&SystemConfig);
}

// *****************************************************************************
//
// SysConfigInit - Default the settings, then attempt load from flash
//
// *****************************************************************************
void SysConfigInit(void)
{
volatile ui16 confSize = 0;

    ASSERT(sizeof(tConfigParameters) == FLASH_PB_SIZE);
    confSize = sizeof(tConfigParameters);

    // Init the TI FlashBlock Driver
    FlashPBInit(FLASH_PB_START, FLASH_PB_END, FLASH_PB_SIZE);

    // Default with factory settings
    SysConfigFactoryDefault();

    // Then if there are any useable saved parameters grab them from flash
    SysConfigLoad();
}

// *****************************************************************************
//
// SysSetBaseName - Set a new Base board name
//
// *****************************************************************************
void SysSetBaseName ( ui8 *buffer, ui8 len )
{
	if (len > SPLASHBASE_NAME_LEN-1) len = SPLASHBASE_NAME_LEN-1;

	ustrncpy((char *)SystemConfig.splashBaseName, (const char *)buffer, len);
	SystemConfig.splashBaseName[len] = 0;

	// And the SolderSplash UDP Protocol
	SSC_SetUnitName((ui8 *)SystemConfig.splashBaseName);

	SysConfigSave();
}

// *****************************************************************************
//
// SysSetSntpAddress - Set a new Base board name
//
// *****************************************************************************
void SysSetSntpAddress ( ui8 *buffer, ui8 len )
{
	if (len > SNTP_SERVER_LEN-1) len = SNTP_SERVER_LEN-1;

	ustrncpy((char *)SystemConfig.sntpServerAddress, (const char *)buffer, len);
	SystemConfig.sntpServerAddress[len] = 0;

	SysConfigSave();
}

// *****************************************************************************
//
// SysConfigRelayEnable - Enable Relay Control
//
// *****************************************************************************
void SysConfigRelayEnable ( void )
{
	//if ( SystemConfig.flags & CONFIG_FOUR_RELAY_EN )
	if ( SystemConfig.flags.FourRelayEnable )
	{
		// It already is!
	}
	else
	{
		//SystemConfig.flags |= CONFIG_FOUR_RELAY_EN;
		SystemConfig.flags.FourRelayEnable = true;
		SysConfigSave();

		RelayInit();
	}
}

// *****************************************************************************
//
// SysConfigRelayDisable - Disable Relay Control
//
// *****************************************************************************
void SysConfigRelayDisable ( void )
{
	//if ( SystemConfig.flags & CONFIG_FOUR_RELAY_EN )
	if ( SystemConfig.flags.FourRelayEnable )
	{
		//SystemConfig.flags &= ~CONFIG_FOUR_RELAY_EN;
		SystemConfig.flags.FourRelayEnable = false;
		SysConfigSave();
	}
	else
	{
		// Already disabled
	}
}

// *****************************************************************************
//
// SysSetBaseName - Set a new Base board name
//
// *****************************************************************************
void SysSetRelayName ( ui8 *buffer, ui8 len, ui8 relayNo, bool saveNow )
{
char * relayName = 0;
ui8 i = 0;

	if (len > SPLASHBASE_RELAYNAME_LEN-1) len = SPLASHBASE_RELAYNAME_LEN-1;

	switch ( relayNo )
	{
		case 1 :
			relayName = (char *)SystemConfig.relayOneName;
		break;

		case 2 :
			relayName = (char *)SystemConfig.relayTwoName;
		break;

		case 3 :
			relayName = (char *)SystemConfig.relayThreeName;
		break;

		case 4 :
			relayName = (char *)SystemConfig.relayFourName;
		break;
	}

	if ( relayName )
	{
		// Copy the new name over
		ustrncpy(relayName, (const char *)buffer, len);

		// Zero-fill the remainder of the space
		for(i=len; i < SPLASHBASE_RELAYNAME_LEN; i++)
		{
			relayName[i] = 0;
		}

		// Update the SolderSplash UDP Protocol
		SSC_SetRelayNames();

		if ( saveNow ) SysConfigSave();
	}
}
