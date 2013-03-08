#define SECONDS_IN_AN_MIN		60L
#define SECONDS_IN_AN_HOUR		3600L
#define SECONDS_IN_AN_DAY		86400L
#define SECONDS_IN_HALF_DAY		43200L


typedef struct TIME_STRUCT
{
	ui8 hour;
	ui8 min;
	ui8 second;

	ui8 day;
	ui8 month;
	ui8 year;

} TIME_STRUCT;

void Time_Task ( void );
ui32 Time_StampNow ( si32 minOffset );
ui32 Time_Uptime ( void );
void Time_Get ( TIME_STRUCT *time );
void Time_SetUnix( ui32 timeStamp );
