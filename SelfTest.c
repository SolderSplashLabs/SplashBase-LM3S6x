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
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/hw_nvic.h"
#include "inc/lm3s6432.h"
#include "datatypes.h"

#include "utils/uartstdio.h"

#include "adcControl.h"
#include "ethernetControl.h"

#include "globals.h"
*/

#ifdef SELF_TEST

const typedef enum TEST_FAIL_TYPE
{
	TEST_FAIL_IO1_1 = 1,
	TEST_FAIL_IO1_2,
	TEST_FAIL_IO1_3,
	TEST_FAIL_IO1_4,
	TEST_FAIL_IO1_5,
	TEST_FAIL_IO1_6,
	TEST_FAIL_IO1_7,
	TEST_FAIL_IO2_1,
	TEST_FAIL_IO2_2,
	TEST_FAIL_IO2_3,
	TEST_FAIL_IO2_4,
	TEST_FAIL_IO2_5,
	TEST_FAIL_IO2_6,
	TEST_FAIL_IO2_7,
	TEST_FAIL_IO2_8,
	TEST_FAIL_IO2_9,
	TEST_FAIL_EEPROM
} TEST_FAIL_TYPE;

//*****************************************************************************
//
// SelfTestFail - Print test failure results
//
//*****************************************************************************
void SelfTestFail ( TEST_FAIL_TYPE reason )
{
	UARTprintf("Self Test Failure : %u\n", reason);
}

//*****************************************************************************
//
// SelfTestPair -
//
//*****************************************************************************
bool SelfTestPair ( ui32 port1, ui32 port1Bit, ui32 port2, ui32 port2Bit, TEST_FAIL_TYPE testNo )
{
bool result = false;
ui8 i = 0;

	GPIOPinWrite(port1, port1Bit, 0);
	GPIOPinTypeGPIOOutput(port1, port1Bit);

	for (i=0; i<100; i++);

	GPIOPinTypeGPIOInput(port2, port2Bit);

	if ( GPIOPinRead(port2, port2Bit) )
	{
		// Wrong
		SelfTestFail(testNo);
	}
	else
	{
		// Good
		result = true;
	}

	GPIOPinWrite(port1, port1Bit, port1Bit);

	for (i=0; i<100; i++);

	if ( GPIOPinRead(port2, port2Bit) )
	{
		// Good
		result = true;
	}
	else
	{
		// Wrong
		SelfTestFail(testNo);
	}

	return( result );
}

//*****************************************************************************
//
// SelfTest -
//
//*****************************************************************************
void SelfTest ( void )
{
ui16 adcValues[3];
ui8 macAddr[8];

	// Is Mac address Default?
	Ethernet_GetMacAddress(&macAddr[0]);
	if ( macAddr[1] == 0xAA ) SelfTestFail(TEST_FAIL_EEPROM);

	// Test Push Button

	// Examine ADC Values are they correct
	AdcAllAdcResults(&adcValues[0], sizeof(adcValues));
	UARTprintf("ADCs : %u, %u\n", adcValues[1], adcValues[2]);

	// Print Temperature Sensor value
	CMD_GetTemperature( 0, 0 );

	// --------------------------------------------------
	// IO1

	// PF1 <-> PF0 <-> PA7
	SelfTestPair ( GPIO_PORTF_BASE, BIT1, GPIO_PORTF_BASE, BIT0, TEST_FAIL_IO1_1 );
	SelfTestPair ( GPIO_PORTF_BASE, BIT1, GPIO_PORTA_BASE, BIT7, TEST_FAIL_IO1_1 );
	// PG1 <-> PA6
	SelfTestPair ( GPIO_PORTG_BASE, BIT1, GPIO_PORTA_BASE, BIT6, TEST_FAIL_IO1_2 );
	// PG0 <-> PA5
	SelfTestPair ( GPIO_PORTG_BASE, BIT0, GPIO_PORTA_BASE, BIT5, TEST_FAIL_IO1_3 );
	// PC7 <-> PA4
	SelfTestPair ( GPIO_PORTC_BASE, BIT7, GPIO_PORTA_BASE, BIT4, TEST_FAIL_IO1_4 );
	// PC6 <-> PA2
	SelfTestPair ( GPIO_PORTC_BASE, BIT6, GPIO_PORTA_BASE, BIT2, TEST_FAIL_IO1_5 );
	// PC5 <-> PA1
	SelfTestPair ( GPIO_PORTC_BASE, BIT5, GPIO_PORTA_BASE, BIT1, TEST_FAIL_IO1_6 );
	// PC4 <-> PA0
	SelfTestPair ( GPIO_PORTC_BASE, BIT4, GPIO_PORTA_BASE, BIT0, TEST_FAIL_IO1_7 );

	// --------------------------------------------------
	// IO2

	// PE4 <-> PE3
	SelfTestPair ( GPIO_PORTE_BASE, BIT4, GPIO_PORTE_BASE, BIT3, TEST_FAIL_IO2_1 );
	// PE2 <-> PE1
	SelfTestPair ( GPIO_PORTE_BASE, BIT2, GPIO_PORTE_BASE, BIT1, TEST_FAIL_IO2_2 );
	// PE0 <-> PD7
	SelfTestPair ( GPIO_PORTE_BASE, BIT0, GPIO_PORTD_BASE, BIT7, TEST_FAIL_IO2_3 );
	// PB6 <-> PD6
	SelfTestPair ( GPIO_PORTB_BASE, BIT6, GPIO_PORTD_BASE, BIT6, TEST_FAIL_IO2_4 );
	// PB5 <-> PB4
	SelfTestPair ( GPIO_PORTB_BASE, BIT5, GPIO_PORTB_BASE, BIT4, TEST_FAIL_IO2_5 );
	// PB3 <-> PD5
	SelfTestPair ( GPIO_PORTB_BASE, BIT3, GPIO_PORTD_BASE, BIT5, TEST_FAIL_IO2_6 );
	// PB2 <-> PD4
	SelfTestPair ( GPIO_PORTB_BASE, BIT2, GPIO_PORTD_BASE, BIT4, TEST_FAIL_IO2_7 );
	// PB1 <-> PD1
	SelfTestPair ( GPIO_PORTB_BASE, BIT1, GPIO_PORTD_BASE, BIT1, TEST_FAIL_IO2_8 );
	// PB0 <-> PD0
	SelfTestPair ( GPIO_PORTB_BASE, BIT0, GPIO_PORTD_BASE, BIT0, TEST_FAIL_IO2_9 );


	// Reset Processor to Re-Init
	UARTprintf("Reboot to restore IO Configuration");
}


#endif
