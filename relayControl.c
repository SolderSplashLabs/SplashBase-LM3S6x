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

#define RELAYCONTROL
#include "SplashBaseHeaders.h"


// *****************************************************************************
//
// RelayInit : Configure IO required to control the relays
//
// *****************************************************************************
void RelayInit( void )
{
	volatile ui32 dummy;

	if ( SystemConfig.flags.FourRelayEnable )
	{
		GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, 0);

		// Set the relay lines as outputs
		GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, RELAY_OUT_BITS);

		UserGpio_AppSetMask(USER_GPIO_PORTD, RELAY_OUT_BITS);
	
		RelayControl( 0, RELAY_OUT_BITS );
	}
	else
	{
		// Do nothing
	}
}

// *****************************************************************************
//
// RelayControl : Control the supplied relay/s
//
// *****************************************************************************
void RelayControl( ui8 closedRelays, ui8 mask )
{
	if ( mask & BIT0 )
	{
		if (closedRelays & BIT0)
		{
			RelaysClosed |= RELAY_1_BIT;
		}
		else
		{
			RelaysClosed &= ~RELAY_1_BIT;
		}
	}
	
	if ( mask & BIT1 )
	{
		if (closedRelays & BIT1)
		{
			RelaysClosed |= RELAY_2_BIT;
		}
		else
		{
			RelaysClosed &= ~RELAY_2_BIT;
		}
	}
	
	if ( mask & BIT2 )
	{
		if (closedRelays & BIT2)
		{
			RelaysClosed |= RELAY_3_BIT;
		}
		else
		{
			RelaysClosed &= ~RELAY_3_BIT;
		}
	}
	
	if ( mask & BIT3 )
	{
		if (closedRelays & BIT3)
		{
			RelaysClosed |= RELAY_4_BIT;
		}
		else
		{
			RelaysClosed &= ~RELAY_4_BIT;
		}
	}
	
	// Apply it to the port	if relay output is allowed
	if ( SystemConfig.flags.FourRelayEnable )
	{
		GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, RelaysClosed);
	}
	

}

void relaysOn( void )
{
	if ( SystemConfig.flags.FourRelayEnable )
	{
		GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, RelaysClosed);
	}
}

void relaysOff( void )
{
	if ( SystemConfig.flags.FourRelayEnable )
	{
		GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, 0);
	}
}

ui8 RelayGetClosed( void )
{
ui8 result = 0;

	if ( RelaysClosed & RELAY_1_BIT )
	{
		result |= BIT0;
	}

	if ( RelaysClosed & RELAY_2_BIT )
	{
		result |= BIT1;
	}

	if ( RelaysClosed & RELAY_3_BIT )
	{
		result |= BIT2;
	}

	if ( RelaysClosed & RELAY_4_BIT )
	{
		result |= BIT3;
	}

	return( result );
}
