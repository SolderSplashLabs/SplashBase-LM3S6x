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
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"

#include "datatypes.h"

// Each 32GPIO SolderBridge
// - Has 2 PCA9555D
// - Can Control 32GPIO
// - Each channel can be an input or an output

// i2c address are : 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
//
// For simplicity we shall imagine each PCA chip is just another GPIO port
// The mapping of port names to I2C addresses is as follows
// Address 0x20 - PORTI
// Address 0x21 - PORTJ
// Address 0x22 - PORTK
// Address 0x23 - PORTL
// Address 0x24 - PORTM
// Address 0x25 - PORTN
// Address 0x26 - PORTO
// Address 0x27 - PORTP

#define SBIO_PORTI_ADDR		0x20
#define SBIO_PORTJ_ADDR		0x21

#define SBIO_PORTK_ADDR		0x22
#define SBIO_PORTL_ADDR		0x23

#define SBIO_PORTM_ADDR		0x24
#define SBIO_PORTN_ADDR		0x25

#define SBIO_PORTO_ADDR		0x26
#define SBIO_PORTP_ADDR		0x27

enum SBIO
{
	EXTIO_PORTI,
	EXTIO_PORTJ,
	EXTIO_PORTK,
	EXTIO_PORTL,
	EXTIO_PORTM,
	EXTIO_PORTN,
	EXTIO_PORTO,
	EXTIO_PORTP,
	EXTIO_MAX
};

typedef struct PORTS_AVAILIBLE
{
	ui8 PortI:1;
	ui8 PortJ:1;
	ui8 PortK:1;
	ui8 PortL:1;
	ui8 PortM:1;
	ui8 PortN:1;
	ui8 PortO:1;
	ui8 PortP:1;
} PORTS_AVAILIBLE;

PORTS_AVAILIBLE PortsAvilible;

// Array of the port state, 1=high
ui16 SBIO_PortOut[SBIO_MAX];
ui16 SBIO_PortIn[SBIO_MAX];

// Array of the port direction, 1=input
ui16 SBIO_PortDir[SBIO_MAX];


// *****************************************************************************
// SBIO_Task
//
// *****************************************************************************
void SBIO_Task ( void )
{
	// Update each IO SolderBridge
}

// *****************************************************************************
// SBIO_Task
//
// *****************************************************************************
bool SBIO_RollCall ( void )
{
	// TODO : find all attached PCA9555D's

	// TODO : for now cheat
	PortsAvilible.PortI = true;
	PortsAvilible.PortJ = true;

	return ( true );
}

// *****************************************************************************
// SBIO_SetPort
// Set port output
// *****************************************************************************
bool SBIO_SetPort( ui8 port, ui16 value, ui16 mask )
{
bool result = false;

	if ( port < SBIO_MAX )
	{
		// TODO : Im tired, this needs set only the bits in the Out that are masked.
		SBIO_PortOut[port] = value;

		// trigger i2c update

		result = true;
	}

	return ( result );
}

// *****************************************************************************
// SBIO_SetPort
// Set port output
// *****************************************************************************
bool SBIO_SetPortBit( ui8 port, ui16 bitMask )
{
bool result = false;

	if ( port < SBIO_MAX )
	{
		// TODO : Im tired, this needs set only the bits in the Out that are masked.
		SBIO_PortOut[port] |= bitMask;

		// trigger i2c update

		result = true;
	}

	return ( result );
}

// *****************************************************************************
// SBIO_SetPort
// Set port output
// *****************************************************************************
bool SBIO_ClearPortBit( ui8 port, ui16 bitMask )
{
bool result = false;

	if ( port < SBIO_MAX )
	{
		// TODO : Im tired, this needs set only the bits in the Out that are masked.
		SBIO_PortOut[port] &= ~bitMask;

		// trigger i2c update

		result = true;
	}

	return ( result );
}

// *****************************************************************************
// SBIO_GetPort
// Get port output
// *****************************************************************************
ui16 SBIO_GetPort( ui8 port )
{
	return ( SBIO_PortOut[port] );
}

