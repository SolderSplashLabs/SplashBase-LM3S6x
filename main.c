/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

// Remember to include add the root of your Stellaris directory to the project include directory
// Project > Build > Compiler > Include Options > Include Path
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "upnp.h"

#include "datatypes.h"
#include "globals.h"

#include "config.h"

#include "ethernetControl.h"
#include "logicController.h"
#include "adcControl.h"
#include "colourModes.h"
#include "pwmControl.h"
#include "relayControl.h"
#include "udpControl.h"
#include "solderBridge/solderBridgeSpi.h"
#include "time.h"

// *****************************************************************************
// SysTickIntHandler
// Called By the SysTick Interrupt
// *****************************************************************************
void SysTickIntHandler(void)
{
	// Keep track of time!
	Time_Task();

	ColourModeTick();

	// Service the ethernet and TCP Stack
	Ethernet_Task();

	// Measure any analogue inputs
	AdcTask();

	// The Logic task handles all of the if 'this' then 'that' functionality
	LogicTask();
}

// *****************************************************************************
// IdleTasks
// Tasks to perform while idle
// *****************************************************************************
void IdleTasks ( void )
{
	// Tasks performed in the Idle task can and will be interrupted so only tasks that can handle
	// that can be performed. they may also be starved if the scheduled tasks are busy.
}


// *****************************************************************************
// InitialiseHW
// Setup the processor
// *****************************************************************************
void InitialiseHW ( void )
{
	// If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
	// a workaround to allow the PLL to operate reliably.
	//
	if(REVISION_IS_A2)
	{
		SysCtlLDOSet(SYSCTL_LDO_2_75V);
	}

	// 50 MHz
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
				   SYSCTL_XTAL_8MHZ);


	// Enable Peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	// Set interrupt priority levels
	IntPrioritySet(INT_ETH, 0x20);
	IntPrioritySet(FAULT_SYSTICK, 0x40);

	AdcInit();
	pwmInit();
	relayInit();

	// Note (From the DriverLib) :
	// It takes five clock cycles after the write to enable a peripheral
	// before the the peripheral is actually enabled.  During this time, attempts
	// to access the peripheral result in a bus fault.  Care should be taken
	// to ensure that the peripheral is not accessed during this brief time
	// period.

	UPnPInit();

	Ethernet_Init();

	LogicStartStop(true);

	// Most, if not all M3's have a SysTick which you can use for scheduling your code
	SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
	SysTickEnable();
	SysTickIntEnable();
}

// *****************************************************************************
// main
// Start of the C Code
// *****************************************************************************
int main(void)
{
	volatile ui8 tmpMacAddr[8];

	InitialiseHW();

	// Grab the Config from Flash
	ConfigInit();

	Ethernet_GetMacAddress((ui8 *)&tmpMacAddr[0]);

	SSC_Init();
	SSC_MACAddrSet((ui8 *)tmpMacAddr);
	SSC_SetUnitName((ui8 *)g_sParameters.ucModName);

	SSC_SetRelayName((ui8 *)g_sParameters.relayOneName, 0);
	SSC_SetRelayName((ui8 *)g_sParameters.relayTwoName, 1);
	SSC_SetRelayName((ui8 *)g_sParameters.relayThreeName, 2);
	SSC_SetRelayName((ui8 *)g_sParameters.relayFourName, 3);

	while (1)
	{
		IdleTasks();
	}
	
}
