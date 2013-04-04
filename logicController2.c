
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

#define _LOGIC_H_
#include "SplashBaseHeaders.h"


ui16 LogicRegisters[LOGIC_NUM_OF_REGISTERS];
ui8 Logic_NoOfConditions = 0;
ui8 cosmLimit = 0;

bool networkConnected = false;
bool prevNetworkConnected = false;
bool justBooted = true;
tTime currentTime;


//*****************************************************************************
//
// LogicTask - Called at a set rate to check for actions to perform.
//
//*****************************************************************************
void LogicTask ( void )
{
ui8 i = 0;
ui8 x = 0;
volatile ui8 condNo = 0;
bool cond1Met = false;
bool cond2Met = false;

	if ( SystemConfig.flags.LogicEnabled )
	{
		ulocaltime( Time_StampNow(SystemConfig.timeOffset), &currentTime );
		LogicCapturePortData();

		networkConnected = Ethernet_Connected();

		for (i=0; i<LOGIC_EVENT_CNT; i++)
		{
			condNo = SystemConfig.LogicEvents.conditionNumber1[i];
			if ( condNo < LOGIC_COND_CNT )
			{
				// is the selected condition valid
				if ( L_EVENT_INVALID != SystemConfig.LogicConditions.condType[condNo] )
				{
					if ( LogicProcessCondition(condNo) )
					{
						// Condition has been met
						cond1Met = true;
					}
				}
			}

			condNo = SystemConfig.LogicEvents.conditionNumber2[i];
			if ( condNo < LOGIC_COND_CNT )
			{
				// is the selected condition valid
				if ( L_EVENT_INVALID != SystemConfig.LogicConditions.condType[condNo] )
				{
					if ( LogicProcessCondition(condNo) )
					{
						// Condition 2 has been met
						cond2Met = true;
					}
				}
			}
			else
			{
				// No 2nd condition, so it's met
				cond2Met = true;
			}

			if (( cond1Met ) && ( cond2Met ))
			{
				// Execute each action, each bit relates to an action no
				for (x=0; x<LOGIC_ACT_CNT; x++)
				{
					if ( SystemConfig.LogicEvents.actionMask[i] & (1<<x) )
					{
						LogicTakeAction(x);
					}
				}
			}

			cond1Met = false;
			cond2Met = false;
		}

		if (cosmLimit) cosmLimit --;

		prevNetworkConnected = networkConnected;

		// If we needed to action on boot up it would have been done now .
		justBooted = false;
	}
}

//*****************************************************************************
//
// LogicStartStop - Enable/Disable Logic
//
//*****************************************************************************
void LogicStartStop ( bool start )
{
ui8 i = 0;

	// Set up the conditions from the config
	//LogicConditions = (LOGIC_CONDITION *)SystemConfig.LogicConditionsBuffer;

	// limit events
	for (i=0; i<LOGIC_EVENT_CNT; i++)
	{
		if (SystemConfig.LogicEvents.conditionNumber1[i] > LOGIC_COND_CNT-1 )
		{
			SystemConfig.LogicEvents.conditionNumber1[i] = 0xFF;
		}
		if (SystemConfig.LogicEvents.conditionNumber2[i] > LOGIC_COND_CNT-1 )
		{
			SystemConfig.LogicEvents.conditionNumber2[i] = 0xFF;
		}

	}

	// Get the initial state
	networkConnected = Ethernet_Connected();

	//LogicRunning = start;
}

