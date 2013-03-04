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

#include "SplashBaseHeaders.h"
#include <string.h>

/*
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"
#include "utils/flash_pb.h"
#include "utils/locator.h"
#include "lwiplib.h"
#include "utils/ustdlib.h"
#include "httpserver_raw/httpd.h"

#include "datatypes.h"
#include "globals.h"

#include "logicController.h"
#include "config.h"

#include "serial.h"
#include "telnet.h"

#ifdef UPNP_ENABLED
#include "upnp.h"
#endif

#include "udpControl.h"
#include "colourModes.h"
*/


//*****************************************************************************
//
//! This structure instance contains the factory-default set of configuration
//! parameters for S2E module.
//
//*****************************************************************************
static const tConfigParameters g_sParametersFactory =
{
    // The sequence number
    0,

    // Checksum
    0,

    // version
    0,

    // flags
    0,

    // SplashBase Name
	{
		'S','p','l','a','s','h','B','a','s','e', 0 , 0 , 0 , 0 , 0 , 0,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0
	},

    // Static IP address
    0x00000000,

    // Default gateway IP address (used only if static IP is in use).
    0x00000000,

    // Subnet mask (used only if static IP is in use).
    0xFFFFFF00,

    // Relay Names
    {
    	'R','e','l','a','y','1', 0, 0, 0, 0, 0, 0,
    },
    {
        'R','e','l','a','y','2', 0, 0, 0, 0, 0, 0,
    },
    {
        'R','e','l','a','y','3', 0, 0, 0, 0, 0, 0,
    },
    {
        'R','e','l','a','y','4', 0, 0, 0, 0, 0, 0,
    },

    // PWM freq = 10kHz
    (ui16)5000,

    (ui8)COLOUR_MODE_OFF,
    // Step length ( in ticks 10ms )
    (ui8)1,

    // No of steps between top and bottom
    (ui16)255,

    // Reserved to fill a whole
    (ui16)0,

    // Top Colour (Colour1)
    (ui32)0x00000000,

    // Bottom Colour (Colour2)
    (ui32)0x00000000,

    // Time offset
    (ui32)0,

    // SNTP Address 0.pool.ntp.org
	{
		'0','.','p','o','o','l','.','n','t','p','.','o','r','g', 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	},

	// SNTP Address 0.pool.ntp.org
	{
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
		 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
	},

    //
    // (compiler will pad to the full length)
    //

};

//*****************************************************************************
//
//! This structure instance contains the run-time set of configuration
//! parameters for S2E module.  This is the active parameter set and may
//! contain changes that are not to be committed to flash.
//
//*****************************************************************************
tConfigParameters g_sParameters;

//*****************************************************************************
//
//! This structure instance points to the most recently saved parameter block
//! in flash.  It can be considered the default set of parameters.
//
//*****************************************************************************
const tConfigParameters *g_psDefaultParameters;

//*****************************************************************************
//
//! This structure contains the latest set of parameter committed to flash
//! and is used by the configuration pages to store changes that are to be
//! written back to flash.  Note that g_sParameters may contain other changes
//! which are not to be written so we can't merely save the contents of the
//! active parameter block if the user requests some change to the defaults.
//
//*****************************************************************************
//static tConfigParameters g_sWorkingDefaultParameters;
// Accessing this externally
// TODO : implement accessor to do it properly
tConfigParameters g_sWorkingDefaultParameters;

//*****************************************************************************
//
//! This structure instance points to the factory default set of parameters in
//! flash memory.
//
//*****************************************************************************
const tConfigParameters *const g_psFactoryParameters = &g_sParametersFactory;

//*****************************************************************************
//
//! Loads the S2E parameter block from factory-default table.
//!
//! This function is called to load the factory default parameter block.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigLoadFactory(void)
{
    //
    // Copy the factory default parameter set to the active and working
    // parameter blocks.
    //
    g_sParameters = g_sParametersFactory;
    g_sWorkingDefaultParameters = g_sParametersFactory;
}

//*****************************************************************************
//
//! Loads the S2E parameter block from flash.
//!
//! This function is called to load the most recently saved parameter block
//! from flash.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigLoad(void)
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
        g_sParameters = *(tConfigParameters *)pucBuffer;
        g_sWorkingDefaultParameters = g_sParameters;
    }
}

