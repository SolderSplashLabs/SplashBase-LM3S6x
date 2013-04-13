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
// We have a number of "Registers" that actions can effect and can then in turn generate new events
#define LOGIC_NUM_OF_REGISTERS	10
#define LOGIC_COND_CNT		24
#define LOGIC_ACT_CNT		24
#define LOGIC_EVENT_CNT		24


typedef enum LOGIC_COND_TYPE
{
	L_EVENT_INVALID,

	L_EVENT_GPIO_RAISING,
	L_EVENT_GPIO_FALLING,
	L_EVENT_GPIO_HIGH,
	L_EVENT_GPIO_LOW,

	L_EVENT_ADC_ABOVE,
	L_EVENT_ADC_BELOW,
	L_EVENT_ADC_BETWEEN,

	L_EVENT_REG_EQUAL,
	L_EVENT_REG_ABOVE,
	L_EVENT_REG_BELOW,

	L_EVENT_DATE_AFTER,
	L_EVENT_DATE_BEFORE,
	L_EVENT_DATE_BETWEEN,
	L_EVENT_TIME_AFTER,
	L_EVENT_TIME_BEFORE,
	L_EVENT_TIME_BETWEEN,

	L_EVENT_NET_DISCONNECT,
	L_EVENT_NET_CONNECT,

	L_EVENT_BOOT,
	L_EVENT_EVERY_TICK,

	L_EVENT_TEMP_ABOVE,
	L_EVENT_TEMP_BELOW,

	L_EVENT_MAX
} LOGIC_COND_TYPE;

// Future Events
// Incoming UDP message
// Debounced GPIO

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
	L_ACTION_SERVO_POS,
	L_ACTION_RGB,
	L_ACTION_SEND_COSM,
	L_ACTION_MAX
} LOGIC_ACTION_TYPE;

// Using a struct of arrays instead of an array of structures to avoid structure packing
typedef struct LOGIC_CONDITIONS
{
	LOGIC_COND_TYPE condType[LOGIC_COND_CNT];
	ui32 condParam1[LOGIC_COND_CNT];
	ui32 condParam2[LOGIC_COND_CNT];

} LOGIC_CONDITIONS;

typedef struct LOGIC_ACTIONS
{
	LOGIC_ACTION_TYPE actionType[LOGIC_ACT_CNT];
	ui32 actionParam1[LOGIC_ACT_CNT];
	ui32 actionParam2[LOGIC_ACT_CNT];
} LOGIC_ACTIONS;

// The logic lists checks 1 or 2 Conditions if true it then actions all actions defined by the actionMask.
typedef struct LOGIC_LIST
{
	ui8 conditionNumber1[LOGIC_EVENT_CNT];
	ui8 conditionNumber2[LOGIC_EVENT_CNT];
	ui32 actionMask[LOGIC_EVENT_CNT];
} LOGIC_EVENTS;


// Note : Add remote control another splashbase option, set/clear relay? set output?

/*
// 28 bytes per. 560 total
typedef struct LOGIC_CONDITION
{
	LOGIC_EVENT_TYPE eventType;			// This event has to occur ( using the 2 parameters ) before the action is taken
	LOGIC_EVENT_TYPE andEventType;		// if this event is set to something other than Invalid, this condition must also be satisfied before executing the action
	LOGIC_ACTION_TYPE actionType;

	ui8 spare2:1;
	ui8 actioned:1;					// Condition has been action, clear event condition is no longer true
	ui8 spare:6;

	ui32 eventParam1;
	ui32 eventParam2;
	ui32 andEventParam1;
	ui32 andEventParam2;
	ui32 actionParam1;
	ui32 actionParam2;

} LOGIC_CONDITION;
*/

#ifdef _LOGIC_H_

// a cache of the GPIO Ports, that are processed as a single snapshot
ui32 LogicGpioData[ GPIO_PORT_TOTAL ];
ui32 LogicGpioDataPrev[ GPIO_PORT_TOTAL ];

// TODO : ADC above and below will constantly trigger how do you use them

/*
const char *LOGIC_EVENT_TYPE_NAMES[L_EVENT_MAX] = 	{	"None",
		"GPIO RAISING",
		"GPIO FALLING",
		"ADC ABOVE",
		"ADC BELOW",
		"NET DISCONNECT",
		"NET CONNECT",
		"TEMP ABOVE",
		"TEMP BELOW",
		"REG EQUAL",
		"REG ABOVE",
		"REG BELOW",
		"REPEAT TIMER",
		"GPIO RAISING DEBOUNCED",
		"GPIO FALLING DEBOUNCED",
		"BOOT",
		"EVERY_TICK",
		"DATE AFTER",
		"DATE BEFORE",
		"TIME AFTER",
		"TIME BEFORE",
		"NET MSG",
		"GPIO_HIGH",
		"GPIO_LOW",
};
*/

const char *LOGIC_ACTION_TYPE_NAMES[L_ACTION_MAX] = {	"None",
		"GPIO HIGH",
		"GPIO LOW",
		"PWM DUTY",
		"INCREMENT REG",
		"DECREMENT REG",
		"CLEAR REG",
		"SET REG",
		"NET MSG",
		"SERIAL MSG",
		"CONTROL RELAY",
		"SERVO POS",
		"RGB",
		"SEND COSM"};

#else

extern const char *LOGIC_EVENT_TYPE_NAMES[L_EVENT_MAX];
//extern LOGIC_CONDITION *LogicConditions;

#endif

void LogicTask ( void );
void LogicCopyActions ( ui8 *buff );
void LogicCopyConditions ( ui8 *buff );
void LogicCopyEvents ( ui8 *buff );
void LogicEditAction( ui8 position, ui8 *newAction );
void LogicEditEvent ( ui8 position, ui8 *newEvent );
void LogicEditCondition( ui8 position, ui8 *newCondition );
bool LogicProcessCondition ( ui8 conditionNo );

void LogicSetPinHigh ( ui8 conditionNo );
void LogicSetPinLow ( ui8 conditionNo );
void LogicTakeAction ( ui8 conditionNo );
bool LogicCheckPinLow ( ui8 conditionNo );
bool LogicCheckPinHigh ( ui8 conditionNo );
void LogicCapturePortData ( void );

void LogicStartStop ( bool start );

/*
const char * LogicGetActionStr ( ui8 position );
const char * LogicGetEventStr ( ui8 position, bool primary );
*/
