#ifdef PWMCONTROL

struct PWM_SETTINGS_STRUCT
{
	ui16 duty0;		// Set PWM Duty
	ui16 duty1;
	ui16 duty2;
	ui16 freq;		// PWM Freq
} PwmSettings;

bool PwmStatus;

#endif 

extern bool PwmStatus;

void pwmInit( void );
void pwmOn( void );
void pwmOff( void );
void pwmSetDuty( ui16 duty, ui8 mask );
void pwmSetFreq( ui16 periodVal, ui8 mask );
ui16 pwmGetFreq( void );
ui16 pwmGetDuty( ui8 pwmNo );
