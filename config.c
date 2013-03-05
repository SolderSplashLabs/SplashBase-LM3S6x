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

