/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2012-02-26 - www.soldersplash.co.uk - C. Matthews - R. Steel

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

*/


typedef struct COLOUR_SETTINGS
{
	ui8 colourMode;
	ui8 topRed;
	ui8 topGreen;
	ui8 topBlue;
	ui8 bottomRed;
	ui8 bottomGreen;
	ui8 bottomBlue;
	ui16 maxSteps;
	ui8 ticksPerStep;
} COLOUR_SETTINGS;

extern COLOUR_SETTINGS ColourSettings;

void ColourModeCycle ( void );
ui8 ColourModeBounce ( void );
void ColourModeFlash ( void );
void ColourModeManual ( void );
void ColourModeTick( void );
void ColourModeSet( ui8 mode, ui32 colourOne, ui32 colourTwo, ui16 noOfSteps, ui8 ticksPerStep );
void ColourModeRandomBounce ( void );

enum COLOUR_MODES 
{
	COLOUR_MODE_OFF = 0,
	COLOUR_MODE_CYCLE,
	COLOUR_MODE_BOUNCE,
	COLOUR_MODE_MANUAL,
	COLOUR_MODE_BOUNCE_RAND
};

#ifdef COLOUR_MODES

// Map the colours to PWM/CCP Channels
#define COLOUR_RED_PWM_CH		0x01
#define COLOUR_GREEN_PWM_CH		0x02
#define COLOUR_BLUE_PWM_CH		0x04

COLOUR_SETTINGS ColourSettings;

#endif /*COLOURMODES_H_*/


