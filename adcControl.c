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
#include "inc/hw_nvic.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "datatypes.h"

#include "adcControl.h"
*/

#define ADC_MV_PER_CNT			2.932551
#define ADC_MV_TEMP_ZERO		500
#define ADC_MV_PER_DEG			19.5


unsigned long AdcResults[4];

unsigned long Adc0TotalAvg;
unsigned long Adc1TotalAvg;
unsigned long Adc2TotalAvg;

unsigned long LastAdc0Value;
unsigned long LastAdc1Value;
unsigned long LastAdc2Value;
unsigned long LastAdc3Value;

//*****************************************************************************
//
// AdcTask -
//
//*****************************************************************************
void AdcTask ( void )
{
volatile ui32 temp;
static bool adcIdle = false;

	if ( ADCIntStatus(ADC0_BASE, 2, false) )
	{
		//
		// Clear the ADC interrupt flag.
		//
		ADCIntClear(ADC0_BASE, 2);

		//
		// Read ADC Value.
		//
		ADCSequenceDataGet(ADC0_BASE, 2, &AdcResults[0]);

		LastAdc0Value = AdcResults[0];
		LastAdc1Value = AdcResults[1];
		LastAdc2Value = AdcResults[2];
		LastAdc3Value = AdcResults[3];

		adcIdle = true;
	}
	else
	{
		if ( adcIdle )
		{
			//
			// Trigger the ADC conversion.
			//
			ADCProcessorTrigger(ADC0_BASE, 2);
			adcIdle = false;
		}
	}
}

//*****************************************************************************
//
// AdcTask -
//
//*****************************************************************************
void AdcInit ( void )
{
	//
	// The ADC0 peripheral must be enabled for use.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	//
	// For this example ADC0 is used with AIN0 on port E7.
	// The actual port and pins used may be different on your part, consult
	// the data sheet for more information.  GPIO port E needs to be enabled
	// so these pins can be used.
	// TODO: change this to whichever GPIO port you are using.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	//
	// Select the analog ADC function for these pins.
	// Consult the data sheet to see which functions are allocated per pin.
	// TODO: change this to select the port/pin you are using.
	//
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	//
	// Enable sample sequence 2 ( 4 steps ), triggered by the processor
	//
	ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);

	//
	// Configure each step of sequence 2, ADC0,1,2 and MCU temp sensors
	//
	ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0 );
	ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH1 );
	ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH2 );
	ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);

	ADCHardwareOversampleConfigure(ADC0_BASE, 64);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_125KSPS);

	//
	// Since sample sequence 2 is now configured, it must be enabled.
	//
	ADCSequenceEnable(ADC0_BASE, 2);

	//
	// Clear the interrupt status flag.  This is done to make sure the
	// interrupt flag is cleared before we sample.
	//
	ADCIntClear(ADC0_BASE, 2);

	//
	// Trigger the ADC conversion for sequence 2
	//
	ADCProcessorTrigger(ADC0_BASE, 2);


}

//*****************************************************************************
//
// AdcLastAdcResult -
//
//*****************************************************************************
ui32 AdcGetResult ( ui8 channel )
{
volatile ui32 result = 0;

	switch ( channel )
	{
		case 0 :
			result = LastAdc0Value;
		break;

		case 1 :
			result = LastAdc1Value;
		break;

		case 2 :
			result = LastAdc2Value;
		break;
	}

	return ( result );
}

//*****************************************************************************
//
// AdcAllAdcResults - Retreve all 3 ADC results
//
//*****************************************************************************
void AdcAllAdcResults ( ui16 *resultsBuff, ui8 buffSize )
{
	resultsBuff[0] = LastAdc0Value;
	resultsBuff[1] = LastAdc1Value;
	resultsBuff[2] = LastAdc2Value;
}

//*****************************************************************************
//
// AdcGetTemperature - Convert ADC0 in to a temperature
//
//*****************************************************************************
ui32 AdcGetTemperature ( void )
{
volatile float result = 0;

	result = LastAdc0Value;

	result = (float)(result * (float)ADC_MV_PER_CNT) - ADC_MV_TEMP_ZERO;

	result = result / ADC_MV_PER_DEG;

	return( result );
}


//*****************************************************************************
//
// AdcGetInternalTemp - Convert MCU temperature Diode adc level in to a temperature
//
//*****************************************************************************
ui32 AdcGetInternalTemp ( void )
{
ui32 ulTemp_ValueC = 0;
	// Taken from TI Sample Code
	//
	// Use non-calibrated conversion provided in the data sheet.  Make
	// sure you divide last to avoid dropout.
	//
	ulTemp_ValueC = ((1475 * 1023) - (2250 * LastAdc3Value)) / 10230;

	return( ulTemp_ValueC );
}
