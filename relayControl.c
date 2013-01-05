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

#include "globals.h"

#define RELAYCONTROL
#include "relayControl.h"

void relayInit( void )
{
	volatile ui32 dummy;

	// Allow relay control
	RelayStatus = 1;
	
	// Enable Portd    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Do a dummy read to insert a few cycles after enabling the peripheral.
    dummy = SYSCTL_RCGC2_R;
    dummy = SYSCTL_RCGC2_R;

    GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, 0);

    // Set the relay lines as outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, RELAY_OUT_BITS);
    
    relayControl( 0, RELAY_OUT_BITS );
}

void relayControl( ui8 closedRelays, ui8 mask )
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
	if (RelayStatus)
	{
		GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, RelaysClosed);
	}
	

}

void relaysOn( void )
{
	GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, RelaysClosed);
	
	RelayStatus = 1;
}

void relaysOff( void )
{
	GPIOPinWrite(GPIO_PORTD_BASE, RELAY_OUT_BITS, 0);
	
	RelayStatus = 0;
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