//*****************************************************************************
//
//! Saves the S2E parameter block to flash.
//!
//! This function is called to save the current S2E configuration parameter
//! block to flash memory.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigSave(void)
{
    unsigned char *pucBuffer;

    //
    // Save the working defaults parameter block to flash.
    //
    FlashPBSave((unsigned char *)&g_sWorkingDefaultParameters);

    //
    // Get the pointer to the most recently saved buffer.
    // (should be the one we just saved).
    //
    pucBuffer = FlashPBGet();

    //
    // Update the default parameter pointer.
    //
    if(pucBuffer)
    {
        g_psDefaultParameters = (tConfigParameters *)pucBuffer;
    }
    else
    {
        g_psDefaultParameters = (tConfigParameters *)g_psFactoryParameters;
    }
}

//*****************************************************************************
//
//! Initializes the configuration parameter block.
//!
//! This function initializes the configuration parameter block.  If the
//! version number of the parameter block stored in flash is older than
//! the current revision, new parameters will be set to default values as
//! needed.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigInit(void)
{
    unsigned char *pucBuffer;
    volatile ui16 confSize = 0;

    //
    // Verify that the parameter block structure matches the FLASH parameter
    // block size.
    //

    ASSERT(sizeof(tConfigParameters) == FLASH_PB_SIZE);
    confSize = sizeof(tConfigParameters);
    //
    // Initialize the flash parameter block driver.
    //
    FlashPBInit(FLASH_PB_START, FLASH_PB_END, FLASH_PB_SIZE);

    //
    // First, load the factory default values.
    //
    ConfigLoadFactory();

    //
    // Then, if available, load the latest non-volatile set of values.
    //
    ConfigLoad();

    //
    // Get the pointer to the most recently saved buffer.
    //
    pucBuffer = FlashPBGet();

    //
    // Update the default parameter pointer.
    //
    if(pucBuffer)
    {
        g_psDefaultParameters = (tConfigParameters *)pucBuffer;
    }
    else
    {
        g_psDefaultParameters = (tConfigParameters *)g_psFactoryParameters;
    }
}

//*****************************************************************************
//
//! \internal
//!
//! Performs any actions necessary in preparation for a change of IP address.
//!
//! This function is called before ConfigUpdateIPAddress to remove the device
//! from the UPnP network in preparation for a change of IP address or
//! switch between DHCP and StaticIP.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigPreUpdateIPAddress(void)
{
    //
    // Stop UPnP and remove ourselves from the network.
    //
#ifdef UPNP_ENABLED
    UPnPStop();
#endif
}

//*****************************************************************************
//
//! \internal
//!
//! Sets the IP address selection mode and associated parameters.
//!
//! This function ensures that the IP address selection mode (static IP or
//! DHCP/AutoIP) is set according to the parameters stored in g_sParameters.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigUpdateIPAddress(void)
{
    //
    // Change to static/dynamic based on the current settings in the
    // global parameter block.
    //
    if((g_sParameters.flags & CONFIG_FLAG_STATICIP) == CONFIG_FLAG_STATICIP)
    {
        lwIPNetworkConfigChange(g_sParameters.ulStaticIP,
                                g_sParameters.ulSubnetMask,
                                g_sParameters.ulGatewayIP,
                                IPADDR_USE_STATIC);
    }
    else
    {
        lwIPNetworkConfigChange(0, 0, 0, IPADDR_USE_DHCP);
    }
#ifdef UPNP_ENABLED
    //
    // Restart UPnP discovery.
    //
    UPnPStart();
#endif
}

//*****************************************************************************
//
//! \internal
//!
//! Performs changes as required to apply all active parameters to the system.
//!
//! \param bUpdateIP is set to \e true to update parameters related to the
//! S2E board's IP address. If \e false, the IP address will remain unchanged.
//!
//! This function ensures that the system configuration matches the values in
//! the current, active parameter block.  It is called after the parameter
//! block has been reset to factory defaults.
//!
//! \return None.
//
//*****************************************************************************
void
ConfigUpdateAllParameters(tBoolean bUpdateIP)
{
    //
    // Have we been asked to update the IP address along with the other
    // parameters?
    //
    if(bUpdateIP)
    {
        //
        // Yes - update the IP address selection parameters.
        //
        ConfigPreUpdateIPAddress();
        ConfigUpdateIPAddress();
    }

    // Update the SplasBase Name and relay names
    SSC_SetUnitName((ui8 *)g_sParameters.splashBaseName);

    SSC_SetRelayName((ui8 *)g_sParameters.relayOneName, 0 );
	SSC_SetRelayName((ui8 *)g_sParameters.relayTwoName, 1 );
	SSC_SetRelayName((ui8 *)g_sParameters.relayThreeName, 2 );
	SSC_SetRelayName((ui8 *)g_sParameters.relayFourName, 3 );
}

