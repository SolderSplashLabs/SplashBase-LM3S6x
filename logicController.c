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
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "lwiplib.h"
#include "utils/ustdlib.h"

#include "datatypes.h"

#include "config.h"

#include "adcControl.h"
#include "pwmControl.h"
#include "relayControl.h"
#include "ethernetControl.h"
#include "time.h"
#include "cosm.h"
#include "solderBridge/servoSolderBridge.h"

#define _LOGIC_H_
#include "logicController.h"


// TODO : tidy up
//LOGIC_CONDITION LogicConditions[LOGIC_MAX_CONDITIONS];
LOGIC_CONDITION *LogicConditions;

ui32 GPIO_REGISTERS[] = { GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE, GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTF_BASE, GPIO_PORTG_BASE, GPIO_PORTH_BASE };

ui16 LogicRegisters[LOGIC_NUM_OF_REGISTERS];

ui8 LogicRunning = false;

ui8 Logic_NoOfConditions = 0;
ui8 cosmLimit = 0;

bool networkConnected = false;
bool prevNetworkConnected = false;
bool justBooted = true;
tTime currentTime;

// TODO : Add function to return the state of this flag, so user can tell if changes to the logic controller have been saved
bool LogicCondInSync = TRUE;

//*****************************************************************************
//
// LogicTask - Called at a set rate to check for actions to perform.
//
//*****************************************************************************
void LogicTask ( void )
{
ui8 i = 0;

	if ( LogicRunning )
	{
		ulocaltime( Time_StampNow(), &currentTime );
		LogicCapturePortData();

		networkConnected = Ethernet_Connected();

		for (i=0; i<LOGIC_MAX_CONDITIONS;i++)
		{
			if ( LogicConditions[i].active )
			{
				// has the first event occurred?
				if ( LogicProcessCondition ( i, false ) )
				{
					if ( L_EVENT_INVALID != LogicConditions[i].andEventType )
					{
						// This is an dual event condition, both events have to be true before activating both actions
						if ( LogicProcessCondition ( i, true ) )
						{
							// both are true
							LogicTakeAction(i);
						}
					}
					else
					{
						// Only a single event, we should now take action
						LogicTakeAction(i);
					}
				}


			}
		}

		if (cosmLimit) cosmLimit --;

		prevNetworkConnected = networkConnected;

		// If we needed to action on boot up it would have been done now .
		justBooted = false;
	}
}


//*****************************************************************************
//
// LogicSaveConditions
// Saves the logic conditions to flash so that they are used upon power up
//
//*****************************************************************************
void LogicSaveConditions ( void )
{
	memcpy(	g_sWorkingDefaultParameters.LogicConditionsBuffer, g_sParameters.LogicConditionsBuffer, sizeof(g_sWorkingDefaultParameters.LogicConditionsBuffer));

	LogicCondInSync = TRUE;
}

