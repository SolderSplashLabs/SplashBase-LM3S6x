
#ifdef _LOGIC_H_

// We have a number of "Registers" that actions can effect and can then in turn generate new events

typedef enum LOGIC_EVENT_TYPE
{
	L_EVENT_INVALID,
	L_EVENT_GPIO_RAISING,
	L_EVENT_GPIO_FALLING,
	L_EVENT_ADC_ABOVE,
	L_EVENT_ADC_BELOW,
	L_EVENT_NET_DISCONNECT,
	L_EVENT_NET_CONNECT,
	L_EVENT_TEMP_ABOVE,
	L_EVENT_TEMP_BELOW,
	L_EVENT_REG_EQUAL,
	L_EVENT_REG_ABOVE,
	L_EVENT_REG_BELOW,
	L_EVENT_REPEAT_TIMER,
	L_EVENT_GPIO_RAISING_DB,
	L_EVENT_GPIO_FALLING_DB,
	L_EVENT_BOOT
} LOGIC_EVENT_TYPE;

typedef enum LOGIC_ACTION_TYPE
{
	L_ACTION_INVALID,
	L_ACTION_GPIO_HIGH,
	L_ACTION_GPIO_LOW,
	L_ACTION_PWM_DUTY,
	L_ACTION_INCREMENT_REG,
	L_ACTION_DECREMENT_REG,
	L_ACTION_CLEAR_REG,
	L_ACTION_SET_REG,
	L_ACTION_NET_MSG,
	L_ACTION_SERIAL_MSG,
	L_ACTION_CONTROL_RELAY,
	L_ACTION_SEND_COSM
} LOGIC_ACTION_TYPE;

typedef struct LOGIC_CONDITION
{
	LOGIC_EVENT_TYPE eventType;			// This event has to occur ( using the 2 parameters ) before the action is taken
	LOGIC_EVENT_TYPE andEventType;		// if this event is set to somthing other than Invalid, this condition must also be satisfied before executing the action
	LOGIC_ACTION_TYPE actionType;

	ui32 eventParam1;
	ui32 eventParam2;
	ui32 andEventParam1;
	ui32 andEventParam2;
	ui32 actionParam1;
	ui32 actionParam2;

	ui8 active:1;					// This condition is active
	ui8 actioned:1;					// Condition has been action, clear event condition is no longer true

} LOGIC_CONDITION;

#define LOGIC_MAX_CONDITIONS	10
#define GPIO_PORTS_MAX 7

// a cache of the GPIO Ports, that are processed as a single snapshot
ui32 LogicGpioData[ GPIO_PORTS_MAX ];

#endif

void LogicTask ( void );
void LogicSetPinHigh ( ui8 conditionNo );
void LogicSetPinLow ( ui8 conditionNo );
void LogicTakeAction ( ui8 conditionNo );
bool LogicCheckPinLow ( ui8 conditionNo );
bool LogicCheckPinHigh ( ui8 conditionNo );
void LogicCapturePortData ( void );
bool LogicProcessCondition ( ui8 conditionNo, bool secondaryEvent );
void LogicInsertNewCondition (ui8 position, ui8 *newCondition );

