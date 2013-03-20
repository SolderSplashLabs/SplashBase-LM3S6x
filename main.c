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

// Remember to include add the root of your Stellaris directory to the project include directory
// Project > Build > Compiler > Include Options > Include Path

#include "SplashBaseHeaders.h"

bool FirmwareUpdateReq = false;

// *****************************************************************************
// SysTickIntHandler
// Called By the SysTick Interrupt
// *****************************************************************************
void SysTickIntHandler(void)
{
	// Keep track of time!
	Time_Task();

	#ifdef SOLDERBRIDGES_ENABLED
		SolderBridge_Task();

		ExtGpio_Task();
	#endif

	#ifdef SERIAL_ENABLED
		Serial_Task();
	#endif

	#ifdef SPLASHPIXEL_ENABLED
		SP_Task();
	#endif

	ColourModeTick();

	// Service the Ethernet and TCP Stack
	Ethernet_Task();

	// Measure any analogue inputs
	AdcTask();

	#ifdef LOGIC_ENABLED
		// The Logic task handles all of the if 'this' then 'that' functionality
		LogicTask();
	#endif

}

// *****************************************************************************
// UpdateFirmware
// Do not call from an interrupt! - Jumps to the bootloader for flash update
// *****************************************************************************
static void UpdateFirmware ( void )
{
	//
	// Disable all processor interrupts.  Instead of disabling them
	// one at a time (and possibly missing an interrupt if new sources
	// are added), a direct write to NVIC is done to disable all
	// peripheral interrupts.
	//
	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;

	//
	// Also disable the SysTick interrupt.
	//
	SysTickIntDisable();

	//
	// Return control to the boot loader.  This is a call to the SVC
	// handler in the boot loader, or to the ROM if available.
	//
	(*((void (*)(void))(*(unsigned long *)0x2c)))();
}

// *****************************************************************************
// IdleTasks
// Tasks to perform while idle
// *****************************************************************************
void UpdateFirmwareReq ( void )
{
	// TODO : Put some protection around this.
	// TODO : Allow it to be disabled
	FirmwareUpdateReq = true;
}

// *****************************************************************************
// IdleTasks
// Tasks to perform while idle
// *****************************************************************************
void IdleTasks ( void )
{
	// Tasks performed in the Idle task can and will be interrupted so only tasks that can handle
	// that can be performed. they may also be starved if the scheduled tasks are busy.

	if ( FirmwareUpdateReq )
	{
		UpdateFirmware();
	}
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
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

	// Enable Peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	// Set interrupt priority levels
	IntPrioritySet(INT_ETH, 0x20);
	IntPrioritySet(FAULT_SYSTICK, 0x40);

	//
	// Enable the peripherals that should continue to run when the processor
	// is sleeping.
	//
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOH);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART1);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_ETH);

	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER1);
	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER2);

    //
    // Enable peripheral clock gating.  Note that this is required in order to
    // measure the the processor usage.
    //
    SysCtlPeripheralClockGating(true);

	// Grab the Config from Flash
	SysConfigInit();

    // Set up the GPIO as specified by the user
    UserGpioInit();

	AdcInit();
	pwmInit();
	RelayInit();

	//usrand(0x23482937);

	// Note (From the DriverLib) :
	// It takes five clock cycles after the write to enable a peripheral
	// before the the peripheral is actually enabled.  During this time, attempts
	// to access the peripheral result in a bus fault.  Care should be taken
	// to ensure that the peripheral is not accessed during this brief time
	// period.

	#ifdef SERIAL_ENABLED
		Serial_Init();
	#endif

	#ifdef UPNP_ENABLED
		UPnPInit();
	#endif

	Ethernet_Init();

	#ifdef LOGIC_ENABLED
		LogicStartStop(true);
	#endif

	#ifdef SOLDERBRIDGES_ENABLED
		SB_Init();
		ExtGpio_Init();

		SolderBridge_StartScan();
		ExtGpio_Scan();
	#endif

	#ifdef SPLASHPIXEL_ENABLED
		SP_Init();
	#endif

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

	Ethernet_GetMacAddress((ui8 *)&tmpMacAddr[0]);

	// Initalise the SolderSplash UDP Coms
	SSC_Init();
	SSC_MACAddrSet((ui8 *)tmpMacAddr);
	SSC_SetUnitName((ui8 *)SystemConfig.splashBaseName);

	SSC_SetRelayNames();

	while (1)
	{
		IdleTasks();
	}
	
}