//*****************************************************************************
//
// LogicProcessCondition - Process the conditions event, return true if it should actioned
//
//*****************************************************************************
bool LogicProcessCondition ( ui8 conditionNo )
{
bool result = false;
ui32 param1;
ui32 param2;
LOGIC_COND_TYPE cond;
volatile ui8 tempUi8 = 0;

	if ( conditionNo < LOGIC_COND_CNT )
	{
		param1 = SystemConfig.LogicConditions.condParam1[conditionNo];
		param2 = SystemConfig.LogicConditions.condParam2[conditionNo];
		cond = SystemConfig.LogicConditions.condType[conditionNo];

		switch ( cond )
		{
			case L_EVENT_GPIO_RAISING :
				if ( LogicCheckPinHigh(conditionNo) )
				{
					result = true;
				}
			break;

			case L_EVENT_GPIO_FALLING :
				if ( LogicCheckPinLow(conditionNo) )
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

				if ( AdcGetTemperature() > param2 )
				{
					result = true;
				}

			break;

			case L_EVENT_TEMP_BELOW :

				if ( AdcGetTemperature() < param2 )
				{
					result = true;
				}

			break;

			case L_EVENT_REG_EQUAL :

				if (param1 < LOGIC_NUM_OF_REGISTERS)
				{
					if (LogicRegisters[param1] == param2)
					{
						result = true;
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
				if ( param1 > Time_StampNow(SystemConfig.timeOffset))
				{
					result = true;
				}
			break;

			case L_EVENT_DATE_BEFORE :
				if ( param1 < Time_StampNow(SystemConfig.timeOffset))
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

			case L_EVENT_NET_MSG :
				// Network message to trigger action
				// TODO : this could supply a value and if it matches the param then triggers the saved action?
			break;

		}
	}

	return ( result );
}

//*****************************************************************************
//
// LogicTakeAction - Complete the supplied action
//
//*****************************************************************************
void LogicTakeAction ( ui8 actionNo )
{
ui32 param1 = SystemConfig.LogicActions.actionParam1[actionNo];
ui32 param2 = SystemConfig.LogicActions.actionParam2[actionNo];

	if ( actionNo < LOGIC_ACT_CNT )
	{
		switch ( SystemConfig.LogicActions.actionType[actionNo] )
		{
			case L_ACTION_GPIO_HIGH :
				LogicSetPinHigh( actionNo );
			break;

			case L_ACTION_GPIO_LOW :
				LogicSetPinLow( actionNo );
			break;

			case L_ACTION_PWM_DUTY :
				// Param 1 would be the PWM channel, 2 is the duty to set now
				pwmSetDuty(param1, param2);
			break;

			case L_ACTION_INCREMENT_REG :
				if ( LogicRegisters[param1] < 0xFFFFFFFF )
				{
					// Increment the register by the parm2 value
					LogicRegisters[ param1 ] += param2;
				}
			break;

			case L_ACTION_DECREMENT_REG :
				if ( LogicRegisters[param1] )
				{
					LogicRegisters[ param1 ] --;
				}
			break;

			case L_ACTION_CLEAR_REG :
				LogicRegisters[param1] = 0;
			break;

			case L_ACTION_SET_REG :
				LogicRegisters[param1] = param2;
			break;

			case L_ACTION_NET_MSG :
				SSC_SendPortInfo(param1, param2);
			break;

			case L_ACTION_SERIAL_MSG :

			break;

			case L_ACTION_CONTROL_RELAY :
				RelayControl(param1, param2);
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
	}
}


//*****************************************************************************
//
// LogicSetPinHigh - Set Pin on Port High
//
//*****************************************************************************
void LogicSetPinHigh ( ui8 actionNo )
{
	UserGpioSetOutputs( SystemConfig.LogicActions.actionParam1[actionNo], SystemConfig.LogicActions.actionParam2[actionNo], SystemConfig.LogicActions.actionParam2[actionNo] );
}

//*****************************************************************************
//
// LogicSetPinLow - Set Pin on Port Low
//
//*****************************************************************************
void LogicSetPinLow ( ui8 actionNo )
{
	UserGpioSetOutputs( SystemConfig.LogicActions.actionParam1[actionNo], SystemConfig.LogicActions.actionParam2[actionNo], 0 );
}


//*****************************************************************************
//
// LogicTask - Called at a set rate to check for actions to perform.
//
//*****************************************************************************
void LogicCapturePortData ( void )
{
ui8 i = 0;
	// Record the status of all the GPIO ports for interrogation

	for ( i=0; i<GPIO_PORT_TOTAL; i++ )
	{
		LogicGpioDataPrev[i] = LogicGpioData[i];
		UserGpioGet(i, &LogicGpioData[i]);
	}
}


//*****************************************************************************
//
// LogicCheckPinHigh - check to see if the pin is high using our cached pin status
//
//*****************************************************************************
bool LogicCheckPinHigh ( ui8 conditionNo )
{
bool result = false;
volatile ui32 param1 = SystemConfig.LogicConditions.condParam1[ conditionNo ];
volatile ui32 param2 = SystemConfig.LogicConditions.condParam2[ conditionNo ];

	if ( param1 < GPIO_PORT_TOTAL )
	{
		if (~(LogicGpioDataPrev[param1] & param2) & (LogicGpioData[param1] & param2))
		{
			// A bit has been pulled high
			result = true;
		}
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
volatile ui32 param1 = SystemConfig.LogicConditions.condParam1[ conditionNo ];
volatile ui32 param2 = SystemConfig.LogicConditions.condParam2[ conditionNo ];

if ( param1 < GPIO_PORT_TOTAL )
	{
		if (( LogicGpioDataPrev[param1] & param2 ) & ~(LogicGpioData[param1] & param2))
		{
			// A bit has been lowered
			result = true;
		}
		else
		{
			// A bit has been pulled high
		}
	}

	return( result );
}

//*****************************************************************************
//
// LogicEditAction - Edits an action in the list
//
//*****************************************************************************
void LogicEditCondition( ui8 position, ui8 *newCondition )
{
	if (( position < LOGIC_COND_CNT ) && ( newCondition ))
	{
		if ( newCondition[0] < L_EVENT_MAX )
		{
			// Action in range
			SystemConfig.LogicConditions.condType[position] = (LOGIC_COND_TYPE)newCondition[0];
			memcpy( &SystemConfig.LogicConditions.condParam1[position], &newCondition[1], 4);
			memcpy( &SystemConfig.LogicConditions.condParam2[position], &newCondition[5], 4);
		}
		else
		{
			SystemConfig.LogicConditions.condType[position] = L_EVENT_INVALID;
		}
	}
	else
	{
		// No! out of range or invalid pointer
	}
}

//*****************************************************************************
//
// LogicEditAction - Edits an action in the list
//
//*****************************************************************************
void LogicEditAction( ui8 position, ui8 *newAction )
{
	if (( position < LOGIC_ACT_CNT ) && ( newAction ))
	{
		if ( newAction[0] < L_ACTION_MAX )
		{
			// Action in range
			SystemConfig.LogicActions.actionType[position] = (LOGIC_ACTION_TYPE)newAction[0];
			memcpy( &SystemConfig.LogicActions.actionParam1[position], &newAction[1], 4);
			memcpy( &SystemConfig.LogicActions.actionParam2[position], &newAction[5], 4);
		}
		else
		{
			SystemConfig.LogicActions.actionType[position] = L_ACTION_INVALID;
		}
	}
	else
	{
		// No! out of range or invalid pointer
	}
}

//*****************************************************************************
//
// LogicEditEvent
//
//*****************************************************************************
void LogicEditEvent ( ui8 position, ui8 *newEvent )
{
	if (( position < LOGIC_EVENT_CNT ) && ( newEvent ))
	{
		// TODO : Range check conditions
		SystemConfig.LogicEvents.conditionNumber1[position] = newEvent[0];
		SystemConfig.LogicEvents.conditionNumber2[position] = newEvent[1];
		memcpy(&SystemConfig.LogicEvents.actionMask[position], &newEvent[2], 4);
	}
	else
	{
		// No! out of range or invalid pointer
	}
}

//*****************************************************************************
//
// LogicCopyActions
//
//*****************************************************************************
void LogicCopyActions ( ui8 *buff )
{
	if ( buff )
	{
		memcpy( buff, &SystemConfig.LogicActions, sizeof( SystemConfig.LogicActions ) );
	}
}

//*****************************************************************************
//
// LogicCopyConditions
//
//*****************************************************************************
void LogicCopyConditions ( ui8 *buff )
{
	if ( buff )
	{
		memcpy( buff, &SystemConfig.LogicConditions, sizeof( SystemConfig.LogicConditions ) );
	}
}

//*****************************************************************************
//
// LogicCopyEvents
//
//*****************************************************************************
void LogicCopyEvents ( ui8 *buff )
{
	if ( buff )
	{
		memcpy( buff, &SystemConfig.LogicEvents, sizeof( SystemConfig.LogicEvents ) );
	}
}
