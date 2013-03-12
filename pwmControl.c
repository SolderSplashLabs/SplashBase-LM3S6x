/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel

*/

#define PWMCONTROL
#include "SplashBaseHeaders.h"

// Each 32 timer can be split into 2
// each 16bit chunk is assigned to a CCP output
// in periodic timer, the timer counts down from periodic interval value 
// upon starting the count down the associated CCP line is asserted
// when the counter matches the Match register the associated CCP line is lowered


// TIMER 0 A=CCP0 B=CCP1
// TIMER 1 A=CCP2

void pwmInit( void )
{
	volatile ui32 periodVal;
	volatile ui32 dutyVal;
	
	// Enable Clocking to the periph
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	
	// Set up the PINs
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_6);
	
	// 2 x 16bit timers (A&B)
	TimerConfigure(TIMER0_BASE, TIMER_CFG_16_BIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_16_BIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
	
	// 1khz PWM
	periodVal = (SysCtlClockGet() / 10000) - 1;
	PwmSettings.freq = periodVal;
	TimerLoadSet(TIMER0_BASE, TIMER_BOTH, periodVal );
	TimerLoadSet(TIMER1_BASE, TIMER_BOTH, periodVal );
	
	// Set up the match value,
	dutyVal = periodVal -1;
	pwmSetDuty(dutyVal, 0x07);
	
	// Enable the Timers
	TimerEnable(TIMER0_BASE, TIMER_BOTH);
	TimerEnable(TIMER1_BASE, TIMER_BOTH);
	
	// if we turn off we need to be able to use GPIOA as IO
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		
	PwmStatus = 1;
}

void pwmOn( void )
{
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_6);
	
	// Tell the UserIOmodule, so that control of these pins is blocked by the user
	UserGpio_AppSetMask(USER_GPIO_PORTB, GPIO_PIN_0 | GPIO_PIN_1);
	UserGpio_AppSetMask(USER_GPIO_PORTA, GPIO_PIN_6);

	// Turn PWM on using current set values	
	pwmSetFreq( PwmSettings.freq, 0x07 );
	
	pwmSetDuty( PwmSettings.duty0, 0x01 );
	pwmSetDuty( PwmSettings.duty1, 0x02 );
	pwmSetDuty( PwmSettings.duty2, 0x04 );
	
	TimerEnable(TIMER0_BASE, TIMER_BOTH);
	TimerEnable(TIMER1_BASE, TIMER_BOTH);
	
	PwmStatus = 1;
}

void pwmOff( void )
{
	// Turn off the Pwm outputs	
	TimerDisable(TIMER0_BASE, TIMER_BOTH);
	TimerDisable(TIMER1_BASE, TIMER_BOTH);
	
	// Force lines low
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	GPIOPinWrite(GPIO_PORTB_BASE, (BIT0 | BIT1), 0);
	
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);
	GPIOPinWrite(GPIO_PORTA_BASE, BIT6, 0);
	
	// Tell the UserIOmodule, so that control of these pins is allowed by the user
	UserGpio_AppClrMask(USER_GPIO_PORTB, GPIO_PIN_0 | GPIO_PIN_1);
	UserGpio_AppClrMask(USER_GPIO_PORTA, GPIO_PIN_6);

	PwmStatus = 0;
}

// duty to set the CCP registers based on the mask
void pwmSetDuty( ui16 dutyVal, ui8 mask )
{
	if ( dutyVal >= PwmSettings.freq )
	{
		dutyVal = PwmSettings.freq - 1;
	}
	
	// update CCP0?
	if ( mask & BIT0)
	{
		TimerMatchSet(TIMER0_BASE, TIMER_A, dutyVal);
		PwmSettings.duty0 = dutyVal;
	}
	
	// update CCP1?
	if ( mask & BIT1)
	{
		TimerMatchSet(TIMER0_BASE, TIMER_B, dutyVal);
		PwmSettings.duty1 = dutyVal;
	}
	
	// update CCP2?
	if ( mask & BIT2)
	{
		TimerMatchSet(TIMER1_BASE, TIMER_A, dutyVal);
		PwmSettings.duty2 = dutyVal;
	}
	
}

void pwmSetFreq( ui16 periodVal, ui8 mask )
{
	PwmSettings.freq = periodVal;
	
	if (PwmSettings.duty0 >= PwmSettings.freq)
	{
		 pwmSetDuty(PwmSettings.freq-1, 0x01);
	}
	
	if (PwmSettings.duty1 >= PwmSettings.freq)
	{
		 pwmSetDuty(PwmSettings.freq-1, 0x02);
	}
	
	if (PwmSettings.duty2 >= PwmSettings.freq)
	{
		 pwmSetDuty(PwmSettings.freq-1, 0x04);
	}
	
	// update CCP0?
	if ( mask & BIT0)
	{
		TimerLoadSet(TIMER0_BASE, TIMER_A, periodVal);
	}
	
	// update CCP1?
	if ( mask & BIT1)
	{
		TimerLoadSet(TIMER0_BASE, TIMER_B, periodVal);
	}
	
	// update CCP2?
	if ( mask & BIT2)
	{
		TimerLoadSet(TIMER1_BASE, TIMER_A, periodVal);
		TimerLoadSet(TIMER1_BASE, TIMER_B, periodVal);
	}	
}

ui16 pwmGetDuty( ui8 pwmNo )
{
	ui16 retVal;
	switch ( pwmNo )
	{
		case 0 :
			retVal = PwmSettings.duty0;
		break;
		
		case 1 :
			retVal = PwmSettings.duty1;
		break;
		
		case 2 :
			retVal = PwmSettings.duty2;
		break;
		
		default :
		break;	
	}
	return ( retVal );
} 

ui16 pwmGetFreq( void )
{
	return( TimerLoadGet( TIMER0_BASE, TIMER_A) );
}