//*****************************************************************************
//
// LogicProcessCondition - Process the conditions event, return true if it should actioned
//
//*****************************************************************************
bool LogicProcessCondition ( ui8 conditionNo, bool secondaryEvent )
{
bool result = false;
ui32 param1;
ui32 param2;
LOGIC_EVENT_TYPE event;
volatile ui8 tempUi8 = 0;


	if ( secondaryEvent )
	{
		param1 = LogicConditions[conditionNo].andEventParam1;
		param2 = LogicConditions[conditionNo].andEventParam2;
		event = LogicConditions[conditionNo].andEventType;
	}
	else
	{
		param1 = LogicConditions[conditionNo].eventParam1;
		param2 = LogicConditions[conditionNo].eventParam2;
		event = LogicConditions[conditionNo].eventType;
	}

	switch ( event )
	{
		case L_EVENT_GPIO_RAISING :
			if (LogicConditions[conditionNo].actioned)
			{
				// previous the line was still high, is it low now?
				if ( LogicCheckPinLow( conditionNo ) )
				{
					// It's low, we are now free to trigger the action if raised
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else if ( LogicCheckPinHigh(conditionNo) )
			{
				result = true;
			}
		break;

		case L_EVENT_GPIO_FALLING :
			if (LogicConditions[conditionNo].actioned)
			{
				// previous the line was still high, is it low now?
				if ( LogicCheckPinHigh(conditionNo) )
				{
					// It's low, we are now free to trigger the action if raised
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else if ( LogicCheckPinLow(conditionNo) )
			{
				result = true;
			}
		break;

		case L_EVENT_ADC_ABOVE :
			if ( AdcGetResult( param1 ) > param2 )
			{
				result = true;
			}
		break;

		case L_EVENT_ADC_BELOW :
			if ( AdcGetResult( param1 ) < param2 )
			{
				result = true;
			}
		break;

		case L_EVENT_NET_DISCONNECT :

			// Only fired once, upon disconnect
			// TODO : Should it be constant?
			if (! networkConnected )
			{
				if (prevNetworkConnected != networkConnected)
				{
					// We were connected not now!
					result = true;
				}
			}

		break;

		case L_EVENT_NET_CONNECT :

			// Only fired once, upon Connect
			// TODO : Should it be constant?

			if ( networkConnected )
			{
				if (prevNetworkConnected != networkConnected)
				{
					// We were connected not now!
					result = true;
				}
			}

		break;

		case L_EVENT_TEMP_ABOVE :
			if (LogicConditions[conditionNo].actioned)
			{
				if ( AdcGetTemperature() < param2 )
				{
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else
			{
				if ( AdcGetTemperature() > param2 )
				{
					result = true;
				}
			}
		break;

		case L_EVENT_TEMP_BELOW :
			if (LogicConditions[conditionNo].actioned)
			{
				if ( AdcGetTemperature() > param2 )
				{
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else
			{
				if ( AdcGetTemperature() < param2 )
				{
					result = true;
				}
			}
		break;

		case L_EVENT_REG_EQUAL :

			if (param1 < LOGIC_NUM_OF_REGISTERS)
			{
				if (LogicRegisters[param1] == param2)
				{
					if (! LogicConditions[conditionNo].actioned)
					{
						result = true;
					}
				}
				else
				{
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else
			{
				// Invalid register
			}

		break;

		case L_EVENT_REG_ABOVE :
			if (param1 < LOGIC_NUM_OF_REGISTERS)
			{
				if (LogicRegisters[param1] > param2)
				{
					result = true;
				}
				else
				{
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else
			{
				// Invalid register
			}
		break;

		case L_EVENT_REG_BELOW :
			if (param1 < LOGIC_NUM_OF_REGISTERS)
			{
				if (LogicRegisters[param1] < param2)
				{
					result = true;
				}
				else
				{
					LogicConditions[conditionNo].actioned = false;
				}
			}
			else
			{
				// Invalid register
			}
		break;

		case L_EVENT_REPEAT_TIMER :

		break;

		case L_EVENT_GPIO_RAISING_DB :

		break;

		case L_EVENT_GPIO_FALLING_DB :

		break;

		case L_EVENT_BOOT :
			if ( justBooted )
			{
				result = true;
			}
		break;

		case L_EVENT_EVERY_TICK :
			result = true;
		break;

		case L_EVENT_DATE_AFTER :
			if ( param1 > Time_StampNow())
			{
				result = true;
			}
		break;

		case L_EVENT_DATE_BEFORE :
			if ( param1 < Time_StampNow())
			{
				result = true;
			}
		break;

		case L_EVENT_TIME_AFTER :

			// Param1 is packed as follows
			// MSB day of week, hour, LSB minutes, second

			if ( param1 & BIT31 )
			{
				// Don't care about the week day, active every day
				tempUi8 = true;
			}
			else
			{
				// TODO : either have a single day bit set or a bit mask to pick days that this is active on
				tempUi8 = (param1 >> 24);
				if ( tempUi8 & (0x01 << currentTime.ucWday) )
				{
					// it's the current day
					tempUi8 = true;
				}
				else if ((0x01 << currentTime.ucWday) > tempUi8)
				{
					// its after this day in the week
					result = true;
				}
				else
				{
					tempUi8 = false;
				}
			}

			if ((tempUi8) && (!result))
			{
				// Take the hours from the param
				tempUi8 = (param1 >> 16);

				if ( currentTime.ucHour > tempUi8)
				{
					result = true;
				}
				else if ( currentTime.ucHour == tempUi8)
				{
					// get the minutes
					tempUi8 = param1 >> 8;
					if ( currentTime.ucMin >= tempUi8 )
					{
						result = true;
					}
				}
			}

		break;

		case L_EVENT_TIME_BEFORE :

			// Param1 is packed as follows
			// MSB day of week, hour, LSB minutes, second

			if ( param1 & BIT31 )
			{
				// Don't care about the week day, active every day
				tempUi8 = true;
			}
			else
			{
				// TODO : either have a single day bit set or a bit mask to pick days that this is active on
				tempUi8 = (param1 >> 24);
				if ( tempUi8 & (0x01 << currentTime.ucWday) )
				{
					// it's the current day
					tempUi8 = true;
				}
				else if ((0x01 << currentTime.ucWday) < tempUi8)
				{
					// its before this day, so a valid condition
					result = true;
				}
				else
				{
					tempUi8 = false;
				}
			}

			if ((tempUi8) && (!result))
			{
				// Take the hours from the param
				tempUi8 = (param1 >> 16);

				if ( currentTime.ucHour < tempUi8)
				{
					result = true;
				}
				else if ( currentTime.ucHour == tempUi8)
				{
					// get the minutes
					tempUi8 = param1 >> 8;
					if ( currentTime.ucMin < tempUi8 )
					{
						result = true;
					}
				}
			}

		break;

	}

	return ( result );
}

//*****************************************************************************
//
// LogicStartStop - Enable/Disable Logic
//
//*****************************************************************************
void LogicStartStop ( bool start )
{
	// TODO : Remove test code
	// Test statement
	/*
	LogicConditions[0].eventType = L_EVENT_GPIO_FALLING;

	// PB5
	LogicConditions[0].eventParam1 = 1;
	LogicConditions[0].eventParam2 = 0x10;

	LogicConditions[0].actionType = L_ACTION_GPIO_HIGH;
	// PB4
	LogicConditions[0].actionParam1 = 1;
	LogicConditions[0].actionParam2 = 0x04;
	LogicConditions[0].active = true;

	// Test statement 2
	LogicConditions[1].eventType = L_EVENT_GPIO_RAISING;

	// PB5
	LogicConditions[1].eventParam1 = 1;
	LogicConditions[1].eventParam2 = 0x10;

	LogicConditions[1].actionType = L_ACTION_INCREMENT_REG;
	// PB4
	LogicConditions[1].actionParam1 = 0;
	LogicConditions[1].actionParam2 = 1;
	LogicConditions[1].active = true;

	// Test statement3
	LogicConditions[2].eventType = L_EVENT_REG_ABOVE;

	// PB5
	LogicConditions[2].eventParam1 = 0;
	LogicConditions[2].eventParam2 = 9;

	LogicConditions[2].actionType = L_ACTION_CONTROL_RELAY;
	// PB4
	LogicConditions[2].actionParam1 = 0xFF;
	LogicConditions[2].actionParam2 = 0xFF;
	LogicConditions[2].active = true;

	// Test statement4
	LogicConditions[3].eventType = L_EVENT_REG_ABOVE;

	// PB5
	LogicConditions[3].eventParam1 = 0;
	LogicConditions[3].eventParam2 = 9;

	LogicConditions[3].actionType = L_ACTION_CLEAR_REG;
	// PB4
	LogicConditions[3].actionParam1 = 0;
	LogicConditions[3].actionParam2 = 0;
	LogicConditions[3].active = true;

	// Test statement 5 - Adc above mid point
	LogicConditions[4].eventType = L_EVENT_ADC_ABOVE;

	// PB5
	LogicConditions[4].eventParam1 = 0;
	LogicConditions[4].eventParam2 = 512;

	LogicConditions[4].actionType = L_ACTION_CONTROL_RELAY;
	// PB4
	LogicConditions[4].actionParam1 = 0x00;
	LogicConditions[4].actionParam2 = 0xFF;
	LogicConditions[4].active = true;
	*/


	// -----------------------------

	// Set up the conditions from the config
	LogicConditions = g_sParameters.LogicConditionsBuffer;

	// Get the initial state
	networkConnected = Ethernet_Connected();

	LogicRunning = start;
}

//*****************************************************************************
//
// LogicCheckPinHigh - check to see if the pin is high using our cached pin status
//
//*****************************************************************************
bool LogicCheckPinHigh ( ui8 conditionNo )
{
bool result = false;

	if ( LogicGpioData[LogicConditions[ conditionNo ].eventParam1] & LogicConditions[ conditionNo ].eventParam2  )
	{
		result = true;
	}
	else
	{
		result = false;
	}

	return( result );
}

//*****************************************************************************
//
// LogicCheckPinLow - check to see if the pin is low using our cached pin status
//
//*****************************************************************************
bool LogicCheckPinLow ( ui8 conditionNo )
{
bool result = false;

	if ( LogicGpioData[LogicConditions[ conditionNo ].eventParam1] & LogicConditions[ conditionNo ].eventParam2  )
	{
		result = false;
	}
	else
	{
		result = true;
	}

	return( result );
}

//*****************************************************************************
//
// LogicTakeAction - Complete the supplied action
//
//*****************************************************************************
void LogicTakeAction ( ui8 conditionNo )
{
ui32 param1 = LogicConditions[conditionNo].actionParam1;
ui32 param2 = LogicConditions[conditionNo].actionParam2;

	switch ( LogicConditions[ conditionNo ].actionType )
	{
		case L_ACTION_GPIO_HIGH :
			LogicSetPinHigh(conditionNo);

			//UARTprintf("Action : L_ACTION_GPIO_HIGH\r\n");
		break;

		case L_ACTION_GPIO_LOW :
			LogicSetPinLow(conditionNo);

			//UARTprintf("Action : L_ACTION_GPIO_LOW\r\n");
		break;

		case L_ACTION_PWM_DUTY :
			// Param 1 would be the PWM channel, 2 is the duty to set now
			pwmSetDuty(param1, param2);
			//UARTprintf("Action : L_ACTION_PWM_DUTY\r\n");
		break;

		case L_ACTION_INCREMENT_REG :
			if ( LogicRegisters[param1] < 0xFFFFFFFF )
			{
				// Increment the register by the parm2 value
				LogicRegisters[ param1 ] += param2;
			}
			//UARTprintf("Action : L_ACTION_INCREMENT_REG\r\n");
		break;

		case L_ACTION_DECREMENT_REG :
			if ( LogicRegisters[param1] )
			{
				LogicRegisters[ param1 ] --;
			}
			//UARTprintf("Action : L_ACTION_DECREMENT_REG\r\n");
		break;

		case L_ACTION_CLEAR_REG :
			LogicRegisters[param1] = 0;
			//UARTprintf("Action : L_ACTION_CLEAR_REG\r\n");
		break;

		case L_ACTION_SET_REG :
			LogicRegisters[param1] = param2;
			//UARTprintf("Action : L_ACTION_SET_REG\r\n");
		break;

		case L_ACTION_NET_MSG :
			//UARTprintf("Action : L_ACTION_NET_MSG\r\n");
		break;

		case L_ACTION_SERIAL_MSG :
			//UARTprintf("Action : L_ACTION_SERIAL_MSG\r\n");
		break;

		case L_ACTION_CONTROL_RELAY :
			relayControl(param1, param2);
			//UARTprintf("Action : L_ACTION_CONTROL_RELAY\r\n");
		break;

		case L_ACTION_SEND_COSM :
			// TODO : Temporary COSM limit to stop us sending more than one request a second
			if (! cosmLimit )
			{
				CosmGetIp();
				cosmLimit = 100;
			}
		break;

		case L_ACTION_SERVO_POS :
			ServoMoveToPos(param1, param2);
		break;
	}

	LogicConditions[conditionNo].actioned = true;
}

//*****************************************************************************
//
// LogicCheckPinLow - Set Pin on Port High
//
//*****************************************************************************
void LogicSetPinHigh ( ui8 conditionNo )
{
volatile ui32 portRegister = GPIO_REGISTERS[LogicConditions[conditionNo].actionParam1];
volatile ui8 pin = 0;

	pin = LogicConditions[conditionNo].actionParam2;

	GPIOPinWrite( portRegister, pin, pin);
}

//*****************************************************************************
//
// LogicCheckPinHigh - Set Pin on Port Low
//
//*****************************************************************************
void LogicSetPinLow ( ui8 conditionNo )
{
volatile ui32 portRegister = GPIO_REGISTERS[LogicConditions[conditionNo].actionParam1];
ui8 pin = 0;

	pin = LogicConditions[conditionNo].actionParam2;

	GPIOPinWrite( portRegister, pin, 0);
}


//*****************************************************************************
//
// LogicTask - Called at a set rate to check for actions to perform.
//
//*****************************************************************************
void LogicCapturePortData ( void )
{
	// Record the status of all the GPIO ports for interrogation
	LogicGpioData[0] = GPIOPinRead(GPIO_REGISTERS[0], 0xFF);
	LogicGpioData[1] = GPIOPinRead(GPIO_REGISTERS[1], 0xFF);
	LogicGpioData[2] = GPIOPinRead(GPIO_REGISTERS[2], 0xFF);
	LogicGpioData[3] = GPIOPinRead(GPIO_REGISTERS[3], 0xFF);
	LogicGpioData[4] = GPIOPinRead(GPIO_REGISTERS[4], 0xFF);
	LogicGpioData[5] = GPIOPinRead(GPIO_REGISTERS[5], 0xFF);
	LogicGpioData[6] = GPIOPinRead(GPIO_REGISTERS[6], 0xFF);
}

//*****************************************************************************
//
// LogicInsertNewCondition - add a new condition to the list from a coms buffer
//
//*****************************************************************************
void LogicInsertNewCondition (ui8 position, ui8 *newCondition )
{
	if (position<LOGIC_MAX_CONDITIONS)
	{
		LogicConditions[position].eventType = (LOGIC_EVENT_TYPE)newCondition[24];
		LogicConditions[position].andEventType = (LOGIC_EVENT_TYPE)newCondition[25];
		LogicConditions[position].actionType = (LOGIC_ACTION_TYPE)newCondition[26];

		memcpy(&(LogicConditions[position].eventParam1), newCondition, 24);

		LogicConditions[position].active = true;
		LogicConditions[position].actioned = false;

		LogicCondInSync = FALSE;
	}
}
