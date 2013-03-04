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
#include "SplashBaseHeaders.h"

/*
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/ethernet.h"

#include "lwiplib.h"				// lwip TCP/IP Stack
#include "upnp.h"

#include "datatypes.h"
#include "globals.h"

#include "config.h"

#include "25AA02E48.h"
#include "sntpClient.h"
#include "ethernetControl.h"
*/

bool EthernetConnected = false;
bool DhcpFail = false;
ui32 IpAddress = 0;
volatile ui8 macAddr[8];


// *****************************************************************************
// Ethernet_Task
//
// *****************************************************************************
void Ethernet_Task ( void )
{
	// Run the lwip stack
	lwIPTimer(SYSTICKMS);
}

// *****************************************************************************
// Ethernet_GetIp
//
// *****************************************************************************
ui32 Ethernet_GetIp ( void )
{
	return ( IpAddress );
}

// *****************************************************************************
// Ethernet_GetGatewayIp
//
// *****************************************************************************
ui32 Ethernet_GetGatewayIp ( void )
{
	return ( lwIPLocalGWAddrGet() );
}

// *****************************************************************************
// Ethernet_GetNetmask
//
// *****************************************************************************
ui32 Ethernet_GetNetmask ( void )
{
	return ( lwIPLocalNetMaskGet() );
}


// *****************************************************************************
// Ethernet_GetMacAddress
// Accessor to retrieve the MAC address
// *****************************************************************************
void Ethernet_GetMacAddress ( ui8 *pBuf )
{
	if ( pBuf != 0 )
	{
		memcpy(pBuf, (ui8 *)macAddr, 6);
	}
}

// *****************************************************************************
// Ethernet_GetIp
//
// *****************************************************************************
bool Ethernet_Connected( void )
{
	return (EthernetConnected);
}

//*****************************************************************************
//
//! Handles the Ethernet interrupt hooks for the client software.
//!
//! This function will run any handlers that are required to run in the
//! Ethernet interrupt context.  All the actual TCP/IP processing occurs within
//! this function (since lwIP is not re-entrant).
//!
//! \return None.
//
//*****************************************************************************

void lwIPHostTimerHandler (void)
{
	static bool askedForTime = false;
	static ui32 msCounter = 0;
    tBoolean bLinkStatusUp;

    bLinkStatusUp = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ? false : true;

	if(bLinkStatusUp != EthernetConnected)
	{
		// Network Connection status has changed
		EthernetConnected = bLinkStatusUp;
	}

	IpAddress = lwIPLocalIPAddrGet();

	if ((IpAddress) && (EthernetConnected))
	{
		if ((IpAddress & 0x000000FF) == 169 )
		{
			// we have a 169. ip which means DHCP has failed
			DhcpFail = true;
		}
		else
		{
			DhcpFail = false;

			// We have an IP, thats not a default local only
			if (! askedForTime)
			{
				//SntpGetTime();
				askedForTime = true;
			}
		}
	}

	msCounter += SYSTICKMS;

#ifdef UPNP_ENABLED
	UPnPHandler(msCounter);
#endif
}

// *****************************************************************************
// Ethernet_Init
// Set up the Ethernet peripheral
// *****************************************************************************
void Ethernet_Init ( void )
{
	// Get a MAC address
	Eeprom_SpiInit();

	// ReadMacAddr is a blocking call
	if (! Eeprom_ReadMacAddr((ui8 *)&macAddr[0]) )
	{
		// MAC address read failed, default it
		// NOTE : This could be due to an add on messing with the SPI
		macAddr[0] = 0x00;
		macAddr[1] = 0xAA;
		macAddr[2] = 0xAA;
		macAddr[3] = 0x00;
		macAddr[4] = 0x00;
		macAddr[5] = 0x01;
	}

	//
	// Enable Port F Ethernet LEDs.
	//  LED0        Bit 3   Output
	//  LED1        Bit 2   Output
	//
	GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);

	//
	// Set the link status based on the LED0 signal (which defaults to link
	// status in the PHY).
	//
	EthernetConnected = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ? false : true;

	// Init the lwip stack

	lwIPInit((const unsigned char *)macAddr, g_sParameters.ulStaticIP, g_sParameters.ulSubnetMask,
	             g_sParameters.ulGatewayIP, ((g_sParameters.flags &
	             CONFIG_FLAG_STATICIP) ? IPADDR_USE_STATIC : IPADDR_USE_DHCP));


	//lwIPInit((const unsigned char*)macAddr, 0, 0, 0, IPADDR_USE_DHCP);

	SntpInit();
}
