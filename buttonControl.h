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

#ifndef BUTTONCONTROL_H_
#define BUTTONCONTROL_H_

#define BUTTON1_PORT	GPIO_PORTB_BASE
#define BUTTON1_PIN		GPIO_PIN_4

#define BUTTON2_PORT	GPIO_PORTB_BASE
#define BUTTON2_PIN		GPIO_PIN_5

#define BUTTON_PRESSED_LIMIT	50
#define BUTTON_HELD_LIMIT		5000


#endif /*BUTTONCONTROL_H_*/

void Buttons_Init ( void );
void Buttons_Task( ui32 tickTimeBase );
ui8 Buttons_CheckForPressed ( void );
ui8 Buttons_CheckForHeld ( void );
void Buttons_Acknowledge ( void );
